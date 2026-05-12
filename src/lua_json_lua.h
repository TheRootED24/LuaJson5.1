/*
 * LuaJson - a json api library for Lua
 *
 *   Copyright (C) 2026 TheRootED24 <TheRootED24@gmail.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
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
 * Lua table conversion helpers.
 *
 * These functions are registered on the `JSON` module. `parse_table` is an
 * alias for `parse_lua`; element instances expose `:totable()` and `:tolua()`.
 *
 * @classmod lua
 */

/**
 * Convert a Lua table to a LuaJson element.
 * @function JSON:parse_lua
 * @param t table Table to convert.
 * @param[opt] as string `"-a"` for array, `"-o"` for object, `"-p"` for parse mode,
 * or an option containing `"v"` for verbose output.
 * @param[opt] mixed_name string Field name used when preserving mixed-key data.
 * @param[opt] no_mixed boolean When true, ignore mixed-key fixups.
 * @return array|object Converted JSON element.
 * @usage local t = {1, 2, 3.45, "test", true}
 * local a = JSON:parse_lua(t, "-a")
 * print(a:tojson()) --> [1,2,3.45,"test",true]
 */

/**
 * Alias for @{JSON:parse_lua}.
 * @function JSON:parse_table
 * @param t table Table to convert.
 * @param[opt] as string Conversion option.
 * @param[opt] mixed_name string Field name used when preserving mixed-key data.
 * @param[opt] no_mixed boolean When true, ignore mixed-key fixups.
 * @return array|object Converted JSON element.
 */

/**
 * Serialize a Lua table or LuaJson element.
 * @function JSON:stringify_lua
 * @param value table|array|object Value to serialize.
 * @param[opt] mode string `"json"` for JSON output or `"lua"` for Lua table output.
 * @param[opt] escape boolean Escape quotes in the rendered string.
 * @return string Serialized value.
 * @usage local t = {1, 2, 3, 4, 5}
 * print(JSON:stringify_lua(t, "json")) --> [1,2,3,4,5]
 */

/**
 * Convert a LuaJson element to a Lua table.
 * @function array:totable
 * @return table Lua table copy.
 * @see object:totable
 */

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

#include "lua_json.h"

#define mixed_keys "mixed_keys"

#ifdef __cplusplus
}
#endif

typedef struct ref ref;
typedef struct json_elm json_elm;
typedef struct lua_parser lua_parser;

int lua_json_lua_stringify(lua_State *L);
int lua_json_parse_lua_object(lua_State *L, lua_parser *p);
int lua_json_parse_lua_array(lua_State *L, lua_parser *p);
int lua_json_lua_table_len(lua_State *L);
int lua_json_parse_lua(lua_State *L);

#endif
