#include <assert.h>
#include <stdlib.h>
#include "epsilon_reader.h"

VTABLE(epsilon)

static void epsilon_clean(const void* reader) {}

static struct ReadingResult epsilon_epsilon(const void* reader) {
    return (struct ReadingResult){empty_trace_list(), NO_READER};
}

static _Noreturn struct ReadingResult epsilon_read(const void* reader, char token) {
    abort();
}

STATIC_RC(_epsilon_reader, struct EpsilonReader, {})

struct Reader epsilon_reader() {
    return (struct Reader){_epsilon_reader(), &epsilon_vtable};
}
