
#ifndef LTASK_SERIALIZE_H
#define LTASK_SERIALIZE_H

#include <lemon/config.h>


typedef struct lua_State lua_State;

EXTERN_C int seri_unpack(lua_State *L);
EXTERN_C int seri_pack(lua_State *L);



#endif