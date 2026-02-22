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
}

static void lua_json_array_on_newindex(void* ctx, size_t rlen, size_t quoted, size_t nkeys) {
	json_elm *self = (json_elm*)ctx;
	self->rlen += rlen;
	self->quoted += quoted;
	self->nkeys += nkeys;
	printf("%s %p rlen updated: %ld quoted updated: %ld nkeys updated: %ld\n", self->typename, self, self->rlen, self->quoted, self->nkeys);
	//self = NULL;

	return;
};

static void lua_json_array_on_change(void* ctx, size_t rlen, size_t quoted, size_t nkeys)  {
	json_elm *self = (json_elm*)ctx;
	self->rlen += rlen;
	self->quoted += quoted;
	self->nkeys += nkeys;

	printf("%s %p rlen updated: %ld quoted updated: %ld nkeys updated: %ld\n", self->typename, self, self->rlen, self->quoted, self->nkeys);
	//self = NULL;

	return;
};

static int lua_json_get_array_val_length(lua_State *L, json_elm *elm) {
	size_t rlen = 0, quoted = 0, nkeys = 0;
	int vtype = lua_type(L, -1), rets = 1;

	switch(vtype) {
		case LUA_TUSERDATA:
		{
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
		case LUA_TNUMBER:
		 {
			lua_pushvalue(L, -1);
			elm->val = lua_tolstring(L, -1, &elm->vlen);
			rlen += (elm->vlen + 1);
			lua_pop(L, 1);
			break;
		}
		case LUA_TBOOLEAN: 
		{
			//lua_pushvalue(L, -1);
			elm->vlen = lua_toboolean(L, -1) == 0 ? 5 : 4;
			rlen += (elm->vlen + 1);
			//lua_pop(L, 1);
			break;
		}
		case LUA_TSTRING:
		{
			elm->val = lua_tolstring(L, -1, &elm->vlen);
			// handle null sentinel here
			if (elm->vlen == 4 && strcmp(elm->val, "null") == 0) {
				elm->vtype = LUA_TNULL;
				rlen += (elm->vlen + 1);
			}
			else 
				rlen += (elm->vlen + 3);

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

static int lua_json_array_tojson(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	elm->escape = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : false;
	elm->mode = MARSHAL_JSON;
	lua_settop(L, 1);

	elm->stringify(L);

	return 1;
};

static int lua_json_array_tolua(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	elm->escape = lua_isboolean(L, 2) ? lua_toboolean(L, 2) : false;
	elm->mode = MARSHAL_LUA;
	lua_settop(L, 1);

	elm->stringify(L);

	return 1;
};

int lua_json_array_insert(lua_State *L) {
	json_elm *elm = check_json_elm(L, -3);
	size_t prev_rlen = elm->rlen, prev_quoted = elm->quoted, prev_nkeys = elm->nkeys;
	// stack { elm, idx, val }
	lua_getfenv(L, -3);
	// stack { elm, idx, val, env }
	lua_insert(L, -3);
	// stack { elm, env, idx, val }
	int pos = (luaL_checkinteger(L, -2) + 1);
	// check pos is inbounds
	if(pos < 1 || pos > (int)elm->nelms+1) {
		fprintf(stderr, "Insert Index %d is out of range\n", pos);

		return 0;
	}
	//int vtype = lua_type(L, -1);
	lua_pushvalue(L, -3);
	// stack { elm, env, idx, val, env}
	for (int i = (int)elm->nelms; i >= pos; i--) {
		lua_rawgeti(L, -1, i);     				// Get element at i
		// stack { elm, idx, val, env, val[i] }
		lua_rawseti(L, -2, i + 1); 				// Move it to i + 1
		// stack { elm, idx, val, env }
	}

	lua_pop(L, 1);
	// stack { elm, env, idx, val }

	if(lua_type(L, -1) == LUA_TUSERDATA) {
		printf("Un-Sub Array Del ..\n");
		elm->event->sub(elm->nested->event->on_newindex, elm, lua_json_array_on_newindex);
		elm->event->sub(elm->nested->event->on_change, elm, lua_json_array_on_change);
	}

	lua_rawseti(L, -3, pos);
	// stack { elm, env }
	elm->nelms++;
	lua_pop(L, 1);
	// stack { elm }
	lua_pushnumber(L, elm->nelms);
	// stack { elm, nelms }
	elm->event->set(elm->event->on_change, (elm->rlen - prev_rlen), (elm->quoted - prev_quoted), (elm->nkeys - prev_nkeys));

	return 1;
};

static int lua_json_elm_array_del(lua_State *L) {
	int nargs = lua_gettop(L);
	json_elm *elm = check_json_elm(L, -nargs);
	size_t prev_rlen = elm->rlen, prev_quoted = elm->quoted, prev_nkeys = elm->nkeys;
	// stack { elm, pos }
	int idx = (luaL_checkinteger(L, -1) + 1);
	lua_pop(L, 1);
	// stack { elm }
	lua_getfenv(L, -(nargs-1));
	// stack { elm, env }
	lua_rawgeti(L, -1, idx);
	// stack { elm, env, val }
	if(!elm->is_nil) {
		lua_json_get_array_val_length(L, elm);
		// stack { elm, env, val, vlen }
		elm->rlen -= luaL_checknumber(L, -1);
		lua_pop(L, 1);
		// stack { elm, env, val }
		if(elm->vtype == LUA_TUSERDATA) {
				elm->quoted -= (size_t)luaL_checknumber(L, -2);
				elm->nkeys -= (size_t)luaL_checknumber(L, -1);
				lua_pop(L, 2);
		}

		if(elm->vtype == LUA_TSTRING)
			elm->quoted--;
	}
	//printf("fire 3 !!\n");
	elm->event->set(elm->event->on_change, (elm->rlen - prev_rlen), (elm->quoted - prev_quoted), (elm->nkeys - prev_nkeys));	

	//lua_pop(L, 1);

	if(lua_type(L, -1) == LUA_TUSERDATA) {
		printf("Un-Sub Array Del ..\n");
		elm->event->unsub(elm->nested->event->on_newindex, elm, lua_json_array_on_newindex);
		elm->event->unsub(elm->nested->event->on_change, elm, lua_json_array_on_change);
	}

	elm->is_nil = false;

	lua_insert(L, -2);

	// stack { elm, val, env }
	for (int i = idx; i <= (int)elm->nelms; i++) {
		lua_rawgeti(L, -1, i + 1); 			// Get element at i + 1
		// stack { elm, val, env, val[i + 1] }
		lua_rawseti(L, -2, i); 	   			// Move it to i 
		// stack { elm, val, env }
	}
	lua_pop(L, 1);
	elm->nelms--;

	// stack { elm, val }
	lua_pushnumber(L, elm->nelms);
	// stack { elm, val, nelms }
	//elm->event->set(elm->event->on_change, (elm->rlen - prev_rlen), (elm->quoted - prev_quoted), (elm->nkeys - prev_nkeys));

	return 1;
};

static int lua_json_elm_array_reverse(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	// stack { elm }
	lua_getfenv(L, 1);
	// stack { elm, env }
	int start = 1, end = (int)elm->nelms;
	for (int s = start, e = end; s < e; s++, e--) {
		lua_rawgeti(L, 2, s);     // Get element at 0
		// stack { elm, env, val0 }
		lua_rawgeti(L, 2, e);  // Get element at end
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

static int lua_json_elm_array_push(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	size_t prev_rlen = elm->rlen, prev_quoted = elm->quoted, prev_nkeys = elm->nkeys;
	// stack { elm, val }
	lua_getfenv(L, 1);
	// stack { elm, val, env }
	lua_insert(L, -2);
	// stack { elm, env, val }
	int pos = (int)(++elm->nelms);
	lua_json_get_array_val_length(L, elm);
	// stack { elm, env, val, vlen }
	elm->rlen += luaL_checknumber(L, -1);
	lua_pop(L, 1);
	
	if(elm->vtype == LUA_TUSERDATA) {
		elm->quoted += (size_t)luaL_checknumber(L, -2);
		elm->nkeys += (size_t)luaL_checknumber(L, -1);
		lua_pop(L, 2);
	}

	if(elm->vtype == LUA_TSTRING)
		elm->quoted++;

	if(lua_type(L, -1) == LUA_TUSERDATA) {
		elm->event->sub(elm->nested->event->on_newindex, elm, lua_json_array_on_newindex);
		elm->event->sub(elm->nested->event->on_change, elm, lua_json_array_on_change);
	}
	
	// stack { elm, env, val }
	lua_rawseti(L, -2, pos);
	// stack { elm, env }
	lua_pop(L, 1);
	// stack { elm }
	lua_pushnumber(L, elm->nelms);
	// stack { elm, nelms }
	elm->event->set(elm->event->on_change, (elm->rlen - prev_rlen), (elm->quoted - prev_quoted), (elm->nkeys - prev_nkeys));

	return 1;
};

int lua_json_elm_array_pop(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	// stack { elm }
	int idx = (int)elm->nelms;
	lua_pushinteger(L, idx-1);
	// stack { elm, pos};
	lua_json_elm_array_del(L);
	// stack { elm, val, nelms};
	lua_pop(L, 1);
	// stack { elm, val }
	return 1;
};

static int lua_json_elm_array_shift(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	if(!elm) return 1;
	lua_getfenv(L, 1);
	// stack { elm, env}
	lua_rawgeti(L, -1, 1);
	//stack {elm, env, val}
	lua_remove(L, -2);
	//stack {elm, val }
	lua_pushinteger(L, 0);
	// stack { val, tbl, pos}
	lua_json_elm_array_del(L);
	// stack { val , nelms }
	lua_pop(L, 1);
	// stack { val }
	return 1;
};

static int _lua_json_elm_array_unshift(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	// stack { elm, val }
	size_t prev_rlen = elm->rlen, prev_quoted = elm->quoted, prev_nkeys = elm->nkeys;
	lua_json_get_array_val_length(L, elm);
	// stack { elm, val, vlen }
	elm->rlen += luaL_checknumber(L, -1);
	lua_pop(L, 1);

	if(elm->vtype == LUA_TUSERDATA) {
		elm->quoted += (size_t)luaL_checknumber(L, -2);
		elm->nkeys += (size_t)luaL_checknumber(L, -1);
		lua_pop(L, 2);
	}

	if(elm->vtype == LUA_TSTRING)
		elm->quoted++;

	// stack { elm, val }
	lua_pushinteger(L, 0);
	// stack { elm, val, pos }
	lua_insert(L, -2);
	// stack { elm, pos, val }
	lua_json_array_insert(L);
	lua_pop(L, 1);
	// stack { elm }
	elm->event->set(elm->event->on_change, (elm->rlen - prev_rlen), (elm->quoted - prev_quoted), (elm->nkeys - prev_nkeys));

	return 1;
};

static int lua_json_elm_array_unshift(lua_State *L) {
	json_elm * elm = check_json_elm(L, 1);
	// stack { elm ,... }
	int nargs = lua_gettop(L);
	for(int i = 1; i < nargs;) {
		lua_pushvalue(L, 1);
		// stack { elm ,... elm }
		lua_pushvalue(L, nargs);
		// stack { elm ,... elm, val[nargs] }
		_lua_json_elm_array_unshift(L);
		// stack { elm ,... elm }
		lua_remove(L, nargs--);
		// stack { elm ,.. elm }
		lua_pop(L, 1);
		// stack { elm ,.. }
	}

	lua_pushnumber(L, elm->nelms);
	// stack { elm, rlen }
	return 1;
};

static int lua_json_array_ref(lua_State *L) {
	check_json_elm(L, 1);
	// stack { elm }
	return 1;
};

static int _lua_json_array_unref(lua_State *L) {
	json_elm *elm = check_json_elm(L, -1);
	size_t len = 0;

	elm->stringify(L);
	const char *json = lua_tolstring(L, -1, &len);
	lua_pop(L, 1);

	lua_pushlstring(L, json, len);
	lua_json_elm_parse(L);

	return 1;
};

static int lua_json_render_array(lua_State *L, struct ref *seen) {
	json_elm *elm = check_json_elm(L, -1);
	seen->elm = elm;
	lua_getfenv(L, -1);

	seen->mode == MARSHAL_JSON ? seen->marshal->arr_open(seen) : seen->marshal->obj_open(seen);

	for(size_t i = 1; i <= elm->nelms; i++) {
		if(lua_istable(L, -1))
			lua_rawgeti(L, -1, i);

		switch(lua_type(L, -1)) {
			case LUA_TUSERDATA: {
				seen->nested = check_json_elm(L, -1);
				uintptr_t next = (uintptr_t)lua_topointer(L, -1);
				if(seen->check_next(seen, next)) {
					lua_pop(L, 1);
					size_t len = strlen(seen->b);
					seen->b[len-1] = 0;
					break; 
				}

				json_elm *nested = check_json_elm(L, -1);
				nested->render(L, seen);
				lua_pop(L, 2);
				break;
			}

			case LUA_TSTRING: {
				seen->marshal->arr_string(L, elm, seen);
				lua_pop(L, 1);
				break;
			}

			case LUA_TNUMBER: {
				seen->marshal->arr_number(L, seen);
				lua_pop(L, 1);
				break;
			}

			case LUA_TBOOLEAN: {
				seen->marshal->arr_bool(L, seen);
				lua_pop(L, 1);
				break;
			}
		}

		if(elm->nelms - i >= 1) seen->marshal->next(seen);
	}

	seen->mode == MARSHAL_JSON ? seen->marshal->arr_close(seen) : seen->marshal->obj_close(seen);

	if(lua_gettop(L) == 1)
		return 1;
	// recursion
	return 0;
};

static int lua_json_array_newindex(lua_State *L) {
	json_elm *elm = check_json_elm(L, -3);
	// stack {..., elm, key, val }
	int vtype = lua_type(L, -1);
	bool is_new = false;
	size_t prev_rlen = elm->rlen, prev_quoted = elm->quoted, prev_nkeys = elm->nkeys;

	if(vtype == LUA_TUSERDATA) {
		json_elm *nested = check_json_elm(L, -1);
		if(nested->vtable == elm->vtable) {
			fprintf(stderr, "Error: recursive element: elm id[%d] --> root id[%d] !!\n \
						element must be Unreferenced first !!\n", nested->vtable, elm->vtable);
			return 0;
		}
	}

	elm->idx = (luaL_checkinteger(L, -2) + 1);
	// check idx is inbounds
	if(elm->idx < 1 || elm->idx > (int)elm->nelms+1) {
		fprintf(stderr, "Index %d is out of range\n", elm->idx);

		return 0;
	}
	// add new element
	if(elm->idx > (int)elm->nelms) {
		lua_json_get_array_val_length(L, elm);
		// stack {..., elm, key, val, vlen }
		elm->rlen += lua_tonumber(L, -1);
		lua_pop(L, 1);
		
		if(elm->vtype == LUA_TUSERDATA) {
			elm->quoted += (size_t)luaL_checknumber(L, -2);
			elm->nkeys += (size_t)luaL_checknumber(L, -1);
			lua_pop(L, 2);

			json_elm *nested = check_json_elm(L, -1);
			elm->event->sub(nested->event->on_newindex, elm, lua_json_array_on_newindex);
			elm->event->sub(nested->event->on_change, elm, lua_json_array_on_change);

			env_val val = {0};
			val.num = nested->vtable;

			lua_json_elm_env_add(L, elm, &val, ids);

		}

		if(elm->vtype == LUA_TSTRING)
			elm->quoted++;

		is_new = true;

	}
	// update existing element
	else if(elm->idx <= (int)elm->nelms) { 
		// stack {..., elm, key, val }
		lua_getfenv(L, -3);
		// stack {..., elm, key, val, env }
		lua_rawgeti(L, -1, elm->idx);
		// stack {..., elm, key, val, env, exval }
		lua_json_get_array_val_length(L, elm);
		// stack {..., elm, key, val, env, exval, exlen}
		elm->rlen -= luaL_checknumber(L, -1);
		lua_pop(L, 1);
		// stack {..., elm, key, val, env, exval } || ..., exval, quoted, nkeys }
		if(elm->vtype == LUA_TUSERDATA) {
			elm->quoted -= (size_t)luaL_checknumber(L, -2);
			elm->nkeys -= (size_t)luaL_checknumber(L, -1);
			lua_pop(L, 2);
			// stack {..., elm, key, val, env, exval }
		}

		if(elm->vtype == LUA_TSTRING )
			elm->quoted--;
		
		lua_pop(L, 2);
	}
	
	if(!is_new && vtype != LUA_TNIL && elm->idx <= (int)elm->nelms) {
		// stack {..., elm, key, val }
		lua_json_get_array_val_length(L, elm);
		// stack {..., elm, key, val, vlen }
		elm->rlen += luaL_checknumber(L, -1);
		lua_pop(L, 1);
		
		if(elm->vtype == LUA_TUSERDATA) {
			elm->quoted += (size_t)luaL_checknumber(L, -2);
			elm->nkeys += (size_t)luaL_checknumber(L, -1);
			lua_pop(L, 2);
		}

		if(elm->vtype == LUA_TSTRING )
			elm->quoted++;

		printf("fire 1!!\n");
		elm->event->set(elm->event->on_change, (elm->rlen - prev_rlen), (elm->quoted - prev_quoted), (elm->nkeys - prev_nkeys));
		
	}

	// remove an element
	if(vtype == LUA_TNIL) {
		lua_pop(L, 1);
		elm->is_nil = true;
		lua_json_elm_array_del(L);
		printf("fire 2 !!\n");
		elm->event->set(elm->event->on_change, (elm->rlen - prev_rlen), (elm->quoted - prev_quoted), (elm->nkeys - prev_nkeys));

		return 0;
	}
	
	// stack {..., elm, key, val }
	lua_getfenv(L, -3);
	// stack {..., elm, key, val, env}
	lua_insert(L, -3);
	//lua_settop(L, 4);
	// stack { elm, env, key, val }
	lua_pushinteger(L, elm->idx);
	// stack {..., elm, env, key, val, idx }
	lua_insert(L, -2);
	// stack {..., elm, env, key, idx, val }
	lua_remove(L, -3);
	// stack {..., elm, env, idx, val }
	lua_rawset(L, -3); 
	// stack {..., elm, env }
	if(elm && (elm->idx > (int)elm->nelms)) {
		//elm->event->set_value(elm->event->on_newindex, (elm->rlen - prev_rlen));
		elm->event->set(elm->event->on_newindex, (elm->rlen - prev_rlen), (elm->quoted - prev_quoted), (elm->nkeys - prev_nkeys));
		elm->nelms++;
	}

    return 0;
};

static int lua_json_array_index(lua_State *L) {
	json_elm *elm = check_json_elm(L, 1);
	if(!elm) return 0;

	lua_getfenv(L, 1);
	// stack { elm, key, env}
	if(!lua_isnumber(L, -1)) {
		const char *key = lua_tostring(L, 2);
		if(strcmp("env", key) == 0) {
			lua_getfenv(L, 1);
			return 1;
		}
		else if(strcmp("tojson", key) == 0) {
			lua_pushcfunction(L, lua_json_array_tojson);
			return 1;
		}
		else if(strcmp("tolua", key) == 0) {
			//lua_pushcfunction(L, lua_json_tolua);
			lua_pushcfunction(L, lua_json_array_tolua);
			return 1;
		}
		else if(strcmp("info", key) == 0) {
				lua_pushcfunction(L, lua_json_elm_info);
			return 1;
		}
		else if(strcmp("rlen", key) == 0) {
			lua_pushcfunction(L, lua_json_elm_get_rlen);
			return 1;
		}
		else if(strcmp("len", key) == 0) {
			lua_pushcfunction(L, lua_json_elm_len);
			return 1;
		}
		else if(strcmp("insert", key) == 0) {
				lua_pushcfunction(L, lua_json_array_insert);
			return 1;
		}
		else if(strcmp("pop", key) == 0) {
				lua_pushcfunction(L, lua_json_elm_array_pop);
			return 1;
		}
		else if(strcmp("push", key) == 0) {
				lua_pushcfunction(L, lua_json_elm_array_push);
			return 1;
		}
		else if(strcmp("shift", key) == 0) {
				lua_pushcfunction(L, lua_json_elm_array_shift);
			return 1;
		}
		else if(strcmp("unshift", key) == 0) {
				lua_pushcfunction(L, lua_json_elm_array_unshift);
			return 1;
		}
		else if(strcmp("reverse", key) == 0) {
				lua_pushcfunction(L, lua_json_elm_array_reverse);
			return 1;
		}
		else if(strcmp("del", key) == 0) {
				lua_pushcfunction(L, lua_json_elm_array_del);
			return 1;
		}
		else if(strcmp("ref", key) == 0) {
				lua_pushcfunction(L, lua_json_array_ref);
			return 1;
		}
		else if(strcmp("unref", key) == 0) {
				lua_pushcfunction(L, _lua_json_array_unref);
			return 1;
		}
	}

	lua_getfenv(L, 1);
	// stack { elm, key, env }
	lua_insert(L, 2);
	// stack { elm, env, key }
	lua_rawgeti(L, 2, (lua_tointeger(L, 3) + 1));
	// stack { elm, env, val }
	lua_remove(L, -2);
	// stack { elm, val }
    return 1;
};

static int lua_json_array_inline_args(lua_State *L, int nargs) {
    // stack { elm, ...,   }
    if(nargs > 0) {
        for (int i = 0; i < nargs; i++) {
            // stack { elm, ... }
            lua_pushinteger(L, i);
            // stack { elm, ..., i }
			if(!lua_isnil(L, 2)) {
				lua_pushvalue(L, 2);
				lua_remove(L, 2);
				// stack { elm, .., i, val[1] }
				lua_pushvalue(L, 1);
				lua_insert(L, -3);
				// stack { elm, .., elm, i, val[1] }
				lua_settable(L, -3);
				// stack { elm, .., elm}
				lua_pop(L, 1);
			}
			else {
				fprintf(stderr, "Invalid Entry at Idnex: %d, aborting create array !!\n", i);
				lua_pushnil(L);
				return 1;
			}
        }
    }

	return 1;
};

static int lua_json_array_init(lua_State *L, int nargs) {
	// stack { elm, args }
     if(lua_istable(L, 2)) {
            lua_remove(L, 2);
            lua_remove(L, 2);
            nargs -= 2;
     }
	if((lua_gettop(L) - 1) > 0 ) {
		int args = lua_type(L, -1);
       
		// get args type
		if(args == LUA_TTABLE)
			lua_json_lua_parse(L);
		else
		// stack { elm, args }
			lua_json_array_inline_args(L, nargs);

		return 1;
	}

	return 0;
};

static int lua_json_array_gc(lua_State *L) {
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

static int lua_json_array_new(lua_State *L, bool parse) {
	int nargs = lua_gettop(L);
	json_elm *elm = (json_elm*)lua_newuserdata(L, sizeof(json_elm));
	memset(elm, 0, sizeof(json_elm));

	alloc_events(elm);

	elm->id = (uintptr_t)lua_topointer(L, -1);
	elm->type = JSON_ARRAY_TYPE;
	elm->typename = "array";
	elm->is_nil = false;
	elm->root = elm;
    	// create env table
    	lua_newtable(L);
    	// create its metatable
	//elm->marshal = lua_json_marshall_new(L);
	//elm->marshal->escape = false;
    	lua_setfenv(L, -2);
    	// elm metattable
    	luaL_getmetatable(L, "JSON.array");
	lua_setmetatable(L, -2);
	// c side methods

	elm->tostring 		= &lua_json_elm_tostring;
	elm->stringify		= &lua_json_elm_stringify;
	elm->parse 		= &lua_json_elm_parse;
	elm->render 		= &lua_json_render_array;
	elm->rlen 		= 2;

	lua_newtable(L);
	lua_newtable(L);
	lua_setfield(L, -2, "ids");
	elm->vtable = luaL_ref(L, LUA_REGISTRYINDEX);

	lua_rawgeti(L, LUA_REGISTRYINDEX, elm->vtable);
	lua_getfield(L, -1, "ids");
	lua_pushinteger(L, elm->vtable);
	lua_rawseti(L, -2, 0);
	lua_pop(L, 2);
	printf("Array Vtable Id: %d\n", elm->vtable);

	if(!parse && nargs > 0) {
		lua_insert(L, 1);
		lua_json_array_init(L, nargs);
	}
	printf("NEW ARRAY SIZE: %ld\n", sizeof(json_elm));
	return 1;
};

int lua_json_array(lua_State *L) {
	lua_json_array_new(L, false);

	return 1;
};

int lua_json_elm_parse_array(lua_State *L) {
	lua_json_array_new(L,true);

	return 1;
};

static const struct luaL_reg json_array_lib_f [] = {
	{"tolua",		lua_json_tolua				},
	{"info", 		lua_json_elm_info			},
	{ "len",		lua_json_elm_len			},
	{"__len", 		lua_json_elm_size			},
	{"__tostring",	lua_json_elm_tostring		},
	{"__index", 	lua_json_array_index		},
	{"__newindex", 	lua_json_array_newindex		},
	{"__gc", 		lua_json_array_gc			},
	{NULL, NULL}
};

static const struct luaL_reg json_array_lib_m [] = {
	{"tolua",		lua_json_tolua				},
	{"info", 		lua_json_elm_info			},
	{ "len",		lua_json_elm_len			},
	{"__len", 		lua_json_elm_size			},
    {"__tostring",	lua_json_elm_tostring	    },
	{"__index", 	lua_json_array_index		},
	{"__newindex", 	lua_json_array_newindex		},
	{"__gc", 		lua_json_array_gc			},
	{NULL, NULL}
};

void lua_json_open_array(lua_State *L) {
	lua_newtable(L);
    lua_newtable(L);
    lua_pushstring(L, "__call");
    lua_pushcfunction(L, lua_json_array);
    lua_settable(L, -3);
    lua_setmetatable(L, -2);
	luaL_register(L, NULL, json_array_lib_m);
	lua_setfield(L, -2, "array");
	// mg_mgr
	luaL_newmetatable(L, "JSON.array");
	lua_pushstring(L, "__index");
	lua_pushvalue(L, -2);  /* pushes the metatable */
	lua_settable(L, -3);  /* metatable.__index = metatable */

	luaL_openlib(L, NULL, json_array_lib_m, 0);
	luaL_openlib(L, "json_array", json_array_lib_f, 0);
	lua_pop(L, 2);
};