#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "reader.h"
#include "token_reader.h"
#include "epsilon_reader.h"
#include "class_token_reader.h"
#include "list_reader.h"
#include "switch_reader.h"
#include "memo_switch_reader.h"
#include "loop_reader.h"
#include "class_token_reader.h"
#include "utils.h"
#include "parser.h"
#include "lexer.h"
#include "json.h"
#include "time.h"

void print_ptr(const void* ptr) {
	if (!ptr) printf("NULL\n");
	else printf("%p\n", ptr);
}

void sep() {
	printf("=-=-=-=-=-=-=-=-=-=-=-=-=\n");
}

void test()
{
	struct ReadingResult res; 
	struct Reader r1 = token_reader_of('a', 0);
	struct Reader r2 = epsilon_reader();
	struct Reader r3 = list_reader_of((struct Reader[]){clone_reader(r1), clone_reader(r2)}, 2, 0);
	struct Reader r4 = list_reader_of((struct Reader[]){clone_reader(r1), clone_reader(r1)}, 2, 0);
	struct Reader r5 = switch_reader_of((struct Reader[]){clone_reader(r1), clone_reader(r2)}, 2, LONGEST, 0);
	struct Reader r6 = switch_reader_of((struct Reader[]){clone_reader(r1), clone_reader(r1)}, 2, LONGEST, 0);
	struct Reader r7 = loop_reader_of(clone_reader(r1), LONGEST, INCREASING, 0);
	struct Reader r8 = class_token_reader_including(256, (char[]){'a', 'b'}, 2, 0);
	struct Reader r9 = memo_switch_reader_of((struct Reader[]){clone_reader(r1), clone_reader(r2)}, 2, LONGEST, 0, 256);
	struct Reader r10 = memo_switch_reader_of((struct Reader[]){clone_reader(r1), clone_reader(r1)}, 2, LONGEST, 0, 256);

	res = epsilon(r1);
	assert(!res.success);
	assert(res.ongoing.self);
	clean_reading_result(res);

	res = read(r1, 'a');
	assert(res.success);
	assert(!res.ongoing.self);
	clean_reading_result(res);

	res = epsilon(r2);
	assert(res.success);
	assert(!res.ongoing.self);
	clean_reading_result(res);

	res = epsilon(r3);
	assert(!res.success);
	assert(res.ongoing.self);
	clean_reading_result(res);

	res = read(r3, 'a');
	assert(res.success);
	assert(!res.ongoing.self);
	clean_reading_result(res);

	res = read(r4, 'a');
	assert(!res.success);
	assert(res.ongoing.self);
	clean_reading_result(res);

	res = epsilon(r5);
	assert(res.success);
	assert(res.ongoing.self);
	clean_reading_result(res);

	res = read(r6, 'a');
	assert(res.success);
	assert(!res.ongoing.self);
	clean_reading_result(res);

	res = epsilon(r7);
	assert(res.success);
	assert(res.ongoing.self);
	clean_reading_result(res);

	res = read(r7, 'a');
	assert(res.success);
	assert(res.ongoing.self);
	clean_reading_result(res);

	res = epsilon(r8);
	assert(!res.success);
	assert(res.ongoing.self);
	clean_reading_result(res);

	res = read(r8, 'a');
	assert(res.success);
	assert(!res.ongoing.self);
	clean_reading_result(res);

	res = read(r8, 'b');
	assert(res.success);
	assert(!res.ongoing.self);
	clean_reading_result(res);

	res = read(r8, 'c');
	assert(!res.success);
	assert(!res.ongoing.self);
	clean_reading_result(res);

	res = epsilon(r9);
	assert(res.success);
	assert(res.ongoing.self);
	clean_reading_result(res);
	res = epsilon(r9);
	assert(res.success);
	assert(res.ongoing.self);
	clean_reading_result(res);

	res = read(r10, 'a');
	assert(res.success);
	assert(!res.ongoing.self);
	clean_reading_result(res);
	res = read(r10, 'a');
	assert(res.success);
	assert(!res.ongoing.self);
	clean_reading_result(res);

	decr_count_reader(r10);
	decr_count_reader(r9);
	decr_count_reader(r8);
	decr_count_reader(r7);
	decr_count_reader(r6);
	decr_count_reader(r5);
	decr_count_reader(r4);
	decr_count_reader(r3);
	decr_count_reader(r2);
	decr_count_reader(r1);

	// struct Reader reader = list_reader_of((struct Reader[]){token_reader_of('a', 0), token_reader_of('b', 0)}, 2, 0);
	// struct ParsingResult pres = parse("ab", 2, reader);
	// struct Reader reader = loop_reader_of(token_reader_of('a', 0), LONGEST, INCREASING, 0);
	// struct ParsingResult pres = parse("aa", 2, reader);
	// struct Reader reader = list_reader_of((struct Reader[]){switch_reader_of((struct Reader[]){token_reader_of('a', 0), epsilon_reader()}, 2, LONGEST, 0), token_reader_of('b', 0)}, 2, 0) ;
	// struct ParsingResult pres = parse("b", 1, reader);
	// assert(is_complete(pres));
	// assert(pres.success);
	// decr_count(pres.success, clean_trace_list);
	// decr_count_reader(reader);
}

size_t file_to_string(const char* file, char** string) {
	FILE* fh = fopen(file, "rb");
	if (!fh) return 0;
	fseek(fh, 0L, SEEK_END);
    size_t len = ftell(fh);
    rewind(fh);
	*string = malloc(len);
	if (!*string) return 0;
	size_t read = fread(*string, len, 1, fh);
	assert(read == 1);
	fclose(fh);
	return len;
}

static inline void print_time(clock_t start, clock_t end, const char* msg) {
	printf("%s = %f\n", msg, ((double)end - start) / CLOCKS_PER_SEC);
}

int main(int argc, char const *argv[]) {
	// test(); return 0;
	// const char* json = "{ \"plop\": null }";
	// size_t json_size = strlen(json);
	clock_t t0 = clock();
	char* json;
	size_t json_size = file_to_string("Foods.json", &json);
	if (!json) return 1;
	clock_t t1 = clock();
	struct RefReaderList ref_reader_list = new_ref_reader_list();
	struct Reader lexer = json_lexer();
	struct Reader parser = json_parser(&ref_reader_list);
	clock_t t2 = clock();
	struct TokenList tokens = lexe(json, json_size, lexer);
	printf("Nb tokens read = %lu\n", tokens.len);
	fflush(stdout);
	clock_t t3 = clock();
	struct ParsingResult res = parse((const char*)get_tokens(tokens), tokens.len, parser);
	if (res.success) printf("Hello world!\n");
	else printf("Nb tokens consumed = %lu\n", res.nb_tokens_read);
	fflush(stdout);
	clock_t t4 = clock();
	assert(res.success);
	if (res.success) decr_count(res.success, clean_trace_list);
	free_token_list(tokens);
	decr_count_reader(parser);
	decr_count_reader(lexer);
	clean_ref_reader_list(ref_reader_list);
	clock_t t5 = clock();
	print_time(t0, t1, "file buffering");
	print_time(t1, t2, "allocating readers");
	print_time(t2, t3, "lexing");
	print_time(t3, t4, "parsing");
	print_time(t4, t5, "freeing readers");
	print_time(t2, t4, "lexing + parsing");
	print_time(t1, t5, "time");
	free((void*)json);
	return 0;
}
