#include "stdafx.h"

#include "common.h"

#include <string.h>

void LPrintVar(lua_State *L, int index) {
	int type = lua_type(L, index);
	if (type == LUA_TSTRING)       printf("%s", lua_tostring(L, index));
	else if (type == LUA_TNUMBER)  printf("%lf", lua_tonumber(L, index));
	else if (type == LUA_TNIL)     printf("nil");
	else if (type == LUA_TBOOLEAN) printf(lua_toboolean(L, index) ? "true" : "false");
	else                           printf("0x%p(%s)", lua_topointer(L, index), lua_typename(L, type));
}

void LPrintTable(lua_State *L, int index) {
	if (!lua_istable(L, index)) {
		printf("Warning: variable at index %d of the stack is a %s. Aborting display...\n", index, lua_typename(L, lua_type(L, index)));
		return;
	}

	printf("=========    TABLE    =========\n");
	printf("pointer ref : 0x%p\n", lua_topointer(L, index));

	printf("\nmetatable : ");

	if (lua_getmetatable(L, index)) {
		printf("0x%p(%s)\n", lua_topointer(L, index), lua_typename(L, lua_type(L, -1)));
		lua_pop(L, 1);
	} else {
		printf("nil\n");
	}

	L_INDEX_CHECK(index);

	printf("\ncontent :\n  ");

	lua_pushnil(L);

	while (lua_next(L, index)) {
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		LPrintVar(L, -2);
		printf(" - ");
		LPrintVar(L, -1);
		printf("\n  ");

		lua_pop(L, 1);
	}

	printf("\n===============================\n");
}

void LPrintStack(lua_State *L) {
	int arg_count = lua_gettop(L);
	int i;

	printf("===============================\n");
	printf("=          LUA STACK          =\n");
	printf("===============================\n");

	for (i = arg_count; i > 0; --i) {
		printf("\n %d : \t", i);
		LPrintVar(L, i);
	}

	printf("\n===============================\n");
}

//------------------------------------------------//

unsigned char LGetBoolField(lua_State *L, int arg, const char *field) {
	unsigned char val = 0;
	lua_getfield(L, arg, field);
	luaL_checktype(L, -1, LUA_TBOOLEAN);
	val = (unsigned char)lua_toboolean(L, -1);
	lua_pop(L, 1);
	return val;
}

int LGetIntField(lua_State *L, int arg, const char *field) {
	int val;
	lua_getfield(L, arg, field);
	val = luaL_checkint(L, -1);
	lua_pop(L, 1);
	return val;
}

int LGetOptIntField(lua_State *L, int arg, const char *field, int def) {
	int val;
	lua_getfield(L, arg, field);
	val = luaL_optint(L, -1, def);
	lua_pop(L, 1);
	return val;
}

float LGetOptFloatField(lua_State *L, int arg, const char *field, float def) {
	float val;
	lua_getfield(L, arg, field);
	val = (float)luaL_optnumber(L, -1, def);
	lua_pop(L, 1);
	return val;
}

double LGetDoubleField(lua_State *L, int arg, const char *field) {
	double val;
	lua_getfield(L, arg, field);
	val = luaL_checknumber(L, -1);
	lua_pop(L, 1);
	return val;
}

double LGetOptDoubleField(lua_State *L, int arg, const char *field, double def) {
	double val;
	lua_getfield(L, arg, field);
	val = luaL_optnumber(L, -1, def);
	lua_pop(L, 1);
	return val;
}

const char *LGetCharField(lua_State *L, int arg, const char *field) {
	const char *val;
	lua_getfield(L, arg, field);
	val = luaL_checkstring(L, -1);
	lua_pop(L, 1);
	return val;
}

const char *LGetCharFieldFull(lua_State *L, int arg, const char *field, size_t *size, const char *def) {
	const char *val;
	lua_getfield(L, arg, field);
	val = luaL_optlstring(L, -1, def, size);
	lua_pop(L, 1);
	return val;
}

//------------------------------------------------//

void LSetIntIndex(lua_State *L, int arg, int index, int value) {
	if (arg < 0 && arg != LUA_REGISTRYINDEX) arg -= 2;
	lua_pushinteger(L, index);
	lua_pushinteger(L, value);
	lua_settable(L, arg);
}

//------------------------------------------------//

int LGetIntIndex(lua_State *L, int arg, int index) {
	int val;

	L_INDEX_CHECK(arg);
	lua_pushnumber(L, index);
	lua_gettable(L, arg);
	val = luaL_checkint(L, -1);
	lua_pop(L, 1);

	return val;
}

int LGetOptIntIndex(lua_State *L, int arg, int index, int def) {
	int val;

	L_INDEX_CHECK(arg);
	lua_pushnumber(L, index);
	lua_gettable(L, arg);
	val = luaL_optint(L, -1, def);
	lua_pop(L, 1);

	return val;
}

float LGetFloatIndex(lua_State *L, int arg, int index) {
	float val;

	L_INDEX_CHECK(arg);
	lua_pushnumber(L, index);
	lua_gettable(L, arg);
	val = (float)luaL_checknumber(L, -1);
	lua_pop(L, 1);

	return val;
}

float LGetOptFloatIndex(lua_State *L, int arg, int index, float def) {
	float val;

	L_INDEX_CHECK(arg);
	lua_pushnumber(L, index);
	lua_gettable(L, arg);
	val = (float)luaL_optnumber(L, -1, def);
	lua_pop(L, 1);

	return val;
}

