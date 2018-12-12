#pragma once
#include <stdlib.h>
#include "utils.h"

typedef enum {LONGEST, SHORTEST} matching_policy_t;

typedef enum {SWITCH, LIST, REVERSED} trace_enum_t;

struct Trace {
    union {
        struct {
            // TODO change index for char
            int index;
            matching_policy_t policy;
        };
        const struct TraceList* next;
    };
    trace_enum_t enum_;
};

struct TraceList {
    const struct TraceList* next;
    union {
        struct Trace trace;
        struct {} empty;
    };
};

STATIC_RC_HEADER(empty_trace_list, struct TraceList, ({.empty = {}, .next = NULL}))

static inline struct TraceList* alloc_trace_list() {
    return (struct TraceList*)ref_counted_alloc(sizeof(struct TraceList));
}

static inline const struct TraceList* push_swith_trace(const struct TraceList* trace_list, int index, matching_policy_t policy) {
    struct TraceList* new_trace_list = alloc_trace_list();
    new_trace_list->next = trace_list;
    new_trace_list->trace.enum_ = SWITCH;
    new_trace_list->trace.index = index;
    new_trace_list->trace.policy = policy;
    return new_trace_list;
}

static inline const struct TraceList* push_list_trace(const struct TraceList* trace_list, const struct TraceList* next) {
    struct TraceList* new_trace_list = alloc_trace_list();
    new_trace_list->next = trace_list;
    new_trace_list->trace.enum_ = LIST;
    new_trace_list->trace.next = next;
    return new_trace_list;
}

static inline const struct TraceList* push_reversed_trace(const struct TraceList* trace_list, const struct TraceList* next) {
    struct TraceList* new_trace_list = alloc_trace_list();
    new_trace_list->next = trace_list;
    new_trace_list->trace.enum_ = REVERSED;
    new_trace_list->trace.next = next;
    return new_trace_list;
}

static inline void clean_trace_list(const void* ptr) {
    const struct TraceList* self = ptr;
    switch (self->trace.enum_) {
    case LIST:
    case REVERSED:
        decr_count(self->trace.next, &clean_trace_list);
        break;
    default:
        break;
    }
    decr_count(self->next, &clean_trace_list);
}
