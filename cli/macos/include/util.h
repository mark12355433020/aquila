#ifndef util_h
#define util_h

#include "common.h"

typedef enum {
    INFO = 0,
    ERROR,
    WARNING,
    VERBOSE
} log_type_t;

#define PREFIX_INFO_COLOR "[\x1B[36mINFO\x1B[0m] "
#define PREFIX_ERROR_COLOR "[\x1B[31mERROR\x1B[0m] "
#define PREFIX_WARNING_COLOR "[\x1B[33mWARNING\x1B[0m] "
#define PREFIX_VERBOSE_COLOR "[\x1B[35mVERBOSE\x1B[0m] "
#define PREFIX_INFO "[INFO] "
#define PREFIX_ERROR "[ERROR] "
#define PREFIX_WARNING "[WARNING] "
#define PREFIX_VERBOSE "[VERBOSE] "

void *load_embed_file(const char *name, size_t *size);
void print_log(log_type_t type, const char *format, ...);

#endif /* util_h */