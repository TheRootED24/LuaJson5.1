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

#include "lua_json_array.h"
extern const char *Json[];

 /*static void dumpstack(lua_State *L)
{
	int top = lua_gettop(L);
	for (int i = 1; i <= top; i++)
	{
		printf("%d\t%s\t", i, luaL_typename(L, i));
		switch (lua_type(L, i))
		{
		case LUA_TNUMBER:
			printf("%g\n", lua_tonumber(L, i));
			break;
		case LUA_TSTRING:
			printf("%s\n", lua_tostring(L, i));
			break;
		case LUA_TBOOLEAN:
			printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
			break;
		case LUA_TNIL:
			printf("%s\n", "nil");
			break;
		default:
			printf("%p\n", lua_topointer(L, i));
			break;
		}
	}
}*/

// return a vals type, length
static int lua_json_get_array_val_length(lua_State *L, json_elm *elm) {
	int vtype = lua_type(L, -1);
	size_t rlen = 0;

	switch(vtype) {
		case LUA_TUSERDATA: {
			json_elm *nested = check_json_elm(L, -1);
			nested->root = elm;
			rlen += nested->rlen > 2 ? nested->rlen : 2;
			break;
		}
		case LUA_TNUMBER: {
			const char *val = lua_pushfstring(L, Json[ArrNumber], lua_tonumber(L, -1));
			rlen += strlen(val)+1;
			lua_pop(L, 1);
			break;
		}
		case LUA_TBOOLEAN: {
			const char *val = lua_pushfstring(L, Json[ArrBool], btoa(lua_toboolean(L, -1)));
			rlen += strlen(val)+1;
			lua_pop(L, 1);
			break;
		}
		case LUA_TSTRING: {
			size_t vlen = 0;
			const char *val = lua_tolstring(L, -1, &vlen);
			
			// handle null sentinel here
			// only perform strcmp if length 4 >null<
			if ( vlen == 4 && strcmp(val, "null") == 0)
					rlen += strlen(val)+1;
			else 
					rlen += strlen(val)+3;

			break;
		}
	}

	lua_pushnumber(L, rlen);

	return 1;
};

static int lua_json_array_tojson(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	elm->tojson(L);

	return 1;
};

int lua_json_array_insert(lua_State *L) {
	json_elm *elm = check_json_elm(L, -3);
	// stack { elm, idx, val }
	lua_getfenv(L, -3);
	// stack { elm, idx, val, env }
	lua_insert(L, -3);
	// stack { elm, env, idx, val }
	int pos = (luaL_checkinteger(L, -2) + 1);
	// check pos is inbounds
	if(pos < 1 || pos > (int)elm->nelms+1) {
		fprintf(stderr, "Insert Index %d is out of range\n", pos);

		return 0;
	}
	//int vtype = lua_type(L, -1);
	lua_pushvalue(L, -3);
	// stack { elm, env, idx, val, env}
	for (int i = (int)elm->nelms; i >= pos; i--) {
		lua_rawgeti(L, -1, i);     				// Get element at i
		// stack { elm, idx, val, env, val[i] }
		lua_rawseti(L, -2, i + 1); 				// Move it to i + 1
		// stack { elm, idx, val, env }
	}
	lua_pop(L, 1);
	// stack { elm, env, idx, val }
	lua_rawseti(L, -3, pos);
	// stack { elm, env }
	elm->nelms++;
	lua_pop(L, 1);
	// stack { elm }
	lua_pushnumber(L, elm->nelms);
	// stack { elm, nelms }
	return 1;
};

