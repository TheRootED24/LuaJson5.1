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

//#define DUMPSTACK 1

#ifdef DUMPSTACK
static void dumpstack(lua_State *L, const char *msg)
{
	printf("************** begin %s ****************\n", msg);
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
	printf("**********************************\n");
}
#endif

// C function to implement foreach(elm, func)
static int
lua_json_object_foreach(lua_State *L) {
	int nargs = lua_gettop(L);
	json_elm *elm = check_json_elm(L, 1);
	if(!elm) return 0;
	// Expects function at index 2
	luaL_checktype(L, 2, LUA_TFUNCTION);
	bool omit_keys =  nargs > 2 ? lua_isboolean(L, 3) ? lua_toboolean(L, 3) : false : false;
	get_json_table(L, 1);
	lua_insert(L, 2);
	// stack { elm, env, func }
	lua_pushnil(L);
	// stack { elm, env, func, nil }
	while (lua_next(L, 2) != 0)
	{
		// omit keys table if set
		if(lua_isnumber(L, -2) && omit_keys) {
			lua_pop(L, 1);
			continue;
		}

		lua_pushvalue(L, 3);  // Push function
		// stack { elm, env, func, key, val, func }
		lua_pushvalue(L, -3); // Push key
		// stack { elm, env, func, key, val, func, key }
		lua_pushvalue(L, -3); // Push value
		// stack { elm, env, func, key, val, func, key, val }
		// 4. Call function(key, value)
		if (lua_pcall(L, 2, 1, 0) != 0)
			return lua_error(L); // Propagate error
		
		// stack { elm, env, func, key, val, result }
		if (!lua_isnil(L, -1))
			return 1; // Stop and return the value
		

		lua_pop(L, 2); 	// Pop result and the value, leaving the key for next iteration
		// stack { elm, env, func, key }
	}

	return 0; // Iteration finished naturally
};

static int
lua_json_object_sort(lua_State *L) {
	int nargs = lua_gettop(L);
	json_elm *elm = check_json_elm(L, 1);
	if(!elm) return 0;
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
}

static int
lua_json_object_tojson(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	if(lua_istable(L, 1) && lua_isuserdata(L, -1))
		lua_replace(L, 1);

	elm->escape = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : 0;
	elm->mode = MARSHAL_JSON;
	lua_settop(L, 1);
	elm->stringify(L);

	return 1;
};

static int
lua_json_object_tolua(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	if(lua_istable(L, 1) && lua_isuserdata(L, -1))
		lua_replace(L, 1);

	elm->escape = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : 0;
	elm->mode = MARSHAL_LUA;
	lua_settop(L, 1);
	elm->stringify(L);

	return 1;
};

static int
lua_json_object_ref(lua_State *L) {
	check_json_elm(L, 1);
	// stack { elm }
	return 1;
};

static int
lua_json_object_unref(lua_State *L) {
	json_elm *elm = check_json_elm(L, -1);
	size_t len = 0;

	//elm->tojson(L);
	elm->stringify(L);
	const char *json = lua_tolstring(L, -1, &len);

	lua_settop(L, 0);
	lua_pushlstring(L, json, len);
	lua_json_elm_parse(L);
	
	return 1;
};

static bool
idx_to_key(json_elm *elm) {
	lua_State *L = elm->L;
	lua_pushlightuserdata(L, (void*)elm->env_id);
	lua_rawget(L, LUA_REGISTRYINDEX);      // +1 (env)
	lua_getfield(L, -1, "keys");           // +1 (keys)
	lua_rawgeti(L, -1, elm->idx);          // +1 (val)

	if(lua_isstring(L, -1)){
		elm->key = lua_tolstring(L, -1, &elm->klen);
		lua_pop(L, 3); // Clean success: Pop val, keys, env
		return elm->idx; 
	}
	
	// FIX: Clean failure
	lua_pop(L, 3); // Pop val, keys, env

    return false;
}

