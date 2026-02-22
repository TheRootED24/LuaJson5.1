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

 /**
*LuaJson lua class
*/

/**
 * Convert a lua table to a lua json element.
 * @function .parse_lua
 * @param tname name of new element
 * @param t lua table
 * @return an initialized lua json array or object.
 * @usage local t = {1,2,3.45,"test",true}
 print(t) --> table: 0x5c92968f1c90
 *
 * local ta = JSON.parse_lua(ta, t)
 * print(ta) --> array: 0x5c92968f09a8
 * print(ta[0])  --> 1
 * print(ta[2])  --> 3.45
 * print(ta[#ta]) --> true
 */

 /**
 * Seriaslize a lua table into a parseable json string
 * @function .stringify_lua
 * @param tname name of new element
 * @param elm lua json element
 * @return lua table
 */
//int lua_json_lua_stringify(lua_State *L)

/**
 * Convert a lua json element to a table.
 * @function .tolua
 * @param tname name of new element
 * @param elm lua json element
 * @return lua table
 */
//int lua_json_tolua(lua_State *L)

 // ####################################################################################### END API DOCUMENTATION ############################################################################################### //

#ifndef LUA_JSON_LUA_H
#define LUA_JSON_LUA_H

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
#include "lua_json.h"

#ifdef __cplusplus
}
#endif
// max size allocated to parse a lua table
#define MAX_LUA_SIZE  1048576 // 1MB should be enough for most cases, may need fine tuining per device type
typedef struct ref ref;
typedef struct json_elm json_elm;

int lua_json_lua_parse(lua_State *L);
int lua_json_lua_stringify(lua_State *L);
int __lua_json_render_lua_object(lua_State *L, struct ref *seen);
int __lua_json_render_lua_array(lua_State *L, struct ref *seen);
int lua_json_tolua(lua_State *L);
int lua_json_lua_table_len(lua_State *L);
int lua_json_lua_is_mixed(lua_State *L);

#endif