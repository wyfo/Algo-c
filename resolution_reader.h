#pragma once
#include "reader.h"
#include "utils.h"

struct ResolutionReader {
    struct Reader succeeded;
    struct Reader still_ongoing;
    const struct TraceList* success_trace;
    size_t cursor;
};

static inline struct Reader resolution_reader_of(const struct ReaderVTable* vtable, struct Reader succeeded, struct Reader still_ongoing, const struct TraceList* success_trace, size_t cursor) {
    if (succeeded.self) {
        if (still_ongoing.self) {
            struct ResolutionReader* ptr = ref_counted_alloc(sizeof(struct ResolutionReader));
            ptr->succeeded = succeeded;
            ptr->still_ongoing = still_ongoing;
            ptr->success_trace = CLONE(success_trace);
            ptr->cursor = cursor;
            return (struct Reader){ptr, vtable};
        } else {
            return succeeded;
        }
    } else {
        if (still_ongoing.self) {
            return still_ongoing;
        } else {
            return NO_READER;
        }
    }
}

typedef const struct TraceList* (*resolve_t)(const struct ResolutionReader* reader, const struct TraceList* succeeded_trace, const struct TraceList* still_ongoing_trace);

static inline const struct TraceList* resolve_success(const struct ResolutionReader* reader, const struct TraceList* succeeded_trace, const struct TraceList* still_ongoing_trace, resolve_t resolve) {
    if (succeeded_trace) {
        if (still_ongoing_trace) {
            return resolve(reader, succeeded_trace, still_ongoing_trace);
        } else {
            return succeeded_trace;
        }
    } else {
        if (still_ongoing_trace) {
            return still_ongoing_trace;
        } else {
            return NULL;
        }
    }
}

static inline struct ReadingResult read_and_resolve(const struct ReaderVTable* vtable, const void* reader, char token, resolve_t resolve) {
    const struct ResolutionReader* self = reader;
    struct ReadingResult succeeded_res = read(self->succeeded, token);
    struct ReadingResult still_ongoing_res = read(self->still_ongoing, token);
    struct Reader ongoing = resolution_reader_of(vtable, succeeded_res.ongoing, still_ongoing_res.ongoing, self->success_trace, self->cursor);
    const struct TraceList* success = resolve_success(self, succeeded_res.success, still_ongoing_res.success, resolve);
    return (struct ReadingResult){success, ongoing};
}

#define RESOLUTION_READER(prefix) \
    VTABLE(resolution_##prefix)\
    static _Noreturn const struct TraceList* resolve(const struct ResolutionReader* reader, const struct TraceList* succeeded_trace, const struct TraceList* still_ongoing_trace); \
    \
    static inline void resolution_##prefix##_clean(const void* reader) { \
        const struct ResolutionReader* self = reader; \
        decr_count_reader(self->succeeded); \
        decr_count_reader(self->still_ongoing); \
        decr_count(self->success_trace, &clean_trace_list); \
    } \
    \
    static inline _Noreturn struct ReadingResult resolution_##prefix##_epsilon(const void* reader) { \
        abort(); \
    } \
    \
    static inline struct ReadingResult resolution_##prefix##_read(const void* reader, char token) { \
        return read_and_resolve(&resolution_##prefix##_vtable, reader, token, &resolve); \
    }
