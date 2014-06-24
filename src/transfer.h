
#ifndef _included_zenith_transfer_h
#define _included_zenith_transfer_h

void ZTransferSetTableTransferMethod(void (*cpy)(lua_State *from, lua_State *to, int idx));
void ZTransferSetDefaultTableTransferMethod(void);

void ZTransferData(lua_State *from, lua_State *to, int idx);
void ZTransferRange(lua_State *from, lua_State *to, int start, int end);

#endif
