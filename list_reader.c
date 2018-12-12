#include <assert.h>
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


static _Noreturn const struct TraceList* resolve(const struct ResolutionReader* reader, const struct TraceList* succeeded_trace, const struct TraceList* still_ongoing_trace) {
    abort();
}
