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

#include "lua_json_object.h"
extern const char *marshal_json[], *marshal_lua[];
static int lua_json_handle_table(lua_State *L);

// C function to implement foreach(elm, func)
static int
lua_json_object_foreach(lua_State *L) {
	//int nargs = lua_gettop(L);
	check_json_elm(L, 1, false);

	// Expects function at index 2
	luaL_checktype(L, 2, LUA_TFUNCTION);
	// stack { elm, func }
	get_json_table(L, 1);
	// stack { elm, func, env }
	lua_insert(L, 2);
	// stack { elm, env, func }
	lua_pushnil(L);
	// stack { elm, env, func, nil }
	while (lua_next(L, 2) != 0)
	{
		lua_pushvalue(L, 3);  // push function
		// stack { elm, env, func, key, val, func }
		lua_pushvalue(L, -3); // push key
		// stack { elm, env, func, key, val, func, key }
		lua_pushvalue(L, -3); // push value
		// stack { elm, env, func, key, val, func, key, val }
		// 4. Call function(key, value)
		if (lua_pcall(L, 2, 1, 0) != 0) // call the function
			return lua_error(L); // Propagate error
		
		// stack { elm, env, func, key, val, result }
		if (!lua_isnil(L, -1))
			return 1; // Stop and return the value
		
		// Pop result and the value, leaving the key for next iteration
		lua_pop(L, 2); 	
		// stack { elm, env, func, key }
	}

	// stack { elm, env, func, key }
	lua_pop(L, 3);
	// stack { elm, env, func }
	//lua_replace(L, -2);
	// stack { elm }

	return 0; // Iteration finished naturally
};

/* static int lua_json_object_sort(lua_State *L) {
	int nargs = lua_gettop(L);
	check_json_elm(L, 1, false);

	bool has_func = nargs > 1 ? lua_isfunction(L, 2) ? true : false : false;
	if(has_func) luaL_checktype(L, 2, LUA_TFUNCTION);
	// stack { elm, user_func }
	lua_getglobal(L, "table");
	// stack { elm, user_func, table_lib }
	lua_getfield(L, -1, "sort");
	// stack { elm, user_func, table_lib, sort_func }
	get_json_table(L, 1);
	// stack { elm, user_func, table_lib, sort_func, env }
	lua_rawgeti(L, -1, 0);
	// stack { elm, user_func, table_lib, sort_func, env, keys }
	lua_remove(L, -2);
	// stack { elm, user_func, table_lib, sort_func, keys }
	if(has_func) lua_pushvalue(L, 2);
	// stack { elm, user_func, table_lib, sort_func, keys, comp }
	if (lua_pcall(L, nargs, 0, 0) != 0){
		// Handle error (e.g., table is not a sequence or has non-comparable types)
		const char *err = lua_tostring(L, -1);
		fprintf(stderr, "Sort error: %s\n", err);
		lua_pop(L, 1);

		return 0;
	}
	// Clean up: pop the 'table' library from the stack
	lua_pop(L, 1);

	return 0;
} */

static int lua_json_object_push(lua_State *L)
{
	check_json_elm(L, 1, false);
	luaL_checkstring(L, 2);

	if(lua_isstring(L, 2) && !lua_isnil(L, 3))
		lua_settable(L, -3);

	return 0;
}

static int lua_json_object_pop(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1, false);
	elm->idx = elm->nelms;
	int ret = 0;

	if((bool)elm->opts->idx_to_key(elm)) {
		// stack { elm }
		get_json_table(L, 1);
		// stack { elm, env }
		lua_pushstring(L, elm->key);
		// stack { elm, env, key }
		lua_rawget(L, -2);
		// stack { elm, env, val }
		lua_replace(L, -2);
		// stack { elm, val }
		lua_pushvalue(L, -2);
		// stack { elm, val, elm }
		lua_pushstring(L, elm->key);
		// stack { elm, val, elm, key }
		lua_pushnil(L);
		// stack { elm, val, elm, key, nil }
		lua_settable(L, -3);
		// stack { elm, val, elm }
		lua_pop(L, 1);
		// stack { elm, val }
		ret = 1;
	}

	elm->idx = 0;
	elm->key = NULL;

	return ret;
}

static int lua_json_object_shift(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1, false);
	elm->idx = 1;

	if((bool)elm->opts->idx_to_key(elm)) {
		// stack { elm }
		get_json_table(L, 1);
		// stack { elm, env }
		lua_pushstring(L, elm->key);
		// stack { elm, env, key }
		lua_rawget(L, -2);
		// stack { elm, env, val }
		lua_replace(L, -2);
		// stack { elm, val }
		lua_pushvalue(L, -2);
		// stack { elm, val, elm }
		lua_pushstring(L, elm->key);
		// stack { elm, val, elm, key }
		lua_pushnil(L);
		// stack { elm, val, elm, key, nil }
		lua_settable(L, -3);
		// stack { elm, val, elm }
		lua_pop(L, 1);
		// stack { elm, val }
		return 1;
	}

	elm->idx = 0;
	elm->key = NULL;

	return 0;
}

