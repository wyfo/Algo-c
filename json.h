#include "reader.h"
#include "ref_reader.h"

struct Reader json_lexer();

struct Reader json_parser(struct RefReaderList* ref_reader_list);
