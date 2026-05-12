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

extern const char *marshal_json[], *marshal_lua[], *marshal_bash[];

static int lua_json_handle_table(lua_State *L);

static json_elm* lua_json_array_elm_index(lua_State *L, idx_m *map)
{
	
	json_elm *elm = check_json_elm(L, 1, map);
	lua_json_elm_align_idx(elm, 2);

	elm->nested = lua_json_is_elm(L, -1) ? check_json_elm(L, -1, false) : NULL;

	if (elm->nested && lua_json_elm_contains(L, elm, elm->nested))
		luaL_error(L, "ERROR: Recursive Elm Detected [ Key: %s elm: %p ]\n", elm->key, lua_topointer(L, -1));

	return elm;
};

static int lua_json_array_tojson(lua_State *L)
{
	idx_m map = move;
	json_elm *elm = check_json_elm(L, 1, &map);
	elm->escape = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
	elm->mode = MARSHAL_JSON;

	if(elm->escape) lua_pop(L, 1);

	elm->ops->stringify(L);

	return 1;
};

static int lua_json_array_tobash(lua_State *L)
{
	idx_m map = move;
	json_elm *elm = check_json_elm(L, 1, &map);
	elm->escape = false; //lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
	elm->mode = MARSHAL_BASH;

	if(elm->escape) lua_pop(L, 1);

	elm->ops->stringify(L);

	return 1;
};


static int lua_json_array_tolua(lua_State *L)
{
	idx_m map = move;
	json_elm *elm = check_json_elm(L, 1, &map);
	elm->escape = lua_isboolean(L, -1) ? lua_toboolean(L, -1) : false;
	elm->mode = MARSHAL_LUA;

	if(elm->escape) lua_pop(L, 1);

	elm->ops->stringify(L);

	return 1;
};

int lua_json_array_move(lua_State *L)
{
	idx_m map = move;
	json_elm *elm = check_json_elm(L, 1, &map);
	// stack { elm , move_idx, to_idx }
	int m = lua_json_elm_align_idx(elm, 2);
	int t = lua_json_elm_align_idx(elm, 3);
	lua_pop(L, 1);

	if(m > t || m < t) {

		get_json_table(L, 1);
		// stack { elm, move_idx, env }
		lua_rawgeti(L, -1, m);
		// stack { elm, move_idx, env, move_val }
		lua_replace(L, 2);
		// stack {elm , move, env }
		for(int i = m; (m > t) ? i > t : i < t; (i > t) ? i-- : i++) {
			// shift to move
			lua_rawgeti(L, -1, (i > t) ? (i - 1) : (i + 1));
			// stack { elm, move, env, arr[i - 1] }
			lua_rawseti(L, -2, i);
			// stack { elm, move, env }
		}
		// stack { elm, move, env }
		lua_pushvalue(L, 2);
		// stack { elm, move, env, move }
		lua_rawseti(L, -2, t);
		// stack { elm, move, env }
		lua_pop(L, 2);
		// stack { elm }
	}

	return 0;
}

int lua_json_array_insert(lua_State *L)
{

	if (lua_gettop(L) < 3)
		return luaL_error(L, "insert usage: arr:insert([index], value)");
	
	// stack { elm , idx, val }
	idx_m map = insert;
	json_elm *elm = lua_json_array_elm_index(L, &map);

	elm->idx = lua_json_elm_align_idx(elm, 2);

	if (lua_json_is_elm(L, -1))
	{
		map = none;
		json_elm *nested = check_json_elm(L, -1, &map);

		if (lua_json_elm_contains(L, elm, nested))
			luaL_error(L, "ERROR: Recursive Elm Detected [ index: %d elm: %p ]\n", elm->idx, nested->env_id);

		elm->nested = nested ? nested : NULL;
	}

	elm_rlen erl = {0};
	elm->ops->init_rlen(elm, &erl);
	erl.toi = NEW_INDEX;

	lua_json_elm_get_val_length(L, elm, &erl);
	update_rlen(elm, &erl);

	get_json_table(L, -3);
	// stack { elm, idx, val, env }
	for (int i = elm->nelms; i >= elm->idx; i--)
	{
		lua_rawgeti(L, -1, i);
		lua_rawseti(L, -2, i + 1);
	}

	lua_pushvalue(L, -2);
	lua_rawseti(L, -2, elm->idx);

	lua_pushnumber(L, elm->base->rlen);

	return 1;
};

