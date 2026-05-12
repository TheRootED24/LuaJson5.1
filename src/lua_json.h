/*
 * LuaJson - a json api library for lua
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
 * LuaJson module.
 *
 * The module exposes callable `array` and `object` factory tables plus helpers
 * for parsing JSON strings, converting Lua tables, and rendering JSON elements.
 *
 * Arrays are zero-indexed by default. Call `json_base(1)` on an element to use
 * Lua-style one-based indexing for numeric API calls.
 *
 * @module JSON
 */

/**
 * Create a new JSON array.
 * @function JSON:array
 * @param[opt] ... Initial values.
 * @return array A new JSON array.
 * @usage local JSON = require "JSON"
 * local a = JSON:array(1, 2, 3.45, "test", true, null)
 * print(a[0])      --> 1
 * print(a[#a - 1]) --> null
 */

/**
 * Create a new JSON object from key/value pairs.
 * @function JSON:object
 * @param[opt] ... Alternating string keys and values.
 * @return object A new JSON object.
 * @usage local o = JSON:object("name", "test", "value", 42, "root", null)
 * print(o.name)  --> test
 * print(o.value) --> 42
 * print(o.root)  --> null
 */

/**
 * Parse a JSON string into a LuaJson array or object.
 * @function JSON:parse
 * @param json string JSON text beginning with `[` or `{`.
 * @return array|object Parsed JSON element.
 * @usage local a = JSON:parse('[1,2,3.45,true]')
 * print(a:tojson()) --> [1,2,3.45,true]
 * print(a[0])       --> 1
 */

/**
 * Convert a Lua table into a LuaJson element.
 * @function JSON:parse_lua
 * @param t table Table to convert.
 * @param[opt] as string Conversion option: `"-a"` for array, `"-o"` for object,
 * `"-p"` to parse and return the converted element, or include `"v"` for verbose output.
 * @param[opt] mixed_name string Field name used for mixed table members.
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
 * @param[opt] mixed_name string Field name used for mixed table members.
 * @param[opt] no_mixed boolean When true, ignore mixed-key fixups.
 * @return array|object Converted JSON element.
 */

/**
 * Serialize a LuaJson element to a JSON string.
 * @function JSON:stringify
 * @param elm array|object LuaJson element.
 * @return string JSON string.
 * @usage local a = JSON:parse('[1,2,3,true]')
 * print(JSON:stringify(a)) --> [1,2,3,true]
 */

/**
 * Serialize a Lua table or LuaJson element.
 * @function JSON:stringify_lua
 * @param value table|array|object Lua table or LuaJson element.
 * @param[opt] mode string `"json"` for JSON output or `"lua"` for Lua table output.
 * @param[opt] escape boolean Escape quotes in the rendered string.
 * @return string Serialized value.
 * @usage local t = {1, 2, 3, 4, 5}
 * print(JSON:stringify_lua(t, "json")) --> [1,2,3,4,5]
 */

/**
 * Return the detected table kind and number of table entries.
 * @function JSON:table_len
 * @param t table Table to inspect.
 * @return number Type id.
 * @return number Entry count.
 */

/**
 * Print the render-length counters for a Lua table.
 * @function JSON:table_rlen
 * @param t table Table to inspect.
 */

// ####################################################################################### END API DOCUMENTATION ############################################################################################### //

#ifndef LUA_JSON_H
#define LUA_JSON_H

#define LUA_JSON "JSON"

#define JSON_METHODS "JSON.json"

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
#include <ctype.h>
#include <errno.h>
#include <inttypes.h> // REQUIRED FOR 64-BIT PRINT FORMAT MACROS
// includes
#include "includes/mongoose/json.h"
#include "includes/mongoose/str.h"

#include "lua_json_array.h"
#include "lua_json_object.h"
#include "lua_json_lua.h"
#include "lua_json_marshal.h"
#include "lua_json_elm_event.h"
#include "lua_json_int64.h"

#ifdef __cplusplus
}
#endif

// VERSION CONTROL MACROS //
#if LUA_VERSION_NUM >= 502
    #define lua_objlen(L, i) lua_rawlen(L, (i))
