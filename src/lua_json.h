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

#define VLEN_ADD(dst, src, sub_ptr) do {                                   \
    const elm_vlen *__s = (sub_ptr);                                       \
    (dst).rlen   += __s ? ((src).rlen   - __s->rlen)   : (src).rlen;       \
    (dst).quoted += __s ? ((src).quoted - __s->quoted) : (src).quoted;     \
    (dst).nkeys  += __s ? ((src).nkeys  - __s->nkeys)  : (src).nkeys;      \
} while(0)

extern const char *marshal_json[], *marshal_lua[];
typedef struct event event;
typedef void (*NotifyFn)(void* context, event *ev);

// GLOBAL DEFINES
#define DEBUG 0
//#define USE_THREADS

// --- Disable Threading Support ---
#define LOCK(s)   ((void)0)
#define UNLOCK(s) ((void)0)
#define INIT_LOCK(s) (1)
#define DESTROY_LOCK(s) ((void)0)

#define lua_absindex(L, i) \
    ((i) > 0 || (i) <= LUA_REGISTRYINDEX ? (i) : lua_gettop(L) + (i) + 1)

#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)

#define ERROR(msg) \
    fprintf(stderr, "[ ERROR ] %s:%d: %s (errno: %d, %s)\n", \
            __FILENAME__, __LINE__, msg, errno, strerror(errno))

extern void dumpstack(lua_State *L, const char *msg);

#define btoa(x) ((x) ? "true" : "false")
#define null "null"
#define LUA_TNULL (-2)

#define MARSHAL_JSON 	0
#define MARSHAL_LUA 	1
extern const char *NULL_CACHE; 

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


typedef struct Subject Subject;

typedef union u_en_val{
	int num;
	uintptr_t env_id;
	const char *key;
}env_val;

struct s_parse_elm{
	struct mg_str *json;
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
	int depth, max, ids;
	lua_State *ML;
	luaL_Buffer *B;
	json_elm *elm, *nested;
	marshal *marshal;
	uintptr_t root, last, next;
	uint8_t mode;
	size_t rlen, quoted, nkeys;
	int elms;
	int ltype;
	bool isRoot, escape;
	int ptype;
	char* b;
	const char **Marshal;
	bool(*check_next)(lua_State*L, struct ref*, uintptr_t);

}ref;

typedef struct nested_len {
    size_t rlen, quoted, nkeys;
}nested_len;

typedef struct elm_event {
	json_elm *root;
	struct Subject *on_newindex, *on_change, *on_env;
	int (*init)		(Subject *s);
	void (*sub)		(Subject *s, void *ctx, NotifyFn fn);
	void (*unsub)	(Subject *s, void *ctx, NotifyFn fn);
	void (*set)		(Subject *s, event *ev);
	void (*get)		(Subject *s, event *ev);
	void (*cleanup)	(Subject *s);
}elm_event;

typedef enum {
	NEW,
	EXT,
	PAR,
	ERR,
	NIL
}new_index_t;

