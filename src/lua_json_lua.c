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

//int __lua_json_render_lua_object(lua_State *L, struct ref *seen) 
typedef struct lua_parser {
    size_t raw_size, arr_size, obj_size;
    uint8_t type;
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
					
					if(np.type % 2 != 0) 
					{
						lua_json_elm_parse_object(L);
						// stack { ..., t, elm, t, elm }
						lua_json_parse_lua_object(L, &np);
					}
					else 
					{
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
				default: {
					lua_pop(L, 1);
					break;
				}
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
			json_elm *e = check_json_elm(L, -1, false);
			if(e->nelms > 0) {
				lua_pushinteger(L, (int)p->arr_size);
				// stack { ..., t, elm, t, elm, i }
				lua_replace(L, -3);
				// stack { ..., t, elm, i, elm }
				lua_settable(L, -3);
			}
			else
				lua_pop(L, 2);
				// stack { ..., t, elm }
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
			// handle them at the end of parse
			if(lua_isnumber(L, -2)) { 
				lua_pop(L, 1);
				continue;
			}

			const char *key = lua_tostring(L, -2);
			int type = lua_type(L, -1);

			switch(type) {
				case LUA_TTABLE:
				{

					// stack { ..., t1, elm1, key, t }
					lua_parser np = {0};
					lua_json_init_lua_parser(L, &np);

					if(np.type % 2 != 0) 
					{
						lua_json_elm_parse_object(L);
						// stack { ..., t1, elm1, key, t2, elm2 }
						lua_json_parse_lua_object(L, &np);
					}
					else 
					{
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

				default: {
					lua_pop(L, 1);
					break;
				}
			}
		}
	}

    	if(p->mixed > 0 ) {
		// stack { ..., t, elm }
		p->mixed = 0;
		lua_pushvalue(L, -2);
		// stack { ..., t, elm, t }
		lua_json_elm_parse_array(L);
		// stack { ..., t, elm, t, elm }
		lua_json_parse_lua_array(L, p);
		// stack { ..., t, elm, t, elm }
		json_elm *e = check_json_elm(L, -1, false);
		if(e && e->nelms > 0) {
			// stack { ..., t, elm, t, elm }
			lua_pushstring(L, p->mixed_name);
			// stack { ..., t, elm, t, elm, key }
			lua_replace(L, -3);
			// stack { ..., t, elm, key, elm }
			lua_settable(L, -3);
		}
		else
			lua_pop(L, 2);
			// stack { ..., t, elm }
    	}
				

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
	bool no_mixed = false, verbose = false;
	char *mixed_name = NULL;

	int nargs = lua_gettop(L);

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
			if (lua_isstring(L, -1)) 
			{
				size_t len = 0;
				parser.mixed_name = lua_tolstring(L, -1, &len);
				if(len > 0) 
				{
					mixed_name = malloc(len+1);
					strncpy(mixed_name, parser.mixed_name, len);
					mixed_name[len]=0;
				}

				lua_pop(L, 1);
			}
		}
		
		if(lua_isstring(L, -1))
		{
			size_t opt_len = 0;
			const char *opt = lua_tolstring(L, -1, &opt_len);

			type = opt[1] == 'p' ? -1 // parse mode 
			     : opt[1] == 'a' ? (uint8_t)JSON_ARRAY_TYPE 
			     : opt[1] == 'o' ? (uint8_t)JSON_OBJECT_TYPE 
			     : type;

			verbose = opt[1] == 'v' || (opt_len >=3 && opt[3] == 'v') ?  true : false;

			lua_pop(L, 1);
		}
	}
	
	if(no_mixed) lua_settop(L, 2);

	if (!lua_istable(L, -1))
		return luaL_error(L, "ERROR: 'expected a table, got %s", lua_typename(L, lua_type(L, -1)));

	if (lua_istable(L, 1) && lua_istable(L, 2))
		lua_remove(L, 1);

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

	if (DEBUG || verbose) {
	int mixed_size = parser.type % 2 == 0 ? (int)(parser.raw_size - parser.arr_size) 
					      : (int)(parser.raw_size - parser.obj_size);

		printf("Result:[ type: %s | object len: %ld | array len: %ld | mixed: %s | fixups: %d ]\n",
			parser.type % 2 == 0 ? "array" : parser.type != 0 ? "object" : "unknown",
			parser.type % 2 != 0 ? parser.obj_size : (parser.raw_size-parser.arr_size)  , parser.arr_size,
			btoa(mixed_size > 1 ? true : false), 
			parser.type > 5 ? mixed_size : 0
		);
	}

	if(mixed_name) free(mixed_name);
	
	if(type == -1) {
		lua_insert(L, 2);
		lua_pop(L, 2);
	}
	else
		lua_replace(L, -2);

	json_elm *tref = check_json_elm(L, -1, false);
	tref->is_ref = true;
	
	return 1;

}

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

	ktype = keys == 0 ? JSON_ARRAY_TYPE 
			  : idxs == 0 ? JSON_OBJECT_TYPE 
			  : keys > idxs ? JSON_NESTED_OBJECT_TYPE 
			  : JSON_NESTED_ARRAY_TYPE; 

	if(DEBUG) printf("Table Type: [ %s ] Table Length: [ %ld ]\n", ktype == 5 ? "object" 
										  : ktype == 4 ? "array" 
										  : ktype == 7 ? "object (mixed)" 
										  : "array (mixed)",  size);
	
    	lua_pushinteger(L, ktype);
	lua_pushnumber(L, size);

	return 2;
}

int lua_json_lua_stringify(lua_State *L)
{
	uint8_t nargs = ((uint8_t)lua_gettop(L) - 1);
	bool esc = false;
	int8_t mode =  lua_istable(L, 2) ? MARSHAL_LUA : lua_isuserdata(L, 2) ? MARSHAL_JSON : -1;


	if(nargs > 1) {
		const char *opt = NULL;
		esc = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;

		if(lua_isstring(L, (esc ? -2 : -1))) {
			opt = lua_tostring(L, (esc ? -2 : -1));
			mode = opt && opt[0] == 'j' ? MARSHAL_JSON : opt && opt[0] == 'l' ? MARSHAL_LUA : mode;
		}

		lua_settop(L, 2);
	}

	if(lua_istable(L, 1) && !lua_isuserdata(L, 2)) {
		lua_json_parse_lua (L);

		json_elm *elm = check_json_elm(L, -1, false);
		elm->mode = mode != -1 ? mode : elm->mode;
		elm->escape = esc;

		elm->ops->stringify(L);

		if(lua_isstring(L, -1))
			printf("%s\n", lua_tostring(L, -1));

		return 1;
	}
	
	if(lua_isuserdata(L,1) || lua_isuserdata(L, 2)) {
		if(lua_istable(L, 1)) lua_replace(L, 1);

		json_elm *elm = check_json_elm(L, 1, false);
		uint8_t orig_mode = elm->mode;
		bool orig_esc = elm->escape;

		elm->mode = mode != -1 ? mode : elm->mode;
		elm->escape = esc;

		elm->ops->stringify(L);

		elm->mode = orig_mode;
		elm->escape = orig_esc;

		if(lua_isstring(L, -1))
			printf("%s\n", lua_tostring(L, -1));

		return 1;
	}

	return 0;
}