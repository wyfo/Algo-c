#include <assert.h>
#include "ref_reader.h"

VTABLE

static void _clean(const void* reader) {
    const struct RefReader* self = reader;
    assert(self->ref.self);
    decr_count_reader(self->ref);
}

static struct ReadingResult _epsilon(const void* reader) {
    const struct RefReader* self = reader;
    assert(self->ref.self);
    return epsilon(self->ref);
}

static _Noreturn struct ReadingResult _read(const void* reader, char token) {
    abort();
}

struct Reader ref_reader() {
    struct RefReader* ptr = fake_rc_alloc(sizeof(struct RefReader));
    ptr->ref = NO_READER;
    return (struct Reader){ptr, &_vtable};
}

void set_ref(const void* ref_reader, struct Reader ref) {
    assert(ref.self);
    struct RefReader* self = (void*)ref_reader;
    self->ref = ref;
}
