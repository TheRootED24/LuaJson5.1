#include "luajson_api.h"
#include "lua_json.h" // Injected project headers
#include "lua_json_int64.h"
#include <stdlib.h>
#include <stdarg.h>

/* ==============================================================================
 * 1. STRUCT DEFINITIONS & PROTOTYPES
 * ============================================================================== */


struct luajson_node {
	int ref_id;
};

/* External declarations matching your core v1.1.1 engine module open entry */
LUALIB_API int luaopen_JSON(lua_State *L);


/*stack_state_t* luajson_store_stack(lua_State *L) {
	uint16_t size = lua_gettop(L);
	int types[size];
	int refs[size];

	for(int i = 0; i < size; i++) {
		types[i] = lua_type(L, i);
		refs[i] = luaL_ref(L, LUA_REGISTRYINDEX);
	}

	stack_state_t *ss = (stack_state_t*)malloc(sizeof(stack_state_t));
	ss->refs = refs;
	ss->types = types;

	return ss;
};*/

/* ==============================================================================
 * 2. VARIADIC REVERSE-DUMPSTACK ENGINE (The Data Ingestion Utility)
 * ============================================================================== */
static int push_args(lua_State *L, const char *fmt, va_list args) {
	int count = 0;
	while (*fmt) {
		switch (*fmt++) {
			case 's': { /* String */
				const char *s = va_arg(args, const char*);
				lua_pushstring(L, s);
				count++;
				break;
			}
			case 'n': { /* Number (Double) */
				double d = va_arg(args, double);
				lua_pushnumber(L, d);
				count++;
				break;
			}
			case 'd': { /* Standard 32-bit Integer Ingestion Track */
				int val32 = va_arg(args, int); // Safely extracts exactly 4 bytes
				
				/* FIX: Push directly as a native number! Bypasses userdata boxing
				 * completely for lightweight, standard 32-bit integer parameters. */
				lua_pushinteger(L, (lua_Number)val32); 
				count++;
				break;
			}
			case 'i': { /* Explicit 64-bit Long Long Ingestion Track */
				int64_t val64 = va_arg(args, int64_t); // Safely extracts exactly 8 bytes
				lua_pushint64(L, val64); // Pushes the full high-resolution capsule
				count++;
				break;
			}
			case 'b': { /* Boolean */
				bool b = va_arg(args, int); // Booleans are passed as ints in variadic parameters
				lua_pushboolean(L, b);
				count++;
				break;
			}
			case 'e': { /* Nested Opaque Node Handle */
				luajson_node_t *node = va_arg(args, luajson_node_t*);
				/* Fetch the hidden underlying table/userdata reference out of the registry */
				lua_rawgeti(L, LUA_REGISTRYINDEX, node->ref_id);
				count++;
				break;
			}
			default:
				break;
		}
	}
	return count;
}

/*
static luajson_types pop_value(lua_State *L, luajson_value_t *val) {
	switch(val->type)
	{
		case LUA_TSTRING: {
			val->s = lua_tolstring(L, -1, &val->size);
			break;
		};

		case LUA_TNUMBER: { 
			if(val->type == L_INT)
				val->i32 = (int32_t)lua_tointeger(L, -1);
			else 
				val->n = lua_tonumber(L, -1);
			
			lua_pushvalue(L, -1);
			lua_tolstring(L, -1, &val->size);
			break;
		};

		case LUA_TBOOLEAN: {
			val->b = lua_toboolean(L, -1);
			val->size = sizeof(bool);
			break;
		};

		case LUA_TUSERDATA: {
			// Unpack your custom 64-bit container unconditionally /
			if (lua_json_is_int64(L, -1)) {
				printf("oohh yaaha i64\n");
				json_int64 *ud = (json_int64 *)lua_touserdata(L, -1);
				val->i64 = ud->value;
				val->size = ud->length(L);
				val->type = L_INT64;
				break;
			}
			
			// Fallthrough to check if it's a native structural nested json_elm /
			if(lua_json_is_elm(L, -1)) {
				json_elm *nested = check_json_elm(L, -1, false);
				val->nested = (luajson_node_t * )nested->env_id;
				val->size = nested->base->rlen;
				val->type = L_NESTED;

			}

			break;
		};

		default:
			break;
	}

	return 0;
}*/
/* ==============================================================================
 * 3. CONTEXT LIFESPAN MANAGEMENT
 * ============================================================================== */
