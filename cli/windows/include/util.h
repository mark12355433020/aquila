#ifndef util_h
#define util_h

#include "common.h"

#define INFO 0
#define ERROR 1
#define WARNING 2
#define VERBOSE 3

#define PREFIX_INFO_COLOR "[\x1B[36mINFO\x1B[0m] "
#define PREFIX_ERROR_COLOR "[\x1B[31mERROR\x1B[0m] "
#define PREFIX_WARNING_COLOR "[\x1B[33mWARNING\x1B[0m] "
#define PREFIX_VERBOSE_COLOR "[\x1B[35mVERBOSE\x1B[0m] "
#define PREFIX_INFO "[INFO] "
#define PREFIX_ERROR "[ERROR] "
#define PREFIX_WARNING "[WARNING] "
#define PREFIX_VERBOSE "[VERBOSE] "

void usleep(int16_t usec);
char *cfstr_to_cstr(CFStringRef str);
CFStringRef cstr_to_cfstr(const char *str);
char *get_mobile_device_path(void);
int platform_init(void);
void *load_embed_file(const char *name, size_t *size);
void print_log(uint32_t type, const char *format, ...);

#endif /* util_h */