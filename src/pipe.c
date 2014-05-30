
#include "stdafx.h"

#include "common.h"
#include "pipe.h"

#include "state.h"

#include "transfer.h"

#include "structure/TArray.h"
#include "thread/tthread.h"

typedef struct _PipeData {
	size_t num_data;
	TCV *v;
	lua_State *waiting;
	unsigned char wants_table;
	TArray *localnumdata;
} PipeData;

typedef struct ZenithPipes {
	lua_State *pipes;
	TMutex *m;
	TArray *data;
	size_t next_id;
} ZenithPipes;

static ZenithPipes *zenithpipes = 0;

static PipeData *pipedata_new(void)
{
	PipeData *p = (PipeData *) malloc(sizeof(PipeData));

	p->num_data = 0;
	p->v = 0;
	p->waiting = 0;
	p->wants_table = 0;
	p->localnumdata = TArrayNew(0);

	return p;
}

static void pipedata_free(PipeData *p)
{
	if(p) {
		TCVFree(p->v);
		TArrayFree(p->localnumdata,free);
		free(p);
	}
}

void zenith_pipe_initialize(void)
{
	if(!zenithpipes) {
		zenithpipes = (ZenithPipes *) malloc(sizeof(ZenithPipes));
		if(!zenithpipes) return;
	}

	zenithpipes->next_id = 1;
	zenithpipes->pipes = lua_open();
	zenithpipes->m = TMutexNew(T_MUTEX_RECURSIVE);

	luaL_openlibs(zenithpipes->pipes);

	lua_newtable(zenithpipes->pipes); // table containing pipes
	l_getElement(zenithpipes->pipes,"table","remove",0); // table.remove must stay on top of the stack

	zenithpipes->data = TArrayNew(0);
}

void zenith_pipe_destroy(void)
{
	if(zenithpipes) {
		size_t i = 0;

		lua_close(zenithpipes->pipes);
		TMutexFree(zenithpipes->m);
		zenithpipes->pipes = 0;
		zenithpipes->m = 0;

		TArrayFree(zenithpipes->data,(TFreeFunc)pipedata_free);
		zenithpipes->data = 0;

		free(zenithpipes);
		zenithpipes = 0;
	}
}

static inline unsigned char pipe_get_pipe(size_t id) {
	lua_checkstack(zenithpipes->pipes,2);

	lua_pushinteger(zenithpipes->pipes,id);
	lua_gettable(zenithpipes->pipes,1);

	return lua_isnil(zenithpipes->pipes,-1);
}

static inline void pipe_send(lua_State *L, size_t id, size_t wait, int idx)
{
	size_t i, j = idx+1, numdata = lua_gettop(L);
	PipeData *d = (PipeData *) TArrayGet(zenithpipes->data,id-1);

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
			TMutexUnlock(zenithpipes->m);
			luaL_error(L,"There are no pipe with id %d",id);
			return;
		}

		i = d->num_data;

		// keep local numdata
		TArrayPush(d->localnumdata,ptr);

		for(; j <= numdata; ++j) {
			lua_pushnumber(zenithpipes->pipes,++i);
			zenith_transfer_data(L,zenithpipes->pipes,j);
			lua_settable(zenithpipes->pipes,-3);
		}

		d->num_data += *ptr;

		lua_pop(zenithpipes->pipes,1); // pop the pipe

		//printf("sent\n");
	}

	if(wait) {
		d->waiting = L;
		if(!d->v) d->v = TCVNew(zenithpipes->m);
		else TCVWake(d->v);
		TCVSleep(d->v,wait);
		if(d->waiting == L) d->waiting = 0;
	}
}

void zenith_pipe_custom_send(lua_State *L, int idx, void (*cpy)(lua_State *from, lua_State *to, int idx))
{
	if(zenithpipes) {
		//find pipe id
		size_t id = l_getintfield(L,idx,"id");
		PipeData *d = (PipeData *) TArrayGet(zenithpipes->data,id-1);

		if(lua_gettop(L) > 1) {
			TMutexLock(zenithpipes->m);
			zenith_transfer_set_table_transfer_method(cpy);
			pipe_send(L,id,0,idx);
			zenith_transfer_set_default_table_transfer_method();
			TMutexUnlock(zenithpipes->m);
		}

		if(d->v) TCVWake(d->v);

		//consume the elements
		lua_pop(L,lua_gettop(L) - idx + 1);
	}
}

