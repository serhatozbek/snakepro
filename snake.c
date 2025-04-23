/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   snake.c                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sozbek <sozbek@student.kocaeli.42.tr>      +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/04/23 01:25:39 by sozbek            #+#    #+#             */
/*   Updated: 2025/04/23 04:52:05 by sozbek           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <ncursesw/ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <locale.h>
#include <wchar.h>

#define DELAY 100000
#define INITIAL_SPEED DELAY
#define MAX_SPEED 50000
#define SPEED_INCREMENT 5000
#define GAME_WIDTH 40        // Genişletilmiş oyun alanı
#define GAME_HEIGHT 20       // Genişletilmiş oyun alanı
#define MAX_FOOD 8
#define INITIAL_LENGTH 4
#define MAX_OBSTACLES 15
#define BONUS_FOOD_CHANCE 20  // 1 in 20 chance for bonus food
#define BONUS_DURATION 30     // Bonus food stays for 30 cycles

// Emojiler
#define SNAKE_HEAD L"🐍"
#define SNAKE_BODY L"🐍"
#define NORMAL_FOOD L"🍎"
#define BONUS_FOOD L"🍏"
#define WALL L"🧱"
#define OBSTACLE L"🌵"
#define EMPTY L"⬛"
#define BACKGROUND L"⬜"

typedef struct {
    int x, y;
} Point;

typedef struct {
    Point position;
    int value;
    int is_bonus;
    int duration;
    wchar_t *symbol;
} Food;

typedef struct {
    Point position;
} Obstacle;

typedef enum {
    EASY = 1,
    MEDIUM = 2,
    HARD = 3
} Difficulty;

typedef enum {
    UP,
    DOWN,
    LEFT,
    RIGHT
} Direction;

typedef struct {
    Point *body;
    int length;
    int max_length;
    Direction direction;
    Direction next_direction;
    int speed;
    int lives;
} Snake;

typedef struct {
    int score;
    int high_score;
    int level;
    Difficulty difficulty;
    int game_over;
    int paused;
} GameState;

// Oyun bileşenleri
Snake snake;
Food foods[MAX_FOOD];
Obstacle obstacles[MAX_OBSTACLES];
GameState state;
int obstacle_count = 0;

// Fonksiyon prototipleri
void init_game();
void end_game();
void reset_game();
void draw_border();
void spawn_food(int index);
void initialize_foods();
void draw_foods();
void initialize_snake();
void draw_snake();
void move_snake();
int handle_collisions();
void draw_obstacles();
void spawn_obstacles();
void handle_input();
void update_game();
void draw_game();
void draw_stats();
void show_menu();
void show_game_over();
void set_difficulty(Difficulty diff);
void add_score(int value);
void draw_background();
void cleanup_ncurses();

int main() {
    setlocale(LC_ALL, "");
    init_game();
    
    show_menu();
    initialize_snake();
    initialize_foods();
    if (state.difficulty > EASY) {
        spawn_obstacles();
    }
    
    while (!state.game_over) {
        if (!state.paused) {
            handle_input();
            update_game();
        } else {
            mvprintw(GAME_HEIGHT / 2, (GAME_WIDTH - 16) / 2, "OYUN DURAKLATILDI");
            mvprintw(GAME_HEIGHT / 2 + 1, (GAME_WIDTH - 22) / 2, "Devam etmek için P'ye basın");
            int ch = getch();
            if (ch == 'p' || ch == 'P') {
                state.paused = 0;
            }
        }
        
        draw_game();
        usleep(snake.speed);
    }
    
    show_game_over();
    end_game();
    return 0;
}

void init_game() {
    initscr();
    start_color();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    srand(time(NULL));
    
    // Renk çiftlerini başlat
    init_pair(1, COLOR_GREEN, COLOR_BLACK);  // Yılan
    init_pair(2, COLOR_RED, COLOR_BLACK);    // Normal yem
    init_pair(3, COLOR_YELLOW, COLOR_BLACK); // Bonus yem
    init_pair(4, COLOR_WHITE, COLOR_BLACK);  // Duvarlar
    init_pair(5, COLOR_CYAN, COLOR_BLACK);   // Başlık
    init_pair(6, COLOR_MAGENTA, COLOR_BLACK); // Engeller
    init_pair(7, COLOR_BLUE, COLOR_BLACK);   // Puan/Seviye
    
    // Oyun durumunu başlat
    state.score = 0;
    state.high_score = 0;
    state.level = 1;
    state.difficulty = MEDIUM;
    state.game_over = 0;
    state.paused = 0;
    
    // Yılanı başlat
    snake.speed = INITIAL_SPEED;
    snake.lives = 3;
}

// NCurses kaynaklarını temizle
void cleanup_ncurses() {
    clear();
    refresh();
    endwin();
}

void end_game() {
    free(snake.body);
    snake.body = NULL;
    cleanup_ncurses();
}

void reset_game() {
    // Önceki yılanı temizle
    free(snake.body);
    snake.body = NULL;
    
    state.score = 0;
    state.level = 1;
    state.game_over = 0;
    state.paused = 0;
    
    snake.speed = INITIAL_SPEED;
    snake.lives = 3;
    
    initialize_snake();
    initialize_foods();
    if (state.difficulty > EASY) {
        spawn_obstacles();
    }
}

void initialize_snake() {
    snake.max_length = 100;  // Başlangıçta 100 segment için yer ayır
    snake.body = malloc(snake.max_length * sizeof(Point));
    if (snake.body == NULL) {
        endwin();
        fprintf(stderr, "Memory allocation error for snake body\n");
        exit(EXIT_FAILURE);
    }
    
    snake.length = INITIAL_LENGTH;
    snake.direction = RIGHT;
    snake.next_direction = RIGHT;
    
    // Yılanın başlangıç pozisyonu
    int center_x = GAME_WIDTH / 4;  // Sol tarafa doğru başlat
    int center_y = GAME_HEIGHT / 2;
    
    // Başlangıçta yılanın tüm parçalarını belirli konumlara yerleştir
    for (int i = 0; i < snake.length; i++) {
        snake.body[i].x = center_x - i;
        snake.body[i].y = center_y;
    }
}

void draw_snake() {
    // Yılanın başını çiz
    attron(COLOR_PAIR(1));
    mvaddwstr(snake.body[0].y, snake.body[0].x * 2, SNAKE_HEAD);
    
    // Yılanın gövdesini çiz
    for (int i = 1; i < snake.length; i++) {
        if (snake.body[i].x >= 0 && snake.body[i].x < GAME_WIDTH &&
            snake.body[i].y >= 1 && snake.body[i].y < GAME_HEIGHT - 1) {
            mvaddwstr(snake.body[i].y, snake.body[i].x * 2, SNAKE_BODY);
        }
    }
    attroff(COLOR_PAIR(1));
}

void move_snake() {
    // Yönü güncelle
    snake.direction = snake.next_direction;
    
    // Yeni baş pozisyonunu kaydet
    Point new_head = snake.body[0];
    
    // Yılanın başını hareket ettir
    switch (snake.direction) {
        case UP:
            new_head.y--;
            break;
        case DOWN:
            new_head.y++;
            break;
        case LEFT:
            new_head.x--;
            break;
        case RIGHT:
            new_head.x++;
            break;
    }
    
    // Ekran sınırlarından geçiş
    if (new_head.x < 1)
        new_head.x = GAME_WIDTH - 2;
    else if (new_head.x >= GAME_WIDTH - 1)
        new_head.x = 1;
        
    if (new_head.y < 1)
        new_head.y = GAME_HEIGHT - 2;
    else if (new_head.y >= GAME_HEIGHT - 1)
        new_head.y = 1;
    
    // Yılanın gövdesini hareket ettir (tüm gövde parçaları bir öncekinin pozisyonunu alır)
    for (int i = snake.length - 1; i > 0; i--) {
        snake.body[i] = snake.body[i - 1];
    }
    
    // Yeni baş pozisyonunu ekle
    snake.body[0] = new_head;
}

void initialize_foods() {
    for (int i = 0; i < MAX_FOOD; i++) {
        spawn_food(i);
    }
}

void spawn_food(int index) {
    // Bonus yiyecek olasılığı
    int is_bonus = (rand() % BONUS_FOOD_CHANCE == 0);
    
    do {
        foods[index].position.x = rand() % (GAME_WIDTH - 4) + 2;
        foods[index].position.y = rand() % (GAME_HEIGHT - 4) + 2;
        
        // Yılanın üzerinde mi kontrol et
        int on_snake = 0;
        for (int i = 0; i < snake.length; i++) {
            if (foods[index].position.x == snake.body[i].x && 
                foods[index].position.y == snake.body[i].y) {
                on_snake = 1;
                break;
            }
        }
        
        // Engellerin üzerinde mi kontrol et
        int on_obstacle = 0;
        for (int i = 0; i < obstacle_count; i++) {
            if (foods[index].position.x == obstacles[i].position.x && 
                foods[index].position.y == obstacles[i].position.y) {
                on_obstacle = 1;
                break;
            }
        }
        
        // Diğer yiyeceklerin üzerinde mi kontrol et
        int on_food = 0;
        for (int i = 0; i < MAX_FOOD; i++) {
            if (i != index && foods[index].position.x == foods[i].position.x && 
                foods[index].position.y == foods[i].position.y) {
                on_food = 1;
                break;
            }
        }
        
        if (!on_snake && !on_obstacle && !on_food) break;
    } while (1);
    
    // Özellikleri ayarla
    foods[index].is_bonus = is_bonus;
    foods[index].value = is_bonus ? 30 : 10;
    foods[index].duration = is_bonus ? BONUS_DURATION : -1;  // -1 = süresiz
    foods[index].symbol = is_bonus ? BONUS_FOOD : NORMAL_FOOD;
}

void draw_foods() {
    for (int i = 0; i < MAX_FOOD; i++) {
        // Bonus yiyeceklerin süresini kontrol et
        if (foods[i].is_bonus) {
            if (foods[i].duration > 0) {
                foods[i].duration--;
            } else if (foods[i].duration == 0) {
                spawn_food(i);
                continue;
            }
        }
        
        // Yiyeceği çiz
        attron(COLOR_PAIR(foods[i].is_bonus ? 3 : 2));
        mvaddwstr(foods[i].position.y, foods[i].position.x * 2, foods[i].symbol);
        attroff(COLOR_PAIR(foods[i].is_bonus ? 3 : 2));
    }
}

void spawn_obstacles() {
    obstacle_count = state.difficulty * 5;  // Zorluk seviyesine göre engel sayısı
    
    for (int i = 0; i < obstacle_count; i++) {
        int valid_position = 0;
        
        while (!valid_position) {
            obstacles[i].position.x = rand() % (GAME_WIDTH - 4) + 2;
            obstacles[i].position.y = rand() % (GAME_HEIGHT - 4) + 2;
            
            // Yılanın etrafında boşluk bırak
            int near_snake = 0;
            for (int j = 0; j < snake.length; j++) {
                if (abs(obstacles[i].position.x - snake.body[j].x) <= 2 && 
                    abs(obstacles[i].position.y - snake.body[j].y) <= 2) {
                    near_snake = 1;
                    break;
                }
            }
            
            // Diğer engellerle çakışma kontrolü
            int on_obstacle = 0;
            for (int j = 0; j < i; j++) {
                if (obstacles[i].position.x == obstacles[j].position.x && 
                    obstacles[i].position.y == obstacles[j].position.y) {
                    on_obstacle = 1;
                    break;
                }
            }
            
            // Yiyeceklerle çakışma kontrolü
            int on_food = 0;
            for (int j = 0; j < MAX_FOOD; j++) {
                if (obstacles[i].position.x == foods[j].position.x && 
                    obstacles[i].position.y == foods[j].position.y) {
                    on_food = 1;
                    break;
                }
            }
            
            if (!near_snake && !on_obstacle && !on_food) {
                valid_position = 1;
            }
        }
    }
}

void draw_obstacles() {
    attron(COLOR_PAIR(6));
    for (int i = 0; i < obstacle_count; i++) {
        mvaddwstr(obstacles[i].position.y, obstacles[i].position.x * 2, OBSTACLE);
    }
    attroff(COLOR_PAIR(6));
}

void draw_border() {
    attron(COLOR_PAIR(4));
    
    // Üst ve alt sınırlar
    for (int x = 0; x < GAME_WIDTH; x++) {
        mvaddwstr(0, x * 2, WALL);
        mvaddwstr(GAME_HEIGHT - 1, x * 2, WALL);
    }
    
    // Sol ve sağ sınırlar
    for (int y = 1; y < GAME_HEIGHT - 1; y++) {
        mvaddwstr(y, 0, WALL);
        mvaddwstr(y, (GAME_WIDTH - 1) * 2, WALL);
    }
    
    attroff(COLOR_PAIR(4));
}

void draw_background() {
    // Arkaplanı çiz
    for (int y = 1; y < GAME_HEIGHT - 1; y++) {
        for (int x = 1; x < GAME_WIDTH - 1; x++) {
            mvaddwstr(y, x * 2, BACKGROUND);
        }
    }
}

void handle_input() {
    int ch = getch();
    
    switch (ch) {
        case KEY_UP:
            if (snake.direction != DOWN) snake.next_direction = UP;
            break;
        case KEY_DOWN:
            if (snake.direction != UP) snake.next_direction = DOWN;
            break;
        case KEY_LEFT:
            if (snake.direction != RIGHT) snake.next_direction = LEFT;
            break;
        case KEY_RIGHT:
            if (snake.direction != LEFT) snake.next_direction = RIGHT;
            break;
        case 'p':
        case 'P':
            state.paused = !state.paused;
            break;
        case 'q':
        case 'Q':
            state.game_over = 1;
            break;
        case 'r':
        case 'R':
            reset_game();
            break;
    }
}

int handle_collisions() {
    // Engel çarpışması
    for (int i = 0; i < obstacle_count; i++) {
        if (snake.body[0].x == obstacles[i].position.x && 
            snake.body[0].y == obstacles[i].position.y) {
            snake.lives--;
            if (snake.lives <= 0) {
                return 1;  // Oyun bitti
            } else {
                // Yılanı başlangıç konumuna geri döndür
                free(snake.body);  // Önceki belleği temizle
                snake.body = NULL;
                initialize_snake();
                return 0;
            }
        }
    }
    
    // Kendi kuyruğuna çarpma kontrolü
    for (int i = 1; i < snake.length; i++) {
        if (snake.body[0].x == snake.body[i].x && snake.body[0].y == snake.body[i].y) {
            snake.lives--;
            if (snake.lives <= 0) {
                return 1;  // Oyun bitti
            } else {
                // Yılanı başlangıç konumuna geri döndür
                free(snake.body);  // Önceki belleği temizle
                snake.body = NULL;
                initialize_snake();
                return 0;
            }
        }
    }
    
    // Yiyecek yeme kontrolü
    for (int i = 0; i < MAX_FOOD; i++) {
        if (snake.body[0].x == foods[i].position.x && 
            snake.body[0].y == foods[i].position.y) {
            // Puanı artır
            add_score(foods[i].value);
            
            // Yılanı büyüt
            if (snake.length < snake.max_length) {
                // Yılanın sonuna yeni bir segment ekle
                // Önceki segment pozisyonuyla aynı (sonraki adımda düzeltilecek)
                snake.body[snake.length] = snake.body[snake.length - 1];
                snake.length++;
            } else {
                // Eğer maksimum uzunluğa ulaşıldıysa, yeni bellek ayır
                Point *new_body = realloc(snake.body, (snake.max_length * 2) * sizeof(Point));
                if (new_body == NULL) {
                    free(snake.body);  // Realloc başarısız olursa eski belleği temizle
                    endwin();
                    fprintf(stderr, "Memory allocation error during snake growth\n");
                    exit(EXIT_FAILURE);
                }
                snake.body = new_body;
                snake.max_length *= 2;
                // Yeni segmenti ekle
                snake.body[snake.length] = snake.body[snake.length - 1];
                snake.length++;
            }
            
            // Yeni yiyecek oluştur
            spawn_food(i);
            
            // Seviye kontrolü
            if (state.score >= state.level * 100) {
                state.level++;
                if (snake.speed > MAX_SPEED) {
                    snake.speed -= SPEED_INCREMENT;  // Oyunu hızlandır
                }
                
                // Yeni engeller ekle (zorluk seviyesi MEDIUM ve üzeri için)
                if (state.difficulty > EASY && obstacle_count < MAX_OBSTACLES) {
                    obstacles[obstacle_count].position.x = rand() % (GAME_WIDTH - 4) + 2;
                    obstacles[obstacle_count].position.y = rand() % (GAME_HEIGHT - 4) + 2;
                    obstacle_count++;
                }
            }
            
            break;
        }
    }
    
    return 0;
}

void update_game() {
    move_snake();
    
    if (handle_collisions()) {
        state.game_over = 1;
    }
}

void draw_game() {
    
    
    draw_background();
    draw_border();
    draw_obstacles();
    draw_foods();
    draw_snake();
    draw_stats();
    
    refresh();
}

void draw_stats() {
    attron(COLOR_PAIR(7));
    mvprintw(GAME_HEIGHT, 2, "Puan: %d | Seviye: %d | Canlar: %d | ", state.score, state.level, snake.lives);
    
    // Zorluk seviyesini göster
    const char* diff_text;
    switch (state.difficulty) {
        case EASY: diff_text = "Kolay"; break;
        case MEDIUM: diff_text = "Orta"; break;
        case HARD: diff_text = "Zor"; break;
        default: diff_text = "Bilinmiyor";
    }
    printw("Zorluk: %s\n Kontroller: Yön tuşları, P:Duraklat, R:Yeniden başlat, Q:Çıkış", diff_text);
    attroff(COLOR_PAIR(7));
}

void show_menu() {
    clear();
    nodelay(stdscr, FALSE);  // Tuş basımını bekle
    
    // Oyun başlık
    attron(COLOR_PAIR(2));
    mvprintw(GAME_HEIGHT / 2 - 5, (GAME_WIDTH * 2 - 22) / 2, "*** SÜPER SNAKE OYUNU ***");
    attroff(COLOR_PAIR(2));
    
    // Menü seçenekleri
    mvprintw(GAME_HEIGHT / 2 - 2, (GAME_WIDTH * 2 - 20) / 2, "Zorluk Seviyesi Seçin:");
    mvprintw(GAME_HEIGHT / 2, (GAME_WIDTH * 2 - 6) / 2, "1: Kolay");
    mvprintw(GAME_HEIGHT / 2 + 1, (GAME_WIDTH * 2 - 6) / 2, "2: Orta");
    mvprintw(GAME_HEIGHT / 2 + 2, (GAME_WIDTH * 2 - 6) / 2, "3: Zor");
    
    // Seçim bekle
    int ch;
    while (1) {
        ch = getch();
        if (ch >= '1' && ch <= '3') {
            set_difficulty((Difficulty)(ch - '0'));
            break;
        }
    }
    
    nodelay(stdscr, TRUE);  // Tuş basımını bekleme
}

void set_difficulty(Difficulty diff) {
    state.difficulty = diff;
    
    // Zorluğa göre ayarları yap
    switch (diff) {
        case EASY:
            snake.speed = INITIAL_SPEED + 20000;  // Daha yavaş
            snake.lives = 5;                      // Daha fazla can
            break;
        case MEDIUM:
            snake.speed = INITIAL_SPEED;
            snake.lives = 3;
            break;
        case HARD:
            snake.speed = INITIAL_SPEED - 20000;  // Daha hızlı
            snake.lives = 2;                      // Daha az can
            obstacle_count = 10;                  // Daha fazla engel
            break;
    }
}

void show_game_over() {
    clear();
    nodelay(stdscr, FALSE);  // Tuş basımını bekle
    
    // Yüksek skoru güncelle
    if (state.score > state.high_score) {
        state.high_score = state.score;
    }
    
    // Oyun sonu mesajı
    attron(COLOR_PAIR(2));
    mvprintw(GAME_HEIGHT / 2 - 3, (GAME_WIDTH * 2 - 18) / 2, "***  OYUN BİTTİ  ***");
    attroff(COLOR_PAIR(2));
    
    mvprintw(GAME_HEIGHT / 2 - 1, (GAME_WIDTH * 2 - 20) / 2, "Skorunuz: %d", state.score);
    mvprintw(GAME_HEIGHT / 2, (GAME_WIDTH * 2 - 20) / 2, "Yüksek Skor: %d", state.high_score);
    mvprintw(GAME_HEIGHT / 2 + 1, (GAME_WIDTH * 2 - 20) / 2, "Ulaşılan Seviye: %d", state.level);
    
    mvprintw(GAME_HEIGHT / 2 + 3, (GAME_WIDTH * 2 - 28) / 2, "Tekrar oynamak için R'ye basın");
    mvprintw(GAME_HEIGHT / 2 + 4, (GAME_WIDTH * 2 - 25) / 2, "Çıkmak için Q'ya basın");
    
    // Tuş bekle
    int ch;
    while (1) {
        ch = getch();
        if (ch == 'r' || ch == 'R') {
            reset_game();
            break;
        } else if (ch == 'q' || ch == 'Q') {
            break;
        }
    }
}

void add_score(int value) {
    state.score += value;
    
    // Bonus puan efektleri
    if (value >= 30) {
        // Bonus yiyecek alındığında özel ödül
        if (snake.lives < 5) {  // Maksimum can sınırı
            snake.lives++;  // Ekstra can
        }
    }
}