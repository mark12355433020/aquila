#include "device.h"
#include "afc.h"
#include "util.h"

static HMODULE cf_handle = NULL;
static HMODULE md_handle = NULL;

CFTypeRef kCFBooleanTrue = NULL;
CFTypeRef kCFBooleanFalse = NULL;
void (*CFRunLoopStop)(CFRunLoopRef rl) = NULL;
void (*CFRunLoopRun)(void) = NULL;
CFRunLoopRef (*CFRunLoopGetCurrent)(void) = NULL;
CFTypeID (*CFStringGetTypeID)(void) = NULL;
CFTypeID (*CFGetTypeID)(CFTypeRef cf) = NULL;
void (*CFRelease)(CFTypeRef cf) = NULL;
CFTypeRef (*CFRetain)(CFTypeRef cf) = NULL;
CFStringRef (*CFStringCreateWithCString)(CFAllocatorRef alloc, const char *cStr, CFStringEncoding encoding) = NULL;
bool (*CFStringGetCString)(CFStringRef theString, char *buffer, CFIndex bufferSize, CFStringEncoding encoding) = NULL;
CFMutableDictionaryRef (*CFDictionaryCreateMutable)(CFAllocatorRef allocator, CFIndex capacity, const CFDictionaryKeyCallBacks *keyCallBacks, const CFDictionaryValueCallBacks *valueCallBacks) = NULL;
void (*CFDictionaryAddValue)(CFMutableDictionaryRef theDict, const void *key, const void *value) = NULL;
const void *(*CFDictionaryGetValue)(CFDictionaryRef theDict, const void *key) = NULL;
bool (*CFEqual)(CFTypeRef cf1, CFTypeRef cf2) = NULL;
CFDataRef (*CFDataCreate)(CFAllocatorRef allocator, const uint8_t *bytes, CFIndex length) = NULL;
void (*CFDictionarySetValue)(CFMutableDictionaryRef theDict, const void *key, const void *value) = NULL;
extern CFIndex (*CFStringGetLength)(CFStringRef theString) = NULL;

static int load_symbol(const char *name, void **ptr) {
    void *symbol = (void *)GetProcAddress(cf_handle, name);
    if (symbol == NULL) symbol = (void *)GetProcAddress(md_handle, name);
    if (symbol == NULL) return -1;

    *ptr = symbol;
    return 0;
}

void usleep(int16_t usec) { 
    LARGE_INTEGER due = {0};
    due.QuadPart = -(10 * usec);

    HANDLE timer = CreateWaitableTimer(NULL, TRUE, NULL); 
    SetWaitableTimer(timer, &due, 0, NULL, NULL, 0); 
    WaitForSingleObject(timer, INFINITE); 
    CloseHandle(timer); 
}

char *cfstr_to_cstr(CFStringRef str) {
    if (str == NULL) return NULL;
    __cfstring_t *str_data = (__cfstring_t *)str;
    if (str_data->isa == 0 || str_data->len == 0) return NULL;

    char *buf = calloc(1, str_data->len+1);
    if (buf == NULL) return NULL;

    memcpy(buf, str_data->data, str_data->len);
    return buf;
}

CFStringRef cstr_to_cfstr(const char *str) {
    if (str == NULL) return NULL;
    CFStringRef cf_str = CFStringCreateWithCString(NULL, str, kCFStringEncodingASCII);

    if (cf_str == NULL) return NULL;
    CFRetain(cf_str);
    return cf_str;
}

char *get_mobile_device_path(void) {
    HKEY key = NULL;
    const char *key_name = "SOFTWARE\\Apple Inc.\\Apple Mobile Device Support";
    const char *entry_name = "InstallDir";

    char value[MAX_PATH] = {0};
    DWORD size = sizeof(value);
    DWORD type = 0;

    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, key_name, 0, KEY_READ, &key) != ERROR_SUCCESS) goto fallback;
    if (RegQueryValueExA(key, entry_name, NULL, &type, (LPBYTE)&value, &size) != ERROR_SUCCESS) {
        RegCloseKey(key);
        goto fallback;
    }

    RegCloseKey(key);
    if (type != REG_SZ) goto fallback;

    if (value[strlen(value)-1] == '\\') {
        value[strlen(value)-1] = '\0';
    }
    return _strdup(value);

