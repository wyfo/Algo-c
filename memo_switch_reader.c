#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>
#include "utils.h"
#include "switch_reader.h"
#include "memo_switch_reader.h"

struct Memoized {
    struct ReadingResult epsilon;
    size_t nb_reads; 
    struct ReadingResult reads[];
};

#define UNINIT_MARKER ((void*)1)

VTABLE(memo_switch)

// #define MEMOIZATION_PTR(ptr) ({ \
//     const struct SwitchReader* _ptr = ptr; \
//     (struct Memoized*)(((char*)_ptr) + sizeof(struct SwitchReader) + _ptr->nb_cases * sizeof(struct SwitchCase)); \
// })

#define SWITCH_READER_PTR(ptr) ({ \
    const struct Memoized* _ptr = ptr; \
    (struct SwitchReader*)((char*)_ptr + sizeof(struct Memoized) + _ptr->nb_reads * sizeof(struct ReadingResult));\
})

static void clean_memo_result(struct ReadingResult res) {
    if (res.success != UNINIT_MARKER) clean_reading_result(res);
}

// static void _clean(const void* reader) {
//     const struct SwitchReader* self = reader;
//     for (size_t i = 0; i < self->nb_cases; ++i) decr_count_reader(self->cases[i].reader);
//     struct Memoized* memo = MEMOIZATION_PTR(reader);
//     clean_memo_result(memo->epsilon);
//     for (size_t i = 0; i < memo->nb_reads; ++i) clean_memo_result(memo->reads[i]);
// }

static void memo_switch_clean(const void* reader) {
    const struct Memoized* self = reader;
    clean_memo_result(self->epsilon);
    for (size_t i = 0; i < self->nb_reads; ++i) clean_memo_result(self->reads[i]);
    const struct SwitchReader* swr = SWITCH_READER_PTR(self);
    for (size_t i = 0; i < swr->nb_cases; ++i) decr_count_reader(swr->cases[i].reader);
}

// static inline struct SwitchReader* alloc(size_t nb_cases, matching_policy_t policy, tag_t tag, size_t nb_tokens) {
//     struct SwitchReader* ptr = ref_counted_alloc(sizeof(struct SwitchReader) + nb_cases * sizeof(struct SwitchCase) + sizeof(struct Memoized) + nb_tokens * sizeof(struct ReadingResult));
//     ptr->nb_cases = nb_cases;
//     struct Memoized* memo = MEMOIZATION_PTR(ptr);
//     memo->nb_reads = nb_tokens;
//     memo->epsilon = (struct ReadingResult){UNINIT_MARKER, NO_READER};
//     for (size_t i = 0; i < nb_tokens; ++i) memo->reads[i] = (struct ReadingResult){UNINIT_MARKER, NO_READER};
//     ptr->policy = policy;
//     ptr->tag = tag;
//     return ptr;
// }

static inline struct Memoized* alloc(size_t nb_cases, matching_policy_t policy, tag_t tag, size_t nb_tokens) {
    struct Memoized* ptr = ref_counted_alloc(sizeof(struct Memoized) + nb_tokens * sizeof(struct ReadingResult) + sizeof(struct SwitchReader) + nb_cases * sizeof(struct SwitchCase));
    ptr->nb_reads = nb_tokens;
    ptr->epsilon = (struct ReadingResult){UNINIT_MARKER, NO_READER};
    for (size_t i = 0; i < nb_tokens; ++i) ptr->reads[i] = (struct ReadingResult){UNINIT_MARKER, NO_READER};
    struct SwitchReader* swr = SWITCH_READER_PTR(ptr);
    swr->policy = policy;
    swr->tag = tag;
    return ptr;
}

#define PROCESS(func, token) \
    const struct Memoized* memo = reader; \
    const struct SwitchReader* self = SWITCH_READER_PTR(reader); \
    assert(self->nb_cases > 0); \
    const struct TraceList* success = NULL; \
    struct Memoized* ongoing = NULL; \
    struct SwitchReader* swr_ongoing = NULL; \
    { \
        size_t i = 0; \
        struct ReadingResult res; \
        for (; i < self->nb_cases; ++i) { \
            res = func(self->cases[i].reader token); \
            if (res.success) { \
                if (!success) success = push_swith_trace(res.success, self->cases[i].index, self->policy); \
                else decr_count(res.success, &clean_trace_list); \
            } \
            if (res.ongoing.self) break; \
        } \
        if (i < self->nb_cases) { \
            ongoing = alloc(self->nb_cases, self->policy, self->tag, memo->nb_reads); \
            swr_ongoing = SWITCH_READER_PTR(ongoing); \
            swr_ongoing->cases[0] = (struct SwitchCase){res.ongoing, self->cases[i].index}; \
            swr_ongoing->nb_cases = 1; \
            i++; \
        } \
        for (; i < self->nb_cases; ++i) { \
            res = func(self->cases[i].reader token); \
            if (res.success) { \
                if (!success) success = push_swith_trace(res.success, self->cases[i].index, self->policy); \
                else decr_count(res.success, &clean_trace_list); \
            } \
            if (res.ongoing.self) { \
                swr_ongoing->cases[swr_ongoing->nb_cases] = (struct SwitchCase){res.ongoing, self->cases[i].index}; \
                swr_ongoing->nb_cases++; \
            } \
        } \
    } \
    return (struct ReadingResult){success, {ongoing, &memo_switch_vtable}}; \

static struct ReadingResult __epsilon(const void* reader) {
    PROCESS(epsilon,)
}

static struct ReadingResult __read(const void* reader, char token) {
    PROCESS(read,COMMA token)
}

static struct ReadingResult memo_switch_epsilon(const void* reader) {
    struct Memoized* self = (void*)reader;
    if (self->epsilon.success == UNINIT_MARKER) {
        self->epsilon = __epsilon(reader);
    }
    return clone_reading_result(self->epsilon);
}

static struct ReadingResult memo_switch_read(const void* reader, char token) {
    struct Memoized* self = (void*)reader;
    size_t index = (unsigned char)token;
    if (self->reads[index].success == UNINIT_MARKER) {
        self->reads[index] = __read(reader, token);
    }
    return clone_reading_result(self->reads[index]);
}


struct Reader memo_switch_reader_of(struct Reader cases[], size_t nb_cases, matching_policy_t policy, tag_t tag, size_t nb_tokens) {
    struct Memoized* ptr = alloc(nb_cases, policy, tag, nb_tokens);
    struct SwitchReader* swr = SWITCH_READER_PTR(ptr);
    swr->nb_cases = nb_cases;
    for (size_t i = 0; i < nb_cases; ++i) swr->cases[i] = (struct SwitchCase){cases[i], i};
    return (struct Reader){ptr, &memo_switch_vtable};
}
