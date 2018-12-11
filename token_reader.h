#pragma once
#include "reader.h"
#include "tag.h"

struct TokenReader {
    char token_ref;
    tag_t tag;
};

struct Reader token_reader_of(char ref, tag_t tag);
