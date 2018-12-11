#include <assert.h>
#include <stdlib.h>
#include "loop_reader.h"
#include "utils.h"
#include "resolution_reader.h"

VTABLE
PREFIXED_VTABLE(resolution)

static void _clean(const void* reader) {
    const struct LoopReader* self = reader;
    decr_count_reader(self->ref);
    decr_count_reader(self->variant);
    decr_count(self->prev_traces, clean_trace_list);
}

static inline struct LoopReader* alloc(struct Reader ref, matching_policy_t policy, loop_ordering_t ordering, tag_t tag) {
    assert(ref.self);
    struct LoopReader* ptr = ref_counted_alloc(sizeof(struct LoopReader));
    ptr->ref = clone_reader(ref);
    ptr->policy = policy;
    ptr->ordering = ordering;
    ptr->tag = tag;
    return ptr;
}

static inline struct Reader first_variant(struct Reader ref) {
    struct ReadingResult eps = epsilon(ref);
    assert(!eps.success);
    assert(eps.ongoing.self);
    return eps.ongoing;
}

static inline struct LoopReader* replace(const struct LoopReader* self, struct Reader ongoing) {
    struct LoopReader* ptr = alloc(self->ref, self->policy, self->ordering, self->tag);
    ptr->cursor = self->cursor;
    ptr->variant = ongoing;
    ptr->prev_traces = CLONE(self->prev_traces);
    return ptr;
}

static inline struct LoopReader* shift(const struct LoopReader* self, const struct TraceList* success) {
    struct LoopReader* ptr = alloc(self->ref, self->policy, self->ordering, self->tag);
    ptr->cursor = self->cursor + 1;
    ptr->variant = first_variant(self->ref);
    ptr->prev_traces = push_reversed_trace(CLONE(self->prev_traces), success);
    return ptr;
}

static struct ReadingResult _epsilon(const void* reader) {
    const struct LoopReader* self = reader;
    return (struct ReadingResult){
        push_swith_trace(empty_trace_list(), 0, self->policy),
        {replace(self, first_variant(self->ref)), &_vtable}
    };
}
static struct ReadingResult _read(const void* reader, char token) {
    const struct LoopReader* self = reader;
    struct ReadingResult res = read(self->variant, token);
    struct Reader ongoing = res.ongoing.self ? (struct Reader){replace(self, res.ongoing), &_vtable} : NO_READER;
    if (res.success) {
        const struct LoopReader* shifted = shift(self, res.success);
        const struct TraceList* success = CLONE(shifted->prev_traces);
        // res.success is used move, but by ref
        // should be incr and decr instead
        struct Reader ongoing2 = resolution_reader_of(&resolution_vtable, (struct Reader){shifted, &_vtable}, ongoing, res.success, self->cursor);
        return (struct ReadingResult){push_swith_trace(success, shifted->cursor * shifted->ordering, shifted->policy), ongoing2};
    } else {
        return (struct ReadingResult){NULL, ongoing};
    }
}

struct Reader loop_reader_of(struct Reader ref, matching_policy_t policy, loop_ordering_t ordering, tag_t tag) {
    struct LoopReader* ptr = alloc(ref, policy, ordering, tag);
    ptr->cursor = 0;
    ptr->variant = ref;
    ptr->prev_traces = empty_trace_list();
    return (struct Reader){ptr, &_vtable};
}

RESOLUTION_READER

static _Noreturn const struct TraceList* resolve(const struct ResolutionReader* reader, const struct TraceList* succeeded_trace, const struct TraceList* still_ongoing_trace) {
    abort();
}
