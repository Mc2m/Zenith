
#ifndef _included_zenith_pipe_h
#define _included_zenith_pipe_h

void zenith_pipe_initialize(void);
void zenith_pipe_destroy(void);

void zenith_pipe_create(lua_State *L1,lua_State *L2, const char *name);

#endif
