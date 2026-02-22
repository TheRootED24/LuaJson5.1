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
extern const char *Json[];

static void dumpstack(lua_State *L)
{
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
};

// C function to implement foreach(elm, func)
static int
lua_json_object_foreach(lua_State *L) {
	int nargs = lua_gettop(L);
	json_elm *elm = check_json_elm(L, 1);
	if(!elm) return 0;
	// Expects function at index 2
	luaL_checktype(L, 2, LUA_TFUNCTION);
	bool omit_keys =  nargs > 2 ? lua_isboolean(L, 3) ? lua_toboolean(L, 3) : false : false;
	lua_getfenv(L, 1);
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
	lua_getfenv(L, 1);
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
 lua_json_get_object_val_length(lua_State *L, json_elm *elm) {
	size_t rlen = 0, quoted = 0, nkeys = 0;
	int vtype = lua_type(L, -1), rets = 1;
	//stack { ..., val }
	switch(vtype) {
		case LUA_TUSERDATA: {
			rlen += (elm->klen + 3);
			//nkeys++;

			json_elm *nested = check_json_elm(L, -1);

			if(elm->id == nested->id)
				elm->recurs++;

			nested->root = elm;
			elm->nested = nested;
			rlen += nested->rlen;
			quoted += nested->quoted;
			nkeys += nested->nkeys;
			break;
		}
		case LUA_TNUMBER: {
			lua_pushvalue(L, -1);
			elm->val = lua_tolstring(L, -1, &elm->vlen);
			rlen += (elm->klen + elm->vlen + 4);
			lua_pop(L, 1);
			break;
		}
		case LUA_TBOOLEAN: {
			elm->vlen = lua_toboolean(L, -1) == 0 ? 5 : 4;
			rlen += (elm->klen + elm->vlen + 4);
			break;
		}
		
		case LUA_TSTRING: {
			elm->val = lua_tolstring(L, -1, &elm->vlen);
			// handle null sentinel here
			if ( elm->vlen  == 4 && strcmp(elm->val, "null") == 0) {
				vtype = LUA_TNULL;
				rlen += (elm->klen + elm->vlen + 4);
			}
			else 
				rlen += (elm->klen + elm->vlen + 6);
			
			break;
		}
	}

	if(vtype == LUA_TUSERDATA) {
		lua_pushnumber(L, quoted);
		lua_pushnumber(L, nkeys);
		rets += 2;
	}

	elm->vtype = elm->vtype != LUA_TNULL ? vtype : elm->vtype;

	lua_pushnumber(L, rlen);

	return rets;
};

static int
lua_json_object_tojson(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	elm->escape = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : false;
	elm->mode = MARSHAL_JSON;
	lua_settop(L, 1);
	
	//elm->tojson(L);
	elm->stringify(L);
	return 1;
};

static int
lua_json_object_tolua(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	elm->escape = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : false;
	elm->mode = MARSHAL_LUA;
	lua_settop(L, 1);

	//elm->tojson(L);
	elm->stringify(L);
	lua_getfenv(L, 1);
	lua_insert(L, -2);
	
	return 2;
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

// vtable method
// elm holds idx in elm->idx
// if key is valid at idx, its set in elm->key and return true 
// if key is not fould elm->key is set to null and return false
static bool
idx_to_key(lua_State *L, json_elm *elm) {
	//stack { ... }
	if(elm) {
		lua_rawgeti(L, LUA_REGISTRYINDEX, elm->vtable); // load its lookup table
		// stack {  ..., vtable }
		lua_getfield(L, -1, "keys");
		// stack {  ..., vtable, keys }
		lua_rawgeti(L, -1, elm->idx);
		// stack { ..., vtable, keys, key }
		if(lua_isstring(L, -1)){
			elm->key = lua_tolstring(L, -1, &elm->klen);
			// stack { ..., vtable, keys , key}
			lua_pop(L, 3);
			// stack { ... }
			return true;
		}
	}
	//stack { ... }
	return false;
};



// elm hold the key at elm->key
static int
key_to_idx(lua_State *L, json_elm *elm, bool add) {
	lua_rawgeti(L, LUA_REGISTRYINDEX, elm->vtable);
	// stack { ..., vtable }
	lua_getfield(L, -1, "keys");
	// stack { vtable, keys }
	size_t size = lua_objlen(L, -1);
	elm->idx = (int)(size + 1);
	for (size_t i = 1; i <= elm->nelms; i++)
	{
		lua_rawgeti(L, -1, i);
		// stack { ...,  vatble, keys, key }
		if (lua_isstring(L, -1))
			if (strcmp(elm->key, lua_tostring(L, -1)) == 0) {
				// stack { ..., vtable, keys, key }
				lua_pop(L, 3);
				// stack { ... }
				elm->idx = (int)i;
				return (int)i;
			}

		lua_pop(L, 1); // pop ex_key
		// stack { ..., vtable, keys }
	}
	// new key, add it
	// stack { ..., vtable, keys }
	if (add)
	{
		env_val val = {0};
		//lua_json_env_manager_init(elm, &env);
		val.key = elm->key;
		//elm->env->add(L, elm, &val, keys);
		lua_json_elm_env_add(L, elm, &val, keys);
		lua_pop(L, 2);
		// stack { ... }
		return ( (int)size + 1 );
	}
	else
		lua_pop(L, 2);
	
	// stack { ... }
	return ((int)size + 1);
};


static int
lua_json_object_keys(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);

	if(elm->type == JSON_OBJECT_TYPE) {
		lua_getfenv(L, 1);
		lua_rawgeti(L, -1, 0);
		lua_insert(L, 2);
		lua_pop(L, 1);

		lua_json_lua_parse(L);

		return 1;
	}
	else
		lua_pushnil(L);

	return 1;
};

static void
lua_json_object_on_newindex(void* ctx, size_t rlen, size_t quoted, size_t nkeys) {
	json_elm *self = (json_elm*)ctx;
	self->rlen += rlen;
	self->quoted += quoted;
	self->nkeys += nkeys;

	printf("%s %p rlen updated: %ld quoted updated: %ld nkeys updated: %ld\n", self->typename, self, self->rlen, self->quoted, self->nkeys);
	//self = NULL;

	return;
}

static void
lua_json_object_on_change(void* ctx, size_t rlen, size_t quoted, size_t nkeys)  {
	json_elm *self = (json_elm*)ctx;
	self->rlen += rlen;
	self->quoted += quoted;
	self->nkeys += nkeys;

	printf("%s %p rlen updated: %ld quoted updated: %ld nkeys updated: %ld\n", self->typename, self, self->rlen, self->quoted, self->nkeys);
	//self = NULL;

	return;
}

static int
lua_json_render_object(lua_State *L, struct ref *seen) 
{
	json_elm *elm = check_json_elm(L, -1);
	seen->elm = elm;
	lua_getfenv(L, -1);

	seen->marshal->obj_open(seen);
	for(size_t i = 1; i <= elm->nelms; i++) {
		elm->idx = i;
		elm->idx_to_key(L, elm);
		if(strcmp(elm->key, "0") == 0) 
			continue;

		lua_getfield(L, -1, elm->key);
		
		switch(lua_type(L, -1)) {
			case LUA_TUSERDATA: {

				uintptr_t next = (uintptr_t)lua_topointer(L, -1);
				seen->nested = check_json_elm(L, -1);
				if(seen->check_next(seen, next)) {
					lua_pop(L, 1);
					size_t len = strlen(seen->b);
					seen->b[len-1] = 0;
					break; 
				}
				seen->marshal->obj_key(L, elm, seen);
				//lua_pop(L, 1);

				json_elm *nested = check_json_elm(L, -1);
				nested->render(L, seen);
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

static int
lua_json_object_del_key(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1);
	// stack { elm, ... }
	lua_getfenv(L, 1);
	// stack { elm, ... env }
	lua_rawgeti(L, -1, 0);
	size_t size = lua_objlen(L, -1);
	for(size_t i = elm->idx; i <= size; i++) {
		// stack { elm, ... env, keys }
		lua_rawgeti(L, -1, i + 1);
		// stack { elm, ... env, keys, keys[i+1]}
		lua_rawseti(L, -2, i);
		// stack { elm, ... env, keys }
	}
	// stack { elm, ... env, keys }
	lua_pop(L, 2);
	// stack { elm, ... }
	elm->nkeys--;
	return 0;
};

bool
is_recursive(lua_State *L, json_elm *elm, json_elm *nested) 
{
	if(nested->id == elm->id) {
		lua_pushfstring(L, "Error: recursive element %d --> %d\n", (int)nested->id, (int)elm->id);

		return true;
	}

	return false;
}

//bool send_conn = false;
static int
lua_json_object_newindex(lua_State *L) 
{
	json_elm *elm = check_json_elm(L, -3);
	json_elm *nested = lua_type(L, -1) == LUA_TUSERDATA ? check_json_elm(L, -1) : NULL;
	//if(!send_conn) { printf("Connecting to Server ...\n"); sleep(1); send_conn = true; }
	//else
		//printf("sending data...\n");
	//eval_len el = {0};
	//lua_json_elm_init_len(elm, &el)
	size_t prev_rlen = elm->rlen;
	size_t prev_quoted = elm->quoted;
	size_t prev_nkeys = elm->nkeys;

	int vtype = lua_type(L, -1); //, ktype = lua_type(L, -2);
	bool is_new = false;

	if(vtype == LUA_TUSERDATA) {
		nested = check_json_elm(L, -1);
		if(is_recursive(L, elm, nested))
			lua_error(L);
	}

	elm->vtype = lua_type(L, -1);
	elm->ktype = lua_type(L, -2);
	// stack { ..., elm, key, val }

	switch(elm->ktype) {
		case LUA_TNUMBER: {
			elm->idx = (luaL_checkinteger(L, -2) +1);

			if(elm->idx < 1 || elm->idx > (int)elm->nelms) {
				fprintf(stderr, "ERROR: Index is out of bounds %d\n", elm->idx );
				lua_error(L);
				return 1;
			}

			bool ok = idx_to_key(L, elm);
			if(ok) printf("It works -------------------->>>>>>>>>>>>> key [ %s ]\n", elm->key);
			// stack { ..., elm, key, val }
			lua_pushstring(L, elm->key);
			// stack { ..., elm, key, val, key }
			lua_insert(L, -3);
			lua_remove(L, -2);
			
			// stack { ..., elm, key, val }
			break;
		}
		case LUA_TSTRING: {
			
			elm->key = lua_tolstring(L, -2, &elm->klen);
			int ok = key_to_idx(L, elm, true);

			if(ok) printf("It works -------------------->>>>>>>>>>>>> idx [ %d  %d ]\n", elm->idx, ok);

			break;
		}
	}

	if(elm->idx > (int)elm->nelms) {
		lua_json_get_object_val_length(L, elm);
		elm->rlen += (size_t)luaL_checknumber(L, -1);
		lua_pop(L, 1);

		if(elm->vtype == LUA_TUSERDATA) {
			elm->quoted += (size_t)luaL_checknumber(L, -2);
			elm->nkeys += (size_t)luaL_checknumber(L, -1);
			lua_pop(L, 2);

			nested = check_json_elm(L, -1);
			//env_val val = {0};
			env_val val = {0};
			//lua_json_env_manager_init(elm, &env);
			val.num = nested->vtable;
			elm->event->sub(nested->event->on_newindex, elm, lua_json_object_on_newindex);
			elm->event->sub(nested->event->on_change, elm, lua_json_object_on_change);
			//elm->event->sub(elm);
			lua_json_elm_env_add(L, elm, &val, ids);

		}

		if(elm->vtype == LUA_TSTRING)
			elm->quoted++;

		elm->nkeys++;

		is_new = true;
	}
	else if(elm->idx <= (int)elm->nelms) {
		//lua_pop(L, 3);
		lua_getfenv(L, 1);
		// stack { ..., elm, key, val, env }
		lua_getfield(L, -1, elm->key);
		// stack { ..., elm, key, val, env, ex_val }
		lua_json_get_object_val_length(L, elm);
		// stack { ..., elm, key, val, env, ex_val, ex_vlen }
		elm->rlen -= luaL_checknumber(L, -1);
		lua_pop(L, 1);

		if(elm->vtype == LUA_TUSERDATA) {
				elm->quoted -= (size_t)luaL_checknumber(L, -2);
				elm->nkeys -= (size_t)luaL_checknumber(L, -1);
				lua_pop(L, 2);
			}

			if(elm->vtype == LUA_TSTRING )
				elm->quoted--;

		lua_pop(L, 2);
		// stack { elm, key, val }
	}

	if(!is_new && vtype != LUA_TNIL && elm->idx <= (int)elm->nelms) {
		lua_json_get_object_val_length(L, elm);
		elm->rlen += luaL_checknumber(L, -1);
		lua_pop(L, 1);

		if(elm->vtype == LUA_TUSERDATA) {
			elm->quoted += (size_t)luaL_checknumber(L, -2);
			elm->nkeys += (size_t)luaL_checknumber(L, -1);
			lua_pop(L, 2);
		}

		if(elm->vtype == LUA_TSTRING )
			elm->quoted++;
		
		elm->event->set(elm->event->on_change, ((elm->rlen - prev_rlen)), ((elm->quoted - prev_quoted)), ((elm->nkeys - prev_nkeys)));
	}
	
	if (vtype == LUA_TNIL)  {
		elm->rlen -= elm->nelms > 1 ?  0 : -1;
		lua_json_object_del_key(L);
		elm->event->set(elm->event->on_change, ((elm->rlen - prev_rlen)), ((elm->quoted - prev_quoted)), ((elm->nkeys - prev_nkeys)));
		if(elm->nested){
			printf("Unsubbubg ..\n");
			elm->event->unsub(elm->nested->event->on_newindex, elm, lua_json_object_on_newindex);
			elm->event->unsub(elm->nested->event->on_change,   elm, lua_json_object_on_change);
		}
		elm->nelms--;
	}
	

	lua_getfenv(L, 1);
	// stack {..., udata, key, val, env}
	lua_insert(L, 2);
	// stack { udata, env, key, val }
	lua_settop(L, 4);
	// stack { udata, env, key, val }
	lua_rawset(L, -3); 

	if(vtype != LUA_TNIL && (elm->idx > (int)elm->nelms)) {
		elm->event->set(elm->event->on_newindex, (elm->rlen - prev_rlen), (elm->quoted - prev_quoted), (elm->nkeys - prev_nkeys));
		elm->nelms++;
	}
	
	//elm->key = "nil";
	//elm->idx = 1;
	
    return 0;
};

//bool recv_conn = false;
static int
lua_json_object_index(lua_State *L)
{
	json_elm *elm = check_json_elm(L, 1);
	if(!elm) return 0;
	
	//if(!recv_conn) { printf("Connecting to Server ...\n"); sleep(1); recv_conn = true; }
	//else
		//printf("fetching data...\n");

	// stack { udata, key }
	int ktype = lua_type(L, -1);
	switch(ktype) {
		case LUA_TSTRING: {
			elm->key = lua_tostring(L, 2);
			break;
		}
		case LUA_TNUMBER: {
			// stack { udata, key }
			
			elm->idx = (luaL_checkinteger(L, -1) +1);
			bool ok = idx_to_key(L, elm);

			lua_pop(L, 1);
			lua_pushstring(L, elm->key);
			if(ok) printf("It works -------------------->>>>>>>>>>>>> key[%d]: %s\n", elm->idx, elm->key);
			dumpstack(L);
			// stack { udata, key }
			break;
		}
	}
	
	if(strcmp("0", elm->key) == 0) {
		return 1;
	}
	if(strcmp("env", elm->key) == 0) {
		lua_getfenv(L, 1);
		return 1;
	}
	else if(strcmp("tojson", elm->key) == 0) {
			lua_pushcfunction(L, lua_json_object_tojson);
			return 1;
	}
	else if(strcmp("tolua", elm->key) == 0) {
			lua_pushcfunction(L, lua_json_object_tolua);
			return 1;
	}
	else if(strcmp("foreach", elm->key) == 0) {
			lua_pushcfunction(L, lua_json_object_foreach);
			return 1;
	}
	else if(strcmp("sort", elm->key) == 0) {
			lua_pushcfunction(L, lua_json_object_sort);
			return 1;
	}
	else if(strcmp("info", elm->key) == 0) {
			lua_pushcfunction(L, lua_json_elm_info);
			return 1;
	}
	else if(strcmp("elm->keys", elm->key) == 0) {
			lua_pushcfunction(L, lua_json_object_keys);
			return 1;
	}
	else if(strcmp("len", elm->key) == 0) {
			lua_pushcfunction(L, lua_json_elm_len);
			return 1;
	}
	else if(strcmp("rlen", elm->key) == 0) {
			lua_pushcfunction(L, lua_json_elm_get_rlen);
			return 1;
	}
	else if(strcmp("ref", elm->key) == 0) {
				lua_pushcfunction(L, lua_json_object_ref);
			return 1;
		}
	else if(strcmp("unref", elm->key) == 0) {
			lua_pushcfunction(L, lua_json_object_unref);
		return 1;
	}

	lua_getfenv(L, 1);
	// stack { udata, key, env }
	lua_insert(L, 2);
	// stack { udata, env, key }
	lua_rawget(L, 2);
	elm->key = "nil";
	//elm->idx = 1;

    return 1;
};

static int
lua_json_object_inline_args(lua_State *L, int nargs)
{
	// stack { elm, ... }
	if ((nargs % 2) == 0) {
		while (lua_gettop(L) > 1) {
			if (!lua_isnil(L, 2) && !lua_isnil(L, 3)) {
				lua_pushvalue(L, 2);
				lua_pushvalue(L, 3);
				// stack {elm, ... key-1, val-1 }
				lua_pushvalue(L, 1);
				lua_insert(L, -3);
				// stack {elm, ... elm, key-1, val-1 }
				lua_remove(L, 3); // rem key-1 original
				lua_remove(L, 2); // remove val-1 original
				lua_settable(L, 1);
				lua_pop(L, 1);
			}
			else {
				fprintf(stderr, "Invalid Key !!, aborting create object\n");
				lua_pushnil(L);
				return 1;
			}
		}
	}

	return 1;
};

static int
lua_json_object_init(lua_State *L, int nargs)
{
	if (lua_istable(L, 2)) {
		lua_remove(L, 2);
		lua_remove(L, 2);
		nargs -= 2;
	}
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
	luaL_unref(L, LUA_REGISTRYINDEX, self->vtable);

	self->event->cleanup(self->event->on_change);
    	self->event->cleanup(self->event->on_newindex);

	free(self->event->on_change);
	free(self->event->on_newindex);

	free(self->event);
	//free(self->env);

	return 0;
}



static int
lua_json_object_new(lua_State *L, bool parse)
{
	int nargs = lua_gettop(L);
	json_elm *elm = (json_elm *)lua_newuserdata(L, sizeof(json_elm));
	memset(elm, 0, sizeof(json_elm));

	
	alloc_events(elm);
	
	elm->isRoot = true;
	elm->id = (uintptr_t)lua_topointer(L, -1);
	elm->type = JSON_OBJECT_TYPE;
	elm->typename = "object";
	elm->is_nil = false;
	// create env table
	lua_newtable(L);

	// set elm env
	lua_setfenv(L, -2);
	// elm metattable
	luaL_getmetatable(L, "JSON.object");
	lua_setmetatable(L, -2);

	// c side methods
	elm->tostring 		= &lua_json_elm_tostring;
	//elm->tojson 		= &lua_json_elm_tojson;
	elm->stringify 		= &lua_json_elm_stringify;
	elm->parse 		= &lua_json_elm_parse;
	elm->render 		= &lua_json_render_object;
	elm->idx_to_key 	= &idx_to_key;
	elm->key_to_idx 	= &key_to_idx;
	elm->rlen 		= 2;

	lua_newtable(L);
	lua_newtable(L);
	lua_setfield(L, -2, "ids");
	lua_newtable(L);
	lua_setfield(L, -2, "keys");
	// create children table (child elm pointers)
	lua_newtable(L);
	lua_setfield(L, -2, "children");
	elm->vtable = luaL_ref(L, LUA_REGISTRYINDEX);

	lua_rawgeti(L, LUA_REGISTRYINDEX, elm->vtable);
	lua_getfield(L, -1, "ids");
	lua_pushinteger(L, elm->vtable);
	lua_rawseti(L, -2, 0);
	lua_pop(L, 2);
	
	printf("Object Vtable Id: %d\n", elm->vtable);

	if (!parse && nargs > 0)
	{
		lua_insert(L, 1);
		lua_json_object_init(L, nargs);
	}
	printf("NEW OBJECT SIZE: %ld\n", sizeof(json_elm));
	dumpstack(L);

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

static const struct
luaL_reg lua_json_object_lib_f[] = {
	{"foreach",	lua_json_object_foreach },
	{"stringify",	lua_json_elm_stringify	},
	{"tolua",	lua_json_tolua		},
	{"info", 	lua_json_elm_info	},
	{ "len",	lua_json_elm_len	},
	{ "get_rlen",	lua_json_elm_get_rlen	},
	{"__len", 	lua_json_elm_size	},
	{"__tostring", 	lua_json_elm_tostring	},
	{"__index", 	lua_json_object_index	},
	{"__newindex", 	lua_json_object_newindex},
	{"__gc", 	lua_json_object_gc	},
	{NULL, NULL}
};

static const luaL_reg 
lua_json_object_lib_m[] = {
	{"foreach",	lua_json_object_foreach },
	{"stringify",	lua_json_elm_stringify	},
	{"tolua",	lua_json_tolua		},
	{"info", 	lua_json_elm_info	},
	{ "len",	lua_json_elm_len	},
	{ "get_rlen",	lua_json_elm_get_rlen	},
	{"__len", 	lua_json_elm_size	},
	{"__tostring", 	lua_json_elm_tostring	},
	{"__index", 	lua_json_object_index	},
	{"__newindex", 	lua_json_object_newindex},
	{"__gc", 	lua_json_object_gc	},
	{NULL, NULL}
};

void 
lua_json_open_object(lua_State *L) {
	lua_newtable(L);
	lua_newtable(L);
	lua_pushstring(L, "__call");
	lua_pushcfunction(L, lua_json_object);
	lua_settable(L, -3);

	lua_setmetatable(L, -2);
	luaL_register(L, NULL, lua_json_object_lib_m);
	lua_setfield(L, -2, "object");
	// mg_mgr
    
	luaL_newmetatable(L, "JSON.object");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);  /* pushes the metatable */
	lua_settable(L, -3);  /* metatable.__index = metatable */
	luaL_openlib(L, NULL, lua_json_object_lib_m, 0);
	luaL_openlib(L, "json_object", lua_json_object_lib_f, 0);
	lua_pop(L, 2);

};