int zenith_pipe_send(lua_State *L)
{
	if(zenithpipes) {
		// STACK
		// 1 - pipe obj
		// 2 - data
		// 3 - data
		// ...

		//find pipe id
		size_t id = l_getintfield(L,1,"id");
		PipeData *d = (PipeData *) TArrayGet(zenithpipes->data,id-1);

		if(lua_gettop(L) > 1) {
			TMutexLock(zenithpipes->m);
			pipe_send(L,id,0,1);
			TMutexUnlock(zenithpipes->m);
		}

		if(d->v) TCVWake(d->v);
	}

	return 0;
}

static inline size_t pipe_receive(lua_State *L, size_t id, unsigned char to_table, unsigned char receive_all)
{
	PipeData *d = (PipeData *) TArrayGet(zenithpipes->data,id-1);
	size_t numdata = d->num_data;

	if(numdata) {
		size_t j = numdata, i = 1;

		if(to_table) lua_createtable(L,0,numdata);

		//printf("receiving\n");

		//get the pipe
		if(pipe_get_pipe(id)) {
			TMutexUnlock(zenithpipes->m);
			lua_pop(L,1);
			luaL_error(L,"There are no pipe with id %d",id);
			return 0;
		}

		//push the data into the table
		numdata = 0;

		if(receive_all) {
			TArrayEmpty(d->localnumdata,free);
		} else {
			int *amount = (int *) TArrayPopIndex(d->localnumdata,0);
			j = *amount;
			free(amount);
		}

		while(j--) {
			lua_pushvalue(zenithpipes->pipes,2); //duplicate table remove function

			lua_pushvalue(zenithpipes->pipes,-2); // pipe table
			lua_pushnumber(zenithpipes->pipes,1); // id
			lua_call(zenithpipes->pipes,2,1);

			if(to_table) {
				lua_pushnumber(L,i++);
				zenith_transfer_data(zenithpipes->pipes,L,-1);
				lua_settable(L,-3);
			} else {
				zenith_transfer_data(zenithpipes->pipes,L,-1);
			}
			numdata++;

			lua_pop(zenithpipes->pipes,1);
		}

		d->num_data -= numdata;

		//printf("received\n");

		lua_pop(zenithpipes->pipes,1); // pop the pipe

		return to_table ? 1 : numdata;
	}

	return 0;
}

int zenith_pipe_receive(lua_State *L)
{
	if(zenithpipes) {
		// STACK
		// 1 pipe object
		// 2 option

		//find pipe name

		size_t id = l_getintfield(L,1,"id"), numdata;
		unsigned char opt = luaL_optint(L,2,1);

		TMutexLock(zenithpipes->m);
		numdata = pipe_receive(L,id,0,opt);
		TMutexUnlock(zenithpipes->m);

		return (int) numdata;
	}

	return 0;
}

int zenith_pipe_receive_table(lua_State *L)
{
	if(zenithpipes) {
		// STACK
		// 1 pipe object

		//find pipe name

		size_t id = l_getintfield(L,1,"id"),numdata;
		unsigned char opt = luaL_optint(L,2,1);

		TMutexLock(zenithpipes->m);
		numdata = pipe_receive(L,id,1,opt);
		TMutexUnlock(zenithpipes->m);

		return (int) numdata;
	}

	return 0;
}

int zenith_pipe_send_wait(lua_State *L)
{
	if(zenithpipes) {
		// STACK
		// 1 - pipe obj
		// 2 - timeout
		// 3 - data
		// 4 - data
		// ...

		if(lua_gettop(L) > 1) {
			//find pipe id
			size_t id = l_getintfield(L,1,"id");
			PipeData *d = (PipeData *) TArrayGet(zenithpipes->data,id-1);
			size_t timeout = luaL_checkint(L,2);
			int numdata = lua_gettop(L);

			TMutexLock(zenithpipes->m);
			d->wants_table = 0;
			pipe_send(L,id,timeout,2);
			TMutexUnlock(zenithpipes->m);

			return lua_gettop(L) - numdata;
		}
	}

	return 0;
}

