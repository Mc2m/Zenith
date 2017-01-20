
#include "stdafx.h"

#include "transfer.h"

#include "debugging/tdebug.h"

#include "tdefine.h"

static int reftable[2] = {0};

static void ZTransferDataInternal(lua_State *from, lua_State *to, int idx, int type);

static int ZTransferFindMatch(lua_State *L, int reftable, int idx) {
	int found = 0;

	if (reftable) {
		lua_pushvalue(L, idx);
		lua_rawget(L, reftable);
		found = lua_tointeger(L, -1);
		lua_pop(L, 1);
	}

	return found;
}

static int ZTransferAddReference(lua_State *L, int idx, int objidx, int index) {
	if (!index) {
		int i;
		//get the index
		lua_rawgeti(L, idx, 1);
		i = lua_tointeger(L, -1);

		//set the next index
		lua_pushinteger(L, i + 1);
		lua_rawseti(L, idx, 1);

		//store the reference
		lua_pushvalue(L, objidx);
		lua_pushinteger(L, i);
		lua_rawset(L, idx);

		lua_pop(L, 1);

		return i;
	}

	//store the reference
	lua_pushvalue(L, objidx);
	lua_rawseti(L, idx, index);
	return 0;
}

static void ZTransferTable(lua_State *from, lua_State *to, int idx) {
	int match;

	if (!lua_checkstack(from, 2)) return;
	if (!lua_checkstack(to, 2)) return;

	idx = L_INDEX_ABS(from, idx);

	match = ZTransferFindMatch(from, reftable[0], idx);
	if (match) {
		lua_rawgeti(to, reftable[1], match);
		return;
	}

	lua_newtable(to);

	if (reftable[0]) {
		int index = ZTransferAddReference(from, reftable[0], idx, 0);
		ZTransferAddReference(to, reftable[1], lua_gettop(to), index);
	}

	lua_pushnil(from);
	while (lua_next(from, idx)) {
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		ZTransferDataInternal(from, to, -2, lua_type(from, -2));
		ZTransferDataInternal(from, to, -1, lua_type(from, -1));

		lua_rawset(to, -3);

		lua_pop(from, 1);
	}

	if (lua_getmetatable(from, idx)) {
		ZTransferDataInternal(from, to, -1, lua_type(from, -1));

		lua_setmetatable(to, -2);
		lua_pop(from, 1);
	}
}

static int ZTransferFunctionInternal(lua_State *L, const char *p, size_t size, luaL_Buffer *B) {
	luaL_addlstring(B, p, size);
	return 0;
}

static void ZTransferFunction(lua_State *from, lua_State *to, int idx) {
	if (lua_iscfunction(from, idx)) {
		lua_pushcfunction(to, lua_tocfunction(from, idx));
	} else {
		unsigned char pushfunc;
		luaL_Buffer B;
		const char *s;
		size_t len;
		int match;

		idx = L_INDEX_ABS(from, idx);

		match = ZTransferFindMatch(from, reftable[0], idx);
		if (match) {
			lua_rawgeti(to, reftable[1], match);
			return;
		}

		pushfunc = idx != lua_gettop(from);
		luaL_buffinit(from, &B);

		if (pushfunc) lua_pushvalue(from, idx);

		if (lua_dump(from, (lua_Writer)ZTransferFunctionInternal, &B)) {
			luaL_error(from, "internal error: function dump failed.");
		}

		luaL_pushresult(&B);

		if (pushfunc) lua_remove(from, -2);

		s = lua_tolstring(from, -1, &len);
		if (luaL_loadbuffer(to, s, len, 0)) {
			luaL_error(from, "internal error: function load failed.");
		}

		lua_pop(from, 1);

		if (reftable[0]) {
			int index = ZTransferAddReference(from, reftable[0], idx, 0);
			ZTransferAddReference(to, reftable[1], lua_gettop(to), index);
		}
	}
}

static void ZTransferDataInternal(lua_State *from, lua_State *to, int idx, int type) {
	if (type == LUA_TBOOLEAN)            lua_pushboolean(to, lua_toboolean(from, idx));
	else if (type == LUA_TLIGHTUSERDATA) lua_pushlightuserdata(to, lua_touserdata(from, idx));
	else if (type == LUA_TNUMBER)        lua_pushnumber(to, lua_tonumber(from, idx));
	else if (type == LUA_TSTRING)        lua_pushstring(to, lua_tostring(from, idx));
	else if (type == LUA_TTABLE)         ZTransferTable(from, to, idx);
	else if (type == LUA_TFUNCTION)      ZTransferFunction(from, to, idx);
	else                                lua_pushnil(to);
}

static inline void ZTransferSetTransferTable(lua_State *L, int source) {
	if (source) {
		//tracking table
		lua_createtable(L, 1, 1);

		//table tracking id
		lua_pushnumber(L, 1);
		lua_rawseti(L, -2, 1);
	} else {
		//tracking table
		lua_createtable(L, 1, 0);
	}
}

void ZTransferData(lua_State *from, lua_State *to, int idx) {
	int type;

	if (!from || !to) return;
	if (!lua_checkstack(to, 1)) return;

	type = lua_type(from, idx);
	if (type == LUA_TNONE) return;

	idx = L_INDEX_ABS(from, idx);

	// We need to keep track of the elements in a separate table for complex elements
	// in order to transfer loops (e.g. table1 { table2 { table1 }})

	if (type == LUA_TTABLE) {
		if (!lua_checkstack(to, 1)) return;
		if (!lua_checkstack(to, 2)) return;
		ZTransferSetTransferTable(from, 1);
		ZTransferSetTransferTable(to, 0);
		reftable[0] = lua_gettop(from);
		reftable[1] = lua_gettop(to);
	}

	ZTransferDataInternal(from, to, idx, type);

	if (type == LUA_TTABLE) {
		lua_remove(from, reftable[0]);
		lua_remove(to, reftable[1]);
		reftable[0] = reftable[1] = 0;
	}
}

void ZTransferRange(lua_State *from, lua_State *to, int start, int range) {
	int end;
	int top;

	if (!from || !to) return;

	top = lua_gettop(from);
	end = start + (range - 1);
	if (top < end) end = (top - start) + 1;


	for (; start <= end; ++start) ZTransferData(from, to, start);
}
