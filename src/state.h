
#ifndef _included_zenith_state_h
#define _included_zenith_state_h

typedef struct ZState {
	char *name;
	lua_State *L;
} ZState;

void ZStateSetAmount(size_t num_states);

void ZStateInitialize(size_t num_states);
void ZStateDestroy(void);

const ZState *ZStateOpen(size_t idx, const char *name);
void ZStateClose(size_t idx);

const ZState *ZStateFromIdx(size_t idx);
const ZState *ZStateFromState(lua_State *L, size_t *idx);

//void register_zenith_state_table(lua_State *L);

#endif
