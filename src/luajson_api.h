#ifndef LUAJSON_API_H
#define LUAJSON_API_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "lua_json.h" // Injected project headers


#ifdef __cplusplus
extern "C" {
#endif

/* ==============================================================================
 * 1. OPAQUE STRUCT HANDLES
 * ============================================================================== */

struct luajson_ctx {
	lua_State *L;
};

typedef struct luajson_ctx  luajson_ctx_t;
typedef struct luajson_node luajson_node_t;
typedef enum LUA_JSON_TYPES luajson_range_type;

typedef enum {
    L_BOOL = 1,
    L_INT ,
    L_NUM ,
    L_STR,
    L_NESTED = 7,
    L_INT64 = 8
} luajson_types;

typedef enum {
	key,
	idx,
	nomap
} range_type;

typedef enum {
    arr,
    obj
} luajson_json_type;

typedef struct lua_json_pos {
    range_type type;
    union {
        const char *key;
        uint16_t idx;
    };
}luajson_pos_t;

typedef struct luajson_stack_frame {
    int8_t type;
    int tref;
} luajson_stack_frame_t;

typedef struct stack_state {
    int *types;
    int *refs;
} stack_state_t;

typedef struct luajson_value {
    luajson_types type;
    luajson_pos_t pos;
    size_t size;
    union {
        bool b;
        double n;
        int32_t i32;
        int64_t i64;
        const char *s;
        luajson_node_t *nested;
    };
}luajson_value_t;

typedef struct array_range {
    uint16_t start;
	uint16_t end;
} array_range_t;

typedef struct object_range {
    luajson_pos_t start;
    luajson_pos_t end;
} object_range_t;

typedef struct luajson_range {
	luajson_json_type type;
    union {
        array_range_t arr;
        object_range_t obj;
    };
} luajson_range_t;

typedef struct luajson_map {
    luajson_json_type maptype;
    luajson_range_t **pos;
    luajson_value_t **values;
} luajson_map_t;

typedef struct luajson_value luajson_value_t;
typedef struct json_elm json_elm;
typedef LUA_JSON_TYPES json_type;


/* ==============================================================================
 * 2. SANDBOX LIFESPAN MANAGEMENT
 * ============================================================================== */


luajson_ctx_t* luajson_init(void);

json_elm *luajson_push_elm(luajson_ctx_t *ctx, luajson_node_t *elm);

void luajson_close(luajson_ctx_t *ctx);

/* ==============================================================================
 * 3. VARIADIC REVERSE-DUMPSTACK CONSTRUCTORS
 * ============================================================================== */


luajson_node_t* luajson_create_object(luajson_ctx_t *ctx, const char *fmt, ...) 
    __attribute__((nonnull(1, 2)));

luajson_node_t* luajson_create_array(luajson_ctx_t *ctx, const char *fmt, ...) 
    __attribute__((nonnull(1, 2)));

/* ==============================================================================
 * 4. HIGH-SPEED SERIALIZATION RUNNER
 * ============================================================================== */

const char* luajson_stringify(luajson_ctx_t *ctx, luajson_node_t *root, luajson_range_t *range, size_t *out_len, bool esc) 
    __attribute__((nonnull(1, 2, 4)));

bool luajson_array_pop(luajson_ctx_t *ctx, luajson_node_t *arr, luajson_value_t *val) 
    __attribute__((nonnull(1, 2)));

size_t luajson_array_push(luajson_ctx_t *ctx, luajson_node_t *arr, luajson_value_t *val) 
    __attribute__((nonnull(1, 2, 3)));

bool luajson_array_move(luajson_ctx_t *ctx, luajson_node_t *arr, luajson_range_t *r)
    __attribute__((nonnull(1, 2, 3)));

bool luajson_array_reverse(luajson_ctx_t *ctx, luajson_node_t *arr, luajson_range_t *r)
    __attribute__((nonnull(1, 2, 3)));

bool luajson_array_set(luajson_ctx_t *ctx, luajson_node_t *arr, uint16_t idx, uint8_t type, luajson_value_t *val)
    __attribute__((nonnull(1, 2, 5)));

bool luajson_array_get(luajson_ctx_t *ctx, luajson_node_t *arr, uint16_t idx, uint16_t type, luajson_value_t *val)
    __attribute__((nonnull(1, 2, 5)));

bool luajson_object_get(luajson_ctx_t *ctx, luajson_node_t *obj, const char *key, uint16_t type, luajson_value_t *val)
    __attribute__((nonnull(1, 2, 5)));

bool luajson_object_set(luajson_ctx_t *ctx, luajson_node_t *obj, const char *key, uint8_t type, luajson_value_t *val)
    __attribute__((nonnull(1, 2, 5)));

bool luajson_object_pop(luajson_ctx_t *ctx, luajson_node_t *obj, luajson_value_t *val)
    __attribute__((nonnull(1, 2)));

bool luajson_object_reverse(luajson_ctx_t *ctx, luajson_node_t *o, luajson_range_t *r)
    __attribute__((nonnull(1, 2)));

size_t luajson_object_push(luajson_ctx_t *ctx, luajson_node_t *obj, const char *key, luajson_value_t *val)
    __attribute__((nonnull(1, 2, 3, 4)));

size_t luajson_get_size(luajson_ctx_t *ctx, luajson_node_t *n)
    __attribute__((nonnull(1, 2)));

void range_key(luajson_range_t *r, uint8_t pos, const char *k)
    __attribute__((nonnull(1)));

void range_num(luajson_range_t *r, uint8_t pos, uint16_t num)
    __attribute__((nonnull(1)));

#ifdef __cplusplus
}
#endif

#endif /* LUAJSON_API_H */