static int lua_json_object_reverse(lua_State *L)
{
	int nargs = lua_gettop(L);
	json_elm *elm = check_json_elm(L, 1, false);
	// stack { elm }
	int start = 1, end = (int)elm->nelms;
	
	if(nargs > 2) {
		if(lua_isnumber(L, 3))
		{
			elm->idx = luaL_checkinteger(L, 3);
			elm->opts->check_idx(elm);
			end = elm->idx;
			lua_pop(L, 1);
		}
		else 
		{
			elm->key = luaL_checkstring(L, 3);
			end = elm->opts->key_to_idx(elm, false);
			elm->opts->check_idx(elm);
			lua_pop(L, 1);
		}
		nargs--;
	}

	if(nargs > 1) {
		if(lua_isnumber(L, 2))
		{
			elm->idx = luaL_checkinteger(L, 2);
			elm->opts->check_idx(elm);
			start = elm->idx;
			lua_pop(L, 1);
		}
		else 
		{
			elm->key = luaL_checkstring(L, 2);
			start = elm->opts->key_to_idx(elm, false);
			elm->opts->check_idx(elm);
			lua_pop(L, 1);
		}
		nargs--;
	}
	
	if(start == end) 
		return luaL_error(L, "Cannot shift %d to %d ..its already there ;)", start, end);

	if(nargs > 1) lua_pop(L, (nargs -1));

	// stack { elm , move }
	lua_pushlightuserdata(L, (void*)elm->env_id);
	// stack { elm, move, env_id }
	lua_rawget(L, LUA_REGISTRYINDEX);
	// stack { elm, move, vtable }
	lua_getfield(L, -1, "keys");
	// stack { elm, move, vtable, keys }
	lua_rawgeti(L, -1, 0); // get the map
	// stack { elm, vtable, keys, map }

	for (int s = start, e = end; s < e; s++, e--)
	{
		// stack { elm, vtable, keys, map }
		lua_rawgeti(L, -2, s); // Get element at start
		// stack { elm, vtable, keys, map, key[1] }
		lua_rawgeti(L, -3, e); // Get element at end
		// stack { elm, vtable, keys, map, key[1], key[#keys] }
		lua_pushvalue(L, -1);
		// stack { elm, vtable, keys, map, key[1], key[#keys], key[#keys] }
		lua_rawseti(L, -5, s); // Move end to start
		// stack { elm, vtable, keys, map, key[1], key[#keys] }
		lua_pushinteger(L, s);
		// stack { elm, vtable, keys, map, key[1], key[#keys], s }
		lua_rawset(L, -4);
		// stack { elm, vtable, keys, map, key[1] )
		lua_pushvalue(L, -1);
		// stack { elm, vtable, keys, map, key[1], key[1] }
		lua_rawseti(L, -4, e); // Move start to end
		// stack { elm, vtable, keys, map, key[1] }
		lua_pushinteger(L, e);
		// stack { elm, vtable, keys, map, key[1], e }
		lua_rawset(L, -3);
		// stack { elm, vtable, keys, map }

	}
	// stack { elm, vtable, keys, map }
	lua_pop(L, 3);
	// stack { elm }
	return 0;

};

static int lua_json_object_move(lua_State *L)
{
	dumpstack(L, "obj move");
	bool is_env = lua_istable(L, 1);
	json_elm *elm = check_json_elm(L, 1, false);
	int m = 0, t = 0;

	if(!lua_isnumber(L, 3)) {
		elm->key =  luaL_checkstring(L, 3);
		t = elm->opts->key_to_idx(elm, false);
		elm->opts->check_idx(elm);
		lua_pop(L, 1);
	}
	else if(lua_isnumber(L, 3)) {
		t = is_env ? luaL_checkinteger(L, 3) : (luaL_checkinteger(L, 3) + 1);
		
		lua_pop(L, 1);
	}

	if(!lua_isnumber(L, 2)) {
		elm->key =  luaL_checkstring(L, 2);
		m = elm->opts->key_to_idx(elm, false);
		elm->opts->check_idx(elm);
	}
	else if(lua_isnumber(L, 2))
	{
		elm->idx = is_env ? luaL_checkinteger(L, 2) : (luaL_checkinteger(L, 2) + 1);
		elm->opts->check_idx(elm);

		if(!(bool)elm->opts->idx_to_key(elm))
			return luaL_error(L, "key at index: %d does not exist !!", elm->idx);

		m = elm->idx;

		lua_pushstring(L, elm->key);
		lua_replace(L, 2);
	}

	if(m > t || m < t) {
		// stack { elm , move }
		lua_pushlightuserdata(L, (void*)elm->env_id);
		// stack { elm, move, env_id }
		lua_rawget(L, LUA_REGISTRYINDEX);
		// stack { elm, move, vtable }
		lua_getfield(L, -1, "keys");
		// stack { elm, move, vtable, keys }
		lua_rawgeti(L, -1, 0); // get the map
		// stack { elm, move, vtable, keys, map }
	
		for(int i = m; (m > t) ? i > t : i < t; (i > t) ? i-- : i++) {
			// shift to move
			lua_rawgeti(L, -2, (i > t) ? (i - 1) : (i + 1));
			// stack { elm, move, vtable, keys, map, keys[i - 1] }
			lua_pushvalue(L, -1);
			// stack { elm, move, vtable, keys, map, keys[i - 1], keys[i - 1] }
			lua_rawseti(L, -4, i);
			// stack { elm, move, vtable, keys, map, keys[i - 1] }
			lua_pushinteger(L, i);
			// stack { elm, move, vtable, keys, map, keys[i - 1], i }
			lua_rawset(L, -3);
			// stack { elm, move, vtable, keys, map }
		}
		// stack { elm, move, vtable, keys, map }
		lua_pushvalue(L, 2);
		// stack { elm, move, before, vtable, keys, map, move }
		lua_pushvalue(L, -1);
		// stack { elm, move, before, vtable, keys, map, move, move }
		lua_rawseti(L, -4, t);
		// stack { elm, move, before, vtable, keys, map, move }
		lua_pushinteger(L, t);
		// stack { elm, move, before, vtable, keys, map, move, t }
		lua_rawset(L, -3);
		// stack { elm, move, before, vtable, keys, map }
		lua_pop(L, 3);
		// stack { elm, move, before }
	}

	return 0;
}

