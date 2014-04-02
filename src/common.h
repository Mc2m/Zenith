
#ifndef _included_zenith_common_h
#define _included_zenith_common_h

void l_print_var(lua_State *L,int index,unsigned char showtype);
void l_print_table(lua_State *L, int index);
void l_print_stack(lua_State *L);

void l_setboolfield (lua_State *L,int index,const char *field, unsigned char value);
void l_setintfield (lua_State *L,int index,const char *field, int value);
void l_setcharfield (lua_State *L,int index,const char *field, const char *value);
void l_setfunctionfield (lua_State *L,int index, const char *field, int (*value)(lua_State *L));
void l_settablefield (lua_State *L,int index, const char *field, void (*value)(lua_State *L));

unsigned char l_getboolfield(lua_State *L,int arg,const char *index);
int l_getintfield (lua_State *L,int arg,const char *index);
double l_getrealfield (lua_State *L,int arg,const char *index);
const char *l_getcharfield (lua_State *L,int arg,const char *index);
const char *l_getoptcharfield (lua_State *L,int arg,const char *index);

void l_setintindex(lua_State *L,int arg,int index, int value);

int l_getintindex(lua_State *L,int arg,int index);

void l_getElement(lua_State *L,const char *first_element,...);

void l_call (lua_State *L, int narg, int numresults);
void l_load (lua_State *L, const char *filename);
void l_parse (lua_State *L, const char *string);

void l_setpkgpath(lua_State *L, const char *path);
void l_appendpkgpath(lua_State *L, const char *path);

#endif
