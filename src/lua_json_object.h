/*
 * LuaJson - a json api library for Lua
 *
 *   Copyright (C) 2026 TheRootED24 <TheRootED24@gmail.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in JSONliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
 // ####################################################################################### API DOCUMENTATION ################################################################################################### //

 /***
*JSON.object Class
@classmod object
*/

/**
 * render an object as valid json
 * @function object:tojson
 * @return json string 
 * 
 *@usage print(obj:tojson()) --> {"test":"obj","age":99,"root":null}
 */
//static int lua_json_object_tojson(lua_State *L) 

/**
 * Create a new array containing the keys of an object
 * @function object:keys
 * @return json array of object keys 
 * 
 *@usage local keys = o:keys() 
 *print(keys:tojson())--> ["test","age","root"]
 */
//static int lua_json_object_keys(lua_State *L) 

// ####################################################################################### END API DOCUMENTATION ############################################################################################### //

#ifndef LUA_JSON_OBJECT_H
#define LUA_JSON_OBJECT_H

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

#include "lua_json.h"

#ifdef __cplusplus
}
#endif

typedef struct ref ref;
typedef struct json_elm json_elm;

typedef struct elm_ids elm_ids;

extern const char *marshal_json[], *marshal_lua[], *fields[];
void lua_json_open_object(lua_State *L);
//int lua_json_object_newindex(lua_State *L);
//int lua_json_object_index(lua_State *L);
int lua_json_object(lua_State *L);
int lua_json_elm_parse_object(lua_State *L);
int __lua_json_render_elm_object(lua_State *L, ref *seen);
int _lua_json_elm_key_to_idx(lua_State *L, const char *key, bool add);
int _lua_json_elm_idx_to_key(lua_State *L, int idx) ;
#endif