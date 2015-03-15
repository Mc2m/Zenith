
#ifndef _included_zenith_state_h
#define _included_zenith_state_h

void ZStateReport(lua_State *L);

lua_State *ZStateNew(const char *name);
void ZStateClose(lua_State *L);

void ZStateRegister(lua_State *L);

#endif
