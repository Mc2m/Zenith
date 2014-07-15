
#include "stdafx.h"

#include "common.h"
#include "pipe.h"
#include "request.h"
#include "request_execute.h"

#include <string.h>
#include <debugging/tlog.h>

void ZTestRequestManager(void) {
	lua_State *L1, *L2;
	ZRManager *m;
	size_t rexecidx;
	char request[512];
	

	// open the states
	L1 = lua_open();
	L2 = lua_open();

	//Create a pipe between the states
	ZPipeInitialize();
	ZPipeCreate(L1,L2,"test");

	//initialize the request manager
	ZRequestManagerInit();

	// setup execute request
	rexecidx = ZAddRequestTemplate(ZRExecuteInit);

	// create new manager
	m = ZRequestManagerNew(L2);

	// set up a test value
	LParse(L2,"testvar = 'bleh'");

	//test request
	snprintf(request,sizeof(request),"local p = Zenith.Pipe.pipes.test p:send(%d,'return testvar')",rexecidx);
	LParse(L1,request);

	ZRequestManagerRun(m);

	snprintf(request,sizeof(request),"local p = Zenith.Pipe.pipes.test return p:receive()");
	LParse(L1,request);

	if(!lua_isstring(L1,-1)) {
		TLogReport(T_LOG_WARNING,"ZTestRequestManager","The request didn't return the proper value");
	} else {
		const char *result;
		result = lua_tostring(L1,-1);
		if(strcmp(result,"bleh"))
			TLogReport(T_LOG_WARNING,"ZTestRequestManager","The request didn't return the proper value");
	}

	// free the manager
	ZRequestManagerFree(m);

	// destroy everything
	ZRequestManagerDestroy();
	ZPipeDestroy();

	lua_close(L1);
	lua_close(L2);
}