luajson_ctx_t* luajson_init(void) {
	luajson_ctx_t *ctx = malloc(sizeof(struct luajson_ctx));
	if (!ctx) return NULL;

	/* Spin up an isolated sandbox virtual state machine */
	ctx->L = luaL_newstate();
	if (!ctx->L) {
		free(ctx);
		return NULL;
	}
	luaL_openlibs(ctx->L);

	/* Boot your native module engine directly onto the stack */
#if LUA_VERSION_NUM >= 504
	luaL_requiref(ctx->L, "JSON", luaopen_JSON, 1);
#else
	lua_pushcfunction(ctx->L, luaopen_JSON);
	lua_pushstring(ctx->L, "JSON");
	lua_call(ctx->L, 1, 1);
	lua_setglobal(ctx->L, "JSON"); 
#endif
	lua_settop(ctx->L, 0); // Flush stack clean

	return ctx;
}

json_elm *luajson_push_elm(luajson_ctx_t *ctx, luajson_node_t *elm)
{
	lua_State *L = ctx->L;
	lua_rawgeti(L, LUA_REGISTRYINDEX, elm->ref_id); 

	json_elm *e = check_json_elm(L, -1, false);

	return e;
}

void luajson_close(luajson_ctx_t *ctx) {
	if (!ctx) return;
	
	/* Wiping the parent state context automatically drops the global registry,
	 * cleanly vaporizing all nested node handles at once with 0 leaks */
	lua_close(ctx->L);
	free(ctx);
}

int64_t luajson_get_int64(luajson_ctx_t *ctx, luajson_node_t *node) {
	lua_State *L = ctx->L;
	
	/* 1. Fetch the node userdata block back onto the top of the stack */
	lua_rawgeti(L, LUA_REGISTRYINDEX, node->ref_id); // stack: { node_userdata }

#if LUA_VERSION_NUM >= 503
	/* LUA 5.3 / 5.4: Read the native 64-bit integer subtype directly */
	int64_t val = (int64_t)lua_tointeger(L, -1);
#else
	/* LUA 5.1: Safely unpack the raw binary bits straight out of our custom memory box! */
	json_int64 *ud = (json_int64 *)luaL_checkudata(L, -1, "JSON.int64");
	int64_t val = ud->value;
#endif

	lua_pop(L, 1); /* Flush stack clean */
	return val;
}

/* ==============================================================================
 * ARRAY CLASS
 * ============================================================================== */
luajson_node_t* luajson_create_array(luajson_ctx_t *ctx, const char *fmt, ...) {
	// FIX: Remove the 'if (!ctx || !fmt) return NULL;' line entirely!
	lua_State *L = ctx->L;

	lua_pushnil(L);
	lua_pushnil(L);

	va_list args;
	va_start(args, fmt);
	push_args(L, fmt, args);
	va_end(args);                                   

	lua_json_array(L);                              

	luajson_node_t *node = malloc(sizeof(struct luajson_node));
	if (!node) {
		lua_settop(L, 0);
		return NULL;
	}
	
	node->ref_id = luaL_ref(L, LUA_REGISTRYINDEX);
	lua_settop(L, 0);
	return node;
};

