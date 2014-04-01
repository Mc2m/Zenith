
#include "stdafx.h"

#include "zenith.h"

#include <windows.h>
#include <conio.h>
#include <io.h>

unsigned char stop = 0;

size_t test1(void *param) {
	int idx = *((int *)param);
	lua_State *L = zenith_state_open(idx);

	luaL_openlibs(L);

	//test 1
	while(!stop) {
		l_parse(L,"print('bleh')");
	}

	return 0;
}

size_t test2(void *param) {
	int idx = *((int *)param);
	lua_State *L = zenith_state_open(idx);

	luaL_openlibs(L);

	//test 2
	while(!stop) {
		l_parse(L,"print('blah')");
	}

	return 0;
}

int main(int argc, char **argv)
{
	//ParticleThread *thread1, *thread2;
	int var1 = 0, var2 = 1;
	lua_State *L;

	zenith_state_initialize(1);

	/*thread1 = particle_thread_create(test1, &var1);
	thread2 = particle_thread_create(test2, &var2);

	particle_thread_join(thread1);
	particle_thread_join(thread2);*/

	L = zenith_state_open(0);

	zenith_lib_table(L);

	l_parse(L,"Zenith.Table.help()");

	zenith_state_destroy();

	return 0;
}
