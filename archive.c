#include "archive.h"
#include "validation.h"
#include "file_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int archive_files(char *files[], int num_files, const char *output_file) {
    size_t total_size = 0;
    
    // dosya kontrolleri
    for (int i = 0; i < num_files; i++) {
        if (!is_ascii_text(files[i])) {
            char *err_filename = strrchr(files[i], '/');
            if (!err_filename) err_filename = files[i];
            else err_filename++;
            fprintf(stderr, "%s giriş dosyasının formatı uyumsuzdur!\n", err_filename);
            return 1;
        }
        
        struct stat st;
        if (stat(files[i], &st) < 0) {
            perror("Dosya bilgisi okunamadı");
            return 1;
        }
        total_size += st.st_size;
    }
    
    if (total_size > MAX_TOTAL_SIZE) {
        fprintf(stderr, "Hata: Toplam boyut 200 MB'ı geçemez!\n");
        return 1;
    }
    
    // metadata yaz
    char *metadata = malloc(16384); // 32 dosya için yeterince büyük bir tampon
    if (!metadata) {
        perror("Bellek ayrılamadı");
        return 1;
    }
    metadata[0] = '\0';
    
    for (int i = 0; i < num_files; i++) {
        struct stat st;
        stat(files[i], &st);
        
        char entry[512];
        char *filename = strrchr(files[i], '/');
        
        if (!filename) filename = files[i];
        else filename++; // Slash'i geç
        
        // formatla
        snprintf(entry, sizeof(entry), "|%s,%o,%ld|", filename, st.st_mode & 0777, (long)st.st_size);
        strcat(metadata, entry);
    }
    
    size_t metadata_len = strlen(metadata);
    
    // ciktigi ac
    int out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (out_fd < 0) {
        perror("Arşiv dosyası oluşturulamadı");
        free(metadata);
        return 1;
    }
    
    // 10 byte uzunluk
    char meta_size_str[METADATA_SIZE_LEN + 1];
    snprintf(meta_size_str, sizeof(meta_size_str), "%010lu", (unsigned long)metadata_len);
    write(out_fd, meta_size_str, METADATA_SIZE_LEN);
    
    // bilgileri yaz
    write(out_fd, metadata, metadata_len);
    free(metadata);
    
    // icerikleri kopyala
    for (int i = 0; i < num_files; i++) {
        int in_fd = open(files[i], O_RDONLY);
        if (in_fd >= 0) {
            char buffer[8192];
            ssize_t bytes_read;
            while ((bytes_read = read(in_fd, buffer, sizeof(buffer))) > 0) {
                write(out_fd, buffer, bytes_read);
            }
            close(in_fd);
        } else {
            perror("Giriş dosyası açılamadı");
        }
    }
    
    close(out_fd);
    printf("Dosyalar birleştirildi.\n");
    return 0;
}
