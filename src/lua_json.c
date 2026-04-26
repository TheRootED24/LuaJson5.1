#include "lua_json.h"

#define DUMPSTACK 1

#ifdef DUMPSTACK
void dumpstack(lua_State *L, const char *msg)
{
	printf("**************  [ DUMPSTACK ] : [ %s  ] ****************\n", msg);
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

const char *NULL_CACHE = NULL;

// may add more fields later
const char *fields[] = {
    "ids",
    "keys"
};


// strict type check and alignment 
json_elm *check_json_elm(lua_State *L, int pos, bool align)
{
	// stack { elm || env, ... }
	json_elm *e = NULL;
	int idx_pos = (pos + 1);

	if (lua_istable(L, pos))
	{
		pos = 1 ;
		idx_pos = 2;
		uintptr_t env_id = (uintptr_t)lua_topointer(L, pos);
		lua_pushlightuserdata(L, (void *)env_id);
		// stack { env, ..., env_id }
		lua_rawget(L, LUA_REGISTRYINDEX);
		// stack { env, ..., vtable }
		lua_getfield(L, -1, "ctx");
		// stack { env, ..., vtable, elm }
		if (lua_isuserdata(L, -1))
		{
			e = (json_elm *)lua_topointer(L, -1);
			if(!e) luaL_error(L, "ERROR: check json elm error: Elm is NULL or corrupted !!");
			// stack { env, ..., vtable, elm }
			lua_replace(L, 1);
			// stack { elm, ..., vtable}
			lua_pop(L, 1);
			// stack { elm, ... }
			// correct index for proceeding 0-indexed calls
			if (align && lua_isnumber(L, idx_pos))
			{
				// stack { elm, idx, .. }
				e->idx = lua_tointeger(L, 2);
				e->opts->check_idx(e);

				lua_pushinteger(L, (e->idx - (int)e->index_json));
				// stack { elm, idx, .. (idx -1) }
				lua_replace(L, 2);

				// stack { elm, idx-1, .. }
			}
			pos = 1;
		}
		else
			luaL_error(L, "`json env' expected, got %s\n", lua_typename(L, (lua_type(L, pos))));
	}
	else
	{
		// stack { elm, ... }
		e = (json_elm *)lua_topointer(L, pos);
		
		//bool align = e->align ? e->align : false;
		//printf("ALIGN: %s\n", btoa(e->align));
		if(e && align){
			if(e->type == JSON_ARRAY_TYPE) {
				if(e->align && lua_isnumber(L, idx_pos))
					e->idx = (lua_tointeger(L, idx_pos) + (int)e->index_json);
				else
					e->idx = lua_tointeger(L, idx_pos);

				e->align = true;
			}
			else if(lua_isnumber(L, idx_pos))
					e->idx = (lua_tointeger(L, idx_pos) + (int)e->index_json);
		}
	}	

	void *ud = NULL;
	if (!e)
	{ // if e is NULL force a lua error and bailout
		ud = luaL_checkudata(L, pos, "JSON.json");
		luaL_argcheck(L, ud != NULL, pos, "`json element' expected");
	}
	switch (e->type)
	{
		case JSON_ARRAY_TYPE:
		{
			ud = luaL_checkudata(L, pos, "JSON.array");
			luaL_argcheck(L, ud != NULL, pos, "`json array' expected");
			break;
		}
		case JSON_OBJECT_TYPE:
		{
			ud = luaL_checkudata(L, pos, "JSON.object");
			luaL_argcheck(L, ud != NULL, pos, "`json object' expected");
			break;
		}
		default:
		{
			ud = luaL_checkudata(L, pos, "JSON.json");
			luaL_argcheck(L, ud != NULL, pos, "`json element' expected");
			break;
		}
	}

	return (json_elm *)ud;
};

// check a ud's type
static int lua_json_check_udata(lua_State *L, int arg, const char *tname) 
{
    void *p = lua_touserdata(L, arg);
    
    if (p != NULL) {
        // 1. Push object's metatable
        if (lua_getmetatable(L, arg)) {             
            // Stack: [..., obj_mt]
            
            // 2. Push target metatable
            luaL_getmetatable(L, tname);            
            // Stack: [..., obj_mt, target_mt]
            
            // 3. Compare & Capture Result
            int match = lua_rawequal(L, -1, -2);
            
            // 4. Clean Stack (MUST happen regardless of match)
            lua_pop(L, 2); 
            
            // 5. Return the truth
            return match; 
        }
    }
    
    return 0; // Not a userdata, or has no metatable
}

// dertermine if ud is an elm type ( used for nesting elms and ignoring non-elm uds )
int lua_json_is_elm(lua_State *L, int pos)
{
    if (lua_json_check_udata(L, pos, "JSON.array"))  return 1;
    if (lua_json_check_udata(L, pos, "JSON.object")) return 1;
    if (lua_json_check_udata(L, pos, "JSON.json"))   return 1;

    return 0;
}

typedef struct fake_ud {
	bool is_fake;
}fake;

static fake* fake_ud(lua_State *L)
{
	void *ud = (fake*)lua_newuserdata(L, sizeof(fake));

	lua_newtable(L);
	lua_setmetatable(L, -2);

	return (fake*)ud;
}

static int new_fake(lua_State *L) {
	fake_ud(L);

	return 1;
}

uint8_t type_of_index(json_elm *elm)
{
	if (elm->idx > (int)elm->nelms)
		return NEW_INDEX;
		
	if (!elm->is_nil)
		return EXT_INDEX;
	
	elm->is_nil = false;	

	return NIL_INDEX;
}

void lua_json_elm_sub(json_elm *elm, json_elm *nested)
{
	elm->event->sub(nested->event->on_newindex, elm, lua_json_elm_on_newindex);
	elm->event->sub(nested->event->on_change, elm, lua_json_elm_on_change);
	elm->event->sub(nested->event->on_env, elm, lua_json_env_on_change);
	if (DEBUG)
		printf("Successfully Subbed Elm: %ld To Nested Elm: %ld\n", elm->env_id, nested->env_id);
}

void lua_json_elm_unsub(json_elm *elm, json_elm *nested)
{
	elm->event->unsub(nested->event->on_newindex, elm, lua_json_elm_on_newindex);
	elm->event->unsub(nested->event->on_change, elm, lua_json_elm_on_change);
	elm->event->unsub(nested->event->on_env, elm, lua_json_env_on_change);
	if (DEBUG)
		printf("Successfully Unsubbed Elm: %ld From Nested Elm: %ld\n", elm->env_id, nested->env_id);
}

void lua_json_elm_on_newindex(void *ctx, event *ev)
{
	if (DEBUG) printf("ON NEWINDEX FIRED !!!!\n");
	json_elm *self = (json_elm *)ctx;
	json_elm *nested = self->nested;

	// 1. THE FIX: Check for Delta first!
	// If this is a bubble event, 'ev->data' contains the 'elm_vlen' Delta.
	if (ev && ev->data)
	{
		elm_vlen *delta = (elm_vlen *)ev->data;
		if (DEBUG) printf("  -> Applying DELTA: %ld\n", delta->rlen);
		VLEN_ADD(*self->base, *delta, NULL);
	}
	// 2. Fallback: Only for fresh links (No Delta provided)
	else
	{
		if (DEBUG) printf("  -> Applying FULL MASS: %ld\n", nested->base->rlen);
		VLEN_ADD(*self->base, *nested->base, NULL);
	}

	// 3. The Bubble: Unconditional Propagation
	if (self->event && self->event->on_newindex && self->event->on_newindex->observers)
	{
		subject_set_values(self->event->on_newindex, (event *)ev);
	}

	// Debug Verification
	if (DEBUG)
		printf("%s %p event type: [ %d ] rlen updated: %ld\n",
		       self->typename, self, ev->type, self->base->rlen);

	return;
};

void lua_json_elm_on_change(void *ctx, event *ev)
{
	if(DEBUG) printf("ON CHANGE FIRED !!!!\n");
	json_elm *self = (json_elm *)ctx;

	// 1. Critical Safety: Check Self AND Payload
	if (!self || !self->base) return;

	// 2. The Delta Guard
	// Unlike newindex, on_change implies a modification to an existing value.
	// It MUST carry a Delta. If 'data' is NULL, it's a phantom event.
	if (ev && ev->data)
	{
		elm_vlen *delta = (elm_vlen *)ev->data;

		if(DEBUG) printf("  -> Bubble Delta: %ld\n", delta->rlen);
		// Apply the math
		VLEN_ADD(*self->base, *delta, NULL);
	}
	else
	{
		// Fail gracefully. Do NOT attempt to read NULL.
		if(DEBUG) printf("WARNING: on_change fired without delta payload. Ignored.\n");

		return;
	}

	// 3. The Bubble: Relay the Signal
	// We pass the SAME event (containing the invariant Delta) up the chain.
	if (self->event && self->event->on_change)
		subject_set_values(self->event->on_change, ev);

	// Debug Verification
	if (DEBUG) printf("%s %p event type: [ %d ] rlen updated: %ld\n",
		       			self->typename, self, ev->type, self->base->rlen);
}

int lua_json_elm_is_stale(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1, false);
	printf("elm->stale: %s", btoa(elm->stale));
	return 0;
}

