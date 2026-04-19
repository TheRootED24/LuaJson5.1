#include "lua_json.h"
const char *NULL_CACHE = NULL;

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

const char *Json[] = {
    "\"%s\":",	     // JsonKey      0
    "\"%s\":\"%s\"", // ObjString    1
    "\"%s\":%f",     // ObjNumber    2
    "\"%s\":%s",     // ObjBool      3
    "\"%s\":%s",     // ObjNull      4
    "\"%s\"",	     // ArrString    5
    "%f",	     // ArrNumber    6
    "%s",	     // ArrBool      7
    "%s",	     // ArrNull      8
    ",",	     // JsonNext     9
    "{",	     // OpenObj      10
    "}",	     // CloseObj     11
    "[",	     // OpenArr      12
    "]"		     // CloseArr     13
};

const char *fields[] = {
    "children",
    "ids",
    "keys",
    "klens",
    "vlens",
    "vtypes"
};

/*int check_json_env(lua_State *L, bool align) {
	// stack { env, ... }
	if(lua_istable(L, 1)) {
		json_elm *elm = NULL;
		uintptr_t env_id = (uintptr_t)lua_topointer(L, 1);
		lua_pushlightuserdata(L, (void *)env_id);
		// stack { env, ..., env_id }
		lua_rawget(L, LUA_REGISTRYINDEX);
		// stack { env, ..., vtable }
		lua_getfield(L, -1, "ctx");
		// stack { env, ..., vtable, elm }
		if (lua_isuserdata(L, -1))
		{
			elm = check_json_elm(L, -1);
			// stack { env, ..., vtable, elm }
			lua_replace(L, 1);
			// stack { elm, ..., vtable}
			lua_pop(L, 1);
			// stack { elm, ... }
			// correct index for proceeding 0-indexed calls
			if (align)
			{
				// stack { elm, idx, .. }
				elm->idx = lua_tointeger(L, 2);
				lua_pushinteger(L, (elm->idx - 1));
				// stack { elm, idx, val, (idx -1) }
				lua_replace(L, 2);
				// stack { elm, (idx-1), .. }
			}
			// stack { elm, (idx-1), .. }
			return 1;
		}
		else 
			luaL_error(L, "`lua env' expected, got %s\n", lua_typename(L, (lua_type(L, 1))));
	}

	return 0;
}

json_elm *check_json_elm(lua_State *L, int pos, bool align)
{
	// stack { elm || env, ... }
	void *ud = NULL;
	int idx_pos = (pos + 1);

	if(lua_isuserdata(L, pos) ) {
	
		json_elm *elm = (json_elm*)lua_topointer(L, pos);
		
		if (!elm)
		{ // if e is NULL force a lua error and bailout
			ud = luaL_checkudata(L, pos, "JSON.json");
			luaL_argcheck(L, ud != NULL, pos, "`json element' expected");
		}

		switch (elm->type)
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

		if(align && elm->align && lua_isnumber(L, (pos + 1))) {
			elm->idx = (lua_tointeger(L, idx_pos) +1);
		}

		elm->align = true; 
	}
	
	return (json_elm *)ud;
};*/



/*json_elm *check_json_elm(lua_State *L, int pos)
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
			// stack { env, ..., vtable, elm }
			lua_replace(L, 1);
			// stack { elm, ..., vtable}
			lua_pop(L, 1);
			// stack { elm, ... }
			// correct index for proceeding 0-indexed calls
			if (lua_isnumber(L, idx_pos))
			{
				if(e->type == JSON_ARRAY_TYPE)
					idx_pos = 2;

				// stack { elm, idx, .. }
				e->idx = lua_tointeger(L, 2);
				e->check_idx(e);
				//if(!lua_isnil(L, -1)){
					lua_pushinteger(L, (e->idx - 1));
					// stack { elm, idx, .. (idx -1) }
					lua_replace(L, 2);
				//}
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

		if(e->type == JSON_ARRAY_TYPE) {
			if(e->align && lua_isnumber(L, idx_pos))
				e->idx = (lua_tointeger(L, idx_pos) + 1);
			else
				e->idx = lua_tointeger(L, idx_pos);

			e->align = true;
		}
		else if(lua_isnumber(L, idx_pos))
				e->idx = (lua_tointeger(L, idx_pos) + 1);
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
};*/


