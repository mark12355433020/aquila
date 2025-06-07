#ifndef common_h
#define common_h

#define FLAG_NONE 0x0000
#define FLAG_JAILBREAK 0x0001
#define FLAG_USE_UUID 0x0002
#define FLAG_FORCE_INSTALL 0x004
#define FLAG_VERBOSE_LOGGING 0x008
#define FLAG_NO_COLOR 0x0010

#define has_flag(flag) ((global_flags & flag) == flag)

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <spawn.h>
#include <fcntl.h>
#include <getopt.h>
#include <sys/mman.h>
#include <sys/sysctl.h>
#include <sys/syscall.h>
#include <sys/syslog.h>
#include <sys/stat.h>
#include <sys/dirent.h>
#include <sys/dir.h>
#include <sys/socket.h>
#include <mach/mach.h>
#include <mach-o/dyld.h>
#include <mach-o/dyld_images.h>
#include <mach-o/loader.h>
#include <mach-o/fat.h>
#include <mach-o/swap.h>
#include <CoreFoundation/CoreFoundation.h>
#include <getopt.h>
#include <asl.h>

extern struct mach_header_64 _mh_execute_header;
extern uint16_t global_flags;

#endif /* common_h */