#include <assert.h>
#include <stdlib.h>
#include "class_token_reader.h"

VTABLE

static void _clean(const void* reader) {}

static struct ReadingResult _epsilon(const void* reader) {
    return (struct ReadingResult){NULL, {CLONE(reader), &_vtable}};
}
static struct ReadingResult _read(const void* reader, char token) {
    const struct ClassTokenReader* self = reader;
    size_t index = (unsigned char)token;
    if (self->results[index].success) incr_count(self->results[index].success);
    return self->results[index];
}

#define SUCCESS ((struct ReadingResult){empty_trace_list(), NO_READER})

static inline struct Reader class_token_reader_of(size_t size, char class_[], size_t class_size, tag_t tag, struct ReadingResult class_value, struct ReadingResult default_) {
    struct ClassTokenReader* ptr = ref_counted_alloc(sizeof(struct ClassTokenReader) + size * sizeof(struct ReadingResult));
    assert(ptr);
    ptr->tag = tag;
    for (size_t i = 0; i < size; ++i) ptr->results[i] = default_;
    for (size_t i = 0; i < class_size; ++i) ptr->results[(unsigned char)class_[i]] = class_value;
    return (struct Reader){ptr, &_vtable};
}

struct Reader class_token_reader_including(size_t size, char class_[], size_t class_size, tag_t tag) {
    return class_token_reader_of(size, class_, class_size, tag, SUCCESS, FAILED);
}

struct Reader class_token_reader_excluding(size_t size, char class_[], size_t class_size, tag_t tag) {
    return class_token_reader_of(size, class_, class_size, tag, FAILED, SUCCESS);
}
