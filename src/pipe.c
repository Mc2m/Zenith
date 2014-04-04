
#include "stdafx.h"

#include "common.h"
#include "pipe.h"

#include "state.h"

#include "transfer.h"

#include "particle/particle.h"

typedef struct _PipeData {
	size_t num_data;
	PCV *v;
} PipeData;

lua_State *pipes = 0;
ParticleMutex *m = 0;
PipeData *data = 0;
size_t next_id = 1;

void zenith_pipe_initialize(void)
{
	pipes = lua_open();
	m = particle_mutex_new(PARTICLE_MUTEX_TYPE_RECURSIVE);

	luaL_openlibs(pipes);

	lua_newtable(pipes); // table containing pipes
	l_getElement(pipes,"table","remove",0); // table.remove must stay on top of the stack
}

void zenith_pipe_destroy(void)
{
	size_t i = 0;

	lua_close(pipes);
	particle_mutex_free(m);
	pipes = 0;
	m = 0;
	
	for(; i < next_id-1; ++i) particle_condition_var_free(data[i].v);

	free(data);
	data = 0;
}

static inline unsigned char pipe_get_pipe(size_t id) {
	lua_checkstack(pipes,2);

	lua_pushinteger(pipes,id);
	lua_gettable(pipes,1);

	return lua_isnil(pipes,-1);
}

int zenith_pipe_send(lua_State *L)
{
	// STACK
	// 1 - pipe obj
	// 2 - data
	// 3 - data
	// ...

	size_t i, j = 2, numdata = lua_gettop(L);
	size_t id;
	PipeData *d;
	
	if(numdata < 2) return 0;

	//find pipe name
	id = l_getintfield(L,1,"id");

	particle_mutex_lock(m);

	//get current pipe
	if(pipe_get_pipe(id)) {
		particle_mutex_unlock(m);
		luaL_error(L,"There are no pipe with id %d",id);
		return 0;
	}

	i = lua_objlen(pipes,-1);

	for(; j <= numdata; ++j) {
		lua_pushnumber(pipes,++i);
		zenith_transfer_data(L,pipes,j);
		lua_settable(pipes,-3);
	}

	d = &data[id-1];
	d->num_data += numdata - 1;
	if(data->v) particle_condition_var_wake(data->v);

	lua_pop(pipes,1); // pop the pipe

	particle_mutex_unlock(m);

	return 0;
}

int zenith_pipe_receive(lua_State *L)
{
	// STACK
	// -1 pipe object or timeout


	//find pipe name

	size_t id = l_getintfield(L,1,"id"), numdata, j, i = 1;

	particle_mutex_lock(m);

	numdata = data[id-1].num_data;

	if(numdata) {
		//get the pipe
		if(pipe_get_pipe(id)) {
			particle_mutex_unlock(m);
			lua_pop(L,1);
			luaL_error(L,"There are no pipe with id %d",id);
			return 0;
		}

		j = numdata;

		//push the data into the table
		while(j--) {
			//duplicate table remove function
			lua_pushvalue(pipes,2);

			lua_pushvalue(pipes,-2); // pipe table
			lua_pushnumber(pipes,1); // id
			lua_call(pipes,2,1);

			zenith_transfer_data(pipes,L,-1);

			lua_pop(pipes,1);
		}

		data[id-1].num_data = 0;
	}

	lua_pop(pipes,1); // pop the pipe

	particle_mutex_unlock(m);

	return numdata;
}

int zenith_pipe_send_wait(lua_State *L)
{
	// STACK
	// 1 - pipe obj
	// 2 - data
	// 3 - data
	// ...

	size_t i, j = 2, numdata = lua_gettop(L);
	size_t id;
	PipeData *d;
	
	if(numdata < 2) return 0;

	//find pipe name
	id = l_getintfield(L,1,"id");

	particle_mutex_lock(m);

	//get current pipe
	if(pipe_get_pipe(id)) {
		particle_mutex_unlock(m);
		luaL_error(L,"There are no pipe with id %d",id);
		return 0;
	}

	i = lua_objlen(pipes,-1);

	for(; j <= numdata; ++j) {
		lua_pushnumber(pipes,++i);
		zenith_transfer_data(L,pipes,j);
		lua_settable(pipes,-3);
	}

	d = &data[id-1];
	d->num_data += numdata - 1;
	if(!data->v) data->v = particle_condition_var_new(m);
	particle_condition_var_sleep(data->v);

	lua_pop(pipes,1); // pop the pipe

	particle_mutex_unlock(m);

	lua_pop(L,numdata-1);

	return zenith_pipe_receive(L);
}

static inline void access_table(lua_State *L)
{
	lua_createtable(L,0,3);

	l_setfunctionfield(L,-1,"send",zenith_pipe_send);
	l_setfunctionfield(L,-1,"wait",zenith_pipe_send_wait);
	l_setfunctionfield(L,-1,"receive",zenith_pipe_receive);

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

	data = (PipeData *) realloc(data,sizeof(PipeData) *next_id);
	data[next_id-1].num_data = 0;
	data[next_id-1].v = 0;

	particle_mutex_unlock(m);

	set_pipe_table(L1,name);
	set_pipe_table(L2,name);

	next_id++;
}
