#include "extract.h"
#include "utils.h"
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
    
    // Hedef dizini oluştur (yoksa)
    struct stat st;
    if (stat(target_dir, &st) == -1) {
        if (mkdir(target_dir, 0755) != 0) {
            perror("Hedef dizin oluşturulamadı");
            close(in_fd);
            return 1;
        }
    }
    
    // Organizasyon bölümünün ilk 10 byte'ını (metadata boyutu) oku
    char meta_size_str[METADATA_SIZE_LEN + 1];
    if (read(in_fd, meta_size_str, METADATA_SIZE_LEN) != METADATA_SIZE_LEN) {
        fprintf(stderr, "Arşiv dosyası uygunsuz veya bozuk!\n");
        close(in_fd);
        return 1;
    }
    meta_size_str[METADATA_SIZE_LEN] = '\0';
    
    // Boyut değerini integer'a çevir
    size_t meta_size;
    unsigned long tmp_meta_size;
    if (sscanf(meta_size_str, "%lu", &tmp_meta_size) != 1) {
        fprintf(stderr, "Arşiv dosyası uygunsuz veya bozuk!\n");
        close(in_fd);
        return 1;
    }
    meta_size = (size_t)tmp_meta_size;
    
    // Metadata'yı okumak için bellek ayır
    char *metadata = malloc(meta_size + 1);
    if (!metadata) {
        perror("Bellek ayrılamadı");
        close(in_fd);
        return 1;
    }
    
    // Metadata kısmını arşivden oku
    if (read(in_fd, metadata, meta_size) != (ssize_t)meta_size) {
        fprintf(stderr, "Arşiv dosyası uygunsuz veya bozuk!\n");
        free(metadata);
        close(in_fd);
        return 1;
    }
    metadata[meta_size] = '\0';
    
    // Metadata'yı | karakterine göre parse et
    char *token = strtok(metadata, "|");
    while (token != NULL) {
        char filename[256];
        unsigned int mode;
        long size;
        
        // Örn: dosyaAdi,izinler,boyut
        if (sscanf(token, "%255[^,],%o,%ld", filename, &mode, &size) == 3) {
            char target_path[512];
            snprintf(target_path, sizeof(target_path), "%s/%s", target_dir, filename);
            
            int out_fd = open(target_path, O_WRONLY | O_CREAT | O_TRUNC, mode);
            if (out_fd < 0) {
                perror("Çıkarılan dosya oluşturulamadı");
                free(metadata);
                close(in_fd);
                return 1;
            }
            
            // Dosya içeriğini byte byte (veya chunk halinde) kopyala
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
            
            // İzinleri Linux stili tekrar ayarla
            chmod(target_path, mode);
        }
        token = strtok(NULL, "|");
    }
    
    free(metadata);
    close(in_fd);
    return 0;
}
