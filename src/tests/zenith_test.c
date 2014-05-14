
#include "stdafx.h"

#include "zenith.h"

#include "thread/tthread.h"

int test1(void *param) {
	lua_State *L = (lua_State *) param;

	l_parse(L,"Zenith.Pipe.pipes.test:send('bleh',nil,'test') Zenith.Pipe.pipes.test:send(3)");

	return 0;
}

int test2(void *param) {
	lua_State *L = (lua_State *) param;

	l_parse(L,"local p = Zenith.Pipe.pipes.test local val = p:listen(0) local val = p:listen(0) print(val)");

	return 0;
}

int main(int argc, char **argv)
{
	TThread *thread1, *thread2;
	lua_State *L1,*L2;

	zenith_state_initialize(2);
	zenith_pipe_initialize();

	L1 = zenith_state_open(0,"sender");
	L2 = zenith_state_open(1,"receiver");

	luaL_openlibs(L1);
	luaL_openlibs(L2);

	zenith_pipe_create(L1,L2,"test");

	thread1 = TThreadCreate(test1,L1);
	TThreadJoin(thread1);

	thread2 = TThreadCreate(test2,L2);
	TThreadJoin(thread2);

	zenith_pipe_destroy();
	zenith_state_destroy();

	return 0;
}
