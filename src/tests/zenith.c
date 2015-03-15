
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

#define NUM_TRANSFER_TESTS 7

int ZTestTransfer(void)
{
	int result = 0;
	char *teststring = (char *) "Teststr ing";
	lua_State *L1 = ZStateNew("Test State 1");
	lua_State *L2 = ZStateNew("Test State 2");

	//test transfer nothing
	ZTransferData(L1,L2,1);
	result++;

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
	lua_pushstring(L1,"Teststr ing");
	ZTransferData(L1,L2,1);
	if(lua_isstring(L2,1) && !strcmp(lua_tostring(L2,1),"Teststr ing")) {
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

	//test table
	//test function

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
	//ZTestPipe();
	//ZTestRequestManager();
	//ZTestExecuteRequest();

	TRWWrite(output,"Tests have been completed.\n");

	ZTestDestroy();

	TRWFree(output);

	return 0;
}