static int key_to_idx(json_elm *elm, bool add) {
    lua_State *L = elm->L;
    int base = lua_gettop(L); // Snapshot stack

    // 1. Setup: [Env, Keys]
    lua_pushlightuserdata(L, (void*)elm->env_id);
    lua_rawget(L, LUA_REGISTRYINDEX);
    lua_getfield(L, -1, "keys");
    
    // 2. Get Map: [Env, Keys, Map]
    lua_rawgeti(L, -1, 0);

    // 3. LOOKUP FIRST (Always check before adding!)
    lua_pushstring(L, elm->key);
    lua_rawget(L, -2); // Stack: [Env, Keys, Map, Result]

    // --- CASE A: EXISTS ---
    if (lua_isnumber(L, -1)) {
        elm->idx = (int)lua_tointeger(L, -1);
        lua_settop(L, base); // Clean stack
        return elm->idx;
    }
    
    // --- CASE B: ADD NEW (Only if NOT found) ---
    if (add) {
        lua_pop(L, 1); // Pop 'nil' result. Stack: [Env, Keys, Map]
        
        // Get size of Array (keys)
        size_t size = lua_objlen(L, -2);
        int new_idx = (int)(size + 1);

        // 1. Update Map: keys[0]["key"] = new_idx
        lua_pushstring(L, elm->key);
        lua_pushinteger(L, new_idx);
        lua_rawset(L, -3);

        // 2. Update Array: keys[new_idx] = "key"
        // We need to pop Map to get to Keys? 
        // Actually, we can just copy keys (at -2) to top or use absolute index.
        // Easier: Pop map first.
        lua_pop(L, 1); // Pop Map. Stack: [Env, Keys]

        lua_pushstring(L, elm->key); // Fix: lua_pushstring, not lua_push
        lua_rawseti(L, -2, new_idx);

        elm->idx = new_idx;
        lua_settop(L, base); // Clean stack
        return new_idx;
    }

    // --- CASE C: NOT FOUND ---
    lua_settop(L, base);
    return 0; // 0 acts as false in C checks
}

static int
lua_json_object_keys(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);

	if(elm->type == JSON_OBJECT_TYPE) {
		lua_pushlightuserdata(L, (void*)elm->env_id);
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_getfield(L, -1, "keys");
		//lua_rawgeti(L, -1, 0);
		lua_insert(L, 2);
		lua_pop(L, 1);

		lua_json_lua_parse(L);

		return 1;
	}
	else
		lua_pushnil(L);

	return 1;
};

static int
lua_json_render_object(lua_State *L, struct ref *seen) 
{
	json_elm *elm = check_json_elm(L, -1);
	get_json_table(L, -1);

	seen->marshal->obj_open(seen);
	for(size_t i = 1; i <= elm->nelms; i++) {
		elm->idx = i;
		elm->idx_to_key(elm);

		lua_getfield(L, -1, elm->key);

		switch(lua_type(L, -1)) {
			case LUA_TUSERDATA: {

				seen->nested = check_json_elm(L, -1);
				seen->marshal->obj_key(L, elm, seen);
				seen->nested->render(L, seen);
				lua_pop(L, 2);
				break;
			}

			case LUA_TSTRING: {
				seen->marshal->obj_string(L, elm, seen);
				lua_pop(L, 1);
				break;
			}

			case LUA_TNUMBER: {
				seen->marshal->obj_number(L, elm, seen);
				lua_pop(L, 1);
				break;
			}

			case LUA_TBOOLEAN: {
				seen->marshal->obj_bool(L, elm, seen);
				lua_pop(L, 1);
				break;
			}
		}
	
		if(elm->nelms - i >= 1) seen->marshal->next(seen);
	}
	
	seen->marshal->obj_close(seen);
	// all done !
	if(lua_gettop(L) == 1)
		return 1;
	// recursion
	return 0;
};

