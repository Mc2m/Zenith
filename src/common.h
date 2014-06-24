
#ifndef _included_zenith_common_h
#define _included_zenith_common_h

void ZPrintVar(lua_State *L,int index,unsigned char showtype);
void ZPrintTable(lua_State *L, int index);
void ZPrintStack(lua_State *L);

void ZSetBoolField(lua_State *L,int index,const char *field, unsigned char value);
void ZSetIntField(lua_State *L,int index,const char *field, int value);
void ZSetCharField(lua_State *L,int index,const char *field, const char *value);
void ZSetFunctionField(lua_State *L,int index, const char *field, int (*value)(lua_State *L));
void ZSetTableField(lua_State *L,int index, const char *field, void (*value)(lua_State *L));

unsigned char ZGetBoolField(lua_State *L,int arg,const char *field);
int ZGetIntField(lua_State *L,int arg,const char *field);
double ZGetRealField(lua_State *L,int arg,const char *field);
const char *ZGetCharField(lua_State *L,int arg,const char *field);
const char *ZGetCharFieldFull(lua_State *L,int arg,const char *field, size_t *size, const char *def);

void ZSetIntIndex(lua_State *L,int arg,int index, int value);

int ZGetIntIndex(lua_State *L,int arg,int index);

void ZGetElement(lua_State *L,const char *first_element,...);

void ZCall(lua_State *L, int narg, int numresults);
void ZLoad(lua_State *L, const char *filename);
void ZParse(lua_State *L, const char *string);

void ZSetPkgPath(lua_State *L, const char *path);
void ZAppendPkgPath(lua_State *L, const char *path);

#endif
