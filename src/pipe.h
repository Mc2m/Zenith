
#ifndef _included_zenith_pipe_h
#define _included_zenith_pipe_h

void ZPipeInitialize(void);
void ZPipeDestroy(void);

void ZPipeSend(lua_State *L, int idx);
void ZPipeCustomSend(lua_State *L, int idx, void (*table_copy)(lua_State *from, lua_State *to, int idx));

void ZPipeCreate(lua_State *L1,lua_State *L2, const char *name);

#endif
