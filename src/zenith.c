
#include "stdafx.h"

#include "zenith.h"

static const char metaKey = 'k';

void ZenithInitialize(int flags) {
	if (flags & ZENITH_LIBRARY_PIPE)
		ZPipeInitialize((void *)&metaKey);

	if (flags & ZENITH_LIBRARY_STATE)
		LSetReportFunc(ZStateReport);
}

void ZenithDestroy(void) {
	ZPipeDestroy();
	LSetReportFunc(0);
}

void ZenithOpenLibrary(lua_State *L, int flags) {
	//setup Zenith library

	//create internal and public tables
	lua_newtable(L);
	lua_newtable(L);

	if (flags & ZENITH_LIBRARY_STATE) {
		//add common state table
		lua_newtable(L);
		lua_pushvalue(L, -1);

		lua_setfield(L, -3, "states");
		lua_setfield(L, -3, "states");

		//register public state functions
		ZStateRegister(L);
	}

	if (flags & ZENITH_LIBRARY_PIPE) {
		//add common pipe table
		lua_newtable(L);
		lua_pushvalue(L, -1);

		lua_setfield(L, -3, "pipes");
		lua_setfield(L, -3, "pipes");
	}

	//set public table in global table
	lua_pushvalue(L, -1);
	lua_setglobal(L, "zenith");

	//set table in loaded
	lua_getfield(L, LUA_REGISTRYINDEX, "_LOADED");
	lua_pushvalue(L, -2);
	lua_setfield(L, -2, "zenith");

	lua_pop(L, 2);

	//store table in registy
	lua_pushlightuserdata(L, (void *)&metaKey);
	lua_pushvalue(L, -2);
	lua_settable(L, LUA_REGISTRYINDEX);

	lua_pop(L, 1);
}
