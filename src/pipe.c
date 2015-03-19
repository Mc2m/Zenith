#include "stdafx.h"

#include "pipe.h"

#include "state.h"

#include "transfer.h"

#include <structure/tarray.h>
#include <tthread.h>

#define PIPE_SEND_DATA_LIMIT 1000
static const char ZPIPEMETA[] = "ZENITH_PIPE_META";

typedef struct _ZPipeData {
	TCV *v;
	lua_State *waiting;
	int firstIndex, lastIndex;
} ZPipeData;

static ZPipeData *ZPipeDataNew(void)
{
	ZPipeData *p = (ZPipeData *) malloc(sizeof(ZPipeData));

	p->v = 0;
	p->waiting = 0;
	p->firstIndex = 1;
	p->lastIndex = 1;

	return p;
}

static void ZPipeDataFree(ZPipeData *p)
{
	if(p) {
		TCVFree(p->v);
		free(p);
	}
}

static struct ZPipes {
	lua_State *transfer;
	TMutex *m;
	TArray *data;
	size_t nextID;
} zenithPipes = {0};

void ZPipeInitialize(void)
{
	zenithPipes.nextID = 1;
	zenithPipes.transfer = lua_open();
	zenithPipes.m = TMutexNew(T_MUTEX_RECURSIVE);

	luaL_openlibs(zenithPipes.transfer);

	lua_newtable(zenithPipes.transfer); // table containing pipes
	LGetElement(zenithPipes.transfer,"table","remove",0); // keep table.remove on top of the stack

	zenithPipes.data = TArrayNew(0);
}

void ZPipeDestroy(void)
{
	if (zenithPipes.transfer) {
		lua_close(zenithPipes.transfer);
		TMutexFree(zenithPipes.m);
		zenithPipes.transfer = 0;
		zenithPipes.m = 0;

		TArrayFree(zenithPipes.data,(TFreeFunc)ZPipeDataFree);
		zenithPipes.data = 0;
	}
}

static lua_State *ZPipeFetchTransferState(void) {
	TMutexLock(zenithPipes.m);

	return zenithPipes.transfer;
}

static void ZPipeReleaseTransferState(lua_State * L) {
	TMutexUnlock(zenithPipes.m);
}

static inline unsigned char ZPipeGetPipe(lua_State *L, size_t id) {
	char valid;
	lua_checkstack(L,2);

	lua_pushinteger(L,id);
	lua_gettable(L,1);

	valid = lua_istable(L,-1);
	if(!valid) lua_pop(L,1);

	return !valid;
}

static inline void ZPipeWait(ZPipeData *d, lua_State *L, size_t delay)
{
	TMutexLock(zenithPipes.m);
	d->waiting = L;

	if(!d->v) d->v = TCVNew(zenithPipes.m);
	else TCVWake(d->v);

	TCVSleep(d->v,delay);

	if(d->waiting == L) d->waiting = 0;

	TMutexUnlock(zenithPipes.m);
}