static int lua_json_elm_array_del(lua_State *L) {
	int nargs = lua_gettop(L);
	json_elm *elm = check_json_elm(L, -nargs);
	// stack {elm, pos}
	lua_getfenv(L, -nargs);
	// stack { elm, pos, env }
	lua_insert(L, -2);
	// stack { elm, env, pos }
	int pos = (luaL_checkinteger(L, -1) + 1);
	lua_pop(L, 1);
	// stack { elm, env }
	lua_rawgeti(L, -1, pos);
	// stack { elm, env, val }
	lua_json_get_array_val_length(L, elm);
	// stack { elm, env, val, vlen }
	elm->rlen -= luaL_checknumber(L, -1);
	lua_pop(L, 2);
	// stack { elm, env }
	for (int i = pos; i <= (int)elm->nelms; i++) {
		lua_rawgeti(L, -1, i + 1); 			// Get element at i + 1
		// stack { elm, env, val[i + 1] }
		lua_rawseti(L, -2, i); 	   			// Move it to i 
		// stack { elm, env }
	}
	lua_pop(L, 1);
	elm->nelms--;
	// stack { elm }
	lua_pushnumber(L, elm->nelms);

	return 1;
};

static int lua_json_elm_array_reverse(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	// stack { elm }
	lua_getfenv(L, 1);
	// stack { elm, env }
	int start = 1, end = (int)elm->nelms;
	for (int s = start, e = end; s < e; s++, e--) {
		lua_rawgeti(L, 2, s);     // Get element at 0
		// stack { elm, env, val0 }
		lua_rawgeti(L, 2, e);  // Get element at end
		// stack { elm, env, val0,  val1 }
		lua_rawseti(L, 2, s); // Move end to start
		// stack { elm, env, elm0}
		lua_rawseti(L, 2, e); // Move start to end
		// stack { elm, env }
	}
	lua_pop(L, 1);
	// stack { elm }
	return 0;
};

static int lua_json_elm_array_push(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	// stack { elm, val }
	lua_getfenv(L, 1);
	// stack { elm, val, env }
	lua_insert(L, -2);
	// stack { elm, env, val }
	int pos = (int)(++elm->nelms);
	lua_json_get_array_val_length(L, elm);
	// stack { elm, env, val, vlen }
	elm->rlen += luaL_checknumber(L, -1);
	lua_pop(L, 1);
	// stack { elm, env, val }
	lua_rawseti(L, -2, pos);
	// stack { elm, env }
	lua_pop(L, 1);
	// stack { elm }
	lua_pushnumber(L, elm->nelms);
	// stack { elm, nelms }
	return 1;
};

int lua_json_elm_array_pop(lua_State *L) {
	//dumpstack(L);
	json_elm *elm = check_json_elm(L, 1);
	lua_getfenv(L, 1);
	// stack { elm, env }
	int pos = (int)elm->nelms;
	lua_rawgeti(L, -1, pos);
	// stack { elm, env, val}
	lua_json_get_array_val_length(L, elm);
	// stack { elm, env, val, vlen }
	elm->rlen -= luaL_checknumber(L, -1);
	lua_remove(L, -2);
	// stack { elm, val}
	lua_pushinteger(L, pos);
	// stack { elm, val, pos}
	lua_json_elm_array_del(L);

	return 1;
};

static int lua_json_elm_array_shift(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	if(!elm) return 1;
	lua_getfenv(L, 1);
	// stack { elm, env}
	lua_rawgeti(L, -1, 1);
	//stack {elm, env, val}
	lua_remove(L, -2);
	//stack {elm, val }
	lua_pushinteger(L, 0);
	// stack { val, tbl, pos}
	lua_json_elm_array_del(L);
	// stack { shifted element }
	return 1;
};

static int _lua_json_elm_array_unshift(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	if(!elm) return 1;
	// stack { elm, val }
	lua_json_get_array_val_length(L, elm);
	// stack { elm, val, vlen }
	elm->rlen += luaL_checknumber(L, -1);
	lua_pop(L, 1);
	// stack { elm, val }
	lua_pushinteger(L, 0);
	// stack { elm, val, pos }
	lua_insert(L, -2);
	// stack { elm, pos, val }
	lua_json_array_insert(L);
	lua_pop(L, 1);
	// stack { elm }
	return 1;
};