#endif

#if LUA_VERSION_NUM < 502
	#define LUA_OK 0
#endif 

#if LUA_VERSION_NUM >= 504
    /* Lua 5.4 uses indexed uservalues (slot 1 is the legacy fenv equivalent) */
    #define get_json_table(L, i)  lua_getiuservalue(L, i, 1)
    #define set_json_table(L, i)  lua_setiuservalue(L, i, 1)
#elif LUA_VERSION_NUM >= 502
    /* Lua 5.2 and 5.3 */
    #define get_json_table(L, i)  lua_getuservalue(L, i)
    #define set_json_table(L, i)  lua_setuservalue(L, i)
#else
    /* Lua 5.1 Original */
    #define get_json_table(L, i)  lua_getfenv(L, i)
    #define set_json_table(L, i)  lua_setfenv(L, i)
#endif

#if LUA_VERSION_NUM >= 504
    // Allocate 1 slot for our "fenv" replacement
    #define json_newuserdata(L, s) lua_newuserdatauv(L, s, 1)
#else
    #define json_newuserdata(L, s) lua_newuserdata(L, s)
#endif

#if LUA_VERSION_NUM >= 502
    // nup is the number of upvalues. For your NULL register calls, it's 0.
    #define luaL_reg_stack(L, methods) luaL_setfuncs(L, methods, 0)
#else
    #define luaL_reg_stack(L, methods) luaL_register(L, NULL, methods)
#endif

#if LUA_VERSION_NUM >= 504
    /* Lua 5.4: Init pushes the box, then we rotate/bury it to Index 1 */
    #define INITIALIZE_JSON_BUFFER(L, B) \
        luaL_buffinit(L, B); \
        lua_insert(L, 1)

    /* Lua 5.4: Bring the hidden Box from Index 1 to the top for the final push */
    #define FINALIZE_JSON_BUFFER(L, B) \
        lua_pushvalue(L, 1); \
        luaL_pushresult(B)
#else
    /* Lua 5.1: Push a dummy pointer to Index 1, then init the buffer on top */
    #define INITIALIZE_JSON_BUFFER(L, B) \
        lua_pushlightuserdata(L, NULL); \
        lua_insert(L, 1); \
        luaL_buffinit(L, B)

    /* Lua 5.1: Standard collapse (ignores the dummy pointer at Index 1) */
    #define FINALIZE_JSON_BUFFER(L, B) \
        luaL_pushresult(B)
#endif

// END OF VERSION CONTROL MACROS //

#define VLEN_ADD(dst, src, sub_ptr) do {                                   \
    const elm_vlen *__s = (sub_ptr);                                       \
    (dst).rlen   += __s ? ((src).rlen   - __s->rlen)   : (src).rlen;       \
    (dst).quoted += __s ? ((src).quoted - __s->quoted) : (src).quoted;     \
    (dst).nkeys  += __s ? ((src).nkeys  - __s->nkeys)  : (src).nkeys;      \
} while(0)

extern const char *marshal_json[], *marshal_lua[], *marshal_bash[];
extern void dumpstack(lua_State *L, const char *msg);

typedef struct event event;
typedef struct Subject Subject;
typedef void (*NotifyFn)(void* context, event *ev);

// GLOBAL DEFINES
#define DEBUG 0
//#define USE_THREADS

// SPECIFIC FUNCTION DEBUG FLAGS
#define SUBSRIPTIONS 0
#define EVENTS 0
#define STRINGIFY 0

// --- Disable Event Threading Support ---
#define LOCK(s)   ((void)0)
#define UNLOCK(s) ((void)0)
#define INIT_LOCK(s) (1)
#define DESTROY_LOCK(s) ((void)0)

#define btoa(x) ((x) ? "true" : "false")

#define null "null"
#define LUA_TNULL (-2)
extern const char *NULL_CACHE;

#define LUA_TINT64 (-4)

#define MARSHAL_JSON 	0
#define MARSHAL_LUA 	1
#define MARSHAL_BASH 	2

