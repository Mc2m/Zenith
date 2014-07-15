#ifndef _included_lux_common_h
#define _included_lux_common_h

#include <luajit.h>
#include <lauxlib.h>
#include <lualib.h>

typedef void (*LLVoidFunc)(lua_State *);
typedef int (*LLIntFunc)(lua_State *);

void LPrintVar(lua_State *L,int index,unsigned char showtype);
void LPrintTable(lua_State *L, int index);
void LPrintStack(lua_State *L);

void LSetBoolField(lua_State *L,int index,const char *field, unsigned char value);
void LSetIntField(lua_State *L,int index,const char *field, int value);
void LSetCharField(lua_State *L,int index,const char *field, const char *value);
void LSetFunctionField(lua_State *L,int index, const char *field, LLIntFunc value);
void LSetTableField(lua_State *L,int index, const char *field, LLVoidFunc value);

unsigned char LGetBoolField(lua_State *L,int arg,const char *field);
int LGetIntField(lua_State *L,int arg,const char *field);
int LGetOptIntField(lua_State *L,int arg,const char *field, int def);
double LGetRealField(lua_State *L,int arg,const char *field);
const char *LGetCharField(lua_State *L,int arg,const char *field);
const char *LGetCharFieldFull(lua_State *L,int arg,const char *field, size_t *size, const char *def);

void LSetIntIndex(lua_State *L,int arg,int index, int value);

int LGetIntIndex(lua_State *L,int arg,int index);

void LGetElement(lua_State *L,const char *first_element,...);

void LSetReportFunc(LLVoidFunc reportfunc);

void LCall(lua_State *L, int narg, int numresults);
void LLoad(lua_State *L, const char *filename);
void LParse(lua_State *L, const char *string);

void LSetPkgPath(lua_State *L, const char *path);
void LAppendPkgPath(lua_State *L, const char *path);

#endif
