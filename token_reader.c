#include <assert.h>
#include <stdlib.h>
#include "token_reader.h"

VTABLE(token)

static void token_clean(const void* reader) {}

static struct ReadingResult token_epsilon(const void* reader) {
    return (struct ReadingResult){NULL, {CLONE(reader), &token_vtable}};
}
static struct ReadingResult token_read(const void* reader, char token) {
    const struct TokenReader* self = reader;
    if (token == self->token_ref) return (struct ReadingResult){empty_trace_list(), NO_READER};
    else return FAILED;
}

struct Reader token_reader_of(char ref, tag_t tag) {
    struct TokenReader* ptr = ref_counted_alloc(sizeof(struct TokenReader));
    assert(ptr);
    *ptr = (struct TokenReader){ref, tag};
    return (struct Reader){ptr, &token_vtable};
}
