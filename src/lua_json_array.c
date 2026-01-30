#include "lua_json_array.h"

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


int lua_json_array_insert(lua_State *L) {
	json_elm *elm = check_json_elm(L, -3);
	// stack {tbl, idx, val}
	lua_getfenv(L, -3);
	lua_insert(L, -3);
	
	//dumpstack(L);
	int pos = (luaL_checkinteger(L, -2) + 1);
	//int vtype = lua_type(L, -1);
	lua_pushvalue(L, -3);
	// stack { tbl, idx, val, tbl}
	for (int i = (int)elm->nelms; i >= pos; i--) {
		lua_rawgeti(L, -1, i);     // Get element at i
		lua_rawseti(L, -2, i + 1); // Move it to i + 1
		//elm->vtypes[i + 1] = elm->vtypes[i];
	}
	lua_pop(L, 1);

	lua_rawseti(L, -3, pos);
	//elm->vtypes[pos] = vtype;
	elm->nelms++;
	lua_pop(L, 1); // pop env

	return 0;
}

static int lua_json_elm_array_del(lua_State *L) {
	int nargs = lua_gettop(L);
	json_elm *elm = check_json_elm(L, -nargs);
	// stack {tbl, pos}
	lua_getfenv(L, -nargs);
	lua_insert(L, -2);
	
	//dumpstack(L);
	int pos = (luaL_checkinteger(L, -1) + 1);
	lua_pop(L, 1);
	// stack {val, tbl}
	// get the value and subtract its length from elm->rlen
	lua_rawgeti(L, -1, pos);
	get_json_val_length(L, elm);
	elm->rlen -= luaL_checknumber(L, -1)+2;
	lua_pop(L, 2);

	for (int i = pos; i <= (int)elm->nelms; i++) {
		lua_rawgeti(L, -1, i + 1); // Get element at i
		lua_rawseti(L, -2, i); 	   // Move it to i + 1
		//elm->vtypes[i] = elm->vtypes[i + 1];
	}
	lua_pop(L, 1);
	// stack { val }
	//elm->vtypes[pos] = vtype;
	elm->nelms--;
	lua_pop(L, 1);

	return 0; // will put nil on stack
	// stack { val, nil }
}

static int lua_json_elm_array_reverse(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	// stack {tbl}
	lua_getfenv(L, 1);
	//dumpstack(L);
	int start = 1, end = (int)elm->nelms;
	for (int s = start, e = end; s < e; s++, e--) {
		lua_rawgeti(L, 2, s);     // Get element at 0
		//stype = elm->vtypes[s]; // get its vtype vtype
		// stack { tbl, elm 0 }
		lua_rawgeti(L, 2, e);  // Get element at end
		//etype = elm->vtypes[e]; // get its vtype
		// stack { tbl, elm0,  elm-1 }
		lua_rawseti(L, 2, s); // Move end to start
		//elm->vtypes[s] = etype; // align vtype
		// stack { tbl, elm0}
		lua_rawseti(L, 2, e); // Move start to end
		//elm->vtypes[e] = stype; // align vtype
		// stack { tbl }
	}
	lua_pop(L, 1);

	return 0;
}

static int lua_json_elm_array_push(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	lua_getfenv(L, 1);
	lua_insert(L, -2);
	// shift elements down from removed element
	// shift vtypes to match
	// preform table.remove on rem
	// stack { ..., array, rem}
	//dumpstack(L);
	int pos = (int)(++elm->nelms);
	//int vtype = lua_type(L, -1);

	lua_rawseti(L, -2, pos);
	//elm->vtypes[pos] = vtype;
	lua_pop(L, 1);

	return 0;
}

int lua_json_elm_array_pop(lua_State *L) {
	//dumpstack(L);
	json_elm *elm = check_json_elm(L, 1);
	lua_getfenv(L, 1);
	// shift elements down from removed element
	// shift vtypes to match
	// preform table.remove on rem
	// stack { ..., array, rem}
	//dumpstack(L);
	int pos = (int)elm->nelms;
	// stack { tbl , val, tbl}
	lua_rawgeti(L, -1, pos);
	lua_remove(L, -2);
	lua_pushinteger(L, pos);
	// stack { val, tbl, pos}
	//dumpstack(L);
	lua_json_elm_array_del(L);
	//dumpstack(L);
	lua_pop(L, 4);

	return 1;
}

static int lua_json_elm_array_shift(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	if(!elm) return 1;
	lua_getfenv(L, 1);
	// stack { elm, env}
	// shift elements down from removed element
	// shift vtypes to match

	lua_rawgeti(L, -1, 0);
	//stack {elm, env, val}
	lua_remove(L, -2);
	//stack {elm, val }
	//lua_insert(L, -2);
	lua_pushinteger(L, 0);
	// stack { val, tbl, pos}
	lua_json_elm_array_del(L);
	//dumpstack(L);
	lua_pop(L, 1);

	return 1;
}

