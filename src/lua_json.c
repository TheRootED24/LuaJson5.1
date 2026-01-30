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

#include "lua_json.h"

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

const char *Json[] = {
    "\"%s\":",      // JsonKey      0
    "\"%s\":\"%s\"",// ObjString    1
    "\"%s\":%f",    // ObjNumber    2
    "\"%s\":%s",    // ObjBool      3
    "\"%s\":%s",    // ObjNull      4
    "\"%s\"",       // ArrString    5
    "%f",           // ArrNumber    6
    "%s",           // ArrBool      7
    "%s",           // ArrNull      8
    ",",            // JsonNext     9
    "{",            // OpenObj      10
    "}",            // CloseObj     11
    "[",            // OpenArr      12
    "]"             // CloseArr     13
};

json_elm *check_json_elm(lua_State *L, int pos) {
    //dumpstack(L);
    void *ud = luaL_checkudata(L, pos, "JSON.json");
    luaL_argcheck(L, ud != NULL, pos, "`json element' expected");

	return (json_elm*)ud;
};

bool check_next(ref *seen, uintptr_t next) {
	if(next == seen->root) {
		if(seen->depth >= seen->max) 
			return true;
		else 
			seen->depth++;
	}
	seen->ids[seen->elms++] = next;
	return false;
};

int lua_json_elm_tojson(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	//printf("RLEN: %ld\n", elm->rlen);
	ref seen = {0};
	seen.root = (uintptr_t)lua_topointer(L, -1);
	seen.ids[seen.elms++] = seen.root;
	seen.check_next = &check_next;

	seen.ltype = LUA_TUSERDATA;
	seen.b = malloc(elm->rlen + 1); // 1MB should be enough for most cases
	memset(seen.b, 0, (elm->rlen + 1) );
	
	elm->type == JSON_ARRAY_TYPE ? __lua_json_render_elm_array(L, &seen) : __lua_json_render_elm_object(L, &seen);
	size_t lt = strlen(seen.b)+1; // null term
	elm->rlen = lt;
	//elm->rlen = lt;
	printf("seen.b strlen: %ld/%ld\n", lt, elm->rlen);

	lua_pushstring(L, seen.b);
	free(seen.b);
	json_elm *e = check_json_elm(L, 1);
	printf("RENDER LENGTH %ld\nPretty Length: %ld\nNESTED OBJECTS %d\n\n", elm->rlen, elm->plen, e->nestedobjects);

	return 1;

};

int lua_json_elm_tostring(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
    if(elm)
        lua_pushfstring(L, "%s: %p", elm->typename, elm->id);
	else
		fprintf(stderr, "To String Error !!!!!\n");

    return 1;
};

int lua_json_elm_len(lua_State *L) {
    json_elm *elm = check_json_elm(L, 1);
    if(elm) {
        lua_pushinteger(L, elm->nelms);
	}

    return 1;
};

void json_type(parse_elm *elm) {
	const char index = elm->index;
	if (index == '{') {
		elm->type = JSON_OBJECT_TYPE;
		return;
	}
	if (index == '[') {
		elm->type = JSON_ARRAY_TYPE;
		return;
	}

	bool isLong = ((elm->l = _lua_json_get_long(*elm->json, elm->key, -1)) == -1) ? false : true;
	bool isStr = ((elm->str = _lua_json_get_str(*elm->json, elm->key)) == NULL) ? false : true;
	bool isNum = _lua_json_get_num(*elm->json, elm->key, &elm->num);
	bool isBool = _lua_json_get_bool(*elm->json, elm->key, &elm->b);

	if(isNum)
		elm->type = JSON_NUMBER_TYPE;
	else if(isStr)
		elm->type = JSON_STRING_TYPE;
	else if(isBool)
		elm->type = JSON_BOOL_TYPE;
	else if(isLong)
		elm->type = JSON_LONG_TYPE;
	else
		elm->type = JSON_NULL_TYPE; 
		// null and unknown/invalid types all get marked as null types for now
		// only the actual null types we be processed later ..
		// all other types will be silently disgaurded by newindex

	return;
}

