#include "lua_json_object.h"

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

extern const char *Json[];
int _lua_json_elm_key_to_idx(lua_State *L, const char *key, bool add) {
	json_elm *elm = check_json_elm(L, -3);
	// its the initial key

	lua_getfenv(L, -3); // load the env
	lua_rawgeti(L, -1, 0); // load its lookup table

	for(size_t i = 1; i <= elm->nelms; i++) {
		lua_rawgeti(L, -1, i);
		if(lua_isstring(L, -1))
			if(strcmp(key, lua_tostring(L, -1)) == 0){
                lua_pushinteger(L, i);
                return 1;
            }

		lua_pop(L, 1); // pop ex_key
	}
	// new key, add it
	int size = lua_objlen(L, -1);
    if(add) {
        lua_pushstring(L, key);
        lua_rawseti(L, -2, (size + 1));
        lua_pop(L, 2); // pop the lookup table and env
        lua_pushinteger(L, (size + 1));

        return 1;
    }
	
    lua_pushnil(L);

    return 1;
};

int _lua_json_elm_idx_to_key(lua_State *L, int idx) {
	json_elm *elm = check_json_elm(L, -2);
	// stack { elm , env }
	lua_rawgeti(L, -1, 0); // load its lookup table
	// stack { elm , env, lookup }
	lua_rawgeti(L, -1, idx);
	// stack { elm , env, lookup, key }
	//dumpstack(L);
	if(elm && lua_isstring(L, -1)){
		lua_insert(L, -2);
		lua_pop(L, 1);
		return 1;
	}
	else
		fprintf(stderr, "Idx: %d has no key referenced!!\n", idx);

	lua_pushnil(L);

	return 1;
};

int __lua_json_render_elm_object(lua_State *L, struct ref *seen) {
    //printf("*********************************** RENDER OBJECT *********************************** \n");
	const char *key = NULL, *v = NULL;
	if(lua_isuserdata(L, -1)) {
		json_elm *elm = check_json_elm(L, -1);
		int len = elm->nelms;
		strcat(seen->b, Json[OpenObj]);
		if(lua_isuserdata(L, -1)) {
			lua_getfenv(L, -1);
			for(int o = 1; o <= len; o++) { // 2
				if(lua_istable(L, -1)) {
					//dumpstack(L);
					_lua_json_elm_idx_to_key(L, o);
					key = lua_tostring(L, -1);
					lua_pop(L, 1);
					lua_getfield(L, -1, key);
				}

				int type = lua_type(L, -1); 
				switch(type) {
					case LUA_TUSERDATA: {
						v = lua_pushfstring(L, Json[JsonKey], key);
						strcat(seen->b, v);
						lua_pop(L, 1);
						v = NULL;
						json_elm *nested = check_json_elm(L, -1);
						nested->type == JSON_ARRAY_TYPE ? __lua_json_render_elm_array(L, seen) : __lua_json_render_elm_object(L, seen);
						lua_pop(L, 2);
						break;
					}
					case LUA_TSTRING: {
						size_t vlen = 0;
						const char *val = lua_tolstring(L, -1, &vlen);
						int vtype = LUA_TSTRING;

						// handle null sentinel here
						if (( vlen ) == 4 && val[0] == 'n')
						// only perform strcmp if length 4 >null<
							if (val && (strcmp(val, "null")) == 0)
								vtype = LUA_TNULL;
						
						v = vtype != LUA_TNULL ? lua_pushfstring(L, Json[ObjString], key, lua_tostring(L, -1)) : lua_pushfstring(L, Json[ObjNull], key, lua_tostring(L, -1));
						break;
					}
					case LUA_TNUMBER: {
						v = lua_pushfstring(L, Json[ObjNumber], key, lua_tonumber(L, -1));
						break;
					}
					case LUA_TBOOLEAN: {
						if(!lua_istable(L, -1))
							v = lua_pushfstring(L, Json[ObjBool], key, btoa(lua_toboolean(L, -1)));
						break;
					}
					case LUA_TNULL: {
						if(!lua_istable(L, -1))
							v = lua_pushfstring(L, Json[ObjNull], key, lua_tostring(L, -1));
						break;
					}
				}
				if(v) {
					strcat(seen->b, v);
					lua_pop(L, 2);
				}
				if(len - o >= 1) strcat(seen->b, Json[JsonNext]);
			}
			strcat(seen->b, Json[CloseObj]);
		}
	}
	if(lua_gettop(L) == 1)
		return 1;

	return 0;
};

