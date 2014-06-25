
#include "stdafx.h"

#include "state_test.h"
#include "transfer_test.h"
#include "pipe_test.h"
#include "request_manager_test.h"

#include <debugging/tlog.h>

int main(int argc, char **argv)
{
	TLogSetFile(stdout);

	TLogReport(T_LOG_PROGRESS,"main","Running tests...");

	ZTestState();
	ZTestTransfer();
	ZTestPipe();
	ZTestRequestManager();

	TLogReport(T_LOG_PROGRESS,"main","Tests have been completed.");

	return 0;
}