static int lua_json_object_del_key(lua_State *L) {
    json_elm *elm = check_json_elm(L, 1);
    
    lua_pushlightuserdata(L, (void*)elm->env_id);
    lua_rawget(L, LUA_REGISTRYINDEX); // stack: env
    lua_getfield(L, -1, "keys");      // stack: env, keys
	// 1. GET THE MAP (keys[0])
	// We must put the map on the stack to update it safely
	lua_rawgeti(L, -1, 0); 
	// Stack: { ..., keys, map }

	size_t size = lua_objlen(L, -2); // Get len of 'keys' (at -2 now)

	// 2. THE SHIFT LOOP (Combines Array & Map update)
	// We loop from 'idx' up to 'size - 1'.
	for (size_t i = elm->idx; i < size; i++) {
		
		// A. Get Next Key (keys[i+1])
		lua_rawgeti(L, -2, i + 1); 
		// Stack: { keys, map, NextKey }

		// B. Update Array: keys[i] = NextKey
		lua_pushvalue(L, -1);  // Duplicate NextKey
		lua_rawseti(L, -4, i); // keys[i] = NextKey (pops duplicate)
		// Stack: { keys, map, NextKey }

		// C. Update Map: map[NextKey] = i
		lua_pushvalue(L, -1);  // Duplicate NextKey (again)
		lua_pushinteger(L, i); // Push New Index
		// Stack: { keys, map, NextKey, NextKey, i }
		lua_rawset(L, -4);     // map[NextKey] = i (pops Key & i)
		// Stack: { keys, map, NextKey } <-- Map is at -3 relative to top here?
		// WAIT: If we push onto top, map is at -4. 
		// Let's verify: Stack was {keys, map, NextKey}. 
		// Pushed NextKey -> {keys, map, NextKey, NextKey}.
		// Pushed i -> {keys, map, NextKey, NextKey, i}.
		// rawset(-4) targets 'map'. Correct.

		// D. Clean Stack
		lua_pop(L, 1); // Pop NextKey (prepare for next iteration)
		// Stack: { keys, map }
	}

	// 3. TAIL CLEANUP (Delete the last item)
	// The loop shifted everything down. Now we kill the duplicate at the end.
	lua_pushnil(L);
	lua_rawseti(L, -3, size); // keys[size] = nil

	// 4. MAP CLEANUP (Remove the deleted key from map)
	// The old key (that we just deleted) is still in the map pointing to 'idx'.
	// Depending on your logic, you might need to nil it out explicitly here
	// or assume the overwrite handled it. 
	// Safest bet:
	lua_pushstring(L, elm->key); // The key being deleted
	lua_pushnil(L);
	lua_rawset(L, -4); // map[deleted_key] = nil

	// 5. Finish
	lua_pop(L, 1); // Pop 'map'
	// Stack: { ..., keys }
	elm->nkeys--;
	elm->nelms--; 

	return 0;
}


static json_elm* lua_json_elm_index(lua_State *L, bool new) {
	json_elm *elm = new ? check_json_elm(L, -3) : check_json_elm(L, -2);
	int8_t pos = new ? 4 : 3; // new = new_index
	int8_t idx = pos - 2;
 
	if(elm->type == JSON_OBJECT_TYPE) {
	int ktype = lua_type(L, -idx);

		switch(ktype) {
			case LUA_TNUMBER: {
				elm->check_idx(elm);
				elm->idx_to_key(elm);
				// Stack Safety: Replace Number Key with String Key for storage
				lua_pushstring(L, elm->key); 
				lua_replace(L, -(idx +1));          
				break;
			}
			case LUA_TSTRING: {
				elm->key = lua_tolstring(L, -idx, &elm->klen);
				// Registry Lookup: Resolves String Key -> Integer Index (elm->idx)
				elm->key_to_idx(elm, true); 
				break;
			}
		}
	}

	if(!elm)
		luaL_error(L, "Invalid Elm!!\n");

	return elm;
}