fallback:
    if (_access("C:\\Program Files\\Common Files\\Apple\\Mobile Device Support", 0) == 0) {
        return _strdup("C:\\Program Files\\Common Files\\Apple\\Mobile Device Support");
    }
    return NULL;
}

int platform_init(void) {
    char *md_path = get_mobile_device_path();
    if (md_path == NULL) return -1;

    wchar_t wide_path[MAX_PATH] = {0};
    MultiByteToWideChar(CP_ACP, 0, md_path, -1, wide_path, MAX_PATH);
    free(md_path);
    AddDllDirectory(wide_path);

    if (!SetDefaultDllDirectories(LOAD_FLAGS)) return -1;
    if ((cf_handle = LoadLibraryExA("CoreFoundation.dll", NULL, LOAD_FLAGS)) == NULL) return -1;
    if ((md_handle = LoadLibraryExA("MobileDevice.dll", NULL, LOAD_FLAGS)) == NULL) return -1;
    
    if (load_symbol("AMDeviceNotificationSubscribe", (void **)&AMDeviceNotificationSubscribe) != 0) return -1;
    if (load_symbol("AMDeviceNotificationUnsubscribe", (void **)&AMDeviceNotificationUnsubscribe) != 0) return -1;
    if (load_symbol("AMDeviceConnect", (void **)&AMDeviceConnect) != 0) return -1;
    if (load_symbol("AMDeviceIsPaired", (void **)&AMDeviceIsPaired) != 0) return -1;
    if (load_symbol("AMDevicePair", (void **)&AMDevicePair) != 0) return -1;
    if (load_symbol("AMDeviceValidatePairing", (void **)&AMDeviceValidatePairing) != 0) return -1;
    if (load_symbol("AMDeviceStartSession", (void **)&AMDeviceStartSession) != 0) return -1;
    if (load_symbol("AMDeviceCopyValue", (void **)&AMDeviceCopyValue) != 0) return -1;
    if (load_symbol("AMDeviceStartService", (void **)&AMDeviceStartService) != 0) return -1;
    if (load_symbol("AMDeviceStartServiceWithOptions", (void **)&AMDeviceStartServiceWithOptions) != 0) return -1;
    if (load_symbol("AMDeviceStopSession", (void **)&AMDeviceStopSession) != 0) return -1;
    if (load_symbol("USBMuxConnectByPort", (void **)&USBMuxConnectByPort) != 0) return -1;
    if (load_symbol("AMDeviceGetConnectionID", (void **)&AMDeviceGetConnectionID) != 0) return -1;
    if (load_symbol("AMDeviceSecureStartService", (void **)&AMDeviceSecureStartService) != 0) return -1;
    if (load_symbol("AMDServiceConnectionReceiveMessage", (void **)&AMDServiceConnectionReceiveMessage) != 0) return -1;
    if (load_symbol("AMDServiceConnectionReceive", (void **)&AMDServiceConnectionReceive) != 0) return -1;
    if (load_symbol("AMDServiceConnectionSendMessage", (void **)&AMDServiceConnectionSendMessage) != 0) return -1;
    if (load_symbol("AMDServiceConnectionSend", (void **)&AMDServiceConnectionSend) != 0) return -1;
    if (load_symbol("AMDeviceLookupApplications", (void **)&AMDeviceLookupApplications) != 0) return -1;

    if (load_symbol("kCFBooleanTrue", (void **)&kCFBooleanTrue) != 0) return -1;
    if (load_symbol("kCFBooleanFalse", (void **)&kCFBooleanFalse) != 0) return -1;
    if (load_symbol("CFRunLoopStop", (void **)&CFRunLoopStop) != 0) return -1;
    if (load_symbol("CFRunLoopRun", (void **)&CFRunLoopRun) != 0) return -1;
    if (load_symbol("CFRunLoopGetCurrent", (void **)&CFRunLoopGetCurrent) != 0) return -1;
    if (load_symbol("CFStringGetTypeID", (void **)&CFStringGetTypeID) != 0) return -1;
    if (load_symbol("CFGetTypeID", (void **)&CFGetTypeID) != 0) return -1;
    if (load_symbol("CFRelease", (void **)&CFRelease) != 0) return -1;
    if (load_symbol("CFRetain", (void **)&CFRetain) != 0) return -1;
    if (load_symbol("CFStringCreateWithCString", (void **)&CFStringCreateWithCString) != 0) return -1;
    if (load_symbol("CFStringGetCString", (void **)&CFStringGetCString) != 0) return -1;
    if (load_symbol("CFDictionaryCreateMutable", (void **)&CFDictionaryCreateMutable) != 0) return -1;
    if (load_symbol("CFDictionaryAddValue", (void **)&CFDictionaryAddValue) != 0) return -1;
    if (load_symbol("CFDictionaryGetValue", (void **)&CFDictionaryGetValue) != 0) return -1;
    if (load_symbol("CFEqual", (void **)&CFEqual) != 0) return -1;
    if (load_symbol("CFDataCreate", (void **)&CFDataCreate) != 0) return -1;
    if (load_symbol("CFDictionarySetValue", (void **)&CFDictionarySetValue) != 0) return -1;
    if (load_symbol("CFStringGetLength", (void **)&CFStringGetLength) != 0) return -1;

    if (load_symbol("AFCConnectionOpen", (void **)&AFCConnectionOpen) != 0) return -1;
    if (load_symbol("AFCDeviceInfoOpen", (void **)&AFCDeviceInfoOpen) != 0) return -1;
    if (load_symbol("AFCDirectoryOpen", (void **)&AFCDirectoryOpen) != 0) return -1;
    if (load_symbol("AFCDirectoryRead", (void **)&AFCDirectoryRead) != 0) return -1;
    if (load_symbol("AFCDirectoryClose", (void **)&AFCDirectoryClose) != 0) return -1;
    if (load_symbol("AFCDirectoryCreate", (void **)&AFCDirectoryCreate) != 0) return -1;
    if (load_symbol("AFCRemovePath", (void **)&AFCRemovePath) != 0) return -1;
    if (load_symbol("AFCRenamePath", (void **)&AFCRenamePath) != 0) return -1;
    if (load_symbol("AFCLinkPath", (void **)&AFCLinkPath) != 0) return -1;
    if (load_symbol("AFCFileRefOpen", (void **)&AFCFileRefOpen) != 0) return -1;
    if (load_symbol("AFCFileRefRead", (void **)&AFCFileRefRead) != 0) return -1;
    if (load_symbol("AFCFileRefWrite", (void **)&AFCFileRefWrite) != 0) return -1;
    if (load_symbol("AFCFileRefClose", (void **)&AFCFileRefClose) != 0) return -1;
    if (load_symbol("AFCFileInfoOpen", (void **)&AFCFileInfoOpen) != 0) return -1;
    if (load_symbol("AFCConnectionClose", (void **)&AFCConnectionClose) != 0) return -1;
    return 0;
}

