#ifndef install_h
#define install_h

#include "common.h"

int create_file(const char *path, mode_t mode, uid_t uid, gid_t gid);
int set_file_permissions(const char *path, mode_t mode, uid_t uid, gid_t gid);
void *load_file(const char *path, size_t *size);
char *get_hw_model(void);
int edit_plist(const char *path, void (^action)(CFMutableDictionaryRef plist));
int install_jailbreak(void);

#endif /* install_h */