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
 * Ordered JSON array.
 *
 * Arrays preserve order, render directly to JSON, and use zero-based numeric
 * indexes by default. The length operator (`#a`) returns the number of
 * elements; `a:len()` returns the rendered JSON byte length.
 *
 * @classmod array
 */

/**
 * Render the array as JSON.
 * @function array:tojson
 * @param[opt] start number Start index.
 * @param[opt] finish number End index.
 * @param[opt] escape boolean Escape quotes in the returned string.
 * @return string JSON array text.
 * @usage local a = JSON:array(1, 2, 3.45, "test", true, null)
 * print(a:tojson())          --> [1,2,3.45,"test",true,null]
 * print(a:tojson(0, 2))      --> [1,2,3.45]
 * print(a:tojson(0, 2, true)) --> [1,2,3.45]
 */

/**
 * Render the array as a Lua table literal string.
 * @function array:tolua
 * @param[opt] start number Start index.
 * @param[opt] finish number End index.
 * @param[opt] escape boolean Escape quotes in the returned string.
 * @return string Lua table literal.
 */

/**
 * Convert the array to a Lua table.
 * @function array:totable
 * @return table Lua table copy.
 */

/**
 * Return the rendered JSON byte length.
 * @function array:len
 * @return number Rendered JSON length.
 */

/**
 * Return the rendered length for a specific output mode.
 * @function array:rlen
 * @param mode string `"-j"` for JSON or `"-l"` for Lua output.
 * @param[opt] escape boolean Include quote escaping in the length.
 * @return number Rendered length.
 */

/**
 * Switch numeric API calls between zero-based and one-based indexing.
 * @function array:json_base
 * @param base number Pass `0` for JSON-style zero-based indexes or `1` for Lua-style one-based indexes.
 */

/**
 * Move an element to another position.
 * @function array:move
 * @param from number Index of the element to move.
 * @param to number Destination index.
 */

/**
 * Insert an element at a position.
 * @function array:insert
 * @param pos number Index where the value should be inserted.
 * @param value any Value to insert.
 * @return number Updated element count.
 */

/**
 * Delete an element at a position.
 * @function array:del
 * @param pos number Index to remove.
 * @return number Updated element count.
 */

/**
 * Reverse all or part of the array.
 * @function array:reverse
 * @param[opt] from number Start index.
 * @param[opt] to number End index.
 */

/**
 * Append a value to the end of the array.
 * @function array:push
 * @param value any Value to append.
 * @return number Updated element count.
 */

/**
 * Remove and return the last element.
 * @function array:pop
 * @return any Removed value, or `nil` when the array is empty.
 */

/**
 * Remove and return the first element.
 * @function array:shift
 * @return any Removed value, or `nil` when the array is empty.
 */

/**
 * Add one or more values to the beginning of the array.
 * @function array:unshift
 * @param value any First value to add.
 * @param[opt] ... any Additional values.
 * @return number Updated element count.
 */

/**
 * Create a new reference to the same backing array.
 * @function array:ref
 * @return array Referenced array.
 */

/**
 * Create an independent copy of the array.
 * @function array:unref
 * @return array Copied array.
 */

// ####################################################################################### END API DOCUMENTATION ############################################################################################### //


#ifndef LUA_JSON_ARRAY_H
#define LUA_JSON_ARRAY_H

#define JSON_ARRAY_METHODS "JSON.array"

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
typedef struct json_opts opts;

extern const char *marshal_json[], *marshal_lua[], *fields[];

void lua_json_open_array(lua_State *L);
int lua_json_array(lua_State *L);
int lua_json_elm_parse_array(lua_State *L);
int lua_json_array_insert(lua_State *L);
int lua_json_elm_array_pop(lua_State *L);
int lua_json_array_move(lua_State *L);
int lua_json_array_reverse(lua_State *L);
int lua_json_elm_array_push(lua_State *L);

#endif
