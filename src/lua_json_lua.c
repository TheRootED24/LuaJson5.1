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
extern const char *Json[];


//int __lua_json_render_lua_object(lua_State *L, struct ref *seen) 
typedef struct lua_parser {
    size_t raw_size, arr_size, obj_size;
    uint8_t type, ptype;
    uint16_t mixed;
    const char *mixed_name;

} lua_parser;

// FOR FIXUPS IN ARRAYS ---> ALL KEY/VAL PAIRS GO INTO NESTED OBJECT AT END OF LIST
// FOR FIXUPS IN OBJECTS ---> ALL KEYLESS VALS GO INTO ARRAY AT END OF MAP

int lua_json_init_lua_parser(lua_State *L, lua_parser *parser) {
    parser->arr_size = lua_objlen(L, -1);
    lua_json_lua_table_len(L);
    // stack { t, size, type }
    parser->type = luaL_checkinteger(L, -2);
    parser->raw_size = (luaL_checknumber(L, -1));
    lua_pop(L, 2);
    // stack { t }
    parser->obj_size = (parser->raw_size - parser->arr_size);
    bool mixed = parser->type > 5 ? true : false;
    parser->mixed = mixed && parser->type % 2 == 0 ? parser->obj_size : mixed ? parser->arr_size : 0; 

    return 0;
}

int lua_json_parse_lua_array(lua_State *L, lua_parser *p) {
	if(lua_istable(L, -2) && lua_isuserdata(L, -1)) {
	// stack { ..., t, elm }
	if(DEBUG) printf("*********************************** PARSE LUA ARRAY *********************************** \n");
	for(int i = 1; i <= (int)p->arr_size; i++) {
	    // stack { ..., t, elm }
	    lua_rawgeti(L, -2, i);

	    int type = lua_type(L, -1);

	    switch(type) {
		case LUA_TTABLE:
		{
			// stack { ..., t, elm, t }
			lua_parser np = {0};
			lua_json_init_lua_parser(L, &np);
		       if(np.type % 2 != 0) {
			    lua_json_elm_parse_object(L);
			    // stack { ..., t, elm, t, elm }
			    lua_json_parse_lua_object(L, &np);
		       }
		       else {
			    lua_json_elm_parse_array(L);
			    // stack { ..., t, elm, t, elm }
			    lua_json_parse_lua_array(L, &np);
		       }

			// stack { ..., t, elm, t, elm }
			lua_pushinteger(L, (i - 1));
			// stack { ..., t, elm, t, elm, i }
			lua_replace(L, -3);
			// stack { ..., t, elm, i, elm }
			lua_settable(L, -3);
			// stack { ..., t, elm }
			break;
		}
		case LUA_TSTRING: 
		case LUA_TNUMBER:
		case LUA_TBOOLEAN:
		{
		     lua_pushinteger(L, (i - 1));
			 // stack { ..., t, elm, val, i }
			lua_insert(L, -2);
			// stack { ..., t, elm, i, val }
			lua_settable(L, -3);
			break;
		}
		// stack { ..., t, elm }
	    }
	}

	if(p->mixed > 0 ) {
	    // stack { ..., t, elm }
	    p->mixed = 0;
	    lua_pushvalue(L, -2);
	    // stack { ..., t, elm, t }
	    lua_json_elm_parse_object(L);
	    // stack { ..., t, elm, t, elm }
	    lua_json_parse_lua_object(L, p);
	    // stack { ..., t, elm, t, elm }
	    lua_pushinteger(L, (int)p->arr_size);
	    // stack { ..., t, elm, t, elm, i }
	    lua_replace(L, -3);
	    // stack { ..., t, elm, i, elm }
	    lua_settable(L, -3);
	}
	}
    // stack { ..., t, elm }

	// all done !!
    if(lua_gettop(L) == 2)
     // stack { t, elm }
	return 1;
    // recursion
    return 0;
    
};

