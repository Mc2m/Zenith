
#include "stdafx.h"

#include "common.h"
#include "state.h"

size_t num_states = 0;
lua_State **states = 0;

void zenith_state_set_amount(size_t _num_states)
{
	if (num_states < _num_states) {
		states = (lua_State **) realloc(states,sizeof(lua_State *) * _num_states);
		memset(states+num_states,0,sizeof(lua_State *) * (_num_states-num_states));
		num_states = _num_states;
	} else if (num_states > _num_states) {
		size_t i = _num_states;
		for(; i < num_states; ++i) zenith_state_close(i);
		states = (lua_State **) realloc(states,sizeof(lua_State *) * _num_states);
		num_states = _num_states;
	}
}

void zenith_state_initialize(size_t num_states)
{
	zenith_state_set_amount(num_states);
}

void zenith_state_destroy(void)
{
	size_t i = 0;
	for(; i < num_states; ++i) {
		if(states[i]) {
			lua_close(states[i]);
			states[i] = 0;
		}
	}

	free(states);
	states = 0;
	num_states = 0;
}

lua_State *zenith_state_open(size_t idx)
{
	lua_State *L;
	
	if (idx >= num_states) return 0;
	L = states[idx];
	if(L) return L;

	L = luaL_newstate();
	l_setintfield(L, LUA_REGISTRYINDEX, "state_idx", idx);

	return L;
}

void zenith_state_close(size_t idx)
{
	if (idx < num_states && states[idx]) {
		lua_close(states[idx]);
		states[idx] = 0;
	}
}

lua_State *zenith_state_from_idx(size_t idx)
{
	if (idx >= num_states) return 0;
	return states[idx];
}

size_t zenith_state_from_state(lua_State *L)
{
	return l_getintfield(L, LUA_REGISTRYINDEX, "state_idx");
}

void register_zenith_state_table(lua_State *L)
{
	//l_settablefield(L, -1, "mutex", register_mutex_functions);
}
