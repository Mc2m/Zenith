
#include "stdafx.h"

#include "zenith.h"

#include "thread/tthread.h"

int test1(void *param) {
	lua_State *L = (lua_State *) param;

	ZParse(L,"Zenith.Pipe.pipes.test:send('bleh',nil,'test') Zenith.Pipe.pipes.test:send(3)");

	return 0;
}

int test2(void *param) {
	lua_State *L = (lua_State *) param;

	ZParse(L,"local p = Zenith.Pipe.pipes.test local val = p:listen(1000,0) print(val) local val = p:listen(1000,0) print(val)");

	return 0;
}

int main(int argc, char **argv)
{
	TThread *thread1, *thread2;
	const ZState *S1,*S2;

	ZStateInitialize(2);
	ZPipeInitialize();

	S1 = ZStateOpen(0,"sender");
	S2 = ZStateOpen(1,"receiver");

	luaL_openlibs(S1->L);
	luaL_openlibs(S2->L);

	ZPipeCreate(S1->L,S2->L,"test");

	thread1 = TThreadCreate(test1,S1->L);
	TThreadJoin(thread1);

	thread2 = TThreadCreate(test2,S2->L);
	TThreadJoin(thread2);

	ZPipeDestroy();
	ZStateDestroy();

	return 0;
}
