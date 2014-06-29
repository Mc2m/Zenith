
#include "stdafx.h"

#include "common.h"
#include "request.h"
#include "request_execute.h"
#include "pipe.h"

#include "debugging/tdebug.h"

#include "transfer.h"

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
			int type = lua_tointeger(L,4);
			lua_pushvalue(L,1); // copy pipe table
			lua_replace(L,4); // put it in place of the type
		
			ZPipeSend(L,4);

			lua_pushinteger(L,type); // put back the type
		}
	}
}

void ZRExecuteInit(ZRequest *r)
{
	r->resume = execute;
}
