
#ifndef __included_stdafx_h
#define __included_stdafx_h

#undef __cplusplus

#if defined(_WIN32) || defined(_WIN64)
#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable : 4996)
#endif

#include <stdlib.h>
#include <stdio.h>

#include <tdefine.h>

#include <luajit.h>
#include <lauxlib.h>
#include <lualib.h>

#endif // __included_stdafx_h