int lua_json_parse_lua_object(lua_State *L, lua_parser *p) {
   
    // stack { ..., t, elm }
	if(lua_istable(L, -2) && lua_isuserdata(L, -1)) {
	if(DEBUG) printf("*********************************** PARSE LUA OBJECT *********************************** \n");
	lua_pushnil(L);
	while(lua_next(L, -3) != 0) { 
	    // omit array items for now
	    if(lua_isnumber(L, -2)) { 
		lua_pop(L, 1);
		continue;
	    }

	    int type = lua_type(L, -1); 
	    const char *key = lua_tostring(L, -2);

	    switch(type) {
		case LUA_TTABLE:
		{

			// stack { ..., t1, elm1, key, t }
			lua_parser np = {0};
			lua_json_init_lua_parser(L, &np);

			if(np.type % 2 != 0) {
			    lua_json_elm_parse_object(L);
			    // stack { ..., t1, elm1, key, t2, elm2 }
			    lua_json_parse_lua_object(L, &np);
			}
			else {
			    lua_json_elm_parse_array(L);
			    // stack { ..., t1, elm1, key, t2, elm2 }
			    lua_json_parse_lua_array(L, &np) ;
			}

			// stack { ..., t1, elm1, key, t2, elm2 }
			lua_pushstring(L, key);
			// stack { ..., t1, elm1, key, t2, elm2, key }
			lua_replace(L, -3);
			// stack { ..., t1, elm1, key, key, elm2 }
			lua_settable(L, -4);
			// stack { ..., t1, elm1, key }
			break;
		}
		case LUA_TSTRING: 
		case LUA_TNUMBER:
		case LUA_TBOOLEAN:
		{
			lua_pushstring(L, key);
			// stack { ..., t, elm, key, val, key }
			lua_insert(L, -3);
			// stack { ..., t, elm, key, key, val }
			lua_settable(L, -4);
			// stack { ..., t, elm, key }
			break;
		}
		default:
		    break;
	    }
	}
	}

    // stack { ..., t, elm }
    if(p->mixed > 0 ) {
	// stack { ..., t, elm }
	p->mixed = 0;
	lua_pushvalue(L, -2);
	// stack { ..., t, elm, t }
	lua_json_elm_parse_array(L);
	// stack { ..., t, elm, t, elm }
	lua_json_parse_lua_array(L, p);
	// stack { ..., t, elm, t, elm }
	lua_pushstring(L, p->mixed_name);
	// stack { ..., t, elm, t, elm, key }
	lua_replace(L, -3);
	// stack { ..., t, elm, key, elm }
	lua_settable(L, -3);
    }
    // stack { ..., t, elm }
    
	// all done !!
    if(lua_gettop(L) == 2)
     // stack { t, elm }
	return 1;
    // recursion
    return 0;
};

int lua_json_parse_lua(lua_State *L)
{
	int8_t type = 0;
	bool no_mixed = false;
	char *mixed_name = NULL;

	int nargs = lua_gettop(L);

	dumpstack(L, "PARSE LUA START");
	lua_parser parser = {0};

	if (nargs > 2)
	{
		if (lua_isboolean(L, -1))
		{
			no_mixed = lua_toboolean(L, -1);
			lua_pop(L, 1);
		}

		if (!no_mixed && lua_isstring(L, -2))
		{
			if (lua_isstring(L, -1)) {
				size_t len = 0;
				parser.mixed_name = lua_tolstring(L, -1, &len);
				if(len > 0) {
					mixed_name = malloc(len+1);
					strncpy(mixed_name, parser.mixed_name, len);
					mixed_name[len]=0;
				}
				lua_pop(L, 1);
			}
		}
		
		if(lua_isstring(L, -1))
		{
			const char *opt = lua_tostring(L, -1);
			type = opt[1] == 'p' ? -1 : opt[1] == 'a' ? (uint8_t)JSON_ARRAY_TYPE : (uint8_t)JSON_OBJECT_TYPE;
			if (DEBUG) printf("TYPE: %s", type == JSON_ARRAY_TYPE ? "array" : "object");
			lua_pop(L, 1);
		}
	}
	
	if(no_mixed) lua_settop(L, 2);

	if (!lua_istable(L, -1))
		return luaL_error(L, "ERROR: 'expected a table, got %s", lua_typename(L, lua_type(L, -1)));

	if (lua_istable(L, 1) && lua_istable(L, 2))
		lua_remove(L, 1);


	// stack { t }
	//lua_pushvalue(L, -1);
	// stack { t, t }
	// int vtable = luaL_ref(L, LUA_REGISTRYINDEX );
	// stack { t }
	lua_json_init_lua_parser(L, &parser);
	
	if (type > 0)
		parser.type = type;

	if(no_mixed)
		parser.mixed = false;

	if(mixed_name)
		parser.mixed_name = mixed_name;
	else
		parser.mixed_name = mixed_keys;

	if (parser.type % 2 != 0)
	{
		lua_json_elm_parse_object(L);
		// stack { t, elm }
		lua_json_parse_lua_object(L, &parser);
	}
	else
	{
		lua_json_elm_parse_array(L);
		// stack {t, elm }
		lua_json_parse_lua_array(L, &parser);
	}

	if (!DEBUG) {
	int mixed_size = parser.type % 2 == 0 ? (int)(parser.raw_size - parser.arr_size) 
					      : (int)(parser.raw_size - parser.obj_size);

		printf("Result:[ type: %s | object len: %ld | array len: %ld | mixed: %s | fixups: %d ]\n",
			parser.type % 2 == 0 ? "array" : parser.type != 0 ? "object" : "unknown",
			parser.obj_size, parser.arr_size,
			btoa(mixed_size > 1 ? true : false), 
			mixed_size
		);
	}
	if(mixed_name) free(mixed_name);
	
	if(type == -1) {
		lua_insert(L, 2);
		lua_pop(L, 2);
	}
	else
		lua_replace(L, -2);

	return 1;

}

