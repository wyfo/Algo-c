#include <assert.h>
#include <memory.h>
#include "ref_reader.h"

VTABLE(ref)

static void ref_clean(const void* reader) {
    abort();
}

static struct ReadingResult ref_epsilon(const void* reader) {
    const struct RefReader* self = reader;
    assert(self->ref.self);
    return epsilon(self->ref);
}

static _Noreturn struct ReadingResult ref_read(const void* reader, char token) {
    abort();
}


struct Reader ref_reader(struct RefReaderList* list) {
    struct RefReader* ptr = fake_rc_alloc(sizeof(struct RefReader));
    assert(ptr);
    ptr->ref = NO_READER;
    if (list->len == list->capacity) {
        list->capacity = list->len ? list->len * 2 : 1;
        const struct RefReader** tmp = malloc(list->capacity * sizeof(struct RefReader*));
        memcpy(tmp, list->start, list->len * sizeof(const struct RefReader*));
        if (list->len) free(list->start);
        list->start = tmp;
    }
    list->start[list->len] = ptr;
    list->len++;
    return (struct Reader){ptr, &ref_vtable};
}

void set_ref(const void* ref_reader, struct Reader ref) {
    assert(ref.self);
    struct RefReader* self = (void*)ref_reader;
    self->ref = ref;
}

static void clean_ref_reader(const void* reader) {
    const struct RefReader* self = reader;
    if (self->ref.self) decr_count_reader(self->ref);
}

void clean_ref_reader_list(struct RefReaderList readers) {
    for (size_t i = 0; i < readers.len; ++i) fake_rc_free(readers.start[i], &clean_ref_reader);
    free(readers.start);
}
