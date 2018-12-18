#pragma once
#include "reader.h"

struct RefReader {
    struct Reader ref;
};

struct RefReaderList {
    const struct RefReader** start;
    size_t capacity;
    size_t len;
};

static inline struct RefReaderList new_ref_reader_list() {
    return (struct RefReaderList) {NULL, 0, 0};
}

struct Reader ref_reader(struct RefReaderList* list);

void set_ref(const void* ref_reader, struct Reader ref);

void clean_ref_reader_list(struct RefReaderList readers);
