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

#ifdef __cplusplus
}
#endif

// GLOBAL DEFINES
#define DEBUG 1

#define btoa(x) ((x) ? "true" : "false")
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

	int vtable;
    uintptr_t id;
    
	// analytics
    size_t type;
    size_t nelms;
    size_t rlen;
	size_t plen;
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
int lua_json_elm_tostring(lua_State *L);
int lua_json_elm_tojson(lua_State *L);
int lua_json_elm_parse(lua_State *L);
int get_json_val_length(lua_State *L, json_elm *elm);
int test_func(lua_State *L);
bool check_next(ref *seen, uintptr_t next);

#endif