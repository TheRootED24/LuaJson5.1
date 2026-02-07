/*
 * LuaJson - a json api library for lua
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
*LuaJson base class
@module JSON
* 
*/

/**
 * Create a new array.
 * @function .array
 * @param arr array name 
 * @param[opt] ... elements
 * @return an initialized lua json array.
 * @usage local a = JSON.array(a, 1,2,3.45,"test",true,null)
 * print(a[0])  --> 1
 * print(a[2])  --> 3.45
 * print(a[#a]) --> null
 */
// int lua_json_array_new(lua_State *L, bool parse)

/**
 * Create a new object.
 * @function .object
 * @param obj object name
 * @param[opt] ... key value pair/s
 * @return an initialized lua json object
 * @usage local o = JSON.object(o, "test","obj", "age",99, "root",null)
 * print(o.test) --> obj
 * print(o.age)  --> 99
 * print(o.root) --> null
 */
//static int lua_json_object_new (lua_State *L, bool parse)

/**
 * get a lua json elements. 
 * @function .tojson
 * @param elm a valid lua json element.
 * @return serialized lua json element.
 *
 * @usage local ta = JSON.array(ta, 1,2,3.45,true)
 * ta = JSON.tojson(ta)
 * print(ta) --> [1,2,3.45,true]
 */
//int lua_json_elm_tojson(lua_State *L)

/**
 * get a lua json elements type and memory address. 
 * @function .tostring
 * @param elm a valid lua json element.
 * @return formatted string containing element type and memory address
 */
//int lua_json_elm_tostring(lua_State *L)

/**
 * get a lua json elements size (alternative to #elm )
 * @function .size
 * @param elm a valid lua json element.
 * @return number of entries in lua json element.
 */
//int lua_json_elm_len(lua_State *L)

/**
 * Serialize a lua json element to json string.
 * @function .parse
 * @param elm lua table
 * @return a valid json element
 * @usage local a = JSON.parse('[1,2,3.45,true]')
 * print(a:tojson()) --> [1,2,3.45,true]
 * print(a[0])  --> 1
 */
//int lua_json_elm_tojson(lua_State *L)

/**
 * Serialize a lua table to json string.
 * @function .parse_lua
 * @param table lua table
 * @return a valid json representation of the table.
 * @usage local t = {1,2,3.45,"test",true}
 * print(t) --> table: 0x5c92968f1c90
 *
 * local ta = JSON.parse_lua(ta, t)
 * print(ta) --> array: 0x5c92968f09a8
 * print(ta[0])  --> 1
 * print(ta[2])  --> 3.45
 * print(ta[#ta]) --> true
 */
//int lua_json_elm_tojson(lua_State *L)

/**
 * Serialize lua json element to json string.
 * @function .stringify
 * @param elm lua json element
 * @return a valid json representation of the lua json element.
 *@usage local a = JSON.parse('[1,2,3.45,true]')
 * print(a:tojson()) --> [1,2,3.45,true]
 * local json_a = JSON.stringify(a)
 * print(json_a) --> [1,2,3.45,"test",true]
 * 
 */

/**
 * Serialize lua json object to json string.
 * @function .stringify_lua
 * @param elm lua json element
 * @return a valid json representation of the lua json element.
 * @usage local t = {1,2,3.45,"test",true}
 * print(t) --> table: 0x5c92968f1c90
 *
 * local json_a = JSON.stringify_lua(t)
 * print(json_a) --> [1,2,3.45,"test",true]
 */

// ####################################################################################### END API DOCUMENTATION ############################################################################################### //

#ifndef LUA_JSON_H
#define LUA_JSON_H

#define LUA_JSON "JSON"

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

// GLOBAL DEFINES
#define DEBUG 1

#define btoa(x) ((x) ? "true" : "false")
#define null "null"
#define LUA_TNULL (-2)

enum LUA_JSON_TYPES {
	JSON_NUMBER_TYPE,		 // 0
	JSON_BOOL_TYPE,			 // 1
	JSON_LONG_TYPE,			 // 2
	JSON_STRING_TYPE,		 // 3
	JSON_ARRAY_TYPE,		 // 4
	JSON_OBJECT_TYPE,		 // 5
	JSON_NESTED_ARRAY_TYPE,	 // 6
	JSON_NESTED_OBJECT_TYPE, // 7
	JSON_ROOT_TYPE,			 // 8
	JSON_NULL_TYPE			 // 9
};

typedef struct lua_str lua_String;

typedef struct lua_cbuffer_t{
    char* data;
    size_t size;

} lua_Cbuffer;

typedef struct {
	struct lua_str *json;
	const char *key;
	char index;
	int type;
	bool isRoot;
	double num;
	long l;
	int i;
	bool b;
	char *str;
}parse_elm;

typedef struct ref {
	int depth, max;
	uintptr_t ids[150], root;
	int elms;
	int ltype;
	bool cont, isRoot;
	int commas, vals;
	int ptype, last_stack;
	char* b;

	bool(*check_next)(struct ref*, uintptr_t);

}ref;

typedef enum {
    JsonKey,    //0
    ObjString,  //1
    ObjNumber,  //2
    ObjBool,    //3
    ObjNull,	//4
    ArrString,  //5
    ArrNumber,  //6
    ArrBool,    //7
    ArrNull,	//8
    JsonNext,   //9
    OpenObj,    //10
    CloseObj,   //11
    OpenArr,    //13
    CloseArr,   //14
    NestedObj,  //12
    NestedArr   //15
}Type;

// BASE JSON ELM CLASS
typedef struct json_elm {
	// paths
    struct json_elm *root;
    struct json_elm *self;

	int idx;
    uintptr_t id;
    
	// analytics
    size_t type;
    size_t nelms;
    size_t rlen;
	int plen, pmode;
    size_t children;

    lua_String *LS;
	
    bool isRoot;

	int nums, longs, bools, strings, arrays, objects, nulls, nestedarrays, nestedobjects;
    const char *typename, *key, *val;
	int(*newindex)(lua_State*);
	int(*index)(lua_State*);

	int(*tostring)(lua_State*);
	int(*tojson)(lua_State*);
	int(*tocstr)(lua_State*);
	int(*parse)(lua_State*);

} json_elm;

json_elm *check_json_elm(lua_State *L, int pos);
void json_type(parse_elm *elm);
int lua_json_elm_newindex(lua_State *L);
int lua_json_elm_index(lua_State *L);
int lua_json_elm_len(lua_State *L);
int lua_json_elm_size(lua_State *L);
int lua_json_elm_tostring(lua_State *L);
int lua_json_elm_tojson(lua_State *L);
int lua_json_elm_parse(lua_State *L);
int get_json_val_length(lua_State *L, json_elm *elm);
int lua_json_elm_info(lua_State *L);
bool check_next(ref *seen, uintptr_t next);

#endif