int __lua_json_render_lua_object(lua_State *L, struct ref *seen)
{
	if(DEBUG) printf("*********************************** RENDER LUA OBJECT *********************************** \n");
	const char *key = NULL, *v = NULL;
	if (lua_istable(L, -1))
	{
		strcat(seen->b, Json[OpenObj]);
		// push a copy of the table
		int idx = 0;
		lua_pushvalue(L, -1);
		lua_pushnil(L);
		while (lua_next(L, -2) != 0)
		{
			// omit object lookup table at index 0
			/*if (lua_isnumber(L, -2) && lua_tonumber(L, -1) == 0)
			{
				lua_pop(L, 1);
				continue;
			}*/
			key = lua_tostring(L, -2);

			if (idx >= 1)
				strcat(seen->b, Json[Next]);
			int type = lua_type(L, -1);
			switch (type)
			{
			case LUA_TTABLE:
			{
				v = lua_pushfstring(L, Json[ObjKey], key);
				strcat(seen->b, v);
				lua_pop(L, 1);
				v = NULL;

				lua_objlen(L, -1) > 0 ? __lua_json_render_lua_array(L, seen) 
						      : __lua_json_render_lua_object(L, seen);
				lua_pop(L, 1);
				break;
			}
			case LUA_TSTRING:
			{
				size_t vlen = 0;
				const char *val = lua_tolstring(L, -1, &vlen);
				int vtype = LUA_TSTRING;

				// handle null sentinel here
				if ((vlen) == 4 && val[0] == 'n')
					// only perform strcmp if length 4 >null<
					if (val && (strcmp(val, "null")) == 0)
						vtype = LUA_TNULL;

				v = vtype != LUA_TNULL ? lua_pushfstring(L, Json[ObjString], key, lua_tostring(L, -1)) : lua_pushfstring(L, Json[ObjNull], key, lua_tostring(L, -1));
				break;
			}
			case LUA_TNUMBER:
			{
				v = lua_pushfstring(L, Json[ObjNumber], key, lua_tonumber(L, -1));
				break;
			}
			case LUA_TBOOLEAN:
			{
				if (!lua_istable(L, -1))
					v = lua_pushfstring(L, Json[ObjBool], key, btoa(lua_toboolean(L, -1)));
				break;
			}
			case LUA_TNULL:
			{
				if (!lua_istable(L, -1))
					v = lua_pushfstring(L, Json[ObjNull], key, lua_tostring(L, -1));
				break;
			}
			}
			if (v)
			{
				strcat(seen->b, v);
				lua_pop(L, 2);
			}
			idx++;
		}
		strcat(seen->b, Json[CloseObj]);
	}

	if (lua_gettop(L) == 1)
		return 1;

	return 0;
};

int __lua_json_render_lua_array(lua_State *L, struct ref *seen) {
   printf("*********************************** RENDER LUA ARRAY *********************************** \n");
   //dumpstack(L);
	if(lua_istable(L, -1)) {
		int len = lua_objlen(L, -1);
		strcat(seen->b, Json[OpenArr]);
	//dumpstack(L);
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
	    if(len - a >= 1) strcat(seen->b, Json[Next]);
	}
	strcat(seen->b, Json[CloseArr]);
		
	}
	// all done !!
	if(lua_gettop(L) == 1)
		return 1;

	// recursion
	return 0;
};
/*
static size_t __lua_json_lua_table_len(lua_State *L, int tbl_pos) {
	size_t len = 0;
	if(tbl_pos != -1) lua_pushvalue(L, tbl_pos);
		if(lua_istable(L, -1))
			len = __lua_json_lua_table_len(L, -1);

	return len;
}

// c side entry
static size_t __lua_json_lua_table_type(lua_State *L, int tbl_pos) {
	int type = 0;
	if(tbl_pos != -1) lua_pushvalue(L, tbl_pos);
		if(lua_istable(L, -1))
			type = __lua_json_lua_table_type(L, -1);

	return type;
}
*/

