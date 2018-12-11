#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define COMMA ,

#define var __auto_type

#define let __auto_type const

typedef uint64_t counter_t; // counter must be large sized to be consistent with padding of embedded struct

#define STATIC_RC(name, type, val) struct { \
    counter_t count; \
    type value; \
} __base_##name = {UINT64_MAX / 2, val}; \
const type* name() { return &__base_##name.value; }

#define STATIC_RC_HEADER(name, type, val) const type* name();

void* ref_counted_alloc(size_t size_of);

void incr_count(const void* ptr);

typedef void (*cleaner_t)(const void*);

void decr_count(const void* ptr, cleaner_t clean_);

void* fake_rc_alloc(size_t size_of);

void fake_rc_free(const void* ptr, cleaner_t clean_);

uint64_t get_count(const void* ptr);

#define CLONE(ptr) ({__auto_type _ptr = ptr; incr_count(_ptr); _ptr;})
