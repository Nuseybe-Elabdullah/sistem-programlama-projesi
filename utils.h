#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>
#include <sys/types.h>

#define MAX_FILES 32
#define MAX_TOTAL_SIZE (200 * 1024 * 1024)
#define METADATA_SIZE_LEN 10

int is_ascii_text(const char *filepath);

#endif