static int lua_json_elm_array_del(lua_State *L)
{
	idx_m map = insert;
	json_elm *elm = check_json_elm(L, 1, &map);
	lua_json_elm_align_idx(elm, 2);

	if (!elm->is_nil)
	{ // Only run if not pre-handled (Standard: Always Run)
		lua_json_elm_align_idx(elm, 2);

		elm_rlen erl = {0};
		elm->ops->init_rlen(elm, &erl);
		lua_pushnil(L);
		erl.toi = NIL_INDEX;

		lua_json_elm_get_val_length(L, elm, &erl);
		// Commit the Subtraction
		update_rlen(elm, &erl);
		// stack { elm, idx, nil }
		lua_pop(L, 1);
		// stack { elm, idx }
	}

	// Safety: Reset flag if it was set by __newindex
	elm->is_nil = 0;
	// harvest the victim
	get_json_table(L, -2);
	// stack { elm, idx, env }
	lua_rawgeti(L, -1, elm->idx);
	// stack { elm, idx, env, victim }
	bool nil_val = lua_isnil(L, -1) ? true : false;

	// stack { elm, idx, env, victim }
	for (int i = elm->idx; i < (int)elm->nelms; i++)
	{
		lua_rawgeti(L, -2, i + 1);
		lua_rawseti(L, -3, i);
	}

	if (nil_val)
	{
		// stack { elm, idx, env, nil }
		lua_pop(L, 1);
		// stack { elm, idx, env }
		lua_replace(L, -2);
		// stack { elm, env }
		elm->nelms--;
		return 0;
	}

	// stack { elm, idx, env, victim }
	lua_replace(L, 2);
	// stack { elm, victim, env }
	lua_pop(L, 1);
	// stack { elm, victim }
	elm->nelms--;

	return 1;
};

