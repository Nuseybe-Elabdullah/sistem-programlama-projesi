#include "utils.h"
#include <fcntl.h>
#include <unistd.h>

int is_ascii_text(const char *filepath) {
    int fd = open(filepath, O_RDONLY);
    if (fd < 0) return 0;

    unsigned char buffer[4096];
    ssize_t bytes_read;
    int is_text = 1;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; ++i) {
            if (buffer[i] == '\0') { // Null byte kontrolü (binary dosyaları yakalar)
                is_text = 0;
                break;
            }
        }
        if (!is_text) break;
    }
    
    close(fd);
    return is_text;
}