static int lua_json_elm_array_unshift(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	if(!elm) return 1;
	//lua_getfenv(L, -1)
	// stack { tbl, val}
	//dumpstack(L);

	// stack { tbl , val, tbl}
	lua_pushinteger(L, 0);
	// stack {tbl, val, pos}
	lua_insert(L, -2);
	// stack {tbl, pos, val}
	lua_json_array_insert(L);

	lua_pop(L, 1);

	return 0;
}

static int lua_json_array_ref(lua_State *L) {
	check_json_elm(L, 1);
	// stack { proxy, env}
	return 1;
}

int _lua_json_array_unref(lua_State *L) {
	size_t len;
	json_elm *elm = check_json_elm(L, 1);
	elm->tojson(L);
	const char *json = lua_tolstring(L, -1, &len);
	lua_settop(L, 0);
	lua_pushlstring(L, json, len);
	lua_json_elm_parse(L);

	return 1;
}


extern const char *Json[];
int __lua_json_render_elm_array(lua_State *L, struct ref *seen) {
   // printf("*********************************** RENDER ARRAY *********************************** \n");
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
	// all done !!
	if(lua_gettop(L) == 1)
		return 1;

	// recursion
	return 0;
};

int lua_json_array_newindex(lua_State *L) {
    //printf("*********************************** ARRAY NEW INDEX *********************************** \n");
    //dumpstack(L);
	json_elm *elm = check_json_elm(L, -3);
    //printf("ELM TYPE: %s\n", elm->typename);

	size_t rlen = 0;
	int idx = 0;

	int ktype = lua_type(L, -2);

   if(ktype == LUA_TTABLE) {
       return 0;
    }

	int vtype = lua_type(L, -1);
    idx = luaL_checkinteger(L, -2);

    switch(vtype) {
        case LUA_TUSERDATA: {
            json_elm *nested = check_json_elm(L, -1);
            rlen += nested->rlen > 2 ? nested->rlen : 2;
            lua_pop(L, 1);
            //rlen += 1; // comma
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
            const char *val = lua_tostring(L, -1);
            size_t vlen = strlen(val);

            // handle null sentinel here
            // only perform strcmp if length 4 >null<
            if (( vlen ) == 4 && val[0] == 'n') { 
                if (val && (strcmp(val, "null")) == 0) {
                    rlen += strlen(val);
                }
            }
            else {
                    rlen += strlen(val)+3;
            }
            break;
        }
    }
	
	lua_getfenv(L, -3);
	// stack {..., udata, key, val, env}
	lua_insert(L, -3);
	// stack { udata, env, key, val }
	//dumpstack(L);
	if(ktype == LUA_TNUMBER) {
		idx = luaL_checkinteger(L, -2);
		lua_pushinteger(L, idx +1 );
		lua_insert(L, -2);
		lua_remove(L, -3);
	}

	lua_rawset(L, -3); 

	if(elm){
		elm->nelms++;
		elm->rlen += rlen;
	}

    return 0;
};

int lua_json_array_index(lua_State *L) {
	//printf("****************************************** ARRAY INDEX ***************************************************\n");
	//dumpstack(L);
	json_elm *elm = check_json_elm(L, 1);
	// stack { udata, key }
	if(!lua_isnumber(L, -1)) {
		const char *key = lua_tostring(L, 2);
		if(strcmp("env", key) == 0) {
			lua_getfenv(L, 1);
			return 1;
		}
		else if(strcmp("tojson", key) == 0) {
				lua_pushcfunction(L, lua_json_elm_tojson);
			return 1;
		}
		else if(strcmp("test", key) == 0) {
				lua_pushcfunction(L, test_func);
			return 1;
		}
		else if(strcmp("insert", key) == 0) {
				lua_pushcfunction(L, lua_json_array_insert);
			return 1;
		}
		else if(strcmp("pop", key) == 0) {
				lua_pushcfunction(L, lua_json_elm_array_pop);
			return 1;
		}
		else if(strcmp("push", key) == 0) {
				lua_pushcfunction(L, lua_json_elm_array_push);
			return 1;
		}
		else if(strcmp("shift", key) == 0) {
				lua_pushcfunction(L, lua_json_elm_array_shift);
			return 1;
		}
		else if(strcmp("unshift", key) == 0) {
				lua_pushcfunction(L, lua_json_elm_array_unshift);
			return 1;
		}
		else if(strcmp("reverse", key) == 0) {
				lua_pushcfunction(L, lua_json_elm_array_reverse);
			return 1;
		}
		else if(strcmp("del", key) == 0) {
				lua_pushcfunction(L, lua_json_elm_array_del);
			return 1;
		}
		else if(strcmp("ref", key) == 0) {
				lua_pushcfunction(L, lua_json_array_ref);
			return 1;
		}
		else if(strcmp("unref", key) == 0) {
				lua_pushcfunction(L, _lua_json_array_unref);
			return 1;
		}
	}
	// stack { udata, key, env }
	lua_getfenv(L, 1);
	lua_insert(L, 2);
	// stack { udata, env, key }


	elm->type == JSON_ARRAY_TYPE ? lua_rawgeti(L, 2, (lua_tointeger(L, 3) + 1)) : lua_rawget(L, 2);

    return 1;
};

