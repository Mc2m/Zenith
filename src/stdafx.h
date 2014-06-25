
#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#endif

#ifndef __included_stdafx_h
#define __included_stdafx_h

#ifndef __STDC__
#define inline _inline
#endif

#ifdef _WINDOWS
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)

#define snprintf _snprintf

#define __STDC__ 1 // Enforces ANSI C compliance.

// __STDC__ disables the following definitions in the C headers
#define strdup _strdup
#define stricmp _stricmp
#endif

#include <stdlib.h>
#include <stdio.h>

#include <luajit.h>
#include <lauxlib.h>
#include <lualib.h>

#endif // __included_stdafx_h
