#include "common.h"
#include "device.h"
#include "afc.h"
#include "jailbreak.h"
#include "util.h"

bool is_jailbroken(afc_info_t *afc_info) {
    int service = md_open_service(afc_info->device, "com.apple.afc2", false);
    if (service != -1) return true;
    

    service = md_open_service(afc_info->device, "haxx.aquila.afc2", false);
    if (service != -1) return true;
    
    CFDictionaryRef dict = NULL;
    AMDeviceLookupApplications(afc_info->device, 0, &dict);
    if (dict != NULL) {
        CFDictionaryRef cydia = CFDictionaryGetValue(dict, CFSTR("com.saurik.Cydia"));
        if (cydia != NULL) return true;
    }
    return false;
}

int upload_dmg_payload(afc_info_t *afc_info, int64_t delay) {
    afc_delete_item(afc_info, "PublicStaging");
    afc_create_dir(afc_info, "PublicStaging");

    if (afc_upload_embed_file(afc_info, "__STOCK_DMG", "PublicStaging/staging.dimage") != 0) return -1;
    if (afc_upload_embed_file(afc_info, "__PAYLOAD_DMG", "PublicStaging/payload.dimage") != 0) return -1;

    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(NULL, 0, NULL, NULL);
    CFDictionarySetValue(dict, CFSTR("Command"), CFSTR("MountImage"));
    CFDictionarySetValue(dict, CFSTR("ImageType"), CFSTR("Developer"));
    CFDictionarySetValue(dict, CFSTR("ImagePath"), CFSTR("/var/mobile/Media/PublicStaging/staging.dimage"));

    size_t signature_size = 0;
    void *signature_data = load_embed_file("__SIGNATURE", &signature_size);
    if (signature_data == NULL) {
        CFRelease(dict);
        return -1;
    }

    CFDataRef cf_signature = CFDataCreate(NULL, signature_data, signature_size);
    CFDictionarySetValue(dict, CFSTR("ImageSignature"), cf_signature);

    void *service = md_open_secure_service(afc_info->device, "com.apple.mobile.mobile_image_mounter");
    if (service == NULL) {
        CFRelease(cf_signature);
        CFRelease(dict);
        return -1;
    }

    AMDServiceConnectionSendMessage(service, dict, kCFPropertyListXMLFormat_v1_0);
    usleep(delay);
    AFCRenamePath(afc_info->connection, "PublicStaging/payload.dimage", "PublicStaging/staging.dimage");

    CFRelease(cf_signature);
    CFRelease(dict);
    CFDictionaryRef status = NULL;

    AMDServiceConnectionReceiveMessage(service, &status, NULL);
    CFRelease(service);
    if (status == NULL) return -1;

    CFStringRef status_str = CFDictionaryGetValue(status, CFSTR("Status"));
    if (status_str != NULL) {
        if (CFEqual(status_str, CFSTR("Complete"))) {
            CFRelease(status);
            return 0;
        }
    }

    CFRelease(status);
    return -1;
}

int remount_rootfs(afc_info_t *afc_info) {
    afc_file_ref_t test_file = NULL;
    if ((test_file = afc_create_file(afc_info, "/.test_file")) != NULL) {
        print_log(VERBOSE, "rootfs already remounted\n");

        afc_close_file(afc_info, test_file);
        afc_delete_item(afc_info, "/.test_file");
        return 0;
    }

    md_open_service(afc_info->device, SERVICE_REMOUNT, true);
    usleep(100000);

    if ((test_file = afc_create_file(afc_info, "/.test_file")) != NULL) {
        afc_close_file(afc_info, test_file);
        afc_delete_item(afc_info, "/.test_file");
        return 0;
    }
    return -1;
}

int jailbreak(am_device_t *device, device_info_t *device_info, afc_info_t *afc_info) {
    print_log(INFO, "starting jailbreak...\n");
    if (is_jailbroken(afc_info)) {
        if (has_flag(FLAG_FORCE_INSTALL)) {
            print_log(WARNING, "device already jailbroken, continuing anyways\n");
        } else {
            print_log(INFO, "device already jailbroken\n");
            return 0;
        }
    }

    print_log(INFO, "uploading dmg payload...\n");
    int64_t upload_delay = 1000;
    bool upload_done = false;
    int upload_tries = 1;

    for (; upload_tries < 100; upload_tries++) {
        if (upload_dmg_payload(afc_info, upload_delay) == 0) {
            upload_done = true;
            break;
        }

        print_log(VERBOSE, "retrying dmg payload upload (attempt #%d)\n", upload_tries);
        upload_delay += 100;
    }

    if (!upload_done) {
        print_log(ERROR, "failed to upload dmg payload\n");
        return -1;
    }
    
    print_log(INFO, "dmg payload uploaded in %d tries\n", upload_tries);
    afc_info_t *afc2_info = afc2_init(device);
    if (afc2_info == NULL) {
        print_log(ERROR, "failed to connect to AFC2\n");
        return -1;
    }

    print_log(INFO, "remounting rootfs...\n");
    if (remount_rootfs(afc2_info) != 0) {
        print_log(ERROR, "failed to remount rootfs\n");
        return -1;
    }

    print_log(INFO, "rootfs remounted\n");
    print_log(INFO, "uploading jailbreak files...\n");
    afc_delete_item(afc2_info, "/private/var/aquila");
    afc_create_dir(afc2_info, "/private/var/aquila");
    afc_upload_embed_file(afc2_info, "__BOOTSTRAP", "/private/var/aquila/bootstrap.tar");

    afc_upload_embed_file(afc2_info, "__LAUNCHD_CONF", "/private/etc/launchd.conf");
    afc_upload_embed_file(afc2_info, "__SPLASHSCREEN", "/private/var/aquila/splashscreen.jp2");
    afc_upload_embed_file(afc2_info, "__LIBMIS", "/private/var/aquila/_libmis.dylib");
    afc_upload_embed_file(afc2_info, "__TAR", "/bin/tar");
    afc_upload_embed_file(afc2_info, "__AQUILA", "/private/var/aquila/aquila");

    afc_delete_item(afc2_info, "/sbin/reboot");
    afc_delete_item(afc2_info, "/Library/Logs/CrashReporter/Baseband");
    afc_upload_embed_file(afc2_info, "__INSTALLER", "/Library/Logs/CrashReporter/Baseband");
    md_open_service(device, "haxx.aquila.housekeeping", true);

    print_log(INFO, "rebooting device...\n");
    md_reboot_device(device);
    return 0;
}
