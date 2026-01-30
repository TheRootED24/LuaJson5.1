#ifndef BASE64_H
#define BASE64_H

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>

size_t _lua_base64_update(unsigned char input_byte, char *buf, size_t len);
size_t _lua_base64_final(char *buf, size_t len);
size_t _lua_base64_encode(const unsigned char *p, size_t n, char *buf, size_t);
size_t _lua_base64_decode(const char *src, size_t n, char *dst, size_t);

#endif