#ifndef LUAJSON_API_H
#define LUAJSON_API_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ==============================================================================
 * 1. OPAQUE STRUCT HANDLES
 * ============================================================================== */
typedef struct luajson_ctx  luajson_ctx_t;
typedef struct luajson_node luajson_node_t;

/* ==============================================================================
 * 2. SANDBOX LIFESPAN MANAGEMENT
 * ============================================================================== */

/**
 * Instantiates an isolated Lua VM context state sandbox.
 * @return Pointer to context handle, or NULL on failure.
 */
luajson_ctx_t* luajson_init(void);

/**
 * Closes the sandbox and cleanly frees all nested structural registries.
 * @param ctx Pointer to the active context handle.
 */
void luajson_close(luajson_ctx_t *ctx);

/* ==============================================================================
 * 3. VARIADIC REVERSE-DUMPSTACK CONSTRUCTORS
 * ============================================================================== */

/**
 * Creates a populated JSON object payload container using type token mappings.
 * Format Tokens (Passed as alternating Key-Value pairs):
 *   's' -> const char* (string key / string value)
 *   'n' -> const char* (string key) + double (numeric value)
 *   'b' -> const char* (string key) + int (boolean flag value: 0 or 1)
 *   'o' -> const char* (string key) + luajson_node_t* (nested node pointer)
 * 
 * Example: luajson_create_object(ctx, "snso", "code", 200.0, "data", sub_node);
 */
luajson_node_t* luajson_create_object(luajson_ctx_t *ctx, const char *fmt, ...) 
    __attribute__((nonnull(1, 2)));

luajson_node_t* luajson_create_array(luajson_ctx_t *ctx, const char *fmt, ...) 
    __attribute__((nonnull(1, 2)));

/* ==============================================================================
 * 4. HIGH-SPEED SERIALIZATION RUNNER
 * ============================================================================== */

/**
 * Triggers the recursive marshalling engine to build the final output JSON.
 * @param ctx Pointer to the active context handle.
 * @param root The top-level array or object node handle to serialize.
 * @param out_len Destination pointer to capture the exact string byte size.
 * @return A constant null-terminated string buffer pointing to the generated payload.
 *         Lifespan matches the parent context; it is vaporized during luajson_close().
 */
const char* luajson_stringify(luajson_ctx_t *ctx, luajson_node_t *root, size_t *out_len) 
    __attribute__((nonnull(1, 2, 3)));

#ifdef __cplusplus
}
#endif

#endif /* LUAJSON_API_H */