bool luajson_array_get(luajson_ctx_t *ctx, luajson_node_t *arr, uint16_t idx, uint16_t type, luajson_value_t *val)
{
	lua_State *L = ctx->L;
	lua_settop(L, 0); // Enforce a predictable, clean base frame

	lua_rawgeti(L, LUA_REGISTRYINDEX, arr->ref_id); // pos 1: parent_elm
	check_json_elm(L, 1, false);

	//get_json_table(L, 1);      // pos 2: env_table
	lua_pushinteger(L, idx);   // pos 3: Lua 1-based index key
	dumpstack(L, "array get");

	lua_gettable(L, 1);            // pos 4: target_value
	dumpstack(L, "array get");
	
	val->size = 0;
	int l_type = lua_type(L, -1);

	/* 2. TRANSLATE DATA AND RECORD SIZES NATIVELY */
	switch(l_type)
	{
		case LUA_TSTRING: 
			val->s = lua_tolstring(L, -1, &val->size);
			break;

		case LUA_TNUMBER: 
			if(type == L_INT)
				val->i32 = (int32_t)lua_tointeger(L, -1);
			else {
				val->n = lua_tonumber(L, -1);
				val->type = L_NUM;
			}
			lua_pushvalue(L, -1);
			lua_tolstring(L, -1, &val->size);
			break;

		case LUA_TBOOLEAN: 
			val->b = lua_toboolean(L, -1);
			val->size = sizeof(bool);
			val->type = L_BOOL;
			break;

		case LUA_TUSERDATA:
        /* Unpack your custom 64-bit container unconditionally */
        if (lua_json_is_int64(L, -1)) {
			printf("oohh yaaha i64\n");
            json_int64 *ud = (json_int64 *)lua_touserdata(L, -1);
            val->i64 = ud->value;
			val->size = ud->length(L);
			val->type = L_INT64;

            break;
        }
        
        /* Fallthrough to check if it's a native structural nested json_elm */
		if(lua_json_is_elm(L, -1)) {
			printf("oohh yess Nested\n");
			json_elm *nested = check_json_elm(L, -1, false);
			val->nested = (luajson_node_t * )nested->env_id;
			val->size = nested->base->rlen;
			val->type = L_NESTED;
		}
        break;

		default:
			break;
	}

	lua_settop(L, 0); // Complete stack flush. Clean and deterministic!
	return (val->size > 0);
}


static int luajson_push_val(luajson_ctx_t *ctx, luajson_value_t *val) 
{
	lua_State *L = ctx->L;
	
	switch(val->type) 
	{
		case L_STR: 
			lua_pushstring(L, val->s);
			break;
		case L_NUM:
			lua_pushnumber(L, val->n);
			break;
		case L_INT:
				lua_pushinteger(L, val->i32);
			break;
		case L_INT64:
				lua_pushint64(L, val->i64);
			break;
		case L_BOOL: 
			lua_pushboolean(L, val->b);
			break;
		case L_NESTED:
			// stack { elm, idx }
			luajson_push_elm(ctx, (luajson_node_t*)val->nested);
			json_elm *nested = check_json_elm(L, -1, false);
			val->size = nested->base->rlen;
			val->type = L_NESTED;
		
			break;
		default:
			break;
	}

	return 1;
}

bool luajson_array_set(luajson_ctx_t *ctx, luajson_node_t *arr, uint16_t idx, uint8_t type, luajson_value_t *val)
{

	//lua_State *L = ctx->L;
	//lua_rawgeti(L, LUA_REGISTRYINDEX, arr->ref_id); 

	//json_elm *elm = check_json_elm(L, -1, false);
	json_elm *elm = luajson_push_elm(ctx, arr);
	lua_State *L = ctx->L;

	elm->idx = idx;
	elm->ops->check_idx(elm);

	//get_json_table(L, -1);
	// stack { elm, env }
	lua_pushnumber(L, idx);
	// stack { env, idx }
	bool ok = false;

	switch(type)
	{
		case L_STR: 
			lua_pushstring(L, val->s);
			break;
		case L_NUM:
			lua_pushnumber(L, val->n);
			break;
		case L_INT:
				lua_pushinteger(L, val->i32);
			break;
		case L_INT64:
				lua_pushint64(L, val->i64);
			break;
		case L_BOOL: 
			lua_pushboolean(L, val->b);
			break;
		case L_NESTED:
			// stack { elm, idx }
			luajson_push_elm(ctx, (luajson_node_t*)val->nested);
			json_elm *nested = check_json_elm(L, -1, false);
			val->size = nested->base->rlen;
			val->type = L_NESTED;
		
			break;
		default:
			break;
	}
	
	if(lua_gettop(L) == 3) {
		dumpstack(L, "que");
		lua_settable(L, -3);
		dumpstack(L, "set");
		ok = true;
	}

	//elm->nelms++;
	lua_settop(L, 0);
	
	return ok;
};