static int lua_json_elm_array_unshift(lua_State *L) {
	json_elm * elm = check_json_elm(L, 1);
	// stack { elm ,... }
	int nargs = lua_gettop(L);
	for(int i = 1; i < nargs;) {
		lua_pushvalue(L, 1);
		// stack { elm ,... elm }
		lua_pushvalue(L, nargs);
		// stack { elm ,... elm, val[nargs] }
		_lua_json_elm_array_unshift(L);
		// stack { elm ,... elm }
		lua_remove(L, nargs--);
		// stack { elm ,.. elm }
		lua_pop(L, 1);
		// stack { elm ,.. }
	}
	lua_pushnumber(L, elm->nelms);
	// stack { elm, rlen }
	return 1;
};

static int lua_json_array_ref(lua_State *L) {
	check_json_elm(L, 1);
	// stack { elm }
	return 1;
};

static int _lua_json_array_unref(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	// stack { elm }
	elm->tojson(L);
	// stack { elm, elm_json }
	const char *json = lua_tostring(L, -1);
	lua_settop(L, 0);
	//stack { }
	lua_pushstring(L, json);
	// stack { elm_json }
	lua_json_elm_parse(L);
	// stack { new_elm }
	return 1;
};

int __lua_json_render_elm_array(lua_State *L, struct ref *seen) {
	if(lua_isuserdata(L, -1)) {
		json_elm *elm = check_json_elm(L, -1);

		int len = elm->nelms;
		strcat(seen->b, Json[OpenArr]);

		if(lua_isuserdata(L, -1)) {
			lua_getfenv(L, -1);
			for(int a = 1; a <= len; a++) {
				if(lua_istable(L, -1))
						lua_rawgeti(L, -1, a);

				int type = lua_type(L, -1);
				const char *v = NULL;
				switch(type) {
					case LUA_TUSERDATA : {
						json_elm *nested = check_json_elm(L, -1);
						nested->type == JSON_ARRAY_TYPE ? __lua_json_render_elm_array(L, seen) : __lua_json_render_elm_object(L, seen);
						lua_pop(L, 2);
						v = NULL;
						break;
					}
					case LUA_TSTRING: {
							const char *val = lua_tostring(L, -1);
							size_t vlen = strlen(val);
							int vtype = LUA_TSTRING;
							
							// handle null sentinel here
							if (( vlen ) == 4 && val[0] == 'n')
							{ // only perform strcmp if length 4 >null<
								if (val && (strcmp(val, "null")) == 0) {
									//printf("NULL: %s\n", val);
									vtype = LUA_TNULL;
								}
							}

							v = vtype != LUA_TNULL ? lua_pushfstring(L, Json[ArrString], lua_tostring(L, -1)) : lua_pushfstring(L, Json[ArrNull], lua_tostring(L, -1)) ;
							break;
						
					}
					case LUA_TNUMBER: {
						if(!lua_istable(L, -1))
							v = lua_pushfstring(L, Json[ArrNumber], lua_tonumber(L, -1));
						break;
					}
					case LUA_TBOOLEAN: {
						if(!lua_istable(L, -1))
							v = lua_pushfstring(L, Json[ArrBool],  btoa(lua_toboolean(L, -1)));
						break;
					}
					case LUA_TNULL: {
						if(!lua_istable(L, -1))
							v = lua_pushfstring(L, Json[ArrNull],  lua_tostring(L, -1));
						break;
					}
				}
				if(v) {
					strcat(seen->b, v);
					lua_pop(L, 2);
				}
				if(len - a >= 1) strcat(seen->b, Json[JsonNext]);
			}
			strcat(seen->b, Json[CloseArr]);
		}
	}
	else
		return 0;
	// all done !!
	if(lua_gettop(L) == 1)
		return 1;

	// recursion
	return 0;
};

