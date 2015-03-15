
#ifndef _included_zenith_pipe_h
#define _included_zenith_pipe_h

void ZPipeInitialize(void);
void ZPipeDestroy(void);

int ZPipeSend(lua_State *L, int idx);
int ZPipeReceive(lua_State *L, int idx);

void ZPipeCreate(const char *name, lua_State *L1, lua_State *L2);

void ZPipeRegister(lua_State *L);

#endif
