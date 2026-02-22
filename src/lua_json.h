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
*Base Class for all JSON elements
@module JSON
* 
*/

/***
 * Introduct
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
 * print(a[#a-1]) --> null
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
 * Convert a lua json element to a table.
 * @function .tolua
 * @param tname name of new element
 * @param elm lua json element
 * @return lua table
 */
//int lua_json_tolua(lua_State *L)

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
#include "lua_json_marshal.h"
#include "lua_json_lua.h"
#include "lua_json_elm_event.h"



#ifdef __cplusplus
}
#endif

#if defined(__GNUC__) || defined(__clang__)
    #define PACKED __attribute__((packed))
#elif defined(_MSC_VER)
    // Microsoft Visual C++ compiler uses __pragma(pack(1))
    // This approach might require using pragmas around the struct definition
    // or using the /Zp compiler flag, as __declspec(packed) is not a direct equivalent.
    // A common non-standard pragma alternative is:
    #define PACKED
    #define PACK_START  __pragma(pack(push, 1))
    #define PACK_END    __pragma(pack(pop))
#else
    // Default case for compilers that might not support it
    #define PACKED
#endif

extern const char *marshal_json[], *marshal_lua[];
//typedef void (*NotifyFn)(void* context, size_t value);
typedef void (*NotifyFn)(void* context, size_t rlen, size_t quoted, size_t nkeys);
// GLOBAL DEFINES
#define DEBUG 0
//#define USE_THREADS

// --- Optional Threading Support ---
#ifdef USE_THREADS
    #include <threads.h>
    #define LOCK(s)   mtx_lock(&(s)->lock)
    #define UNLOCK(s) mtx_unlock(&(s)->lock)
    #define INIT_LOCK(s) (mtx_init(&(s)->lock, mtx_plain | mtx_recursive) == thrd_success)
    #define DESTROY_LOCK(s) mtx_destroy(&(s)->lock)
#else
    #define LOCK(s)   ((void)0)
    #define UNLOCK(s) ((void)0)
    #define INIT_LOCK(s) (1)
    #define DESTROY_LOCK(s) ((void)0)
#endif

#define btoa(x) ((x) ? "true" : "false")
#define null "null"
#define LUA_TNULL (-2)

#define ADD_VAL 1
#define SUB_VAL (-1)
#define EVAL_VAL 0

#define MARSHAL_JSON 	0
#define MARSHAL_LUA 	1
#define MARSHAL_ESC		2

typedef enum {
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
}LUA_JSON_TYPES;

typedef struct lua_str lua_String;
typedef struct Subject Subject;

typedef struct lua_cbuffer_t{
    char* data;
    size_t size;

} lua_Cbuffer;


typedef union {
	int num;
	const char *key;
}env_val;

struct s_parse_elm{
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
};
typedef struct s_parse_elm parse_elm;

typedef enum {
    ObjKey,     	//0
    ObjString,  	//1
    ObjNumber,  	//2
    ObjBool,    	//3
    ObjNull,		//4
    ArrString,  	//5
    ArrNumber,  	//6
    ArrBool,    	//7
    ArrNull,		//8
    Next,   		//9
    OpenObj,    	//10
    CloseObj,   	//11
    OpenArr,    	//12
    CloseArr,   	//13
    EscObjString,  	//14
    EscArrString,   //15
	EscObjKey,    	//16
    EscObjNumber, 	//17
    EscObjBool,		//18
    EscObjNull		//19
}Type;


// Marshal
typedef struct marshal_t {
	json_elm *elm;
	uint8_t mtype;
	bool escape;
	
	const char **mode;
	int(*set_mode)(lua_State*);

	int(*next)	(ref*);

	int(*obj_open)	(ref*);
	int(*obj_close)	(ref*);
	
	int(*obj_key)	(lua_State*, json_elm*, ref*);
	int(*obj_string)(lua_State*, json_elm*, ref*);
	int(*obj_number)(lua_State*, json_elm*, ref*);
	int(*obj_bool)	(lua_State*, json_elm*, ref*);

	int(*arr_open)	(ref*);
	int(*arr_close)	(ref*);

	int(*arr_string)(lua_State*, json_elm*, ref*);
	int(*arr_number)(lua_State*, ref*);
	int(*arr_bool)	(lua_State*, ref*);
	
} marshal;

