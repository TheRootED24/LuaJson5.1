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

/***
 * Ordered JSON object.
 *
 * Objects preserve insertion order and allow key-based or position-based
 * access. Numeric indexes address ordered key/value pairs; string indexes
 * address named fields.
 *
 * @classmod object
 */

/**
 * Render the object as JSON.
 * @function object:tojson
 * @param[opt] start string|number Start key or ordered index.
 * @param[opt] finish string|number End key or ordered index.
 * @param[opt] escape boolean Escape quotes in the returned string.
 * @return string JSON object text.
 * @usage local obj = JSON:object("name", "test", "value", 42, "root", null)
 * print(obj:tojson())              --> {"name":"test","value":42,"root":null}
 * print(obj:tojson("name", "value")) --> {"name":"test","value":42}
 */

/**
 * Render the object as a Lua table literal string.
 * @function object:tolua
 * @param[opt] start string|number Start key or ordered index.
 * @param[opt] finish string|number End key or ordered index.
 * @param[opt] escape boolean Escape quotes in the returned string.
 * @return string Lua table literal.
 */

/**
 * Convert the object to a Lua table.
 * @function object:totable
 * @return table Lua table copy.
 */

/**
 * Return the rendered JSON byte length.
 * @function object:len
 * @return number Rendered JSON length.
 */

/**
 * Return the rendered length for a specific output mode.
 * @function object:rlen
 * @param mode string `"-j"` for JSON or `"-l"` for Lua output.
 * @param[opt] escape boolean Include quote escaping in the length.
 * @return number Rendered length.
 */

/**
 * Switch numeric API calls between zero-based and one-based indexing.
 * @function object:json_base
 * @param base number Pass `0` for JSON-style zero-based indexes or `1` for Lua-style one-based indexes.
 */

/**
 * Create an array containing this object's keys in insertion order.
 * @function object:keys
 * @return array Array of keys.
 * @usage local keys = obj:keys()
 * print(keys:tojson()) --> ["name","value","root"]
 */

/**
 * Iterate over key/value pairs until the callback returns a non-nil value.
 * @function object:foreach
 * @param fn function Called as `fn(key, value)`.
 * @return any First non-nil callback result, or no value when iteration completes.
 */

/**
 * Move an existing key/value pair to another ordered position.
 * @function object:move
 * @param from string|number Existing key or ordered index.
 * @param to string|number Destination key or ordered index.
 */

/**
 * Insert a key/value pair at an ordered position.
 * @function object:insert
 * @param at string|number Existing key or ordered index to insert before.
 * @param key string New key.
 * @param value any New value.
 * @return number Updated pair count.
 */

/**
 * Append a key/value pair to the object.
 * @function object:push
 * @param key string New key.
 * @param value any New value.
 */

/**
 * Remove and return the last value.
 * @function object:pop
 * @return any Removed value, or `nil` when the object is empty.
 */

/**
 * Remove and return the first value.
 * @function object:shift
 * @return any Removed value, or `nil` when the object is empty.
 */

/**
 * Add one or more key/value pairs to the beginning of the object.
 * @function object:unshift
 * @param key string First key.
 * @param value any First value.
 * @param[opt] ... any Additional key/value pairs.
 * @return number Updated pair count.
 */

/**
 * Reverse all or part of the ordered key list.
 * @function object:reverse
 * @param[opt] from string|number Start key or ordered index.
 * @param[opt] to string|number End key or ordered index.
 */

/**
 * Create a new reference to the same backing object.
 * @function object:ref
 * @return object Referenced object.
 */

/**
 * Create an independent copy of the object.
 * @function object:unref
 * @return object Copied object.
 */


// ####################################################################################### END API DOCUMENTATION ############################################################################################### //

#ifndef LUA_JSON_OBJECT_H
#define LUA_JSON_OBJECT_H

#define JSON_OBJECT_METHODS "JSON.object"

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

#define KEYS "keys"
typedef struct ref ref;
typedef struct json_elm json_elm;
typedef struct elm_ids elm_ids;
typedef struct json_opts opts;

extern const char *marshal_json[], *marshal_lua[], *fields[];

void lua_json_open_object(lua_State *L);
int lua_json_object(lua_State *L);
int lua_json_elm_parse_object(lua_State *L);
int lua_json_object_pop(lua_State *L);
int lua_json_object_reverse(lua_State *L);

#endif