void lua_json_env_on_change(void *ctx, event *ev)
{
	if(DEBUG) printf("ON ENV FIRED !!!!\n");
	json_elm *self = (json_elm *)ctx;

	// 1. Critical Safety: Check Self AND Payload
	if (!self || !self->base) return;

	// 2. The Delta Guard
	// Unlike newindex, on_change implies a modification to an existing value.
	// It MUST carry a Delta. If 'data' is NULL, it's a phantom event.
	if(self->stale) return;

	self->stale = true;

	// 3. The Bubble: Relay the Signal
	// We pass the SAME event (containing the invariant Delta) up the chain.
	if (self->event && self->event->on_env)
		subject_set_values(self->event->on_env, ev);

	// Debug Verification
	if (DEBUG && self) printf("%s %p event type: [ %s ] elm stale: %s\n",
		       			self->typename, self, "ON_ENV", btoa(self->stale));
}

void lua_json_elm_check_idx(json_elm *elm)
{
	switch (elm->type)
	{
		case JSON_OBJECT_TYPE:
		{
			if (elm->idx < 1 || elm->idx > (int)elm->nelms)
				luaL_error(elm->L, "ERROR: Index is out of bounds %d/%d\n", elm->idx, (int)elm->nelms);

			break;
		}
		case JSON_ARRAY_TYPE:
		{
			if (elm->idx < 1 || elm->idx > (int)elm->nelms + 1)
				luaL_error(elm->L, "ERROR: Index is out of bounds %d/%d\n", elm->idx, (int)elm->nelms+1);

			break;
		}
	}
}