int lua_json_array_reverse(lua_State *L)
{
	int nargs = (lua_gettop(L) -1);
	idx_m map = reverse;
	json_elm *elm = check_json_elm(L, 1, &map);
	// stack { elm }
	uint16_t start = nargs ? lua_json_elm_align_idx(elm, 2) : 1; 
	uint16_t end = nargs > 1 ? lua_json_elm_align_idx(elm, 3) : (int)elm->nelms;

	if(nargs)lua_pop(L, nargs);

	get_json_table(L, 1);
	// stack { elm, env }
	
	for (uint16_t s = start < end ? start : end, e = end > start ? end : start; s < e; s++, e--)
	{
		lua_rawgeti(L, 2, s); // Get element at start
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

int lua_json_elm_array_push(lua_State *L)
{
	idx_m map = none;
	json_elm *elm = check_json_elm(L, 1, &map);
	// stack { elm, val }
	lua_pushinteger(L, elm->nelms);
	// stack { elm, val, idx }
	lua_insert(L, 2);
	// stack { elm, idx, val }
	lua_settable(L, -3);
	// stack { elm }
	lua_pushinteger(L, elm->nelms);
	// stack { elm, nelms }
	return 1;
};

int lua_json_elm_array_pop(lua_State *L)
{
	idx_m map = none;
	json_elm *elm = check_json_elm(L, 1, &map);
	// stack { elm }
	get_json_table(L, 1);
	// stack { elm, env }
	lua_rawgeti(L, -1, (int)elm->nelms);
	// stack { elm, env, val }
	lua_replace(L, 2);
	// stack { elm, val }
	lua_pushinteger(L, (int)elm->nelms-1);
	// stack { elm, val, pos};
	lua_pushnil(L);
	// stack { elm, val, pos, nil };
	lua_settable(L, -4);
	// stack { elm, val };
	lua_replace(L, 1);
	// stack { val };
	return 1;
};

static int lua_json_array_shift(lua_State *L)
{
	idx_m map = none;
	json_elm *elm = check_json_elm(L, 1, &map);
	// stack { elm }
	lua_settop(L, 1);
	if (elm->nelms == 0)
		return 0;

	lua_pushinteger(L, 0);
	// stack { elm, 0 }
	// Delegate task to delete
	return lua_json_elm_array_del(L);
};

static int lua_json_array_unshift(lua_State *L)
{
	idx_m map = none;
	json_elm *elm = check_json_elm(L, 1, &map);
	int n = lua_gettop(L);
	// stacl { elm, ... }
	for (int i = n; i >= 2; i--)
	{
		lua_pushcfunction(L, lua_json_array_insert);
		// stack { elm, ..., insert }
		lua_pushvalue(L, 1); // elm
		// stack { elm, ..., insert, elm }
		lua_pushinteger(L, 0);
		// stack { elm, ..., insert, elm, 1 }
		lua_pushvalue(L, i);
		// stack { elm, ..., insert, elm, 1, i }
		lua_call(L, 3, 0);
		// stack { elm, ... }
	}

	lua_pushinteger(L, elm->nelms);
	// stack { elm, #elm }
	return 1;
};

static int lua_json_array_ref(lua_State *L)
{
	check_json_elm(L, 1, false);
	// stack { elm }
	return 1;
};

static int lua_json_array_unref(lua_State *L)
{
	idx_m map = none;
	json_elm *elm = check_json_elm(L, -1, &map);
	// stack { elm }
	elm->ops->stringify(L);
	// stack { elm, env, json }
	lua_json_elm_parse(L);
	// { elm }
	return 1;
};

static int lua_json_array_render(lua_State *L, struct ref *seen)
{
	idx_m map = none;
	json_elm *elm = check_json_elm(L, -1, &map);

	size_t s = (seen->start > 0) ? (size_t)seen->start : 1;
	size_t e = (seen->end > 0) ? (size_t)seen->end : elm->nelms;

	seen->start= 0;
	seen->end = 0;
	// stack { elm }
	get_json_table(L, -1);
	// stack { elm, env }
	seen->mode == MARSHAL_JSON ? seen->marshal->arr_open(seen) : seen->marshal->obj_open(seen);

	for (size_t i = s; i <= e; i++)
	{
		if (lua_istable(L, -1))
			lua_rawgeti(L, -1, i);
			// stack { elm, env, val }

		bool ok = true;

		if(!lua_istable(L, -1)) {
			while (lua_isnil(L, -1))
			{
				// stack { elm, env, nil }
				ok = false;
				elm->align = false;
				elm->is_nil = true;
				
				lua_pop(L, 1);
				// stack { elm, env }
				lua_pushvalue(L, -2);
				// stack { elm, env, elm }
				lua_pushinteger(L, i);
				// stack { elm, env, elm, i }
				lua_json_elm_array_del(L);
				// stack { elm, env, elm, nil }
				lua_pop(L, 2);
				// stack { elm, env }
				lua_rawgeti(L, -1, i);
				// stack { elm, env, val }
				if (i >= elm->nelms) break;
				// how bout now ??
				ok = true;
			}
		}

		if(!seen->trusted && (seen->rlen >= seen->L_max)) {
			seen->needs_state = true;
			break;
		}

		if(lua_isuserdata(L, -1) && !lua_json_is_elm(L, -1) && !lua_json_is_int64(L, -1)) {
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

			switch (lua_type(L, -1))
			{

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

				case LUA_TUSERDATA:
				case LUA_TTABLE:
				{
					if (lua_json_is_int64(L, -1))
					{
						seen->marshal->arr_int64(L, seen);
						lua_pop(L, 2);
						break;
					}

					uintptr_t table_id = 0;

					if(lua_istable(L, -1)) {
						table_id = (uintptr_t)lua_topointer(L, -1);
						seen->has_refs += 1;
						lua_json_handle_table(L);
					}

					if(lua_json_is_elm(L, -1)) {
						seen->nested = check_json_elm(L, -1, false);
						seen->children++;

						seen->isRoot = false;
						seen->nested->ops->render(L, seen);
						seen->isRoot = (i > s && i < e) ? false : true;
						//printf("Arr Seen Root ? %s \n", seen->isRoot ? "true" : "false");

						if(table_id > 0)
							 seen->clear_next(L, seen, table_id);
						
					}

					lua_pop(L, 2);
					break;
				}

				case LUA_TBOOLEAN:
				{
					seen->marshal->arr_bool(L, seen);
					lua_pop(L, 1);
					break;
				}

				case LUA_TNIL:
				{
					lua_pushstring(L, "null");
					lua_replace(L, -2);
					seen->marshal->arr_string(L, elm, seen);
					lua_pop(L, 1);
					elm->nelms++;
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

	seen->mode == MARSHAL_JSON ? seen->marshal->arr_close(seen) : seen->marshal->obj_close(seen);

	if (lua_gettop(L) == 1)
		return 1;
	// recursion
	return 0;
};

static int lua_json_array_newindex(lua_State *L)
{
	//dumpstack(L, "array n.index");
	idx_m map = newindex;
	json_elm *elm = lua_json_array_elm_index(L, &map);

	if(lua_json_is_printable(L) || lua_isnil(L, -1)) {
		elm_rlen erl = {0};
		elm->ops->init_rlen(elm, &erl);
		elm->vtype = lua_type(L, -1);
		elm->ktype = lua_type(L, -2);

		// Determine Intent: NEW_INDEX (Append) vs EXT_INDEX (Replace) vs NIL_INDEX (Delete)
		erl.toi = type_of_index(elm);

		// Calculate Deltas (Recursive)
		// Populates erl.new (and erl.ex if replacing)
		lua_json_elm_get_val_length(L, elm, &erl);

		// Commit Transaction
		// Updates rlen, quoted, nkeys, nelms, and fires ON_CHANGE/ON_NEWINDEX
		update_rlen(elm, &erl);
		// ---------------------------------------------------------
		// STORAGE & CLEANUP
		// ---------------------------------------------------------
		if (elm->vtype == LUA_TNIL)
		{
			// Clean up Registry/VTable if deleting
			lua_pop(L, 1);
			elm->is_nil = true;

			// THE ATOMIC STRIKE: Let del handle EVERYTHING.
			lua_json_elm_array_del(L);

			return 0;
		}
	}
	// array style write
	//  stack {..., elm, key, val }
	get_json_table(L, -3);
	// stack {..., elm, key, val, env}
	lua_replace(L, -3);
	//  stack { elm, env, val }
	lua_rawseti(L, -2, elm->idx);

	return 0;
};

static int lua_json_array_index(lua_State *L)
{
	idx_m map = newindex;
	json_elm *elm = check_json_elm(L, 1, &map);

	// PATH B: String Key -> METHOD LOOKUP
	// We look inside the Method Table (Upvalue 1)
	if (lua_type(L, -1) == LUA_TSTRING)
	{
		// Optimization: Check for "env" explicitly if it's a special property
		const char *key = lua_tostring(L, -1);
		if (key && strcmp(key, "env") == 0)
		{
			get_json_table(L, 1);
			luaL_getmetatable(L, JSON_ARRAY_METHODS);
			lua_setmetatable(L, -2);
			elm->stale = true;

			if (elm->event)
			{
				event ev = {0};
				ev.type = ON_ENV;
				elm->event->set(elm->event->on_env, &ev);
			}
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
		lua_json_elm_align_idx(elm, 2);

		get_json_table(L, 1);
		// stack { elm, key, env }
		lua_rawgeti(L, -1, elm->idx);
		// stack { elm, key, env, val }
		lua_remove(L, -2);
		// stack { elm, key, val }
		return 1;
	}

	return 0;
};

static int lua_json_handle_table(lua_State *L)
{
	// stack { elm, t, ... }
	int base = lua_gettop(L);
	lua_State *L1 = lua_newthread(L);
	// stack { elm, t, ..., thread }
	lua_pushcfunction(L1, lua_json_parse_lua);
	// thread stack { parse_lua }
	lua_pushvalue(L, -2);
	// stack { elm, t, ..., thread, t }
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
		luaL_error(L, "ERROR: failed to parse table !!");

	lua_xmove(L1, L, 1);

	lua_replace(L, base);
	lua_settop(L, base);

	return 0;
};

static int lua_json_array_inline_args(lua_State *L, int nargs)
{
	// stack { elm, ... }
	// 1. Iterate 0 to N (Linear Scan)
	for (int i = 0; i < nargs; i++)
	{
		// Arguments start at stack index 2
		int stack_index = 2 + i;

		// 2. Strict Safety Check
		// verify the value *before* pushing keys to keep the stack clean
		if (lua_isnil(L, stack_index))
		{
			fprintf(stderr, "Invalid Entry at Index: %d, aborting create array !!\n", i);
			lua_pushnil(L); // Return nil to indicate failure

			return 1;
		}

		// 3. Dispatch (elm[i] = val)
		lua_pushinteger(L, i);		// Key: 0, 1, 2... (0-based for C-Elm)
		lua_pushvalue(L, stack_index); 	// Value: Copy from stack position

		// triggers __newindex(elm, i, val)
		// updates RLEN and quoted counters automatically
		lua_settable(L, 1);
	}
	// 4. Cleanup & Return
	// Leave only the 'elm' userdata on the stack
	lua_settop(L, 1);

	return 1;
};

static int lua_json_array_init(lua_State *L, int nargs)
{
	if ((lua_gettop(L) - 1) > 0)
		lua_json_array_inline_args(L, nargs);

	if(lua_isnil(L, -1)) return 0; 
		 
	return 1;
};

static int lua_json_array_gc(lua_State *L)
{
	idx_m map = none;
	json_elm *self = check_json_elm(L, 1, &map);
	
	if (!self) return 0;

	// 1. Isolate event cleanup first while the memory structures are completely intact
	if (self->event) {
		// Run your custom inner hook teardowns
		if (self->event->cleanup) {
			if (self->event->on_change) self->event->cleanup(self->event->on_change);
			if (self->event->on_newindex) self->event->cleanup(self->event->on_newindex);
			if (self->event->on_env) self->event->cleanup(self->event->on_env);
#ifdef WASM
			if (self->event->on_mutate) self->event->cleanup(self->event->on_mutate);
#endif
		}

		// Save pointers locally before clearing the struct
		void *oc = self->event->on_change;
		void *on = self->event->on_newindex;
		void *oe = self->event->on_env;
#ifdef WASM
		void *om = self->event->on_mutate;
#endif
		void *ev = self->event;

		// Break the pointers inside the struct immediately to prevent double-free
		self->event = NULL;

		// Safely free the component blocks
		free(oc);
		free(on);
		free(oe);
#ifdef WASM
		free(om);
#endif
		free(ev);
	}

	// 2. Free the primary base buffer at the absolute end of the function
	if (self->base) {
		free(self->base);
		self->base = NULL;
	}

	self->ops = NULL;
	return 0;
};

// Bundle B: The "Array" Specialist (Common Only)
static struct json_ops OPS_ARRAY = {
     // Real Hashing logic
    	.tostring	= &lua_json_elm_tostring,
    	.stringify	= &lua_json_elm_stringify,
    	.parse      = &lua_json_elm_parse,
	
		.init_rlen	= &lua_json_elm_init_rlen,
		.check_idx	= &lua_json_elm_check_idx,
    
  		.render     = &lua_json_array_render,
   		.del		= &lua_json_elm_array_del
};

static int lua_json_array_new(lua_State *L, bool parse)
{
	int nargs = lua_gettop(L);
	json_elm *elm = (json_elm *)lua_newuserdata(L, sizeof(json_elm));
	memset(elm, 0, sizeof(json_elm));
	// alloc event lists
	alloc_events(elm);
	elm->L = L;
	elm->index_json = true;
	elm->align = true;
	elm->type = JSON_ARRAY_TYPE;
	elm->typename = "array";
	elm->is_nil = false;
	elm->root = elm;
	elm->base = (elm_vlen *)malloc(sizeof(elm_vlen));
	memset(elm->base, 0, sizeof(elm_vlen));

	//  create env table
	lua_newtable(L);

	elm->env_id = (uintptr_t)lua_topointer(L, -1);

	set_json_table(L, -2);
	// elm metattable
	luaL_getmetatable(L, JSON_ARRAY_METHODS);
	lua_setmetatable(L, -2);

	// c side methods
	elm->ops = &OPS_ARRAY;
	elm->base->rlen = 2;

	lua_pushlightuserdata(L, (void *)elm->env_id);

	lua_newtable(L);
	lua_pushvalue(L, -3);
	lua_setfield(L, -2, "ctx");
	lua_newtable(L);
	lua_setfield(L, -2, "ids");

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
	if (DEBUG)
		printf("Array Vtable Id: %ld\n", elm->env_id);

	if (!parse && nargs > 0)
	{
		lua_replace(L, 1);
		lua_remove(L, 2);
		nargs -= 2;
		lua_json_array_init(L, nargs);
	}

	if (DEBUG)
		printf("NEW ARRAY SIZE: %ld\n", sizeof(json_elm));

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
	// --------------------------------------
	// 1. CORE API (From strcmp dispatch)
	// --------------------------------------
	{"tojson", 		lua_json_array_tojson	},
	{"tolua", 		lua_json_array_tolua	},
	{"tobash",		lua_json_array_tobash	},
	{"totable", 	lua_json_elm_to_table	},
	{"len", 		lua_json_elm_len		},
	{"rlen", 		lua_json_elm_get_rlen	},

	// --------------------------------------
	// 2. ARRAY MUTATION (The CRUD Suite)
	// --------------------------------------
	{"move",		lua_json_array_move		},
	{"insert",		lua_json_array_insert	},
	{"push",		lua_json_elm_array_push	},
	{"pop",			lua_json_elm_array_pop	},
	{"shift",		lua_json_array_shift	},
	{"unshift",		lua_json_array_unshift	},
	{"reverse",		lua_json_array_reverse	},
	{"del", 		lua_json_elm_array_del	},

	// --------------------------------------
	// 3. HIERARCHY & BINDING
	// ---------------------------------------
	{"ref",			lua_json_array_ref		},
	{"unref",		lua_json_array_unref	},
	{"json_base", 	lua_json_elm_index_base	},
	{NULL, NULL}
};

void lua_json_open_array(lua_State *L)
{
    // --- PART 1: The Factory ---
    lua_newtable(L); 
    lua_newtable(L); 

    lua_pushcfunction(L, lua_json_array);
    lua_setfield(L, -2, "__call"); // Use setfield for clarity

    lua_setmetatable(L, -2);

    // Register methods onto the Factory table
    luaL_reg_stack(L, lua_json_array_lib_m);
    lua_setfield(L, -2, "array"); 

    // --- PART 2: The Instance Type ---
    luaL_newmetatable(L, JSON_ARRAY_METHODS);

    // A. Method Table as Upvalue
    lua_newtable(L);
    luaL_reg_stack(L, lua_json_array_lib_m);

    // B. Optimized Index Closure
    // This pushes the closure and consumes the table above as Upvalue 1
    lua_pushcclosure(L, lua_json_array_index, 1);
    lua_setfield(L, -2, "__index");

    // D. Register other Metamethods
    // Define a small helper or just list them:
    struct { const char *name; lua_CFunction func; } metamethods[] = {
        {"__newindex", 	lua_json_array_newindex	},
        {"__gc", 		lua_json_array_gc		},
        {"__tostring", 	lua_json_elm_tostring	},
        {"__len", 		lua_json_elm_size		},
        {NULL, NULL}
    };

    for (int i = 0; metamethods[i].name; i++) {
        lua_pushcfunction(L, metamethods[i].func);
        lua_setfield(L, -2, metamethods[i].name);
    }

    lua_pop(L, 1); // Pop "JSON.array" metatable
}