typedef struct new_vlen {
    size_t rlen, quoted, nkeys;
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


// BASE JSON ELM CLASS
struct json_elm
{
	struct json_elm *root, *nested;
	// events
	bool index_json;
	elm_event *event;
	lua_State *L;
	elm_vlen *base;
	uint8_t toi;
	int vtable, idx;
	uintptr_t id, env_id;
	bool is_env_index, is_exval, stale, align;
	const char* dom_id; // Add this for the JS bridge
	uint8_t mode; // marshall mode
	
	// analytics
	size_t type;
	size_t nelms;
	size_t rlen, klen, vlen, quoted, nkeys;
	int plen, ktype, vtype, xvtype;
	size_t children;
	bool isRoot, is_nil, c_out, escape;
	uint8_t elm_bi, env_bi;
	bool err, parsed;
	const char *typename, *key, *val, *errmsg;

	int (*key_to_idx)(json_elm *, bool);
	bool (*idx_to_key)(json_elm *);
	void (*init_rlen)(json_elm *, elm_rlen *);
	void (*check_idx)(json_elm *);
	int (*tostring)(lua_State *);
	int (*stringify)(lua_State *);
	int (*del)(lua_State *);
	int (*parse)(lua_State *);
	// recursion call
	int (*render)(lua_State *, ref *);
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

typedef enum {
	NEW_INDEX,
	EXT_INDEX,
	NIL_INDEX,
	INS_INDEX,
}index_type;


//json_elm* check_json_env(lua_State *L, bool align);
//json_elm *check_json_elm(lua_State *L, int pos);
json_elm *check_json_elm2(lua_State *L, int pos, bool align);
//json_elm *check_json_elm(lua_State *L, int pos, bool align);
//bool lua_json_elm_contians(lua_State *L, json_elm *elm, json_elm *nested);
bool lua_json_elm_contains(lua_State *L, json_elm *elm, json_elm *nested);
int lua_json_elm_len(lua_State *L);
int lua_json_elm_size(lua_State *L);
int lua_json_elm_tostring(lua_State *L);
//int lua_json_elm_get_val_length(lua_State *L, json_elm *elm);
void lua_json_elm_get_val_length(lua_State *L, json_elm *elm, elm_rlen *erl);
int lua_json_elm_stringify(lua_State *L);
int lua_json_elm_parse(lua_State *L);
int lua_json_elm_info(lua_State *L);
int lua_json_elm_get_rlen(lua_State *L);
int lua_json_elm_env_add(lua_State *L, json_elm *elm, env_val *val, env_field field);
int lua_json_elm_env_insert(lua_State *L, json_elm *elm, int idx, env_val *val, env_field field);
// int lua_json_elm_env_rem(lua_State *L, json_elm *elm, int idx, env_field field);
int lua_json_elm_env_rem(lua_State *L, json_elm *elm, uintptr_t env_id, env_field field);
int lua_json_elm_env_get(lua_State *L, json_elm *elm , int idx, env_field field);
void alloc_events(json_elm *elm);
void lua_json_elm_update_len(json_elm *elm, nested_len *nl);
void lua_json_elm_init_len(json_elm *elm, nested_len *nl);
void json_type(parse_elm *elm);
void lua_json_elm_unsub(json_elm *elm, json_elm *nested) ;
void lua_json_elm_sub(json_elm *elm, json_elm *nested);
void lua_json_elm_on_newindex(void* ctx, event *ev);
void lua_json_elm_on_change(void* ctx, event *ev);
void lua_json_env_on_change(void* ctx, event *ev);
int L_json_elm_bind_dom(lua_State *L);
int lua_json_elm_gc(lua_State *L);

int lua_json_elm_env_getr(lua_State *L);
int lua_json_elm_env_addr(lua_State *L);
int lua_json_elm_env_insertr(lua_State *L);
int lua_json_elm_env_remover(lua_State *L);
int lua_json_elm_get_props(lua_State *L);
uint8_t type_of_index(json_elm *elm);
void lua_json_elm_get_val_length2(lua_State *L, json_elm *elm, elm_rlen *erl);
void lua_json_elm_check_idx(json_elm *elm);
void lua_json_elm_init_rlen(json_elm *elm, elm_rlen *erl);
void update_rlen(json_elm *elm, elm_rlen *erl);
int lua_json_elm_to_table(lua_State *L);
int lua_json_elm_get_quoted(lua_State *L);
int lua_json_elm_get_nkeys(lua_State *L);
bool check_next(lua_State *L, ref *seen, uintptr_t next);
int lua_json_elm_print_ids(lua_State *L);
int lua_json_elm_is_stale(lua_State *L);
size_t lua_json_lua_elm_find_nil(lua_State *L, json_elm *elm);
int find_nil(lua_State *L);
int lua_json_elm_index_base(lua_State *L);
//int lua_json_elm_env_add(lua_State *L, json_elm *elm, env_val *val, env_field field);
//int lua_json_elm_env_insert(lua_State *L, json_elm *elm, int idx, env_val *val, env_field field);
//int lua_json_elm_env_rem(lua_State *L, json_elm *elm, int idx, env_field field);

#endif 