bool _isRoot = true;
static int __lua_json_elm_parse(lua_State *L, struct lua_str json, int depth) {
	//printf("Here-->\n");
	//dumpstack(L);
	_isRoot = depth == -1 ? true : false;
	json_elm *jelm = check_json_elm(L, 1);
	// stack {..., env }
	parse_elm elm = {0};
	int idx = 0;
	int n = 0, o = _lua_json_get(json, "$", &n);
	jelm->plen = n;
	if (json.buf[o] == '{' || json.buf[o] == '[') {
		struct lua_str key, val, sub = lua_str_n(json.buf + o, (size_t)n);
		size_t ofs = 0;
		while ((ofs = _lua_json_next(sub, ofs, &key, &val)) > 0) {
			bool isKeyed = key.len > 0 ? true : false;
			elm.isRoot = _isRoot;
			if(elm.isRoot) depth = 1;
			elm.index = val.buf[0];
			
			if (isKeyed) { 
				lua_pushlstring(L, key.buf + 1, key.len - 2);
				elm.key = lua_pushfstring(L, "$.%s", lua_tostring(L, -1));
				lua_pop(L, 2);

				elm.json = &json;
				json_type(&elm);

			}
			else {
				// construct a json object with a dummy key and the value to check type
				lua_pushlstring(L, val.buf, (int)val.len);
				const char *buf = lua_pushfstring(L, "{\"%s\": %s}", "dkey", lua_tostring(L, -1));
				lua_pop(L, 2); 
				//dumpstack(L);
				struct lua_str djson = lua_str(buf);
				elm.json = &djson;
				elm.key = "$.dkey";
				json_type(&elm);	
			}
			switch (elm.type) {
				case JSON_NUMBER_TYPE: {
					if (isKeyed) {
						lua_pushlstring(L, key.buf + 1, key.len - 2);
						lua_pushnumber(L, elm.num);
					}
					else {
						lua_pushnumber(L, idx++);
						lua_pushnumber(L, elm.num);
					}
					//printf("JSON NUMBER\n");
					//dumpstack(L);
					jelm->nums++;
					lua_settable(L, -3);
					break;
				}
				case JSON_BOOL_TYPE: {
					if (isKeyed) {
						lua_pushlstring(L, key.buf + 1, key.len - 2);
						lua_pushboolean(L, elm.b);
					}
					else {
						lua_pushnumber(L, idx++);
						lua_pushboolean(L, elm.b);
					}
					//printf("JSON BOOL\n");
					//dumpstack(L);
					jelm->bools++;
					lua_settable(L, -3);
					break;
				}
				case JSON_LONG_TYPE: {
					if (isKeyed) {
						lua_pushlstring(L, key.buf + 1, key.len - 2);
						lua_pushnumber(L, elm.l);
					}
					else {
						lua_pushnumber(L, idx++);
						lua_pushnumber(L, elm.l);
					}
					//printf("JSON LONG\n");
					//dumpstack(L);
					jelm->longs++;
					lua_settable(L, -3);
					break;
				}
				case JSON_STRING_TYPE: {
					if (isKeyed) {
						lua_pushlstring(L, key.buf + 1, key.len - 2);
						lua_pushstring(L, elm.str);
					}
					else {
						lua_pushnumber(L, idx++);
						lua_pushstring(L, elm.str);
					}
					free(elm.str);
					//printf("JSON STRING\n");
					//dumpstack(L);
					jelm->strings++;
					lua_settable(L, -3);
					break;
				}
				case JSON_NULL_TYPE: {
					if (isKeyed) {
						lua_pushlstring(L, key.buf + 1, key.len - 2);
						lua_pushlstring(L, val.buf, val.len);
					}
					else {
						lua_pushnumber(L, idx++);
						lua_pushlstring(L, val.buf, val.len);
					}
					//printf("JSON BOOL\n");
					//dumpstack(L);
					jelm->nulls++;
					lua_settable(L, -3);
					break;
				}
				case JSON_ARRAY_TYPE: {
					//printf("NEW OBJECT !!!!!!!!!!!!!!!!\n");
					isKeyed ? lua_pushlstring(L, key.buf + 1, key.len - 2) : lua_pushnumber(L, idx++);
					lua_json_elm_parse_array(L);
					jelm->arrays++;
					break;
				}
				case JSON_OBJECT_TYPE: {
					//printf("NEW OBJECT !!!!!!!!!!!!!!!!\n");
					if (isKeyed){
						lua_pushlstring(L, key.buf + 1, key.len - 2);
						lua_json_elm_parse_object(L);
					}
					else {
						lua_pushnumber(L, idx++);
						lua_json_elm_parse_object(L);
					}
					jelm->objects++;
					break;
				}
				break;
			}
			// found nested object or array
			if (*val.buf == '[' || *val.buf == '{') {
				switch (*val.buf) {
					case '[': {
						//jelm->arrays++;
						jelm->nestedarrays++;
						break;
					}
					case '{': {
						jelm->nestedobjects++;	
						break;
					}
				}
				__lua_json_elm_parse(L, val, depth += 1);
			}
			// found the end of the current object / array
			char last = json.buf[ofs];
			if (last == ']' || last == '}') {

				// we're finished parsing when stack size is 1
				bool finished = lua_gettop(L) == 1 ? true : false;

				if (!finished) {
					lua_settable(L, -3);
					depth -= 1;
					idx = 0;
				}
			}
		}
	}
	// all done!! leave the table on the stack and return it
	if (depth == 1) {
		return 1;
	}
	// recursion
	return 0;
};

