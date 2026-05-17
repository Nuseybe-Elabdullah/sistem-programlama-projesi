# Tarsau Arşivleme Aracı

Bu proje, metin dosyalarını arşivlemek ve açmak için POSIX sistem çağrılarını kullanan bir C uygulamasıdır. `tar` komutuna benzer bir mantıkla çalışır ancak sadece 1-byte ASCII metin dosyalarını destekler.

## Sistem Mimarisi
Proje modüler olarak bölünmüştür:
- `main.c`: Argüman kontrolü ve yönlendirme.
- `archive.c`: Dosyaları `.sau` formatında birleştirme.
- `extract.c`: Arşivi açıp dosyaları geri oluşturma.
- `validation.c`: ASCII kontrolü, null-byte reddi ve path traversal engelleme.
- `file_utils.c`: Klasör oluşturma (`mkdir`) işlemleri.

## .SAU Formatı
Arşiv dosyası iki kısımdan oluşur:
1. **Metadata:** İlk 10 byte metadata boyutunu tutar (örn: `0000000045`). Devamında dosyaların özellikleri yer alır: `|dosya_adi,izinler,boyut|`.
2. **Veri:** Metadata bittikten sonra orijinal dosyaların içerikleri ardışık olarak yazılır. Araya karakter girmez, byte sayısına göre okunur.

## Derleme Komutları
Makefile kullanılarak derlenir:
```bash
make          # projeyi derler
make clean    # object ve exe dosyalarını temizler
make debug    # asan (address sanitizer) aktif şekilde derler
make run      # çalıştırır
```

## Kullanım Örnekleri
**Arşivleme:**
```bash
./tarsau -b file1.txt file2.txt -o myarchive.sau
./tarsau -b t1 t2 t3.txt         # a.sau olarak çıkartır
```
**Çıkarma:**
```bash
./tarsau -a myarchive.sau hedef_klasor
./tarsau -a myarchive.sau        # mevcut klasöre çıkartır
```

## Güvenlik ve Limitler
- Sadece `../`, `/`, `\` gibi tehlikeli yollar engellendi (Path traversal koruması).
- `sprintf`, `strcpy` yerine `snprintf` kullanılarak buffer overflow engellendi.
- Nul byte (`\0`) ve 127'den büyük karakter içeren (UTF-8, binary) dosyalar reddedilir.
- En fazla 32 dosya eklenebilir.
- Toplam arşiv boyutu en fazla 200 MB olabilir.

## Memory Safety (Valgrind & ASAN)
- Tüm dinamik bellekler `free()` ile temizlendi.
- Valgrind üzerinde "All heap blocks were freed -- no leaks are possible" sonucunu veriyor.
- Double free veya invalid read/write hataları yoktur.

## Test Senaryoları
Aşağıdaki durumlar test edilmiştir:
- **Boş dosya arşivi:** Çalışıyor.
- **Çok büyük dosya:** Parçalı (8192 byte) okunarak sisteme yüklenmeden halledildi.
- **33 dosya sınırı:** "Maksimum 32 dosya kabul edilir" hatası veriyor.
- **Bozuk .sau:** Yanlış header veya eksik byte olduğunda anında çıkış yapıp uyarıyor.
- **UTF-8 içeren dosya:** Engelleniyor.
- **İzin Restore:** Linux `chmod` ile arşiv öncesindeki dosyaların izinleri birebir geri yükleniyor.
- **İç içe klasör oluşturma:** Olmayan bir klasör yolu girilirse sırayla oluşturuluyor.
