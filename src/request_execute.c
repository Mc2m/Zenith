
#include "stdafx.h"

#include "common.h"
#include "request.h"
#include "request_execute.h"
#include "pipe.h"

#include "debugging/tdebug.h"

#include "transfer.h"

#define FETCH_META 1

static int *params = 0;
static size_t offset = 0;

void tbl_transfer(lua_State *from, lua_State *to, int idx)
{
	if(idx < 0) idx--;

	lua_newtable(to);

	lua_pushnil(from);

	while (lua_next(from,idx)) {
		if(!lua_istable(from,-2) && !lua_istable(from,-1)) {
			ZTransferData(from,to,-2);
			ZTransferData(from,to,-1);

			lua_settable(to,-3);
		}

		lua_pop(from, 1);
	}

	if(params[offset++] & FETCH_META) {
		lua_getmetatable(from,idx);
		if(!lua_isnil(from,-1) && !lua_equal(from,-1,-2)) {
			ZTransferData(from,to,-1);

			lua_setmetatable(to,-2);
		}
		lua_pop(from,1);
	}
}

static inline void error(lua_State *L,const char *errormsg,...)
{
	lua_pushvalue(L,3); // copy send function
	lua_pushvalue(L,1); // copy pipe table
	lua_pushinteger(L,2); // error value
	lua_pushstring(L,errormsg);
	lua_call(L,3,0); // pipe:send()
}

static inline int check_obj(lua_State *L, int i)
{
	if(lua_isstring(L,i)) return 0;

	error(L,"Error: fetch command is not a string.");

	return 1;
}

static void execute(lua_State *L,ZRequest *r)
{
	size_t numparam;
	int status = 0;

	if(!r->waiting) {
		if(check_obj(L,5)) return;

		luaL_loadstring(L,lua_tostring(L,5));
		lua_replace(L,5);

		numparam = lua_gettop(L) - 5;
	} else {
		numparam = 0;
		r->waiting = 0;
	}

	if(lua_isstring(L,5)) {
		error(L,lua_tostring(L,5));
		return;
	}

	status = lua_resume(L,numparam);

	if(status == LUA_YIELD) r->waiting = 1;
	else if(status > LUA_YIELD) {
		printf("%s\n",lua_tostring(L,-1));
		TAbort("Crash in remote function");
		//TODO fix this mess (for some reason the stack is corrupt after lua resume if there is an error)
	} else {
		numparam = lua_gettop(L);

		if(numparam - 4) {
			size_t i = 5, j = 0, size = (numparam - 4)/2;
			lua_pushvalue(L,1); // copy pipe table
			params = size ? (int *) malloc(size) : 0;
		
			while(i <= numparam) {
				lua_pushvalue(L,i);

				if(lua_istable(L,i) && lua_isnumber(L,i+1)) {
					params[j++] = lua_tointeger(L,++i);
				}

				i++;
			}
		
			ZPipeCustomSend(L,numparam + 1, tbl_transfer);

			offset = 0;
			free(params);
		}
	}
}

void ZRExecuteInit(ZRequest *r)
{
	r->resume = execute;
}
