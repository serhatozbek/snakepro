# 🐍 Süper Snake Oyunu

## Proje Hakkında
Bu proje, klasik Snake (Yılan) oyununun terminal tabanlı gelişmiş bir versiyonudur. ncurses kütüphanesi kullanılarak C dilinde geliştirilmiştir ve emoji karakterler ile zengin bir görsel deneyim sunar.

## Özellikler

- 🎮 Emoji tabanlı grafikler (yılan, yemler, engeller)
- 🏆 Puan sistemi ve yüksek puan takibi
- ⚙️ Üç zorluk seviyesi (Kolay, Orta, Zor)
- 🍎 Normal ve bonus yemler
- 🌵 Zorluk seviyesine göre engeller
- 🔄 Seviye atlama sistemi
- ❤️ Çoklu can sistemi
- ⏸️ Oyunu duraklatma özelliği

## Gereksinimler

- GCC veya başka bir C derleyicisi
- ncursesw kütüphanesi
- Unicode/emoji desteği olan bir terminal

## Kurulum

### Linux / macOS

```bash
# ncurses kütüphanesini yükleyin
# Debian/Ubuntu:
sudo apt-get install libncursesw5-dev

# macOS:
brew install ncurses

# Projeyi derleyin
gcc -o snake snake.c -lncursesw

# Oyunu çalıştırın
./snake
```

### Windows (WSL veya MinGW ile)

Windows'ta WSL (Windows Subsystem for Linux) kullanarak Linux kurulum adımlarını takip edebilir veya MinGW ile derleyebilirsiniz:

```bash
# MinGW ile:
gcc -o snake.exe snake.c -lncursesw
```

## Oyun Kontrolleri

- 🔼 **Yukarı Ok**: Yukarı git
- 🔽 **Aşağı Ok**: Aşağı git
- ◀️ **Sol Ok**: Sola git
- ▶️ **Sağ Ok**: Sağa git
- **P tuşu**: Oyunu duraklat/devam ettir
- **R tuşu**: Oyunu sıfırla
- **Q tuşu**: Oyundan çık

## Oyun Mekanikleri

### Zorluk Seviyeleri

1. **Kolay**:
   - Daha yavaş oyun hızı
   - 5 can
   - Engel yok

2. **Orta**:
   - Normal oyun hızı
   - 3 can
   - Az sayıda engel

3. **Zor**:
   - Hızlı oyun temposu
   - 2 can
   - Çok sayıda engel

### Yemler

- 🍎 **Normal Yem**: 10 puan kazandırır
- 🍏 **Bonus Yem**: 30 puan kazandırır ve sınırlı süre oyunda kalır. Yenildiğinde ekstra can verebilir.

### Seviye Atlama

Her 100 puan toplandığında bir sonraki seviyeye geçilir. Seviye atlandıkça:
- Oyun hızı artar
- Orta ve Zor seviyede yeni engeller eklenir

## Kod Yapısı

- Oyun, standart ncurses döngüsü üzerine kurulmuştur
- Modüler fonksiyon yapısı ile kod organizasyonu sağlanmıştır
- Oyun elementleri (yılan, yemler, engeller) ayrı yapılarda tutulur
- Emojiler için geniş karakter desteği (wchar_t) kullanılmıştır

## Özelleştirme

Oyunu özelleştirmek için `snake.c` dosyasının başındaki tanımları değiştirebilirsiniz:

```c
#define DELAY 100000            // Oyun hızını ayarlar (daha düşük = daha hızlı)
#define GAME_WIDTH 40           // Oyun alanı genişliği
#define GAME_HEIGHT 20          // Oyun alanı yüksekliği
#define MAX_FOOD 8              // Maksimum yiyecek sayısı
#define BONUS_FOOD_CHANCE 20    // Bonus yiyecek çıkma olasılığı (1/20)
```

Ayrıca emojileri değiştirerek görsel stili özelleştirebilirsiniz:

```c
#define SNAKE_HEAD L"🐍"     // Yılan başı
#define SNAKE_BODY L"🐍"     // Yılan gövdesi
#define NORMAL_FOOD L"🍎"    // Normal yiyecek
#define BONUS_FOOD L"🍏"     // Bonus yiyecek
```

## Bilinen Sorunlar ve Sınırlamalar

- Bazı terminallerde emoji desteği düzgün çalışmayabilir
- Windows CMD'de Unicode karakterleri doğru görüntülenmeyebilir

## Gelecek Geliştirmeler

- Oyun durumunu kaydetme/yükleme özelliği
- Çoklu oyuncu modu
- Özel harita desteği
- Daha fazla güç-yükseltme (power-up) türü
- Ses efektleri

## Lisans

Bu proje açık kaynak olarak sunulmuştur.

## Katkıda Bulunanlar

- sozbek (Geliştirici)

---

Happy coding and happy gaming! 🎮
