
#include "stdafx.h"

#include "common.h"
#include "pipe.h"

#include "state.h"

#include "transfer.h"

#include "particle/particle.h"

lua_State *pipes = 0;
ParticleMutex *m = 0;

void zenith_pipe_initialize(void)
{
	pipes = lua_open();
	m = particle_mutex_new(PARTICLE_MUTEX_TYPE_RECURSIVE);

	luaL_openlibs(pipes);
	l_getElement(pipes,"table","remove",0); // table.remove must stay on top of the stack
}

void zenith_pipe_destroy(void)
{
	lua_close(pipes);
	particle_mutex_free(m);
	pipes = 0;
	m = 0;
}

inline void pipe_get_pipe(const char *name) {
	lua_checkstack(pipes,2);
	lua_getglobal(pipes,name);
	if(lua_isnil(pipes,-1)) {
		lua_pop(pipes,1);
		lua_newtable(pipes);
		lua_pushvalue(pipes,-1);
		lua_setglobal(pipes,name);
	}
}

static int pipe_send(lua_State *L)
{
	// STACK
	// 1 - pipe obj
	// 2 - data
	// 3 - data
	// ...

	
	size_t i, j = 2, numdata = lua_gettop(L);
	const char *name;
	
	if(numdata < 2) return 0;

	//find pipe name
	name = l_getcharfield(L,1,"name");
	if(name == 0) return 0;

	particle_mutex_lock(m);

	pipe_get_pipe(name); //get current pipe
	if(lua_isnil(L,-1)) {
		particle_mutex_unlock(m);
		luaL_error(L,"There are no pipe named %s",name);
		return 0;
	}

	i = lua_objlen(pipes,-1);

	for(j; j <= numdata; ++j) {
		lua_pushnumber(pipes,++i);
		zenith_transfer_data(L,pipes,j);
		lua_settable(pipes,-3);
	}

	particle_mutex_unlock(m);

	return 0;
}

static int pipe_receive(lua_State *L)
{
	// STACK
	// -1 pipe object

	//find pipe name

	const char *name = l_getcharfield(L,-1,"name");
	if(name == 0) return 0;

	particle_mutex_lock(m);

	//duplicate table remove function
	lua_pushvalue(pipes,1);

	pipe_get_pipe(name); //first argument
	lua_pushnumber(pipes,1); // second argument
	lua_call(pipes,2,1);
	zenith_transfer_data(pipes,L,-1);
	lua_pop(pipes,1);
	particle_mutex_unlock(m);

	return 1;
}

inline void access_table(lua_State *L)
{
	lua_createtable(L,0,3);

	l_setfunctionfield(L,-1,"send",pipe_send);
	l_setfunctionfield(L,-1,"receive",pipe_receive);

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
		lua_getfield(L,-1,"pipes");
	}

	// STACK
	// -1 pipes table
	// -2 access table
	// -3 Pipe table
	// -4 Zenith table
	
	lua_createtable(L,0,1);
	l_setcharfield(L,-1,"name",name);

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
	lua_newtable(pipes);
	lua_setglobal(pipes,name);
	particle_mutex_unlock(m);

	set_pipe_table(L1,name);
	set_pipe_table(L2,name);
}
