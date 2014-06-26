
#include "stdafx.h"

#include "common.h"
#include "state.h"
#include "request.h"

#include "debugging/tdebug.h"
#include "structure/tarray.h"

static TArray requesttypes;

size_t ZAddRequestTemplate(ZRInitFunc init)
{
	return TArrayAppend(&requesttypes,init);
}

// ============================================================================

void ZRequestInit(ZRequest *r)
{
	r->data = 0;
	r->resume = 0;
	r->waiting = 0;
	r->clear = 0;
}

void ZRequestClear(ZRequest *r)
{
	if(r->clear) r->clear(r);
	ZRequestInit(r);
}

// ============================================================================

typedef struct _ZRequestHandler {
	lua_State *L;
	unsigned char done;
	ZRequest r;
} ZRequestHandler;

ZRequestHandler *ZRequestHandlerNew(lua_State *L, int pipeidx)
{
	ZRequestHandler *rh = (ZRequestHandler *) malloc(sizeof(ZRequestHandler));
	rh->L = lua_newthread(L);
	ZRequestInit(&rh->r);
	rh->done = 1;

	lua_pushvalue(L,pipeidx);
	lua_xmove(L,rh->L,1);

	TAssert(lua_istable(rh->L,-1));

	lua_getfield(rh->L,-1,"receive");
	TAssert(lua_isfunction(rh->L,-1));

	lua_getfield(rh->L,-2,"send");
	TAssert(lua_isfunction(rh->L,-1));

	return rh;
}

void ZRequestHandlerFree(ZRequestHandler *rh)
{
	if(rh) {
		ZRequestClear(&rh->r);
		free(rh);
	}
}

static inline void clear(ZRequestHandler *r)
{
	lua_pop(r->L,lua_gettop(r->L)-3);
	r->done = 1;
	ZRequestClear(&r->r);
}

static inline void receive(ZRequestHandler *r)
{
	lua_pushvalue(r->L,2); // copy receive function
	lua_pushvalue(r->L,1); // copy pipe table
	lua_pushnumber(r->L,0); // don't receive all
	lua_call(r->L,2,LUA_MULTRET); // pipe:receive(0)
}

static inline void error(ZRequestHandler *r,const char *errormsg,...)
{
	va_list l;

	va_start(errormsg,l);

	lua_pushvalue(r->L,3); // copy send function
	lua_pushvalue(r->L,1); // copy pipe table
	lua_pushvfstring(r->L,errormsg,l);
	lua_call(r->L,2,0); // pipe:send(errormsg)

	va_end(l);
}

static inline unsigned char initRequest(ZRequestHandler *r, size_t type)
{
	ZRInitFunc init = (ZRInitFunc) TArrayGet(&requesttypes,type);
	
	if(init) init(&r->r);
	else {
		lua_pop(r->L,lua_gettop(r->L)-3); //pop everything
		error(r,"Unknown type %ld.",type);
	}

	return init != 0;
}

void ZRequestHandlerRun(ZRequestHandler *r)
{
	if(r->done) {
		int numvalue;
		unsigned char typeiscorrect;
		
		receive(r);
		numvalue = lua_gettop(r->L);
		typeiscorrect = lua_isnumber(r->L,4);

		if(numvalue > 4 && typeiscorrect) {
			if(initRequest(r,lua_tointeger(r->L,4))) r->done = 0;
		} else {
			lua_pop(r->L,numvalue-3); //pop everything

			if(numvalue >= 4 && !typeiscorrect)
				error(r,"request type is not a number");
		}
	}

	if(r->r.resume) {
		r->r.resume(r->L,&r->r);
		if(!r->r.waiting) {
			clear(r);
			r->done = 1;
		}
	}
}

// ============================================================================

struct ZRManager {
	TArray handlers;
};

void ZRequestManagerInit(void)
{
	TArrayInit(&requesttypes,0);
}

void ZRequestManagerDestroy(void)
{
	TArrayEmptyFull(&requesttypes,0);
}

ZRManager *ZRequestManagerNew(lua_State *L)
{
	ZRManager *m = (ZRManager *) malloc(sizeof(ZRManager));
	size_t tblidx;
	
	TArrayInit(&m->handlers,0);

	//parse pipes
	ZGetElement(L,"Zenith","Pipe","pipes",0);
	tblidx = lua_gettop(L);
	lua_pushnil(L);

	while(lua_next(L,tblidx)) {
		TArrayAppend(&m->handlers,ZRequestHandlerNew(L,-2));

		lua_remove(L,-2); // remove the pipe
		lua_pushvalue(L,-2); // put the pipe name on top
		lua_remove(L,-3); // remove the pipe name from previous location
	}

	return m;
}

void ZRequestManagerFree(ZRManager *m)
{
	TArrayEmptyFull(&m->handlers,(TFreeFunc) ZRequestHandlerFree);
}

void ZRequestManagerRun(ZRManager *m)
{
	TArrayForeach(&m->handlers,(TIterFunc) ZRequestHandlerRun);
}
