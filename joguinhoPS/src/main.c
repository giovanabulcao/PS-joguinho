#include "wasm4.h"
#include "tiles4.h"
#include "util.h"

// ─── Telas ───────────────────────────────────────────────
#define TELA_MENU     0
#define TELA_DUNGEON  1
#define TELA_GAMEOVER 2
#define TELA_WIN      3

// ─── Mapa ────────────────────────────────────────────────
#define TILE  8
#define COLS 20
#define ROWS 20

#define T_FLOOR  0
#define T_WALL   1
#define T_SHARD  2
#define T_EXIT   3

#define TOTAL_SHARDS 6
#define NUM_ENEMIES  6

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

// ─── Estado global ────────────────────────────────────────
static int telaAtual;
static int pcol, prow;     // posição do jogador em tiles (colunas/linhas)
static int fragmentos;
static int frames;
static int msg_timer;
static char msg[32];
static int prevBtn;

// ─── Inimigos ─────────────────────────────────────────────
// Inimigos se movem tile a tile, alinhados ao grid.
// axis: 0=horizontal, 1=vertical
// dir:  0=positivo (+col ou +row), 1=negativo
// chasing: 0=patrulha normal, 1=perseguindo jogador
#define CHASE_RADIUS   6   // distância em tiles para ativar perseguição
#define CHASE_SPEED   14   // frames entre passos ao perseguir (mais rápido)

typedef struct {
    int col, row;
    int dir;
    int axis;
    int timer;
    int speed;   // frames entre cada passo (patrulha)
    int chasing; // 0=patrulha, 1=perseguindo
} Enemy;

static Enemy enemies[NUM_ENEMIES];

// ─── Funções auxiliares ───────────────────────────────────
static int map_at(int col, int row) {
    if (col < 0 || col >= COLS || row < 0 || row >= ROWS) return T_WALL;
    return map[row][col];
}

static void set_msg(const char *s) {
    int i = 0;
    while (s[i] && i < 31) { msg[i] = s[i]; i++; }
    msg[i] = '\0';
    msg_timer = 90;
}

// ─── start() ─────────────────────────────────────────────
void start() {
    // Paleta com contraste mais suave: tons de cinza-azulado
    PALETTE[0] = 0x153c4a ;  // fundo: azul escuro
    PALETTE[1] = 0x9c341a ;  // paredes(base): musgo / texto secundário: marrom
    PALETTE[2] = 0x77b02a;  // paredes(musgo): texto em destaque: verde
    PALETTE[3] = 0xf8ffb8;  // jogador: creme claro

    for (int r = 0; r < ROWS; r++)
        for (int c = 0; c < COLS; c++)
            map[r][c] = MAP[r][c];

    pcol = 1;
    prow = 1;

    fragmentos = 0;
    frames     = 0;
    msg[0]     = '\0';
    msg_timer  = 0;
    prevBtn    = 0;
    telaAtual  = TELA_MENU;

    // Inimigo 0: horizontal, linha 1, col 7..18
    enemies[0].col = 7;  enemies[0].row = 1;
    enemies[0].dir = 0;  enemies[0].axis = 0; enemies[0].speed = 18;

    // Inimigo 1: vertical, col 16, linha 1..7
    enemies[1].col = 16; enemies[1].row = 1;
    enemies[1].dir = 0;  enemies[1].axis = 1; enemies[1].speed = 22;

    // Inimigo 2: horizontal, linha 9, col 6..11
    enemies[2].col = 6;  enemies[2].row = 9;
    enemies[2].dir = 0;  enemies[2].axis = 0; enemies[2].speed = 15;

    // Inimigo 3: vertical, col 6, linha 9..18
    enemies[3].col = 6;  enemies[3].row = 14;
    enemies[3].dir = 1;  enemies[3].axis = 1; enemies[3].speed = 20;

    // Inimigo 4: horizontal, linha 15, col 2..12
    enemies[4].col = 2;  enemies[4].row = 15;
    enemies[4].dir = 0;  enemies[4].axis = 0; enemies[4].speed = 25;

    // Inimigo 5: vertical, col 18, linha 1..17
    enemies[5].col = 18; enemies[5].row = 7;
    enemies[5].dir = 1;  enemies[5].axis = 1; enemies[5].speed = 17;

    for (int i = 0; i < NUM_ENEMIES; i++) {
        enemies[i].timer   = 0;
        enemies[i].chasing = 0;
    }
}