void lua_json_elm_init_rlen(json_elm *elm, elm_rlen *erl)
{
	erl->root = elm;
	erl->base = (*elm->base);
}

static bool null_str(json_elm *elm, elm_rlen *erl)
{
	if (elm->val == NULL_CACHE)
	{
		if (erl->is_exval)
			erl->ex_vtype = LUA_TNULL;
		else
			erl->vtype = LUA_TNULL;

		erl->is_null = true;

		return true;
	}

	erl->is_exval ? erl->ex.quoted++
		      : erl->new.quoted++;
	return false;
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

elm_vlen* lua_json_table_rlen(lua_State *L, elm_vlen *tbl_rlen) {

	if(lua_istable(L, -1)) 
	{
		lua_pushvalue(L, -1);
		lua_json_handle_table(L);
		json_elm *elm = check_json_elm(L, -1, false);
		tbl_rlen = elm->base;
		lua_pop(L, 1);
	}

	return tbl_rlen;
}


static int print_table_rlen(lua_State *L) {
	elm_vlen tlen = {0};

	if(lua_istable(L, 1))
		tlen = *lua_json_table_rlen(L, &tlen);

	printf("Table rlen: %ld Table nkeys: %ld Table quoted: %ld\n", tlen.rlen, tlen.nkeys, tlen.quoted);
	
	return 0;
}

void update_rlen(json_elm *elm, elm_rlen *erl)
{
	event ev = {0};
	elm_rlen delta = {0};

	// 1. Topology (Sub/Unsub)
	// ------------------------------------------------
	// NEW_INDEX: Add Sub
	if (erl->toi == NEW_INDEX && elm->vtype == LUA_TUSERDATA && erl->sub)
	{
		lua_json_elm_sub(elm, erl->sub);
		env_val val = {.env_id = erl->sub->env_id};
		lua_json_elm_env_add(elm->L, elm, &val, ids);
	}
	// EXT/NIL: Remove Unsub
	else if (erl->unsub)
	{ // Handles both EXT and NIL cleanup
		lua_json_elm_unsub(elm, erl->unsub);
		lua_json_elm_env_rem(elm->L, elm, erl->unsub->env_id, ids);

		// EXT Only: Add new Sub (Swap)
		if (erl->toi == EXT_INDEX && elm->vtype == LUA_TUSERDATA && erl->sub)
		{
			lua_json_elm_sub(elm, erl->sub);
			env_val val = {.env_id = erl->sub->env_id};
			lua_json_elm_env_add(elm->L, elm, &val, ids);
		}
	}

	// 2. Pre-Commit Prep (Sanitize Data)
	// ------------------------------------------------
	if (erl->toi == NEW_INDEX)
	{
		if (elm->vtype == LUA_TNIL)
			luaL_error(elm->L, "JSON Error: Cannot assign 'nil' to a new key.");

		if(elm->vtype == LUA_TTABLE)
			elm->base->trefs++;

		// NKEYS Logic: Handled in get_val_length2 via erl->base.nkeys++
		elm->nelms++;
	}
	else
	{ // EXT or NIL
		// CRITICAL FIX: Zero out 'new' BEFORE modifying the Base.
		// Use 'erl->is_nil', not 'elm->is_nil'
		if (erl->is_nil)
		{
			memset(&erl->new, 0, sizeof(elm_vlen));

			// STRUCTURAL DECREMENT (The Missing Piece)
			if (elm->type == JSON_OBJECT_TYPE)
			{
				elm->base->nkeys--; // Global State
				delta.base.nkeys--; // Delta Bubble
			}
			// elm->nelms--; // Local State MUST BE UPDATED BY CALLER ( GAURD ITERATORS !!) 
		}
	}

	// 3. The Atomic Commit
	// ------------------------------------------------
	// Now that 'new' is clean, we can update the Global Base.
	VLEN_ADD(*elm->base, erl->new, (erl->toi == NEW_INDEX ? NULL : &erl->ex));

	// 4. The Bubble (Events)
	// ------------------------------------------------
	// Calculate Delta for the Event Payload
	if (erl->toi == NEW_INDEX)
	{
		VLEN_ADD(delta.base, erl->new, NULL); // Delta = New
		ev.type = ON_NEWINDEX;
	}
	else
	{
		VLEN_ADD(delta.base, erl->new, &erl->ex); // Delta = New - Old
		ev.type = ON_CHANGE;
	}

	ev.data = &delta.base;

	// Unified Event Trigger
	if (elm->event)
	{
		if (ev.type == ON_NEWINDEX && elm->event->on_newindex)
			elm->event->set(elm->event->on_newindex, &ev);
		else if (ev.type == ON_CHANGE && elm->event->on_change)
			elm->event->set(elm->event->on_change, &ev);
	}
}

void lua_json_elm_get_val_length(lua_State *L, json_elm *elm, elm_rlen *erl)
{
	bool obj = elm->type == JSON_OBJECT_TYPE;
	// 1. TARGET SELECTION (Pointer Magic)
	// direct access to the correct bucket. No memcpy needed later.
	elm_vlen *target = !erl->is_exval ? &erl->new : &erl->ex;
	// 2. RESOLVE TYPE
	int vtype = !erl->is_exval ? (elm->vtype = lua_type(L, -1)) : erl->ex_vtype;

	// ---------------------------------------------------------
	// HARVEST BLOCK
	// ---------------------------------------------------------
	if (erl->toi != NEW_INDEX && !erl->is_exval)
	{
		get_json_table(L, -3);

		if (obj)
		{
			lua_pushvalue(L, 2);
			lua_rawget(L, -2);
		}
		else
		{
			int pos = lua_isnil(L, -1) ? -3 : -1;
			lua_rawgeti(L, pos, elm->idx);
		}

		erl->ex_vtype = lua_type(L, -1);

		if (erl->ex_vtype == LUA_TUSERDATA || erl->ex_vtype == LUA_TTABLE)
		{
			if(lua_istable(L, -1)){
				elm_vlen *tlen = NULL;
				tlen = lua_json_table_rlen(L, tlen);

				VLEN_ADD(erl->ex, *tlen, NULL);


				// "key":  -> klen + 2 quotes + 1 colon = klen + 3
				erl->ex.rlen += obj ? (elm->klen + 3) : 0;

				elm->base->trefs--;
			}
			else 
			{
				if(lua_json_is_elm(L, -1))
				{
					json_elm *nested = check_json_elm(L, -1, false);

					if (nested)
					{
						// 1. Harvest the Body (The Child Object)
						VLEN_ADD(erl->ex, *nested->base, NULL);

						// "key":  -> klen + 2 quotes + 1 colon = klen + 3
						erl->ex.rlen += obj ? (elm->klen + 3) : 0;

						erl->unsub = nested;
					}
				}
			}
		}
		else
		{
			// Primitives recurse, so they hit the 'switch' later
			// which automatically adds klen. Userdata shortcuts skip the switch,
			// so we MUST do it manually above.
			erl->is_exval = true;
			lua_json_elm_get_val_length(L, elm, erl);
		}

		lua_pop(L, 2);
		erl->is_exval = false;
	}
	else
	{
		// STRUCTURAL KEY (Write to Target, NOT base)
		// We want the "New" bucket to carry +1 Key so VLEN_ADD adds it later.
		if (obj && !erl->is_exval)
			target->nkeys++;
	}

	// ---------------------------------------------------------
	// MEASUREMENT BLOCK
	// ---------------------------------------------------------
	switch (vtype)
	{
		case LUA_TUSERDATA:
		{
			if(lua_json_is_elm(L, -1))
			{
				json_elm *nested = check_json_elm(L, -1, false);
				target->rlen += obj ? (elm->klen + 3) : 0;
				
				if (!erl->is_exval) 
					erl->sub = nested;
				else
					erl->unsub = nested;

				// Add nested stats directly to target
				VLEN_ADD(*target, *nested->base, NULL);
			}

			break;
		}

		case LUA_TTABLE:
		{
			target->rlen += obj ? (elm->klen + 3) : 0;
			elm_vlen *tlen = NULL;
			tlen = lua_json_table_rlen(L, tlen);

			if(tlen)
				VLEN_ADD(*target, *tlen, NULL);

			break;
		}
		
		case LUA_TNUMBER:
		{
			lua_pushvalue(L, -1);
			elm->val = lua_tolstring(L, -1, &elm->vlen);
			target->rlen += obj ? (elm->klen + elm->vlen + 4) : (elm->vlen + 1);
			lua_pop(L, 1);
			break;
		}

		case LUA_TBOOLEAN:
		{
			elm->vlen = lua_toboolean(L, -1) == 0 ? 5 : 4;
			target->rlen += obj ? (elm->klen + elm->vlen + 4) : (elm->vlen + 1);
			break;
		}

		case LUA_TSTRING:
		{
			elm->val = lua_tolstring(L, -1, &elm->vlen);

			// null_str modifies 'target->quoted' internally via erl ptr
			if (null_str(elm, erl))
				target->rlen += obj ? (elm->klen + elm->vlen + 4) : (elm->vlen + 1);
			else
				target->rlen += obj ? (elm->klen + elm->vlen + 6) : (elm->vlen + 3);
			break;
		}

		case LUA_TNIL:
		{
			if (erl->is_exval)
			{	
				if(lua_type(L, -3) == LUA_TTABLE)
					elm->base->trefs--;

				if(obj) {
					// stack { elm, key, val, env, nil }
					lua_pushvalue(L, 2);
					elm->opts->key_to_idx(elm, true);
					// stack { elm, key, val, env, nil, key }
					lua_replace(L, -2);
					// stack { elm, key, val, env, key }
					lua_pushstring(L, "null");
					// stack { elm, key, val, env, key, null }
					lua_rawset(L, -3);
					// stack { elm, key, val, env }
					lua_pushstring(L, "null");
				 }
				 else {

					lua_rawseti(L, -2, elm->idx);
				 }
				
				target->rlen += obj ? (elm->klen + elm->vlen + 4) : (elm->vlen + 1);
				break;
				//luaL_error(L, "Existing Value %s[%d] is nil",
				//	elm->type == JSON_ARRAY_TYPE ? "index" : "key_idx",
				//	elm->idx);
			}

			if(DEBUG) printf("New Value %s[%d] is nil\n",
					elm->type == JSON_ARRAY_TYPE ? "index" : "key_idx",
					elm->idx);

			erl->is_nil = true;
			break;
		}
	}

}

bool lua_json_elm_contains(lua_State *L, json_elm *elm, json_elm *nested)
{
	// stack { ... }
	if (elm->env_id == nested->env_id) return true;
	// 1. Get Nested Object's Environment
	lua_pushlightuserdata(L, (void *)nested->env_id);
	// stack { ..., nested_env_key }
	lua_rawget(L, LUA_REGISTRYINDEX);
	// stack { ..., nested_env_table }

	// 2. Get the 'ids' Set
	lua_getfield(L, -1, fields[ids]);
	// stack { ..., nested_env_table, ids }

	// 3. Check if Parent (elm) exists in Child's (nested) IDs
	lua_pushinteger(L, elm->env_id);
	// stack { ..., nested_env_table, ids, elm_env_id }

	lua_rawget(L, -2);
	// stack { ..., nested_env_table, ids, result_bool }
	// 4. Evaluate
	// If result is true, 'elm' is already inside 'nested' -> CYCLE DETECTED.
	if (lua_toboolean(L, -1) == 1)
	{
		// 5. Cleanup
		lua_pop(L, 3);
		// stack { ... } -> Pops: result_bool, ids, nested_env_table
		return true;
	}
	lua_pop(L, 3);

	// check the root table from nested
	lua_pushlightuserdata(L, (void *)elm->env_id);
	// stack { ..., nested_env_key }
	lua_rawget(L, LUA_REGISTRYINDEX);
	// 2. Get the 'ids' Set
	lua_getfield(L, -1, fields[ids]);
	// stack { ..., nested_env_table, ids }

	// 3. Check if Parent (elm) exists in Child's (nested) IDs
	lua_pushinteger(L, nested->env_id);
	// stack { ..., nested_env_table, ids, elm_env_id }

	lua_rawget(L, -2);
	// stack { ..., nested_env_table, ids, result_bool }

	if (lua_toboolean(L, -1) == 1)
	{
		// 5. Cleanup
		lua_pop(L, 3);
		// stack { ... } -> Pops: result_bool, ids, nested_env_table
		return true;
	}
	lua_pop(L, 3);

	return false;
}

size_t __lua_json_elm_get_rlen(json_elm *elm, int mode, bool esc)
{
	size_t rlen = 0;

	switch (mode)
	{
	case MARSHAL_JSON:
		rlen += esc ? (elm->base->rlen + (elm->base->quoted * 2) + (elm->base->nkeys * 2))
			    : (elm->base->rlen);
		break;

	case MARSHAL_LUA:
		rlen += esc ? ((elm->base->rlen + (elm->base->quoted * 2)) - (elm->base->nkeys * 2))
			    : (elm->base->rlen - (elm->base->nkeys * 2));
		break;
	}

	return rlen;
}

int lua_json_elm_get_rlen(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1, false);
	int rtype = luaL_checkinteger(L, 2);
	bool esc = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : false;

	size_t rlen = __lua_json_elm_get_rlen(elm, rtype, esc);

	lua_pushnumber(L, rlen);

	return 1;
}