static int
lua_json_object_newindex(lua_State *L) 
{
    // ---------------------------------------------------------
    // 1. SHORT STOP & VALIDATION
    // ---------------------------------------------------------
    //json_elm *elm = check_json_elm(L, -3);
	json_elm *elm = lua_json_elm_index(L, true);//check_json_elm(L, -3);
    // Recursion Safety Check
    elm->nested = lua_type(L, -1) == LUA_TUSERDATA ? check_json_elm(L, -1) : NULL;

    if(elm->nested && lua_json_elm_contains(L, elm, elm->nested))
			luaL_error(L, "ERROR: Recursive Elm Detected [ Key: %s elm: %p ]\n",elm->key, lua_topointer(L, -1));
			

	if(elm->nested) elm->nested->root = elm;
    // ---------------------------------------------------------
    // 2. PREPARE TRANSACTION
    // ---------------------------------------------------------
    elm_rlen erl = {0};
    elm->init_rlen(elm, &erl); // Links erl.base to elm->base
    elm->vtype = lua_type(L, -1);
    elm->ktype = lua_type(L, -2);

    // ---------------------------------------------------------
    // 4. THE CORE MECHANIC (Measure -> Update)
    // ---------------------------------------------------------
    // A. Determine Intent: NEW_INDEX (Append) vs EXT_INDEX (Replace) vs NIL_INDEX (Delete)
    erl.toi = type_of_index(elm); 

    // B. Calculate Deltas (Recursive)
    // Populates erl.new (and erl.ex if replacing)
    lua_json_elm_get_val_length2(L, elm, &erl); 

    // C. Commit Transaction
    // Updates rlen, quoted, nkeys, nelms, and fires ON_CHANGE/ON_NEWINDEX
    update_rlen(elm, &erl);

    // ---------------------------------------------------------
    // 5. STORAGE & CLEANUP
    // ---------------------------------------------------------
    if (elm->vtype == LUA_TNIL) {
        // Clean up Registry/VTable if deleting
        lua_json_object_del_key(L); 
    }

    // Standard Storage: env[key] = val
    get_json_table(L, 1);      // Stack: { ..., key, val, env }
    lua_insert(L, 2);          // Stack: { ..., env, key, val }
    lua_settop(L, 4);          // Trim extranous stack items
    lua_rawset(L, -3);         // Perform the write

    return 0;
};

static int object_get_root(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);

	printf("Elm Root: %p\n", &elm->root);
	lua_pop(L, 1);

	return 0;
}

static int lua_json_object_index(lua_State *L) {
	printf("----------------------------------------------------> Object New Index Fired\n");
    // ---------------------------------------------------------
    // 1. LIGHTWEIGHT PRE-FLIGHT
    // ---------------------------------------------------------
    // Use check_json_elm (or luaL_checkudata) just to get the pointer.
    // Do NOT call lua_json_elm_index() yet. It has side effects.
    json_elm *elm = check_json_elm(L, 1);
	
    // ---------------------------------------------------------
    // 2. METHOD GUARD (The Upvalue Check)
    // ---------------------------------------------------------
    // If the key is a string, check the Method Table (Upvalue 1)
    if (lua_type(L, 2) == LUA_TSTRING) {
        lua_pushvalue(L, 2);                // Push Key ("len")
        lua_rawget(L, lua_upvalueindex(1)); // Check Upvalue
        
        // If found, return function immediately.
        // CRITICAL: We skip lua_json_elm_index() entirely. 
        // The object state (idx) remains untouched.
        if (!lua_isnil(L, -1)) {
            return 1; 
        }
        lua_pop(L, 1); // Pop nil, fall through to data
    }

    // ---------------------------------------------------------
    // 3. DATA ACCESS (Now we run the logic)
    // ---------------------------------------------------------
    // We confirmed it's not a method. Now we prepare the element 
    // for data retrieval. This is where 'elm->key' gets set.
    lua_json_elm_index(L, false); 

    // Special "env" property
    if (elm->key && strcmp("env", elm->key) == 0) {
        get_json_table(L, 1);
		luaL_getmetatable(L, "JSON.object");
		lua_setmetatable(L, -2);

        return 1;
    }

    // ---------------------------------------------------------
    // 4. STORAGE LOOKUP
    // ---------------------------------------------------------
    get_json_table(L, 1); // Stack: { UD, Key, Env }
    lua_pushvalue(L, 2);  // Stack: { UD, Key, Env, Key }
    lua_rawget(L, -2);    // Stack: { UD, Key, Env, Value }

    // Cleanup
    elm->key = "nil"; 
    
    return 1;
}

