
#include "stdafx.h"

#include "state.h"
#include "transfer.h"
#include "pipe.h"

#include "zenith.h"

#include <debugging/tdebug.h>
#include <io/trw.h>

#include <string.h>

void ZTestInitialize(void)
{
	ZenithInitialize(ZENITH_LIBRARY_PIPE | ZENITH_LIBRARY_STATE);
}

#define NUM_STATE_TESTS 1

int ZTestState(void)
{
	lua_State *L = ZStateNew("Test State");
	if(!L) return 0;

	ZStateClose(L);

	return 1;
}

#define NUM_TRANSFER_TABLE_TESTS 6

int ZTestTransferTable(lua_State *L1, lua_State *L2)
{
	int result = 0;
	const char *teststring = "test";

	// simple table
	lua_newtable(L1);
	ZTransferData(L1,L2,1);
	if(lua_istable(L2,1)) {
		result++;
	} else {
		// no need to continue this
		lua_pop(L1,1);
		lua_pop(L2,1);
		return result;
	}
	lua_pop(L2,1);

	//add a string to the table
	lua_pushstring(L1,teststring);
	lua_setfield(L1, -2, teststring);
	ZTransferData(L1,L2,1);
	if(lua_istable(L2,1)) {
		lua_getfield(L2,1,teststring);
		if(lua_isstring(L2,-1) && !strcmp(lua_tostring(L2,-1),teststring))
			result++;
		lua_pop(L2,1);
	}
	lua_pop(L2,1);

	//add an index to the table
	lua_pushinteger(L1,1);
	lua_pushstring(L1,teststring);
	lua_settable(L1,1);
	ZTransferData(L1,L2,1);
	if(lua_istable(L2,1)) {
		lua_pushinteger(L2,1);
		lua_gettable(L2, 1);
		if(lua_isstring(L2,-1) && !strcmp(lua_tostring(L2,-1),teststring))
			result++;
		lua_pop(L2,1);
	}
	lua_pop(L2,1);

	//add a table in it
	lua_pushinteger(L1,1);
	lua_newtable(L1);
	lua_settable(L1,1);
	ZTransferData(L1,L2,1);
	if(lua_istable(L2,1)) {
		lua_pushinteger(L2,1);
		lua_gettable(L2, 1);
		if(lua_istable(L2,-1))
			result++;
		else {
			lua_pop(L2,1);
			return result;
		}

		lua_pop(L2,1);
	} else {
		return result;
	}
	lua_pop(L2,1);

	//set metatable
	lua_pushinteger(L1,1);
	lua_gettable(L1, 1);
	lua_setmetatable(L1, 1);
	ZTransferData(L1,L2,1);
	if(lua_istable(L2,1)) {
		lua_getmetatable(L2,1);
		lua_pushinteger(L2,1);
		lua_gettable(L2, 1);

		if(lua_topointer(L2, -1) == lua_topointer(L2, -2))
			result++;
		lua_pop(L2,2);
	}
	lua_pop(L2,1);
	lua_pop(L1,1);

	//set table as key

	//have table as key and value

	//reference base table int base table
	lua_newtable(L1);
	lua_pushinteger(L1,1);
	lua_pushvalue(L1,1);
	lua_settable(L1,1);
	ZTransferData(L1,L2,1);
	if(lua_istable(L2,1)) {
		lua_pushinteger(L2,1);
		lua_gettable(L2,1);
		LPrintStack(L2);

		if(lua_topointer(L2, -1) == lua_topointer(L2, 1))
			result++;
		lua_pop(L2,1);
	}
	lua_pop(L2,1);
	lua_pop(L1,1);

	return result;
}

#define NUM_TRANSFER_TESTS 8 + NUM_TRANSFER_TABLE_TESTS

int ZTestCFunction(lua_State *L)
{
	lua_pushinteger(L,1);
	return 1;
}