int lua_json_elm_env_add(lua_State *L, json_elm *elm, env_val *val, env_field field)
{
	lua_pushlightuserdata(L, (void *)elm->env_id);
	// stack { ..., env_id }
	lua_rawget(L, LUA_REGISTRYINDEX);
	// stack { ..., vtable }
	lua_getfield(L, -1, fields[field]);
	// stack { ..., vtable, ids||keys }
	size_t size = lua_objlen(L, -1);

	switch (field)
	{
		case keys:
		{
			// stack { ..., vtable, keys }
			lua_pushstring(L, val->key);
			// stack { ..., vtable, keys, key }
			lua_rawseti(L, -2, size + 1);
			// stack { ..., vtable, keys }
			break;
		}

		case ids:
		{
			// stack { ..., vtable, ids }
			lua_pushinteger(L, val->env_id);
			// stack { ..., vtable, ids, id }
			lua_pushboolean(L, 1);
			// stack { ..., vtable, ids, id, true }
			lua_rawset(L, -3);
			// stack { ..., vtable, ids }
			break;
		}

		default:
			break;
	}
	// stack { ..., vtable, ids||keys }
	lua_pop(L, 2);
	// stack { ... }
	if (DEBUG)
	{
		field == keys ? printf("%s\n", lua_pushfstring(L, "Added %s to %s[%d]", val->key, fields[field], (int)(size + 1)))
			      : printf("%s\n", lua_pushfstring(L, "Added %d to %s[%d]", val->num, fields[field], (int)(size + 1)));
		// stack { ..., fstring}
		lua_pop(L, 1);
		// stack { ... }
	}
	// stack { ... }
	return 0;
}


