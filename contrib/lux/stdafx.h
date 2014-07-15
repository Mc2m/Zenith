#ifndef _included_lux_stdafx_h
#define _included_lux_stdafx_h

#if defined(_WIN32) || defined(_WIN64)
#define snprintf _snprintf
#endif

#if _MSC_VER >= 1400
#define __STDC__ 1  // Enforces ANSI C compliance.

// __STDC__ disables the following definitions in the C headers
#define strdup _strdup
#define stricmp _stricmp
#endif

#define UNREFERENCED_PARAMETER(x) x

#include <stdlib.h>
#include <stdio.h>

#endif
