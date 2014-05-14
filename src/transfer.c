
#include "stdafx.h"

#include "common.h"
#include "transfer.h"

#include "debugging/tdebug.h"

static int function_cpy(lua_State *L,const char *p,size_t size,luaL_Buffer *B)
{
	luaL_addlstring(B,p,size);
	return 0;
}

void copy_table(lua_State *from, lua_State *to, int idx)
{
	lua_checkstack(from,2);
	lua_checkstack(to,3);

	if(idx < 0) idx--;


	lua_newtable(to);

	lua_pushnil(from);

	while (lua_next(from,idx)) {
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		zenith_transfer_data(from,to,-2);
		zenith_transfer_data(from,to,-1);

		lua_settable(to,-3);

		lua_pop(from, 1);
	}

	lua_getmetatable(from,idx);
	if(! lua_isnil(from,-1)) {
		copy_table(from,to,-1);

		lua_setmetatable(to,-2);
	}
	lua_pop(from,1);
}

static void (*tbl_cpy)(lua_State *from, lua_State *to, int idx) = copy_table;

void zenith_transfer_set_table_transfer_method(void (*cpy)(lua_State *from, lua_State *to, int idx))
{
	tbl_cpy = cpy;
}

void zenith_transfer_set_default_table_transfer_method()
{
	tbl_cpy = copy_table;
}

void copy_function(lua_State *from, lua_State *to, int idx)
{
	if(lua_iscfunction(from,idx)) {
		lua_pushcfunction(to,lua_tocfunction(from,idx));
	} else {
		unsigned char pushfunc = idx != lua_gettop(from);
		luaL_Buffer B;
		const char *s;
		size_t len;

		luaL_buffinit(from,&B);

		if(pushfunc) lua_pushvalue(from,idx);

		if(lua_dump(from,(lua_Writer) function_cpy,&B))
		{
			luaL_error(from, "internal error: function dump failed.");
		}

		luaL_pushresult(&B);

		if(pushfunc) lua_remove(from,-2);

		s = lua_tolstring(from,-1,&len);
		if(luaL_loadbuffer(to, s, len, 0)) {
			luaL_error(from, "internal error: function load failed.");
		}

		lua_pop(from,1);
	}
}

void zenith_transfer_data(lua_State *from, lua_State *to, int idx)
{
	int type = lua_type(from,idx);

	TAssert(lua_checkstack(to,1));

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
		if(tbl_cpy) tbl_cpy(from,to,idx);
		else lua_pushnil(to);
	} else if(type == LUA_TFUNCTION) {
		copy_function(from,to,idx);
	} else if(type == LUA_TUSERDATA) {
		//TODO probably
	}
}

void zenith_transfer_range(lua_State *from, lua_State *to, int start, int end)
{
	int i = start;

	if(start <= end) {
		for(; i < end; ++i) {
			zenith_transfer_data(from,to,i);
		}
	} else {
		for(; i > end; i--) {
			zenith_transfer_data(from,to,i);
		}
	}
}
