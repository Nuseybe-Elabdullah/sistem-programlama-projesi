#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "archive.h"
#include "extract.h"

int main(int argc, char *argv[]) {
    // argumanlar
    if (argc < 2) {
        fprintf(stderr, "Kullanım: \n"
                        "  tarsau -b dosya1 dosya2 ... -o arsiv.sau\n"
                        "  tarsau -a arsiv.sau hedefDizin\n");
        return 1;
    }

    // arsivleme
    if (strcmp(argv[1], "-b") == 0) {
        char *files[32];
        int num_files = 0;
        char *output_file = "a.sau";
        
        for (int i = 2; i < argc; i++) {
            if (strcmp(argv[i], "-o") == 0) {
                if (i + 1 < argc) {
                    output_file = argv[i + 1];
                    break;
                }
            } else {
                if (num_files < 32) {
                    files[num_files++] = argv[i];
                } else {
                    fprintf(stderr, "Hata: Maksimum 32 dosya kabul edilir.\n");
                    return 1;
                }
            }
        }
        
        if (num_files == 0) {
            fprintf(stderr, "Hata: Arşivlenecek dosya belirtilmedi.\n");
            return 1;
        }

        return archive_files(files, num_files, output_file);
    } 
    // cikarma
    else if (strcmp(argv[1], "-a") == 0) {
        if (argc < 3 || argc > 4) {
            fprintf(stderr, "Hata: Geçersiz argüman. Kullanım: tarsau -a arsiv.sau [hedefDizin]\n");
            return 1;
        }
        const char *archive_file = argv[2];
        const char *target_dir = (argc == 4) ? argv[3] : ".";
        return extract_archive(archive_file, target_dir);
    } 
    // hatali
    else {
        fprintf(stderr, "Hata: Geçersiz parametre %s\n", argv[1]);
        return 1;
    }

    return 0;
}
