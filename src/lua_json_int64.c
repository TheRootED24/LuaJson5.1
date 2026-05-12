
#include "lua.h"
#include "lauxlib.h"
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include "lua_json_int64.h"

#define LUA_INT64_METATABLE "JSON.int64"

/* 1. THE CONCRETE CALCULATION FUNCTION
 * This is the real machine code block that executes behind the scenes */
static size_t native_int64_get_len(lua_State *L) {
    /* Safe lookahead extraction: index -1 is always the target on the stack frame */
    json_int64 *ud = (json_int64 *)lua_touserdata(L, -1);
    if (!ud) return 0;

    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%" PRId64, ud->value);
    
    return (len > 0) ? (size_t)len : 0;
}



/* Forward declaration for the object constructor pusher */
void lua_pushint64(lua_State *L, int64_t val);

/* Standard cross-version helper to push an integer onto the stack safely */
static void push_version_int(lua_State *L, lua_Integer val) {
#if LUA_VERSION_NUM >= 503
    lua_pushinteger(L, val);
#else
    lua_pushnumber(L, (lua_Number)val);
#endif
}

/* Helper to safely extract a 64-bit value from either an int64 udata or a native number */
static int64_t get_int64_arg(lua_State *L, int arg_idx) {
    if (lua_isuserdata(L, arg_idx)) {
        json_int64 *ud = (json_int64 *)luaL_checkudata(L, arg_idx, LUA_INT64_METATABLE);
        return ud->value;
    }
    /* Fallback to reading standard float-doubles if a script mixes types (e.g. udata + 10) */
    return (int64_t)luaL_checknumber(L, arg_idx);
}

/* --- CORE METAMETHODS --- */

/* __tostring metamethod: converts the 64-bit int to a text string automatically */
static int int64_tostring(lua_State *L) {
    json_int64 *ud = (json_int64 *)luaL_checkudata(L, 1, LUA_INT64_METATABLE);
    char buf[32];
    snprintf(buf, sizeof(buf), "%" PRId64, ud->value);
    lua_pushstring(L, buf);
    return 1;
}

/* __len metamethod: Returns the precise string digit count across ALL versions */
static int int64_len(lua_State *L) {
    json_int64 *ud = (json_int64 *)luaL_checkudata(L, 1, LUA_INT64_METATABLE);
    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%" PRId64, ud->value);
    
    push_version_int(L, len); 
    return 1;
}

/* --- RELATIONAL METAMETHODS --- */

/* __eq metamethod: allows all versions to compare two 64-bit userdata objects safely */
static int int64_eq(lua_State *L) {
    int64_t a = get_int64_arg(L, 1);
    int64_t b = get_int64_arg(L, 2);
    lua_pushboolean(L, a == b);
    return 1;
}

/* __lt metamethod: allows all versions to compare less-than boundaries safely */
static int int64_lt(lua_State *L) {
    int64_t a = get_int64_arg(L, 1);
    int64_t b = get_int64_arg(L, 2);
    lua_pushboolean(L, a < b);
    return 1;
}

/* --- ARITHMETIC METAMETHODS --- */

static int int64_add(lua_State *L) {
    int64_t a = get_int64_arg(L, 1);
    int64_t b = get_int64_arg(L, 2);
    lua_pushint64(L, a + b);
    return 1;
}

static int int64_sub(lua_State *L) {
    int64_t a = get_int64_arg(L, 1);
    int64_t b = get_int64_arg(L, 2);
    lua_pushint64(L, a - b);
    return 1;
}

static int int64_mul(lua_State *L) {
    int64_t a = get_int64_arg(L, 1);
    int64_t b = get_int64_arg(L, 2);
    lua_pushint64(L, a * b);
    return 1;
}

static int int64_div(lua_State *L) {
    int64_t a = get_int64_arg(L, 1);
    int64_t b = get_int64_arg(L, 2);
    if (b == 0) return luaL_error(L, "Critical: Division by zero inside int64 vtable loop.");
    lua_pushint64(L, a / b);
    return 1;
}

static int int64_unm(lua_State *L) {
    json_int64 *ud = (json_int64 *)luaL_checkudata(L, 1, LUA_INT64_METATABLE);
    lua_pushint64(L, -ud->value);
    return 1;
}

/* ==============================================================================
 * REGISTRY AND CREATION LIFE CYCLE HOOKS
 * ============================================================================== */

/* Internal helper to register the metatable once during luajson_init() */
void luajson_register_int64(lua_State *L) {
    if (luaL_newmetatable(L, LUA_INT64_METATABLE)) {
        lua_pushcfunction(L, int64_tostring); lua_setfield(L, -2, "__tostring");
        lua_pushcfunction(L, int64_len);      lua_setfield(L, -2, "__len");
        lua_pushcfunction(L, int64_add);      lua_setfield(L, -2, "__add");
        lua_pushcfunction(L, int64_sub);      lua_setfield(L, -2, "__sub");
        lua_pushcfunction(L, int64_mul);      lua_setfield(L, -2, "__mul");
        lua_pushcfunction(L, int64_div);      lua_setfield(L, -2, "__div");
        lua_pushcfunction(L, int64_unm);      lua_setfield(L, -2, "__unm");
        lua_pushcfunction(L, int64_eq);       lua_setfield(L, -2, "__eq");
        lua_pushcfunction(L, int64_lt);       lua_setfield(L, -2, "__lt");
        lua_pop(L, 1); /* pop metatable */
    }
}

/* Helper to push a new 64-bit userdata object onto the stack */
/* 2. THE ALLOCATION LIFECYCLE HOOK
 * Every time an int64 object is created, we wire up the pointer immediately */
void lua_pushint64(lua_State *L, int64_t val) {
    /* Allocate the raw structure memory chunk inside the Lua VM state */
    json_int64 *ud = (json_int64 *)lua_newuserdata(L, sizeof(json_int64));
    
    /* Assign the raw 8-byte payload */
    ud->value = val;
    
    /* PLUMB THE POINTER: This binds your virtual method to the real C block!
     * This stops ud->length from pointing into wild space and guarantees safety. */
    ud->length = native_int64_get_len;
    
    /* Set the metatable for script accessibility */
    luaL_getmetatable(L, LUA_INT64_METATABLE);
    lua_setmetatable(L, -2);
}


