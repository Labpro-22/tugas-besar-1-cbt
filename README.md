# Nimonspoli

**IF2010 Pemrograman Berorientasi Objek**

## Kontributor

**Kelompok CapekBanget**

| Nama | NIM |
|------|-----|
| Nathan Adhika Santosa | 13524041 |
| Farrell Limjaya| 13524046 |
| Renuno Yuqa Frinardi | 13524080 |
| Valentino Daniel Kusumo | 13524104 |
| Michael James Liman | 13524106 |

---

## Ringkasan

Nimonspoli adalah permainan strategi yang memungkinkan pemain membeli properti, membayar sewa, mengikuti lelang, hingga mengelola kekayaan sambil berkeliling papan permainan.

## Struktur Proyek

```text
tugas-besar-1-cbt/
├── .gitignore
├── README.md
├── makefile
├── bin/                          # Hasil kompilasi (executable)
├── build/                        # Berkas objek (.o)
├── config/                       # Berkas konfigurasi permainan
│   ├── aksi.txt
│   ├── misc.txt
│   ├── property.txt
│   ├── railroad.txt
│   ├── special.txt
│   ├── tax.txt
│   └── utility.txt
├── docs/                         # Laporan dan dokumen proyek
├── include/                      # Deklarasi kode header (.hpp)
│   ├── app/
│   ├── core/
│   ├── data/
│   ├── exception/
│   ├── gui/
│   ├── models/
│   ├── utils/
│   └── views/
├── raylib/                       # Library eksternal (GUI)
└── src/                          # Berkas kode sumber (.cpp)
    ├── main.cpp
    ├── app/
    ├── core/
    ├── data/
    ├── exception/
    ├── gui/
    ├── models/
    ├── utils/
    └── views/
```

## Instalasi

### Requirement
- Kompiler GCC (disarankan: GCC 9.0+)
- Utilitas Make
- Lingkungan Linux/WSL/macOS

### Petunjuk Build

1. **Clone repositori**

2. **Jalankan program**
```bash
make run
```

3. **Hapus berkas hasil build**
```bash
make clean
```