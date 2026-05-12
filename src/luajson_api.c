#include "luajson_api.h"
#include "lua_json.h" // Injected project headers
#include <stdlib.h>
#include <stdarg.h>

/* ==============================================================================
 * 1. STRUCT DEFINITIONS & PROTOTYPES
 * ============================================================================== */
struct luajson_ctx {
    lua_State *L;
};

struct luajson_node {
    int ref_id;
};

/* External declarations matching your core v1.1.1 engine module open entry */
LUALIB_API int luaopen_JSON(lua_State *L);

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
             case 'i': { /* Number (Double) */
                int i = va_arg(args, int);
                lua_pushinteger(L, i);
                count++;
                break;
            }
            case 'b': { /* Boolean */
                int b = va_arg(args, int); // Booleans are passed as ints in variadic parameters
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

void luajson_close(luajson_ctx_t *ctx) {
    if (!ctx) return;
    
    /* Wiping the parent state context automatically drops the global registry,
     * cleanly vaporizing all nested node handles at once with 0 leaks */
    lua_close(ctx->L);
    free(ctx);
}

/* ==============================================================================
 * 4. VARIADIC CONSTRUCTORS (Objects & Arrays)
 * ============================================================================== */
luajson_node_t* luajson_create_object(luajson_ctx_t *ctx, const char *fmt, ...) {
    // FIX: Remove the 'if (!ctx || !fmt) return NULL;' line entirely!
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
}

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
}

/* ==============================================================================
 * 5. SERIALIZATION PIPELINE
 * ============================================================================== */
const char* luajson_stringify(luajson_ctx_t *ctx, luajson_node_t *root, size_t *out_len) {
    // FIX: Remove the 'if (!ctx || !root) return NULL;' line entirely!
    lua_State *L = ctx->L;

    lua_rawgeti(L, LUA_REGISTRYINDEX, root->ref_id); 

    json_elm *elm = (json_elm *)lua_touserdata(L, -1);
    if (!elm) {
        lua_settop(L, 0);
        return NULL;
    }

    elm->escape = 0; 
    elm->mode = MARSHAL_JSON;

    elm->ops->stringify(L);                          

    const char *json_res = lua_tolstring(L, -1, out_len);

    lua_settop(L, 0); 
    return json_res;
}