json_elm *check_json_elm2(lua_State *L, int pos, bool align)
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
				e->check_idx(e);

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
		if(align){
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

uint8_t type_of_index(json_elm *elm)
{
	if (elm->idx > (int)elm->nelms)
		return NEW_INDEX;
	else if (elm->is_nil)
		elm->is_nil = false;
	else
		return EXT_INDEX;

	return NIL_INDEX;
}

void lua_json_elm_sub(json_elm *elm, json_elm *nested)
{
	elm->event->sub(nested->event->on_newindex, elm, lua_json_elm_on_newindex);
	elm->event->sub(nested->event->on_change, elm, lua_json_elm_on_change);
	elm->event->sub(nested->event->on_env, elm, lua_json_env_on_change);
	if (DEBUG)
		printf("Successfully Subbed Elm: %ld To Nested Elm: %ld\n", elm->id, nested->id);
}

void lua_json_elm_unsub(json_elm *elm, json_elm *nested)
{
	elm->event->unsub(nested->event->on_newindex, elm, lua_json_elm_on_newindex);
	elm->event->unsub(nested->event->on_change, elm, lua_json_elm_on_change);
	elm->event->unsub(nested->event->on_env, elm, lua_json_env_on_change);
	if (DEBUG)
		printf("Successfully Unsubbed Elm: %ld From Nested Elm: %ld\n", elm->id, nested->id);
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
	json_elm *elm = check_json_elm2(L, 1, false);
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
				luaL_error(elm->L, "Index %d is out of range\n", elm->idx);

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

size_t lua_json_lua_elm_find_nil(lua_State *L, json_elm *elm) {
	size_t size = 0;
	// stack { elm , env }
	if(lua_istable(L, -1)) {
		switch(elm->type) {
			case JSON_OBJECT_TYPE: {
				for(size_t i = 1; i <= elm->nelms; i++ ) {
					elm->idx = i;
					bool ok = elm->idx_to_key(elm);
					if(!ok) break;
					lua_pushstring(L, elm->key);
					// stack { elm , env, key }
					lua_rawget(L, -2);
					// stack { elm , env, key }
					if(lua_isnil(L, -1)) break;
					size++;

					lua_pop(L, 1);
				}

				lua_pop(L, 1);
				break;
			}
			case JSON_ARRAY_TYPE: {
				
				for(size_t i = 1; i <= elm->nelms; i++ ) {
					lua_rawgeti(L, -1, i);
					if(lua_isnil(L, -1)) break;
					size++;

					lua_pop(L, 1);
				}

				lua_pop(L, 1);
				break;
			}
		}
	}

	return size;
}

int find_nil(lua_State *L) {
	json_elm *elm = check_json_elm2(L, 1, false);
	if(!lua_istable(L, -1))
		get_json_table(L, 1);

	size_t size = lua_json_lua_elm_find_nil(L, elm);
	lua_pushnumber(L, size);

	return 1;
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

		if (erl->ex_vtype == LUA_TUSERDATA)
		{
			json_elm *nested = check_json_elm2(L, -1, false);

			if (nested)
			{
				// 1. Harvest the Body (The Child Object)
				VLEN_ADD(erl->ex, *nested->base, NULL);

				// 2. THE FIX: Harvest the Wrapper (The Key + Punc)
				// "key":  -> klen + 2 quotes + 1 colon = klen + 3
				erl->ex.rlen += obj ? (elm->klen + 3) : 0;

				erl->unsub = nested;
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
			json_elm *nested = check_json_elm2(L, -1, false);
			target->rlen += obj ? (elm->klen + 3) : 0;

			if (!erl->is_exval)
				erl->sub = nested;
			else
				erl->unsub = nested;

			// Add nested stats directly to target
			VLEN_ADD(*target, *nested->base, NULL);
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
				if(obj) {
					// stack { elm, key, val, env, nil }
					lua_pushvalue(L, 2);
					elm->key_to_idx(elm, true);
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

	// NO MEMCPY NEEDED.
	// We wrote directly to erl->new or erl->ex via *target.
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
			    : (elm->base->rlen)+1;
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
	json_elm *elm = check_json_elm2(L, 1, false);
	int rtype = luaL_checkinteger(L, 2);
	bool esc = lua_isboolean(L, 3) ? lua_toboolean(L, 3) : false;

	size_t rlen = __lua_json_elm_get_rlen(elm, rtype, esc);

	lua_pushnumber(L, rlen);

	return 1;
}

int lua_json_elm_get_quoted(lua_State *L)
{
	json_elm *elm = check_json_elm2(L, 1, false);
	lua_pushnumber(L, elm->base->quoted);
	return 1;
}

int lua_json_elm_get_nkeys(lua_State *L)
{
	json_elm *elm = check_json_elm2(L, 1, false);
	lua_pushnumber(L, elm->base->nkeys);
	return 1;
}

int lua_json_elm_get_props(lua_State *L)
{
	json_elm *elm = check_json_elm2(L, 1, false);
	lua_pushlightuserdata(L, (void *)elm->env_id);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_getfield(L, -1, "props");

	lua_replace(L, 1);
	lua_pop(L, 1);

	return 1;
}

int lua_json_elm_env_get(lua_State *L, json_elm *elm, int idx, env_field field)
{
	lua_pushlightuserdata(L, (void *)elm->env_id);
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_getfield(L, -1, fields[field]);

	// ... bounds check ...
	if (idx < 1 || idx > (int)elm->nelms + 1)
	{
		fprintf(stderr, "Insert Id Index %d is out of range\n", idx);

		return 0;
	}

	lua_rawgeti(L, -1, idx); // Target at -1

	// THE FIX: Move the target to the position of the vtable and pop the rest
	lua_replace(L, -3); // Puts Result where VTable was
	lua_pop(L, 1);	    // Pops the Field Table

	return 1; // Now only the Result is left on the stack above the inputs
}

int lua_json_elm_print_ids(lua_State *L)
{
	json_elm *elm = check_json_elm2(L, 1, false);
	// stack { elm }
	lua_pushlightuserdata(L, (void *)elm->env_id);
	// stack { elm, env_id }
	lua_rawget(L, LUA_REGISTRYINDEX);
	// stack { elm, vtable }
	lua_getfield(L, -1, "ids");
	// stack { elm, vtable, ids }
	lua_pushnil(L);

	while (lua_next(L, -2) != 0)
	{
		if (lua_isnumber(L, -2))
			printf("id: %ld = %s\n", lua_tointeger(L, -2), btoa(lua_toboolean(L, -2)));

		lua_pop(L, 1);
	}
	lua_pop(L, 3);

	return 0;
}

int lua_json_elm_env_getr(lua_State *L)
{
	json_elm *elm = check_json_elm2(L, 1, false);

	int idx = luaL_checkinteger(L, 2);
	env_field field = luaL_checkinteger(L, 3);
	lua_json_elm_env_get(L, elm, idx, field);

	return 1; //
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

int lua_json_elm_env_addr(lua_State *L)
{
	json_elm *elm = check_json_elm2(L, 1, false);
	env_val val = {0};
	// bool is_num = false;
	if (lua_isnumber(L, 2))
	{
		val.env_id = lua_tointeger(L, 2);
		// is_num = true;
	}
	else
		val.key = luaL_checkstring(L, 2);

	env_field field = luaL_checkinteger(L, 3);

	if (DEBUG)
		field != keys ? printf("\n\nEnv Add --> elm: %ld val: %d field: %s\n\n", elm->id, val.num, fields[field])
			      : printf("\n\nEnv Add --> elm: %ld val: %s field: %s\n\n", elm->id, val.key, fields[field]);

	lua_json_elm_env_add(L, elm, &val, field);

	return 0;
}

int lua_json_elm_to_table(lua_State *L)
{
	// 1. Retrieve the Userdata
	json_elm *elm = check_json_elm2(L, 1, false);

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
	// Calls your existing logic. Pushes string to Stack[-1]
	elm->stringify(L);

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
		// If your stringify logic is perfect, this should never happen.
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

int lua_json_elm_env_insertr(lua_State *L)
{
	json_elm *elm = check_json_elm2(L, 1, false);
	int idx = luaL_checkinteger(L, 2);
	env_val val = {0};
	// bool is_num = false;

	if (lua_isnumber(L, 3))
	{
		val.num = lua_tonumber(L, 3);
		// is_num = true;
	}
	else
		val.key = luaL_checkstring(L, 3);

	env_field field = luaL_checkinteger(L, 4);

	if (DEBUG)
		field != keys ? printf("\n\nEnv Insert --> elm: %ld val: %d field: %s\n\n", elm->id, val.num, fields[field])
			      : printf("\n\nEnv Insert --> elm: %ld val: %s field: %s\n\n", elm->id, val.key, fields[field]);

	lua_json_elm_env_insert(L, elm, idx, &val, field);

	return 0; //
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

int lua_json_elm_env_remover(lua_State *L)
{
	json_elm *elm = check_json_elm2(L, 1, false);
	int idx = luaL_checkinteger(L, 2);
	env_field field = luaL_checkinteger(L, 3);

	if (DEBUG)
		printf("\n\nEnv Remove --> elm: %ld val: %d field: %s\n\n", elm->id, idx, fields[field]);
	lua_json_elm_env_rem(L, elm, idx, field);

	return 0; //
}

int L_json_elm_bind_dom(lua_State *L)
{
	// stack { elm, id_str}
	json_elm *self = check_json_elm2(L, 1, false);
	const char *id_str = luaL_checkstring(L, 2);

	// 1. Update the C-Struct for raw speed
	// if (self->dom_id) free(self->dom_id);
	self->dom_id = id_str;

	// 3. Force it into Index 0 of the 'dom_ids' table
	// We use a custom version of env_add or a manual set to hit index 0

	lua_rawgeti(L, LUA_REGISTRYINDEX, self->env_id);
	// stack { elm, id_str, vtable }
	lua_getfield(L, -1, "props");
	// stack { elm, id_str, vtable, dom_ids }
	lua_pushvalue(L, 2);
	// stack { elm, id_str, vtable, dom_ids, id_str }
	lua_setfield(L, -2, "dom_id");
	// lua_rawseti(L, -2, 0);
	//  stack { elm, id_str, vtable, dom_ids }
	lua_pop(L, 2);
	// stack { elm, id_str }

	return 0;
}

static int l_strip_all(lua_State *L)
{
	size_t len;
	const char *s = luaL_checklstring(L, -1, &len);
	// Create a temporary buffer on the C stack (or use luaL_Buffer)
	luaL_Buffer b;
	luaL_buffinit(L, &b);

	for (size_t i = 0; i < len; i++)
	{
		if (!isspace((unsigned char)s[i]))
		{
			luaL_addchar(&b, s[i]);
		}
	}
	lua_pop(L, 1);

	luaL_pushresult(&b);

	return 1;
}

char *trim_const_char(const char *s)
{
	if (s == NULL)
		return NULL;

	const char *start = s;
	const char *end = s + strlen(s) - 1;

	// Move start pointer forward past whitespace
	while (*start && isspace((unsigned char)*start))
	{
		start++;
	}

	// Move end pointer backward past whitespace
	while (end > start && isspace((unsigned char)*end))
	{
		end--;
	}

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

int lua_json_elm_stringify(lua_State *L)
{
	json_elm *elm = check_json_elm2(L, 1, false);
	ref seen = {0};
	seen.elm = elm;
	seen.max = 1;
	seen.depth = 0;
	seen.marshal = lua_json_marshall_new(L);
	seen.root = seen.last = elm->env_id;
	size_t esc_len = ((elm->base->quoted * 2) + (elm->base->nkeys * 2));
	size_t lua_len = (elm->base->nkeys * 2);
	// seen.check_next = &check_next;
	seen.mode = elm->mode ? elm->mode : MARSHAL_JSON;
	seen.escape = elm->escape ? elm->escape : false;
	seen.Marshal = elm->mode == MARSHAL_JSON ? marshal_json : marshal_lua;
	seen.rlen = __lua_json_elm_get_rlen(elm, elm->mode, elm->escape) + 256; // add 256 byte cushion
	///seen.rlen = (int)seen.rlen > 0 ? seen.rlen : (elm->base->rlen * 2);
	seen.ltype = LUA_TUSERDATA;
	
	// start the global seen_ids table
	// Determine if a second lua_State is required  
	bool need_state = ((seen.rlen + elm->nelms) > 1024) ? true : false;
	seen.ML = need_state ? luaL_newstate() : L;
	luaL_Buffer b;
    luaL_buffinit(seen.ML, &b); // Bind Buffer to the Target State (ML)
    seen.B = &b;                // Store pointer in struct

	//seen.b = malloc(seen.rlen + 1);
	//seen.b[0] = '\0'; // Start with an empty string

	/*// start the global seen_ids table
	lua_newtable(L); // --seen_ids
	// stack { ..., seen_ids }
	lua_pushinteger(L, elm->env_id);
	// stack { ..., seen_ids, env_id }
	lua_pushboolean(L, true);
	// stack { ..., seen_ids, env_id, true }
	lua_rawset(L, -3); // --> seen_ids{ ..., env_id=true }
	// stack { ..., seen_ids }
	seen.ids = luaL_ref(L, LUA_REGISTRYINDEX);*/

	elm->render(L, &seen);
	luaL_pushresult(seen.B);
	
	//seen.b = lua_tolstring(L, -1, &lt);
	// elm->base->rlen += lt + 1;
	//lua_pushlstring(L, seen.b, lt);
	//free(seen.b);
	free(seen.marshal);

	if (need_state) {
		lua_xmove(seen.ML, L, 1); // Move result from ML to L
		lua_close(seen.ML);       // Destroy the sandbox
    	}

	
	lua_tolstring(L, -1, &seen.rlen);



	// free the ids
	// luaL_unref(L, LUA_REGISTRYINDEX, seen.ids);
	if(seen.escape)
		seen.rlen -= esc_len;

	if(seen.mode == MARSHAL_LUA)
		seen.rlen += lua_len;

	if(elm->stale) elm->base->rlen = seen.rlen-1;
	elm->base->nkeys = seen.nkeys;
	elm->base->quoted = seen.quoted;
	//elm->base->rlen =  __lua_json_elm_get_rlen(elm, MARSHAL_JSON, false)-1;

	if (!DEBUG)
	{
		printf("\n\n########### result ###########\n\n");
		printf("mode: %s\nescape: %s\n", elm->mode == MARSHAL_JSON ? "json" : "lua", btoa(elm->escape));
		printf("marhsaled len: %ld\nrender len: %ld\n", __lua_json_elm_get_rlen(elm, elm->mode, elm->escape)-1,  __lua_json_elm_get_rlen(elm, elm->mode, elm->escape));
		printf("\n########### %ld/%ld ###########\n\n", __lua_json_elm_get_rlen(elm, elm->mode, elm->escape)-1,  __lua_json_elm_get_rlen(elm, elm->mode, elm->escape));

		printf("JSON C_STR MARSHAL LENGTH:\t%ld\nLUA C_STR MARSHAL LENGTH:\t%ld\nLUA MARSHAL LENGTH:\t\t%ld\nJSON MARSHAL LENGTH:\t\t%ld\n\n",
		       __lua_json_elm_get_rlen(elm, MARSHAL_JSON, true),
		       __lua_json_elm_get_rlen(elm, MARSHAL_LUA, true) ,
		       __lua_json_elm_get_rlen(elm, MARSHAL_LUA, false),
		       __lua_json_elm_get_rlen(elm, MARSHAL_JSON, false) - 1);
	}

	/*if(seen.escape)
		seen.rlen -= esc_len;

	if(seen.mode == MARSHAL_LUA)
		seen.rlen += lua_len;

	elm->base->rlen = seen.rlen;*/
	elm->stale = false;

	return 1;
};

/*bool check_next(lua_State *L, ref *seen, uintptr_t next) {

	if(next == seen->root || next == seen->last) return true;
	//printf("ELM: %ld <<====>> Nested: %ld\n", seen->root, next);
	// stack { ... }
	lua_rawgeti(L, LUA_REGISTRYINDEX, seen->ids);
	// stack { ...,  seen->ids }
	lua_pushinteger(L, next);
	// stack { ...,  seen->ids, next }
	lua_rawget(L, -2);
	// stack { ..., seen->ids, result_bool }

	if(lua_toboolean(L, -1) == 1) {
		// WE'VE SEEN IT
		// stack { ..., seen->ids, true }
		lua_pop(L, 2);
		// stack { ... }
		return true;
	}

	// ADD IT
	// stack { ..., seen->ids, false }
	lua_pop(L, 1);
	// stack { ...,  seen->ids }
	lua_pushinteger(L, next);
	// stack { ..., seen_ids, env_id }
	lua_pushboolean(L, true);
	// stack { ..., seen_ids, env_id, true }
	lua_rawset(L, -3); // --> seen_ids{ ..., env_id=true }
	// stack { ..., seen_ids }
	lua_pop(L, 1);
	// stack { ... }
	seen->last = next;

	return false;
};*/

int lua_json_elm_tostring(lua_State *L)
{
	json_elm *elm = check_json_elm2(L, 1, false);
		lua_pushfstring(L, "%s: %p", elm->typename, elm->env_id);

	return 1;
};

int lua_json_elm_len(lua_State *L)
{
	json_elm *elm = check_json_elm2(L, 1, false);

	if(elm->stale) {
		uint8_t tmode = elm->mode;
		elm->mode = MARSHAL_JSON;
		elm->stringify(L);
		lua_settop(L, 1);
		elm->mode = tmode;
	}

	lua_pushinteger(L, elm->base->rlen);

	return 1;
};

int lua_json_elm_size(lua_State *L)
{
	json_elm *elm = check_json_elm2(L, 1, false);
	lua_pushinteger(L, elm->nelms);

	return 1;
};

void json_type(parse_elm *elm)
{
	const char index = elm->index;
	if (index == '{')
	{
		elm->type = JSON_OBJECT_TYPE;
		return;
	}
	if (index == '[')
	{
		elm->type = JSON_ARRAY_TYPE;
		return;
	}

	bool isLong = ((elm->l = mg_json_get_long(*elm->json, elm->key, -1)) == -1) ? false : true;
	bool isStr = ((elm->str = mg_json_get_str(*elm->json, elm->key)) == NULL) ? false : true;
	bool isNum = mg_json_get_num(*elm->json, elm->key, &elm->num);
	bool isBool = mg_json_get_bool(*elm->json, elm->key, &elm->b);

	if (isNum)
		elm->type = JSON_NUMBER_TYPE;
	else if (isStr)
		elm->type = JSON_STRING_TYPE;
	else if (isBool)
		elm->type = JSON_BOOL_TYPE;
	else if (isLong)
		elm->type = JSON_LONG_TYPE;
	else
		elm->type = JSON_NULL_TYPE;

	return;
};

bool _isRoot = true;

static int __lua_json_elm_parse(lua_State *L, struct mg_str json, int depth)
{
	_isRoot = depth == -1 ? true : false;
	json_elm *jelm = check_json_elm2(L, -1, false);
	// stack {..., env }
	parse_elm elm = {0};
	int idx = 0;
	int n = 0, o = mg_json_get(json, "$", &n);
	if (_isRoot)
		jelm->plen = n;
	if (json.buf[o] == '{' || json.buf[o] == '[')
	{
		struct mg_str key, val, sub = mg_str_n(json.buf + o, (size_t)n);
		size_t ofs = 0;
		while ((ofs = mg_json_next(sub, ofs, &key, &val)) > 0)
		{
			bool isKeyed = key.len > 0 ? true : false;
			elm.isRoot = _isRoot;
			if (elm.isRoot)
				depth = 1;

			elm.index = val.buf[0];

			if (isKeyed)
			{
				key.buf[0] == '"' ? lua_pushlstring(L, key.buf + 1, key.len - 2) : lua_pushlstring(L, key.buf, key.len);
				elm.key = lua_pushfstring(L, "$.%s", lua_tostring(L, -1));
				lua_pop(L, 2);

				elm.json = &json;
				json_type(&elm);
			}
			else
			{
				// construct a json object with a dummy key and the value to check type
				lua_pushlstring(L, val.buf, (int)val.len);
				const char *buf = lua_pushfstring(L, "{\"%s\": %s}", "dkey", lua_tostring(L, -1));
				lua_pop(L, 2);

				struct mg_str djson = mg_str(buf);
				elm.json = &djson;
				elm.key = "$.dkey";
				json_type(&elm);
			}
	
			switch (elm.type)
			{
				case JSON_NUMBER_TYPE:
				{
					if (isKeyed)
					{
						key.buf[0] == '"' ? lua_pushlstring(L, key.buf + 1, key.len - 2) : lua_pushlstring(L, key.buf, key.len);
						lua_pushnumber(L, elm.num);
					}
					else
					{
						lua_pushnumber(L, idx++);
						lua_pushnumber(L, elm.num);
					}

					lua_settable(L, -3);
					break;
				}
				case JSON_BOOL_TYPE:
				{
					if (isKeyed)
					{
						key.buf[0] == '"' ? lua_pushlstring(L, key.buf + 1, key.len - 2) : lua_pushlstring(L, key.buf, key.len);
						lua_pushboolean(L, elm.b);
					}
					else
					{
						lua_pushnumber(L, idx++);
						lua_pushboolean(L, elm.b);
					}

					lua_settable(L, -3);
					break;
				}
				case JSON_LONG_TYPE:
				{
					if (isKeyed)
					{
						key.buf[0] == '"' ? lua_pushlstring(L, key.buf + 1, key.len - 2) : lua_pushlstring(L, key.buf, key.len);
						lua_pushnumber(L, elm.l);
					}
					else
					{
						lua_pushnumber(L, idx++);
						lua_pushnumber(L, elm.l);
					}

					lua_settable(L, -3);
					break;
				}
				case JSON_STRING_TYPE:
				{
					if (isKeyed)
					{
						key.buf[0] == '"' ? lua_pushlstring(L, key.buf + 1, key.len - 2) : lua_pushlstring(L, key.buf, key.len);
						lua_pushstring(L, elm.str);
					}
					else
					{
						lua_pushnumber(L, idx++);
						lua_pushstring(L, elm.str);
					}
					lua_settable(L, -3);
					free(elm.str);
					break;
				}
				case JSON_NULL_TYPE:
				{
					if (isKeyed)
					{
						key.buf[0] == '"' ? lua_pushlstring(L, key.buf + 1, key.len - 2) : lua_pushlstring(L, key.buf, key.len);
						lua_pushlstring(L, val.buf, val.len);
					}
					else
					{
						lua_pushnumber(L, idx++);
						lua_pushlstring(L, val.buf, val.len);
					}
					lua_settable(L, -3);
					free(elm.str);
					break;
				}
				case JSON_ARRAY_TYPE:
				{
					if (isKeyed)
					{
						key.buf[0] == '"' ? lua_pushlstring(L, key.buf + 1, key.len - 2) : lua_pushlstring(L, key.buf, key.len);
						lua_json_elm_parse_array(L);
					}
					else
					{
						lua_pushnumber(L, idx++);
						lua_json_elm_parse_array(L);
					}

					break;
				}
				case JSON_OBJECT_TYPE:
				{
					if (isKeyed)
					{
						key.buf[0] == '"' ? lua_pushlstring(L, key.buf + 1, key.len - 2) : lua_pushlstring(L, key.buf, key.len);
						lua_json_elm_parse_object(L);
					}
					else
					{
						lua_pushnumber(L, idx++);
						lua_json_elm_parse_object(L);
					}
					break;
				}
				break;
			}

			// found nested object or array
			if (*val.buf == '[' || *val.buf == '{')
			{
				switch (*val.buf)
				{
				case '[':
				{
					break;
				}
				case '{':
				{
					break;
				}
				}
				__lua_json_elm_parse(L, val, depth += 1);
			}
			// found the end of the current object / array
			char last = json.buf[ofs];
			if (last == ']' || last == '}')
			{

				// we're finished parsing when stack size is 1
				bool finished = lua_gettop(L) == 1 ? true : false;

				if (!finished)
				{
					lua_settable(L, -3);
					depth -= 1;
					idx = 0;
				}
			}
		}
	}

	// all done!! leave the table on the stack and return it
	if (depth == 1)
	{
		return 1;
	}
	// recursion
	return 0;
};

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
	json_elm *elm = check_json_elm2(L, 1, false);

	elm->index_json = lua_isnumber(L, -1) ? !(bool)lua_tointeger(L, -1) : 1;

	return 0;
}

int lua_json_elm_info(lua_State *L)
{
	json_elm *elm = check_json_elm2(L, 1, false);
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
	json_elm *elm = check_json_elm2(L, 1, false);
	if (DEBUG)
		printf("Destroy Elm: %ld\n", elm->id);
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
	{"parse",			lua_json_elm_parse		},
	{"parse_lua",		lua_json_lua_parse		},
	{"stringify",		lua_json_elm_stringify	},
	{"stringify_lua",	lua_json_lua_stringify	},
	{"strip",			l_strip_all				},
	{"len",				lua_json_elm_len		},
	{"tlen",			lua_json_parse_lua		},
	{"table_len",		lua_json_lua_table_len	},
	{"parse_table",		lua_json_parse_lua		},
	{"table_type",		lua_json_lua_is_mixed	},
	{"__tostring",		lua_json_elm_tostring	},
	{"env_get",			lua_json_elm_env_getr	},
	{"__len",			lua_json_elm_size		},
	{"__gc",			lua_json_elm_gc			},
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
