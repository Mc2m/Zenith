
#include "stdafx.h"

#include "common.h"
#include "state.h"

#include <string.h>

static size_t num_states = 0;
static ZState *states = 0;

void ZStateSetAmount(size_t _num_states)
{
	if (num_states < _num_states) {
		states = (ZState *) realloc(states,sizeof(ZState) * _num_states);
		memset(states+num_states,0,sizeof(ZState) * (_num_states-num_states));

		num_states = _num_states;
	} else if (num_states > _num_states) {
		size_t i = _num_states;
		for(; i < num_states; ++i) ZStateClose(i);

		states = (ZState *) realloc(states,sizeof(ZState) * _num_states);

		num_states = _num_states;
	}
}

void ZStateInitialize(size_t num_states)
{
	ZStateSetAmount(num_states);
}

void ZStateDestroy(void)
{
	size_t i = 0;
	for(; i < num_states; ++i) {
		if(states[i].L) {
			lua_close(states[i].L);
			free(states[i].name);
			states[i].name = 0;
		}
	}

	free(states);
	states = 0;

	num_states = 0;
}

const ZState *ZStateOpen(size_t idx, const char *name)
{
	ZState *S;
	
	if (idx >= num_states) return 0;
	S = &states[idx];
	if(!S->L) {
		S->L = luaL_newstate();
		ZSetIntField(S->L, LUA_REGISTRYINDEX, "state_idx", idx);
	}

	if(name) {
		free(S->name);
		S->name = strdup(name);
	}

	return S;
}

void ZStateClose(size_t idx)
{
	if (idx < num_states && states[idx].L) {
		lua_close(states[idx].L);
		states[idx].L = 0;

		free(states[idx].name);
		states[idx].name = 0;
	}
}

const ZState *ZStateFromIdx(size_t idx)
{
	if (idx >= num_states) return 0;
	return &states[idx];
}

const ZState *ZStateFromState(lua_State *L, size_t *idx)
{
	size_t index = ZGetOptIntField(L, LUA_REGISTRYINDEX, "state_idx",num_states);
	if(idx) *idx = index;
	return ZStateFromIdx(index);
}

/*void register_zenith_state_table(lua_State *L)
{
	l_settablefield(L, -1, "mutex", register_mutex_functions);
}*/
