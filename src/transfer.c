
#include "stdafx.h"

#include "transfer.h"

#include "debugging/tdebug.h"

#include "tdefine.h"

static int reftable[2] = {0};

static void ZTransferDataInternal(lua_State *from, lua_State *to, int idx, int type);

static int ZTransferFindMatch(lua_State *L, int reftable, const char *field, int idx)
{
	if(reftable) {
		int len, i = 1;
		const void *ptr = lua_topointer(L,idx);
		//maybe table exists
		lua_getfield(L,reftable,field);

		len = lua_objlen(L, -1);
		for(; i <= len; ++i) {
			lua_pushnumber(L,i);
			lua_gettable(L,-2);
			if(lua_istable(L,-1)) {
				if(ptr == lua_topointer(L,-1)) {
					lua_pop(L,2);
					return i;
				}
			}
			lua_pop(L,1);
		}

		lua_pop(L,1);
	}

	return 0;
}

static void ZTransferAddReference(lua_State *L, int idx, const char *field, int objidx)
{
	lua_getfield(L,idx,field);
	lua_pushinteger(L,lua_objlen(L,-1)+1);
	lua_pushvalue(L,objidx);
	lua_settable(L, -3);
	lua_pop(L, 1);
}

static void ZTransferTable(lua_State *from, lua_State *to, int idx)
{
	int match;

	if(!lua_checkstack(from,2)) return;
	if(!lua_checkstack(to,2)) return;

	if(idx < 0) idx += lua_gettop(from) + 1;

	match = ZTransferFindMatch(from,reftable[0],"tables",idx);
	if(match) {
		lua_getfield(to,reftable[1],"tables");
		lua_pushinteger(to,match);
		lua_gettable(to, -2);
		lua_remove(to, -2);
		return;
	}

	lua_newtable(to);

	if(reftable[0]) {
		ZTransferAddReference(from,reftable[0],"tables",idx);
		ZTransferAddReference(to,reftable[1],"tables",lua_gettop(to));
	}

	lua_pushnil(from);

	while (lua_next(from,idx)) {
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		ZTransferDataInternal(from,to,-2, lua_type(from, -2));
		ZTransferDataInternal(from,to,-1, lua_type(from, -1));

		lua_settable(to,-3);

		lua_pop(from, 1);
	}

	if(lua_getmetatable(from,idx)) {
		ZTransferDataInternal(from,to,-1, lua_type(from, -1));

		lua_setmetatable(to,-2);
		lua_pop(from,1);
	}
}

static int ZTransferFunctionInternal(lua_State *L,const char *p,size_t size,luaL_Buffer *B)
{
	luaL_addlstring(B,p,size);
	return 0;
}

static void ZTransferFunction(lua_State *from, lua_State *to, int idx)
{
	if(lua_iscfunction(from,idx)) {
		lua_pushcfunction(to,lua_tocfunction(from,idx));
	} else {
		unsigned char pushfunc;
		luaL_Buffer B;
		const char *s;
		size_t len;
		int match;

		if(idx <= 0) idx += lua_gettop(from) + 1;

		match = ZTransferFindMatch(from,reftable[0],"functions",idx);
		if(match) {
			lua_getfield(to,reftable[1],"functions");
			lua_pushinteger(to,match);
			lua_gettable(to, -2);
			lua_remove(to, -2);
			return;
		}

		pushfunc = idx != lua_gettop(from);
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

		if(reftable[0]) {
			ZTransferAddReference(from,reftable[0],"functions",idx);
			ZTransferAddReference(to,reftable[1],"functions",lua_gettop(to));
		}
	}
}

static void ZTransferDataInternal(lua_State *from, lua_State *to, int idx, int type)
{
	if(type == LUA_TBOOLEAN)            lua_pushboolean(to,lua_toboolean(from,idx));
	else if(type == LUA_TLIGHTUSERDATA) lua_pushlightuserdata(to,lua_touserdata(from,idx));
	else if(type == LUA_TNUMBER)        lua_pushnumber(to,lua_tonumber(from,idx));
	else if(type == LUA_TSTRING)        lua_pushstring(to,lua_tostring(from,idx));
	else if(type == LUA_TTABLE)         ZTransferTable(from,to,idx);
	else if(type == LUA_TFUNCTION)      ZTransferFunction(from,to,idx);
	else                                lua_pushnil(to);
}

static inline void ZTransferSetTransferTable(lua_State *L)
{
	lua_newtable(L);

	lua_newtable(L);
	lua_setfield(L, -2, "tables");

	lua_newtable(L);
	lua_setfield(L, -2, "functions");
}

void ZTransferData(lua_State *from, lua_State *to, int idx)
{
	int type;

	if(!from || !to) return;
	if(!lua_checkstack(to,1)) return;

	type = lua_type(from,idx);
	if(type == LUA_TNONE) return;

	//We need to keep track of the elements in a separate table for complex elements
	if(type == LUA_TTABLE) {
		if(!lua_checkstack(to,1)) return;
		if(!lua_checkstack(to,2)) return;
		ZTransferSetTransferTable(from);
		ZTransferSetTransferTable(to);
		reftable[0] = lua_gettop(from);
		reftable[1] = lua_gettop(to);
	}

	ZTransferDataInternal(from,to,idx,type);

	if(type == LUA_TTABLE) {
		lua_remove(from,reftable[0]);
		lua_remove(to,reftable[1]);
		reftable[0] = reftable[1] = 0;
	}
}

void ZTransferRange(lua_State *from, lua_State *to, int start, int end)
{
	int i;

	if(!from || !to) return;
	if(start > end) TSWAPT(start,end,int);

	for(i = start; i < end; ++i) ZTransferData(from, to, i);
}
