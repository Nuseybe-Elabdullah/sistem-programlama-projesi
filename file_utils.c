#include "file_utils.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

int create_target_dir(const char *dir_path) {
    char tmp[512];
    snprintf(tmp, sizeof(tmp), "%s", dir_path);
    size_t len = strlen(tmp);
    if(len == 0) return 0;
    
    // sondaki slash varsa sil
    if(tmp[len - 1] == '/') {
        tmp[len - 1] = 0;
    }
    
    // dongu ile olustur
    for(char *p = tmp + 1; *p; p++) {
        if(*p == '/') {
            *p = 0;
            if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
                return -1;
            }
            *p = '/';
        }
    }
    
    // olustur
    if (mkdir(tmp, 0755) != 0 && errno != EEXIST) {
        return -1;
    }
    return 0;
}
