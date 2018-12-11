#include "reader.h"

struct RefReader {
    struct Reader ref;
};

struct Reader ref_reader();

void set_ref(const void* ref_reader, struct Reader ref);
