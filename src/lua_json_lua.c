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

#include "lua_json_lua.h"

static void dumpstack(lua_State *L)
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
}

extern const char *Json[];

int __lua_json_render_lua_object(lua_State *L, struct ref *seen) {
    printf("*********************************** RENDER LUA OBJECT *********************************** \n");
    dumpstack(L);
	const char *key = NULL, *v = NULL;
	if(lua_istable(L, -1)) {
		strcat(seen->b, Json[OpenObj]);
        // push a copy of the table
        int idx = 0; 
        lua_pushvalue(L, -1);
        lua_pushnil(L);
        while(lua_next(L, -2) != 0) { 
            // omit object lookup table at index 0
            if(lua_isnumber(L, -2)) { 
                lua_pop(L, 1);
                continue;
            }
            key = lua_tostring(L, -2);

            if(idx >= 1) strcat(seen->b, Json[JsonNext]);
            int type = lua_type(L, -1); 
            switch(type) {
                case LUA_TTABLE: {
                    v = lua_pushfstring(L, Json[JsonKey], key);
                    strcat(seen->b, v);
                    lua_pop(L, 1);
                    v = NULL;

                    lua_objlen(L, -1) > 0 ? __lua_json_render_lua_array(L, seen) : __lua_json_render_lua_object(L, seen);
                    lua_pop(L, 1);
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
            idx++;
            
        }
        strcat(seen->b, Json[CloseObj]);
	}

	if(lua_gettop(L) == 1)
		return 1;

	return 0;
};

int __lua_json_render_lua_array(lua_State *L, struct ref *seen) {
   printf("*********************************** RENDER LUA ARRAY *********************************** \n");
   dumpstack(L);
	if(lua_istable(L, -1)) {
		int len = lua_objlen(L, -1);
		strcat(seen->b, Json[OpenArr]);
        dumpstack(L);
        for(int a = 1; a <= (int)len; a++) {
            if(lua_istable(L, -1))
                    lua_rawgeti(L, -1, a);

            int type = lua_type(L, -1);
            const char *v = NULL;
            switch(type) {
                case LUA_TTABLE : {
                        lua_objlen(L, -1) > 0 ? __lua_json_render_lua_array(L, seen) : __lua_json_render_lua_object(L, seen);
                        lua_pop(L, 1);
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
	// all done !!
	if(lua_gettop(L) == 1)
		return 1;

	// recursion
	return 0;
};

int lua_json_lua_tojson(lua_State *L, bool parse) {
    size_t size = lua_objlen(L, -1);
    int type = size > 0 ? JSON_ARRAY_TYPE : JSON_OBJECT_TYPE;

	ref seen = {0};
	seen.root = (uintptr_t)lua_topointer(L, -1);
	seen.ids[seen.elms++] = seen.root;
	seen.check_next = &check_next;

	seen.ltype = LUA_TUSERDATA;
	seen.b = malloc(MAX_LUA_SIZE+1); // 1MB by default (adjust in "lua_json_lua.h" to suite your needs)
	memset(seen.b, 0, MAX_LUA_SIZE);
	
	type == JSON_ARRAY_TYPE ? __lua_json_render_lua_array(L, &seen) : __lua_json_render_lua_object(L, &seen);
    type == JSON_ARRAY_TYPE ? lua_pop(L, 2) : lua_pop(L, 3);
	lua_pushstring(L, seen.b);
	free(seen.b);
    dumpstack(L);
    if(lua_isnil(L, 1)) lua_remove(L, 1);
    if(parse) lua_json_elm_parse(L);

	return 1;
};

bool lua_json_lua_is_prop(const char *key) {
    if(strcmp(key, "ctx") == 0) 
        return true;
    else if(strcmp(key, "__index") == 0)
        return true;
    else if(strcmp(key, "0") == 0)
        return true;
    else if(strcmp(key, "tojson") == 0)
        return true;
    else
        return false;
}

static int _lua_json_tolua(lua_State *L, bool unref) {
    json_elm *elm = check_json_elm(L, 1);
    lua_getfenv(L, 1);
    // just pass reference to env table
    if(!unref) return 1;
    // create a copy of the element first to unref env table
    lua_json_lua_tojson(L, true);
    // return its env table 
    lua_getfenv(L, -1);
    if(elm->type == JSON_OBJECT_TYPE) {
        // remove the keys lookup table
        lua_pushnil(L);
        lua_rawseti(L, -2, 0);
    }

    return 1;
}

int lua_json_lua_parse(lua_State *L) {
    if(lua_gettop(L) < 2) {
        fprintf(stderr, "Invalid arguments ... aborting lua_parse !!\n");
        lua_pushnil(L);

        return 1;
    }
    
    lua_json_lua_tojson(L, true);
    
    return 1;
}

int lua_json_lua_stringify(lua_State *L) {
    lua_json_lua_tojson(L, false);

    return 1;
}

int lua_json_tolua(lua_State *L){
    bool unref = false;
    if(lua_isboolean(L, 2)) {
        unref = lua_toboolean(L, 2);
        lua_pop(L, 1);
    }
    _lua_json_tolua(L, unref);

    return 1;
};