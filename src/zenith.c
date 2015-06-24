
#include "stdafx.h"

#include "zenith.h"

void ZenithInitialize(int flags)
{
	if(flags & ZENITH_LIBRARY_PIPE)
		ZPipeInitialize();

	if (flags & ZENITH_LIBRARY_STATE)
		LSetReportFunc(ZStateReport);
}

void ZenithDestroy(void)
{
	ZPipeDestroy();
	LSetReportFunc(0);
}

void ZenithOpenLibrary(lua_State *L, int flags)
{
	lua_newtable(L);

	if (flags & ZENITH_LIBRARY_STATE) {
		lua_newtable(L);
		ZStateRegister(L);
		lua_setfield(L,-2,"State");
	}

	/*if (flags & ZENITH_LIBRARY_PIPE) {
		lua_newtable(L);
		ZPipeRegister(L);
		lua_setfield(L,-2,"Pipe");
	}*/

	lua_setglobal(L,"Zenith");
}