int lua_json_lua_tojson(lua_State *L, bool parse) {
	size_t size = lua_objlen(L, -1);
	lua_json_lua_table_len(L); 
	//size_t tlen = lua_tonumber(L, -1);
	//int ktype = lua_tointeger(L, -2);
	lua_pop(L, 2);
	int type = size > 0 ? JSON_ARRAY_TYPE : JSON_OBJECT_TYPE;
	ref seen = {0};
	seen.root = (uintptr_t)lua_topointer(L, -1);
	seen.last = seen.root;
	//seen.check_next = &check_next;

	seen.ltype = LUA_TUSERDATA;
	seen.b = malloc(MAX_LUA_SIZE+1); // 1MB by default (adjust in "lua_json_lua.h" to suite your needs)
	memset(seen.b, 0, MAX_LUA_SIZE);

	type == JSON_ARRAY_TYPE ? __lua_json_render_lua_array(L, &seen) : __lua_json_render_lua_object(L, &seen);
	type == JSON_ARRAY_TYPE ? lua_pop(L, 2) : lua_pop(L, 3);
	lua_pushstring(L, seen.b);
	free(seen.b);
	//dumpstack(L);

	if(lua_isnil(L, 1)) lua_remove(L, 1);
	if(parse) lua_json_elm_parse(L);

		return 1;
};

int lua_json_lua_table_len(lua_State *L) {
	int keys = 0, idxs = 0, ktype = 0;
	size_t size = 0;
	if (lua_istable(L, -1)) {
		lua_pushnil(L);

		while (lua_next(L, -2) != 0 && !lua_isnil(L, -2)) {
			ktype = lua_type(L, -2);
			ktype == LUA_TSTRING ? keys++ : ktype == LUA_TNUMBER ? idxs++ : ktype; 
			lua_pop(L, 1);
			size++;
		}

	}

	ktype = keys == 0 ? JSON_ARRAY_TYPE :  idxs == 0 ? JSON_OBJECT_TYPE : keys > idxs ? JSON_NESTED_OBJECT_TYPE : JSON_NESTED_ARRAY_TYPE; 
	if(DEBUG) printf("Table Type: [ %s ] Table Length: [ %ld ]\n", ktype == 5 ? "object" : ktype == 4 ? "array" : ktype == 7 ? "object (mixed)" : "array (mixed)",  size);
	
    lua_pushinteger(L, ktype);
	lua_pushnumber(L, size);

	return 2;
}

int lua_json_lua_is_mixed(lua_State *L) {
	int ktype = 0, last = 0, idx = 0;
	if(lua_istable(L, -1)) {
		size_t len = lua_objlen(L, -1);
		lua_pushvalue(L, -1);
		lua_pushnil(L);

		while (lua_next(L, -2) != 0) {
			if (lua_isnil(L, -2))
				break;
			else
				ktype = lua_type(L, -2);

			if((idx > 0 && ktype != last) || (len > 0 && ktype == LUA_TSTRING)) {
				if(DEBUG) printf("Table Type: [ %s ]\n", "mixed");
				lua_pop(L, 3);
				lua_pushboolean(L, true);
				return 1;
			}
	
			lua_pop(L, 1);
			last = ktype;
			idx++;
		}

		lua_pop(L, 2);
	}
	if(DEBUG) printf("Table Type: [ %s ]\n", last == LUA_TSTRING ? "object" : "array");
	lua_pushboolean(L, false);

	return 1;
}

// omit element propertys during conversion
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

// convert a lua json element to a regular lua table
static int _lua_json_tolua(lua_State *L, bool unref) {
   // json_elm *elm = check_json_elm(L, 1);
    lua_getfenv(L, 1);
    // just pass reference to env table
    if(!unref) return 1;
    // create a copy of the element and unref env table
    lua_json_lua_tojson(L, true);
    // return its env table 
    lua_getfenv(L, -1);

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