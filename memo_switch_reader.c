#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <stdbool.h>
#include "utils.h"
#include "switch_reader.h"
#include "memo_switch_reader.h"

struct Memoized {
    struct ReadingResult epsilon;
    unsigned char flags[32];
    bool eps_flag;
    size_t nb_reads; 
    struct ReadingResult reads[];
};

static inline bool is_init(const struct Memoized* self, unsigned char token) {
    return self->flags[token / 8] & (1 << (token % 8));
}

VTABLE(memo_switch)

#define SWITCH_READER_PTR(ptr) ({ \
    const struct Memoized* _ptr = ptr; \
    (struct SwitchReader*)((char*)_ptr + sizeof(struct Memoized) + _ptr->nb_reads * sizeof(struct ReadingResult));\
})

static inline void clean_memoized_result(struct ReadingResult res) {
    if (res.success) decr_count(res.success, clean_trace_list);
    if (res.ongoing.self) fake_rc_free(res.ongoing.self, memo_switch_clean);
}

static void memo_switch_clean(const void* reader) {
    const struct Memoized* self = reader;
    const struct SwitchReader* swr = SWITCH_READER_PTR(self);
    for (size_t i = 0; i < swr->nb_cases; ++i) decr_count_reader(swr->cases[i].reader);
    if (self->eps_flag) clean_memoized_result(self->epsilon);
    for (size_t i = 0; i < self->nb_reads; ++i) if (is_init(self, i)) clean_memoized_result(self->reads[i]);
}

static inline struct Memoized* alloc(size_t nb_cases, matching_policy_t policy, tag_t tag, size_t nb_tokens) {
    struct Memoized* ptr = ref_counted_alloc(sizeof(struct Memoized) + nb_tokens * sizeof(struct ReadingResult) + sizeof(struct SwitchReader) + nb_cases * sizeof(struct SwitchCase));
    ptr->nb_reads = nb_tokens;
    memset(ptr->flags, 0, 32);
    ptr->eps_flag = false;
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
    if (!self->eps_flag) {
        self->eps_flag = true;
        self->epsilon = __epsilon(reader);
    }
    return clone_reading_result(self->epsilon);
}

static struct ReadingResult memo_switch_read(const void* reader, char token) {
    struct Memoized* self = (void*)reader;
    size_t index = (unsigned char)token;
    if (!is_init(self, index)) {
        self->flags[index / 8] |= 1 << index % 8;
        assert(is_init(self, index));
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
