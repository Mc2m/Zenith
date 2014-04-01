
#include "stdafx.h"

#include "zenith.h"

#include "table.h"

void load_zenith_table(lua_State *L)
{
	lua_checkstack(L,1);

	lua_getglobal(L,"Zenith");
	if(lua_isnil(L,-1)) {
		lua_pop(L,1);
		lua_newtable(L);

		lua_checkstack(L,2);
		lua_pushvalue(L,-1);
		lua_setglobal(L,"Zenith");
	}
}

void zenith_lib_table(lua_State *L)
{
	load_zenith_table(L);

	l_settablefield(L, -1, "Table", register_zenith_table);

	lua_pop(L,1);
}

void zenith_lib_state(lua_State *L)
{
	load_zenith_table(L);

	l_settablefield(L, -1, "State", register_zenith_state_table);

	lua_pop(L,1);
}
