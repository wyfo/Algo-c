#include <string.h>
#include "json.h"
#include "token_reader.h"
#include "list_reader.h"
#include "loop_reader.h"
#include "epsilon_reader.h"
#include "switch_reader.h"
#include "memo_switch_reader.h"
#include "class_token_reader.h"
#include "ref_reader.h"

struct Reader char_reader(char c) {
    return token_reader_of(c, 0);
}

struct Reader word_reader(const char* word) {
    struct Reader readers[strlen(word)];
    for (size_t i = 0; i < strlen(word); ++i) {
        readers[i] = char_reader(word[i]);
    }
    return list_reader_of(readers, strlen(word), 0);
}

struct Reader opt_reader(struct Reader reader) {
    return switch_reader_of((struct Reader[]){epsilon_reader(), reader}, 2, LONGEST, 0);
}

struct Reader json_lexer() {
    struct Reader LEFT_BRACE = char_reader('{');
    struct Reader RIGHT_BRACE = char_reader('}');
    struct Reader COMMA_ = char_reader(',');
    struct Reader COLON = char_reader(':');
    struct Reader LEFT_BRACKET = char_reader('[');
    struct Reader RIGHT_BRACKET = char_reader(']');
    struct Reader TRUE_ = word_reader("true");
    struct Reader FALSE_ = word_reader("false");
    struct Reader NULL_ = word_reader("null");
    struct Reader WS = class_token_reader_including(256, (char[]){' ', '\t', '\n', '\r'}, 4, 0);
    struct Reader DIGIT = class_token_reader_including(256, (char[]){'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'}, 10, 0);
    struct Reader INT = list_reader_of((struct Reader[]){
        clone_reader(DIGIT),
        loop_reader_of(DIGIT, LONGEST, INCREASING, 0)
    }, 2, 0);
    struct Reader EXP = list_reader_of((struct Reader[]){
        class_token_reader_including(256, (char[]){'e', 'E'}, 2, 0),
        opt_reader(class_token_reader_including(256, (char[]){'-', '+'}, 2, 0)),
        clone_reader(INT)
    }, 3, 0);
    struct Reader NUMBER = list_reader_of((struct Reader[]){
        opt_reader(char_reader('-')),
        clone_reader(INT),
        opt_reader(list_reader_of((struct Reader[]){
            char_reader('.'),
            INT
        }, 2, 0)),
        opt_reader(EXP)
    }, 4, 0);
    struct Reader HEX = class_token_reader_including(256, (char[]){'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'A', 'B', 'C', 'D', 'E', 'F'}, 22, 0);
    struct Reader UNICODE = list_reader_of((struct Reader[]) {
        char_reader('u'),
        clone_reader(HEX), clone_reader(HEX), clone_reader(HEX), HEX
    }, 5, 0);
    struct Reader ESC = list_reader_of((struct Reader[]){
        char_reader('\\'),
        switch_reader_of((struct Reader[]){
            class_token_reader_including(256, (char[]){'\\', '\"', 'n', 't'}, 2, 0),
            UNICODE
        }, 2, LONGEST, 0)
    }, 2, 0);
    struct Reader DOUBLE_QUOTE = char_reader('"');
    struct Reader STRING = list_reader_of((struct Reader[]){
        clone_reader(DOUBLE_QUOTE),
        loop_reader_of(switch_reader_of((struct Reader[]){
            ESC,
            class_token_reader_excluding(256, (char[]){'\\', '"'}, 2, 0)
        }, 2, LONGEST, 0), LONGEST, INCREASING, 0),
        DOUBLE_QUOTE,
    }, 3, 0);
    return memo_switch_reader_of((struct Reader[]){
        LEFT_BRACE,     // 0
        RIGHT_BRACE,    // 1
        COMMA_,         // 2
        COLON,          // 3
        LEFT_BRACKET,   // 4
        RIGHT_BRACKET,  // 5
        TRUE_,          // 6
        FALSE_,         // 7
        NULL_,          // 8
        WS,             // 9
        NUMBER,         // 10
        STRING,         // 11
    }, 12, LONGEST, 0, 256);

}

struct Reader json_parser() {
    struct Reader value = ref_reader();
    struct Reader array = list_reader_of((struct Reader[]){
        char_reader(4),
        opt_reader(list_reader_of((struct Reader[]){
            value,
            loop_reader_of(list_reader_of((struct Reader[]){
                char_reader(2),
                value
            }, 2, 0), LONGEST, INCREASING, 0)}, 2, 0)),
        char_reader(5),
    }, 3, 0);
    struct Reader pair = list_reader_of((struct Reader[]){
        char_reader(11),
        char_reader(3),
        value
    }, 3, 0);
    struct Reader obj = list_reader_of((struct Reader[]){
        char_reader(0),
        opt_reader(list_reader_of((struct Reader[]){
            clone_reader(pair),
            loop_reader_of(list_reader_of((struct Reader[]){
                char_reader(2),
                pair
            }, 2, 0), LONGEST, INCREASING, 0)}, 2, 0)),
        char_reader(1),
    }, 3, 0);
    set_ref(value.self, memo_switch_reader_of((struct Reader[]){
        char_reader(11),
        char_reader(10),
        obj,
        array,
        char_reader(6),
        char_reader(7),
        char_reader(8)
    }, 7, LONGEST, 0, 12));
    return value;
}
