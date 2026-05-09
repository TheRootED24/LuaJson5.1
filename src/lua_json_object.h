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

 /***
*JSON.object Class
@classmod object
*/

/**
 * render an object as valid json object
 * @function object:tojson
 * @param start string or number: start position (optional)
 * @param end string or number: end position (optional)
 * @param escape bool: escape output (optional)
 * @return string: json string 
 * 
 * @usage print(obj:tojson()) --> {"test":"obj","age":99,"root":null}
 * @usage print(obj:tojson(0)) --> {"test":"obj"}
 * @usage print(obj:tojson("test", "age")) --> {"test":"obj", "age", 99}
 * @usage print(obj:tojson(1,2,true)) --> {\"age\":99,\"root\":false}
 */
//static int lua_json_object_tojson(lua_State *L) 

/**
 * render an object as a serialized lua table
 * @function object:tolua
 * @param start string or number: start position (optional)
 * @param end string or number: end position (optional)
 * @param escape bool: escape output (optional)
 * @return string: serialized table 
 * 
 * @usage print(obj:tolua()) --> {test="obj",age=99,root=null,}
 * @usage print(obj:tolua(0)) --> {test="obj"}
 * @usage print(obj:tolua("test", "age")) --> {test="obj",age=99}
 * @usage print(obj:tolua(1,2,true)) --> {test=\"obj\",age=99,root=false}
 */
//static int lua_json_object_tojson(lua_State *L) 

/**
 * return lua json object as a lua table
 * @function object:totable
 * @return lua table
 * @see JSON:object
 * @usage local t = o:totable()
 * print(t) --> table: 0x627a723b6400
 * print(o) --> object: 0x627a723a9d30
 * print(o[0]) --> test
 * print(t[1]) --> test
 * print(o[2]) --> null
 * print(t[3]) --> null
 */
// static int lua_json_array_tojson(lua_State *L)

/**
 * Create a new array containing the keys of an object
 * @function object:keys
 * @return array: json array of the objects keys 
 * @usage print(obj:tojson()) --> {"test":"obj","age":99,"root":null}
 * local keys = obj:keys() 
 * print(keys:tojson())--> ["test","age","root"]
 */
//static int lua_json_object_keys(lua_State *L) 


/**
 * move an existing key/value pair to any exisiting index
 * @function object:move
 *  @param  move string or number: exisiting key or index of pair to move
 *  @param to string or number: exisiting key or index a pair to start the shift right
 *  @return no value retured.
 * 
 * @usage print(obj:tojson()) --> {"test":"obj","age":99,"root":null}
 * obj:move("root", "test")
 * print(obj:tojson()) --> {"root":null,"test":"obj","age":99}
 */
// static int lua_json_object_move(lua_State *L)

/**
 * insert new key/value pair at any existing position
 * @function object:insert
 * @param at string or number: exisitng key or index to insert the new pair
 * @param new          string: new key
 * @param value           any:  new value
 * @return number: updated size of object.
 * 
 * @usage print(obj:tojson()) --> {"test":"obj","age":99,"root":null}
 * local s = obj:insert(0, "new","value")
 * print(obj:tojson()) --> {"new":"value","test":"obj","root":null,"age":99}
 * print(s) --> 4 
 */
// static int lua_json_object_insert(lua_State *L)

/**
 * push new key/value pair to end of object
 * @function object:push
 * @param key string: new key
 * @param value any: new value
 * @return number: updated size of object.
 * 
 * @usage print(obj:tojson()) --> {"test":"obj","age":99,"root":null}
 * local s = obj:push("new","value")
 * print(obj:tojson()) --> {"test":"obj","age":99,"root":null,"new":"value"}
 * print(s) --> 4
 */
// static int lua_json_object_push(lua_State *L)

/**
 * pop the last key/value pair from object
 * @function object:pop
 * @return any: the popped value.
 * 
 * @usage print(obj:tojson()) --> {"test":"obj","age":99,"root":null}
 * local p = obj:pop()
 * print(obj:tojson()) --> {"test":"obj","age":99}
 * print(p) --> null
 */
// static int lua_json_object_pop(lua_State *L)

/**
 * Remove the first key/value pair from object. 
 * @function object:shift
 * @return any: the shifted value.
*  @usage print(obj:tojson()) --> {"test":"obj","age":99,"root":null}
 * local s = obj:shift()
 * print(obj:tojson()) --> {"age":99,"root":null}
 * print(s) --> obj
 */
// static int lua_json_object_shift(lua_State *L)

 /**
 * add one or more key/value pairs to the beginning of an object.
 * @function object:unshift
 * @param key string: new key
 * @param val any: new value
 * @param[opt] ...  additional pairs
 * @return number: the updated object size.
 * @usage print(obj:tojson()) --> {"test":"obj","age":99,"root":null}
 * local npairs = obj:unshift("name", "teddy", "id", 101, "admin", false)) 
 * print(obj:tojson()) --> {"name":"teddy","id":101,"admin":false,"age":99,"root":null}
 * print(npairs) --> 5
 */
//static int lua_json_object_unshift(lua_State *L)

/**
 * reverses the indexes of an object key/value pairs.  
 * @function object:reverse
 * @param from string or number: key or index to start reversal. (optional) 
 * @param to string or number: key or index to end of reversal. (optional)
 * @return no value returned
 * @usage print(obj:tojson()) --> {"test":"obj","age":99,"root":null, "name":"teddy"}
 * npairs = obj:reverse("test", "root")) 
 * print(obj:tojson()) --> {""root":null,"age":99,test":"obj","name":"teddy"}
 */
//static int lua_json_object_reverse(lua_State *L)


// ####################################################################################### END API DOCUMENTATION ############################################################################################### //

#ifndef LUA_JSON_OBJECT_H
#define LUA_JSON_OBJECT_H

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

void lua_json_open_object(lua_State *L);
int lua_json_object(lua_State *L);
int lua_json_elm_parse_object(lua_State *L);

#endif