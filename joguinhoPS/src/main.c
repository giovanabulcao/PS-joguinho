#include <string.h>
#include "wasm4.h"
#include "tiles4.h"
#include "util.h"

// sons do jogo
#define SFX_SELECT 0
#define SFX_MOVE   1
#define SFX_SHARD  2
#define SFX_HURT   3
#define SFX_WIN    4
#define SFX_BLOCK  5

void play(int id) {
    if (id == SFX_SELECT) {
        int attack = 12;
        int decay = 6;
        int duracao = attack + decay * 256;
        tone(880, duracao, 80, TONE_PULSE1 | TONE_MODE3);
    }
    else if (id == SFX_MOVE) {
        tone(700, 6, 70, TONE_TRIANGLE);
    }
    else if (id == SFX_SHARD) {
        int freq1 = 988;
        int freq2 = 1319;
        int freq = freq1 + freq2 * 65536;
        int attack = 8;
        int decay = 18;
        int duracao = attack + decay * 256;
        tone(freq, duracao, 90, TONE_PULSE1 | TONE_MODE2);
    }
    else if (id == SFX_HURT) {
        int freq1 = 440;
        int freq2 = 220;
        int freq = freq1 + freq2 * 65536;
        int attack = 18;
        int decay = 10;
        int duracao = attack + decay * 256;
        tone(freq, duracao, 90, TONE_PULSE1 | TONE_MODE2);
    }
    else if (id == SFX_WIN) {
        int freq1 = 523;
        int freq2 = 1047;
        int freq = freq1 + freq2 * 65536;
        int attack = 30;
        int decay = 20;
        int duracao = attack + decay * 256;
        tone(freq, duracao, 90, TONE_PULSE1 | TONE_MODE2);
    }
    else if (id == SFX_BLOCK) {
        int attack = 10;
        int decay = 6;
        int duracao = attack + decay * 256;
        tone(800, duracao, 80, TONE_NOISE);
    }
}

// telas do jogo
#define TELA_MENU     0
#define TELA_HISTORIA 1
#define TELA_DUNGEON  2
#define TELA_GAMEOVER 3
#define TELA_WIN      4


// mapa
#define TILE  8
#define COLS 20
#define ROWS 20

#define T_FLOOR  0
#define T_WALL   1
#define T_SHARD  2
#define T_EXIT   3

#define TOTAL_SHARDS 6
#define NUM_ENEMIES  6

// cores
#define COR_FUNDO    0x2e222f  
#define COR_PAREDE   0x9babb2  
#define COR_NINJA    0xc32454 
#define COR_BRANCO   0xffffff  


