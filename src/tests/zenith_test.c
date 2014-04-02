
#include "stdafx.h"

#include "zenith.h"

#include "particle/particle.h"

unsigned char stop = 0;

size_t test1(void *param) {
	int idx = *((int *)param);
	lua_State *L = zenith_state_from_idx(idx);

	luaL_openlibs(L);

	//test 1
	while(!stop) {
		l_parse(L,"print('bleh')");
	}

	return 0;
}

size_t test2(void *param) {
	int idx = *((int *)param);
	lua_State *L = zenith_state_from_idx(idx);

	luaL_openlibs(L);

	//test 2
	while(!stop) {
		l_parse(L,"print('blah')");
	}

	return 0;
}

int main(int argc, char **argv)
{
	ParticleThread *thread1, *thread2;
	int var1 = 0, var2 = 1;
	lua_State *L1,*L2;

	zenith_state_initialize(2);
	zenith_pipe_initialize();

	L1 = zenith_state_open(0);
	L2 = zenith_state_open(1);

	luaL_openlibs(L1);
	luaL_openlibs(L2);

	zenith_pipe_create(L1,L2,"test");

	lua_pushstring(L1,"bleh");
	l_parse(L1,"Zenith.Pipe.pipes.test:send(3,\"test\")");

	l_parse(L2,"print(Zenith.Pipe.pipes.test:receive())");
	l_parse(L2,"print(Zenith.Pipe.pipes.test:receive())");
	
	//zenith_pipe_create(zenith_state_open(0),zenith_state_open(1));

	/*thread1 = particle_thread_create(test1, &var1);
	thread2 = particle_thread_create(test2, &var2);

	particle_thread_join(thread1);
	particle_thread_join(thread2);*/

	zenith_pipe_destroy();
	zenith_state_destroy();

	return 0;
}
