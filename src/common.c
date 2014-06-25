
#include "stdafx.h"

#include "common.h"
#include "state.h"

#include <string.h>

#define INDEX_CHECK(i) (i < 0 && i != LUA_REGISTRYINDEX ? --i : i)
#define INDEX_RESTORE(i) (i < 0 && i != LUA_REGISTRYINDEX ? i++ : i)

void ZPrintVar(lua_State *L,int index,unsigned char showtype)
{
	int type = lua_type(L, index);
	if (type == LUA_TSTRING)       printf( "%s", lua_tostring(L,index));
	else if (type == LUA_TNUMBER)  printf( "%lf", lua_tonumber(L,index));
	else if (type == LUA_TNIL)     printf( "nil");
	else if (type == LUA_TBOOLEAN) printf( lua_toboolean(L,index) ? "true" : "false");
	else                           printf( "%p", lua_topointer(L,index));

	if(showtype) printf( "(%s)", lua_typename(L,type));
}

void ZPrintTable(lua_State *L, int index)
{
	if(!lua_istable(L,index)) {
		printf("Warning: variable at index %d of the stack is a %s.\n",index,lua_typename(L,lua_type(L,index)));
		return;
	}

	printf("=========    TABLE    =========\n");
	printf("pointer ref : %p\n",lua_topointer(L,index));

	printf("\nmetatable : ");

	if(lua_getmetatable(L,index)) {
		printf( "%p\n", lua_topointer(L,index));
		lua_pop(L,1);
	} else {
		printf("nil\n");
	}

	INDEX_CHECK(index);

	printf("\ncontent :\n\t");

	lua_pushnil(L);

	while (lua_next(L, index)) {
		/* uses 'key' (at index -2) and 'value' (at index -1) */
		ZPrintVar(L, -2, 0);
		printf(" - ");
		ZPrintVar(L, -1, 0);
		printf("\n\t");

		lua_pop(L, 1);
	}

	printf("\n===============================\n");
}

void ZPrintStack(lua_State *L)
{
    int arg_count = lua_gettop( L );
    int i;

	printf("===============================\n");
	printf("=          LUA STACK          =\n");
	printf("===============================\n");

    for ( i = arg_count; i > 0; -- i ) {
		printf("\n %d : \t",i);
		ZPrintVar(L,i,1);
    }

	printf("\n===============================\n\n");
}

//------------------------------------------------//

void ZSetBoolField(lua_State *L,int index,const char *field, unsigned char value) {
	INDEX_CHECK(index);

	lua_pushboolean(L,value);
	lua_setfield(L,index,field);
}

void ZSetIntField(lua_State *L,int index,const char *field, int value) {
	INDEX_CHECK(index);

	lua_pushnumber(L, value);
	lua_setfield(L,index,field);
}

void ZSetCharField(lua_State *L,int index,const char *field, const char *value) {
	INDEX_CHECK(index);

	lua_pushstring(L, value);
	lua_setfield(L,index,field);
}

void ZSetFunctionField(lua_State *L,int index,const char *field, int (*value)(lua_State *L)) {
	INDEX_CHECK(index);

	lua_pushcfunction(L, value);
	lua_setfield(L,index,field);
}

void ZSetTableField(lua_State *L,int index,const char *field, void (*value)(lua_State *L)) {
	INDEX_CHECK(index);

	lua_newtable(L);
	if(value) value(L);
	lua_setfield(L,index,field);
}

//------------------------------------------------//

unsigned char ZGetBoolField(lua_State *L,int arg,const char *field)
{
	unsigned char val = 0;
	lua_getfield(L,arg,field);
	luaL_checktype(L,-1,LUA_TBOOLEAN);
	val = lua_toboolean(L,-1);
	lua_pop(L,1);
	return val;
}

int ZGetIntField (lua_State *L,int arg,const char *field) {
	int val = 0;
	lua_getfield(L,arg,field);
	val = luaL_checkint(L,-1);
	lua_pop(L,1);
	return val;
}