static int lua_json_array_newindex(lua_State *L) {
	json_elm *elm = check_json_elm(L, -3);
	// stack {..., elm, key, val }
	int vtype = lua_type(L, -1);
	elm->idx = (luaL_checkinteger(L, -2) + 1);
	// check idx is inbounds
	if(elm->idx < 1 || elm->idx > (int)elm->nelms+1) {
		fprintf(stderr, "Index %d is out of range\n", elm->idx);

		return 0;
	}
	// add new element
	if(elm->idx > (int)elm->nelms) {
		lua_json_get_array_val_length(L, elm);
		// stack {..., elm, key, val, vlen }
		elm->rlen += lua_tonumber(L, -1);
		lua_pop(L, 1);
	}
	// remove an element
	else if(vtype == LUA_TNIL) {
		lua_pop(L, 1);
		lua_json_elm_array_del(L);

		return 0;
	}
	// update existing element
	else {
		lua_json_get_array_val_length(L, elm);
		// stack {..., elm, key, val, vlen }
		elm->rlen += luaL_checknumber(L, -1);
		lua_pop(L, 1);
		// stack {..., elm, key, val }
		lua_getfenv(L, -3);
		// stack {..., elm, key, val, env }
		lua_rawgeti(L, -1, elm->idx);
		// stack {..., elm, key, val, env, exval }
		lua_json_get_array_val_length(L, elm);
		// stack {..., elm, key, val, env, exval, exlen}
		elm->rlen -= luaL_checknumber(L, -1);
		lua_pop(L, 3);
		// stack {..., elm, key, val }
	}
	
	// stack {..., elm, key, val }
	lua_getfenv(L, -3);
	// stack {..., elm, key, val, env}
	lua_insert(L, -3);
	//lua_settop(L, 4);
	// stack { elm, env, key, val }
	lua_pushinteger(L, elm->idx);
	// stack {..., elm, env, key, val, idx }
	lua_insert(L, -2);
	// stack {..., elm, env, key, idx, val }
	lua_remove(L, -3);
	// stack {..., elm, env, idx, val }
	lua_rawset(L, -3); 
	// stack {..., elm, env }
	if(elm && (elm->idx > (int)elm->nelms))
		elm->nelms++;

    return 0;
};

static int lua_json_array_index(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	if(!elm) return 0;

	lua_getfenv(L, 1);
	// stack { elm, key, env}
	if(!lua_isnumber(L, -1)) {
		const char *key = lua_tostring(L, 2);
		if(strcmp("env", key) == 0) 
				lua_getfenv(L, 1);
		else if(strcmp("tojson", key) == 0)
				lua_pushcfunction(L, lua_json_array_tojson);
		else if(strcmp("tolua", key) == 0) 
				lua_pushcfunction(L, lua_json_tolua);
		else if(strcmp("info", key) == 0)
				lua_pushcfunction(L, lua_json_elm_info);
		else if(strcmp("len", key) == 0) 
				lua_pushcfunction(L, lua_json_elm_len);
		else if(strcmp("insert", key) == 0)
				lua_pushcfunction(L, lua_json_array_insert);
		else if(strcmp("pop", key) == 0)
				lua_pushcfunction(L, lua_json_elm_array_pop);
		else if(strcmp("push", key) == 0)
				lua_pushcfunction(L, lua_json_elm_array_push);
		else if(strcmp("shift", key) == 0)
				lua_pushcfunction(L, lua_json_elm_array_shift);
		else if(strcmp("unshift", key) == 0)
				lua_pushcfunction(L, lua_json_elm_array_unshift);
		else if(strcmp("reverse", key) == 0)
				lua_pushcfunction(L, lua_json_elm_array_reverse);
		else if(strcmp("del", key) == 0)
				lua_pushcfunction(L, lua_json_elm_array_del);
		else if(strcmp("ref", key) == 0)
				lua_pushcfunction(L, lua_json_array_ref);
		else if(strcmp("unref", key) == 0)
				lua_pushcfunction(L, _lua_json_array_unref);
		else
				lua_pushnil(L);

		return 1;
	}

	lua_getfenv(L, 1);
	// stack { elm, key, env }
	lua_insert(L, 2);
	// stack { elm, env, key }
	lua_rawgeti(L, 2, (lua_tointeger(L, 3) + 1));
	// stack { elm, env, val }
	lua_remove(L, -2);
	// stack { elm, val }
    return 1;
};

