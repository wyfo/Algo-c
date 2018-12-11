#pragma once
#include "reader.h"
#include "tag.h"


struct ListReader {
    const struct TraceList* prev_traces;
    const struct ReaderList* elts;
    struct Reader cur_elt;
    size_t cursor;
    tag_t tag;
};

struct ReaderList {
    size_t size;
    struct Reader readers[];
};

void clean_reader_list(const void* reader_list);

struct Reader list_reader_of(const struct Reader elts[], size_t nb_elts, tag_t tag);
