
#include "stdafx.h"

#include "zenith.h"

#include "particle/particle.h"

size_t test1(void *param) {
	lua_State *L = (lua_State *) param;

	l_parse(L,"local res = Zenith.Pipe.pipes.test:wait(3) if res then print(res) else print('nil') end");

	return 0;
}

size_t test2(void *param) {
	lua_State *L = (lua_State *) param;

	l_parse(L,"local p = Zenith.Pipe.pipes.test local val = p:receive() while not val do val = p:receive() end print(val) p:send()");

	return 0;
}

int main(int argc, char **argv)
{
	ParticleThread *thread1, *thread2;
	lua_State *L1,*L2;

	zenith_state_initialize(2);
	zenith_pipe_initialize();

	L1 = zenith_state_open(0,"sender");
	L2 = zenith_state_open(1,"receiver");

	luaL_openlibs(L1);
	luaL_openlibs(L2);

	zenith_pipe_create(L1,L2,"test");

	thread1 = particle_thread_create(test1,L1);
	thread2 = particle_thread_create(test2,L2);

	particle_thread_join(thread1);
	particle_thread_join(thread2);

	zenith_pipe_destroy();
	zenith_state_destroy();

	return 0;
}
