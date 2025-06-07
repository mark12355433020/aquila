#ifndef common_h
#define common_h

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <io.h>
#include <windows.h>

#define FLAG_NONE 0x0000
#define FLAG_JAILBREAK 0x0001
#define FLAG_USE_UUID 0x0002
#define FLAG_FORCE_INSTALL 0x004
#define FLAG_VERBOSE_LOGGING 0x008
#define FLAG_NO_COLOR 0x0010

#define kCFStringEncodingWindowsLatin1 0x0500
#define kCFStringEncodingISOLatin1 0x0201
#define kCFStringEncodingASCII 0x0600
#define kCFStringEncodingUnicode 0x0100
#define kCFStringEncodingUTF8 0x08000100
#define kCFPropertyListXMLFormat_v1_0 0x64

#define LOAD_FLAGS (LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_USER_DIRS)
#define CFSTR(str) (CFStringCreateWithCString(NULL, str, kCFStringEncodingUTF8))
#define has_flag(flag) ((global_flags & flag) == flag)

typedef uint32_t CFPropertyListFormat;
typedef uint32_t CFTypeID;
typedef uint32_t CFStringEncoding;
typedef uint32_t CFIndex;
typedef uint32_t CFRange;
typedef void * CFStringRef;
typedef void * CFPropertyListRef;
typedef void * CFDictionaryRef;
typedef void * CFRunLoopRef;
typedef void * CFTypeRef;
typedef void * CFAllocatorRef;
typedef void * CFMutableDictionaryRef;
typedef void * CFDictionaryKeyCallBacks;
typedef void * CFDictionaryValueCallBacks;
typedef void * CFDataRef;

#pragma pack(push, 1)
typedef struct {
    uint64_t isa;
    uint64_t info;
    uint8_t len;
    char data[];
} __cfstring_t;
#pragma pack(pop)

extern CFTypeRef kCFBooleanTrue;
extern CFTypeRef kCFBooleanFalse;
extern void (*CFRunLoopStop)(CFRunLoopRef rl);
extern void (*CFRunLoopRun)(void);
extern CFRunLoopRef (*CFRunLoopGetCurrent)(void);
extern CFTypeID (*CFStringGetTypeID)(void);
extern CFTypeID (*CFGetTypeID)(CFTypeRef cf);
extern void (*CFRelease)(CFTypeRef cf);
extern CFTypeRef (*CFRetain)(CFTypeRef cf);
extern CFStringRef (*CFStringCreateWithCString)(CFAllocatorRef alloc, const char *cStr, CFStringEncoding encoding);
extern bool (*CFStringGetCString)(CFStringRef theString, char *buffer, CFIndex bufferSize, CFStringEncoding encoding);
extern CFMutableDictionaryRef (*CFDictionaryCreateMutable)(CFAllocatorRef allocator, CFIndex capacity, const CFDictionaryKeyCallBacks *keyCallBacks, const CFDictionaryValueCallBacks *valueCallBacks);
extern void (*CFDictionaryAddValue)(CFMutableDictionaryRef theDict, const void *key, const void *value);
extern const void *(*CFDictionaryGetValue)(CFDictionaryRef theDict, const void *key);
extern bool (*CFEqual)(CFTypeRef cf1, CFTypeRef cf2);
extern CFDataRef (*CFDataCreate)(CFAllocatorRef allocator, const uint8_t* bytes, CFIndex length);
extern void (*CFDictionarySetValue)(CFMutableDictionaryRef theDict, const void * key, const void * value);
extern CFIndex (*CFStringGetLength)(CFStringRef theString);

extern uint16_t global_flags;

#endif /* common_h */