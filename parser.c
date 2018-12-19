#include "parser.h"
#include <stdio.h>

struct ParsingResult parse(const char tokens[], size_t nb_tokens, struct Reader reader_) {
    // printf("start parsing\n"); fflush(stdout);
    struct ReadingResult eps = epsilon(reader_);
    const struct TraceList* success = eps.success;
    struct Reader reader = eps.ongoing;
    size_t success_len = 0;
    size_t nb_tokens_read = 0;
    for (size_t i = 0; i < nb_tokens; ++i) {
        // printf("nb_alloc = %lu\n", nb_alloc);
        if (!reader.self) break;
        char token = tokens[i];
        // printf("%c - %u\n", token, (unsigned char)token); fflush(stdout);
        nb_tokens_read++;
        struct ReadingResult res = read(reader, token);
        decr_count_reader(reader);
        if (res.success) {
            if (success) decr_count(success, clean_trace_list);
            success = res.success;
            success_len = nb_tokens_read;
        }
        reader = res.ongoing;
    }
    if (reader.self) decr_count_reader(reader);
    return (struct ParsingResult){success, success_len, nb_tokens_read};
}