static const int MAP[ROWS][COLS] = {
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    {1,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,1},
    {1,0,1,1,0,0,1,0,1,1,1,0,1,1,1,1,0,1,0,1},
    {1,2,1,0,0,0,0,0,0,0,1,0,0,0,0,1,0,1,0,1},
    {1,0,1,0,1,1,1,1,0,0,1,0,1,1,0,1,0,0,0,1},
    {1,0,0,0,1,2,0,1,0,0,0,0,1,0,0,0,0,1,0,1},
    {1,0,1,0,1,0,0,1,0,1,1,1,1,0,1,1,0,1,0,1},
    {1,0,1,0,0,0,0,0,0,1,0,0,0,0,1,0,0,1,2,1},
    {1,0,1,1,1,1,0,1,1,1,0,1,1,0,1,0,1,1,0,1},
    {1,0,0,0,0,1,0,0,0,0,0,0,1,0,0,0,0,0,0,1},
    {1,1,1,0,0,1,0,1,1,0,1,0,1,0,1,1,1,0,1,1},
    {1,0,0,0,1,1,0,1,0,0,1,0,0,0,0,0,1,0,0,1},
    {1,0,1,0,0,0,0,0,0,1,1,1,0,1,1,0,1,0,1,1},
    {1,0,1,1,0,1,1,1,0,0,0,1,0,1,0,0,0,0,2,1},
    {1,0,0,1,0,0,0,1,0,1,0,1,0,0,0,1,1,1,0,1},
    {1,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,1},
    {1,0,0,1,0,1,1,1,1,1,0,1,1,1,0,1,0,1,0,1},
    {1,0,1,1,0,0,0,0,0,0,0,0,0,0,0,1,2,1,0,1},
    {1,0,0,0,0,1,0,2,0,1,0,0,0,0,0,0,0,0,3,1},
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

static int map[ROWS][COLS];
static int frames;


// estado do jogo
static int telaAtual;
static int pcol, prow;     // posicao do jogador (coluna e linha)
static int fragmentos;
static int msg_timer;
static char msg[32];
static int prevBtn;
static int historiaPag;

// inimigos
// axis: 0 = anda na horizontal, 1 = anda na vertical
// dir: 0 = anda pro lado positivo, 1 = anda pro lado negativo
// chasing: 0 = so patrulhando, 1 = perseguindo o jogador
#define CHASE_RADIUS   5
#define CHASE_SPEED   15

typedef struct {
    int col, row;
    int dir;
    int axis;
    int timer;
    int speed;
    int chasing;
} Enemy;

static Enemy enemies[NUM_ENEMIES];

static int map_at(int col, int row) {
    if (col < 0 || col >= COLS || row < 0 || row >= ROWS) {
        return T_WALL;
    }
    return map[row][col];
}

static void set_msg(const char *s) {
    int i = 0;
    while (s[i] != '\0' && i < 31) {
        msg[i] = s[i];
        i = i + 1;
    }
    msg[i] = '\0';
    msg_timer = 90;
}

// centraliza um texto na tela
static void text_centered(const char *s, int y, uint16_t color) {
    int tamanho = strlen(s);
    int x = (SCREEN_SIZE - tamanho * FONT_SIZE) / 2;
    *DRAW_COLORS = color;
    text(s, x, y);
}

// texto com uma caixinha de fundo atras
static void draw_text_box(const char *s, int x, int y, uint16_t fg, uint16_t bg) {
    int tamanho = strlen(s);

    int padX = 3;
    int padY = 2;
    int largura = tamanho * FONT_SIZE + padX * 2;
    int altura = FONT_SIZE + padY * 2;

    *DRAW_COLORS = bg;
    rect(x - padX, y - padY, largura, altura);

    *DRAW_COLORS = fg;
    text(s, x, y);
}

// balao de fala da historia, com borda e um rabinho embaixo
static void draw_speech_box(const char *l1, const char *l2, const char *l3,
                             int x, int y, int w, int h) {
    *DRAW_COLORS = 4;
    rect(x, y, w, h);

    *DRAW_COLORS = 2;
    rect(x, y, w, 2);
    rect(x, y + h - 2, w, 2);
    rect(x, y, 2, h);
    rect(x + w - 2, y, 2, h);

    int tx = x + 14;
    int ty = y + h;
    rect(tx, ty, 10, 8);
    *DRAW_COLORS = 4;
    rect(tx + 2, ty, 6, 6);

    *DRAW_COLORS = 2;
    if (l1) text(l1, x + 8, y + 8);
    if (l2) text(l2, x + 8, y + 8 + (FONT_SIZE + 4));
    if (l3) text(l3, x + 8, y + 8 + (FONT_SIZE + 4) * 2);
}

// le a cor (0 a 3) de um pixel de uma sprite da tiles4
static int tile_pixel(int x, int y, int width) {
    int posicao = y * width + x;
    int indiceByte = posicao / 4;
    int pixelDoByte = posicao % 4;

    int valor = tiles4[indiceByte];

    // cada byte guarda 4 pixels, o primeiro pixel fica na parte mais
    // significativa do byte
    if (pixelDoByte == 0) {
        return (valor / 64) % 4;
    } else if (pixelDoByte == 1) {
        return (valor / 16) % 4;
    } else if (pixelDoByte == 2) {
        return (valor / 4) % 4;
    } else {
        return valor % 4;
    }
}

// desenha uma sprite 8x8 da tiles4 aumentada, usando o mesmo esquema
// de cor (colorMap) que o resto do jogo usa no DRAW_COLORS
static void draw_sprite_scaled(int dx, int dy, int srcX, int srcY, int scale, uint16_t colorMap) {
    for (int sy = 0; sy < 8; sy++) {
        for (int sx = 0; sx < 8; sx++) {
            int v = tile_pixel(srcX + sx, srcY + sy, (int)tiles4Width);

            int cor;
            if (v == 0) {
                cor = colorMap % 16;
            } else if (v == 1) {
                cor = (colorMap / 16) % 16;
            } else if (v == 2) {
                cor = (colorMap / 256) % 16;
            } else {
                cor = (colorMap / 4096) % 16;
            }

            if (cor == 0) {
                continue; // transparente
            }

            *DRAW_COLORS = (uint16_t)cor;
            rect(dx + sx * scale, dy + sy * scale, scale, scale);
        }
    }
}

// retrato do ninja pra tela de historia
static void draw_hero_portrait(int x, int y, int scale) {
    int c = 96;
    int srcX = (c % 16) * 8;
    int srcY = (c / 16) * 8;
    draw_sprite_scaled(x, y, srcX, srcY, scale, 0x4230);
}

// retrato da caveirinha inimiga pra tela de historia
static void draw_skull_portrait(int x, int y, int scale, int frame) {
    int c = 112 + (frame % 2);
    int srcX = (c % 16) * 8;
    int srcY = (c / 16) * 8;
    draw_sprite_scaled(x, y, srcX, srcY, scale, 0x4010);
}


void start() {
    PALETTE[0] = COR_FUNDO;
    PALETTE[1] = COR_PAREDE;
    PALETTE[2] = COR_NINJA;
    PALETTE[3] = COR_BRANCO;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            map[r][c] = MAP[r][c];
        }
    }

    pcol = 1;
    prow = 1;

    fragmentos = 0;
    frames     = 0;
    msg[0]     = '\0';
    msg_timer  = 0;
    prevBtn    = 0;
    telaAtual  = TELA_MENU;

    enemies[0].col = 7;  enemies[0].row = 2;
    enemies[0].dir = 0;  enemies[0].axis = 0; enemies[0].speed = 18;

    enemies[1].col = 18; enemies[1].row = 7;
    enemies[1].dir = 0;  enemies[1].axis = 1; enemies[1].speed = 20;

    enemies[2].col = 10; enemies[2].row = 9;
    enemies[2].dir = 0;  enemies[2].axis = 0; enemies[2].speed = 16;

    enemies[3].col = 12; enemies[3].row = 13;
    enemies[3].dir = 0;  enemies[3].axis = 1; enemies[3].speed = 20;

    enemies[4].col = 7;  enemies[4].row = 17;
    enemies[4].dir = 0;  enemies[4].axis = 0; enemies[4].speed = 14;

    enemies[5].col = 14; enemies[5].row = 16;
    enemies[5].dir = 0;  enemies[5].axis = 1; enemies[5].speed = 17;

    for (int i = 0; i < NUM_ENEMIES; i++) {
        enemies[i].timer   = 0;
        enemies[i].chasing = 0;
    }
}

