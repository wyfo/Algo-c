#pragma once
#include "reader.h"
#include "tag.h"

struct ClassTokenReader {
    tag_t tag;
    struct ReadingResult results[];
};

struct Reader class_token_reader_including(size_t size, char class_[], size_t class_size, tag_t tag);
struct Reader class_token_reader_excluding(size_t size, char class_[], size_t class_size, tag_t tag);
