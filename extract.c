#include "extract.h"
#include "validation.h"
#include "file_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>


int extract_archive(const char *archive_file, const char *target_dir) {
    int in_fd = open(archive_file, O_RDONLY);
    if (in_fd < 0) {
        fprintf(stderr, "Arşiv dosyası uygunsuz veya bozuk!\n");
        return 1;
    }
    
    struct stat arch_st;
    if (fstat(in_fd, &arch_st) < 0) {
        fprintf(stderr, "Arşiv dosyası uygunsuz veya bozuk!\n");
        close(in_fd);
        return 1;
    }
    
    // dizin oluştur
    if (strcmp(target_dir, ".") != 0) {
        if (create_target_dir(target_dir) != 0) {
            perror("Hedef dizin oluşturulamadı");
            close(in_fd);
            return 1;
        }
    }
    
    // boyutu oku
    char meta_size_str[METADATA_SIZE_LEN + 1];
    if (read(in_fd, meta_size_str, METADATA_SIZE_LEN) != METADATA_SIZE_LEN) {
        fprintf(stderr, "Arşiv dosyası uygunsuz veya bozuk!\n");
        close(in_fd);
        return 1;
    }
    meta_size_str[METADATA_SIZE_LEN] = '\0';
    
    // sayiya cevir
    size_t meta_size;
    unsigned long tmp_meta_size;
    if (sscanf(meta_size_str, "%lu", &tmp_meta_size) != 1) {
        fprintf(stderr, "Arşiv dosyası uygunsuz veya bozuk!\n");
        close(in_fd);
        return 1;
    }
    meta_size = (size_t)tmp_meta_size;
    
    if (meta_size == 0 || meta_size > (size_t)arch_st.st_size) {
        fprintf(stderr, "Arşiv dosyası uygunsuz veya bozuk!\n");
        close(in_fd);
        return 1;
    }
    
    // metadata ayir
    char *metadata = malloc(meta_size + 1);
    if (!metadata) {
        perror("Bellek ayrılamadı");
        close(in_fd);
        return 1;
    }
    
    // metadata kismini oku
    if (read(in_fd, metadata, meta_size) != (ssize_t)meta_size) {
        fprintf(stderr, "Arşiv dosyası uygunsuz veya bozuk!\n");
        free(metadata);
        close(in_fd);
        return 1;
    }
    metadata[meta_size] = '\0';
    
    // token ayir
    char *token = strtok(metadata, "|");
    int files_extracted = 0;
    
    while (token != NULL) {
        char filename[256];
        unsigned int mode;
        long size;
        
        // format: dosyaAdi,izinler,boyut
        if (sscanf(token, "%255[^,],%o,%ld", filename, &mode, &size) == 3) {
            if (!is_safe_filename(filename)) {
                fprintf(stderr, "Arşiv dosyası uygunsuz veya bozuk!\n");
                free(metadata);
                close(in_fd);
                return 1;
            }
            
            char target_path[512];
            if (strcmp(target_dir, ".") == 0) {
                snprintf(target_path, sizeof(target_path), "%s", filename);
            } else {
                snprintf(target_path, sizeof(target_path), "%s/%s", target_dir, filename);
            }
            
            int out_fd = open(target_path, O_WRONLY | O_CREAT | O_TRUNC, mode);
            if (out_fd < 0) {
                perror("Çıkarılan dosya oluşturulamadı");
                free(metadata);
                close(in_fd);
                return 1;
            }
            
            // icerigi kopyala
            char buffer[8192];
            long bytes_left = size;
            while (bytes_left > 0) {
                size_t to_read = bytes_left > (long)sizeof(buffer) ? sizeof(buffer) : (size_t)bytes_left;
                ssize_t bytes_read = read(in_fd, buffer, to_read);
                
                if (bytes_read <= 0) {
                    fprintf(stderr, "Arşiv dosyası uygunsuz veya bozuk!\n");
                    close(out_fd);
                    free(metadata);
                    close(in_fd);
                    return 1;
                }
                
                write(out_fd, buffer, bytes_read);
                bytes_left -= bytes_read;
            }
            close(out_fd);
            
            // izinleri duzenle
            chmod(target_path, mode);
            files_extracted++;
        } else {
             fprintf(stderr, "Arşiv dosyası uygunsuz veya bozuk!\n");
             free(metadata);
             close(in_fd);
             return 1;
        }
        token = strtok(NULL, "|");
    }
    
    free(metadata);
    close(in_fd);
    
    if (files_extracted > 0) {
        printf("%s dizininde dosyalar açıldı.\n", target_dir);
    } else {
        fprintf(stderr, "Arşiv dosyası uygunsuz veya bozuk!\n");
        return 1;
    }
    
    return 0;
}