int ZGetOptIntField(lua_State *L,int arg,const char *field, int def)
{
	int val = 0;
	lua_getfield(L,arg,field);
	val = luaL_optint(L,-1,def);
	lua_pop(L,1);
	return val;
}

double ZGetRealField(lua_State *L,int arg,const char *field)
{
	double val = 0;
	lua_getfield(L,arg,field);
	val = luaL_checknumber(L,-1);
	lua_pop(L,1);
	return val;
}

const char *ZGetCharField(lua_State *L,int arg,const char *field) {
	const char *val = 0;
	lua_getfield(L,arg,field);
	val = luaL_checkstring(L,-1);
	lua_pop(L,1);
	return val;
}

const char *ZGetCharFieldFull(lua_State *L,int arg,const char *field, size_t *size, const char *def) {
	const char *val = 0;
	lua_getfield(L,arg,field);
	val = luaL_optlstring(L,-1, def,size);
	lua_pop(L,1);
	return val;
}

//------------------------------------------------//

void ZSetIntIndex(lua_State *L,int arg,int index, int value)
{
	if(arg < 0 && arg != LUA_REGISTRYINDEX) arg -= 2;
	lua_pushinteger(L,index);
	lua_pushinteger(L,value);
	lua_settable(L,arg);
}

//------------------------------------------------//

int ZGetIntIndex(lua_State *L,int arg,int index) {
	int val = 0;

	if(arg < 0 && arg != LUA_REGISTRYINDEX) arg -= 2;
	lua_pushnumber(L,index);
	lua_gettable(L,arg);
	val = luaL_checkint(L,-1);
	lua_pop(L,1);

	return val;
}

//------------------------------------------------//

void ZGetElement(lua_State *L,const char *first_element,...)
{
	va_list vl;
	const char *elem;

	if(!first_element) return;

	lua_getglobal(L,first_element);

	va_start(vl,first_element);

	while((elem = va_arg(vl,const char *)))
	{
		lua_getfield(L,-1,elem);
		lua_remove(L,-2);
	}

	va_end(vl);
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

static void report(lua_State *L)
{
	if (!lua_isnil(L, -1)) {
		const char *msg = lua_tostring(L, -1);
		size_t idx;
		const ZState *S = ZStateFromState(L,&idx);

		if(!msg) msg = "(error object is not a string)";

		if(S && S->name) printf("State %s: %s\n",S->name, msg);
		else printf("State %d: %s\n",idx, msg);
		lua_pop(L, 1);
	}
}

void ZCall (lua_State *L, int narg, int numresults) {
	int status;
	int base = lua_gettop(L) - narg;

	lua_pushcfunction(L, traceback);
	lua_insert(L,base);

	status = lua_pcall(L, narg, numresults, base);
	lua_remove(L, base);
	if (status != 0) {
		report(L);
		lua_gc(L, LUA_GCCOLLECT, 0);
	}
}

void ZLoad (lua_State *L, const char *filename) {
	if(luaL_loadfile(L, filename))
		report(L);
	else
		ZCall(L,0,LUA_MULTRET);
}

void ZParse (lua_State *L, const char *string)
{
	if(luaL_loadstring(L, string))
		report(L);
	else
		ZCall(L,0,LUA_MULTRET);
}

void ZSetPkgPath(lua_State *L, const char *path)
{
	lua_getglobal(L,"package");
	lua_pushstring(L,path);
	lua_setfield(L,-2,"path");
	lua_pop(L,1);
}

void ZAppendPkgPath(lua_State *L, const char *path)
{
	const char *currentpaths;
	char *paths;
	size_t totalsize = 0;

	lua_getglobal(L,"package");
	currentpaths = ZGetCharFieldFull(L, -1,"path",&totalsize,0);

	totalsize += strlen(path) + 2;
	paths = (char *) malloc(totalsize);
	snprintf(paths,totalsize,"%s;%s",currentpaths,path);

	lua_pushstring(L,paths);
	lua_setfield(L,-2,"path");
	lua_pop(L,1);

	free(paths);
}
