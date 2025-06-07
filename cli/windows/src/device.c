#include "common.h"
#include "util.h"
#include "device.h"

int (*AMDeviceNotificationSubscribe)(void *callback, uint32_t __unk0, uint32_t __unk1, uint32_t cookie, am_device_notification_t **subscription);
int (*AMDeviceNotificationUnsubscribe)(am_device_notification_t* subscription);
int (*AMDeviceConnect)(am_device_t *device);
int (*AMDeviceIsPaired)(am_device_t *device);
int (*AMDevicePair)(am_device_t *device);
int (*AMDeviceValidatePairing)(am_device_t *device);
int (*AMDeviceStartSession)(am_device_t *device);
void *(*AMDeviceCopyValue)(am_device_t *device, CFStringRef domain, CFStringRef cfstring);
int (*AMDeviceStartService)(am_device_t *device, CFStringRef service_name, int *socket_fd);
int (*AMDeviceStartServiceWithOptions)(am_device_t *device, CFStringRef service_name, CFDictionaryRef options, int *socket_fd);
int (*AMDeviceStopSession)(am_device_t *device);
int (*USBMuxConnectByPort)(int connectionID, int iPhone_port_network_byte_order, int* outHandle);
int (*AMDeviceGetConnectionID)(am_device_t *device);
int (*AMDeviceSecureStartService)(am_device_t *device, CFStringRef service, CFDictionaryRef flags, void *handle);
int (*AMDServiceConnectionReceiveMessage)(void *service, CFPropertyListRef message, CFPropertyListFormat *format);
int (*AMDServiceConnectionReceive)(void *service, char *buf, size_t size);
int (*AMDServiceConnectionSendMessage)(void *service, CFPropertyListRef message, CFPropertyListFormat format);
int (*AMDServiceConnectionSend)(void *service, const void *message, size_t length);
int (*AMDeviceLookupApplications)(am_device_t *device, CFDictionaryRef options, CFDictionaryRef *result);

static am_device_notification_t *md_notification = NULL;
static bool wait_for_device = false;
static am_device_t *last_device = NULL;

static void md_connect_handler(am_device_notification_callback_info_t *info, int cookie) {
    if (info->msg == ADNCI_MSG_CONNECTED) {
        if (AMDeviceConnect(info->dev) != 0) return;
        if (AMDeviceIsPaired(info->dev) != 1) return;
        if (AMDeviceValidatePairing(info->dev) != 0) return;
        if (AMDeviceStartSession(info->dev) != 0) return;

        last_device = info->dev;
        if (wait_for_device) {
            CFRunLoopStop(CFRunLoopGetCurrent());
            wait_for_device = false;
        }
    }
}

int md_init(void) {
    if (AMDeviceNotificationSubscribe(md_connect_handler, 0, 0, 0, &md_notification) != 0) return -1;
    return 0;
}

void md_deinit(void) {
    if (md_notification != NULL) {
        AMDeviceNotificationUnsubscribe(md_notification);
        md_notification = NULL;
    }

    CFRunLoopStop(CFRunLoopGetCurrent());
    last_device = NULL;
    wait_for_device = false;
}

am_device_t *md_await_device(void) {
    wait_for_device = true;
    CFRunLoopRun();
    return last_device;
}

char *md_get_device_value(am_device_t *device, const char *key) {
    print_log(VERBOSE, "copying value: %s\n", key);
    CFStringRef cf_key = CFStringCreateWithCString(NULL, key, kCFStringEncodingASCII);
    CFStringRef value = (CFStringRef)AMDeviceCopyValue(device, 0, cf_key);
    CFRelease(cf_key);

    if (value == NULL) return NULL;
    if (CFGetTypeID(value) != CFStringGetTypeID()) {
        CFRelease(value);
        return NULL;
    }

    CFIndex len = CFStringGetLength(value);
    if (len == 0) {
        CFRelease(value);
        return NULL;
    }

    char *buf = calloc(1, len+2);
    CFStringGetCString(value, buf, len+1, kCFStringEncodingUTF8);
    CFRelease(value);
    return buf;
}

device_info_t *md_device_info(am_device_t *device) {
    char *cpu_arch = NULL;
    char *hw_model = NULL;
    char *uuid = NULL;
    char *product_type = NULL;
    char *version = NULL;

    if ((cpu_arch = md_get_device_value(device, "CPUArchitecture")) == NULL) goto err;
    if ((hw_model = md_get_device_value(device, "HardwareModel")) == NULL) goto err;
    if ((uuid = md_get_device_value(device, "UniqueDeviceID")) == NULL) goto err;
    if ((product_type = md_get_device_value(device, "ProductType")) == NULL) goto err;
    if ((version = md_get_device_value(device, "ProductVersion")) == NULL) goto err;

    device_info_t *info = calloc(1, sizeof(device_info_t));
    info->cpu_arch = cpu_arch;
    info->hw_model = hw_model;
    info->uuid = uuid;
    info->product_type = product_type;

    sscanf(version, "%d.%d.%d", &info->version[0], &info->version[1], &info->version[2]);
    free(version);
    version = NULL;
    return info;

err:
    if (cpu_arch != NULL) free(cpu_arch);
    if (hw_model != NULL) free(hw_model);
    if (uuid != NULL) free(uuid);
    if (product_type != NULL) free(product_type);
    if (version != NULL) free(version);
    return NULL;
}

int md_open_service(am_device_t *device, const char *name, bool timeout) {
    print_log(VERBOSE, "opening service: %s\n", name);
    CFStringRef cf_name = CFStringCreateWithCString(NULL, name, kCFStringEncodingASCII);
    int64_t service = -1;
    int rv = -1;

    if (timeout) {
        CFMutableDictionaryRef dict = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
        CFDictionaryAddValue(dict, CFSTR("TimeoutConnection"), kCFBooleanTrue);
        CFDictionaryAddValue(dict, CFSTR("CloseOnInvalidate"), kCFBooleanTrue);
        rv = AMDeviceStartServiceWithOptions(device, cf_name, dict, (int *)&service);
        CFRelease(dict);
    } else {
        rv = AMDeviceStartService(device, cf_name, (int *)&service);
    }
    
    CFRelease(cf_name);
    return (rv == 0) ? (int)service : -1;
}

void *md_open_secure_service(am_device_t *device, const char *name) {
    print_log(VERBOSE, "opening service: %s\n", name);
    CFStringRef cf_name = CFStringCreateWithCString(NULL, name, kCFStringEncodingASCII);
    void *service = NULL;

    int rv = AMDeviceSecureStartService(device, cf_name, NULL, &service);
    CFRelease(cf_name);
    return (rv == 0) ? service : NULL;
}

void md_close_service(int service) {
   // if (service >= 0) close(service);
}

int md_reboot_device(am_device_t *device) {
    void *service = md_open_secure_service(device, "com.apple.mobile.diagnostics_relay");
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
    CFDictionaryAddValue(dict, CFSTR("Request"), CFSTR("Restart"));

    AMDServiceConnectionSendMessage(service, dict, kCFPropertyListXMLFormat_v1_0);
    CFRelease(dict);

    CFDictionaryRef status = NULL;
    AMDServiceConnectionReceiveMessage(service, &status, NULL);

    dict = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
    CFDictionaryAddValue(dict, CFSTR("Request"), CFSTR("Goodbye"));
    AMDServiceConnectionSendMessage(service, dict, kCFPropertyListXMLFormat_v1_0);
    CFRelease(dict);
    return 0;
}