int lua_json_elm_parse(lua_State *L) {
	const char *elm = luaL_checkstring(L, 1);
	lua_pop(L, 1);
	//printf("JSON: %s\n", elm);
	if(elm[0] == '{') 
		lua_json_object(L);
	else if(elm[0] == '[')
		lua_json_array(L);
	else {
		fprintf(stderr, " (_lua_json_elm_parse) Ivalid Json Element\n");
		return 0;
	}

	struct lua_str json = lua_str(elm); 
	__lua_json_elm_parse(L, json, -1);

	json_elm *jelm = check_json_elm(L, 1);

	printf("PARSED ELM TYPE: %s\nPARSED LENGTH: %ld\n", jelm->typename, jelm->rlen);
	
	return 1;
};

int get_json_val_length(lua_State *L, json_elm *elm) {
	dumpstack(L);
	size_t rlen = 0;
	int vtype = lua_type(L, -1);
	switch(elm->type) {
		case JSON_ARRAY_TYPE: {
			//idx = luaL_checkinteger(L, -2);
	
			switch(vtype) {
				case LUA_TUSERDATA: {
					json_elm *nested = check_json_elm(L, -1);
					nested->root = elm;
					rlen += nested->rlen > 2 ? nested->rlen : 2;
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
					size_t vlen = 0;
					const char *val = lua_tolstring(L, -1, &vlen);
					
					// handle null sentinel here
					// only perform strcmp if length 4 >null<
					if ( vlen == 4 && val[0] == 'n') {
						if (val && (strcmp(val, "null")) == 0) 
							rlen += strlen(val)+1;
					}
					else 
							rlen += strlen(val)+3;

					break;
				}
				//rlen++;
			}
			break;
		}
		case JSON_OBJECT_TYPE: {
			//int vtype = lua_type(L, -1);
			//const char *key = lua_tostring(L, -2);
			//_lua_json_elm_key_to_idx(L, key, true);
			//idx = luaL_checkinteger(L, -1);
			//lua_pop(L, 1);

			switch(vtype) {
				case LUA_TUSERDATA: {
					const char *val = lua_pushfstring(L, Json[JsonKey], elm->key);
					rlen += strlen(val);
					lua_pop(L, 1);

					json_elm *nested = check_json_elm(L, -1);
					nested->root = elm;
					rlen += nested->rlen > 2 ? nested->rlen : 2;
					break;
				}
				case LUA_TNUMBER: {
					const char *val = lua_pushfstring(L, Json[ObjNumber], elm->key, lua_tonumber(L, -1));
					rlen += strlen(val)+1;
					lua_pop(L, 1);
					break;
				}
				case LUA_TBOOLEAN: {
					const char *val = lua_pushfstring(L, Json[ObjBool], elm->key, btoa(lua_toboolean(L, -1)));
					rlen += strlen(val)+1;
					lua_pop(L, 1);
					break;
				}
				case LUA_TSTRING: {
					size_t vlen = 0;
					const char *val = lua_tolstring(L, -1, &vlen);
					
					// handle null sentinel here
					// only perform strcmp if length 4 >null<
					if (( vlen ) == 4 && val[0] == 'n') { 
						if (strcmp(val, "null") == 0) {
							//printf("VAL:%s == null\n", val);
							vtype = LUA_TNULL;
							val = lua_pushfstring(L, Json[ObjNull], elm->key, lua_tostring(L, -1));
						}
					}
					else 
							val = lua_pushfstring(L, Json[ObjString], elm->key, lua_tostring(L, -1));

					rlen += strlen(val)+1;
					lua_pop(L, 1);
					break;
				}
				//rlen++;
			}
			break;
		}
	}
	//lua_settop(L, 0);
	lua_pushnumber(L, rlen);

	return 1;
}