static int lua_json_object_insert(lua_State *L)
{
	// stack { elm, key||idx, key, val }
	bool is_env = lua_istable(L, 1);
	json_elm *elm = check_json_elm(L, 1, false);
	int insert = 0;

	if(lua_isstring(L, 2)) {
		// stack { elm, key, key, val }
		elm->key = luaL_checkstring(L, 2);
		insert = elm->opts->key_to_idx(elm, false);
		lua_pushinteger(L, insert);
		// stack { elm, key, key, val, insert }
		lua_replace(L, -4);
		// stack { elm, insert, key, val }
	}
	else
	{
		// stack { elm, insert, key, val }
		elm->idx = is_env ? luaL_checkinteger(L, 2) : (luaL_checkinteger(L, 2) + 1);
		bool ok = (bool)elm->opts->idx_to_key(elm);

		if(! ok ) 
			return luaL_error(L, "key at index: %d does not exist !!", elm->idx);

	}

	// stack { elm, insert, key, val } 
	lua_settable(L, -4);
	// stack { elm, insert }
	lua_pushinteger(L, (int)elm->nelms-1);
	// stack { elm, insert, nelms }
	lua_insert(L, -2);
	// stack { elm, nelms, insert }
	lua_json_object_move(L);
	// stack { elm, nelms, insert }
	lua_pop(L, 2);
	// stack { elm }
	lua_pushnumber(L, elm->nelms);
	// stack { elm, nelms }
	return 1;
}


static int lua_json_object_unshift(lua_State *L) {
	int nargs = (lua_gettop(L) - 1);

	if ((nargs % 2) != 0) 
		return luaL_error(L, "Invalid arguments: Object requires Key/Value pairs.");

	json_elm *elm = check_json_elm(L, 1, false);
	lua_State *L1 = lua_newthread(L);
	// stack { elm, ...,  thread }
	lua_pushvalue(L, 1);
	// stack { elm, ...,  thread, elm }
	lua_pushcfunction(L1, lua_json_object_reverse);
	// thread stack { object_rev }
	lua_xmove(L, L1, 1);
	// thread stack { object_rev, elm }

	if((bool)lua_resume(L1, 1)) 
		luaL_error(L, "ERROR: failed to reverse object !!");

	// stack { elm, ...,  thread }	
	lua_pop(L, 1);
	// stack { elm, ... }
	for(int i = 0; i <= nargs; i++) {
		lua_settable(L, 1);
		nargs -= 2;
	}

	L1 = lua_newthread(L);
	// stack { elm, thread }
	lua_pushvalue(L, 1);
	// stack { elm, thread, elm }
	lua_pushcfunction(L1, lua_json_object_reverse);
	// thread stack { object_rev }
	lua_xmove(L, L1, 1);
	// thread stack { object_rev, elm }

	if((bool)lua_resume(L1, 1)) 
		luaL_error(L, "ERROR: failed to reverse object !!");

	// stack { elm , thread }
	lua_pop(L, 1);
	// stack { elm }
	lua_pushnumber(L, elm->nelms);
	// stack { elm, nelms }
	return 1;

}

static int
lua_json_object_tojson(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1, false);

	elm->escape = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : 0;
	elm->mode = MARSHAL_JSON;
	lua_settop(L, 1);

	elm->opts->stringify(L);

	return 1;
};