int zenith_pipe_send_wait_table(lua_State *L)
{
	if(zenithpipes) {
		// STACK
		// 1 - pipe obj
		// 2 - timeout
		// 3 - data
		// 4 - data
		// ...

		size_t id = l_getintfield(L,1,"id");
		PipeData *d = (PipeData *) TArrayGet(zenithpipes->data,id-1);
		size_t timeout = luaL_checkint(L,2);

		if(lua_gettop(L) > 1) {
			//find pipe id
			int numdata = lua_gettop(L);

			TMutexLock(zenithpipes->m);
			d->wants_table = 1;
			pipe_send(L,id,timeout,2);
			TMutexUnlock(zenithpipes->m);

			return lua_gettop(L) - numdata;
		}

		if(d->v) TCVWake(d->v);
	}

	return 0;
}

int zenith_pipe_listen(lua_State *L)
{
	if(zenithpipes) {
		// STACK
		// 1 - pipe obj
		// 2 - timeout
		// 3 - option

		//find pipe id
		size_t id = l_getintfield(L,1,"id");
		PipeData *d = (PipeData *) TArrayGet(zenithpipes->data,id-1);
		size_t numdata;
		size_t timeout = luaL_checkint(L,2);
		unsigned char opt = luaL_optint(L,3,1);

		TMutexLock(zenithpipes->m);

		if(d->num_data) {
			numdata = pipe_receive(L,id,0,opt);
		} else {
			d->wants_table = 0;
			d->waiting = L;
			if(!d->v) d->v = TCVNew(zenithpipes->m);
			TCVSleep(d->v,timeout);
			if(d->waiting == L) d->waiting = 0;
			numdata = lua_gettop(L) - 1;
		}
	
		TMutexUnlock(zenithpipes->m);

		return numdata;
	}

	return 0;
}

int zenith_pipe_listen_table(lua_State *L)
{
	if(zenithpipes) {
		// STACK
		// 1 - pipe obj
		// 2 - timeout
		// 2 - data
		// 3 - data
		// ...

		size_t id = l_getintfield(L,1,"id");
		PipeData *d = (PipeData *) TArrayGet(zenithpipes->data,id-1);
		size_t numdata;
		size_t timeout = luaL_checkint(L,2);
		unsigned char opt = luaL_optint(L,2,1);

		TMutexLock(zenithpipes->m);

		if(d->num_data) {
			numdata = pipe_receive(L,id,1,opt);
		} else {
			d->wants_table = 1;
			d->waiting = L;
			if(!d->v) d->v = TCVNew(zenithpipes->m);
			TCVSleep(d->v,timeout);
			d->waiting = 0;
			numdata = lua_gettop(L) - 1;
		}
	
		TMutexUnlock(zenithpipes->m);

		return numdata;
		}
	return 0;
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
	l_setintfield(L,-1,"id",zenithpipes->next_id);

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

	if(!zenithpipes) zenith_pipe_initialize();

	if(!name) {
		size_t i1 = zenith_state_from_state(L1);
		size_t i2 = zenith_state_from_state(L2);

		snprintf(namerplcmnt,sizeof(namerplcmnt),"pipe%d%d",i1,i2);

		name = namerplcmnt;
	}

	//set pipe
	TMutexLock(zenithpipes->m);
	lua_pushinteger(zenithpipes->pipes,zenithpipes->next_id);
	lua_newtable(zenithpipes->pipes);
	lua_settable(zenithpipes->pipes,1);

	TArrayPush(zenithpipes->data,pipedata_new());
	TMutexUnlock(zenithpipes->m);

	set_pipe_table(L1,name);
	set_pipe_table(L2,name);

	zenithpipes->next_id++;
}