// ─── MENU ────────────────────────────────────────────────
static void update_menu() {
    uint8_t btn = *GAMEPAD1;
    uint8_t pressed = btn & ~(uint8_t)prevBtn;
    prevBtn = btn;

    // Limpa a tela
    memset(FRAMEBUFFER, 0x00, 160 * 160 / 4);

    // Fundo
    *DRAW_COLORS = 0x11;
    rect(0, 0, 160, 160);

    *DRAW_COLORS = 0x22;
    rect(18, 8, 124, 16);

    *DRAW_COLORS = 0x40;
    text("MAZE ESCAPE", 28, 12);

    *DRAW_COLORS = 0x22;
    rect(6, 34, 148, 38);

    *DRAW_COLORS = 0x40;
    text("CONTROLS", 12, 38);

    *DRAW_COLORS = 0x30;
    text("\x86\x87\x84\x85 Move", 12, 52);
    text("\x80 Collect shard", 12, 62);

    *DRAW_COLORS = 0x22;
    rect(6, 82, 148, 50);

    *DRAW_COLORS = 0x40;
    text("OBJECTIVE", 12, 86);

    *DRAW_COLORS = 0x30;
    text("Collect 6 shards", 12, 100);
    text("Reach exit >", 12, 110);
    text("Avoid enemies", 12, 120);

    *DRAW_COLORS = 0x22;
    rect(18, 142, 124, 14);

    if ((frames / 25) % 2 == 0)
        *DRAW_COLORS = 0x40;
    else
        *DRAW_COLORS = 0x30;

    text("Press X to play", 26, 146);

    if (pressed & BUTTON_1) {
        telaAtual = TELA_DUNGEON;
    }
}

// ─── DUNGEON ─────────────────────────────────────────────
static void draw_map() {
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            int t  = map[r][c];
            int sx = c * TILE;
            int sy = r * TILE;

            if (t == T_WALL) {
                *DRAW_COLORS = 0x22;
                rect(sx, sy, TILE, TILE);
                // borda sutil
                *DRAW_COLORS = 0x32;
                rect(sx, sy, TILE - 1, 1);
                rect(sx, sy, 1, TILE - 1);
            } else if (t == T_SHARD) {
                *DRAW_COLORS = 0x30;
                rect(sx + 3, sy + 0, 2, 8);
                rect(sx + 0, sy + 3, 8, 2);
                *DRAW_COLORS = 0x40;
                rect(sx + 3, sy + 3, 2, 2);
            } else if (t == T_EXIT) {
                if ((frames / 14) % 2 == 0) { *DRAW_COLORS = 0x21; }
                else                         { *DRAW_COLORS = 0x31; }
                rect(sx, sy, TILE, TILE);
                *DRAW_COLORS = 0x40;
                text(">", sx, sy);
            }
        }
    }
}

static void draw_player() {
    int sx = pcol * TILE;
    int sy = prow * TILE;
    int c = 96 + (frames / 10) % 2;
    *DRAW_COLORS = 0x4320;
    blitSub(tiles4, sx, sy, 8, 8,
            (uint32_t)(c % 16) * 8, (uint32_t)(c / 16) * 8,
            tiles4Width, BLIT_2BPP);
}

static void draw_enemies() {
    for (int i = 0; i < NUM_ENEMIES; i++) {
        int sx = enemies[i].col * TILE;
        int sy = enemies[i].row * TILE;
        *DRAW_COLORS = 0x21;
        rect(sx + 1, sy + 1, 6, 6);
        *DRAW_COLORS = 0x31;
        rect(sx + 2, sy + 2, 2, 2);
        rect(sx + 4, sy + 2, 2, 2);
        rect(sx + 2, sy + 5, 4, 1);
    }
}

static void draw_hud() {
    char buf[16];

    *DRAW_COLORS = 0x11;
    rect(0, 0, 160, 11);

    *DRAW_COLORS = 0x30;
    text("Shards:", 2, 1);
    *DRAW_COLORS = 0x40;
    itoa(buf, fragmentos);
    text(buf, 58, 1);
    *DRAW_COLORS = 0x30;
    text("/6", 66, 1);

    if (msg_timer > 0) {
        *DRAW_COLORS = 0x11;
        rect(0, 140, 160, 20);
        *DRAW_COLORS = 0x40;
        text(msg, 4, 144);
    }
}

