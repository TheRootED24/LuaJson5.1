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

static int lua_json_handle_table(lua_State *L);

static json_elm *lua_json_array_elm_index(lua_State *L, int pos)
{
	json_elm *elm = check_json_elm(L, pos, true);

	elm->nested = lua_json_is_elm(L, -1) ? check_json_elm(L, -1, false) : NULL;

	if (elm->nested && lua_json_elm_contains(L, elm, elm->nested))
		luaL_error(L, "ERROR: Recursive Elm Detected [ Key: %s elm: %p ]\n", elm->key, lua_topointer(L, -1));

	return elm;
}

static int lua_json_array_tojson(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1, false);
	elm->escape = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : false;
	elm->mode = MARSHAL_JSON;
	lua_settop(L, 1);

	elm->opts->stringify(L);

	return 1;
};

static int lua_json_array_tolua(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1, false);
	elm->escape = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : false;
	elm->mode = MARSHAL_LUA;
	lua_settop(L, 1);

	elm->opts->stringify(L);

	return 1;
};


static int lua_json_array_move(lua_State *L)
{
	dumpstack(L, "arr move");
	check_json_elm(L, 1, false);
	// stack { elm , move, to }
	int m = luaL_checkinteger(L, 2);
	int t = luaL_checkinteger(L, 3);

	lua_pop(L, 1);

	if(m > t || m < t) {

		get_json_table(L, 1);
		// stack { elm, move, env }

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
	json_elm *elm = lua_json_array_elm_index(L, -3);

	if (lua_json_is_elm(L, -1))
	{
		json_elm *nested = check_json_elm(L, -1, false);

		if (lua_json_elm_contains(L, elm, nested))
			luaL_error(L, "ERROR: Recursive Elm Detected [ index: %d elm: %p ]\n", elm->idx, lua_topointer(L, -1));

		elm->nested = nested ? nested : NULL;
	}

	elm_rlen erl = {0};
	elm->opts->init_rlen(elm, &erl);
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
}

static int lua_json_elm_array_del(lua_State *L)
{
	json_elm *elm = check_json_elm(L, -2, true); // lua_json_array_elm_index(L, -2);

	if (!elm->is_nil)
	{ // Only run if not pre-handled (Standard: Always Run)
		elm_rlen erl = {0};
		elm->opts->init_rlen(elm, &erl);
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
}

static int lua_json_array_reverse(lua_State *L)
{
	bool is_env = lua_istable(L, 1);
	int nargs = (lua_gettop(L) -1);
	json_elm *elm = check_json_elm(L, 1, false);
	// stack { elm }
	int start = 1, end = (int)elm->nelms;

	if(nargs) 
		start = is_env ? luaL_checkinteger(L, 2) : (luaL_checkinteger(L, 2) + 1);

	if(nargs > 1)
		end = is_env ? luaL_checkinteger(L, 3) : (luaL_checkinteger(L, 3) + 1);

	
	if(nargs)lua_pop(L, nargs);


	if(start >= end)
		return luaL_error(L, "ERROR: index: %d cannot be reversed from index: %d ", start, end);

	get_json_table(L, 1);
	// stack { elm, env }
	
	for (int s = start, e = end; s < e; s++, e--)
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

static int lua_json_elm_array_push(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1, false);
	// stack { elm, val }
	lua_pushinteger(L, elm->nelms);
	// stack { elm, val, idx }
	lua_insert(L, 2);
	// stack { elm, idx, val }
	lua_json_array_insert(L);
	// stack { elm }
	lua_pushinteger(L, elm->nelms);
	// stack { elm, nelms }
	return 1;
}

int lua_json_elm_array_pop(lua_State *L)
{
	json_elm *elm = check_json_elm(L, -1, false);
	// stack { elm }
	lua_pushinteger(L, (int)elm->nelms - 1);
	// stack { elm, pos};
	elm->is_nil = false;
	lua_json_elm_array_del(L);
	// stack { elm, val, victim };
	return 1;
};

static int lua_json_array_shift(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1, false);
	// stack { elm }
	lua_settop(L, 1);
	if (elm->nelms == 0)
		return 0;

	// Resolver bumps 0 -> 1.
	lua_pushinteger(L, 0); // Stack: [UD, 0]
	// stack { elm, 0 }
	// Delegate task to delete
	return lua_json_elm_array_del(L);
}

static int lua_json_array_unshift(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1, false);
	int n = lua_gettop(L);
	// stacl { elm, ... }
	for (int i = n; i >= 2; i--)
	{
		// disable alignment
		elm->align = false;
		lua_pushcfunction(L, lua_json_array_insert);
		// stack { elm, ..., insert }
		lua_pushvalue(L, 1); // elm
		// stack { elm, ..., insert, elm }
		lua_pushinteger(L, 1);
		// stack { elm, ..., insert, elm, 1 }
		lua_pushvalue(L, i);
		// stack { elm, ..., insert, elm, 1, i }
		lua_call(L, 3, 0);
		// stack { elm, ... }
	}
	// re-enable alignment 
	elm->align = true;
	lua_pushinteger(L, elm->nelms);
	// stack { elm, #elm }
	return 1;
}