static int
lua_json_object_tolua(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1, false);

	elm->escape = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : 0;
	elm->mode = MARSHAL_LUA;
	lua_settop(L, 1);

	elm->opts->stringify(L);

	return 1;
};

static int
lua_json_object_ref(lua_State *L) {
	check_json_elm(L, 1, false);
	// stack { elm }
	return 1;
};

static int
lua_json_object_unref(lua_State *L) {
	json_elm *elm = check_json_elm(L, -1, false);

	elm->opts->stringify(L);

	lua_replace(L, 1);
	lua_pop(L, 1);

	lua_json_elm_parse(L);
	
	return 1;
};

static int
lua_json_object_idx_to_key(json_elm *elm) {
	// stack { elm ..., }
	lua_State *L = elm->L;
	lua_pushlightuserdata(L, (void*)elm->env_id);
	// stack { elm ..., env_id }
	lua_rawget(L, LUA_REGISTRYINDEX);      // +1 (env)
	// stack { elm ..., vtable }
	lua_getfield(L, -1, "keys");           // +1 (keys)
	// stack { elm ..., vtable, keys }
	lua_rawgeti(L, -1, elm->idx);          // +1 (val)
	// stack { elm ..., vtable, keys, res }
	if(lua_isstring(L, -1)) {
		elm->key = lua_tolstring(L, -1, &elm->klen);
		lua_pop(L, 3);
		// stack { elm ... }
		return true;
	}

	// FIX: Clean failure
	lua_pop(L, 3);
	// stack { elm ... }
	return false;
}

static int 
lua_json_object_key_to_idx(json_elm *elm, bool add) {
	// stack { elm }
	lua_State *L = elm->L;
	int base = lua_gettop(L); // Snapshot stack
	
	lua_pushlightuserdata(L, (void*)elm->env_id);
	// stack { elm, env_id }
	lua_rawget(L, LUA_REGISTRYINDEX);
	// stack { elm, vtable }
	lua_getfield(L, -1, "keys");
	// stack { elm, vtable, keys }
	lua_rawgeti(L, -1, 0);
	// stack { elm, vtable, keys, map }
	// LOOKUP FIRST (Always check before adding!)
	lua_pushstring(L, elm->key);
	// stack { elm, vtable, keys, map, key }
	lua_rawget(L, -2); 
	// stack { elm, vtable, keys, map, res }
	if (lua_isnumber(L, -1)) {
		elm->idx = (int)lua_tointeger(L, -1);
		// stack { elm, vtable, keys, map, idx }
		lua_settop(L, base); // Clean stack
		// stack { elm }
		return elm->idx;
	}
	else
		lua_pop(L, 1);
		// stack { elm, vtable, keys, map }

	// ---  ADD NEW (Only if NOT found) ---
	if (add) {
		
		// Get size of Array (keys)
		size_t size = lua_objlen(L, -2);
		int new_idx = (int)(size + 1);
		
		// add the map
		lua_pushstring(L, elm->key);
		// stack { elm, vtable, keys, map, key }
		lua_pushinteger(L, new_idx);
		// stack { elm, vtable, keys, map, key, idx }
		lua_rawset(L, -3);
		// stack { elm, vtable, keys, map }
		lua_pop(L, 1);
		// stack { elm, vtable, keys }
		lua_pushstring(L, elm->key);
		// stack { elm, vtable, keys, key }
		lua_rawseti(L, -2, new_idx);
		// stack { elm, vtable, keys }
		lua_settop(L, base);
		// stack { elm }
		elm->idx = new_idx;

		return new_idx;
	}

	// --- CASE C: NOT FOUND ---
	lua_settop(L, base);
	return 0;
}

static int
lua_json_object_keys(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1, false);

	if(elm->type == JSON_OBJECT_TYPE) {
		lua_pushlightuserdata(L, (void*)elm->env_id);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_getfield(L, -1, "keys");

		lua_replace(L, 2);

		lua_json_parse_lua(L);
	}

	return 1;
};

