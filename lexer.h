#include <stdlib.h>
#include "reader.h"
#include "tag.h"

struct Token {
    // tag_t tag;
    // const struct TraceList* traces;
    // size_t start;
    // size_t stop;
    char id;
};

struct TokenList {
    struct Token* start;
    size_t capacity;
    size_t len;
};

static inline const struct Token* get_tokens(struct TokenList token_list) {
    return token_list.start;
}

static inline void free_token_list(struct TokenList token_list) {
    if (token_list.start) free((void*)token_list.start);
}

struct TokenList lexe(const char[], size_t nb_tokens, struct Reader reader_);
