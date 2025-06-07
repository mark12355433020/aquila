#include "device.h"
#include "afc.h"
#include "jailbreak.h"
#include "util.h"

uint16_t global_flags = FLAG_NONE;

void print_help(void) {
    fprintf(stdout, "Usage: aquila [-jaobvnch]\n");
    fprintf(stdout, "   -j, --jailbreak \t\t install jailbreak (default)\n");
    fprintf(stdout, "   -f, --force-install \t\t install if already jailbroken\n");
    fprintf(stdout, "   -v, --verbose \t\t enable verbose logging\n");
    fprintf(stdout, "   -n, --no-color \t\t disable color in logs\n");
    fprintf(stdout, "   -c, --credits \t\t print credits\n");
    fprintf(stdout, "   -h, --help \t\t\t print help\n\n");
    exit(0);
}

void print_credits(void) {
    fprintf(stdout, "Credits:\n");
    fprintf(stdout, "   staturnz - jailbreak and exploit\n");
    fprintf(stdout, "   comex - ddi race condition and sandbox patch\n");
    fprintf(stdout, "   planetbeing - ios-jailbreak-patchfinder\n");
    fprintf(stdout, "   PanguTeam - CVE-2014-4461 (kernel exploit bug)\n");
    fprintf(stdout, "   evad3rs - amfid bypass method\n\n");
    exit(0);
}

void parse_args(int argc, char **argv) {
    for (int i = 1; i < argc; i++) {
        char *arg = argv[i];

        if (arg[0] == '-' && arg[1] != '-') {
            for (int j = 1; arg[j] != '\0'; j++) {
                switch (arg[j]) {
                    case 'j': global_flags |= FLAG_JAILBREAK; break;
                    case 'f': global_flags |= FLAG_FORCE_INSTALL; break;
                    case 'v': global_flags |= FLAG_VERBOSE_LOGGING; break;
                    case 'n': global_flags |= FLAG_NO_COLOR; break;
                    case 'c': return print_credits();
                    default: return print_help();
                }
            }
        } else if (strcmp(arg, "--jailbreak") == 0) {
            global_flags |= FLAG_JAILBREAK;
        } else if (strcmp(arg, "--force-install") == 0) {
            global_flags |= FLAG_FORCE_INSTALL;
        } else if (strcmp(arg, "--verbose") == 0) {
            global_flags |= FLAG_VERBOSE_LOGGING;
        } else if (strcmp(arg, "--no-color") == 0) {
            global_flags |= FLAG_NO_COLOR;
        } else if (strcmp(arg, "--credits") == 0) {
            return print_credits();
        } else {
            return print_help();
        }
    }

    global_flags |= FLAG_JAILBREAK;
    if (!isatty(1) || !isatty(2)) global_flags |= FLAG_NO_COLOR;
}

int main(int argc, char **argv) {
    print_log(INFO, "aquila jailbreak for iOS 6 (v1.0)\n");
    parse_args(argc, argv);

    if (platform_init() != 0) {
        print_log(ERROR, "failed to initialize platform\n");
        return -1;
    }

    if (md_init() != 0) {
        print_log(ERROR, "failed to initialize MobileDevice API\n");
        return -1;
    }

    print_log(INFO, "waiting for device...\n");
    am_device_t *device = md_await_device();
    if (device == NULL) {
        print_log(ERROR, "failed to connect to device\n");
        return -1;
    }

    device_info_t *device_info = md_device_info(device);
    if (device_info == NULL) {
        print_log(ERROR, "failed to get device info\n");
        return -1;
    }

    if (device_info->version[2] == 0) {
        print_log(INFO, "%s on iOS %d.%d connected\n", device_info->product_type, device_info->version[0], device_info->version[1]);
    } else {
        print_log(INFO, "%s on iOS %d.%d.%d connected\n", device_info->product_type, device_info->version[0], device_info->version[1], device_info->version[2]);
    }

    if (device_info->version[0] != 6) {
        print_log(ERROR, "device version is not supported\n");
        return -1;
    }

    afc_info_t *afc_info = afc_init(device);
    if (afc_info == NULL) {
        print_log(ERROR, "failed to connect to AFC\n");
        return -1;
    }

    if (jailbreak(device, device_info, afc_info) != 0) {
        print_log(ERROR, "failed to jailbreak device\n");
        return -1;
    }

    print_log(INFO, "done\n");
    return 0;
}