bool luajson_array_pop(luajson_ctx_t *ctx, luajson_node_t *arr, luajson_value_t *val) 
{
	bool ret = val ? true : false;

	lua_State *L = ctx->L;
	lua_rawgeti(L, LUA_REGISTRYINDEX, arr->ref_id);

	check_json_elm(L, -1, false);

	lua_settop(L, 1);
	lua_json_elm_array_pop(L);

	if(ret) {
		uint8_t vtype = lua_type(L, -1);

		switch(vtype)
		{
			case L_STR: 
				val->s = lua_tolstring(L, -1, &val->size);
				break;
			case L_NUM:
			case L_INT:
				if(vtype == L_NUM && val->type != L_INT) // <-- user must specify val.type = L_INT to receive an int in lua 5.1
				// NEED VERSION CHECK if(LUA_VERSOIN >=503 && lua_checkinteger(L, -1)) {
				//val->buf.int32 = lua_tointeger(L, -1);
				// val->type = L_INT;
					val->n = lua_tonumber(L, -1);
				else
					val->i32 = lua_tointeger(L, -1);

				lua_pushvalue(L, -1);
				lua_tolstring(L, -1, &val->size);
				break;
			case L_BOOL: 
				val->b = lua_toboolean(L, -1);
				val->size = sizeof(bool);
				val->type = L_BOOL;
				break;
			case L_NESTED:
			case L_INT64:
				if(lua_json_is_elm(L, -1)) {
					json_elm *nested = check_json_elm(L, -1, false);
					val->nested = (luajson_node_t*)nested->env_id;
					val->size = nested->base->rlen;
					val->type = L_NESTED;
				}

				if(lua_json_is_int64(L, -1)) {
					json_int64 *ud = (json_int64 *)lua_touserdata(L, -1);
					val->i64 = ud->value;
					val->size = ud->length(L);
					val->type = L_INT64;
				}

				break;
			default:
				break;
		}
	}

	lua_settop(L, 0);
	return (ret && val->size > 0);
};

size_t luajson_array_push(luajson_ctx_t *ctx, luajson_node_t *arr, luajson_value_t *val) 
{
	lua_State *L = ctx->L;
	json_elm *e = luajson_push_elm(ctx, arr); //lua_rawgeti(L, LUA_REGISTRYINDEX, arr->ref_id);

	lua_settop(L, 1);
	lua_pushinteger(L, e->nelms);
	luajson_push_val(ctx, val);
	if(lua_isnil(L, -1))
		return (size_t)luaL_error(L, "ERROR: Array: Failed to push vaule type: %s", lua_typename(L, lua_type(L, -1)));

	lua_settable(L, -3);

	lua_settop(L, 0);

	return e->nelms;
};

bool luajson_array_move(luajson_ctx_t *ctx, luajson_node_t *arr, luajson_range_t *r) 
{
	lua_State *L = ctx->L;
	luajson_push_elm(ctx, arr); //lua_rawgeti(L, LUA_REGISTRYINDEX, arr->ref_id);

	lua_settop(L, 1);

	lua_pushinteger(L, r->arr.start);
	lua_pushinteger(L, r->arr.end);

	if(lua_isnil(L, -1))
		return luaL_error(L, "ERROR: Array: Failed to push vaule type: %s", lua_typename(L, lua_type(L, -1)));

	dumpstack(L, "move");
	lua_json_array_move(L);

	lua_settop(L, 0);

	return true;
};