void *load_embed_file(const char *name, size_t *size) {
    HMODULE module = GetModuleHandle(NULL);
    HRSRC resource = FindResourceA(NULL, name, RT_RCDATA);
    if (resource == NULL) return NULL;

    HGLOBAL global_data = LoadResource(module, resource);
    *size = (size_t)SizeofResource(module, resource);
    return LockResource(global_data);
}

void print_log(uint32_t type, const char *format, ...) {    
    va_list va;
    FILE *file = NULL;
    const char *prefix = NULL;
    bool use_color = (global_flags & FLAG_NO_COLOR) == 0;

    switch (type) {
        case INFO: 
            file = stdout;
            prefix = use_color ? PREFIX_INFO_COLOR : PREFIX_INFO;
            break;
        case ERROR: 
            file = stderr;
            prefix = use_color ? PREFIX_ERROR_COLOR : PREFIX_ERROR;
            break;
        case WARNING: 
            file = stderr;
            prefix = use_color ? PREFIX_WARNING_COLOR : PREFIX_WARNING;
            break;
        case VERBOSE:
            if ((global_flags & FLAG_VERBOSE_LOGGING) == 0) return;
            file = stdout;
            prefix = use_color ? PREFIX_VERBOSE_COLOR : PREFIX_VERBOSE;
            break;
        default:
            return;
    }

    if (file != NULL) {
        if (prefix != NULL) fprintf(file, "%s", prefix);
        va_start(va, format);
        vfprintf(file, format, va);
        va_end(va);
    }
}
