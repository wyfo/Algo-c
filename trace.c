#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "trace.h"
#include "utils.h"

STATIC_RC(empty_trace_list, struct TraceList, {.empty = {} COMMA .next = NULL})