static inline int ZPipeSendInternal(lua_State *L, int idx)
{
	// Stack
	//
	// idx - pipe
	// idx + 1 - delay
	// idx + x - data

	
	size_t id;
	ZPipeData *d;

	int i;
	int numdata;
	int delay;

	// use positive id for the index
	if(idx < 0) idx += lua_gettop(L)+1;

	//check the stack
	if(lua_gettop(L) < idx+2 || !lua_istable(L,idx) || (!lua_isnumber(L,idx+1) && !lua_isnil(L,idx+1))) {
		lua_pushstring(L,"invalid function parameter, you should have pipe:send(delay,...)");
		return 1;
	}

	// get pipe id
	id = LGetIntField(L,idx,"id");
	// get internal data
	d = (ZPipeData *) TArrayGet(zenithPipes.data,id-1);

	{
		i = idx + 2; // first data index in stack
		numdata = lua_gettop(L) - idx - 1; // amount of data
		delay = luaL_optinteger(L,idx + 1, 0);

		// Prevent pipe flooding
		if(i > PIPE_SEND_DATA_LIMIT)
			luaL_error(L,"Too much data in storage, limit is %d. Wait for receiver to consume.", PIPE_SEND_DATA_LIMIT);
	
		// Prevent pipe flooding
		if(numdata < 1) {
			lua_pushstring(L,"There is no data in the pipe. No transfer has been done.");
			return 1;
		}
	}

	if(d->waiting) {
		// the other end is listening, send the data directly
		TMutexLock(zenithPipes.m);
		ZTransferRange(L,d->waiting,i,numdata);
		TMutexUnlock(zenithPipes.m);
		//printf("sent\n");
	} else {
		int j;

		//get current pipe from the transfer state
		lua_State *transfer = ZPipeFetchTransferState();
		if(ZPipeGetPipe(transfer, id)) {
			ZPipeReleaseTransferState(transfer);
			luaL_error(L,"Internal Error.");
			return 0;
		}

		//place the data in a table
		lua_pushinteger(transfer,d->lastIndex++);
		lua_createtable(transfer,numdata,0);

		//transfer the data
		for(j = 1; j <= numdata; ++i, ++j) {
			lua_pushnumber(transfer,j);
			ZTransferData(L,transfer,i);
			lua_settable(transfer,-3);
		}

		lua_settable(transfer,-3);

		lua_pop(transfer,1); // pop the pipe

		ZPipeReleaseTransferState(transfer);
		//printf("stored\n");
	}

	if(d->v) TCVWake(d->v);

	if(delay) {
		int top = lua_gettop(L);
		// wait for an answer
		ZPipeWait(d,L,delay);
		return lua_gettop(L) - top;
	}

	return 0;
}

static inline int ZPipeTransferDataTo(lua_State *from, lua_State *to, ZPipeData *d)
{
	int numtransfered = 0;
	int i, numData;

	//fetch the table from firstIndex
	lua_pushinteger(from,d->firstIndex);
	lua_gettable(from,-2);

	numData = lua_objlen(from,-1);
	if(!lua_checkstack(to,numData)) {
		// not enough space. stop this
		lua_pop(from,1);
		return -1;
	}

	//transfer
	for(i=1; i <= numData; ++i) {
		//fetch the data from the table
		lua_pushinteger(from,i);
		lua_gettable(from,-2);

		//transfer it
		ZTransferData(from,to,-1);
		lua_pop(from,1);
		numtransfered++;
	}

	//remove that table
	lua_pop(from,1);
	lua_pushinteger(from,d->firstIndex);
	lua_pushnil(from);
	lua_settable(from,-3);

	return numtransfered;
}

static inline int ZPipeReceiveInternal(lua_State *L, int idx)
{
	// Stack
	//
	// idx - pipe
	// idx + 1 - timeout
	// idx + 2 - receive all

	size_t id;
	ZPipeData *d;

	char allData;
	int numtransfered = 0;
	lua_State *transfer;

	// use positive id for the index
	if(idx < 0) idx += lua_gettop(L)+1;

	//check the stack
	if(lua_gettop(L) < idx+1 || !lua_istable(L,idx) || !lua_isnumber(L,idx+1)) {
		lua_pushstring(L,"invalid function parameter, you should have pipe:receive(timeout,...)");
		return 1;
	}

	// get pipe id
	id = LGetIntField(L,idx,"id");
	// get internal data
	d = (ZPipeData *) TArrayGet(zenithPipes.data,id-1);

	allData = lua_isboolean(L,idx+2) ? lua_toboolean(L,idx+2) : luaL_optint(L,idx + 2,1);

	transfer = ZPipeFetchTransferState();
	if(d->lastIndex > 1) {
		//fetch the pipe data
		if(ZPipeGetPipe(transfer, id)) {
			ZPipeReleaseTransferState(transfer);
			luaL_error(L,"Internal Error.");
			return 0;
		}

		do {
			int transfered = ZPipeTransferDataTo(transfer,L,d);
			if(transfered == -1) break;
			numtransfered += transfered;
			d->firstIndex++;
		} while(allData && d->firstIndex != d->lastIndex);

		//clean up
		lua_pop(transfer,1);

		if(d->firstIndex == d->lastIndex) {
			// reset the indexes
			d->firstIndex = d->lastIndex = 1;
		}
		ZPipeReleaseTransferState(transfer);
	} else {
		ZPipeReleaseTransferState(transfer);
		// no data in this pipe
		// do we have to wait ?
		int delay = lua_tointeger(L, idx + 1);
		int top = lua_gettop(L);

		if(delay) ZPipeWait(d,L,delay);

		numtransfered = lua_gettop(L) - top;
		if(numtransfered < 0) numtransfered = 0;
	}

	return numtransfered;
}