// tela de historia
#define HIST_PAGINAS 3

#define HIST_BOX_X 6
#define HIST_BOX_Y 48
#define HIST_BOX_W 148
#define HIST_BOX_H 54
#define HIST_DECO_X 128
#define HIST_DECO_Y 16

static void draw_historia_fundo(int pag) {
    // pinta a tela toda com a cor de fundo
    *DRAW_COLORS = 1;
    rect(0, 0, 160, 160);

    // umas pedrinhas decorando em cima e embaixo
    *DRAW_COLORS = 2;
    for (int c = 0; c < COLS; c++) {
        rect(c * TILE, 0, TILE - 1, TILE - 1);
        rect(c * TILE, 152, TILE - 1, TILE - 1);
    }

    // brilho de fragmento na segunda pagina
    if (pag == 1) {
        *DRAW_COLORS = 4;
        rect(HIST_DECO_X + 3, HIST_DECO_Y + 0, 2, 8);
        rect(HIST_DECO_X + 0, HIST_DECO_Y + 3, 8, 2);
        rect(HIST_DECO_X + 3, HIST_DECO_Y + 3, 2, 2);
    }

    // inimigo espreitando na terceira pagina
    if (pag == 2) {
        draw_skull_portrait(HIST_DECO_X, HIST_DECO_Y, 2, frames / 10);
    }
}