static int lua_json_object_del_key(lua_State *L) {
	json_elm *elm = check_json_elm(L, -3, false);
	// stack { elm, key, nil }
	lua_pushlightuserdata(L, (void*)elm->env_id);
	// stack { elm, key, nil, env_id }
	lua_rawget(L, LUA_REGISTRYINDEX);
	// stack { elm, key, nil, vtable }
	lua_getfield(L, -1, "keys");
	// stack { elm, key, nil, vatble, keys }

	// put the map on the stack to update it safely
	lua_rawgeti(L, -1, 0); 
	// stack { elm, key, nil, vatble, keys, map }
	lua_rawgeti(L, -2, elm->idx);
	// stack { elm, key, nil, vatble, keys, map, key }
	lua_pushnil(L);
	// stack { elm, key, nil, vatble, keys, map, key, nill }
	lua_rawset(L, -3);

	size_t size = lua_objlen(L, -2); 
	if(elm->idx < (int)size) {
		for (size_t i = elm->idx; i < size; i++) {
			
			// stack { elm, key, nil, vatble, keys, map }
			lua_rawgeti(L, -2, i + 1); 
			// stack { elm, key, nil, vatble, keys, map, key[i+1] }
			lua_pushvalue(L, -1);
			
			// stack { elm, key, nil, vatble, keys, map, key[i+1], key[i+1] }
			lua_rawseti(L, -4, i);
			// stack { elm, key, nil, vatble, keys, map, keys[1+1] }
			lua_pushinteger(L, i); // Push New Index
			// stack { elm, key, nil, vatble, keys, map, keys[1+1], idx }
			lua_rawset(L, -3);  
			// stack { elm, key, nil, vatble, keys, map }
		}
	}
	// stack { elm, key, nil, vatble, keys, map }

	lua_pushnil(L);
	// stack { elm, key, nil, vatble, keys, map, nil }
	lua_rawseti(L, -3, size); // keys[size] = nil
	// stack { elm, key, nil, vatble, keys, map }
	lua_pop(L, 3);
	// stack { elm, key, nil }

	elm->base->nkeys--;
	elm->nelms--; 

	return 0;
}

static int
lua_json_object_render(lua_State *L, struct ref *seen) 
{
	json_elm *elm = check_json_elm(L, -1, false);
	// stack { elm }
	bool ok = true;
	get_json_table(L, -1);
	// stack { elm, env }
	seen->marshal->obj_open(seen);
	for(size_t i = 1; i <= elm->nelms; i++) {
		elm->idx = i;
		ok = (bool)elm->opts->idx_to_key(elm);
		
		if(!ok) break; // no key found.. bailout!!
		lua_getfield(L, -1, elm->key);
		// stack { elm, env, res }
		
		if(!lua_istable(L, -1)) {
			while(lua_isnil(L, -1)) {
				// stack { elm, env, nil }
				lua_pop(L, 1);
				// stack { elm, env }
				elm->is_nil = true;
				lua_pushnil(L);
				// stack { elm, env, nil }
				lua_json_object_del_key(L);
				// stack { elm, env, nil }
				lua_pop(L, 1);
				// stack { elm, env }
				ok = (bool)elm->opts->idx_to_key(elm);
				if(!ok) break;
				lua_getfield(L, -1, elm->key);
				// stack { elm, env, val }
			}
		}

		if(lua_isuserdata(L, -1) && !lua_json_is_elm(L, -1)) {
			lua_pop(L, 1);
			continue;
		}

		if( ok ) {
			
			if(i > 1) seen->marshal->next(seen);
		
			switch(lua_type(L, -1)) 
			{
				case LUA_TSTRING:
				{
					seen->marshal->obj_string(L, elm, seen);
					lua_pop(L, 1);
					break;
				}

				case LUA_TNUMBER:
				{
					seen->marshal->obj_number(L, elm, seen);
					lua_pop(L, 1);
					break;
				}

				case LUA_TUSERDATA:
				case LUA_TTABLE: 
				{
					if(lua_istable(L, -1))
					{
						seen->has_refs += 1;
						lua_json_handle_table(L);
					}

					if(lua_json_is_elm(L, -1))
					{
						seen->nested = check_json_elm(L, -1, false);
						seen->marshal->obj_key(L, elm, seen);
						seen->nested->opts->render(L, seen);
					}

					lua_pop(L, 2);
					break;
				}

				case LUA_TBOOLEAN:
				{
					seen->marshal->obj_bool(L, elm, seen);
					lua_pop(L, 1);
					break;
				}
	
				case LUA_TNIL:
				{
					lua_pushstring(L, "null");
					lua_replace(L, -2);
					//idx_to_key(elm);
					seen->marshal->obj_string(L, elm, seen);
					lua_pop(L, 1);
					break;
				}

				default: {
					lua_pop(L, 1);
					break;
				}
			}
		}
		else
			break;
	}

	seen->marshal->obj_close(seen);

	// all done !
	if(lua_gettop(L) == 1)
		return 1;
	// recursion
	return 0;
};

static json_elm* lua_json_elm_index(lua_State *L, bool new) {
	json_elm *elm = new ? check_json_elm(L, -3, true) 
			    : check_json_elm(L, -2, true);

	int8_t pos = new ? 4 : 3; // new = new_index
	int8_t idx = pos - 2;

	if(elm->type == JSON_OBJECT_TYPE)
	{
		int ktype = lua_type(L, -idx);

		switch(ktype)
		{
			case LUA_TNUMBER:
			{
				elm->opts->check_idx(elm);
				elm->opts->idx_to_key(elm);
				// Stack Safety: Replace Number Key with String Key for lookup
				lua_pushstring(L, elm->key); 
				lua_replace(L, -(idx +1));          
				break;
			}

			case LUA_TSTRING: {
				elm->key = lua_tolstring(L, -idx, &elm->klen);

				if(strcmp(elm->key, "env") != 0) 
					elm->opts->key_to_idx(elm, true); 

				break;
			}
		}
	}

	elm->nested = lua_json_is_elm(L, -1) ? check_json_elm(L, -1, false) : NULL;

	if (elm->nested && lua_json_elm_contains(L, elm, elm->nested))
		luaL_error(L, "ERROR: Recursive Elm Detected [ Key: %s elm: %p ]\n", elm->key, lua_topointer(L, -1));

	if (elm->nested)
		elm->nested->root = elm;

	return elm;
}