bool luajson_array_reverse(luajson_ctx_t *ctx, luajson_node_t *arr, luajson_range_t *r) 
{
	lua_State *L = ctx->L;
	json_elm *e = luajson_push_elm(ctx, arr);

	lua_settop(L, 1);

	lua_pushinteger(L, (r->arr.start > 0 ? r->arr.start : 0));
	lua_pushinteger(L, (r->arr.end > 0 ? r->arr.end : e->nelms-1));


	if(lua_isnil(L, -1))
		return luaL_error(L, "ERROR: Array: Failed to reverse %s", lua_typename(L, lua_type(L, -1)));

	dumpstack(L, "reverse");
	lua_json_array_reverse(L);

	lua_settop(L, 0);

	return true;
};


/* ==============================================================================
 * OBJECT CLASS
 * ============================================================================== */
luajson_node_t* luajson_create_object(luajson_ctx_t *ctx, const char *fmt, ...) {
	lua_State *L = ctx->L;

	lua_pushnil(L); 
	lua_pushnil(L); 

	va_list args;
	va_start(args, fmt);
	push_args(L, fmt, args);
	va_end(args);                                   

	lua_json_object(L);                             

	luajson_node_t *node = malloc(sizeof(struct luajson_node));
	if (!node) {
		lua_settop(L, 0);
		return NULL;
	}
	
	node->ref_id = luaL_ref(L, LUA_REGISTRYINDEX);   
	lua_settop(L, 0);                               
	return node;
};

bool luajson_object_get(luajson_ctx_t *ctx, luajson_node_t *obj, const char *key, uint16_t type, luajson_value_t *val)
{
	lua_State *L = ctx->L;
	lua_settop(L, 0); // Enforce a predictable baseline state

	json_elm *elm = luajson_push_elm(ctx, obj);
	elm->key = key;
	// stack { elm }
	//lua_pushstring(L, elm->key);   // pos 3: dictionary string key
	if(!elm->ops->key_to_idx(elm, false)) {
		lua_settop(L, 0);
		return false; 
	} else {
		elm->ops->check_idx(elm);
	}

	// stack { elm }
	lua_getfield(L, 1, "arr2");            // pos 4: target_value sitting at absolute index 4
	// stack { elm, val }
	if(lua_isnil(L, -1)) return false;
	
	val->size = 0;
	int l_type = lua_type(L, -1);

	/* 1. TRANSLATE DATA AND RECORD SIZES NATIVELY MATCHING ARRAY SPECS */
	switch(l_type)
	{
		case LUA_TSTRING: 
			val->s = lua_tolstring(L, -1, &val->size);
			break;

		case LUA_TNUMBER: 
			if(type == L_INT) {
				val->i32 = (int32_t)lua_tointeger(L, -1);
			} else {
				val->n = lua_tonumber(L, -1);
			}
			lua_pushvalue(L, -1);
			lua_tolstring(L, -1, &val->size);
			break;

		case LUA_TBOOLEAN: 
			val->b = lua_toboolean(L, -1);
			val->size = sizeof(bool);
			break;

		case LUA_TUSERDATA:
			
			if (lua_json_is_int64(L, -1)) {
				printf("oohh yaaha\n");
				json_int64 *ud = (json_int64 *)lua_touserdata(L, -1);
				val->i64 = ud->value;
				val->size = ud->length(L);
				val->type = L_INT64;

				break;
			}
			
			/* Fallthrough to check if it's a native structural nested json_elm */
			if(lua_json_is_elm(L, -1)) {
					printf("oohh yaaha nest it!!\n");
					json_elm *nested = check_json_elm(L, -1, false);
					printf("NESTED: %p %s\n", (json_elm*)nested, nested->typename);
					val->nested = (luajson_node_t * )nested->env_id;
					val->size = nested->base->rlen;
					val->type = L_NESTED;
			}
			break;

		default:
			break;
	}
	dumpstack(L, "obj get end");
	lua_settop(L, 0); // Complete stack flush. Clean and deterministic!
	return (val->size > 0);
}