static int lua_json_array_table_args(lua_State *L) {
	//json_elm *elm = check_json_elm(L, -3);
	size_t size = 0;

    size = lua_objlen(L, -1);
    for(int i = 1; i <= (int)size; i++) {
        lua_rawgeti(L, 2, i);
        // stack { elm, args, val[i] }
        lua_pushinteger(L, i);
        // stack { elm, args, val[i], i}
        lua_insert(L, 2);
        // stack { elm, i, args, val[i] }
        lua_insert(L, 3);
        // stack { elm, i, val[i], args }
        lua_settable(L, 1);
        // stack { elm, args }
    }

	return 1;	
}

static int lua_json_array_inline_args(lua_State *L, int nargs) {
	//json_elm *elm = check_json_elm(L, 1);

    //printf("ARRAY INLINE ARGS\n");
    // stack { elm, ...,   }
    //dumpstack(L);
    if(nargs > 0) {
        for (int i = 0; i < nargs; i++) {
            // stack { elm, ... }
            lua_pushinteger(L, i);
            // stack { elm, ..., i }
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
    }

	return 1;
}

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
			lua_json_array_table_args(L);
		else
		// stack { elm, args }
			lua_json_array_inline_args(L, nargs);

		return 1;
	}

	return 0;
}



int lua_json_array_new(lua_State *L, bool parse) {
	int nargs = lua_gettop(L);
	json_elm *elm = (json_elm*)lua_newuserdata(L, sizeof(json_elm));
	memset(elm, 0, sizeof(json_elm));

	elm->id = (uintptr_t)lua_topointer(L, -1);
	elm->type = JSON_ARRAY_TYPE;
	elm->typename = "array";

    // create env table
    lua_newtable(L);
	lua_pushvalue(L, -2);
	lua_setfield(L, -2, "ctx");

	lua_pushstring(L, "atest");
	lua_pushcfunction(L, test_func);
	lua_settable(L, -3);

	lua_pushstring(L, "tojson");
	lua_pushcfunction(L, lua_json_elm_tojson);
	lua_settable(L, -3); // set the __newindex in the metatable (-3)

    // create its metatable

	lua_pushstring(L, "atest");
	lua_pushcfunction(L, test_func);
	lua_settable(L, -3);

    lua_newtable(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, -3, "__index");

    lua_pushstring(L, "__index");
    lua_pushcfunction(L, lua_json_array_index);
    lua_settable(L, -3);
	
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, lua_json_elm_newindex);
    lua_settable(L, -3);

    // set envs metatable
	lua_setmetatable(L, -2);
    lua_setfenv(L, -2);

    // elm metattable
    luaL_newmetatable(L, "JSON.json");

	lua_pushstring(L, "atest");
	lua_pushcfunction(L, test_func);
	lua_settable(L, -3);

    lua_pushstring(L, "__index");
    lua_pushcfunction(L, lua_json_array_index);
    lua_settable(L, -3);

	lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, lua_json_elm_newindex);
    lua_settable(L, -3);
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

	return 1;  /* new userdatum is already on the stack */
};

int lua_json_array(lua_State *L) {
	lua_json_array_new(L, false);

	return 1;
}


int lua_json_elm_parse_array(lua_State *L) {
	lua_json_array_new(L,true);

	return 1;
}

static const struct luaL_reg json_array_lib_f [] = {
    {"new",	        lua_json_array	        	},
	{"atest", 		test_func					},
	{"__len",	    lua_json_elm_len	    	},
	{"__tostring",	lua_json_elm_tostring		},
	{NULL, NULL}
};

static const struct luaL_reg json_array_lib_m [] = {
    {"new",	        lua_json_array	            },
	{"atest", 		test_func					},
	{"__len",	    lua_json_elm_len	        },
    {"__tostring",	lua_json_elm_tostring	    },
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
	luaL_newmetatable(L, "JSON.json");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);  /* pushes the metatable */
	lua_settable(L, -3);  /* metatable.__index = metatable */

	luaL_openlib(L, NULL, json_array_lib_m, 0);
	luaL_openlib(L, "json_array", json_array_lib_f, 0);
	lua_pop(L, 2);
};