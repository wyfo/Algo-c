#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdarg.h>
#include "list_reader.h"
#include "utils.h"
#include "resolution_reader.h"

void clean_reader_list(const void* reader_list) {
    const struct ReaderList* self = reader_list;
    for (size_t i = 0; i < self->size; ++i) {
        decr_count_reader(self->readers[i]);
    }
}

const struct ReaderList* reader_list_of(const struct Reader readers[], size_t size) {
    struct ReaderList* ptr = ref_counted_alloc(sizeof(struct ReaderList) + size * sizeof(struct Reader));
    ptr->size = size;
    for (size_t i = 0; i < size; ++i) {
        ptr->readers[i] = readers[i];
    }
    return ptr;
}

VTABLE(list)
RESOLUTION_READER(list)

static void list_clean(const void* reader) {
    const struct ListReader* self = reader;
    decr_count_reader(self->cur_elt);
    decr_count(self->elts, clean_reader_list);
    decr_count(self->prev_traces, clean_trace_list);
}

static inline struct ListReader* alloc(const struct ReaderList* elts) {
    assert(elts);
    struct ListReader* ptr = ref_counted_alloc(sizeof(struct ListReader));
    ptr->elts = elts;
    return ptr;
}

static inline struct ListReader* replace(const struct ListReader* self, struct Reader ongoing) {
    struct ListReader* ptr = alloc(CLONE(self->elts));
    ptr->cursor = self->cursor;
    ptr->cur_elt = ongoing;
    ptr->prev_traces = CLONE(self->prev_traces);
    return ptr;
}

static inline struct ListReader shift(const struct ListReader* self, const struct TraceList* success) {
    struct ListReader shifted;
    shifted.elts = self->elts;
    shifted.cursor = self->cursor + 1;
    shifted.cur_elt = self->elts->readers[shifted.cursor];
    shifted.prev_traces = push_reversed_trace(CLONE(self->prev_traces), success);
    return shifted;
}

static inline struct ReadingResult process(const struct ListReader* self, struct ReadingResult res) {
    struct Reader ongoing = res.ongoing.self ? (struct Reader){replace(self, res.ongoing), &list_vtable} : NO_READER;
    // TODO maybe NO_READER can be replaced by res.ongoing to improve perf,
    // or maybe i should modify res in place and return it instead.
    if (res.success) {
        if (self->cursor == self->elts->size - 1) {
            const struct TraceList* success = push_reversed_trace(CLONE(self->prev_traces), res.success);
            return (struct ReadingResult){success, ongoing};
        } else {
            struct ListReader shifted = shift(self, res.success);
            struct ReadingResult forward_epsilon = list_epsilon(&shifted);
            struct Reader forward_ongoing = resolution_reader_of(&resolution_list_vtable, forward_epsilon.ongoing, ongoing, res.success, self->cursor);
            decr_count(shifted.prev_traces, clean_trace_list);
            return (struct ReadingResult){forward_epsilon.success, forward_ongoing};
        }
    } else {
        return (struct ReadingResult){NULL, ongoing};
    }
}

static struct ReadingResult list_epsilon(const void* reader) {
    const struct ListReader* self = reader;
    return process(self, epsilon(self->cur_elt));
}
static struct ReadingResult list_read(const void* reader, char token) {
    const struct ListReader* self = reader;
    return process(self, read(self->cur_elt, token));
}

struct Reader list_reader_of(const struct Reader elts[], size_t nb_elts, tag_t tag) {
    struct ListReader* ptr = alloc(reader_list_of(elts, nb_elts));
    ptr->cursor = 0;
    ptr->cur_elt = clone_reader(elts[0]);
    ptr->prev_traces = empty_trace_list();
    ptr->tag = tag;
    return (struct Reader){ptr, &list_vtable};
}

IMPURE_VTABLE(list)
IMPURE_RESOLUTION_READER(list)

static inline struct ReadingResult impure_process(struct ListReader* self, struct ReadingResult res, bool need_incr) {
    if (res.ongoing.self) {
        if (need_incr) incr_count(self);
        decr_count_reader(self->cur_elt);
        self->cur_elt = res.ongoing;
        if (res.success) {
            if (self->cursor == self->elts->size - 1) {
                const struct TraceList* success = push_reversed_trace(CLONE(self->prev_traces), res.success);
                return (struct ReadingResult){success, {self, &impure_list_vtable}};
            } else {
                struct ListReader* shifted = alloc(CLONE(self->elts));
                shifted->prev_traces = push_reversed_trace(CLONE(self->prev_traces), CLONE(res.success));
                shifted->cursor = self->cursor + 1;
                shifted->cur_elt = clone_reader(shifted->elts->readers[shifted->cursor]);
                struct ReadingResult forward_epsilon = impure_process(shifted, epsilon(shifted->cur_elt), false);
                struct ResolutionReader* reso = ref_counted_alloc(sizeof(struct ResolutionReader));
                reso->succeeded = forward_epsilon.ongoing;
                reso->still_ongoing = (struct Reader){self, &impure_list_vtable};
                reso->success_trace = res.success;
                reso->cursor = self->cursor;
                assert(reso->succeeded.vtable == &impure_list_vtable || reso->succeeded.vtable == &impure_resolution_list_vtable); \
                assert(reso->still_ongoing.vtable == &impure_list_vtable || reso->still_ongoing.vtable == &impure_resolution_list_vtable); \
                struct Reader forward_ongoing = (struct Reader){reso, &impure_resolution_list_vtable};
                return (struct ReadingResult){forward_epsilon.success, forward_ongoing};

            }
        } else {
            return (struct ReadingResult){NULL, {self, &impure_list_vtable}};
        }
    } else if (res.success) {
        if (self->cursor == self->elts->size - 1) {
            const struct TraceList* success = push_reversed_trace(CLONE(self->prev_traces), res.success);
            return (struct ReadingResult){success, NO_READER};
        } else {
            if (need_incr) incr_count(self);
            self->cursor++;
            decr_count_reader(self->cur_elt);
            self->cur_elt = clone_reader(self->elts->readers[self->cursor]);
            self->prev_traces = push_reversed_trace(self->prev_traces, res.success);
            return impure_process(self, epsilon(self->cur_elt), false);
        }
    } else {
        return res;
    } 
}
static struct ReadingResult impure_list_read(const void* reader, char token) {
    struct ListReader* self = reader;
    return impure_process(self, read(self->cur_elt, token), true);
}

static struct ReadingResult impure_list_epsilon(const void* reader) {
    const struct ListReader* self = reader;
    struct ListReader* tmp = alloc(CLONE(self->elts));
    tmp->prev_traces = CLONE(self->prev_traces);
    tmp->cursor = self->cursor;
    tmp->cur_elt = clone_reader(self->cur_elt);
    return impure_process(tmp, epsilon(tmp->cur_elt), false);
}

struct Reader impure_list_reader_of(const struct Reader elts[], size_t nb_elts, tag_t tag) {
    struct ListReader* ptr = alloc(reader_list_of(elts, nb_elts));
    ptr->cursor = 0;
    ptr->cur_elt = clone_reader(elts[0]);
    ptr->prev_traces = empty_trace_list();
    ptr->tag = tag;
    return (struct Reader){ptr, &impure_list_vtable};
}


static _Noreturn const struct TraceList* resolve(const struct ResolutionReader* reader, const struct TraceList* succeeded_trace, const struct TraceList* still_ongoing_trace) {
    abort();
}