static int
lua_json_object_newindex(lua_State *L)
{
	// ---------------------------------------------------------
	// 1. SHORT STOP & VALIDATION
	// ---------------------------------------------------------
	// json_elm *elm = check_json_elm(L, -3);
	// stack (elm index) { elm, key, val }
	json_elm *elm = lua_json_elm_index(L, true); // check_json_elm(L, -3);
	// ---------------------------------------------------------
	// 2. PREPARE TRANSACTION
	// ---------------------------------------------------------
	elm_rlen erl = {0};
	elm->opts->init_rlen(elm, &erl); // Links erl.base to elm->base
	elm->vtype = lua_type(L, -1);
	elm->ktype = lua_type(L, -2);

	// ---------------------------------------------------------
	// 4. THE CORE MECHANIC (Measure -> Update)
	// ---------------------------------------------------------
	// A. Determine Intent: NEW_INDEX (Append) vs EXT_INDEX (Replace) vs NIL_INDEX (Delete)
	erl.toi = type_of_index(elm);

	// B. Calculate Deltas (Recursive)
	// Populates erl.new (and erl.ex if replacing)
	lua_json_elm_get_val_length(L, elm, &erl);

	// C. Commit Transaction
	// Updates rlen, quoted, nkeys, nelms, and fires ON_CHANGE/ON_NEWINDEX
	update_rlen(elm, &erl);

	// ---------------------------------------------------------
	// 5. STORAGE & CLEANUP
	// ---------------------------------------------------------
	if (elm->vtype == LUA_TNIL)
		lua_json_object_del_key(L);
	
	// Standard Storage: env[key] = val
	get_json_table(L, 1); // Stack: { ..., key, val, env }
	lua_insert(L, 2);	  // Stack: { ..., env, key, val }
	lua_settop(L, 4);	  // Trim extranous stack items
	lua_rawset(L, -3);	  // Perform the write

	return 0;
};

static int lua_json_object_index(lua_State *L)
{
	// ---------------------------------------------------------
	// 1. LIGHTWEIGHT PRE-FLIGHT
	// ---------------------------------------------------------
	// Use check_json_elm (or luaL_checkudata) just to get the pointer.
	// Do NOT call lua_json_elm_index() yet. It has side effects.
	json_elm *elm = check_json_elm(L, 1, false);
	// ---------------------------------------------------------
	// 2. METHOD GUARD (The Upvalue Check)
	// ---------------------------------------------------------
	// If the key is a string, check the Method Table (Upvalue 1)
	if(lua_type(L, 2) == LUA_TSTRING)
	{
		lua_pushvalue(L, 2);				// Push Key ("len")
		lua_rawget(L, lua_upvalueindex(1)); // Check Upvalue

		// If found, return function immediately.
		// CRITICAL: We skip lua_json_elm_index() entirely.
		// The object state (idx) remains untouched.
		if(!lua_isnil(L, -1)) return 1;

		lua_pop(L, 1); // Pop nil, fall through to data
	}

	// ---------------------------------------------------------
	// 3. DATA ACCESS (Now we run the logic)
	// ---------------------------------------------------------
	// We confirmed it's not a method. Now we prepare the element
	// for data retrieval. This is where 'elm->key' gets set.
	lua_json_elm_index(L, false);

	// Special "env" property
	if(elm->key && strcmp("env", elm->key) == 0)
	{
		get_json_table(L, 1);
		luaL_getmetatable(L, "JSON.object");
		lua_setmetatable(L, -2);
		elm->stale = true;
		elm->align = false;

		if(elm->event)
		{
			event ev = {0};
			ev.type = ON_ENV;
			elm->event->set(elm->event->on_env, &ev);
		}
	
		return 1;
	}

	// ---------------------------------------------------------
	// 4. STORAGE LOOKUP
	// ---------------------------------------------------------
	get_json_table(L, 1); 	// Stack: { UD, Key, Env }
	lua_pushvalue(L, 2);	// Stack: { UD, Key, Env, Key }
	lua_rawget(L, -2);	// Stack: { UD, Key, Env, Value }

	// Cleanup
	elm->key = "nil";

	return 1;
}

static int lua_json_handle_table(lua_State *L) {
	// stack { elm, t, ... }
	int base = lua_gettop(L);
	lua_State *L1 = lua_newthread(L);
	// stack { elm, t, ...,  thread }
	lua_pushcfunction(L1, lua_json_parse_lua);
	// thread stack { parse_lua }
	lua_pushvalue(L, -2);
	// stack { elm, t, ...,  thread, t }
	lua_xmove(L, L1, 1);
	// thread stack { parse_lua, t }
	if((bool)lua_resume(L1, 1)) 
		luaL_error(L, "ERROR: failed to parse table !!");

	lua_xmove(L1, L, 1);
	lua_replace(L, base);
	lua_settop(L, base);

	return 0;
}