bool luajson_object_set(luajson_ctx_t *ctx, luajson_node_t *obj, const char *key, uint8_t type, luajson_value_t *val)
{
	lua_State *L = ctx->L;
	lua_settop(L, 0); // Enforce a predictable baseline stack state

	lua_rawgeti(L, LUA_REGISTRYINDEX, obj->ref_id); // pos 1: parent_elm userdata
	json_elm *elm = check_json_elm(L, -1, false);

	elm->key = key;
	bool ok = elm->ops->key_to_idx(elm, true);
	//dumpstack(L, "obj set");
	//elm->ops->check_idx(elm);

	//get_json_table(L, 1);         // pos 2: env_table
	lua_pushstring(L, elm->key);  // pos 3: String key dictionary index
	dumpstack(L, "obj set");
	switch(type)
	{
		case L_STR: 
			lua_pushstring(L, val->s);
			break;
		case L_NUM:
			lua_pushnumber(L, val->n);
			break;
		case L_INT:
				lua_pushinteger(L, val->i32);
			break;
		case L_INT64:
				lua_pushint64(L, val->i64);
				break;
		case L_BOOL: 
			lua_pushboolean(L, val->b);
			break;
		case L_NESTED:
			// stack { elm, idx }
			luajson_push_elm(ctx, (luajson_node_t*)val->nested);
			json_elm *nested = check_json_elm(L, -1, false);
			val->size = nested->base->rlen;
			val->type = L_NESTED;
			break;
		default:
			lua_settop(L, 0);
			return false;
	}
	
	/* Absolute stack deep validation ensures total stability.
	 * pos 1: parent, pos 2: env_table, pos 3: string_key, pos 4: value_payload */
	if(lua_gettop(L) == 3) {
		dumpstack(L, "obj set end");
		lua_settable(L, 1); // env_table[string_key] = value_payload
		dumpstack(L, "obj set end");
		ok = true;
	}
	
	lua_settop(L, 0);
	return ok;
};

bool luajson_object_pop(luajson_ctx_t *ctx, luajson_node_t *obj, luajson_value_t *val) 
{
	bool ret = val ? true : false;

	lua_State *L = ctx->L;
	json_elm *e = luajson_push_elm(ctx, obj);
	if(!e) printf("OH SHIT!!!!\n");
	lua_settop(L, 1);
	lua_json_object_pop(L);

	if(ret) {
		uint8_t vtype = lua_type(L, -1);

		switch(vtype)
		{
			case L_STR: 
				val->s = lua_tolstring(L, -1, &val->size);
				break;
			case L_NUM:
			case L_INT:
				if(vtype == L_NUM && val->type != L_INT) // <-- user must specify val.type = L_INT to receive an int in lua 5.1
				// NEED VERSION CHECK if(LUA_VERSOIN >=503 && lua_checkinteger(L, -1)) {
				//val->buf.int32 = lua_tointeger(L, -1);
				// val->type = L_INT;
					val->n = lua_tonumber(L, -1);
				else
					val->i32 = lua_tointeger(L, -1);

				lua_pushvalue(L, -1);
				lua_tolstring(L, -1, &val->size);
				break;
			case L_BOOL: 
				val->b = lua_toboolean(L, -1);
				val->size = sizeof(bool);
				val->type = L_BOOL;
				break;
			case L_NESTED:
			case L_INT64:
				if(lua_json_is_elm(L, -1)) {
					json_elm *nested = check_json_elm(L, -1, false);
					val->nested = (luajson_node_t*)nested->env_id;
					val->size = nested->base->rlen;
					val->type = L_NESTED;
				}

				if(lua_json_is_int64(L, -1)) {
					json_int64 *ud = (json_int64 *)lua_touserdata(L, -1);
					val->i64 = ud->value;
					val->size = ud->length(L);
					val->type = L_INT64;
				}

				break;
			default:
				break;
		}
	}

	lua_settop(L, 0);
	return (ret && val->size > 0);
};

