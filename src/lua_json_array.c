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

extern const char *marshal_json[], *marshal_lua[];

 #define DUMPSTACK 1
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

static json_elm *lua_json_array_elm_index(lua_State *L, int pos)
{
	json_elm *elm = check_json_elm(L, pos);
	// this wil be used for staging args if neeeded
	return elm;
}

static int lua_json_array_tojson(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1);
	elm->escape = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : false;
	elm->mode = MARSHAL_JSON;
	lua_settop(L, 1);

	elm->stringify(L);

	return 1;
};

static int lua_json_array_tolua(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1);
	elm->escape = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : false;
	elm->mode = MARSHAL_LUA;
	lua_settop(L, 1);

	elm->stringify(L);

	return 1;
};

int lua_json_array_insert(lua_State *L)
{

	if (lua_gettop(L) < 3)
		return luaL_error(L, "insert usage: arr:insert([index], value)");
	// ---------------------------------------------------------
	// 1. SETUP & HELPER
	// ---------------------------------------------------------
	// stack { elm , idx, val }
	json_elm *elm = lua_json_array_elm_index(L, -3); // must be above the normalization check

	// ---------------------------------------------------------
	// 2. NESTED PREP (Critical for Measurement)
	// ---------------------------------------------------------
	// We must link the child BEFORE calling get_val_length2 so it
	// knows whether to measure a scalar or read the child's cache.
	if (lua_type(L, -1) == LUA_TUSERDATA)
	{
		json_elm *nested = check_json_elm(L, -1);

		if (lua_json_elm_contains(L, elm, nested))
			luaL_error(L, "ERROR: Recursive Elm Detected [ index: %d elm: %p ]\n", elm->idx, lua_topointer(L, -1));

		elm->nested = nested ? nested : NULL;
	}

	// ---------------------------------------------------------
	// 3. THE MATH PROTOCOL (Init -> Measure -> Update)
	// ---------------------------------------------------------
	elm_rlen erl = {0};

	// Step A: Initialize and Link base pointers
	elm->init_rlen(elm, &erl);
	erl.toi = NEW_INDEX; // toi must be set prior to call to lua_json_elm_get_val_length2
	// Step B: Measure the Value (Calculate the Delta)
	// Reads VAL at stack top (-1) and populates 'erl' (elm render length)
	lua_json_elm_get_val_length2(L, elm, &erl);

	// Step C: Commit the Update (Bubble Up)
	update_rlen(elm, &erl);

	// ---------------------------------------------------------
	// 4. PHYSICS (Shift Right)
	// ---------------------------------------------------------
	get_json_table(L, -3); // Stack: [UD, IDX, VAL, ENV]

	// Loop: From Tail down to Index
	for (int i = elm->nelms; i >= elm->idx; i--)
	{
		lua_rawgeti(L, -1, i);
		lua_rawseti(L, -2, i + 1);
	}

	// ---------------------------------------------------------
	// 5. INJECTION
	// ---------------------------------------------------------
	lua_pushvalue(L, -2);	      // Copy VAL
	lua_rawseti(L, -2, elm->idx); // ENV[idx] = VAL

	lua_pushnumber(L, elm->base->rlen);

	return 1;
}

static int lua_json_elm_array_del(lua_State *L)
{
	// ---------------------------------------------------------
	// 1. SETUP & RESOLUTION
	// ---------------------------------------------------------
	// Stack: { elm, idx }
	// lua_json_array_elm_index handles the default (pop) vs explicit (remove)
	// and ensures elm->idx is set to the target.
	// stack { elm, idx }
	json_elm *elm = lua_json_array_elm_index(L, -2);

	// ---------------------------------------------------------
	// 3. THE MATH PROTOCOL (The "Negative" Transaction)
	// ---------------------------------------------------------
	// We perform this BEFORE shifting so we measure the object
	// while it is still intact at its original index.

	if (!elm->is_nil)
	{ // Only run if not pre-handled (Standard: Always Run)
		elm_rlen erl = {0};
		elm->init_rlen(elm, &erl); // Bind Base
		lua_pushnil(L);
		// CONTEXT: WE ARE DESTROYING
		// This tells the calculator to calculate the "Removal Cost"
		// (Length of Item + Contextual Comma)
		erl.toi = NIL_INDEX;

		// Measure the Victim (at Stack Top: -1)
		lua_json_elm_get_val_length2(L, elm, &erl);

		// Commit the Subtraction
		update_rlen(elm, &erl);
	}
	// Safety: Reset flag if it was set by __newindex
	elm->is_nil = 0;
	// harvest the victim
	get_json_table(L, 1);
	// stack { elm, idx, env }
	lua_rawgeti(L, -1, elm->idx);
	// stack { elm, idx, env, victim }

	// ---------------------------------------------------------
	// 4. THE PHYSICAL SHIFT (Left Shift)
	// ---------------------------------------------------------
	if (elm->idx > (int)elm->nelms)
		elm->idx = (int)elm->nelms;

	// stack { elm, idx, env, victim }
	for (int i = elm->idx; i <= (int)elm->nelms; i++)
	{
		lua_rawgeti(L, -2, i + 1);
		lua_rawseti(L, -3, i);
	}

	// ---------------------------------------------------------
	// 6. DECREMENT NELMS AND RETURN VICTIM
	// ---------------------------------------------------------
	// Victim is still at Stack[-1].
	elm->nelms--;

	return 1;
}

