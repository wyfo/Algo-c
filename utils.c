#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include "utils.h"

struct RefCounted {
    uint64_t count;
    char bytes[];
};

void* ref_counted_alloc_with_count(size_t size_of, counter_t count) {
    struct RefCounted* ptr = malloc(sizeof(struct RefCounted) + size_of);
    assert(ptr);
    ptr->count = count;
    return &ptr->bytes;
}

void* ref_counted_alloc(size_t size_of) {
    return ref_counted_alloc_with_count(size_of, 1);
}

#define TO_REF_COUNTED(ptr) ((struct RefCounted*)(((char*)(ptr)) - sizeof(counter_t)))

void incr_count(const void* ptr) {
    assert(ptr);
    TO_REF_COUNTED(ptr)->count++;    
}

void decr_count(const void* ptr, cleaner_t clean_) {
    assert(ptr);
    assert(clean_);
    if (!--TO_REF_COUNTED(ptr)->count) {
        clean_((void*)ptr);
        free(TO_REF_COUNTED(ptr));
    }
}

void* fake_rc_alloc(size_t size_of) {
    return ref_counted_alloc_with_count(size_of, UINT64_MAX / 2);
}

void fake_rc_free(const void* ptr, cleaner_t clean_) {
    clean_((void*)ptr);
    free(TO_REF_COUNTED(ptr));
}

uint64_t get_count(const void* ptr) {
    return TO_REF_COUNTED(ptr)->count;
}