typedef enum {
	JSON_NUMBER_TYPE,		// 0
	JSON_BOOL_TYPE,			// 1
	JSON_LONG_TYPE,			// 2
	JSON_STRING_TYPE,		// 3
	JSON_ARRAY_TYPE,		// 4
	JSON_OBJECT_TYPE,		// 5
	JSON_NESTED_ARRAY_TYPE,	// 6
	JSON_NESTED_OBJECT_TYPE,// 7
	JSON_ROOT_TYPE,			// 8
	JSON_NULL_TYPE,			// 9
	JSON_INVALID_TYPE		// 10
}LUA_JSON_TYPES;

typedef enum {
	ObjKey,     	//0
	ObjString,  	//1
	ObjNumber,  	//2
	ObjInteger,  	//3
	ObjBool,    	//4
	ObjNull,		//5
	ArrString,  	//6
	ArrNumber,  	//7
	ArrInteger,  	//8
	ArrBool,    	//9
	ArrNull,		//10
	Next,   		//11
	OpenObj,    	//12
	CloseObj,   	//13
	OpenArr,    	//14
	CloseArr,   	//15
	EscObjString,  	//16
	EscArrString,   //17
	EscObjKey,    	//18
	EscObjNumber, 	//19
	EscObjInteger, 	//20
	EscObjBool,		//21
	EscObjNull,		//22
	EscObjInt64, 	//23
	ArrInt64,		//24
	ObjInt64,		//25
	BashKey, 		//26
	BashArr,   		//27
	BashClose    	//28
}Type;

typedef enum {
	newindex,
	insert,
	move,
	reverse,
	none
} idx_m;

typedef enum {
	ids,
	keys,
}env_field;

typedef union env_val{
	int num;
	uintptr_t env_id;
	const char *key;
}env_val;

typedef enum {
	NEW_INDEX,
	EXT_INDEX,
	NIL_INDEX
}index_type;

typedef struct new_vlen {
    size_t rlen, quoted, nkeys, trefs, nulls, children;
}elm_vlen;

typedef struct elm_rlen {
	json_elm *root, *sub, *unsub;

	elm_vlen base;
	elm_vlen new;
	elm_vlen ex;

	uint8_t toi;
	uint8_t vtype, ex_vtype;
	bool is_exval, is_nil, is_null;

}elm_rlen;

typedef struct elm_event {
	json_elm *root;
	struct Subject *on_newindex, *on_change, *on_env, *on_mutate;
	int (*init)		(Subject *s);
	void (*sub)		(Subject *s, void *ctx, NotifyFn fn);
	void (*unsub)	(Subject *s, void *ctx, NotifyFn fn);
	void (*set)		(Subject *s, event *ev);
	void (*get)		(Subject *s, event *ev);
	void (*cleanup)	(Subject *s);
}elm_event;

struct json_ops {
    // Key/Index Logic (Specific to Objects/Arrays)
    int (*key_to_idx)(json_elm *, bool);
    int (*idx_to_key)(json_elm *);
    void (*check_idx)(json_elm *);

    // Common Logic (Render, Parse, etc.)
    void (*init_rlen)(json_elm *, elm_rlen *);
    int (*tostring)	 (lua_State *);
    int (*stringify) (lua_State *);
    int (*del)		 (lua_State *);
    int (*parse)	 (lua_State *);
    int (*render)	 (lua_State *, ref *);
	int (*towasm)	 (lua_State*);
};

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
	int(*obj_int64)(lua_State*,  json_elm*, ref*);
	int(*obj_bool)	(lua_State*, json_elm*, ref*);

	int(*arr_open)	(ref*);
	int(*arr_close)	(ref*);

	int(*arr_string)(lua_State*, json_elm*, ref*);
	int(*arr_number)(lua_State*, ref*);
	int(*arr_int64) (lua_State*, ref*);
	int(*arr_bool)	(lua_State*, ref*);
	
} marshal;