static int lua_json_elm_array_reverse(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1);
	// stack { elm }
	get_json_table(L, 1);
	// stack { elm, env }
	int start = 1, end = (int)elm->nelms;
	for (int s = start, e = end; s < e; s++, e--)
	{
		lua_rawgeti(L, 2, s); // Get element at 0
		// stack { elm, env, val0 }
		lua_rawgeti(L, 2, e); // Get element at end
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

static int lua_json_elm_array_push(lua_State *L)
{
	lua_pushstring(L, "insert");
	lua_insert(L, 2);
	json_elm *elm = check_json_elm(L, 1);
	lua_remove(L, 2);
	// ---------------------------------------------------------
	// 2. INJECT INDEX
	// ---------------------------------------------------------
	// Target: Append (Insert AFTER the last element)
	// Logic: Pass 'nelms'.
	lua_pushinteger(L, elm->nelms); // Stack: [UD, VAL, IDX]
	lua_insert(L, 2);		// Stack: [UD, IDX, VAL]

	// ---------------------------------------------------------
	// 3. EXECUTE CORE
	// ---------------------------------------------------------
	// Handles: Recursion Check, Env Update, RLEN Math, Events
	lua_json_array_insert(L);

	// ---------------------------------------------------------
	// 4. RETURN RESULT
	// ---------------------------------------------------------
	lua_pushinteger(L, elm->nelms);

	return 1;
}

int lua_json_elm_array_pop(lua_State *L)
{
	json_elm *elm = check_json_elm(L, -1);
	// stack { elm }
	lua_pushinteger(L, (int)elm->nelms - 1);
	// stack { elm, pos};
	elm->is_nil = false;
	lua_json_elm_array_del(L);
	// stack { elm, val, victim };
	return 1;
};

static int lua_json_elm_array_shift(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1);

	lua_settop(L, 1);
	if (elm->nelms == 0)
		return 0;

	// 2. Target: First Element
	// Resolver bumps 0 -> 1.
	lua_pushinteger(L, 0); // Stack: [UD, 0]

	// 3. Delegate
	return lua_json_elm_array_del(L);
}

static int lua_json_elm_array_unshift(lua_State *L)
{

	// Now Stack[1] is guaranteed to be UD.
	json_elm *elm = check_json_elm(L, 1);

	// Recount stack (essential if lua_replace happened)
	int n = lua_gettop(L);

	// ---------------------------------------------------------
	// 2. REVERSE PROXY LOOP
	// ---------------------------------------------------------
	// Loop is unchanged. It works perfectly.
	for (int i = n; i >= 2; i--)
	{
		lua_pushcfunction(L, lua_json_array_insert);
		lua_pushvalue(L, 1);   // elm
		lua_pushinteger(L, 0); // index 0 will be incremented in new index +1
		lua_pushvalue(L, i);   // val
		lua_call(L, 3, 0);
	}

	lua_pushinteger(L, elm->nelms);

	return 1;
}

static int lua_json_array_ref(lua_State *L)
{
	check_json_elm(L, 1);
	// stack { elm }
	return 1;
};