int ZTestTransfer(void)
{
	int result = 0;
	char *teststring = strdup("Teststr ing");
	lua_State *L1 = ZStateNew("Test State 1");
	lua_State *L2 = ZStateNew("Test State 2");

	//test transfer nothing
	ZTransferData(L1,L2,1);
	if(lua_gettop(L2) == 0) {
		result++;
	} else {
		lua_pop(L2,1);
	}

	//test nil transfer
	lua_pushnil(L1);
	ZTransferData(L1,L2,1);
	if(lua_isnil(L2,1)) {
		result++;
	}
	lua_pop(L1,1);
	lua_pop(L2,1);

	//test bool transfer
	lua_pushboolean(L1,0);
	ZTransferData(L1,L2,1);
	if(lua_isboolean(L2,1) && !lua_toboolean(L2,1)) {
		result++;
	}
	lua_pop(L1,1);
	lua_pop(L2,1);

	//test integer (and double) transfer
	lua_pushinteger(L1,50);
	ZTransferData(L1,L2,1);
	if(lua_isnumber(L2,1) && lua_tointeger(L2,1) == 50) {
		result++;
	}
	lua_pop(L1,1);
	lua_pop(L2,1);

	//test string
	lua_pushstring(L1,teststring);
	ZTransferData(L1,L2,1);
	if(lua_isstring(L2,1) && !strcmp(lua_tostring(L2,1),teststring)) {
		result++;
	}
	lua_pop(L1,1);
	lua_pop(L2,1);

	//test lightuserdata
	lua_pushlightuserdata(L1,teststring);
	ZTransferData(L1,L2,1);
	if(lua_isuserdata(L2,1) && teststring == lua_touserdata(L2,1)) {
		result++;
	}
	lua_pop(L1,1);
	lua_pop(L2,1);

	//test c function
	lua_pushcfunction(L1,ZTestCFunction);
	ZTransferData(L1,L2,1);
	if(lua_iscfunction(L2,1)) {
		lua_call(L2,0,1);
		if(lua_isnumber(L2,1) && lua_tointeger(L2,1) == 1)
			result++;
	}
	lua_pop(L1,1);
	lua_pop(L2,1);

	//test function
	luaL_loadstring(L1,"return 3");
	ZTransferData(L1,L2,1);
	if(lua_isfunction(L2,1)) {
		lua_call(L2,0,1);
		LPrintStack(L2);
		if(lua_isnumber(L2,1) && lua_tointeger(L2,1) == 3)
			result++;
	}
	lua_pop(L1,1);
	lua_pop(L2,1);

	//test table
	result += ZTestTransferTable(L1,L2);

	ZStateClose(L1);
	ZStateClose(L2);

	free(teststring);
	teststring = 0;

	return result;
}

int ZTestPipe(void)
{

}

void ZTestDestroy(void)
{
	ZenithDestroy();
}

int main(int argc, char **argv)
{
	int result;

	TRW *output = TRWFromFilePointer(stdout,0);

	ZTestInitialize();

	TRWWrite(output,"Running tests...\n");

	TRWWrite(output,"Testing States... ");
	result = ZTestState();
	TRWWrite(output,"Completed %d/%d tests\n",result,NUM_STATE_TESTS);
	if(result != NUM_STATE_TESTS) {
		TRWWrite(output,"Cannot continue testing. Aborting.\n");
		return 0;
	}

	TRWWrite(output,"Testing Transfer... ");
	result = ZTestTransfer();
	TRWWrite(output,"Completed %d/%d tests\n",result,NUM_TRANSFER_TESTS);
	if(result != NUM_TRANSFER_TESTS) {
		TRWWrite(output,"Cannot continue testing. Aborting.\n");
		return 0;
	}
	//ZTestPipe();
	//ZTestRequestManager();
	//ZTestExecuteRequest();

	TRWWrite(output,"Tests have been completed.\n");

	ZTestDestroy();

	TRWFree(output);

	return 0;
}
