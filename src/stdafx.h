
#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#endif

#ifndef __included_stdafx_h
#define __included_stdafx_h

#define inline _inline

#ifdef _WINDOWS
#pragma warning(disable : 4996)

#define snprintf _snprintf

#define __STDC__ 1 // Enforces ANSI C compliance.

// __STDC__ disables the following definitions in the C headers
#define strdup _strdup
#define stricmp _stricmp

#include <windows.h>
#include <conio.h>
#include <io.h>

#else

#include <pthread.h>
#endif

#include <assert.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <luajit.h>
#include <lauxlib.h>
#include <lualib.h>

//#include "particle/particle.h"

#endif // __included_stdafx_h