int ZPipeSend(lua_State *L, int idx)
{
	int data;
	if(zenithPipes.transfer) {
		//transfer
		data = ZPipeSendInternal(L,idx);

		//consume the elements
		if(idx < 0) idx += lua_gettop(L)+1;
		if(data) {
			int size = lua_gettop(L) -1;
			for(; size >= idx; size--)
				lua_remove(L,size);
		} else {
			lua_pop(L,lua_gettop(L) - idx + 1);
		}
	}

	return data;
}

static int ZPipeLuaSend(lua_State *L)
{
	int data;
	if(zenithPipes.transfer) {
		//transfer
		data = ZPipeSendInternal(L,1);
	}
	return data;
}

int ZPipeReceive(lua_State *L, int idx)
{
	if(zenithPipes.transfer) {
		int top = lua_gettop(L);
		int size = ZPipeReceiveInternal(L,idx);

		if(idx < 0) idx += lua_gettop(L)+1;
		for(; top >= idx; top--)
			lua_remove(L,top);

		return size;
	}

	return 0;
}

static int ZPipeLuaReceive(lua_State *L)
{
	if(zenithPipes.transfer)
		return ZPipeReceiveInternal(L,1);

	return 0;
}

void ZPipeSetState(lua_State *L, const char *name)
{
	lua_getfield(L,-1,"Access");
	lua_getfield(L,-2,"Pipes");

	// STACK
	// -1 pipes table
	// -2 access table
	// -3 Pipe table
	
	// create pipe
	lua_createtable(L,0,1);
	LSetIntField(L,-1,"id",zenithPipes.nextID);

	lua_pushvalue(L,-1);
	lua_setfield(L,-2,"__index");

	//set access as metatable
	lua_pushvalue(L,-3);
	lua_setmetatable(L,-2);

	//push to pipes table
	lua_setfield(L,-2,name);

	lua_pop(L,3);
}

static void ZPipeCreateInternal(void)
{
	TMutexLock(zenithPipes.m);
	lua_pushinteger(zenithPipes.transfer,zenithPipes.nextID);
	lua_newtable(zenithPipes.transfer);
	lua_settable(zenithPipes.transfer,1);

	TArrayPush(zenithPipes.data,ZPipeDataNew());
	TMutexUnlock(zenithPipes.m);
}

void ZPipeCreate(const char *name, lua_State *L1, lua_State *L2)
{
	char namerplcmnt[64];
	if(L1 == L2) return;

	LGetElement(L1,"Zenith","Pipe",NULL);
	if(!lua_istable(L1,-1)) return;

	LGetElement(L2,"Zenith","Pipe",NULL);
	if(!lua_istable(L2,-1)) return;

	if(!name) {
		snprintf(namerplcmnt,sizeof(namerplcmnt),"pipe%d",zenithPipes.nextID);
		name = namerplcmnt;
	}

	//set pipe
	ZPipeCreateInternal();

	ZPipeSetState(L1,name);
	ZPipeSetState(L2,name);

	zenithPipes.nextID++;
}

static const struct luaL_Reg ZPipeAccessTableFunctions[] = {
	{"send",ZPipeLuaSend},
	{"receive",ZPipeLuaReceive},
	{NULL,NULL}
};

void ZPipeRegister(lua_State *L)
{
	//this supposes that Zenith is on top
	lua_newtable(L);
	lua_setfield(L, -2, "Pipes");

	//create access table
	lua_createtable(L,0,2);
	luaL_openlib(L, 0, ZPipeAccessTableFunctions, 0);

	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	lua_setfield(L, -2, "Access");
}
