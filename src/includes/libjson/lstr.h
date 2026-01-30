#ifndef LUA_STR_H
#define LUA_STR_H

#ifndef __cplusplus
// LUA LIBS FOR gcc
#include <lua.h>                               
#include <lauxlib.h>                           
#include <lualib.h>
#endif

#ifdef __cplusplus
// LUA LIBS FOR g++
#include <lua.hpp>
extern "C" {
#endif

// STD LIBS
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

#ifdef __cplusplus
}
#endif
// Describes an arbitrary chunk of memory
struct lua_str {
  char *buf;   // String data
  size_t len;  // String length
};



// Using macro to avoid shadowing C++ struct constructor, see #1298
#define lua_str(s) lua_str_s(s)

struct lua_str lua_str(const char *s);
struct lua_str lua_str_n(const char *s, size_t n);
int lua_casecmp(const char *s1, const char *s2);
int lua_strcmp(const struct lua_str str1, const struct lua_str str2);
int lua_strcasecmp(const struct lua_str str1, const struct lua_str str2);
struct lua_str lua_strdup(const struct lua_str s);
bool lua_match(struct lua_str str, struct lua_str pattern, struct lua_str *caps);
bool lua_span(struct lua_str s, struct lua_str *a, struct lua_str *b, char delim);

bool lua_str_to_num(struct lua_str, int base, void *val, size_t val_len);


#endif