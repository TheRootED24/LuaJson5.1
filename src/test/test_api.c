#include "../src/luajson_api.h"

#include <stdio.h>
#include <stdlib.h>

int main(void) {
    printf("Compiled with: %s\n", LUA_RELEASE);
    printf("=== Initializing Standalone Variadic C API ===\n");
    luajson_ctx_t *ctx = luajson_init();
    if (!ctx) {
        fprintf(stderr, "Fatal: Failed to instantiate Lua VM context sandbox.\n");
        return 1;
    }

    /* 1. Build a populated sub-array sequence in ONE single line using format tokens
     * "nnnn" -> Expects 4 sequential double numbers */
    printf("-> Building nested sequence array node\n");
    int64_t massive_timer = 9223372036854775807LL; // Max signed 64-bit Int
    luajson_node_t *my_arr = luajson_create_array(ctx, "dddnii", 1, 2, 3, 4.11, massive_timer,massive_timer );
    if (!my_arr) {
        fprintf(stderr, "Fatal: Failed to allocate array handle.\n");
        luajson_close(ctx);
        return 1;
    }

      /* 2. Build the primary object dictionary using format tokens
     * FIX: "ssssssno" maps out your 6 keys/values + 1 number + 1 node reference cleanly! */
    printf("-> Building root object dictionary node with nested payload\n");
    luajson_node_t *root = luajson_create_object(ctx, "sssssdse", 
    "status",  "success",                   // s, s
    "message", "C API payload test pass",   // s, s
    "code",    200,                         // s, d
    "payload", my_arr                       // s, o
    );

    if (!root) {
        fprintf(stderr, "Fatal: Failed to allocate root object handle.\n");
        luajson_close(ctx);
        return 1;
    }

    /* 3. Execute the high-speed recursive marshaling serialization loop */
      /* 3. Execute the high-speed recursive marshaling serialization loop */
    printf("-> Triggering stringify serialization pipeline\n");
    size_t json_len = 0;
    const char *json_out = luajson_stringify(ctx, root, NULL, &json_len, false);

    if (json_out && json_len > 0) {
        printf("\n=== Generated Payload (%zu bytes) ===\n%s\n\n", json_len, json_out);
    } else {
        fprintf(stderr, "Error: Stringify engine returned an empty pointer.\n");
    }

    size_t size = luajson_get_size(ctx, my_arr);
    printf("SIZE: %ld\n", size);

    luajson_value_t val = {0};
    
    // test pop
    bool ok = luajson_array_pop(ctx, my_arr, &val);
    if(ok) printf("Result: %ld\n", val.i64);

    size = luajson_get_size(ctx, my_arr);
    printf("SIZE: %ld\n", size);
    memset(&val, 0, sizeof(luajson_value_t));

    printf("-> Triggering stringify serialization pipeline\n");
    json_len = 0;
    json_out = luajson_stringify(ctx, root, NULL, &json_len, false);

    if (json_out && json_len > 0) {
        printf("\n=== Generated Payload (%zu bytes) ===\n%s\n\n", json_len, json_out);
    } else {
        fprintf(stderr, "Error: Stringify engine returned an empty pointer.\n");
    }
    memset(&val, 0, sizeof(luajson_value_t));

    // set a value
    //val.buf.s = "test me mofo!!";
    val.i64 = 9223372036854775807LL;
    size = luajson_get_size(ctx, my_arr);
    printf("SIZE: %ld\n", size);
    luajson_array_set(ctx, my_arr, 5, L_INT64, &val );
    memset(&val, 0, sizeof(luajson_value_t));
    size = luajson_get_size(ctx, my_arr);
    printf("SIZE: %ld\n", size);
    ok = luajson_array_get(ctx, my_arr, 5, L_INT64, &val);
    if(ok) printf("Result: %ld\n", val.i64);
    size = luajson_get_size(ctx, my_arr);
    printf("SIZE: %ld\n", size);

    printf("-> Triggering stringify serialization pipeline\n");
    luajson_range_t r = {0};

    r.arr.start = 0;
    r.arr.end = 5;

    json_len = 0;
    json_out = luajson_stringify(ctx, my_arr, &r, &json_len, true);
   
    size = luajson_get_size(ctx, my_arr);
    printf("SIZE: %ld\n", size);

    if (json_out && json_len > 0) {
        printf("\n=== Generated Payload (%zu bytes) ===\n%s\n\n", json_len, json_out);
    } else {
        fprintf(stderr, "Error: Stringify engine returned an empty pointer.\n");
    }

    json_len = 0;
    json_out = luajson_stringify(ctx, root, NULL, &json_len, false);

    if (json_out && json_len > 0) {
        printf("\n=== Generated Payload (%zu bytes) ===\n%s\n\n", json_len, json_out);
    } else {
        fprintf(stderr, "Error: Stringify engine returned an empty pointer.\n");
    }

    // make a second array
    //luajson_elm_parse(ctx, array:[1,2,3,4,true]<]])
    luajson_node_t *my_arr2 = luajson_create_array(ctx, "ddddb", 1, 2, 3, 4, true);
    if (!my_arr2) {
        fprintf(stderr, "Fatal: Failed to allocate array handle.\n");
        luajson_close(ctx);
        return 1;
    }
    memset(&val, 0, sizeof(luajson_value_t));
    val.nested = my_arr2;
    luajson_object_set(ctx, root, "arr2", L_NESTED, &val);
    memset(&val, 0, sizeof(luajson_value_t));

    luajson_object_get(ctx, root, "arr2", L_NESTED, &val);
    ok = printf("Result: %p\n", (void*)val.nested);
    memset(&val, 0, sizeof(luajson_value_t));

    val.nested = my_arr2;
    luajson_array_set(ctx, my_arr, 6, L_NESTED, &val);
    ok = luajson_array_get(ctx, my_arr, 6, L_NESTED, &val);
    if(ok) printf("Result: %p\n", (void*)val.nested);
    memset(&val, 0, sizeof(luajson_value_t));

    val.type = L_STR;
    val.s = "some stringer";

    luajson_array_push(ctx, my_arr2, &val);
    memset(&val, 0, sizeof(luajson_range_t));

    r.arr.start = 0;
    r.arr.end = 5;
    r.type = arr;

    luajson_array_move(ctx, my_arr2, &r);

    luajson_array_reverse(ctx, my_arr2, &r);

    memset(&val, 0, sizeof(luajson_value_t));
    val.b = false;
    val.type = L_BOOL;
    luajson_object_push(ctx, root, "bhool", &val);

    json_len = 0;
    json_out = luajson_stringify(ctx, root, NULL, &json_len, false);

    if (json_out && json_len > 0) {
        printf("\n=== Generated Payload (%zu bytes) ===\n%s\n\n", json_len, json_out);
    } else {
        fprintf(stderr, "Error: Stringify engine returned an empty pointer.\n");
    }

    memset(&val, 0, sizeof(luajson_value_t));
    ok = luajson_object_pop(ctx, root, &val);
    printf("Result: %s\n", val.b > 0 ? "true" : "false");
    memset(&r, 0, sizeof(luajson_range_t));
   
    r.type = obj;
    range_key(&r, 0, "status");
    range_num(&r, 1, 3);

    luajson_object_reverse(ctx, root, &r);

    json_len = 0;
    json_out = luajson_stringify(ctx, root, NULL, &json_len, false);

    if (json_out && json_len > 0) {
        printf("\n=== Generated Payload (%zu bytes) ===\n%s\n\n", json_len, json_out);
    } else {
        fprintf(stderr, "Error: Stringify engine returned an empty pointer.\n");
    }

    free(my_arr2);
    free(my_arr);
    free(root);
    /* 4. Complete global clean sweep (frees hidden VM states and data trees) */
    printf("=== Destroying Sandbox Context (All allocations vaporized) ===\n");
    luajson_close(ctx);
    
    return 0;
}