size_t luajson_object_push(luajson_ctx_t *ctx, luajson_node_t *obj, const char *key, luajson_value_t *val) 
{
	lua_State *L = ctx->L;
	json_elm *e = luajson_push_elm(ctx, obj); //lua_rawgeti(L, LUA_REGISTRYINDEX, arr->ref_id);

	lua_settop(L, 1);
	e->key = key;
	bool ok = e->ops->key_to_idx(e, true);

	if(ok) {
		//lua_pushinteger(L, e->nelms-1);
		lua_pushstring(L, key);
		luajson_push_val(ctx, val);
	}
	else
		return (size_t)luaL_error(L, "ERROR: Object: Failed to push vaule type: %s", lua_typename(L, lua_type(L, -1)));

	lua_settable(L, -3);

	lua_settop(L, 0);

	return e->nelms;
};

void range_num(luajson_range_t *r, uint8_t pos, uint16_t num) 
{
	switch(r->type) {
		case arr : {
			if(pos > 0) 
				r->arr.end = num;
			else
				r->arr.start = num;

			break;
		}
		case obj : {
			if(pos > 0) {
				r->obj.end.idx = num;
				r->obj.end.type = idx;
			}
			else {
				r->obj.start.idx = num;
				r->obj.end.type = idx;
			}
			break;
		}
		default:
			break;
	}
}

void range_key(luajson_range_t *r, uint8_t pos, const char *k)
{
	if(pos > 0) {
		r->obj.end.key = k;
		r->obj.end.type = key;
	}
	else {
		r->obj.start.key = k;
		r->obj.start.type = key;
	}
	
}

/*int json_string(luajson_ctx_t *ctx, const char *str)
{
	lua_State *L = (lua_State*)ctx->L;
	lua_pushstring(L, str);
}*/

bool luajson_object_reverse(luajson_ctx_t *ctx, luajson_node_t *o, luajson_range_t *r) 
{
	lua_State *L = ctx->L;
	json_elm *e = luajson_push_elm(ctx, o);

	if(!e) { lua_settop(L, 0); return 0;}
	lua_settop(L, 1);

	if(r && r->type == obj) {
		if(r->obj.start.type == key)
			lua_pushstring(L, r->obj.start.key);
		else
			lua_pushinteger(L, r->obj.start.idx);

		if(r->obj.end.type == key)
			lua_pushstring(L, r->obj.end.key);
		else
			lua_pushinteger(L, r->obj.end.idx);
	}

	if(lua_isnil(L, -1))
		return luaL_error(L, "ERROR: Object: Failed to reverse %s", lua_typename(L, lua_type(L, -1)));

	dumpstack(L, "reverse");
	lua_json_object_reverse(L);

	lua_settop(L, 0);

	return true;
};



