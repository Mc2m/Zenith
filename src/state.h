
#ifndef _included_zenith_state_h
#define _included_zenith_state_h

void zenith_state_set_amount(size_t num_states);

void zenith_state_initialize(size_t num_states);
void zenith_state_destroy(void);

lua_State *zenith_state_open(size_t idx);
void zenith_state_close(size_t idx);

lua_State *zenith_state_from_idx(size_t idx);
size_t zenith_state_from_state(lua_State *L);

void register_zenith_state_table(lua_State *L);

#endif