int lua_json_elm_to_table(lua_State *L)
{
	// 1. Retrieve the Userdata
	json_elm *elm = check_json_elm(L, 1, false);

	// 2. SAVE STATE (Critical)
	// We are about to change the mode to LUA for generation.
	// We must restore it so we don't permanently flip the object's behavior.
	int original_mode = elm->mode;
	bool original_escape = elm->escape;

	// 3. Configure for Lua Output
	// We force MARSHAL_LUA to ensure we get "{key=val}" not "{\"key\":\"val\"}"
	elm->mode = MARSHAL_LUA;
	elm->escape = false; // Standard Lua tables don't need extra escaping usually

	// 4. Generate the String
	// Pushes string to Stack[-1]
	elm->opts->stringify(L);

	// 5. RESTORE STATE
	elm->mode = original_mode;
	elm->escape = original_escape;

	// 6. Prepend "return "
	// The string "{...}" is a statement. To get the table, we need "return {...}"
	lua_pushliteral(L, "return ");
	lua_insert(L, -2); // Swap: [UD, "return ", String]
	lua_concat(L, 2);  // Join: [UD, "return {...}"]

	// 7. Compile (Loadstring)
	// Parses the string into a Bytecode Chunk (Function)
	if (luaL_loadstring(L, lua_tostring(L, -1)) != 0)
	{
		// If stringify logic is perfect, this should never happen.
		return lua_error(L);
	}

	// 8. Execute
	// Run the chunk. It returns the constructed Table.
	lua_call(L, 0, 1); // Stack: [UD, Table]

	return 1;
}

