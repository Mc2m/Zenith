
#include "stdafx.h"

#include "common.h"
#include "transfer.h"

void transfer_data(lua_State *from, lua_State *to, int idx)
{
	int type = lua_type(from,idx);

	assert(lua_checkstack(to,1));

	if(type == LUA_TNIL) {
		lua_pushnil(to);
	} else if(type == LUA_TBOOLEAN) {
		lua_pushboolean(to,lua_toboolean(from,idx));
	} else if(type == LUA_TLIGHTUSERDATA) {
		lua_pushlightuserdata(to,lua_touserdata(from,idx));
	} else if(type == LUA_TNUMBER) {
		lua_pushnumber(to,lua_tonumber(from,idx));
	} else if(type == LUA_TSTRING) {
		lua_pushstring(to,lua_tostring(from,idx));
	} else if(type == LUA_TTABLE) {
		lua_pushstring(to,lua_tostring(from,idx));
	} else if(type == LUA_TFUNCTION) {
		lua_pushstring(to,lua_tostring(from,idx));
	} else if(type == LUA_TUSERDATA) {
		lua_pushstring(to,lua_tostring(from,idx));
	}
}

void transfer_range(lua_State *from, lua_State *to, int start, int end)
{

}