typedef struct ref {
	int depth, max;
	json_elm *elm, *nested;
	marshal *marshal;
	uintptr_t root, last, next;
	uint8_t mode;
	size_t rlen;
	int elms;
	int ltype;
	bool isRoot, escape;
	//int commas, vals;
	int ptype;
	char* b;
	const char **Marshal;
	bool(*check_next)(struct ref*, uintptr_t);

}ref;


typedef struct lua_parser {
	marshal *marshal;
	lua_State *state;
	uint8_t parent_type;
	uint8_t lua_type, json_type;
	uint16_t nelms, nkeys, idxs;
	bool nest;
	
	int(*type_size) (lua_State *L, uint8_t pos);
	int(*needs_nest)(lua_State *L, uint8_t pos);
	int(*render)	(lua_State*, ref*);

} lua_parser;


typedef struct elm_events {
	json_elm *root;
	struct Subject *on_newindex, *on_change;
	int (*init)		(Subject *s);
	void (*sub)		(Subject *s, void *ctx, NotifyFn fn);
	void (*unsub)	(Subject *s, void *ctx, NotifyFn fn);
	void (*set)	(Subject *s, size_t rlen, size_t quoted, size_t nkeys);
	void (*get)		(Subject *s, size_t rlen, size_t quoted, size_t nkeys);
	void (*cleanup)	(Subject *s);
}elm_events;

// BASE JSON ELM CLASS
struct json_elm {
    struct json_elm *root, *nested;
	// events
	elm_events *event;

	int vtable, idx;
    uintptr_t id;
	uint8_t mode; // marshall mode
	size_t recurs;
    
	// analytics
    size_t type;
    size_t nelms;
    size_t rlen, klen, vlen, quoted, nkeys;
	int plen, ktype, vtype;
    size_t children;
    //lua_String *LS;
    bool isRoot, is_nil, c_out, escape;
	//char* rbuf;
	uint16_t nums, longs, bools, strings, arrays, objects, nulls, nestedarrays, nestedobjects;
    const char *typename, *key, *val, *sep;


	int(*key_to_idx)(lua_State*, json_elm*, bool);
	bool(*idx_to_key)(lua_State*, json_elm*);
	int(*tostring)	(lua_State*);
	int(*stringify)	(lua_State*);
	int(*parse)		(lua_State*);

	// recursion call
	int(*render)	(lua_State*, ref*);

};
typedef struct json_elm json_elm;

typedef enum {
	children,
	ids,
	keys,
	klens,
	vlens,
	vtypes
}env_field;

typedef struct elm_env {
	json_elm *elm;
	env_field field;
	env_val val;
	int(*add)	(lua_State*, json_elm*, env_val*, env_field);
	int(*rem)	(lua_State*, json_elm*, int, env_field);
	int(*insert)(lua_State*, json_elm*, int, env_val*, env_field);
	int(*get)	(lua_State*, json_elm*, int, env_field);
} elm_env;

json_elm *check_json_elm(lua_State *L, int pos);
void json_type(parse_elm *elm);
int lua_json_elm_newindex(lua_State *L);
int lua_json_elm_index(lua_State *L);
int lua_json_elm_len(lua_State *L);
int lua_json_elm_size(lua_State *L);
int lua_json_elm_tostring(lua_State *L);

int lua_json_elm_stringify(lua_State *L);
int lua_json_elm_parse(lua_State *L);
int get_json_val_length(lua_State *L, json_elm *elm);
int lua_json_elm_info(lua_State *L);
bool check_next(ref *seen, uintptr_t next);
int lua_json_elm_get_rlen(lua_State *L);

int lua_json_elm_add_id(lua_State *L, json_elm *elm, int id);
int lua_json_elm_insert_id(lua_State *L, json_elm *elm, int idx, int id);
//int lua_json_elm_env_add(lua_State *L, json_elm *elm, void *val, env_field field);
int lua_json_elm_env_add(lua_State *L, json_elm *elm, env_val *val, env_field field);
int lua_json_elm_env_insert(lua_State *L, json_elm *elm, int idx, env_val *val, env_field field);

int lua_json_elm_rem_id(lua_State *L, json_elm *elm, int idx);
int lua_json_elm_get_id(lua_State *L, json_elm *elm , int idx);
int lua_json_elm_env_rem(lua_State *L, json_elm *elm, int idx, env_field field);
int lua_json_elm_env_get(lua_State *L, json_elm *elm , int idx, env_field field);
void alloc_events(json_elm *elm);


void lua_json_env_manager_init(json_elm *elm, elm_env *env);
#endif