int lua_json_elm_env_insert(lua_State *L, json_elm *elm, int idx, env_val *val, env_field field)
{
	// stack {..., elm }

	lua_pushlightuserdata(L, (void *)elm->env_id);
	lua_rawget(L, LUA_REGISTRYINDEX);
	// stack {..., elm, vtable }
	lua_getfield(L, -1, fields[field]);
	// stack {..., elm, vtable, ids }
	if (idx < 1 || idx > (int)elm->nelms + 1)
	{
		fprintf(stderr, "Insert Id Index %d is out of range\n", idx);

		return 0;
	}

	for (int i = (int)elm->nelms; i >= idx; i--)
	{
		lua_rawgeti(L, -1, i); // Get element at i
		// stack {..., elm, vtable, ids, val[i] }
		lua_rawseti(L, -2, i + 1); // Move it to i + 1
					   // stack {..., elm, vtable, ids }
	}
	// stack {..., elm, vtable, ids }
	field == keys ? lua_pushstring(L, val->key) : lua_pushinteger(L, val->num);

	// stack {..., elm, vtable, ids, id }
	lua_rawseti(L, -2, idx);
	// stack {..., elm, vtable, ids }
	lua_pop(L, 2);
	// stack {..., elm }
	return 0;
}

int lua_json_elm_env_rem(lua_State *L, json_elm *elm, uintptr_t env_id, env_field field)
{
	// stack {..., elm }

	lua_pushlightuserdata(L, (void *)elm->env_id);
	// stack {..., elm, env_key }

	lua_rawget(L, LUA_REGISTRYINDEX);
	// stack {..., elm, env }

	lua_getfield(L, -1, fields[field]);
	// stack {..., elm, env, ids }

	switch (field)
	{
	case ids:
	{
		lua_pushnumber(L, env_id);
		// stack {..., elm, env, ids, rem_id }

		lua_pushnil(L);
		// stack {..., elm, env, ids, rem_id, nil }

		lua_rawset(L, -3);
		// stack {..., elm, env, ids }

		lua_pop(L, 2);
		// stack {..., elm }

		return 0;
	}
	default:
		break;
		// feature cases ..ie wasm version props table
	}
	return 0;
}

char *trim_const_char(const char *s)
{
	if (s == NULL)
		return NULL;

	const char *start = s;
	const char *end = s + strlen(s) - 1;

	// Move start pointer forward past whitespace
	while (*start && isspace((unsigned char)*start))
		start++;

	// Move end pointer backward past whitespace
	while (end > start && isspace((unsigned char)*end))
		end--;

	// Calculate length of the trimmed string
	size_t len = (start <= end) ? (size_t)(end - start + 1) : 0;

	// Allocate memory for the new string (+1 for null terminator)
	char *trimmed = (char *)malloc(len + 1);
	if (trimmed)
	{
		memcpy(trimmed, start, len);
		trimmed[len] = '\0';
	}

	return trimmed;
}

int lua_json_elm_get_quoted(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1, false);
	lua_pushnumber(L, elm->base->quoted);
	return 1;
}

int lua_json_elm_get_nkeys(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1, false);
	lua_pushnumber(L, elm->base->nkeys);
	return 1;
}

