#include "lua_json_elm_event.h"

// --- API Implementation ---

/*void subject_init(Subject *s, size_t initial_val) {
    atomic_init(&s->value, initial_val);
    s->observers = NULL;
    // Use mtx_recursive so callbacks can safely modify the observer list
    mtx_init(&s->lock, mtx_plain | mtx_recursive);
}*/

int subject_init(Subject* s) {
    s->rlen = 0;
    s->quoted = 0;
    s->nkeys = 0;
    s->observers = NULL;
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

void subject_set_values(Subject* s, size_t r, size_t q, size_t n) {
    LOCK(s);
    s->rlen = r;
    s->quoted = q;
    s->nkeys = n;

    ObserverNode* curr = s->observers;
    while (curr) {
        curr->callback(curr->context, r, q, n);
        curr = curr->next;
    }
    UNLOCK(s);
}

void subject_get_values(Subject* s, size_t* r, size_t* q, size_t* n) {
    LOCK(s);
    if (r) *r = s->rlen;
    if (q) *q = s->quoted;
    if (n) *n = s->nkeys;
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
    UNLOCK(s);
    DESTROY_LOCK(s);
}

/*
void subject_subscribe(Subject *s, void* context, NotifyFn callback) {
    ObserverNode* newNode = malloc(sizeof(ObserverNode));
    if (!newNode) return;
    
    newNode->context = context;
    newNode->callback = callback;

    mtx_lock(&s->lock);
    newNode->next = s->observers;
    s->observers = newNode;
    mtx_unlock(&s->lock);
}

void subject_set_value(Subject* s, size_t new_val) {
    // Lock-free update of the actual value
    atomic_store(&s->value, new_val);

    // Notify all registered observers
    mtx_lock(&s->lock);
    ObserverNode* curr = s->observers;
    while (curr != NULL) {
        curr->callback(curr->context, new_val);
        curr = curr->next;
    }
    mtx_unlock(&s->lock);
}

void subject_unsubscribe(Subject *s, void *context, NotifyFn callback) {
    // 1. Lock the list to prevent concurrent modification
    mtx_lock(&s->lock);
    ObserverNode **curr = &s->observers;
    while (*curr != NULL) {
        ObserverNode *entry = *curr;
        
        // 2. Identify the specific listener by matching both context and function
        if (entry->context == context && entry->callback == callback) {
            *curr = entry->next; // Bypass the node in the list
            free(entry);         // Deallocate the node memory
            mtx_unlock(&s->lock); // Release lock and exit early
            printf("Unsubscribed !!\n");
            return;
        }
        curr = &entry->next;
    }
    
    // 3. Always unlock, even if no match was found
    mtx_unlock(&s->lock);
    printf("Unsubscribed !!\n");
}

void subject_cleanup(Subject* s) {
    mtx_lock(&s->lock);
    ObserverNode* curr = s->observers;
    while (curr != NULL) {
        ObserverNode* next = curr->next;
        free(curr);
        curr = next;
    }
    s->observers = NULL;
    mtx_unlock(&s->lock);
    mtx_destroy(&s->lock);
}

// --- Usage Example ---

void on_change_logger(void* ctx, size_t val) {
    printf("[Signal] Value updated to: %zu\n", val);
}
*/