static int lua_json_array_inline_args(lua_State *L, int nargs) {
    // stack { elm, ...,   }
    if(nargs > 0) {
        for (int i = 0; i < nargs; i++) {
            // stack { elm, ... }
            lua_pushinteger(L, i);
            // stack { elm, ..., i }
			if(!lua_isnil(L, 2)) {
				lua_pushvalue(L, 2);
				lua_remove(L, 2);
				// stack { elm, .., i, val[1] }
				lua_pushvalue(L, 1);
				lua_insert(L, -3);
				// stack { elm, .., elm, i, val[1] }
				lua_settable(L, -3);
				// stack { elm, .., elm}
				lua_pop(L, 1);
			}
			else {
				fprintf(stderr, "Invalid Entry at Idnex: %d, aborting create array !!\n", i);
				lua_pushnil(L);
				return 1;
			}
        }
    }

	return 1;
};

static int lua_json_array_init(lua_State *L, int nargs) {
	// stack { elm, args }
     if(lua_istable(L, 2)) {
            lua_remove(L, 2);
            lua_remove(L, 2);
            nargs -= 2;
     }
	if((lua_gettop(L) - 1) > 0 ) {
		int args = lua_type(L, -1);
       
		// get args type
		if(args == LUA_TTABLE)
			lua_json_lua_parse(L);
		else
		// stack { elm, args }
			lua_json_array_inline_args(L, nargs);

		return 1;
	}

	return 0;
};

static int lua_json_array_new(lua_State *L, bool parse) {
	int nargs = lua_gettop(L);
	json_elm *elm = (json_elm*)lua_newuserdata(L, sizeof(json_elm));
	memset(elm, 0, sizeof(json_elm));

	elm->id = (uintptr_t)lua_topointer(L, -1);
	elm->type = JSON_ARRAY_TYPE;
	elm->typename = "array";

    // create env table
    lua_newtable(L);
    // create its metatable
    lua_setfenv(L, -2);
    // elm metattable
    luaL_getmetatable(L, "JSON.array");
	lua_setmetatable(L, -2);
	// c side methods
	elm->tostring = &lua_json_elm_tostring;
	elm->tojson = &lua_json_elm_tojson;
	elm->parse = &lua_json_elm_parse;
	elm->rlen = 2;

	if(!parse && nargs > 0) {
		lua_insert(L, 1);
		lua_json_array_init(L, nargs);
	}

	return 1;
};

int lua_json_array(lua_State *L) {
	lua_json_array_new(L, false);

	return 1;
};

int lua_json_elm_parse_array(lua_State *L) {
	lua_json_array_new(L,true);

	return 1;
};

static const struct luaL_reg json_array_lib_f [] = {
	{"tolua",		lua_json_tolua				},
	{"info", 		lua_json_elm_info			},
	{ "len",		lua_json_elm_len			},
	{"__len", 		lua_json_elm_size			},
	{"__tostring",	lua_json_elm_tostring		},
	{"__index", 	lua_json_array_index		},
	{"__newindex", 	lua_json_array_newindex		},
	{NULL, NULL}
};

static const struct luaL_reg json_array_lib_m [] = {
	{"tolua",		lua_json_tolua				},
	{"info", 		lua_json_elm_info			},
	{ "len",		lua_json_elm_len			},
	{"__len", 		lua_json_elm_size			},
    {"__tostring",	lua_json_elm_tostring	    },
	{"__index", 	lua_json_array_index		},
	{"__newindex", 	lua_json_array_newindex		},
	{NULL, NULL}
};

void lua_json_open_array(lua_State *L) {
	lua_newtable(L);
    lua_newtable(L);
    lua_pushstring(L, "__call");
    lua_pushcfunction(L, lua_json_array);
    lua_settable(L, -3);
    lua_setmetatable(L, -2);
	luaL_register(L, NULL, json_array_lib_m);
	lua_setfield(L, -2, "array");
	// mg_mgr
	luaL_newmetatable(L, "JSON.array");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);  /* pushes the metatable */
	lua_settable(L, -3);  /* metatable.__index = metatable */

	luaL_openlib(L, NULL, json_array_lib_m, 0);
	luaL_openlib(L, "json_array", json_array_lib_f, 0);
	lua_pop(L, 2);
};