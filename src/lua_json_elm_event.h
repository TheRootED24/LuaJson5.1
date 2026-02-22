/*
 * LuaJson - a json api library for Lua
 *
 *   Copyright (C) 2026 TheRootED24 <TheRootED24@gmail.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in JSONliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef LUA_JSON_ELM_EVENT_H
#define LUA_JSON_ELM_EVENT_H

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


#include <stddef.h>
#include <stdatomic.h>
#include <threads.h>

// includes
#include "lua_json.h"

#ifdef __cplusplus
}
#endif



typedef void (*NotifyFn)(void* context, size_t rlen, size_t quoted, size_t nkeys);

typedef struct ObserverNode {
    void* context;
    NotifyFn callback;
    struct ObserverNode* next;
} ObserverNode;

typedef struct Subject {
    size_t rlen;
    size_t quoted;
    size_t nkeys;
    ObserverNode* observers;
#ifdef USE_THREADS
    mtx_t lock;
#endif
} Subject;

int subject_init(Subject* s);
void subject_subscribe(Subject* s, void* context, NotifyFn callback);
void subject_unsubscribe(Subject* s, void* context, NotifyFn callback);
void subject_set_values(Subject* s, size_t r, size_t q, size_t n);
void subject_get_values(Subject* s, size_t* r, size_t* q, size_t* n);
void subject_cleanup(Subject* s);
//void subject_init(Subject *s, size_t initial_val);
//void subject_subscribe(Subject* s, void* context, NotifyFn callback);
//void subject_unsubscribe(Subject* s, void* context, NotifyFn callback);
//void subject_set_value(Subject* s, size_t new_val);
//void subject_cleanup(Subject* s);

#endif