static int _lua_json_array_unref(lua_State *L)
{
	json_elm *elm = check_json_elm(L, -1);
	size_t len = 0;

	elm->stringify(L);

	const char *json = lua_tolstring(L, -1, &len);
	lua_pop(L, 3);

	lua_pushlstring(L, json, len);
	lua_json_elm_parse(L);

	return 1;
};

static int lua_json_render_array(lua_State *L, struct ref *seen)
{
	json_elm *elm = check_json_elm(L, -1);
	get_json_table(L, -1);

	seen->mode == MARSHAL_JSON ? seen->marshal->arr_open(seen) : seen->marshal->obj_open(seen);

	for (size_t i = 1; i <= elm->nelms; i++)
	{
		if (lua_istable(L, -1))
			lua_rawgeti(L, -1, i);

		switch (lua_type(L, -1))
		{
			case LUA_TUSERDATA:
			{
				seen->nested = check_json_elm(L, -1);
				seen->nested->render(L, seen);
				lua_pop(L, 2);
				break;
			}

			case LUA_TSTRING:
			{
				seen->marshal->arr_string(L, elm, seen);
				lua_pop(L, 1);
				break;
			}

			case LUA_TNUMBER:
			{
				seen->marshal->arr_number(L, seen);
				lua_pop(L, 1);
				break;
			}

			case LUA_TBOOLEAN:
			{
				seen->marshal->arr_bool(L, seen);
				lua_pop(L, 1);
				break;
			}
		}

		if (elm->nelms - i >= 1)
			seen->marshal->next(seen);
	}

	seen->mode == MARSHAL_JSON ? seen->marshal->arr_close(seen) : seen->marshal->obj_close(seen);

	if (lua_gettop(L) == 1)
		return 1;
	// recursion
	return 0;
};

static int array_get_root(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1);

	printf("Elm Root: %p\n", &elm->root);
	lua_pop(L, 1);

	return 0;
}

static int
lua_json_array_newindex(lua_State *L)
{
	// ---------------------------------------------------------
	// 1. SHORT STOP & VALIDATION
	// ---------------------------------------------------------
	json_elm *elm = lua_json_array_elm_index(L, -3);
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
	printf("NEW RLEN: %ld\n", elm->base->rlen);
	// ---------------------------------------------------------
	// 5. STORAGE & CLEANUP
	// ---------------------------------------------------------
	if (elm->vtype == LUA_TNIL)
	{
		// Clean up Registry/VTable if deleting
		lua_pop(L, 1);
		elm->is_nil = true;

		// THE ATOMIC STRIKE: Let del handle EVERYTHING.
		elm->idx_set = true;
		lua_json_elm_array_del(L);

		return 0;
	}

	// array style write
	//  stack {..., elm, key, val }
	get_json_table(L, -3);
	// stack {..., elm, key, val, env}
	lua_insert(L, -3);
	// lua_settop(L, 4);
	//  stack { elm, env, key, val }
	lua_pushinteger(L, elm->idx);
	// stack {..., elm, env, key, val, idx }
	lua_insert(L, -2);
	// stack {..., elm, env, key, idx, val }
	lua_remove(L, -3);
	// stack {..., elm, env, idx, val }
	lua_rawset(L, -3);

	return 0;
};

static int lua_json_array_index(lua_State *L)
{
	json_elm *elm = lua_json_array_elm_index(L, -2); // check_json_elm(L, -2);

	dumpstack(L, "arr index");

	// PATH B: String Key -> METHOD LOOKUP
	// We look inside the Method Table (Upvalue 1)
	if (lua_type(L, -1) == LUA_TSTRING)
	{
		// Optimization: Check for "env" explicitly if it's a special property
		const char *key = lua_tostring(L, -1);
		if(key && strcmp(key, "env") == 0)
		{
			get_json_table(L, 1);
			luaL_getmetatable(L, "JSON.array");
			lua_setmetatable(L, -2);
			return 1;
		}

		// Standard Method Lookup
		lua_settop(L, 2);
		// stack { elm, key }
		lua_pushvalue(L, 2);
		// stack { elm, key, key }
		lua_rawget(L, lua_upvalueindex(1)); // Lookup in Method Table (Upvalue 1)
		// stack { elm, key, env, nil|| method }
		lua_replace(L, -2);
		// stack { elm, key, nil || method }
		// If nil, return nil (Method not found)
		// If function, it sits at top of stack
		return 1;
	}

	// stack { elm, key }
	// 2. HYBRID DISPATCH
	// PATH A: Integer Key -> DATA LOOKUP
	if (lua_type(L, -1) == LUA_TNUMBER)
	{
		get_json_table(L, 1);
		// stack { elm, key, env }
		lua_rawgeti(L, -1, elm->idx);
		// stack { elm, key, env, val }
		lua_remove(L, -2);
		// stack { elm, key, val }
		return 1;
	}

	return 0;
}

