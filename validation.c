#include "validation.h"
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int is_ascii_text(const char *filepath) {
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) return 0;

    unsigned char buffer[4096];
    ssize_t bytes_read;
    int is_text = 1;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; ++i) {
            if (buffer[i] > 127 || buffer[i] == '\0') {
                is_text = 0;
                break;
            }
        }
        if (!is_text) break;
    }
    
    close(fd);
    return is_text;
}

int is_safe_filename(const char *filename) {
    if (filename == NULL || filename[0] == '\0') return 0;
    
    // path traversal kontrolu
    for (int i = 0; filename[i] != '\0'; ++i) {
        if (filename[i] == '/' || filename[i] == '\\' || filename[i] == ':' || filename[i] == '~') {
            return 0;
        }
    }
    
    // . ve .. isimlerini engelle
    if (strcmp(filename, ".") == 0 || strcmp(filename, "..") == 0) {
        return 0;
    }
    
    return 1;
}
