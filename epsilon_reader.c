#include <assert.h>
#include <stdlib.h>
#include "epsilon_reader.h"

VTABLE

static void _clean(const void* reader) {}

static struct ReadingResult _epsilon(const void* reader) {
    return (struct ReadingResult){empty_trace_list(), NO_READER};
}

static _Noreturn struct ReadingResult _read(const void* reader, char token) {
    abort();
}

STATIC_RC(_epsilon_reader, struct EpsilonReader, {})

struct Reader epsilon_reader() {
    return (struct Reader){_epsilon_reader(), &_vtable};
}