int lua_json_elm_stringify(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1, false);
	ref seen = {0};
	seen.marshal = lua_json_marshall_new(L);
	seen.mode = elm->mode ? elm->mode : MARSHAL_JSON;
	seen.escape = elm->escape ? elm->escape : false;
	seen.Marshal = elm->mode == MARSHAL_JSON ? marshal_json : marshal_lua;
	seen.rlen = __lua_json_elm_get_rlen(elm, elm->mode, elm->escape); // add 256 byte cushion

	// Determine if a second lua_State is required
	// add a libreral 1/2 elm size to rlen in case the elm is very stale
	bool need_state = ((seen.rlen + (seen.rlen / 2)) > 1024) ? true : false;
	seen.ML = need_state ? luaL_newstate() : L;

	luaL_Buffer b;
	luaL_buffinit(seen.ML, &b); // Bind Buffer to the Target State (ML)
	seen.B = &b;                // Store pointer in struct

	elm->opts->render(L, &seen);

	luaL_pushresult(seen.B);
	free(seen.marshal);
	const char *res = NULL;

	if (need_state && seen.ML) {
		res = lua_tolstring(seen.ML, -1, &seen.rlen);
		lua_pushlstring(L, res, seen.rlen);
		lua_close(seen.ML);       // Destroy the sandbox
    	}
	else
		lua_tolstring(L, -1, &seen.rlen);
	
	/* Re-sync Rlen if its free */
	elm->base->nkeys = seen.nkeys;
	elm->base->quoted = seen.quoted;
	elm->base->trefs = seen.has_refs;

	size_t esc_len = (elm->base->quoted * 2);
	size_t lua_len = (elm->base->nkeys * 2);

	if(DEBUG > 1) printf("SEEN ESCAPE: %s SEEN MODE: %s\nSEEN NKEYS: %ld SEEN QUOTED: %ld SEEN TREFS: %ld\n", seen.escape ? "true" : "false", seen.mode == MARSHAL_JSON ? "json" : "lua",
										    seen.nkeys, seen.quoted, seen.has_refs);
	if(seen.escape)
		seen.rlen -= seen.mode == MARSHAL_LUA ? (esc_len - lua_len) : (esc_len + lua_len);
		
	if( !seen.escape && seen.mode == MARSHAL_LUA )
		seen.rlen += lua_len;

	elm->base->rlen = seen.rlen;
	elm->stale = false;

	if (!DEBUG)
	{
		printf("\n\n########### result ###########\n\n");
		printf("mode: %s\nescape: %s\n", elm->mode == MARSHAL_JSON ? "json" : "lua", btoa(elm->escape));
		printf("marhsaled len: %ld\nrender len: %ld\n", __lua_json_elm_get_rlen(elm, elm->mode, elm->escape),  __lua_json_elm_get_rlen(elm, elm->mode, elm->escape)+1);
		printf("\n########### %ld/%ld ###########\n\n", __lua_json_elm_get_rlen(elm, elm->mode, elm->escape),  __lua_json_elm_get_rlen(elm, elm->mode, elm->escape)+1);

		printf("JSON C_STR MARSHAL LENGTH:\t%ld\nLUA C_STR MARSHAL LENGTH:\t%ld\nLUA MARSHAL LENGTH:\t\t%ld\nJSON MARSHAL LENGTH:\t\t%ld\n\n",
		       __lua_json_elm_get_rlen(elm, MARSHAL_JSON, true),
		       __lua_json_elm_get_rlen(elm, MARSHAL_LUA, true) ,
		       __lua_json_elm_get_rlen(elm, MARSHAL_LUA, false),
		       __lua_json_elm_get_rlen(elm, MARSHAL_JSON, false));
	}
	
	return 1;
};

int lua_json_elm_tostring(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1, false);
		lua_pushfstring(L, "%s: %p", elm->typename, elm->env_id);

	return 1;
};

int lua_json_elm_print_refs(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1, false);

	printf("ELM REFS: %ld\n", elm->base->trefs);

	return 0;
}

int lua_json_elm_len(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1, false);

	if(elm->stale || elm->base->trefs > 0) {
		uint8_t tmode = elm->mode;
		elm->mode = MARSHAL_JSON;

		elm->opts->stringify(L);

		lua_settop(L, 1);
		elm->mode = tmode;
	}

	lua_pushinteger(L, elm->base->rlen);

	return 1;
};

int lua_json_elm_size(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1, false);
	lua_pushinteger(L, elm->nelms);

	return 1;
};

static int push_number(lua_State *L, struct mg_str *val)
{
    // 1. Stack Buffer (Avoids Malloc)
    // We must copy to stack to append the \0 terminator for strtod
    char tmp[64]; 
    int copy_len = val->len < 63 ? val->len : 63;
    
    memcpy(tmp, val->buf, copy_len);
    tmp[copy_len] = '\0'; 
    
    // 2. Parse & Push
    char *end;
    lua_pushnumber(L, strtod(tmp, &end)); 

    return 1; 
}

static int detect_type(struct mg_str *json) {
    switch (json->buf[0]) {
        case '{': return JSON_OBJECT_TYPE;
        case '[': return JSON_ARRAY_TYPE;
        case '"': return JSON_STRING_TYPE;
        case 't': return JSON_BOOL_TYPE;  // 't'rue
        case 'f': return JSON_BOOL_TYPE;  // 'f'alse
        case 'n': return JSON_NULL_TYPE;  // 'n'ull
        case '-': 
        case '0' ... '9': return JSON_NUMBER_TYPE;
        default: return JSON_INVALID_TYPE;
    }
}

static int __lua_json_elm_parse(lua_State *L, struct mg_str json, int depth)
{
    bool isRoot = (depth == -1);
    //json_elm *jelm = check_json_elm(L, -1, false);
    
    int idx = 0;
    int n = 0;
    int o = mg_json_get(json, "$", &n);
    
    if (isRoot) depth = 1; // Normalize depth for the first pass


    if (json.buf[o] == '{' || json.buf[o] == '[')
    {
        struct mg_str key, val, sub = mg_str_n(json.buf + o, (size_t)n);
        size_t ofs = 0;

        while ((ofs = mg_json_next(sub, ofs, &key, &val)) > 0)
        {
            bool isKeyed = (key.len > 0);

            // 1. Prepare Key or Index
            if (isKeyed)
                (key.buf[0] == '"') ? lua_pushlstring(L, key.buf + 1, key.len - 2) 
                                    : lua_pushlstring(L, key.buf, key.len);
            else 
                lua_pushnumber(L, idx++);
            

            // 2. Handle Values (Primitives or Container Starts)
            uint8_t type = (uint8_t)detect_type(&val);
            switch (type)
            {
                case JSON_NUMBER_TYPE:
                    push_number(L, &val);
                    break;
                case JSON_BOOL_TYPE:
                    lua_pushboolean(L, *val.buf == 't');
                    break;
                case JSON_STRING_TYPE:
                    lua_pushlstring(L, val.buf + 1, val.len - 2);
                    break;
                case JSON_NULL_TYPE:
                    lua_pushlstring(L, val.buf, val.len);
                    break;
                case JSON_ARRAY_TYPE:
                    lua_json_elm_parse_array(L); // Pushes new table for array
                    break;
                case JSON_OBJECT_TYPE:
                    lua_json_elm_parse_object(L); // Pushes new table for object
                    break;
            }

            // 3. If nested, fill the table we just pushed in step 2
            if (*val.buf == '[' || *val.buf == '{') {
                // Recursion: The new table is at -1, parent env is at -3
                __lua_json_elm_parse(L, val, depth + 1);
            }

            // 4. Finalize: Set the value (primitive or filled table) into parent env
            // Stack is: [elm, key, value]
	    if(lua_isuserdata(L, -3))
            	lua_settable(L, -3);
        }
    }

    // All done! If we are back at the start, return the final result
    return (depth == 1) ? 1 : 0;
}

