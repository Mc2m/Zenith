
#include "stdafx.h"

#include "state.h"

void ZStateReport(lua_State *L)
{
	if (!lua_isnil(L, -1)) {
		const char *msg = lua_tostring(L, -1);
		const char *name;
		size_t namelen;

		name = LGetCharFieldFull(L,LUA_REGISTRYINDEX,"Zenith_state_name",&namelen,0);

		if(!msg) msg = "(error object is not a string)";

		printf("State %s: %s\n",name, msg);
		lua_pop(L, 1);
	}
}

lua_State *ZStateNew(const char *name)
{
	lua_State *L = luaL_newstate();
	
	if(name) LSetCharField(L,LUA_REGISTRYINDEX, "Zenith_state_name", name);

	return L;
}

void ZStateClose(lua_State *L)
{
	lua_close(L);
}

static int ZStateluaNew(lua_State *L)
{
	const char *name = luaL_optstring(L,1,0);

	lua_pushlightuserdata(L, ZStateNew(name));

	return 1;
}

static int ZStateluaClose(lua_State *L)
{
	if (lua_islightuserdata(L,1)) {
		lua_State *state = (lua_State *) lua_touserdata(L,1);

		ZStateClose(state);
	}

	return 0;
}

static const struct luaL_Reg ZStateFunctions[] = {
	{"open", ZStateluaNew},
	{"close", ZStateluaClose},
	{NULL, NULL}
};

void ZStateRegister(lua_State *L)
{
	LOpenLib(L, ZStateFunctions);
}