int lua_json_object_newindex(lua_State *L) {
    //printf("*********************************** OBJECT NEW INDEX *********************************** \n");
    //dumpstack(L);
	json_elm *elm = check_json_elm(L, -3);
	size_t rlen = 0;
	//int idx = 0;

	int vtype = lua_type(L, -1);
    int ktype = lua_type(L, -2);

    if(ktype == LUA_TTABLE)
        return 0;

    const char *key = lua_tostring(L, -2);
    _lua_json_elm_key_to_idx(L, key, true);
    //idx = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    
    switch(vtype) {
        case LUA_TUSERDATA: {
            const char *val = lua_pushfstring(L, Json[JsonKey], key);
            rlen += strlen(val);
            lua_pop(L, 1);

            json_elm *nested = check_json_elm(L, -1);
            rlen += nested->rlen > 2 ? nested->rlen : 2;
            break;
        }
        case LUA_TNUMBER: {
            const char *val = lua_pushfstring(L, Json[ObjNumber], key, lua_tonumber(L, -1));
            rlen += strlen(val)+1;
            lua_pop(L, 1);
            break;
        }
        case LUA_TBOOLEAN: {
            const char *val = lua_pushfstring(L, Json[ObjBool], key, btoa(lua_toboolean(L, -1)));
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
                if (strcmp(val, "null") == 0) {
                    //printf("VAL:%s == null\n", val);
                    vtype = LUA_TNULL;
                    val = lua_pushfstring(L, Json[ObjNull], key, lua_tostring(L, -1));
                }
            }
            else
                    val = lua_pushfstring(L, Json[ObjString], key, lua_tostring(L, -1));

			rlen += strlen(val)+1;
            lua_pop(L, 1);

            break;
        }
    }

	lua_getfenv(L, -3);
	// stack {..., udata, key, val, env}
	lua_insert(L, -3);
	// stack { udata, env, key, val }
	//dumpstack(L);

	lua_rawset(L, -3); 

	if(elm){
		elm->nelms++;
		elm->rlen += rlen;
	}

    return 0;
};

int lua_json_object_index(lua_State *L) {
    //printf("*********************************** OBJECT INDEX *********************************** \n");
	json_elm *elm = check_json_elm(L, 1);
	// stack { udata, key }
    if(elm) {
        const char *key = lua_tostring(L, 2);
        if(strcmp("env", key) == 0) {
            lua_getfenv(L, 1);
            return 1;
        }
        else if(strcmp("tojson", key) == 0) {
                lua_pushcfunction(L, lua_json_elm_tojson);
            return 1;
        }

        // stack { udata, key, env }
        lua_getfenv(L, 1);
        lua_insert(L, 2);
        // stack { udata, env, key }

        lua_rawget(L, 2);
    }

    return 1;
};

static int lua_json_object_table_args(lua_State *L) {
	//json_elm *elm = check_json_elm(L, -3);
	//size_t size = 0;

    lua_pushnil(L);
    // stack { elm, args, nil }
    while (lua_next(L, -2)) {
        // stack { elm, args, key, val }
        lua_insert(L, 2);
        // stack { elm, val, args, key }
        lua_pushvalue(L, -1);
        // stack { elm, val, args, key, key }
        lua_insert(L, 2);
        // stack { elm, key, val, args, key }
        lua_settable(L, 1);
        // stack { elm, args, key }
    }
    lua_settop(L, 1);

	return 1;	
}

static int lua_json_object_inline_args(lua_State *L, int nargs) {
	//json_elm *elm = check_json_elm(L, 1);
	// stack { elm, ... }

    //printf("OBJECT INLINE ARGS %d\n", nargs);
    //dumpstack(L);
    // stack {elm, ... }
    if((nargs % 2) == 0) {
        //dumpstack(L);
        
        while(lua_gettop(L) > 1) {
            lua_pushvalue(L, 2);
            lua_pushvalue(L, 3);
            // stack {elm, ... key-1, val-1 }
            lua_pushvalue(L, 1);
            lua_insert(L, -3);
            // stack {elm, ... elm, key-1, val-1 }
            lua_remove(L, 3); // rem key-1 original
            lua_remove(L, 2); // remove val-1 original
            lua_settable(L, 1);
            lua_pop(L, 1);
        }
    }

	return 1;
}