static void update_historia() {
    uint8_t btn = *GAMEPAD1;
    uint8_t pressed = btn & ~(uint8_t)prevBtn;
    prevBtn = btn;

    draw_historia_fundo(historiaPag);

    draw_hero_portrait(68, 14, 3);

    if (historiaPag == 0) {
        draw_speech_box("I'm Kiro, hunter",
                         "of cursed loot,",
                         "and bad puns!",
                         HIST_BOX_X, HIST_BOX_Y, HIST_BOX_W, HIST_BOX_H);
    } else if (historiaPag == 1) {
        draw_speech_box("Rumor has it,",
                         "shiny shards",
                         "hide down here.",
                         HIST_BOX_X, HIST_BOX_Y, HIST_BOX_W, HIST_BOX_H);
    } else {
        draw_speech_box("Only one catch:",
                         "it's crawling",
                         "with angry mobs!",
                         HIST_BOX_X, HIST_BOX_Y, HIST_BOX_W, HIST_BOX_H);
    }
<<<<<<< Updated upstream
 
    text_centered("DUNGEONS OF SHARDS", 130, 4);
 
=======

    text_centered("MAZE ESCAPE", 130, 4);

>>>>>>> Stashed changes
    if ((frames / 20) % 2 == 0) {
        text_centered("Press a button", 144, 3);
    }

    if (pressed != 0) {
        if (historiaPag < HIST_PAGINAS - 1) {
            play(SFX_MOVE);
            historiaPag = historiaPag + 1;
        } else {
            play(SFX_SELECT);
            telaAtual = TELA_DUNGEON;
        }
    }
}

static void update_menu() {
    uint8_t btn = *GAMEPAD1;
    uint8_t pressed = btn & ~(uint8_t)prevBtn;
    prevBtn = btn;

    // pinta a tela toda com a cor 2 (azul medio)
    *DRAW_COLORS = 2;
    rect(0, 0, 160, 160);

    text_centered("------------", 22, 3);
    text_centered("DUNGEONS OF SHARDS", 32, 4);
    text_centered("------------", 42, 3);

    text_centered("CONTROLS:", 62, 4);
    // \x84 \x85 \x86 \x87 sao as setas, \x80 e o botao X
    text("\x84\x85\x86\x87 - Move", 14, 80);
    text("\x80 - Collect shard", 14, 90);

    if ((frames / 20) % 2 == 0) {
        text_centered("Press \x80 to play", 118, 4);
    }

    if (pressed & BUTTON_1) {
        play(SFX_SELECT);
        telaAtual = TELA_HISTORIA;
    }
}


// tela da dungeon
static void draw_map() {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int t  = map[r][c];
            int sx = c * TILE;
            int sy = r * TILE;

            if (t == T_WALL) {
                *DRAW_COLORS = 0x22;
                rect(sx, sy, TILE, TILE);
                *DRAW_COLORS = 4;
                rect(sx, sy, TILE - 1, 1);
                rect(sx, sy, 1, TILE - 1);
            } else if (t == T_SHARD) {
                *DRAW_COLORS = 4;
                rect(sx + 3, sy + 0, 2, 8);
                rect(sx + 0, sy + 3, 8, 2);
                rect(sx + 3, sy + 3, 2, 2);
            } else if (t == T_EXIT) {
                if ((frames / 14) % 2 == 0) {
                    *DRAW_COLORS = 0x22;
                } else {
                    *DRAW_COLORS = 0x44;
                }
                rect(sx, sy, TILE, TILE);
            }
        }
    }
}

