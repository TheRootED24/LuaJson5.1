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
 * parse a lua table into lua json element.
 * @function .parse_table
 * @param t lua table: table to parse
 * @param as string: tac arg "-o" = parse as object | "-a" parse as array (optional)
 * @param verbose string: tac arg "-v" = verbose output (optional)
 * @param no_mixed boolean: silently ignore mixed values 
 * @return an initialized lua json array or object. (is "as in not specified, default will output the type with the most parameters)
 * @usage local t = {1,2,3.45,"test",true} --> 
 * print(t) --> table: 0x5c92968f1c90
 *
 * local ta = JSON.parse_table(t)
 * print(ta) --> array: 0x5c92968f09a8
 * print(ta[0])  --> 1
 * print(ta[2])  --> 3.45
 * print(ta[#ta]) --> true
 */

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
#include <ctype.h>
#include <errno.h>
// includes
#include "includes/mongoose/json.h"
#include "includes/mongoose/str.h"

#include "lua_json_array.h"
#include "lua_json_object.h"
#include "lua_json_lua.h"
#include "lua_json_marshal.h"
#include "lua_json_elm_event.h"

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

extern const char *marshal_json[], *marshal_lua[];
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

#define MARSHAL_JSON 	0
#define MARSHAL_LUA 	1

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
	ObjInteger,  	//2
	ObjBool,    	//3
	ObjNull,		//4
	ArrString,  	//5
	ArrNumber,  	//6
	ArrInteger,  	//6
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
	EscObjInteger, 	//17
	EscObjBool,		//18
	EscObjNull		//19
}Type;

typedef enum {
	ELM, // 0 base
	ENV  // 1 base
} idx_t;

typedef enum {
	newindex,
	insert,
	move,
	reverse,
	none
} idx_m;

typedef struct idx_range {
	uint16_t s;
	uint16_t e;
}idx_r;

typedef struct idx_valid {
	idx_m mode;
	idx_t type;
	uint16_t idx;
	idx_r range;
}idx_v;

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
    size_t rlen, quoted, nkeys, trefs;
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
	void (*unsub)		(Subject *s, void *ctx, NotifyFn fn);
	void (*set)		(Subject *s, event *ev);
	void (*get)		(Subject *s, event *ev);
	void (*cleanup)		(Subject *s);
}elm_event;

struct json_ops {
    // Key/Index Logic (Specific to Objects/Arrays)
    int (*key_to_idx)(json_elm *, bool);
    int (*idx_to_key)(json_elm *);
    void (*check_idx)(json_elm *);

    // Common Logic (Render, Parse, etc.)
    void (*init_rlen)(json_elm *, elm_rlen *);
    int (*tostring)(lua_State *);
    int (*stringify)(lua_State *);
    int (*del)(lua_State *);
    int (*parse)(lua_State *);
    int (*render)(lua_State *, ref *);
	int (*towasm)(lua_State*);
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
	int(*obj_bool)	(lua_State*, json_elm*, ref*);

	int(*arr_open)	(ref*);
	int(*arr_close)	(ref*);

	int(*arr_string)(lua_State*, json_elm*, ref*);
	int(*arr_number)(lua_State*, ref*);
	int(*arr_bool)	(lua_State*, ref*);
	
} marshal;

typedef struct ref {
	lua_State *ML;
	luaL_Buffer *B;
	json_elm *elm, *nested;
	marshal *marshal;
	uintptr_t root, next;
	uint8_t mode;
	size_t rlen, quoted, nkeys, has_refs;
	bool isRoot, escape, needs_state, trusted;
	const char **Marshal;
	size_t L_max;
	size_t start, end;
	int thread_ref;
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
    
    // CRITICAL: Pointer to the VTable (Standardized)
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

    // Padding: Compiler adds 1 byte here automatically to reach 8-byte alignment
};
typedef struct json_elm json_elm;

/* In struct event (or separate typedef) */
typedef struct mutation_evt {
    lua_State *L;
    json_elm *elm;
    int key_idx;
    int val_idx;
} mutation_evt;

//json_elm *check_json_elm(lua_State *L, int pos, bool align);
json_elm *check_json_elm(lua_State *L, int pos, idx_m *map);
int lua_json_is_elm(lua_State *L, int pos);
bool lua_json_elm_contains(lua_State *L, json_elm *elm, json_elm *nested);
uint8_t type_of_index(json_elm *elm);

int lua_json_elm_len(lua_State *L);
int lua_json_elm_size(lua_State *L);
int lua_json_elm_tostring(lua_State *L);
int lua_json_elm_to_table(lua_State *L);
int lua_json_elm_is_stale(lua_State *L);
int lua_json_elm_index_base(lua_State *L);
int lua_json_elm_stringify(lua_State *L);
int lua_json_elm_parse(lua_State *L);
int lua_json_elm_get_rlen(lua_State *L);
int lua_json_elm_env_add(lua_State *L, json_elm *elm, env_val *val, env_field field);
int lua_json_elm_env_insert(lua_State *L, json_elm *elm, int idx, env_val *val, env_field field);
int lua_json_elm_env_rem(lua_State *L, json_elm *elm, uintptr_t env_id, env_field field);
int lua_json_elm_env_get(lua_State *L, json_elm *elm , int idx, env_field field);
int lua_json_elm_get_quoted(lua_State *L);
int lua_json_elm_get_nkeys(lua_State *L);

void alloc_events(json_elm *elm);
void lua_json_elm_unsub(json_elm *elm, json_elm *nested) ;
void lua_json_elm_sub(json_elm *elm, json_elm *nested);
void lua_json_elm_on_newindex(void* ctx, event *ev);
void lua_json_elm_on_change(void* ctx, event *ev);
void lua_json_env_on_change(void* ctx, event *ev);
void lua_json_elm_get_val_length(lua_State *L, json_elm *elm, elm_rlen *erl);
void lua_json_elm_check_idx(json_elm *elm);
bool lua_json_idx_inbounds(json_elm *elm, int min, int max, bool is_env);
void lua_json_elm_init_rlen(json_elm *elm, elm_rlen *erl);
void update_rlen(json_elm *elm, elm_rlen *erl);
bool lua_json_is_printable(lua_State *L);
uint16_t lua_json_elm_align_idx(json_elm *elm, int pos);
uint16_t lua_json_elm_push_key(json_elm *elm, uint16_t pos);
void lua_json_elm_push_idx(json_elm *elm, uint16_t pos);

#endif 