typedef struct ref {
	lua_State *ML;
	luaL_Buffer *B;
	json_elm *elm, *nested;
	marshal *marshal;
	uintptr_t root, next;
	uint8_t mode;
	size_t rlen, quoted, nkeys, has_refs, nulls, children;
	size_t L_max;
	size_t start, end;
	bool isRoot, escape, needs_state, trusted;
	const char **Marshal;
	
	bool(*check_next)(lua_State *, ref*, uintptr_t);
	void(*clear_next)(lua_State *, ref *, uintptr_t);
}ref;

struct json_elm {
    /* POINTERS (8-byte aligned) */
    struct json_elm *root;
    struct json_elm *nested;
    lua_State *L;
    elm_event *event;
    elm_vlen *base;
    
    /* DATA PAYLOAD */
    const char *typename;
    const char *key;
    const char *val;
    const char *dom_id;
    
    // Pointer to the VTable (Standardized)
    const struct json_ops *ops; 

    uintptr_t env_id;
    size_t nelms;
    size_t klen;
    size_t vlen;

    /* THE COMPRESSED BLOCK  */
    
    // The Index (16 bits)
    uint16_t idx;
	uint16_t range;

    // The Mode (8 bits - kept full for safety)
    uint8_t mode;
    
    // The Nibbles (4 bits each - Range 0-15)
    uint8_t type   : 4;
    uint8_t toi    : 4;
    uint8_t ktype  : 4;
    uint8_t vtype  : 4;

    // The Flags (1 bit each)
    bool is_nil    : 1;
    bool escape    : 1;
    bool index_json: 1;
    bool stale     : 1;
    bool align     : 1;
	uint8_t map	   : 1;
    bool is_ref    : 1;
	bool is_env	   : 1;
};
typedef struct json_elm json_elm;

/* In struct event (or separate typedef) */
typedef struct mutation_evt {
    lua_State *L;
    json_elm *elm;
    int key_idx;
    int val_idx;
} mutation_evt;

json_elm *check_json_elm(lua_State *L, int pos, idx_m *map);

uint16_t lua_json_elm_align_idx(json_elm *elm, int pos);
uint16_t lua_json_elm_push_key(json_elm *elm, uint16_t pos);
uint8_t type_of_index(json_elm *elm);

bool lua_json_elm_contains(lua_State *L, json_elm *elm, json_elm *nested);
bool lua_json_is_printable(lua_State *L);

int lua_json_is_int64(lua_State *L, int pos);
int lua_json_is_elm(lua_State *L, int pos);

int lua_json_elm_stringify(lua_State *L);
int lua_json_elm_parse(lua_State *L);

int lua_json_elm_len(lua_State *L);
int lua_json_elm_get_rlen(lua_State *L);
int lua_json_elm_size(lua_State *L);
int lua_json_elm_tostring(lua_State *L);
int lua_json_elm_to_table(lua_State *L);
int lua_json_elm_index_base(lua_State *L);

void alloc_events(json_elm *elm);
void lua_json_elm_unsub(json_elm *elm, json_elm *nested) ;
void lua_json_elm_sub(json_elm *elm, json_elm *nested);
void lua_json_elm_on_newindex(void* ctx, event *ev);
void lua_json_elm_on_change(void* ctx, event *ev);
void lua_json_env_on_change(void* ctx, event *ev);

int lua_json_elm_env_add(lua_State *L, json_elm *elm, env_val *val, env_field field);
int lua_json_elm_env_insert(lua_State *L, json_elm *elm, int idx, env_val *val, env_field field);
int lua_json_elm_env_rem(lua_State *L, json_elm *elm, uintptr_t env_id, env_field field);
int lua_json_elm_env_get(lua_State *L, json_elm *elm , int idx, env_field field);

void lua_json_elm_get_val_length(lua_State *L, json_elm *elm, elm_rlen *erl);
void lua_json_elm_check_idx(json_elm *elm);
void lua_json_elm_init_rlen(json_elm *elm, elm_rlen *erl);
void update_rlen(json_elm *elm, elm_rlen *erl);

void lua_json_elm_push_idx(json_elm *elm, uint16_t pos);

void luajson_register_int64(lua_State *L);
void lua_pushint64(lua_State *L, int64_t val);

#endif