static void draw_player() {
    int sx = pcol * TILE;
    int sy = prow * TILE;
    int c = 96 + (frames / 10) % 2; // alterna 2 quadros pra animar o passo

    *DRAW_COLORS = 0x4230;
    blitSub(tiles4, sx, sy, 8, 8,
            (uint32_t)(c % 16) * 8, (uint32_t)(c / 16) * 8,
            tiles4Width, BLIT_2BPP);
}

static void draw_enemies() {
    for (int i = 0; i < NUM_ENEMIES; i++) {
        int sx = enemies[i].col * TILE;
        int sy = enemies[i].row * TILE;
        int c = 112 + (frames / 10 + i) % 2;

        *DRAW_COLORS = 0x4010;
        blitSub(tiles4, sx, sy, 8, 8,
                (uint32_t)(c % 16) * 8, (uint32_t)(c / 16) * 8,
                tiles4Width, BLIT_2BPP);
    }
}

static void draw_hud() {
    char buf[16];

    *DRAW_COLORS = 0x11;
    rect(0, 0, 160, 11);

    *DRAW_COLORS = 3;
    text("Shards:", 2, 1);
    *DRAW_COLORS = 4;
    itoa(buf, fragmentos);
    text(buf, 58, 1);
    *DRAW_COLORS = 3;
    text("/6", 66, 1);

    if (msg_timer > 0) {
        draw_text_box(msg, 6, 146, 4, 1);
    }
}

static void update_enemies() {
    for (int i = 0; i < NUM_ENEMIES; i++) {

        int dc = pcol - enemies[i].col;
        int dr = prow - enemies[i].row;
        if (dc < 0) dc = -dc;
        if (dr < 0) dr = -dr;
        int distancia = dc + dr;

        if (distancia <= CHASE_RADIUS) {
            enemies[i].chasing = 1;
        } else {
            enemies[i].chasing = 0;
        }

        int velocidade = enemies[i].speed;
        if (enemies[i].chasing == 1) {
            velocidade = CHASE_SPEED;
        }

        enemies[i].timer = enemies[i].timer + 1;
        if (enemies[i].timer >= velocidade) {
            enemies[i].timer = 0;

            if (enemies[i].chasing == 1) {
                // persegue o jogador: primeiro tenta alinhar a coluna,
                // depois a linha
                int nc = enemies[i].col;
                int nr = enemies[i].row;

                if (pcol > enemies[i].col) {
                    nc = nc + 1;
                } else if (pcol < enemies[i].col) {
                    nc = nc - 1;
                } else if (prow > enemies[i].row) {
                    nr = nr + 1;
                } else if (prow < enemies[i].row) {
                    nr = nr - 1;
                }

                if (map_at(nc, nr) != T_WALL) {
                    enemies[i].col = nc;
                    enemies[i].row = nr;
                }
            } else {
                // patrulha: anda sempre pro mesmo lado e vira quando
                // bate numa parede
                int nc = enemies[i].col;
                int nr = enemies[i].row;

                if (enemies[i].axis == 0) {
                    if (enemies[i].dir == 0) {
                        nc = nc + 1;
                    } else {
                        nc = nc - 1;
                    }

                    if (map_at(nc, nr) == T_WALL) {
                        if (enemies[i].dir == 0) {
                            enemies[i].dir = 1;
                        } else {
                            enemies[i].dir = 0;
                        }
                    } else {
                        enemies[i].col = nc;
                    }
                } else {
                    if (enemies[i].dir == 0) {
                        nr = nr + 1;
                    } else {
                        nr = nr - 1;
                    }

                    if (map_at(nc, nr) == T_WALL) {
                        if (enemies[i].dir == 0) {
                            enemies[i].dir = 1;
                        } else {
                            enemies[i].dir = 0;
                        }
                    } else {
                        enemies[i].row = nr;
                    }
                }
            }
        }

        // checa colisao com o jogador todo frame, senao da pra passar
        // por cima de um inimigo parado sem morrer
        if (enemies[i].col == pcol && enemies[i].row == prow) {
            play(SFX_HURT);
            telaAtual = TELA_GAMEOVER;
        }
    }
}

