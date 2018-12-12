#include <assert.h>
#include "ref_reader.h"

VTABLE(ref)

static void ref_clean(const void* reader) {
    const struct RefReader* self = reader;
    assert(self->ref.self);
    decr_count_reader(self->ref);
}

static struct ReadingResult ref_epsilon(const void* reader) {
    const struct RefReader* self = reader;
    assert(self->ref.self);
    return epsilon(self->ref);
}

static _Noreturn struct ReadingResult ref_read(const void* reader, char token) {
    abort();
}

struct Reader ref_reader() {
    struct RefReader* ptr = fake_rc_alloc(sizeof(struct RefReader));
    ptr->ref = NO_READER;
    return (struct Reader){ptr, &ref_vtable};
}

void set_ref(const void* ref_reader, struct Reader ref) {
    assert(ref.self);
    struct RefReader* self = (void*)ref_reader;
    self->ref = ref;
}
