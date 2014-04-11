
#ifndef _included_zenith_pipe_h
#define _included_zenith_pipe_h

void zenith_pipe_initialize(void);
void zenith_pipe_destroy(void);

void zenith_pipe_custom_send(lua_State *L, int idx, void (*table_copy)(lua_State *from, lua_State *to, int idx));

void zenith_pipe_create(lua_State *L1,lua_State *L2, const char *name);

#endif
