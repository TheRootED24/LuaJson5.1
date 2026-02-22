
#ifndef LUA_JSON_MARSHAL_H
#define LUA_JSON_MARHSAL_H

#ifndef __cplusplus
// LUA LIBS FOR gcc
#include <lua.h>                               
#include <lauxlib.h>                           
#include <lualib.h>
#endif

#ifdef __cplusplus
// LUA LIBS FOR g++
#include <lua.hpp>
extern "C" {
#endif
// STD LIBS
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
// includes
#include "includes/libjson/json.h"
#include "includes/libjson/lstr.h"

#include "lua_json_array.h"
#include "lua_json_object.h"
#include "lua_json_lua.h"

#ifdef __cplusplus
}
#endif

typedef struct ref ref;

typedef enum LUA_JSON_TYPES JSON_TYPE;
typedef enum Types Types;
typedef enum marshal_mode mode;

struct marshal_t* lua_json_marshall_new();


#endif