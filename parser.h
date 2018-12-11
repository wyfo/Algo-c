#include "reader.h"
#include <stdbool.h>

struct ParsingResult {
    const struct TraceList* success;
    size_t success_len;
    size_t nb_tokens_read;
};

static inline bool is_complete(const struct ParsingResult res) {
    return res.success_len == res.nb_tokens_read;
}

struct ParsingResult parse(const char tokens[], size_t nb_tokens, struct Reader reader);