static int lua_json_object_inline_args(lua_State *L, int nargs) {
	// 1. Sanity Check: Pairs only
	if ((nargs % 2) != 0) 
		return luaL_error(L, "Invalid arguments: Object requires Key/Value pairs.");

	// 2. The "Stable Stack" Loop
	// elm is at Index 1. Args start at Index 2.
	// We step by 2 to grab (Key, Value) pairs.
	int top = lua_gettop(L);
	
	for (int i = 2; i < top; i += 2) {
	// i     = Key
	// i + 1 = Value

	// Strict Nil Check (Standard JSON Rule)
	if (lua_isnil(L, i)) {
		return luaL_error(L, "Invalid Key: JSON keys cannot be nil.");
	}

	//if(lua_istable(L, (i + 1))) {
		//lua_json_handle_table(L, (i + 1));
	//}

	// 3. Dispatch
	lua_pushvalue(L, i);     	// Copy Key to top
	lua_pushvalue(L, (i + 1)); 	// Copy Value to top
	
	// elm[key] = val
	// This triggers your optimized __newindex / update_rlen logic automatically
	lua_settable(L, 1); 
	}

	// 4. Cleanup & Return 
	// We leave only the 'elm' (Userdata) on the stack
	lua_settop(L, 1);
	return 1;
}

static int
lua_json_object_init(lua_State *L, int nargs)
{
	// stack { elm, args }
	if ((lua_gettop(L) - 1) > 0) 
		lua_json_object_inline_args(L, nargs);

	if(lua_isnil(L, -1)) return 0;

	return 1;
};

static int
lua_json_object_gc(lua_State *L) {
	json_elm *self = check_json_elm(L, 1, false);
	if(self->base)
		free(self->base);

	lua_pushlightuserdata(L, (void*)self->env_id);
	lua_pushnil(L);
	lua_rawset(L, LUA_REGISTRYINDEX);

	self->event->cleanup(self->event->on_change);
	self->event->cleanup(self->event->on_newindex);
	self->event->cleanup(self->event->on_env);

	free(self->event->on_change);
	free(self->event->on_newindex);
	free(self->event->on_env);

	free(self->event);

	return 0;
}

static struct json_ops OPS_OBJECT = {
     // Real Hashing logic
    .tostring	= &lua_json_elm_tostring,
    .stringify	= &lua_json_elm_stringify,
    .parse      = &lua_json_elm_parse,
	
	.init_rlen	= &lua_json_elm_init_rlen,
	.check_idx	= &lua_json_elm_check_idx,
    
    
    .idx_to_key	= &lua_json_object_idx_to_key,
    .key_to_idx	= &lua_json_object_key_to_idx,
   	.render     = &lua_json_object_render,
    .del	    = &lua_json_object_del_key
};

static int
lua_json_object_new(lua_State *L, bool parse)
{
	int nargs = lua_gettop(L);
	json_elm *elm = (json_elm *)lua_newuserdata(L, sizeof(json_elm));
	memset(elm, 0, sizeof(json_elm));
	// alloc event lists
	alloc_events(elm);
	elm->L = L;
	elm->index_json = true;
	elm->align = true;
	elm->type = JSON_OBJECT_TYPE;
	elm->typename = "object";
	elm->is_nil = false;
	elm->root = elm;
	elm->base = (elm_vlen*)malloc(sizeof(elm_vlen));
	memset(elm->base, 0, sizeof(elm_vlen));
	// create env table
	lua_newtable(L);

	elm->env_id = (uintptr_t)lua_topointer(L, -1);

	set_json_table(L, -2);
	// elms metatable*/
	luaL_getmetatable(L, "JSON.object");
	lua_setmetatable(L, -2);
	elm->opts = &OPS_OBJECT;
	// c side methods
	/*elm->tostring 		= &lua_json_elm_tostring;
	elm->stringify 		= &lua_json_elm_stringify;
	elm->parse 			= &lua_json_elm_parse;
	elm->render 		= &lua_json_object_render;
	elm->idx_to_key 	= &lua_json_object_idx_to_key;
	elm->key_to_idx 	= &lua_json_object_key_to_idx;
	elm->check_idx 		= &lua_json_elm_check_idx;
	elm->init_rlen		= &lua_json_elm_init_rlen;
	elm->del			= &lua_json_object_del_key;*/
	elm->base->rlen 	= 2;

	lua_pushlightuserdata(L, (void*)elm->env_id);  // Push Key

	lua_newtable(L);
	lua_pushvalue(L, -3);
	lua_setfield(L, -2, "ctx");
	lua_newtable(L);
	lua_setfield(L, -2, "ids");
	lua_newtable(L);
	lua_newtable(L);
	lua_rawseti(L, -2, 0);
	lua_setfield(L, -2, "keys");
	lua_newtable(L);
	lua_setfield(L, -2, "props");

	// stack { elm, env_id, vtable }
	
	lua_getfield(L, -1, "ids");
	// stack { elm, env_id, vtable, ids }
	//lua_pushvalue(L, -3);
	lua_pushinteger(L, elm->env_id);
	// stack { elm, env_id, vtable, ids, env_id }
	lua_pushboolean(L, 1);
	// stack { elm, env_id, vtable, ids, env_id, true }
	// --> elm[ids][env_id]=true
	lua_rawset(L, -3);
	// stack { elm, env_id, vtable, ids}
	lua_pop(L, 1);
	// stack { elm, env_id, vtable }
	lua_rawset(L, LUA_REGISTRYINDEX);
	// stack { elm }
	if(DEBUG) printf("Object Vtable Id: %ld\n", elm->env_id);

	if (!parse && nargs > 0)
	{
		lua_replace(L, 1);
		lua_remove(L, 2);
		nargs -= 2;
		lua_json_object_init(L, nargs);
	}

	if(DEBUG) printf("NEW OBJECT SIZE: %ld\n", sizeof(json_elm));

	return 1;
};