double LGetOptDoubleIndex(lua_State *L, int arg, int index, double def) {
	double val;

	L_INDEX_CHECK(arg);
	lua_pushnumber(L, index);
	lua_gettable(L, arg);
	val = luaL_optnumber(L, -1, def);
	lua_pop(L, 1);

	return val;
}

//------------------------------------------------//

int LCheckBool(lua_State *L, int idx) {
	if (lua_isboolean(L, idx))
		return lua_toboolean(L, idx);

	return luaL_typerror(L, idx, lua_typename(L, LUA_TBOOLEAN));
}

//------------------------------------------------//

void LOpenLibCreate(lua_State *L, const struct luaL_reg *functions) {
	lua_newtable(L);
	luaL_openlib(L, 0, functions, 0);
}

void LOpenLibOptCreate(lua_State *L, const struct luaL_reg *functions) {
	if (!lua_istable(L, -1))
		lua_newtable(L);

	luaL_openlib(L, 0, functions, 0);
}

void LOpenLibIndex(lua_State *L, int idx, const struct luaL_reg *functions) {
	if (lua_istable(L, idx))
		luaL_openlib(L, 0, functions, 0);
}

//------------------------------------------------//

void LGetElement(lua_State *L, const char *first_element, ...) {
	va_list vl;
	const char *elem;

	if (!first_element) return;

	lua_getglobal(L, first_element);
	if (lua_isnil(L, -1)) return;

	va_start(vl, first_element);

	while ((elem = va_arg(vl, const char *))) {
		if (lua_isnil(L, -1)) return;
		lua_getfield(L, -1, elem);
		lua_remove(L, -2);
	}

	va_end(vl);
}

//------------------------------------------------//

static void LReportDefault(lua_State *L) {
	if (!lua_isnil(L, -1)) {
		const char *msg = lua_tostring(L, -1);

		if (!msg) msg = "(error object is not a string)";

		printf("%s\n", msg);
		lua_pop(L, 1);
	}
}

void(*LReport)(lua_State *L) = LReportDefault;

void LSetReportFunc(LLVoidFunc reportfunc) {
	LReport = reportfunc;
	if (!LReport) LReport = LReportDefault;
}

//------------------------------------------------//

static int traceback(lua_State *L) {
	if (!lua_isstring(L, 1)) { /* Non-string error object? Try metamethod. */
		if (lua_isnoneornil(L, 1) || !luaL_callmeta(L, 1, "__tostring") || !lua_isstring(L, -1))
			return 1;  /* Return non-string error object. */
		lua_remove(L, 1);  /* Replace object by result of __tostring metamethod. */
	}
	luaL_traceback(L, L, lua_tostring(L, 1), 1);
	return 1;
}

int LCall(lua_State *L, int narg, int numresults) {
	int status;
	int base = lua_gettop(L) - narg;

	lua_pushcfunction(L, traceback);
	lua_insert(L, base);

	status = lua_pcall(L, narg, numresults, base);
	lua_remove(L, base);
	if (status) {
		LReport(L);
		lua_gc(L, LUA_GCCOLLECT, 0);
	}

	return status;
}

int LLoad(lua_State *L, const char *filename) {
	int status = luaL_loadfile(L, filename);
	if (status) LReport(L);
	else status = LCall(L, 0, LUA_MULTRET);
	return status;
}

int LParse(lua_State *L, const char *string) {
	int status = luaL_loadstring(L, string);
	if (status) LReport(L);
	else status = LCall(L, 0, LUA_MULTRET);
	return status;
}

int LParseB(lua_State *L, const unsigned char *buffer, size_t size) {
	int status = luaL_loadbuffer(L, (const char *)buffer, size, "");
	if (status) LReport(L);
	else status = LCall(L, 0, LUA_MULTRET);
	return status;
}

void LSetPkgPath(lua_State *L, const char *path) {
	lua_getglobal(L, "package");
	lua_pushstring(L, path);
	lua_setfield(L, -2, "path");
	lua_pop(L, 1);
}

void LAppendPkgPath(lua_State *L, const char *path) {
	const char *currentpaths;
	char *paths;
	size_t pathLen = 0, len = 0;

	lua_getglobal(L, "package");
	currentpaths = LGetCharFieldFull(L, -1, "path", &len, 0);

	pathLen = strlen(path);
	
	if (currentpaths) {
		paths = (char *)malloc(sizeof(char) * (len + pathLen + 2));
		memcpy(paths, currentpaths, len);
		paths[len] = ';';
		memcpy(paths + len + 1, path, pathLen + 1);
	} else {
		len += pathLen + 1;
		paths = (char *)malloc(sizeof(char) * len);
		memcpy(paths, path, len);
	}

	lua_pushstring(L, paths);
	lua_setfield(L, -2, "path");
	lua_pop(L, 1);

	free(paths);
}

void LRemoveLastPkgPath(lua_State *L) {
	const char *currentpaths;
	size_t totalsize = 0;

	lua_getglobal(L, "package");
	currentpaths = LGetCharFieldFull(L, -1, "path", &totalsize, 0);

	if (currentpaths) {
		const char *last = strrchr(currentpaths, ';');
		if (last) {
			char *paths;

			totalsize = last - currentpaths;
			paths = (char *)malloc(sizeof(char) * (totalsize + 1));
			memcpy(paths, currentpaths, totalsize);
			paths[totalsize] = 0;

			lua_pushstring(L, paths);
			lua_setfield(L, -2, "path");

			free(paths);
		} else {
			lua_pushnil(L);
			lua_setfield(L, -2, "path");
		}
	}

	lua_pop(L, 1);
}