int lua_json_elm_newindex(lua_State *L) {
	//printf("****************************************** ELM NEW INDEX ***************************************************\n");
	//dumpstack(L);
	json_elm *elm = check_json_elm(L, -3);
	size_t rlen = 0;
	int idx = 0;
	//dumpstack(L);
	int ktype = lua_type(L, -2);
	//int vtype = lua_type(L, -1);
	
	if(ktype == LUA_TTABLE) return 0;
	//printf("ELM TYPE: %s\n", elm->typename);
	switch(elm->type) {
		case JSON_ARRAY_TYPE: {
			idx = luaL_checkinteger(L, -2);
			get_json_val_length(L, elm);
			rlen += lua_tonumber(L, -1);
			lua_pop(L, 1);
			break;
			/*switch(vtype) {
				case LUA_TUSERDATA: {
					json_elm *nested = check_json_elm(L, -1);
					nested->root = elm;
					rlen += nested->rlen > 2 ? nested->rlen : 2;
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
					size_t vlen = 0;
					const char *val = lua_tolstring(L, -1, &vlen);
					
					// handle null sentinel here
					// only perform strcmp if length 4 >null<
					if ( vlen == 4 && val[0] == 'n') {
						if (val && (strcmp(val, "null")) == 0) 
							rlen += strlen(val)+1;
					}
					else 
							rlen += strlen(val)+3;

					break;
				}
				//rlen++;
			}*/
			break;
		}
		case JSON_OBJECT_TYPE: {
			//int vtype = lua_type(L, -1);
			const char *key = lua_tostring(L, -2);
			_lua_json_elm_key_to_idx(L,key, true);
			idx = luaL_checkinteger(L, -1);
			lua_pop(L, 1);
			elm->key = key;
			get_json_val_length(L, elm);
			rlen += lua_tonumber(L, -1);
			lua_pop(L, 1);
			break;

			/*switch(vtype) {
				case LUA_TUSERDATA: {
					const char *val = lua_pushfstring(L, Json[JsonKey], key);
					rlen += strlen(val);
					lua_pop(L, 1);

					json_elm *nested = check_json_elm(L, -1);
					nested->root = elm;
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
					size_t vlen = 0;
					const char *val = lua_tolstring(L, -1, &vlen);
					
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
				//rlen++;
			}
			break;*/
		}
	}
	//dumpstack(L);
	lua_getfenv(L, 1);
	// stack {..., udata, key, val, env}
	lua_insert(L, 2);
	lua_settop(L, 4);
	// stack { udata, env, key, val }
	//dumpstack(L);
	if(ktype == LUA_TNUMBER) {
		idx = luaL_checkinteger(L, -2)+1;
		lua_pushinteger(L, idx);
		lua_insert(L, -2);
		lua_remove(L, -3);
	}
	//dumpstack(L);
	lua_rawset(L, -3); 
	//idx = elm->type == JSON_ARRAY_TYPE ? (idx + 1) : idx;
	if(elm && (idx > (int)elm->nelms))
		elm->nelms++;
	
	elm->rlen += rlen;
	

    return 0;
};

int lua_json_elm_index(lua_State *L) {
	//printf("****************************************** INDEX ***************************************************\n");
	json_elm *elm = check_json_elm(L, 1);
	// stack { udata, key }
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


	// stack { udata, key, env }
	lua_getfenv(L, 1);
	lua_insert(L, 2);
	// stack { udata, env, key }


	elm->type == JSON_ARRAY_TYPE ? lua_rawgeti(L, 2, (lua_tointeger(L, 3) + 1)) : lua_rawget(L, 2);

    return 1;
};

int test_func(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	printf("Elm Type: %s Elm Size: %ld\n", elm->typename, elm->nelms);

	return 0;
}

static const struct luaL_reg lua_json_methods[] = {
	{ "parse",	    lua_json_elm_parse	    },
	{ "stringify",	lua_json_elm_tojson	    },
	{ "__tostring",	lua_json_elm_tostring	},
    { "__len",		lua_json_elm_len		},
	{NULL, NULL}
};

int luaopen_JSON(lua_State *L){
	luaL_newmetatable(L, "JSON.json");
	lua_pushvalue(L, -1);	// pushes the metatable
	lua_setfield(L, -2, "__index");	// metatable.__index = metatable
	luaL_register(L, LUA_JSON, lua_json_methods);

    lua_json_open_array(L);
    lua_json_open_object(L);

    return 1;
}
