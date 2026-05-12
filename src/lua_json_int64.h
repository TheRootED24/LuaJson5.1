#ifndef LUA_JSON_INT64_H
#define LUA_JSON_INT64_H

#include "lua.h"
#include <stdint.h>

#include "lua.h"
#include "lauxlib.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define LUA_INT64_METATABLE "JSON.int64"

/* Structure declaration */
typedef struct {
    int64_t value;
    size_t(*length)(lua_State*);
} json_int64;

/* Public API Function Prototypes */
void luajson_register_int64(lua_State *L);
void lua_pushint64(lua_State *L, int64_t val);



#endif /* LUA_JSON_INT64_H */