#include <assert.h>
#include <stdlib.h>
#include "token_reader.h"

VTABLE

static void _clean(const void* reader) {}

static struct ReadingResult _epsilon(const void* reader) {
    return (struct ReadingResult){NULL, {CLONE(reader), &_vtable}};
}
static struct ReadingResult _read(const void* reader, char token) {
    const struct TokenReader* self = reader;
    if (token == self->token_ref) return (struct ReadingResult){empty_trace_list(), NO_READER};
    else return FAILED;
}

struct Reader token_reader_of(char ref, tag_t tag) {
    struct TokenReader* ptr = ref_counted_alloc(sizeof(struct TokenReader));
    assert(ptr);
    *ptr = (struct TokenReader){ref, tag};
    return (struct Reader){ptr, &_vtable};
}
