#pragma once
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define COMMA ,

#define var __auto_type

#define let __auto_type const

#define MOVED(x) x

#define REF(x) x

typedef uint64_t counter_t; // counter must be large sized to be consistent with padding of embedded struct

extern unsigned int nb_alloc;
extern void* log_;

struct RefCounted {
    uint64_t count;
    char bytes[];
};

#define STATIC_RC(name, type, val) struct { \
    counter_t count; \
    type value; \
} __base_##name = {UINT64_MAX / 2, val}; \
const type* name() { return &__base_##name.value; }

#define STATIC_RC_HEADER(name, type, val) const type* name();

static inline void* ref_counted_alloc_with_count(size_t size_of, counter_t count) {
    struct RefCounted* ptr = malloc(sizeof(struct RefCounted) + size_of);
    assert(ptr);
    nb_alloc++;
    ptr->count = count;
    return &ptr->bytes;
}

static inline void* ref_counted_alloc(size_t size_of) {
    return ref_counted_alloc_with_count(size_of, 1);
}

#define TO_REF_COUNTED(ptr) ((struct RefCounted*)(((char*)(ptr)) - sizeof(counter_t)))

static inline void incr_count(const void* ptr) {
    assert(ptr);
    TO_REF_COUNTED(ptr)->count++;    
}

typedef void (*cleaner_t)(const void*);

static inline void decr_count(const void* ptr, cleaner_t clean_) {
    assert(ptr);
    assert(clean_);
    assert(TO_REF_COUNTED(ptr)->count > 0); 
    if (!--TO_REF_COUNTED(ptr)->count) {
        clean_((void*)ptr);
        free(TO_REF_COUNTED(ptr));
    }
}

static inline void* fake_rc_alloc(size_t size_of) {
    return ref_counted_alloc_with_count(size_of, UINT64_MAX / 2);
}

static inline void fake_rc_free(const void* ptr, cleaner_t clean_) {
    clean_((void*)ptr);
    free(TO_REF_COUNTED(ptr));
}

uint64_t get_count(const void* ptr);

#define CLONE(ptr) ({__auto_type _ptr = ptr; incr_count(_ptr); _ptr;})