static int lua_json_array_ref(lua_State *L)
{
	check_json_elm(L, 1, false);
	// stack { elm }
	return 1;
};

static int lua_json_array_unref(lua_State *L)
{
	json_elm *elm = check_json_elm(L, -1, false);
	// stack { elm }
	elm->opts->stringify(L);
	// stack { elm, env, json }
	lua_replace(L, 1);
	// stack { json, env }
	lua_pop(L, 1);
	// stack { json }
	lua_json_elm_parse(L);
	// { elm }
	return 1;
};

static int lua_json_array_render(lua_State *L, struct ref *seen)
{
	
	json_elm *elm = check_json_elm(L, -1, false);
	// stack { elm }
	get_json_table(L, -1);
	// stack { elm, env }
	seen->mode == MARSHAL_JSON ? seen->marshal->arr_open(seen) : seen->marshal->obj_open(seen);

	for (size_t i = 1; i <= elm->nelms; i++)
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

		if(lua_isuserdata(L, -1) && !lua_json_is_elm(L, -1)) {
			lua_pop(L, 1);
			continue;
		}

		if( ok ) {

			if(i > 1) seen->marshal->next(seen);

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
					if(lua_istable(L, -1)) {
						seen->has_refs += 1;
						lua_json_handle_table(L);
					}

					if(lua_json_is_elm(L, -1)) {
						seen->nested = check_json_elm(L, -1, false);
						seen->nested->opts->render(L, seen);
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
	json_elm *elm = lua_json_array_elm_index(L, -3);

	elm_rlen erl = {0};
	elm->opts->init_rlen(elm, &erl);
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
	// array style write
	//  stack {..., elm, key, val }
	get_json_table(L, -3);
	// stack {..., elm, key, val, env}
	lua_replace(L, -3);
	// lua_settop(L, 4);
	// lua_remove(L, -2);
	//  stack { elm, env, val }
	// lua_pushinteger(L, elm->idx);
	// stack {..., elm, env, key, val, idx }
	// lua_insert(L, -2);
	// stack {..., elm, env, key, idx, val }
	// lua_remove(L, -3);
	// stack {..., elm, env, idx, val }
	lua_rawseti(L, -2, elm->idx);

	return 0;
};

static int lua_json_array_index(lua_State *L)
{
	json_elm *elm = check_json_elm(L, -2, true); // lua_json_array_elm_index(L, -2); // check_json_elm(L, -2);

	// PATH B: String Key -> METHOD LOOKUP
	// We look inside the Method Table (Upvalue 1)
	if (lua_type(L, -1) == LUA_TSTRING)
	{
		// Optimization: Check for "env" explicitly if it's a special property
		const char *key = lua_tostring(L, -1);
		if (key && strcmp(key, "env") == 0)
		{
			get_json_table(L, 1);
			luaL_getmetatable(L, "JSON.array");
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

static int lua_json_handle_table(lua_State *L) {
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
	if((bool)lua_resume(L1, 1)) 
		luaL_error(L, "ERROR: failed to parse table !!");

	lua_xmove(L1, L, 1);

	lua_replace(L, base);
	lua_settop(L, base);

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
		// verify the value *before* pushing keys to keep the stack clean
		if (lua_isnil(L, stack_index))
		{
			fprintf(stderr, "Invalid Entry at Index: %d, aborting create array !!\n", i);
			lua_pushnil(L); // Return nil to indicate failure
			return 1;
		}

		//if(lua_istable(L, stack_index))
			//lua_json_handle_table(L, stack_index);

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
}

static int lua_json_array_init(lua_State *L, int nargs)
{
	if ((lua_gettop(L) - 1) > 0)
		lua_json_array_inline_args(L, nargs);

	if(lua_isnil(L, -1)) return 0; 
		 
	return 1;
};

static int lua_json_array_gc(lua_State *L)
{
	json_elm *self = check_json_elm(L, 1, false);

	if(self->base)
		free(self->base);

	lua_pushlightuserdata(L, (void *)self->env_id);
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

// Bundle B: The "Array" Specialist (Common Only)
static struct json_ops OPS_ARRAY = {
     // Real Hashing logic
    .tostring	= &lua_json_elm_tostring,
    .stringify	= &lua_json_elm_stringify,
    .parse      = &lua_json_elm_parse,
	
	.init_rlen	= &lua_json_elm_init_rlen,
	.check_idx	= &lua_json_elm_check_idx,
    
   .render     	= &lua_json_array_render,
   .del			= &lua_json_elm_array_del

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
	luaL_getmetatable(L, "JSON.array");
	lua_setmetatable(L, -2);
	// c side methods

	elm->opts = &OPS_ARRAY;
	/*elm->tostring = &lua_json_elm_tostring;
	elm->stringify = &lua_json_elm_stringify;
	elm->parse = &lua_json_elm_parse;
	elm->render = &lua_json_array_render;
	elm->init_rlen = &lua_json_elm_init_rlen;
	elm->check_idx = &lua_json_elm_check_idx;
	elm->del = &lua_json_elm_array_del;*/
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

static int lua_json_array_is_ref(lua_State *L) {
	json_elm *ref = check_json_elm(L, 1, false);

	lua_pushboolean(L, ref->is_ref);

	return 1;
}

static const struct luaL_Reg lua_json_array_lib_m[] = {
	// --------------------------------------
	// 1. LIFECYCLE & OPERATORS (Metamethods)
	// --------------------------------------
	{"__gc",	lua_json_array_gc	},
	{"__tostring",	lua_json_elm_tostring	},
	{"__len",	lua_json_elm_size	},

	// --------------------------------------
	// 2. CORE API (From strcmp dispatch)
	// --------------------------------------
	{"tojson", 	lua_json_array_tojson	},
	{"tolua", 	lua_json_array_tolua	},
	{"totable", 	lua_json_elm_to_table	},
	{"len", 	lua_json_elm_len	},
	{"rlen", 	lua_json_elm_get_rlen	},

	// --------------------------------------
	// 3. ARRAY MUTATION (The CRUD Suite)
	// --------------------------------------
	{"move",	lua_json_array_move	},
	{"insert",	lua_json_array_insert	},
	{"push",	lua_json_elm_array_push	},
	{"pop",		lua_json_elm_array_pop	},
	{"shift",	lua_json_array_shift	},
	{"unshift",	lua_json_array_unshift	},
	{"reverse",	lua_json_array_reverse	},
	{"del", 	lua_json_elm_array_del	},

	// --------------------------------------
	// 4. HIERARCHY & BINDING
	// ---------------------------------------
	{"ref",		lua_json_array_ref	},
	{"unref",	lua_json_array_unref	},
	{"is_stale", 	lua_json_elm_is_stale	},
	{"json_base", 	lua_json_elm_index_base	},
	{"refs",	lua_json_elm_print_refs	},
	{"is_ref",	lua_json_array_is_ref	},
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