/*bool luajson_object_get(luajson_ctx_t *ctx, luajson_node_t *obj, const char *key, uint16_t type, luajson_value_t *val)
{

	lua_State *L = ctx->L;
	lua_rawgeti(L, LUA_REGISTRYINDEX, obj->ref_id); 

	json_elm *elm = check_json_elm(L, -1, false);

	elm->key = key;

	if(!elm->ops->key_to_idx(elm, false))
		luaL_error(L, "ERROR: key ( %s ) does not exist", key);
	else
		elm->ops->check_idx(elm);

	get_json_table(L, 1);
	lua_pushstring(L, elm->key);
	lua_rawget(L, -2);

	val->size = 0;

	switch(lua_type(L, -1))
	{
		case L_STR: 
			val->buf.s = lua_tolstring(L, -1, &val->size);
			break;
		case L_NUM:
		case L_INT:
			if(type == L_INT)
				val->buf.i32 = lua_tointeger(L, -1);
			else
				val->buf.n = lua_tonumber(L, -1);

			lua_pushvalue(L, -1);
			lua_tolstring(L, -1, &val->size);

			break;
		case L_BOOL: 
			val->buf.b = lua_toboolean(L, -1);
			val->size = sizeof(bool);
			break;
		case L_NESTED:
			val->buf.nested = check_json_elm(L, -1, false);
			val->size = val->buf.nested->base->rlen;
			break;
		default:
			break;
	}
	lua_settop(L, 0);

	return (val->size > 0);
}

bool luajson_object_set(luajson_ctx_t *ctx, luajson_node_t *obj, const char *key, uint8_t type, luajson_value_t *val)
{
	lua_State *L = ctx->L;
	lua_rawgeti(L, LUA_REGISTRYINDEX, obj->ref_id); 

	json_elm *elm = check_json_elm(L, -1, false);

	elm->key = key;
	bool ok = elm->ops->key_to_idx(elm, true);
	elm->ops->check_idx(elm);

	get_json_table(L, 1);
	// stack { env, env }
	lua_pushstring(L, elm->key);
	// stack { env, env, key }

	switch(type)
	{
		case L_STR: 
			lua_pushstring(L, val->buf.s);
			break;
		case L_NUM:
		case L_INT:
			if(type == L_NUM)
				lua_pushnumber(L, val->buf.n);
			else
				lua_pushinteger(L, val->buf.i32);

			break;
		case L_BOOL: 
			lua_pushboolean(L, val->buf.b);
			break;
		case L_NESTED:
			lua_pushlightuserdata(L, (void*)val->buf.nested->env_id);
			// stack { env, env, key,  env_id }
			lua_rawget(L, LUA_REGISTRYINDEX);
			// stack { env, env, key, vtable }
			lua_getfield(L, -1, "ctx");
			// stack { elm, env, key, vtable, nested }
			if(lua_type(L, -1) != LUA_TUSERDATA)
				lua_replace(L, -2);

			break;
		default:
			break;
	}
	
	if(lua_gettop(L) == 4) {
		lua_settable(L, -3);
		ok = true;
	}
	
	lua_settop(L, 0);
	return ok;
};*/

/* ==============================================================================
 * 5. SERIALIZATION PIPELINE
 * ============================================================================== */

const char* luajson_stringify(luajson_ctx_t *ctx, luajson_node_t *root, luajson_range_t *range, size_t *out_len, bool esc) {
	lua_State *L = ctx->L;
	json_elm *elm = luajson_push_elm(ctx, root);
	if (!elm) {
		lua_settop(L, 0);
		return NULL;
	}

	elm->escape = esc ? esc : 0;
	elm->mode = MARSHAL_JSON;

	if(range) {
		switch(range->type) {

			case arr:
				lua_pushinteger(L, range->arr.start);
				lua_pushinteger(L, range->arr.end);

				break;
			
			case obj: 
				if(range->obj.start.type != nomap)
					range->obj.start.type == key 
							? lua_pushstring(L, range->obj.start.key) 
							: lua_pushinteger(L, range->obj.start.idx);
					

				if(range->obj.end.type != nomap)
					range->obj.end.type == key
							? lua_pushstring(L, range->obj.start.key) 
							: lua_pushinteger(L, range->obj.start.idx);

				break;
			
			default:
				break;
		}
	}

	dumpstack(L, "stringify");
	elm->ops->stringify(L);                          
	dumpstack(L, "stringify");
	const char *json_res = lua_tolstring(L, -1, out_len);

	lua_settop(L, 0); 
	return json_res;
}

size_t luajson_get_size(luajson_ctx_t *ctx, luajson_node_t *n) {
	json_elm *elm = luajson_push_elm(ctx, n);
	lua_State *L = elm->L;
	size_t size = elm ? elm->nelms : 0;
	lua_settop(L, 0);

	return size;
}