static void update_dungeon() {
    PALETTE[0] = COR_FUNDO;

    uint8_t btn = *GAMEPAD1;
    uint8_t pressed = btn & ~(uint8_t)prevBtn;
    prevBtn = btn;

    // move um tile por vez, so quando o botao acabou de ser apertado
    int dc = 0, dr = 0;
    if (pressed & BUTTON_RIGHT) dc =  1;
    if (pressed & BUTTON_LEFT)  dc = -1;
    if (pressed & BUTTON_DOWN)  dr =  1;
    if (pressed & BUTTON_UP)    dr = -1;

    int nc = pcol + dc;
    int nr = prow + dr;

    if (map_at(nc, nr) != T_WALL) {
        pcol = nc;
        prow = nr;
    }

    // pega o shard apertando X em cima dele
    if (pressed & BUTTON_1) {
        if (map_at(pcol, prow) == T_SHARD) {
            play(SFX_SHARD);
            fragmentos = fragmentos + 1;
            map[prow][pcol] = T_FLOOR;
            set_msg("Shard collected!");
        }
    }

    // tambem pega o shard so de andar em cima
    if (map_at(pcol, prow) == T_SHARD) {
        play(SFX_SHARD);
        fragmentos = fragmentos + 1;
        map[prow][pcol] = T_FLOOR;
        set_msg("Shard collected!");
    }

    if (map_at(pcol, prow) == T_EXIT) {
        if (fragmentos >= TOTAL_SHARDS) {
            if (dc != 0 || dr != 0) play(SFX_WIN);
            telaAtual = TELA_WIN;
        } else {
            if (dc != 0 || dr != 0) play(SFX_BLOCK);
            set_msg("Need all 6 shards!");
        }
    }

    if (msg_timer > 0) {
        msg_timer = msg_timer - 1;
    }

    update_enemies();
    draw_map();
    draw_enemies();
    draw_player();
    draw_hud();
}

// tela de game over
static void update_gameover() {
    PALETTE[0] = COR_FUNDO;
    *DRAW_COLORS = 0x11;
    rect(0, 0, 160, 160);

    text_centered("GAME OVER", 50, 2);
    text_centered("Caught by enemy!", 64, 3);

    char linha[24];
    char num[16];
    itoa(num, fragmentos);
    strcpy(linha, "Shards: ");
    strcat(linha, num);
    strcat(linha, "/6");
    text_centered(linha, 90, 4);

    if ((frames / 20) % 2 == 0) {
        text_centered("Press \x80 to retry", 120, 4);
    }

    uint8_t btn = *GAMEPAD1;
    uint8_t pressed = btn & ~(uint8_t)prevBtn;
    prevBtn = btn;
    if (pressed & BUTTON_1) {
        start();
    }
}

// tela de vitoria
static void update_win() {
    PALETTE[0] = COR_FUNDO;
    *DRAW_COLORS = 0x11;
    rect(0, 0, 160, 160);

    uint16_t cor_piscando;
    if ((frames / 10) % 2 == 0) {
        cor_piscando = 4;
    } else {
        cor_piscando = 3;
    }
    text_centered("YOU ESCAPED!", 40, cor_piscando);

    text_centered("All 6 shards", 64, 3);
    text_centered("collected!", 74, 3);
    text_centered("The exit is open!", 90, 3);

    if ((frames / 20) % 2 == 0) {
        text_centered("Press \x80 to play", 120, 4);
        text_centered("again", 130, 4);
    }

    uint8_t btn = *GAMEPAD1;
    uint8_t pressed = btn & ~(uint8_t)prevBtn;
    prevBtn = btn;
    if (pressed & BUTTON_1) {
        start();
    }
}

void update() {
    frames = frames + 1;

    if (telaAtual == TELA_MENU) {
        update_menu();
    } else if (telaAtual == TELA_HISTORIA) {
        update_historia();
    } else if (telaAtual == TELA_DUNGEON) {
        update_dungeon();
    } else if (telaAtual == TELA_GAMEOVER) {
        update_gameover();
    } else {
        update_win();
    }
}