static int lua_json_object_init(lua_State *L, int nargs) {
	// stack { elm, args }
	if((lua_gettop(L) - 1) > 0 ) {
		int args = lua_type(L, -1);
		// get args type
		if(args == LUA_TTABLE)
			lua_json_object_table_args(L);
		else
		// stack { elm, args }
			lua_json_object_inline_args(L, nargs);

		return 1;
	}

	return 0;
}


static int lua_json_object_new (lua_State *L, bool parse) {
	int nargs = lua_gettop(L);
	json_elm *elm = (json_elm*)lua_newuserdata(L, sizeof(json_elm));
	memset(elm, 0, sizeof(json_elm));

	elm->id = (uintptr_t)lua_topointer(L, -1);
	elm->type = JSON_OBJECT_TYPE;
    elm->typename = "object";
	
    // create env table
    lua_newtable(L);
	lua_pushvalue(L, -2);
	lua_setfield(L, -2, "ctx");

	// add a key lookup table to objects
    lua_newtable(L);
    lua_rawseti(L, -2, 0);

	// methods 
	lua_pushstring(L, "test");
	lua_pushcfunction(L, test_func);
	lua_settable(L, -3);

	lua_pushstring(L, "tojson");
	lua_pushcfunction(L, lua_json_elm_tojson);
	lua_settable(L, -3); // set the __newindex in the metatable (-3)

    // create its metatable
    lua_newtable(L);
	lua_pushvalue(L, -1);
	lua_setfield(L, -3, "__index");

    lua_pushstring(L, "__index");
    lua_pushcfunction(L, lua_json_elm_index);
    lua_settable(L, -3);
	
    lua_pushstring(L, "__newindex");
    lua_pushcfunction(L, lua_json_elm_newindex);
    lua_settable(L, -3);

    // set envs metatable
	lua_setmetatable(L, -2);
    lua_setfenv(L, -2);

    // elm metattable
    luaL_newmetatable(L, "JSON.json");

    lua_pushstring(L, "__index");
    lua_pushcfunction(L, lua_json_elm_index);
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
		lua_json_object_init(L, nargs);
	}
    //printf("********************************** NEW OBJECT ********************************************\n");
	return 1;  /* new userdatum is already on the stack */
};

int lua_json_object(lua_State *L) {
	lua_json_object_new(L, false);

	return 1;
}

int lua_json_elm_parse_object(lua_State *L) {
	lua_json_object_new(L, true);

	return 1;
};

static const struct luaL_reg lua_json_object_lib_f[] = {
	{"new", 		lua_json_object			},
	{"test", 		test_func				},
	{"__len", lua_json_elm_len				},
	{"__tostring", 	lua_json_elm_tostring	},
	{NULL, NULL}};

static const luaL_reg lua_json_object_lib_m[] = {
	{"new", 		lua_json_object			},
	{"test", 		test_func				},
	{"__len", 		lua_json_elm_len		},
	{"__tostring", 	lua_json_elm_tostring	},
	{NULL, NULL}};

void lua_json_open_object(lua_State *L) {
	lua_newtable(L);
    lua_newtable(L);
    lua_pushstring(L, "__call");
    lua_pushcfunction(L, lua_json_object);
    lua_settable(L, -3);

    lua_setmetatable(L, -2);
	luaL_register(L, NULL, lua_json_object_lib_m);
	lua_setfield(L, -2, "object");
	// mg_mgr
    
	luaL_newmetatable(L, "JSON.json");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);  /* pushes the metatable */
	lua_settable(L, -3);  /* metatable.__index = metatable */
	luaL_openlib(L, NULL, lua_json_object_lib_m, 0);
	luaL_openlib(L, "json_object", lua_json_object_lib_f, 0);
	lua_pop(L, 2);

};