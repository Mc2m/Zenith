
#include "stdafx.h"

#include "common.h"
#include "pipe.h"

#include "state.h"

#include "transfer.h"

#include "structures/Array.h"
#include "particle/particle.h"

typedef struct _PipeData {
	size_t num_data;
	PCV *v;
	lua_State *waiting;
	unsigned char wants_table;
	Array *localnumdata;
} PipeData;

lua_State *pipes = 0;
ParticleMutex *m = 0;
Array *data = 0;
size_t next_id = 1;

static PipeData *pipedata_new(void)
{
	PipeData *p = (PipeData *) malloc(sizeof(PipeData));

	p->num_data = 0;
	p->v = 0;
	p->waiting = 0;
	p->wants_table = 0;
	p->localnumdata = array_new(0);

	return p;
}

static void pipedata_free(PipeData *p)
{
	if(p) {
		particle_condition_var_free(p->v);
		array_free(p->localnumdata,free);
		free(p);
	}
}

void zenith_pipe_initialize(void)
{
	pipes = lua_open();
	m = particle_mutex_new(PARTICLE_MUTEX_TYPE_RECURSIVE);

	luaL_openlibs(pipes);

	lua_newtable(pipes); // table containing pipes
	l_getElement(pipes,"table","remove",0); // table.remove must stay on top of the stack

	data = array_new(0);
}

void zenith_pipe_destroy(void)
{
	size_t i = 0;

	lua_close(pipes);
	particle_mutex_free(m);
	pipes = 0;
	m = 0;

	array_free(data,(void (*) (void *))pipedata_free);
	data = 0;
}

static inline unsigned char pipe_get_pipe(size_t id) {
	lua_checkstack(pipes,2);

	lua_pushinteger(pipes,id);
	lua_gettable(pipes,1);

	return lua_isnil(pipes,-1);
}

static inline void pipe_send(lua_State *L, size_t id, unsigned char wait, int idx)
{
	size_t i, j = idx+1, numdata = lua_gettop(L);
	PipeData *d = (PipeData *) array_get(data,id-1);

	//printf("sending\n");

	if(d->waiting) {
		// copy the sent data directly
		if(d->wants_table) {
			i = 1;
			lua_createtable(d->waiting,0,numdata-1);
			for(; j <= numdata; ++j) {
				lua_pushnumber(d->waiting,i++);
				zenith_transfer_data(L,d->waiting,j);
				lua_settable(d->waiting,-3);
			}
		} else for(; j <= numdata; ++j) zenith_transfer_data(L,d->waiting,j);
	} else {
		int *ptr = (int *) malloc(sizeof(int));
		*ptr = numdata - 1;

		//get current pipe
		if(pipe_get_pipe(id)) {
			particle_mutex_unlock(m);
			luaL_error(L,"There are no pipe with id %d",id);
			return;
		}

		i = d->num_data;

		// keep local numdata
		array_push(d->localnumdata,ptr);

		for(; j <= numdata; ++j) {
			lua_pushnumber(pipes,++i);
			zenith_transfer_data(L,pipes,j);
			lua_settable(pipes,-3);
		}

		d->num_data += *ptr;

		lua_pop(pipes,1); // pop the pipe

		//printf("sent\n");
	}

	if(wait) {
		d->waiting = L;
		if(!d->v) d->v = particle_condition_var_new(m);
		else particle_condition_var_wake(d->v);
		particle_condition_var_sleep(d->v);
		if(d->waiting == L) d->waiting = 0;
	}
}

void zenith_pipe_custom_send(lua_State *L, int idx, void (*cpy)(lua_State *from, lua_State *to, int idx))
{
	//find pipe id
	size_t id = l_getintfield(L,idx,"id");
	PipeData *d = (PipeData *) array_get(data,id-1);

	if(lua_gettop(L) > 1) {
		particle_mutex_lock(m);
		zenith_transfer_set_table_transfer_method(cpy);
		pipe_send(L,id,0,idx);
		zenith_transfer_set_default_table_transfer_method();
		particle_mutex_unlock(m);
	}

	if(d->v) particle_condition_var_wake(d->v);

	//consume the elements
	lua_pop(L,lua_gettop(L) - idx + 1);
}