static void update_enemies() {
    for (int i = 0; i < NUM_ENEMIES; i++) {
        enemies[i].timer++;
        if (enemies[i].timer < enemies[i].speed) continue;
        enemies[i].timer = 0;

        int nc = enemies[i].col;
        int nr = enemies[i].row;

        if (enemies[i].axis == 0) {
            nc += (enemies[i].dir == 0 ? 1 : -1);
            if (map_at(nc, nr) == T_WALL) {
                enemies[i].dir = 1 - enemies[i].dir;
            } else {
                enemies[i].col = nc;
            }
        } else {
            nr += (enemies[i].dir == 0 ? 1 : -1);
            if (map_at(nc, nr) == T_WALL) {
                enemies[i].dir = 1 - enemies[i].dir;
            } else {
                enemies[i].row = nr;
            }
        }

        // colisão com jogador
        if (enemies[i].col == pcol && enemies[i].row == prow) {
            telaAtual = TELA_GAMEOVER;
        }
    }
}

static void update_dungeon() {
    uint8_t btn = *GAMEPAD1;
    uint8_t pressed = btn & ~(uint8_t)prevBtn;
    prevBtn = btn;

    // Movimento tile a tile: só move em pressed (não em btn)
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

    // coleta com X — tenta pegar shard na posição atual
    if (pressed & BUTTON_1) {
        if (map_at(pcol, prow) == T_SHARD) {
            fragmentos++;
            map[prow][pcol] = T_FLOOR;
            set_msg("Shard collected!");
        }
    }

    // shard também coletado ao andar em cima
    if (map_at(pcol, prow) == T_SHARD) {
        fragmentos++;
        map[prow][pcol] = T_FLOOR;
        set_msg("Shard collected!");
    }

    // saída
    if (map_at(pcol, prow) == T_EXIT) {
        if (fragmentos >= TOTAL_SHARDS) {
            telaAtual = TELA_WIN;
        } else {
            set_msg("Need all 6 shards!");
        }
    }

    if (msg_timer > 0) msg_timer--;

    update_enemies();
    draw_map();
    draw_enemies();
    draw_player();
    draw_hud();
}

// ─── GAMEOVER ────────────────────────────────────────────
static void update_gameover() {
    *DRAW_COLORS = 0x11;
    rect(0, 0, 160, 160);

    *DRAW_COLORS = 0x20;
    text("GAME OVER", 32, 50);
    *DRAW_COLORS = 0x30;
    text("Caught by enemy!", 4, 70);

    char buf[16];
    *DRAW_COLORS = 0x40;
    text("Shards:", 32, 92);
    itoa(buf, fragmentos);
    text(buf, 88, 92);
    *DRAW_COLORS = 0x30;
    text("/6", 96, 92);

    if ((frames / 20) % 2 == 0) {
        *DRAW_COLORS = 0x40;
        text("Press X to retry", 4, 130);
    }

    uint8_t btn = *GAMEPAD1;
    uint8_t pressed = btn & ~(uint8_t)prevBtn;
    prevBtn = btn;
    if (pressed & BUTTON_1) { start(); }
}

// ─── WIN ─────────────────────────────────────────────────
static void update_win() {
    *DRAW_COLORS = 0x11;
    rect(0, 0, 160, 160);

    if ((frames / 10) % 2 == 0) { *DRAW_COLORS = 0x40; }
    else                         { *DRAW_COLORS = 0x30; }
    text("YOU ESCAPED!", 16, 40);

    *DRAW_COLORS = 0x30;
    text("All 6 shards", 24, 64);
    text("collected!", 32, 74);
    text("The exit is open!", 4, 90);

    if ((frames / 20) % 2 == 0) {
        *DRAW_COLORS = 0x40;
        text("Press X to play", 4, 130);
        text("again", 56, 140);
    }

    uint8_t btn = *GAMEPAD1;
    uint8_t pressed = btn & ~(uint8_t)prevBtn;
    prevBtn = btn;
    if (pressed & BUTTON_1) { start(); }
}

// ─── update() ────────────────────────────────────────────
void update() {
    frames++;
    if(telaAtual==TELA_MENU){
        update_menu();
    }else if(telaAtual==TELA_DUNGEON){
        update_dungeon();
    }else if(telaAtual==TELA_GAMEOVER){
        update_gameover();
    }else{
        update_win();
    }
}