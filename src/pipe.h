
#ifndef _included_zenith_pipe_h
#define _included_zenith_pipe_h

void zenith_pipe_initialize(void);
void zenith_pipe_destroy(void);

int zenith_pipe_send(lua_State *L);
int zenith_pipe_receive(lua_State *L);

void zenith_pipe_create(lua_State *L1,lua_State *L2, const char *name);

#endif
