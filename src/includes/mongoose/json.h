#pragma once

//#include "arch.h"
#include "str.h"


#ifndef MG_JSON_MAX_DEPTH
#define MG_JSON_MAX_DEPTH 30
#endif

// Error return values - negative. Successful returns are >= 0
enum { MG_JSON_TOO_DEEP = -1, MG_JSON_INVALID = -2, MG_JSON_NOT_FOUND = -3 };
int mg_json_get(struct mg_str json, const char *path, int *toklen);
size_t mg_json_next(struct mg_str obj, size_t ofs, struct mg_str *key, struct mg_str *val);