int zenith_pipe_send(lua_State *L)
{
	// STACK
	// 1 - pipe obj
	// 2 - data
	// 3 - data
	// ...

	//find pipe id
	size_t id = l_getintfield(L,1,"id");
	PipeData *d = (PipeData *) array_get(data,id-1);

	if(lua_gettop(L) > 1) {
		particle_mutex_lock(m);
		pipe_send(L,id,0,1);
		particle_mutex_unlock(m);
	}

	if(d->v) particle_condition_var_wake(d->v);

	return 0;
}

static inline size_t pipe_receive(lua_State *L, size_t id, unsigned char to_table, unsigned char receive_all)
{
	PipeData *d = (PipeData *) array_get(data,id-1);
	size_t numdata = d->num_data;

	if(numdata) {
		size_t j = numdata, i = 1;

		if(to_table) lua_createtable(L,0,numdata);

		//printf("receiving\n");

		//get the pipe
		if(pipe_get_pipe(id)) {
			particle_mutex_unlock(m);
			lua_pop(L,1);
			luaL_error(L,"There are no pipe with id %d",id);
			return 0;
		}

		//push the data into the table
		numdata = 0;

		if(receive_all) {
			array_empty(d->localnumdata,free);
		} else {
			int *amount = (int *) array_pop(d->localnumdata,0);
			j = *amount;
			free(amount);
		}

		while(j--) {
			lua_pushvalue(pipes,2); //duplicate table remove function

			lua_pushvalue(pipes,-2); // pipe table
			lua_pushnumber(pipes,1); // id
			lua_call(pipes,2,1);

			if(to_table) {
				lua_pushnumber(L,i++);
				zenith_transfer_data(pipes,L,-1);
				lua_settable(L,-3);
			} else {
				zenith_transfer_data(pipes,L,-1);
			}
			numdata++;

			lua_pop(pipes,1);
		}

		d->num_data -= numdata;

		//printf("received\n");

		lua_pop(pipes,1); // pop the pipe

		return to_table ? 1 : numdata;
	}

	return 0;
}

int zenith_pipe_receive(lua_State *L)
{
	// STACK
	// 1 pipe object
	// 2 option

	//find pipe name

	size_t id = l_getintfield(L,1,"id"),numdata;
	unsigned char opt = luaL_optint(L,2,1);

	particle_mutex_lock(m);
	numdata = pipe_receive(L,id,0,opt);
	particle_mutex_unlock(m);

	return (int) numdata;
}

int zenith_pipe_receive_table(lua_State *L)
{
	// STACK
	// 1 pipe object

	//find pipe name

	size_t id = l_getintfield(L,1,"id"),numdata;
	unsigned char opt = luaL_optint(L,2,1);

	particle_mutex_lock(m);
	numdata = pipe_receive(L,id,1,opt);
	particle_mutex_unlock(m);

	return (int) numdata;
}

int zenith_pipe_send_wait(lua_State *L)
{
	// STACK
	// 1 - pipe obj
	// 2 - data
	// 3 - data
	// ...

	if(lua_gettop(L) > 1) {
		//find pipe id
		size_t id = l_getintfield(L,1,"id");
		PipeData *d = (PipeData *) array_get(data,id-1);
		int numdata = lua_gettop(L);

		particle_mutex_lock(m);
		d->wants_table = 0;
		pipe_send(L,id,1,1);
		particle_mutex_unlock(m);

		return lua_gettop(L) - numdata;
	}

	return 0;
}

int zenith_pipe_send_wait_table(lua_State *L)
{
	// STACK
	// 1 - pipe obj
	// 2 - data
	// 3 - data
	// ...

	size_t id = l_getintfield(L,1,"id");
	PipeData *d = (PipeData *) array_get(data,id-1);

	if(lua_gettop(L) > 1) {
		//find pipe id
		int numdata = lua_gettop(L);

		particle_mutex_lock(m);
		d->wants_table = 1;
		pipe_send(L,id,1,1);
		particle_mutex_unlock(m);

		return lua_gettop(L) - numdata;
	}

	if(d->v) particle_condition_var_wake(d->v);

	return 0;
}

int zenith_pipe_listen(lua_State *L)
{
	// STACK
	// 1 - pipe obj
	// 2 - option

	//find pipe id
	size_t id = l_getintfield(L,1,"id");
	PipeData *d = (PipeData *) array_get(data,id-1);
	size_t numdata;
	unsigned char opt = luaL_optint(L,2,1);

	particle_mutex_lock(m);

	if(d->num_data) {
		numdata = pipe_receive(L,id,0,opt);
	} else {
		d->wants_table = 0;
		d->waiting = L;
		if(!d->v) d->v = particle_condition_var_new(m);
		particle_condition_var_sleep(d->v);
		if(d->waiting == L) d->waiting = 0;
		numdata = lua_gettop(L) - 1;
	}
	
	particle_mutex_unlock(m);

	return numdata;
}