int
lua_json_object(lua_State *L) {
	lua_json_object_new(L, false);

	return 1;
};

int
lua_json_elm_parse_object(lua_State *L) {
	lua_json_object_new(L, true);

	return 1;
};

static int lua_json_object_is_ref(lua_State *L) {
	json_elm *ref = check_json_elm(L, 1, false);

	lua_pushboolean(L, ref->is_ref);

	return 1;
}

static const luaL_Reg lua_json_object_lib_m[] = {
	// --- Standard Methods ---
	{"foreach",	lua_json_object_foreach	},
	{"tojson",	lua_json_object_tojson	},
	{"tolua",	lua_json_object_tolua	},
	{"totable",	lua_json_elm_to_table	},
	{"len",		lua_json_elm_len	},
	{"rlen", 	lua_json_elm_get_rlen	},
	{"keys",	lua_json_object_keys	},
	//{"sort",	lua_json_object_sort	},

	// --- Memory / internal ---
	{"ref",		lua_json_object_ref	},
	{"unref",	lua_json_object_unref	},

	// --- Advanced Tools 
	{"is_stale",	lua_json_elm_is_stale	},
	{"json_base",	lua_json_elm_index_base	},
	{"move",	lua_json_object_move	},
	{"push",	lua_json_object_push	},
	{"pop",		lua_json_object_pop	},
	{"shift",	lua_json_object_shift	},
	{"unshift",	lua_json_object_unshift	},
	{"reverse",	lua_json_object_reverse	},
	{"insert",	lua_json_object_insert	},
	{"nkeys",	lua_json_elm_get_nkeys	},
	{"quoted",	lua_json_elm_get_quoted	},
	{"refs",	lua_json_elm_print_refs	},
	{"is_ref",	lua_json_object_is_ref	},
	{NULL, NULL}
};

void lua_json_open_object(lua_State *L) {
	// --- PART 1: The Factory (JSON.object) ---
	lua_newtable(L);  // The Factory Table
	lua_newtable(L);  // The Factory's Metatable
	
	lua_pushstring(L, "__call");
	lua_pushcfunction(L, lua_json_object); // The Constructor
	lua_settable(L, -3);

	lua_setmetatable(L, -2);
	
	// Optional: Register methods on the Factory
	luaL_register(L, NULL, lua_json_object_lib_m);
	lua_setfield(L, -2, "object"); // Attach to Module

	// --- PART 2: The Instance Type (JSON.object metatable) ---
	luaL_newmetatable(L, "JSON.object");

	// A. Create the Method Table (The Upvalue)
	// Populate a raw table with methods to serve as the lookup cache
	lua_newtable(L); 
	luaL_register(L, NULL, lua_json_object_lib_m); 

	// B. Create the Optimized Index Closure
	// We give 'lua_json_object_index' access to the method table (Upvalue 1)
	lua_pushcclosure(L, lua_json_object_index, 1);
	
	// C. Assign the Closure to __index
	// This allows us to check Methods (Upvalue) -> Then Storage (Env)
	lua_setfield(L, -2, "__index"); 

	// D. Register other Metamethods
	lua_pushcfunction(L, lua_json_object_newindex);
	lua_setfield(L, -2, "__newindex");
	
	lua_pushcfunction(L, lua_json_object_gc);
	lua_setfield(L, -2, "__gc");

	lua_pushcfunction(L, lua_json_elm_tostring);
	lua_setfield(L, -2, "__tostring");

	lua_pushcfunction(L, lua_json_elm_size);
	lua_setfield(L, -2, "__len");

	lua_pop(L, 1); // Pop "JSON.object" metatable
}