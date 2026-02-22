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
*JSON.array Class
@classmod array
*/

/**
 * render an array as valid json
 * @function array:tojson
 * @return json string 
 * @see JSON:array
 * @usage print(a:tojson()) --> [1,2,3.45,"test",true,null]
 */
// static int lua_json_array_tojson(lua_State *L)

/**
 * Inserts an element at position. 
 * @function array:insert
 * @param pos a valid index to insert item.
 * @param elm any
 * @return  updated length of array.
 * @usage local a = JSON:array(1,2,3.45,"test",true,null)
 * print(a:tojson()) --> [1,2,3.45,"test",true,null]
 * a:insert(0,0)
 * print(a:tojson()) --> [0,1,2,3.45,"test",true,null]
 */
//int lua_json_array_insert(lua_State *L)

/**
 * Deletes an element at position. 
 * @function array:del
 * @param pos index of item to be removed.
 * @return  the updated length of array.
 * @usage local a = JSON:array(0,1,2,3.45,"test",true,null)
 * print(a:tojson()) --> [0,1,2,3.45,"test",true,null]
 * print(a:len()) --> 30
 * print(#a) --> 7
 * local n = a:del(4)
 * print(a:tojson()) --> [0,1,2,3.45,true,null]
 * print(n)  --> 6
 * print(#a) --> 6
 * print(a:len()) --> 23
 */
//static int lua_json_elm_array_del(lua_State *L) 

/**
 * reverses the indexes of the elements of an array.  
 * @function array:reverse
 * @return 1 on success, 0 on failure.
 * @usage local a = JSON:array(0,1,2,3.45,"test",true,null)
 * print(a:tojson()) -->  [0,1,2,3.45,"test",true,null]
 * a:reverse()
 * print(a:tojson()) --> [null,true,"test",3.45,2,1,0]
 */
//static int lua_json_elm_array_reverse(lua_State *L)

/**
 * Adds element to the end of an array. 
 * @function array:push
 * @param elm Any
 * @return the updated length of array.
 * @usage local a = JSON:array(0,1,2,3.45,true,null)
 * print(a:tojson()) --> [0,1,2,3.45,true,null]
 * print(a:len()) --> 23
 * print(#a) --> 6
 * local n = a:push("test")
 * print(a:tojson()) --> [0,1,2,3.45,true,null,"test"]
 * print(n)  --> 7
 * print(#a) --> 7
 * print(a:len()) --> 30
 */
//static int lua_json_elm_array_push(lua_State *L)

/**
 * Pops and returns the last element of an array. 
 * @function array:pop
 * @return the popped element or nil if array is empty.
 * @usage local a = JSON:array(0,1,2,3.45,true,null)
 * print(a:tojson()) --> [0,1,2,3.45,true,null]
 * print(a:len()) --> 23
 * print(#a) --> 6
 * local n = a:pop()
 * print(a:tojson()) --> [0,1,2,3.45,true]
 * print(n)  --> 5
 * print(#a) --> 5
 * print(a:len()) --> 11
 */
//int lua_json_elm_array_pop(lua_State *L)

/**
 * Removes element at index 0. 
 * @function array:shift
 * @return the removed element or nil if array is empty.
 * @usage local a = JSON:array(0,1,2,3.45,true,null)
 * print(a:tojson()) --> [0,1,2,3.45,true,null]
 * print(a:len()) --> 23
 * print(#a) --> 6
 * local n = a:shift()
 * print(a:tojson()) --> [1,2,3.45,true,null]
 * print(n)  --> 0
 * print(#a) --> 5
 * print(a:len()) --> 21
 */
//static int lua_json_elm_array_shift(lua_State *L) 

/**
 * adds one or more elements to the beginning of an array.
 * @function array:unshift
 * @param elm Any
 * @param[opt] ...  Any
 * @return the updated length of the array.
 *  @usage local a = JSON:array(0,1,2,3.45,true,null)
 * print(a:tojson()) --> [0,1,2,3.45,true,null]
 * print(a:len()) --> 23
 * print(#a) --> 6
 * local n = a:unshift(9,8,7)
 * print(a:tojson()) --> [9,8,7,0,1,2,3.45,true,null]
 * print(n)  --> 9
 * print(#a) --> 9
 * print(a:len()) --> 29
 */
//static int _lua_json_elm_array_unshift(lua_State *L)

/**
 * Creates a new reference to an existing array. (pass by reference)
 * @function array:ref
 * @return reference to array.
 * @usage local a_ref = a:ref()
 * print(a_ref[0]) --> 1
 * print(a_ref[#a_ref - 1]) --> true
 * a_ref[0] = 20
 * print(a_ref[0], a[0]) --> 20    20
 * @see JSON.array
 * @see array:unref
 */
//static int lua_json_array_ref(lua_State *L)

/**
 * Creates an unreferenced copy of an existing array. (pass by value) 
 * @function array:unref
 * @return unreference an array. (create a seperate copy)
 * @usage local a_copy = a:unref()
 * print(a_copy[0]) --> 1
 * a_copy[0] = 20
 * print(a_copy[0], a[0]) --> 20    1
 * @see JSON.array
 * @see array:ref
 */
//int _lua_json_array_unref(lua_State *L)

// ####################################################################################### END API DOCUMENTATION ############################################################################################### //


#ifndef LUA_JSON_ARRAY_H
#define LUA_JSON_ARRAY_H

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

typedef struct ref ref;
typedef struct json_elm json_elm;
extern const char *marshal_json[], *marshal_lua[], *fields[];

void lua_json_open_array(lua_State *L);
//int lua_json_array_newindex(lua_State *L);
//int lua_json_array_index(lua_State *L);
int lua_json_array(lua_State *L);
int lua_json_elm_parse_array(lua_State *L);
int __lua_json_render_elm_array(lua_State *L, ref *seen);
int lua_json_array_insert(lua_State *L);
int lua_json_elm_array_pop(lua_State *L);

#endif