static int lua_json_object_inline_args(lua_State *L, int nargs) {
    // 1. Sanity Check: Pairs only
    if ((nargs % 2) != 0) {
        return luaL_error(L, "Invalid arguments: Object requires Key/Value pairs.");
    }

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

        // 3. Dispatch
        lua_pushvalue(L, i);     // Copy Key to top
        lua_pushvalue(L, i + 1); // Copy Value to top
        
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
	if ((lua_gettop(L) - 1) > 0) {
		int args = lua_type(L, -1);
		// get args type
		if (args == LUA_TTABLE)
			lua_json_lua_parse(L);
		else
			// stack { elm, args }
			lua_json_object_inline_args(L, nargs);

		return 1;
	}

	return 0;
};

static int
lua_json_object_gc(lua_State *L) {
	json_elm *self = check_json_elm(L, 1);
	if(self->base)
		free(self->base);

	lua_pushlightuserdata(L, (void*)self->env_id);
	lua_pushnil(L);
	lua_rawset(L, LUA_REGISTRYINDEX);

	self->event->cleanup(self->event->on_change);
    self->event->cleanup(self->event->on_newindex);

	free(self->event->on_change);
	free(self->event->on_newindex);

	free(self->event);

	return 0;
}

static int
lua_json_object_new(lua_State *L, bool parse)
{
	int nargs = lua_gettop(L);
	json_elm *elm = (json_elm *)lua_newuserdata(L, sizeof(json_elm));
	memset(elm, 0, sizeof(json_elm));
	// alloc event lists
	alloc_events(elm);
	elm->L = L;
	elm->isRoot = true;
	elm->type = JSON_OBJECT_TYPE;
	elm->typename = "object";
	elm->is_nil = false;
	elm->is_env_index = false;
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
	elm->tostring 		= &lua_json_elm_tostring;
	elm->stringify 		= &lua_json_elm_stringify;
	elm->parse 			= &lua_json_elm_parse;
	elm->render 		= &lua_json_render_object;
	elm->idx_to_key 	= &idx_to_key;
	elm->key_to_idx 	= &key_to_idx;
	elm->check_idx 		= &lua_json_elm_check_idx;
	elm->init_rlen		= &lua_json_elm_init_rlen;
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

static const luaL_Reg lua_json_object_lib_m[] = {
    // --- Standard Methods ---
    {"foreach",     lua_json_object_foreach},
    {"tojson",      lua_json_object_tojson},   // Fixes warning
    {"tolua",       lua_json_object_tolua},    // Fixes warning
	{"totable",     lua_json_elm_to_table},    // Fixes warning
    {"info",        lua_json_elm_info},
    {"len",         lua_json_elm_len},
    {"keys",        lua_json_object_keys},     // Fixes warning
    {"sort",        lua_json_object_sort},     // Fixes warning
    {"rlen",        lua_json_elm_get_rlen},
	{ "nkeys",		lua_json_elm_get_nkeys	},
	{ "quoted",		lua_json_elm_get_quoted	},
    
    // --- Memory / internal ---
    {"ref",         lua_json_object_ref},      // Fixes warning
    {"unref",       lua_json_object_unref},    // Fixes warning
    {"get_root",    object_get_root},          // Fixes warning
    
    // --- Advanced Tools (Add these if you have the C functions) ---
    {"get_env",     lua_json_elm_env_getr},
    {"env_insert",  lua_json_elm_env_insertr},
    {"env_add",     lua_json_elm_env_addr},
    {"env_rem",     lua_json_elm_env_remover},
    {"bind_dom",    L_json_elm_bind_dom},
    {"props",       lua_json_elm_get_props},
	{"ids", 		lua_json_elm_print_ids },

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
    
    // Optional: Register methods on the Factory too? (As seen in your snippet)
    luaL_register(L, NULL, lua_json_object_lib_m);
    lua_setfield(L, -2, "object"); // Attach to Module

    // --- PART 2: The Instance Type (JSON.object metatable) ---
    luaL_newmetatable(L, "JSON.object");

    // A. Create the Method Table (The Upvalue)
    // We populate a raw table with your methods to serve as the lookup cache
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