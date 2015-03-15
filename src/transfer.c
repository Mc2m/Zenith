
#include "stdafx.h"

#include "transfer.h"

#include "debugging/tdebug.h"

#include "tdefine.h"

static int ZTransferFunctionInternal(lua_State *L,const char *p,size_t size,luaL_Buffer *B)
{
	luaL_addlstring(B,p,size);
	return 0;
}

void ZTransferTable(lua_State *from, lua_State *to, int idx)
{
	if(!lua_checkstack(from,2)) return;
	if(!lua_checkstack(to,2)) return;

	if(idx < 0) idx--;

	lua_newtable(to);

	lua_pushnil(from);

	while (lua_next(from,idx)) {
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		ZTransferData(from,to,-2);
		ZTransferData(from,to,-1);

		lua_settable(to,-3);

		lua_pop(from, 1);
	}

	if(idx < 0) idx++;

	if(lua_getmetatable(from,idx)) {
		ZTransferTable(from,to,-1);

		lua_setmetatable(to,-2);
		lua_pop(from,1);
	}
}

void ZTranferFunction(lua_State *from, lua_State *to, int idx)
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

		if(lua_dump(from,(lua_Writer) ZTransferFunctionInternal,&B))
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

void ZTransferData(lua_State *from, lua_State *to, int idx)
{
	int type;

	if(!from || !to) return;
	if(!lua_checkstack(to,1)) return;

	type = lua_type(from,idx);
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
		ZTransferTable(from,to,idx);
	} else if(type == LUA_TFUNCTION) {
		ZTranferFunction(from,to,idx);
	} else if(type == LUA_TUSERDATA) {
		//TODO probably
	}
}

void ZTransferRange(lua_State *from, lua_State *to, int start, int end)
{
	int i;

	if(!from || !to) return;
	if(start > end) TSWAPT(start,end,int);

	for(i = start; i < end; ++i) ZTransferData(from,to,i);
}