static int lua_json_array_inline_args(lua_State *L, int nargs)
{
	// stack { elm, ... }

	// 1. Iterate 0 to N (Linear Scan)
	for (int i = 0; i < nargs; i++)
	{
		// Arguments start at stack index 2
		int stack_index = 2 + i;

		// 2. Strict Safety Check
		// We verify the value *before* pushing keys to keep the stack clean
		if (lua_isnil(L, stack_index))
		{
			fprintf(stderr, "Invalid Entry at Index: %d, aborting create array !!\n", i);
			lua_pushnil(L); // Return nil to indicate failure
			return 1;
		}

		// 3. Dispatch (elm[i] = val)
		lua_pushinteger(L, i);	       // Key: 0, 1, 2... (0-based for C-Elm)
		lua_pushvalue(L, stack_index); // Value: Copy from stack position

		// triggers __newindex(elm, i, val)
		// updates RLEN and quoted counters automatically
		lua_settable(L, 1);
	}

	// 4. Cleanup & Return
	// Leave only the 'elm' userdata on the stack
	lua_settop(L, 1);
	return 1;
}

static int lua_json_array_init(lua_State *L, int nargs)
{
	// stack { elm, args }
	if (lua_istable(L, 2))
	{
		lua_remove(L, 2);
		lua_remove(L, 2);
		nargs -= 2;
	}
	if ((lua_gettop(L) - 1) > 0)
	{
		// get args type
		if (lua_type(L, -1) == LUA_TTABLE)
			lua_json_lua_parse(L);
		else
			// stack { elm, args }
			lua_json_array_inline_args(L, nargs);

		return 1;
	}

	return 0;
};

static int lua_json_array_gc(lua_State *L)
{
	json_elm *self = check_json_elm(L, 1);

	if (self->base)
		free(self->base);

	lua_pushlightuserdata(L, (void*)self->env_id);
	lua_pushnil(L);
	lua_rawset(L, LUA_REGISTRYINDEX); 

	self->event->cleanup(self->event->on_change);
	self->event->cleanup(self->event->on_newindex);

	free(self->event->on_change);
	free(self->event->on_newindex);

	free(self->event);
	// free(self->env);

	return 0;
}

static int lua_json_array_new(lua_State *L, bool parse)
{
	int nargs = lua_gettop(L);
	json_elm *elm = (json_elm *)lua_newuserdata(L, sizeof(json_elm));
	memset(elm, 0, sizeof(json_elm));
	// alloc event lists
	alloc_events(elm);
	elm->L = L;
	elm->isRoot = true;
	elm->id = (uintptr_t)lua_topointer(L, -1);
	elm->type = JSON_ARRAY_TYPE;
	elm->typename = "array";
	elm->is_nil = false;
	
	elm->root = elm;
	elm->base = (elm_vlen *)malloc(sizeof(elm_vlen));
	memset(elm->base, 0, sizeof(elm_vlen));

	//  create env table
	lua_newtable(L);
	elm->env_id = (uintptr_t)lua_topointer(L, -1);
	elm->is_env_index = false;
	set_json_table(L, -2);
	// elm metattable
	luaL_getmetatable(L, "JSON.array");
	lua_setmetatable(L, -2);
	// c side methods

	elm->tostring = &lua_json_elm_tostring;
	elm->stringify = &lua_json_elm_stringify;
	elm->parse = &lua_json_elm_parse;
	elm->render = &lua_json_render_array;
	elm->init_rlen = &lua_json_elm_init_rlen;
	elm->check_idx = &lua_json_elm_check_idx;
	elm->base->rlen = 2;

	lua_pushlightuserdata(L, (void *)elm->env_id);

	lua_newtable(L);
	lua_pushvalue(L, -3);
	lua_setfield(L, -2, "ctx");
	lua_newtable(L);
	lua_setfield(L, -2, "ids");
	lua_newtable(L);
	lua_setfield(L, -2, "props");

	lua_getfield(L, -1, "ids");
	// stack { elm, env_id, vtable, ids }
	// lua_pushvalue(L, -3);
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
	if (DEBUG) printf("Array Vtable Id: %ld\n", elm->env_id);

	if (!parse && nargs > 0)
	{
		lua_replace(L, 1);
		lua_remove(L, 2);
		nargs -= 2;
		lua_json_array_init(L, nargs);
	}

	if (DEBUG) printf("NEW ARRAY SIZE: %ld\n", sizeof(json_elm));

	return 1;
};

