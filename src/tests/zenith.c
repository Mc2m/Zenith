
#include "stdafx.h"

#include "state.h"
#include "transfer.h"
#include "pipe.h"

#include "zenith.h"

#include <debugging/tdebug.h>
#include <io/trw.h>
#include <tthread.h>

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

#define NUM_PIPE_TESTS 6

int ZTestPipeThread1(lua_State *L)
{
	int amount = 0;

	lua_pushvalue(L,1);
	lua_pushinteger(L,5000);
	lua_pushinteger(L,1);
	amount = ZPipeSend(L,2);

	if(amount > 1) amount = 1;

	return amount;
}

int ZTestPipeThread2(lua_State *L)
{
	int amount = 0;

	while (!amount) {
		lua_pushvalue(L,1);
		lua_pushinteger(L,1000);
		amount = ZPipeReceive(L,2);
	}
	lua_pop(L,1);

	lua_pushvalue(L,1);
	lua_pushinteger(L,0);
	lua_pushinteger(L,1);
	ZPipeSend(L,2);

	return 0;
}

int ZTestPipe(void)
{
	int result = 0;
	int amount;
	TThread *thread1, *thread2;

	lua_State *L1 = ZStateNew("Test State 1");
	lua_State *L2 = ZStateNew("Test State 2");

	ZenithOpenLibrary(L1,ZENITH_LIBRARY_PIPE);
	ZenithOpenLibrary(L2,ZENITH_LIBRARY_PIPE);

	//test pipe creation
	ZPipeCreate("Test pipe",L1,L2);

	LGetElement(L1,"Zenith","Pipe","Pipes","Test pipe",NULL);
	LGetElement(L2,"Zenith","Pipe","Pipes","Test pipe",NULL);
	if(lua_istable(L1,1) && lua_istable(L2,1)) {
		result++;
	} else {
		//no need to continue
		ZStateClose(L1);
		ZStateClose(L2);
		return result;
	}

	// the pipe exists now we can test sending and receiving

	// send malformed from c
	lua_pushvalue(L1,1);
	lua_pushstring(L1, "love to test");
	ZPipeSend(L1,2);

	if(lua_isstring(L1,-1)) {
		result++;
		lua_pop(L1,1);
	}

	// send correct from c
	lua_pushvalue(L1,1);
	lua_pushinteger(L1,0);
	lua_pushstring(L1, "love to test");
	ZPipeSend(L1,2);

	if(lua_gettop(L1) == 1) {
		result++;
	}

	// receive from c
	lua_pushvalue(L2,1);
	lua_pushinteger(L2,0);
	ZPipeReceive(L2,2);
	
	if(lua_isstring(L2,2) && !strcmp("love to test",lua_tostring(L2,2))) {
		result++;
	}
	lua_pop(L2,1);

	// send 3 groups of data
	lua_pushvalue(L1,1);
	lua_pushinteger(L1,0);
	lua_pushstring(L1, "send");
	lua_pushinteger(L1, 1);
	ZPipeSend(L1,2);

	lua_pushvalue(L1,1);
	lua_pushinteger(L1,0);
	lua_pushstring(L1, "send");
	lua_pushinteger(L1, 2);
	ZPipeSend(L1,2);

	lua_pushvalue(L1,1);
	lua_pushinteger(L1,0);
	lua_pushstring(L1, "send");
	lua_pushinteger(L1, 3);
	ZPipeSend(L1,2);

	// receive the first group
	lua_pushvalue(L2,1);
	lua_pushinteger(L2,0);
	lua_pushboolean(L2,0);
	amount = ZPipeReceive(L2,2);
	
	if(amount == 2) {
		char validstr = lua_isstring(L2,2) && !strcmp("send",lua_tostring(L2,2));
		char validint = lua_isnumber(L2,3) && lua_tointeger(L2,3) == 1;
		if(validstr && validint)
			result++;
	}
	lua_pop(L2,lua_gettop(L2)-1);

	// receive the rest
	lua_pushvalue(L2,1);
	lua_pushinteger(L2,0);
	amount = ZPipeReceive(L2,2);
	
	if(amount == 4) {
		char validint = lua_isnumber(L2,3) && lua_tointeger(L2,3) == 2;
		validint = validint && lua_isnumber(L2,5) && lua_tointeger(L2,5) == 3;
		if(validint)
			result++;
	}
	lua_pop(L2,lua_gettop(L2)-1);

	// create threads
	thread2 = TThreadCreate((TThreadFunc) ZTestPipeThread2,L2);
	thread1 = TThreadCreate((TThreadFunc) ZTestPipeThread1,L1);

	TThreadJoin(thread2);
	result += TThreadJoin(thread1);

	ZStateClose(L1);
	ZStateClose(L2);

	return result;
}

int ZTestRequest(void)
{
	int result = 0;

	lua_State *L1 = ZStateNew("Test State 1");
	lua_State *L2 = ZStateNew("Test State 2");

	luaL_openlibs(L1);
	luaL_openlibs(L2);

	ZenithOpenLibrary(L1,ZENITH_LIBRARY_PIPE);
	ZenithOpenLibrary(L2,ZENITH_LIBRARY_PIPE);

	//test pipe creation
	ZPipeCreate("Test pipe",L1,L2);

	LLoad(L1,"zenith_test.lua");
	
	LParse(L2,"local RequestManager = require('RequestManager') RequestManager:receive()");

	lua_getglobal(L2,"test");
	if(lua_isstring(L2,1)) {
		result++;
	}

	ZStateClose(L1);
	ZStateClose(L2);

	return result;
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

	TRWWrite(output,"Testing Pipe... ");
	result = ZTestPipe();
	TRWWrite(output,"Completed %d/%d tests\n",result,NUM_PIPE_TESTS);
	if(result != NUM_PIPE_TESTS) {
		TRWWrite(output,"Cannot continue testing. Aborting.\n");
		return 0;
	}

	ZTestRequest();

	TRWWrite(output,"Tests have been completed.\n");

	ZTestDestroy();

	TRWFree(output);

	return 0;
}
