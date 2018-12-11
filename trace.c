#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "trace.h"
#include "utils.h"

static inline struct TraceList* alloc() {
    return (struct TraceList*)ref_counted_alloc(sizeof(struct TraceList));
}

const struct TraceList* push_swith_trace(const struct TraceList* trace_list, int index, matching_policy_t policy) {
    struct TraceList* new_trace_list = alloc();
    new_trace_list->next = trace_list;
    new_trace_list->trace.enum_ = SWITCH;
    new_trace_list->trace.index = index;
    new_trace_list->trace.policy = policy;
    return new_trace_list;
}

const struct TraceList* push_list_trace(const struct TraceList* trace_list, const struct TraceList* next) {
    struct TraceList* new_trace_list = alloc();
    new_trace_list->next = trace_list;
    new_trace_list->trace.enum_ = LIST;
    new_trace_list->trace.next = next;
    return new_trace_list;
}

const struct TraceList* push_reversed_trace(const struct TraceList* trace_list, const struct TraceList* next) {
    struct TraceList* new_trace_list = alloc();
    new_trace_list->next = trace_list;
    new_trace_list->trace.enum_ = REVERSED;
    new_trace_list->trace.next = next;
    return new_trace_list;
}

STATIC_RC(empty_trace_list, struct TraceList, {.empty = {} COMMA .next = NULL})

void clean_trace_list(const void* ptr) {
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
