#include "utils.h"

uint64_t get_count(const void* ptr) {
    return TO_REF_COUNTED(ptr)->count;
}
