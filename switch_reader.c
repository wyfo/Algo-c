#include <assert.h>
#include <stdlib.h>
#include "utils.h"
#include "switch_reader.h"

VTABLE(switch)

static void switch_clean(const void* reader) {
    const struct SwitchReader* self = reader;
    for (size_t i = 0; i < self->nb_cases; ++i) decr_count_reader(self->cases[i].reader);
}

static inline struct SwitchReader* alloc(size_t nb_cases, matching_policy_t policy, tag_t tag) {
    struct SwitchReader* ptr = ref_counted_alloc(sizeof(struct SwitchReader) + nb_cases * sizeof(struct SwitchCase));
    ptr->policy = policy;
    ptr->tag = tag;
    return ptr;
}

#define PROCESS(func, token) \
    const struct SwitchReader* self = reader; \
    assert(self->nb_cases > 0); \
    const struct TraceList* success = NULL; \
    struct SwitchReader* ongoing = NULL; \
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
            ongoing = alloc(self->nb_cases, self->policy, self->tag); \
            ongoing->cases[0] = (struct SwitchCase){res.ongoing, self->cases[i].index}; \
            ongoing->nb_cases = 1; \
            i++; \
        } \
        for (; i < self->nb_cases; ++i) { \
            res = func(self->cases[i].reader token); \
            if (res.success) { \
                if (!success) success = push_swith_trace(res.success, self->cases[i].index, self->policy); \
                else decr_count(res.success, &clean_trace_list); \
            } \
            if (res.ongoing.self) { \
                ongoing->cases[ongoing->nb_cases] = (struct SwitchCase){res.ongoing, self->cases[i].index}; \
                ongoing->nb_cases++; \
            } \
        } \
    } \
    return (struct ReadingResult){success, {ongoing, &switch_vtable}}; \

static struct ReadingResult switch_epsilon(const void* reader) {
    PROCESS(epsilon,)
}

static struct ReadingResult switch_read(const void* reader, char token) {
    PROCESS(read,COMMA token)
}

struct Reader switch_reader_of(struct Reader cases[], size_t nb_cases, matching_policy_t policy, tag_t tag) {
    struct SwitchReader* ptr = alloc(nb_cases, policy, tag);
    ptr->nb_cases = nb_cases;
    for (size_t i = 0; i < nb_cases; ++i) ptr->cases[i] = (struct SwitchCase){cases[i], i};
    return (struct Reader){ptr, &switch_vtable};
}
