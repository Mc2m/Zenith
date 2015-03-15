
#ifndef _included_zenith_h
#define _included_zenith_h

#define ZENITH_NAME        "Zenith"
#define ZENITH_DESCRIPTION "C platform for Lua IPC"
#define ZENITH_VERSION     "0.5.0"

#define ZENITH_LIBRARY_STATE 1
#define ZENITH_LIBRARY_PIPE 2

#include "state.h"

#include "pipe.h"

void ZenithInitialize(int flags);
void ZenithDestroy(void);

void ZenithOpenLibrary(lua_State *L, int flags);

#endif