int lua_json_array(lua_State *L)
{
	lua_json_array_new(L, false);

	return 1;
};

int lua_json_elm_parse_array(lua_State *L)
{
	lua_json_array_new(L, true);

	return 1;
};

static const struct luaL_Reg lua_json_array_lib_m[] = {
	// ---------------------------------------------------------
	// 1. LIFECYCLE & OPERATORS (Metamethods)
	// ---------------------------------------------------------
	{"__gc", lua_json_array_gc},
	{"__tostring", lua_json_elm_tostring}, // Stringify
	{"__len", lua_json_elm_size},	   // #arr operator

	// ---------------------------------------------------------
	// 2. CORE API (From strcmp dispatch)
	// ---------------------------------------------------------
	{"tojson", lua_json_array_tojson},
	{"tolua", lua_json_array_tolua},	// Updated from generic to array specific
	{"totable", lua_json_elm_to_table}, // Fixes warning
	{"info", lua_json_elm_info},
	{"rlen", lua_json_elm_get_rlen},
	{"len", lua_json_elm_len}, // Method version of length

	// ---------------------------------------------------------
	// 3. ARRAY MUTATION (The CRUD Suite)
	// ---------------------------------------------------------
	{"insert", lua_json_array_insert},
	{"push", lua_json_elm_array_push},
	{"pop", lua_json_elm_array_pop},
	{"shift", lua_json_elm_array_shift},
	{"unshift", lua_json_elm_array_unshift},
	{"reverse", lua_json_elm_array_reverse},
	{"del", lua_json_elm_array_del},

	// ---------------------------------------------------------
	// 4. HIERARCHY & BINDING
	// ---------------------------------------------------------
	{"ref", lua_json_array_ref},
	{"unref", _lua_json_array_unref},
	{"get_root", array_get_root},
	{"get_env", lua_json_elm_env_getr},
	{"bind_dom", L_json_elm_bind_dom},
	{"props", lua_json_elm_get_props},
	{"ids", lua_json_elm_print_ids},
	{NULL, NULL}
};

void lua_json_open_array(lua_State *L)
{
	// --- PART 1: The Factory (JSON.object) ---
	lua_newtable(L); // The Factory Table
	lua_newtable(L); // The Factory's Metatable

	lua_pushstring(L, "__call");
	lua_pushcfunction(L, lua_json_array); // The Constructor
	lua_settable(L, -3);

	lua_setmetatable(L, -2);

	// Optional: Register methods on the Factory too? (As seen in your snippet)
	luaL_register(L, NULL, lua_json_array_lib_m);
	lua_setfield(L, -2, "array"); // Attach to Module

	// --- PART 2: The Instance Type (JSON.array metatable) ---
	luaL_newmetatable(L, "JSON.array");

	// A. Create the Method Table (The Upvalue)
	// We populate a raw table with your methods to serve as the lookup cache
	lua_newtable(L);
	luaL_register(L, NULL, lua_json_array_lib_m);

	// B. Create the Optimized Index Closure
	// We give 'lua_json_array_index' access to the method table (Upvalue 1)
	lua_pushcclosure(L, lua_json_array_index, 1);

	// C. Assign the Closure to __index
	// This allows us to check Methods (Upvalue) -> Then Storage (Env)
	lua_setfield(L, -2, "__index");

	// D. Register other Metamethods
	lua_pushcfunction(L, lua_json_array_newindex);
	lua_setfield(L, -2, "__newindex");

	lua_pushcfunction(L, lua_json_array_gc);
	lua_setfield(L, -2, "__gc");

	lua_pushcfunction(L, lua_json_elm_tostring);
	lua_setfield(L, -2, "__tostring");

	lua_pushcfunction(L, lua_json_elm_size);
	lua_setfield(L, -2, "__len");

	lua_pop(L, 1); // Pop "JSON.array" metatable
};