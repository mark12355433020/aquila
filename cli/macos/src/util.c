#include "util.h"

void *load_embed_file(const char *name, size_t *size) {
    struct mach_header_64 *hdr = &_mh_execute_header;
    struct load_command *load_cmd = (struct load_command *)(hdr + 1);

    for (int i = 0; i < hdr->ncmds; i++) {
        if (load_cmd->cmd == LC_SEGMENT_64) {
            struct segment_command_64 *seg_cmd = (struct segment_command_64 *)load_cmd;
            if (strcmp(seg_cmd->segname, "__DATA") == 0) {
                struct section_64 *sect = (struct section_64 *)(seg_cmd + 1);

                for (uint32_t j = 0; j < seg_cmd->nsects; j++) {
                    if (strcmp(sect->sectname, name) == 0) {
                        *size = sect->size;
                        return (void *)((uint8_t *)hdr + sect->offset);
                    }
                    sect++;
                }
            }
        }
        load_cmd = (struct load_command *)((uint64_t)load_cmd + load_cmd->cmdsize);
    }
    return NULL;
}

void print_log(log_type_t type, const char *format, ...) {    
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
