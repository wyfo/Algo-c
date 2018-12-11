#pragma once
#include "reader.h"
#include "tag.h"

typedef enum {INCREASING = 1, DECREASING = -1} loop_ordering_t;

struct LoopReader {
    const struct TraceList* prev_traces;
    struct Reader ref;
    struct Reader variant;
    size_t cursor;
    matching_policy_t policy;
    loop_ordering_t ordering;
    tag_t tag;
};

struct Reader loop_reader_of(struct Reader ref, matching_policy_t policy, loop_ordering_t ordering, tag_t tag);