int lua_json_elm_parse(lua_State *L)
{
	const char *e = luaL_checkstring(L, -1);
	const char *elm = trim_const_char(e);
	lua_pop(L, lua_gettop(L));

	if (elm[0] == '{')
		lua_json_elm_parse_object(L);
	else if (elm[0] == '[')
		lua_json_elm_parse_array(L);
	else
	{
		fprintf(stderr, " (_lua_json_elm_parse) Ivalid Json Element\n");
		return 0;
	}

	struct mg_str json = mg_str(elm);
	__lua_json_elm_parse(L, json, -1);

	if (json.len > 0)
		free(json.buf);

	return 1;
};

int lua_json_elm_index_base(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1, false);

	elm->index_json = lua_isnumber(L, -1) ? !(bool)lua_tointeger(L, -1) : 1;

	return 0;
}

int lua_json_elm_info(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1, false);
	if (lua_toboolean(L, 2) != 1)
	{
		lua_pushnumber(L, elm->nelms);
		lua_pushstring(L, elm->typename);

		return 2;
	}
	else
		lua_pushfstring(L, "Elm Type: %s Elm Size: %d", elm->typename, (int)elm->nelms);

	return 1;
};

int lua_json_elm_gc(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1, false);
	if (DEBUG)
		printf("Destroy Elm: %ld\n", elm->env_id);
	// if(elm->dom_id)
	// free(elm->dom_id);

	return 0;
}

// need to refactor to use evn_val instead of void *
void alloc_events(json_elm *elm)
{
	elm->event = (elm_event *)malloc(sizeof(elm_event));
	memset(elm->event, 0, sizeof(elm_event));

	elm->event->on_newindex = (Subject *)malloc(sizeof(Subject));
	memset(elm->event->on_newindex, 0, sizeof(*elm->event->on_newindex));

	elm->event->on_change = (Subject *)malloc(sizeof(Subject));
	memset(elm->event->on_change, 0, sizeof(*elm->event->on_change));

	elm->event->on_env = (Subject *)malloc(sizeof(Subject));
	memset(elm->event->on_env, 0, sizeof(*elm->event->on_env));

	elm->event->init = &subject_init;
	elm->event->sub = &subject_subscribe;
	elm->event->set = &subject_set_values;
	elm->event->unsub = &subject_unsubscribe;
	elm->event->cleanup = &subject_cleanup;

	elm->event->init(elm->event->on_newindex);
	elm->event->init(elm->event->on_change);
	elm->event->init(elm->event->on_env);

	return;
};

static const struct luaL_Reg lua_json_methods[] = {
	{"parse",		lua_json_elm_parse	},
	{"parse_lua",		lua_json_parse_lua	},
	{"stringify",		lua_json_elm_stringify	},
	{"stringify_lua",	lua_json_lua_stringify	},
	{"len",			lua_json_elm_len	},
	{"table_len",		lua_json_lua_table_len	},
	{"parse_table",		lua_json_parse_lua	},
	{"table_rlen",		print_table_rlen	},
	{"__tostring",		lua_json_elm_tostring	},
	{"fake",		new_fake 		},
	{"__len",		lua_json_elm_size	},
	{"__gc",		lua_json_elm_gc		},
	{NULL, NULL}
};

int luaopen_JSON(lua_State *L)
{
	luaL_newmetatable(L, "JSON.json");
	lua_pushvalue(L, -1);		// pushes the metatable
	lua_setfield(L, -2, "__index"); // metatable.__index = metatable
	luaL_register(L, LUA_JSON, lua_json_methods);

	// 2. Create the String "null"
	lua_pushliteral(L, "null");

	// 3. CAPTURE THE POINTER (The Magic Step)
	// We save the internal address of this specific string instance.
	NULL_CACHE = lua_tostring(L, -1);

	// 4. Anchor it (Prevent GC)
	// We duplicate it: One for the Registry (C safety), One for Global (User)
	lua_pushvalue(L, -1);
	luaL_ref(L, LUA_REGISTRYINDEX); // Anchor 1: Registry (Keeps NULL_CACHE valid)

	lua_setglobal(L, "null"); // Anchor 2: Global (Pops the original)

	lua_json_open_array(L);
	lua_json_open_object(L);
	lua_pop(L, 3);

	return 0;
};
