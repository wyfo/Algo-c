#pragma once
#include "reader.h"
#include "tag.h"


struct SwitchReader {
    size_t nb_cases;
    matching_policy_t policy;
    tag_t tag;
    struct SwitchCase {
        struct Reader reader;
        int index;
    } cases[];
};

struct Reader switch_reader_of(struct Reader cases[], size_t nb_cases, matching_policy_t policy, tag_t tag);

struct Reader impure_switch_reader_of(struct Reader cases[], size_t nb_cases, matching_policy_t policy, tag_t tag);
