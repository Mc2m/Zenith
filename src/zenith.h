
#ifndef _included_zenith_h
#define _included_zenith_h

#define ZENITH_NAME        "Zenith"
#define ZENITH_DESCRIPTION "C platform for Lua IPC"
#define ZENITH_VERSION     "0.5.0"

#include "common.h"
#include "state.h"

#include "pipe.h"

void ZLibTable(lua_State *L);
void ZLibState(lua_State *L);

#endif
