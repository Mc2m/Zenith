
#ifndef _included_zenith_transfer_h
#define _included_zenith_transfer_h

void zenith_transfer_set_table_transfer_method(void (*cpy)(lua_State *from, lua_State *to, int idx));
void zenith_transfer_set_default_table_transfer_method(void);

void zenith_transfer_data(lua_State *from, lua_State *to, int idx);
void zenith_transfer_range(lua_State *from, lua_State *to, int start, int end);

#endif
