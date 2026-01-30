#ifndef JSON_H
#define JSON_H

#include "lstr.h"
#include "base64.h"

#ifndef LUA_JSON_MAX_DEPTH
#define LUA_JSON_MAX_DEPTH 30
#endif

// Error return values - negative. Successful returns are >= 0
enum { LUA_JSON_TOO_DEEP = -1, LUA_JSON_INVALID = -2, LUA_JSON_NOT_FOUND = -3 };
int _lua_json_get(struct lua_str json, const char *path, int *toklen);

struct lua_str _lua_json_get_tok(struct lua_str json, const char *path);
bool _lua_json_get_num(struct lua_str json, const char *path, double *v);
bool _lua_json_get_bool(struct lua_str json, const char *path, bool *v);
long _lua_json_get_long(struct lua_str json, const char *path, long dflt);
char *_lua_json_get_str(struct lua_str json, const char *path);
char *_lua_json_get_hex(struct lua_str json, const char *path, int *len);
char *_lua_json_get_b64(struct lua_str json, const char *path, int *len);
bool _lua_json_unescape(struct lua_str str, char *buf, size_t len);
size_t _lua_json_next(struct lua_str obj, size_t ofs, struct lua_str *key,
                    struct lua_str *val);

#endif