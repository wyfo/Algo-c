#include <stdlib.h>
#include <memory.h>
#include <assert.h>
#include "lexer.h"
#include "parser.h"
#include "switch_reader.h"

static struct TokenList new_token_list() {
    return (struct TokenList){NULL, 0, 0};
}

static void push_token(struct TokenList* token_list, struct Token token) {
    if (token_list->len == token_list->capacity) {
        token_list->capacity = token_list->len ? token_list->len * 2 : 1;
        struct Token* tmp = malloc(token_list->capacity * sizeof(struct Token));
        memcpy(tmp, token_list->start, token_list->len * sizeof(struct Token));
        if (token_list->len) free(token_list->start);
        token_list->start = tmp;
    }
    token_list->start[token_list->len] = token;
    token_list->len++;
}

struct TokenList lexe(const char chars[], size_t nb_chars, struct Reader reader) {
    struct TokenList token_list = new_token_list();
    size_t consumed = 0;
    while (consumed < nb_chars) {
        struct ParsingResult res = parse(&chars[consumed], nb_chars - consumed, reader);
        if (!res.success) {
            free_token_list(token_list);
            printf("consumed = %lu\n", consumed);
            return new_token_list();
        }
        assert(res.success->trace.enum_ == SWITCH);
        char id = res.success->trace.index;
        // TODO make it cleaner
        decr_count(res.success, clean_trace_list);
        if (id != 9) {
            push_token(&token_list, (struct Token){id});
        }
        // tag_t tag = 0;
        // push_token(&token_list, (struct Token){tag, res.success, consumed, consumed + res.success_len, id});
        consumed += res.success_len;
    }
    printf("consumed = %lu\n", consumed);
    return token_list;
}
