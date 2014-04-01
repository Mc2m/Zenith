
#ifndef _included_zenith_transfer_h
#define _included_zenith_transfer_h

void transfer_data(lua_State *from, lua_State *to, int idx);
void transfer_range(lua_State *from, lua_State *to, int start, int end);

#endif
