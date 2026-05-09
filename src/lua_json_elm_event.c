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
 
#include "lua_json_elm_event.h"

// --- API Implementation ---

int subject_init(Subject* s) {
    s->event = (event*)malloc(sizeof(event));
    memset(s->event, 0, sizeof(event)); 
    return INIT_LOCK(s);
}

void subject_subscribe(Subject* s, void* context, NotifyFn callback) {
    ObserverNode* newNode = malloc(sizeof(ObserverNode));
    if (!newNode) return;
    newNode->context = context;
    newNode->callback = callback;

    LOCK(s);
    newNode->next = s->observers;
    s->observers = newNode;
    UNLOCK(s);
}

void subject_unsubscribe(Subject* s, void* context, NotifyFn callback) {
    LOCK(s);
    ObserverNode** curr = &s->observers;
    while (*curr) {
        ObserverNode* entry = *curr;
        if (entry->context == context && entry->callback == callback) {
            *curr = entry->next;
            free(entry);
            UNLOCK(s);
            return;
        }
        curr = &entry->next;
    }
    UNLOCK(s);
}

void subject_set_values(Subject* s, event *ev) {
    LOCK(s);
    ObserverNode* curr = s->observers;
    while (curr) {
        curr->callback(curr->context, ev);
        curr = curr->next;
    }
    UNLOCK(s);
}

void print_subscribers(Subject* s) {
    LOCK(s);
    ObserverNode* curr = s->observers;
    while (curr) {
        ObserverNode* next = curr->next;
        
        curr = next;
    }
    UNLOCK(s);
}

void subject_cleanup(Subject* s) {
    LOCK(s);
    ObserverNode* curr = s->observers;
    while (curr) {
        ObserverNode* next = curr->next;
        free(curr);
        curr = next;
    }
    s->observers = NULL;

    free(s->event);
    UNLOCK(s);
    DESTROY_LOCK(s);
}
