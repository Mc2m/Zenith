#ifndef _included_lux_common_h
#define _included_lux_common_h

#include <luajit.h>
#include <lauxlib.h>
#include <lualib.h>

#define L_INDEX_CHECK(i) if(i < 0 && i != LUA_REGISTRYINDEX) --i
#define L_INDEX_RESTORE(i) if(i < 0 && i != LUA_REGISTRYINDEX) i++

#define L_INDEX_ABS(L, i) ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)

#define L_TABLE_OR_DIE(i) if (!lua_istable(L, i)) return luaL_typerror(L, i, "table")

typedef void(*LLVoidFunc)(lua_State *);

void LPrintVar(lua_State *L, int index);
void LPrintTable(lua_State *L, int index);
void LPrintStack(lua_State *L);

//------------------------------------------------//

static void LSetBoolField(lua_State *L, int index, const char *field, unsigned char value) {
    L_INDEX_CHECK(index);

    lua_pushboolean(L, value);
    lua_setfield(L, index, field);
}

static void LSetIntField(lua_State *L, int index, const char *field, int value) {
    L_INDEX_CHECK(index);

    lua_pushnumber(L, value);
    lua_setfield(L, index, field);
}

static void LSetRealField(lua_State *L, int index, const char *field, double value) {
	L_INDEX_CHECK(index);

	lua_pushnumber(L, value);
	lua_setfield(L, index, field);
}

static void LSetCharField(lua_State *L, int index, const char *field, const char *value) {
    L_INDEX_CHECK(index);

    lua_pushstring(L, value);
    lua_setfield(L, index, field);
}

static void LSetFunctionField(lua_State *L, int index, const char *field, lua_CFunction value) {
    L_INDEX_CHECK(index);

    lua_pushcfunction(L, value);
    lua_setfield(L, index, field);
}

static void LRawSetFunctionField(lua_State *L, int index, const char *field, lua_CFunction value) {
	lua_pushstring(L, field);
	lua_pushcfunction(L, value);
	lua_rawset(L, index);
}

static void LSetTableField(lua_State *L, int index, const char *field, LLVoidFunc value) {
    L_INDEX_CHECK(index);

    lua_newtable(L);
    if (value) value(L);
    lua_setfield(L, index, field);
}

static void LSetTableFieldS(lua_State *L, int index, const char *field, const struct luaL_reg *content) {
    L_INDEX_CHECK(index);

    lua_newtable(L);
    if (content) luaL_openlib(L, 0, content, 0);
    lua_setfield(L, index, field);
}

static void LSetValueField(lua_State *L, int index, const char *field, int valueIdx) {
	L_INDEX_CHECK(index);

	lua_pushvalue(L, valueIdx);
	lua_setfield(L, index, field);
}

static void LRawSetValueField(lua_State *L, int index, const char *field, int valueIdx) {
	lua_pushstring(L, field);
	lua_pushvalue(L, valueIdx);
	lua_rawset(L, index);
}

//------------------------------------------------//

unsigned char LGetBoolField(lua_State *L, int arg, const char *field);
int LGetIntField(lua_State *L, int arg, const char *field);
int LGetOptIntField(lua_State *L, int arg, const char *field, int def);
float LGetOptFloatField(lua_State *L, int arg, const char *field, float def);
double LGetDoubleField(lua_State *L, int arg, const char *field);
double LGetOptDoubleField(lua_State *L, int arg, const char *field, double def);
const char *LGetCharField(lua_State *L, int arg, const char *field);
const char *LGetCharFieldFull(lua_State *L, int arg, const char *field, size_t *size, const char *def);

void LSetIntIndex(lua_State *L, int arg, int index, int value);
static void LSetRealIndex(lua_State *L, int arg, int index, double value) {
	if (arg < 0 && arg != LUA_REGISTRYINDEX) arg -= 2;

	lua_pushinteger(L, index);
	lua_pushnumber(L, value);
	lua_settable(L, arg);
}

int LGetIntIndex(lua_State *L, int arg, int index);
int LGetOptIntIndex(lua_State *L, int arg, int index, int def);
float LGetFloatIndex(lua_State *L, int arg, int index);
float LGetOptFloatIndex(lua_State *L, int arg, int index, float def);
double LGetOptDoubleIndex(lua_State *L, int arg, int index, double def);

int LCheckBool(lua_State *L, int idx);

static void LOpenLib(lua_State *L, const struct luaL_reg *functions) {
	if (lua_istable(L, -1)) luaL_openlib(L, 0, functions, 0);
}

void LOpenLibCreate(lua_State *L, const struct luaL_reg *functions);
void LOpenLibOptCreate(lua_State *L, const struct luaL_reg *functions);
void LOpenLibIndex(lua_State *L, int idx, const struct luaL_reg *functions);

static void LSetMetaTable(lua_State *L, const char *field, const struct luaL_reg *functions) {
	lua_newtable(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index"); /* table.__index = table */
	LOpenLib(L, functions);
	lua_setfield(L, LUA_REGISTRYINDEX, field);
}

static void LGetRegistyElement(lua_State *L, void *root, ...) {
	va_list vl;
	const char *elem;

	if (!root) return;

	lua_pushlightuserdata(L, root);
	lua_gettable(L, LUA_REGISTRYINDEX);

	va_start(vl, root);
	while ((elem = va_arg(vl, const char *))) {
		if (lua_isnil(L, -1)) return;
		lua_getfield(L, -1, elem);
		lua_remove(L, -2);
	}
	va_end(vl);
}
void LGetElement(lua_State *L, const char *first_element, ...);

void LSetReportFunc(LLVoidFunc reportfunc);
extern void(*LReport)(lua_State *L);

int LCall(lua_State *L, int narg, int numresults);
int LLoad(lua_State *L, const char *filename);
int LParse(lua_State *L, const char *string);
int LParseB(lua_State *L, const unsigned char *buffer, size_t size);

void LSetPkgPath(lua_State *L, const char *path);
void LAppendPkgPath(lua_State *L, const char *path);
void LRemoveLastPkgPath(lua_State *L);

#endif
