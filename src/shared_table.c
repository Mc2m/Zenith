
#include "stdafx.h"

#include "common.h"
#include "shared_table.h"

/*void *(*m_new)(void)     = 0;
void (*m_free)(void *)   = 0;
void (*m_lock)(void *)   = 0;
void (*m_unlock)(void *) = 0;

void set_st_mutex_functions(void *(*mutex_new)(void),void (*mutex_free)(void *),void (*mutex_lock)(void *),void (*mutex_unlock)(void *))
{
	m_new = mutex_new;
	m_free = mutex_free;
	m_lock = mutex_lock;
	m_unlock = mutex_unlock;
}

//----------------------------------------------//

#define SHARED_INF_FIELD "shared_information"
#define SHARED_INF_META "shared_information_meta"

typedef struct _Lua_Shared_Information {
	char *access;
	size_t state_idx;
	int last_modification;
	unsigned char deleted;
} Lua_Shared_Information;

void shared_information_init(Lua_Shared_Information *lsi, int index)
{
	lsi->last_modification = index;
	lsi->state_idx = 0;
	lsi->access = 0;
	lsi->deleted = 0;
}

Lua_Shared_Information *l_tosharedinf(lua_State *L, int idx)
{
	Lua_Shared_Information *lsi = 0;

	luaL_checktype(L, idx, LUA_TUSERDATA);
	lsi = (Lua_Shared_Information *)luaL_checkudata(L, idx, SHARED_INF_META);
	if (lsi == 0) luaL_typerror(L, idx, SHARED_INF_META);

	return lsi;
}

int shared_information_gc(lua_State *L)
{
	Lua_Shared_Information *lsi = l_tosharedinf(L,-1);

	free(lsi->access);

	return 0;
}

//----------------------------------------------//

#define SHARED_MUT_FIELD "sh_mut"
#define SHARED_MUT_META "shared_mut_meta"

typedef struct _Lua_Shared_Mutex {
	void *mut;
} Lua_Shared_Mutex;

void shared_mut_init(Lua_Shared_Mutex *lsm, int index)
{
	lsm->mut = m_new();
}

Lua_Shared_Mutex *l_tosharedmut(lua_State *L, int idx)
{
	Lua_Shared_Mutex *lsm = 0;

	luaL_checktype(L, idx, LUA_TUSERDATA);
	lsm = (Lua_Shared_Mutex *)luaL_checkudata(L, idx, SHARED_MUT_META);
	if (lsm == 0) luaL_typerror(L, idx, SHARED_MUT_META);

	return lsm;
}

int shared_mut_gc(lua_State *L)
{
	Lua_Shared_Mutex *lsm = l_tosharedmut(L,-1);
	m_free(lsm->mut);

	return 0;
}

//----------------------------------------------//

int shared_table_set(lua_State *L)
{
	if(lua_istable(L,-1)) {
		Lua_Shared_Information *lsi = (Lua_Shared_Information *) lua_newuserdata(L,sizeof(Lua_Shared_Information));
		lsi->state_idx = 1;
		luaL_getmetatable(L,SHARED_INF_META);
		lua_setmetatable(L,-2);
		lua_setfield(L, -2, SHARED_INF_FIELD);
	}

	return 0;
}

int shared_table_set_mutex(lua_State *L)
{
	if(lua_istable(L,-1)) {
		Lua_Shared_Mutex *lsm = (Lua_Shared_Mutex *) lua_newuserdata(L,sizeof(Lua_Shared_Mutex));
		luaL_getmetatable(L,SHARED_MUT_META);
		lua_setmetatable(L,-2);
		lua_setfield(L, -2, SHARED_MUT_FIELD);
	}

	return 0;
}

Lua_Shared_Mutex *shared_table_get_mutex(lua_State *L)
{
	Lua_Shared_Mutex *lsm = 0;
	lua_getfield(L,-1,SHARED_MUT_FIELD);
	if(!lua_isnil(L, -1)) lsm = l_tosharedmut(L,-1);
	lua_pop(L,-1);

	return lsm;
}

int shared_table_lock(lua_State *L)
{
	Lua_Shared_Mutex *lsm = shared_table_get_mutex(L);
	lua_getfield(L,-1,SHARED_MUT_FIELD);
	lsm = l_tosharedmut(L,-1);
	lua_pop(L,-1);
	m_lock(lsm->mut);

	return 0;
}

int shared_table_update(lua_State *L)
{
	return 0;
}

int shared_table_release(lua_State *L)
{
	Lua_Shared_Mutex *lsm;
	lua_getfield(L,-1,SHARED_MUT_FIELD);
	lsm = l_tosharedmut(L,-1);

	lua_pop(L,-1);

	shared_table_update(L);

	m_unlock(lsm->mut);

	return 0;
}

int shared_table_sync(lua_State *L)
{
	shared_table_lock(L);

	return 0;
}

int shared_table_get(lua_State *L)
{
	return 0;
}

int shared_table_set(lua_State *L)
{
	return 0;
}

int shared_table_help(lua_State *L)
{
	return 0;
}

//----------------------------------------------//

static const struct luaL_reg shared_table_funcs [] = {
	{"set_mutex", shared_table_set_mutex},
	{"help", shared_table_help},
	{0, 0}
};

static const struct luaL_reg shared_mut_meta_funcs [] = {
	{"__gc", shared_information_gc},
	{0, 0}
};

static const struct luaL_reg shared_inf_meta_funcs [] = {
	{"__gc", shared_mut_gc},
	{0, 0}
};

void register_zenith_shared_table(lua_State *L)
{
	luaL_openlib(L, 0, shared_table_funcs, 0);

	luaL_newmetatable(L,SHARED_INF_META); // image metatable

	lua_pushvalue(L, -1);
	lua_setfield(L,-2,"__index"); // metatable.__index = metatable

	luaL_openlib(L, 0, shared_inf_meta_funcs, 0);

	luaL_newmetatable(L,SHARED_MUT_META); // image metatable
	lua_pushvalue(L, -1);
	lua_setfield(L,-2,"__index"); // metatable.__index = metatable

	luaL_openlib(L, 0, shared_mut_meta_funcs, 0);
}*/
