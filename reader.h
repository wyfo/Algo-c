#pragma once
#include <stdlib.h>
#include <assert.h>
#include "trace.h"
#include "utils.h"

struct ReadingResult;

struct Reader {
	const void* self;
	const struct ReaderVTable* vtable;
};

#define NO_READER ((struct Reader){NULL, NULL})

struct ReaderVTable {
	struct ReadingResult (*read)(const void* reader, char token);
	struct ReadingResult (*epsilon)(const void* reader);
	cleaner_t free;
};

struct ReadingResult {
	const struct TraceList* success;
	struct Reader ongoing;
};

#define FAILED ((struct ReadingResult){NULL, NO_READER})

static inline void incr_count_reader(struct Reader reader) {
	assert(reader.self && reader.vtable);
	incr_count(reader.self);
}

static inline void decr_count_reader(struct Reader reader) {
	assert(reader.self && reader.vtable);
	decr_count(reader.self, reader.vtable->free);
}

static inline struct Reader clone_reader(struct Reader reader) {
	assert(reader.self && reader.vtable);
	incr_count(reader.self);
	return reader;
}

static inline struct ReadingResult clone_reading_result(struct ReadingResult res) {
	if (res.success) incr_count(res.success);
	if (res.ongoing.self) incr_count_reader(res.ongoing);
	return res;
}

static inline void clean_reading_result(struct ReadingResult res) {
	if (res.success) decr_count(res.success, clean_trace_list);
	if (res.ongoing.self) decr_count_reader(res.ongoing);
}

static inline struct ReadingResult epsilon(struct Reader reader) {
	assert(reader.self && reader.vtable);
	return reader.vtable->epsilon(reader.self);
}

static inline struct ReadingResult read(struct Reader reader, char token) {
	assert(reader.self && reader.vtable);
	return reader.vtable->read(reader.self, token);
}

#define PREFIXED_VTABLE(prefix) \
	static void prefix##_clean(const void* reader); \
	static struct ReadingResult prefix##_epsilon(const void* reader); \
	static struct ReadingResult prefix##_read(const void* reader, char token); \
	static struct ReaderVTable prefix##_vtable = {&prefix##_read, &prefix##_epsilon, &prefix##_clean};

#define VTABLE PREFIXED_VTABLE()
