/*
 * LuaJson - a json api library for Lua
 *
 *   Copyright (C) 2026 TheRootED24 <TheRootED24@gmail.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
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
static int lua_json_object_foreach(lua_State *L) {
	idx_m map = none;
	json_elm *elm = check_json_elm(L, 1, &map);
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

	// stack { elm }
	elm->idx = 0;
	elm->key = NULL;

	return 0; // Iteration finished naturally
};

static int lua_json_object_push(lua_State *L)
{
	idx_m map = none;
	json_elm *elm = check_json_elm(L, 1, &map);
	luaL_checkstring(L, 2);

	if(lua_isstring(L, 2) && !lua_isnil(L, 3))
		lua_settable(L, -3);

	elm->idx = 0;
	elm->key = NULL;

	return 0;
};

static int lua_json_object_pop(lua_State *L)
{
	idx_m map = none;
	json_elm *elm = check_json_elm(L, 1, &map);

	elm->idx = elm->nelms;
	int ret = 0;

	if((bool)elm->ops->idx_to_key(elm)) {
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
};

static int lua_json_object_shift(lua_State *L)
{
	idx_m map = none;
	json_elm *elm = check_json_elm(L, 1, &map);
	elm->idx = 1;

	if((bool)elm->ops->idx_to_key(elm)) {
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
};

static int lua_json_object_reverse(lua_State *L)
{
	int nargs = (lua_gettop(L) - 1);

	idx_m map = reverse;
	json_elm *elm = check_json_elm(L, 1, &map);
	// stack { elm }
	uint16_t start = 1, end = (uint16_t)elm->nelms;
	
	start = nargs ? lua_json_elm_align_idx(elm, 2) : start;
	end =  nargs > 1 ? lua_json_elm_align_idx(elm, 3) : end;

	lua_settop(L, 2);

	// stack { elm , move }
	lua_pushlightuserdata(L, (void*)elm->env_id);
	// stack { elm, move, env_id }
	lua_rawget(L, LUA_REGISTRYINDEX);
	// stack { elm, move, vtable }
	lua_getfield(L, -1, "keys");
	// stack { elm, move, vtable, keys }
	lua_rawgeti(L, -1, 0); // get the map
	// stack { elm, vtable, keys, map }

	for (uint16_t s = start < end ? start : end, e = end > start ? end : start; s < e; s++, e--)
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
	elm->idx = 0;
	elm->key = NULL;

	return 0;

};

static int lua_json_object_move(lua_State *L)
{
	idx_m map = move;
	
	json_elm *elm = check_json_elm(L, 1, &map);
	uint16_t m = lua_json_elm_push_key(elm, 2); //lua_json_elm_align_idx(elm, 2);
	uint16_t t = lua_json_elm_align_idx(elm, 3);
	lua_pop(L, 1);

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

	elm->idx = 0;
	elm->key = NULL;

	return 0;
};

static int lua_json_object_insert(lua_State *L)
{
	// stack { elm, key||idx, key, val }
	idx_m map = insert;
	json_elm *elm = check_json_elm(L, 1, &map);

	elm->idx = lua_json_elm_align_idx(elm, 2);

	if(!lua_isnumber(L, 2)) {
		lua_pushinteger(L, elm->idx-1);
		lua_replace(L, 2);
	}

	elm->ops->check_idx(elm);
	// stack { elm, insert, key, val } 
	lua_settable(L, -4);
	// stack { elm, insert }
	lua_pushinteger(L, (int)elm->nelms-1);
	// stack { elm, insert, nelms }
	lua_insert(L, -2);
	// stack { elm, nelms, insert }
	elm->align = false;
	lua_json_object_move(L);
	// stack { elm, nelms, insert }
	lua_pop(L, 2);
	// stack { elm }
	lua_pushnumber(L, elm->nelms);
	// stack { elm, nelms }
	elm->idx = 0;
	elm->key = NULL;

	return 1;
};

static int lua_json_object_unshift(lua_State *L) {
	int nargs = (lua_gettop(L) - 1);

	if ((nargs % 2) != 0) 
		return luaL_error(L, "Invalid arguments: Object requires Key/Value pairs.");

	idx_m map = none;
	json_elm *elm = check_json_elm(L, 1, &map);

	lua_State *L1 = lua_newthread(L);
	// stack { elm, ...,  thread }
	lua_pushvalue(L, 1);
	// stack { elm, ...,  thread, elm }
	lua_pushcfunction(L1, lua_json_object_reverse);
	// thread stack { object_rev }
	lua_xmove(L, L1, 1);
	// thread stack { object_rev, elm }

	int status;
#if LUA_VERSION_NUM >= 504
	int nres;
	status = lua_resume(L1, L, 1, &nres);
#elif LUA_VERSION_NUM >= 502
	status = lua_resume(L1, L, 1);
#else
	status = lua_resume(L1, 1);
#endif

	// In all Lua versions, LUA_OK is 0 and LUA_YIELD is 1.
	// Errors are > 1.
	if (status != LUA_OK && status != LUA_YIELD)
	{
		luaL_error(L, "ERROR: failed to parse table !!");
	}

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

#if LUA_VERSION_NUM >= 504
	status = lua_resume(L1, L, 1, &nres);
#elif LUA_VERSION_NUM >= 502
	status = lua_resume(L1, L, 1);
#else
	status = lua_resume(L1, 1);
#endif

	// In all Lua versions, LUA_OK is 0 and LUA_YIELD is 1.
	// Errors are > 1.
	if (status != LUA_OK && status != LUA_YIELD)
	{
		luaL_error(L, "ERROR: failed to parse table !!");
	}

	// stack { elm , thread }
	lua_pop(L, 1);
	// stack { elm }
	lua_pushnumber(L, elm->nelms);
	// stack { elm, nelms }
	elm->idx = 0;
	elm->key = NULL;

	return 1;
};

static int lua_json_object_tojson(lua_State *L) {
	idx_m map = move;
	json_elm *elm = check_json_elm(L, 1, &map);

	elm->escape = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : 0;
	elm->mode = MARSHAL_JSON;

	if(elm->escape) lua_pop(L, 1);

	elm->ops->stringify(L);

	elm->idx = 0;
	elm->key = NULL;

	return 1;
};

static int lua_json_object_tolua(lua_State *L) {
	idx_m map = move;
	json_elm *elm = check_json_elm(L, 1, &map);

	elm->escape = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : 0;
	elm->mode = MARSHAL_LUA;

	if(elm->escape) lua_pop(L, 1);

	elm->ops->stringify(L);

	elm->idx = 0;
	elm->key = NULL;

	return 1;
};

#ifdef WASM
static int lua_json_pair_to_wasm(lua_State *L) {
	idx_m map = none;
	json_elm *elm = check_json_elm(L, 1, &map);
	// stack { elm, key, val }
	lua_newtable(L);
	// stack { elm, key, val, tbl }
	lua_pushvalue(L, 2);
	// stack { elm, key, val, tbl, key }
	lua_pushvalue(L, 3);
	// stack { elm, key, val, tbl, key, val }
	lua_rawset(L, -3);
	// stack { elm, key, val, tbl }
	lua_json_handle_table(L);
	// stack { elm, key, val, elm }
	elm = check_json_elm(L, -1, &map);
	
	elm->escape = true;
	// stack { elm }
	elm->ops->stringify(L);
	// stack { elm, json }
	elm->idx = 0;
	elm->key = NULL;

	return 1;
};
#endif

static int lua_json_object_ref(lua_State *L) {
	idx_m map = none;
	json_elm *elm = check_json_elm(L, 1, &map);
	// stack { elm }
	elm->idx = 0;
	elm->key = NULL;

	return 1;
};

static int lua_json_object_unref(lua_State *L) {
	idx_m map = none;
	json_elm *elm = check_json_elm(L, 1, &map);

	elm->ops->stringify(L);

	//lua_replace(L, 1);
	//lua_pop(L, 1);

	lua_json_elm_parse(L);

	elm->idx = 0;
	elm->key = NULL;
	
	return 1;
};

static int lua_json_object_idx_to_key(json_elm *elm) {
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

	// Clean failure
	lua_pop(L, 3);
	// stack { elm ... }
	return false;
};

static int lua_json_object_key_to_idx(json_elm *elm, bool add) {
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
};

static int lua_json_object_keys(lua_State *L) {
	idx_m map = none;
	json_elm *elm = check_json_elm(L, 1, &map);

	if(elm->type == JSON_OBJECT_TYPE) {
		lua_pushlightuserdata(L, (void*)elm->env_id);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_getfield(L, -1, "keys");

		lua_replace(L, 2);

		lua_json_parse_lua(L);
	}

	elm->idx = 0;
	elm->key = NULL;

	return 1;
};

static int lua_json_object_del_key(lua_State *L) {
	idx_m map = none;
	json_elm *elm = check_json_elm(L, -3, &map);
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

	elm->idx = 0;
	elm->key = NULL;

	return 0;
};

static int lua_json_object_render(lua_State *L, struct ref *seen) 
{
	idx_m map = none;
	json_elm * elm = check_json_elm(L, -1, &map);
	// stack { elm }
	size_t s = (seen->start > 0) ? (size_t)seen->start : 1;
	size_t e = (seen->end > 0) ? (size_t)seen->end : elm->nelms;

	seen->start= 0;
	seen->end = 0;

	bool ok = true;
	get_json_table(L, -1);
	// stack { elm, env }
	seen->marshal->obj_open(seen);
	for(size_t i = s; i <= e; i++) {
		elm->idx = i;
		ok = (bool)elm->ops->idx_to_key(elm);
		
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
				ok = (bool)elm->ops->idx_to_key(elm);
				if(!ok) break;
				lua_getfield(L, -1, elm->key);
				// stack { elm, env, val }
			}
		}

		if(!seen->trusted && (seen->rlen >= seen->L_max)) {
			seen->needs_state = true;
			break;
		}

		if(lua_isuserdata(L, -1) && !lua_json_is_elm(L, -1)) {
			lua_pop(L, 1);
			continue;
		}

		if(lua_istable(L, -1)) {
			uintptr_t next = (uintptr_t)lua_topointer(L, -1);
			if(seen->check_next(L, seen, next)) {
				lua_pop(L, 1);
				continue;
			}
		}
		
		if( ok ) {
			
			if(i > s) seen->marshal->next(seen);
		
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
					uintptr_t table_id = 0;

					if(lua_istable(L, -1)) {
						table_id = (uintptr_t)lua_topointer(L, -1);
					
						seen->has_refs += 1;
						lua_json_handle_table(L);
					}

					if(lua_json_is_elm(L, -1))
					{
						idx_m map = none;
						
						seen->nested = check_json_elm(L, -1, &map);
						seen->marshal->obj_key(L, elm, seen);
						seen->nested->ops->render(L, seen);

						if(table_id > 0)
							 seen->clear_next(L, seen, table_id);
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

static json_elm* lua_json_elm_index(lua_State *L) {
	idx_m map = newindex;

	json_elm *elm = check_json_elm(L, 1, &map);

	if(lua_isnumber(L, 2)) 
		lua_json_elm_push_key(elm, 2);
	
	elm->key = lua_tolstring(L, 2, &elm->klen);
	elm->idx = elm->ops->key_to_idx(elm, false);

	if((strcmp(elm->key, "env") != 0) && (lua_json_is_printable(L))) 
		elm->ops->key_to_idx(elm, true); 
	
	map = none;	
	elm->nested = lua_json_is_elm(L, -1) ? check_json_elm(L, -1, &map) : NULL;

	if (elm->nested && lua_json_elm_contains(L, elm, elm->nested))
		luaL_error(L, "ERROR: Recursive Elm Detected [ Key: %s elm: %p ]\n", elm->key, lua_topointer(L, -1));

	if (elm->nested)
		elm->nested->root = elm;

	return elm;
};

static int lua_json_object_newindex(lua_State *L)
{
	// ---------------------------------------------------------
	// 1. SHORT STOP & VALIDATION
	// ---------------------------------------------------------
	// stack (elm index) { elm, key, val }
	json_elm *elm = lua_json_elm_index(L);

	elm->vtype = lua_type(L, -1);
	elm->ktype = lua_type(L, -2);
	// ---------------------------------------------------------
	// 2. PREPARE TRANSACTION
	// ---------------------------------------------------------
	if(lua_json_is_printable(L) || lua_isnil(L, -1)) {
		elm_rlen erl = {0};
		elm->ops->init_rlen(elm, &erl); // Links erl.base to elm->base
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
	}

	if (elm->vtype == LUA_TNIL)
		lua_json_object_del_key(L);
		// stack (elm index) { elm, key, nil }
	

	// Standard Storage: env[key] = val
	get_json_table(L, 1); 
	// stack (elm index) { elm, key, val, env }
	#ifdef WASM
		lua_pushvalue(L, 2);
		lua_pushvalue(L, 3);
	
	#else
		lua_insert(L, 2);	  // Stack: { ..., env, key, val }
		lua_settop(L, 4);
	#endif
	// stack (elm index) { elm, env, key, val } || { elm, key, val, env, key, val}
	lua_rawset(L, -3); // Perform the write
	// stack (elm index) { elm, env } || { elm, key, val, env }
	lua_pop(L, 1);
	// stack { elm } || { elm, key, val }
	if(lua_gettop(L) > 1) {
		if (elm->event && elm->event->on_mutate) {
			// V19.0 Binding picks up here. 
			// Calls elm->towasm(L), sends JSON, pops result.
			elm->event->set(elm->event->on_mutate, NULL);
		}
	}
		
	return 0;
};

static int lua_json_object_index(lua_State *L)
{
	// ---------------------------------------------------------
	// 1. LIGHTWEIGHT PRE-FLIGHT
	// ---------------------------------------------------------
	// Use check_json_elm (or luaL_checkudata) just to get the pointer.
	// Do NOT call lua_json_elm_index() yet. It has side effects.
	idx_m map = newindex;
	json_elm *elm = check_json_elm(L, 1, &map);//check_json_elm(L, 1, true);
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
	lua_json_elm_index(L);

	// Special "env" property
	if(elm->key && strcmp("env", elm->key) == 0)
	{
		get_json_table(L, 1);
		luaL_getmetatable(L, "JSON.object");
		lua_setmetatable(L, -2);
		elm->stale = true;
		//elm->align = false;

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
};

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
	int status;
#if LUA_VERSION_NUM >= 504
	int nres;
	status = lua_resume(L1, L, 1, &nres);
#elif LUA_VERSION_NUM >= 502
	status = lua_resume(L1, L, 1);
#else
	status = lua_resume(L1, 1);
#endif

	// In all Lua versions, LUA_OK is 0 and LUA_YIELD is 1.
	// Errors are > 1.
	if (status != LUA_OK && status != LUA_YIELD)
	{
		luaL_error(L, "ERROR: failed to parse table !!");
	}

	lua_xmove(L1, L, 1);
	lua_replace(L, base);
	lua_settop(L, base);

	return 0;
};

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
};

static int lua_json_object_init(lua_State *L, int nargs)
{
	// stack { elm, args }
	if ((lua_gettop(L) - 1) > 0) 
		lua_json_object_inline_args(L, nargs);

	if(lua_isnil(L, -1)) return 0;

	return 1;
};

static int lua_json_object_gc(lua_State *L) {
	idx_m map = none;
	json_elm *self = check_json_elm(L, 1, &map);

	if (!self) return 0;

	// 1. Clean up event-driven callbacks while all parent memory blocks are intact
	if (self->event) {
		if (self->event->cleanup) {
			if (self->event->on_change) self->event->cleanup(self->event->on_change);
			if (self->event->on_newindex) self->event->cleanup(self->event->on_newindex);
			if (self->event->on_env) self->event->cleanup(self->event->on_env);
#ifdef WASM
			if (self->event->on_mutate) self->event->cleanup(self->event->on_mutate);
#endif
		}

		// Cache component tracking addresses locally
		void *oc = self->event->on_change;
		void *on = self->event->on_newindex;
		void *oe = self->event->on_env;
#ifdef WASM
		void *om = self->event->on_mutate;
#endif
		void *ev = self->event;

		// Unbind reference instantly to shield against concurrency or re-entrant triggers
		self->event = NULL;

		// Safely free the individual event buffers
		free(oc);
		free(on);
		free(oe);
#ifdef WASM
		free(om);
#endif
		free(ev);
	}

	// 2. Clear out the environment context from the Lua Registry Index
	lua_pushlightuserdata(L, (void*)self->env_id);
	lua_pushnil(L);
	lua_rawset(L, LUA_REGISTRYINDEX);

	// 3. Free the base structure memory block at the absolute end of the sweep phase
	if (self->base) {
		free(self->base);
		self->base = NULL;
	}

	self->ops = NULL;
	return 0;
}

static struct json_ops OPS_OBJECT = {
	// Real Hashing logic
	.tostring 	= &lua_json_elm_tostring,
#ifdef WASM
	.towasm		= &lua_json_pair_to_wasm,
#endif
	.stringify 	= &lua_json_elm_stringify,
	.parse 		= &lua_json_elm_parse,

	.init_rlen 	= &lua_json_elm_init_rlen,
	.check_idx 	= &lua_json_elm_check_idx,

	.idx_to_key = &lua_json_object_idx_to_key,
	.key_to_idx = &lua_json_object_key_to_idx,
	.render 	= &lua_json_object_render,
	.del 		= &lua_json_object_del_key
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

	// c side methods
	elm->ops = &OPS_OBJECT;
	// initial length
	elm->base->rlen = 2;
	// create the env_id
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

	// stack { elm, env_id, vtable }
	
	lua_getfield(L, -1, "ids");
	// stack { elm, env_id, vtable, ids }
	lua_pushinteger(L, elm->env_id);
	// stack { elm, env_id, vtable, ids, env_id }
	lua_pushboolean(L, 1);
	// stack { elm, env_id, vtable, ids, env_id, true }
	lua_rawset(L, -3); // --> elm[ids][0]={ env_id=true ...}
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

static const 
luaL_Reg lua_json_object_lib_m[] = {
	// --- Standard Methods ---
	{"foreach",	lua_json_object_foreach		},
	{"tojson",	lua_json_object_tojson		},
	{"tolua",	lua_json_object_tolua		},
	{"totable",	lua_json_elm_to_table		},
	{"len",		lua_json_elm_len			},
	{"rlen", 	lua_json_elm_get_rlen		},
	{"keys",	lua_json_object_keys		},

	// --- Memory / internal ---
	{"ref",		lua_json_object_ref			},
	{"unref",	lua_json_object_unref		},

	// --- Advanced Tools 
	{"json_base",	lua_json_elm_index_base	},
	{"move",	lua_json_object_move		},
	{"push",	lua_json_object_push		},
	{"pop",		lua_json_object_pop			},
	{"shift",	lua_json_object_shift		},
	{"unshift",	lua_json_object_unshift		},
	{"reverse",	lua_json_object_reverse		},
	{"insert",	lua_json_object_insert		},
	{NULL, NULL}
};

void lua_json_open_object(lua_State *L) {
    // --- PART 1: The Factory (JSON.object) ---
    lua_newtable(L);  
    lua_newtable(L);  
    
    lua_pushcfunction(L, lua_json_object);
    lua_setfield(L, -2, "__call"); 

    lua_setmetatable(L, -2);
    
    // Register methods using the compatibility macro
    luaL_reg_stack(L, lua_json_object_lib_m);
    lua_setfield(L, -2, "object"); 

    // --- PART 2: The Instance Type (JSON.object metatable) ---
    luaL_newmetatable(L, "JSON.object");

    // A. Create the Method Table as an Upvalue
    lua_newtable(L); 
    luaL_reg_stack(L, lua_json_object_lib_m); 

    // B. Create the Optimized Index Closure
    // Consumes the table above as Upvalue 1
    lua_pushcclosure(L, lua_json_object_index, 1);
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

    lua_pop(L, 1); // Pop "JSON.object" metatable from the stack
}