int zenith_pipe_listen_table(lua_State *L)
{
	// STACK
	// 1 - pipe obj
	// 2 - data
	// 3 - data
	// ...

	size_t id = l_getintfield(L,1,"id");
	PipeData *d = (PipeData *) array_get(data,id-1);
	size_t numdata;
	unsigned char opt = luaL_optint(L,2,1);

	particle_mutex_lock(m);

	if(d->num_data) {
		numdata = pipe_receive(L,id,1,opt);
	} else {
		d->wants_table = 1;
		d->waiting = L;
		if(!d->v) d->v = particle_condition_var_new(m);
		particle_condition_var_sleep(d->v);
		d->waiting = 0;
		numdata = lua_gettop(L) - 1;
	}
	
	particle_mutex_unlock(m);

	return numdata;
}

static inline void table_table(lua_State *L)
{
	l_setfunctionfield(L,-1,"wait",zenith_pipe_send_wait_table);
	l_setfunctionfield(L,-1,"receive",zenith_pipe_receive_table);
	l_setfunctionfield(L,-1,"listen",zenith_pipe_listen_table);
}

static inline void access_table(lua_State *L)
{
	lua_createtable(L,0,3);

	l_setfunctionfield(L,-1,"send",zenith_pipe_send);
	l_setfunctionfield(L,-1,"wait",zenith_pipe_send_wait);
	l_setfunctionfield(L,-1,"listen",zenith_pipe_listen);
	l_setfunctionfield(L,-1,"receive",zenith_pipe_receive);
	l_settablefield(L,-1,"table",table_table);

	lua_pushvalue(L,-1);
	lua_setfield(L,-2,"__index");

	lua_pushvalue(L,-1);
	lua_setfield(L,-3,"access");
}

void set_pipe_table(lua_State *L, const char *name)
{
	// check for Zenith table
	lua_getglobal(L,"Zenith");
	if(lua_isnil(L,-1)) {
		lua_pop(L,1);

		lua_createtable(L,0,1);
		lua_pushvalue(L,-1);
		lua_setglobal(L,"Zenith");
	}

	// check for Pipe table
	lua_getfield(L,-1,"Pipe");
	if(lua_isnil(L,-1)) {
		lua_pop(L,1);

		//pipe table
		lua_createtable(L,0,2);

		//access table
		access_table(L);

		//pipes table
		lua_createtable(L,0,1);
		lua_pushvalue(L,-1);
		lua_setfield(L,-4,"pipes");

		lua_pushvalue(L,-3);
		lua_setfield(L,-5,"Pipe");
	} else {
		lua_getfield(L,-1,"access");
		lua_getfield(L,-2,"pipes");
	}

	// STACK
	// -1 pipes table
	// -2 access table
	// -3 Pipe table
	// -4 Zenith table
	
	lua_createtable(L,0,1);
	l_setintfield(L,-1,"id",next_id);

	lua_pushvalue(L,-1);
	lua_setfield(L,-2,"__index");

	//set access as metatable
	lua_pushvalue(L,-3);
	lua_setmetatable(L,-2);

	//push to pipes table
	lua_setfield(L,-2,name);

	lua_pop(L,4);
}

void zenith_pipe_create(lua_State *L1,lua_State *L2, const char *name)
{
	char namerplcmnt[64];
	if(L1 == L2) return;

	if(!pipes) zenith_pipe_initialize();

	if(!name) {
		size_t i1 = zenith_state_from_state(L1);
		size_t i2 = zenith_state_from_state(L2);

		snprintf(namerplcmnt,sizeof(namerplcmnt),"pipe%d%d",i1,i2);

		name = namerplcmnt;
	}

	//set pipe
	particle_mutex_lock(m);
	lua_pushinteger(pipes,next_id);
	lua_newtable(pipes);
	lua_settable(pipes,1);

	array_push(data,pipedata_new());
	particle_mutex_unlock(m);

	set_pipe_table(L1,name);
	set_pipe_table(L2,name);

	next_id++;
}
