#include "stdafx.h"

#include "common.h"
#include "table.h"

static char ptrcompare(lua_State *L)
{
	if(lua_topointer(L,-1) < lua_topointer(L,-2)) return 1;
	else if (lua_topointer(L,-1) > lua_topointer(L,-2)) return -1;

	return 0;
}

static int table_binary_insert(lua_State *L)
{
	// STACK
	// 3 - optional function
	// 2 - element to insert
	// 1 - table

	unsigned char customcomp = !lua_isnil(L,3);
	unsigned int mini = 0, maxi = lua_objlen(L,1);
	unsigned int i = (maxi+mini)/2;

	/*printf("input:\n");
	printf("table: "); lua_global_print_table_index(L,1);
	printf("\nvalue to insert: "); lua_print_var(L,2,0);
	printf("\n----------------------------\n");*/

	if(customcomp) lua_pushvalue(L,3);
	lua_pushnumber(L,i+1);
	lua_gettable(L,1);

	while(!lua_isnil(L,-1)) {
		char larger = 0;

		lua_pushvalue(L,2);

		if (customcomp) {

			lua_call(L,2,1);
			larger = (char) luaL_checkint(L,-1);

			lua_pop(L,1);

			lua_pushvalue(L,3);
		} else {
			larger = ptrcompare(L);
			lua_pop(L,2);
		}

		if(larger == -1) {
			maxi = i;
		} if(larger) {
			mini = i;
		} else break;

		if(mini+1 == maxi) break;
		i = (maxi+mini)/2;

		lua_pushnumber(L,i+1);
		lua_gettable(L,1);
	}

	LGetElement(L,"table","insert",0);
	lua_pushvalue(L,1);
	lua_pushnumber(L,maxi+1);
	lua_pushvalue(L,2);
	lua_call(L,3,0);

	lua_pop(L,customcomp ? 2 : 1);

	return 0;
}

static int table_help(lua_State *L)
{
	printf("Table:\n");
	printf("Contains functions specific to table manipulation.\n\n");
	printf("Content:\n");
	printf("\thelp () : display this text\n");
	printf("\tbin_insert (table,item, opt function) : \n");
	printf("\t\tsort insert the item into the table according to the provided function. Nil value for function will use default comparison\n");

	return 0;
}

void LRegisterTable(lua_State *L)
{
	LSetFunctionField(L, -1, "bin_insert", table_binary_insert);
	LSetFunctionField(L, -1, "help"      , table_help);
}
