#pragma once
#include "reader.h"
#include "tag.h"

struct Reader memo_switch_reader_of(struct Reader cases[], size_t nb_cases, matching_policy_t policy, tag_t tag, size_t nb_tokens);

extern struct ReaderVTable* memo_vtable;

void memo_clean(const void* reader);
