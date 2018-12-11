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

const struct TraceList* push_swith_trace(const struct TraceList* trace_list, int index, matching_policy_t policy);
const struct TraceList* push_list_trace(const struct TraceList* trace_list, const struct TraceList* next);
const struct TraceList* push_reversed_trace(const struct TraceList* trace_reversed, const struct TraceList* next);

void clean_trace_list(const void*);
