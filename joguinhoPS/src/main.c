#include <string.h>
#include "wasm4.h"
#include "tiles4.h"
#include "util.h"

// ─── Telas ───────────────────────────────────────────────
#define TELA_MENU     0
#define TELA_HISTORIA 1
#define TELA_DUNGEON  2
#define TELA_GAMEOVER 3
#define TELA_WIN      4


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

// ─── Paleta ─────────────────────────────────────────────
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
static int frames; // forward-declarado aqui pra poder ser usado no ciclo de cores abaixo


// ─── Estado global ────────────────────────────────────────
static int telaAtual;
static int pcol, prow;     // posição do jogador em tiles (colunas/linhas)
static int fragmentos;
static int msg_timer;
static char msg[32];
static int prevBtn;
static int historiaPag; // pag atual da hist.

// ─── Inimigos ─────────────────────────────────────────────
// Inimigos se movem tile a tile, alinhados ao grid.
// axis: 0=horizontal, 1=vertical
// dir:  0=positivo (+col ou +row), 1=negativo
// chasing: 0=patrulha normal, 1=perseguindo jogador
#define CHASE_RADIUS   5   // distância em tiles para ativar perseguição
#define CHASE_SPEED   15   // frames entre passos ao perseguir (mais rápido)

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

// Desenha um texto centralizado horizontalmente na tela (cada caractere
// tem FONT_SIZE pixels de largura, então calculamos o x pra deixar a
// string inteira no meio dos 160px de largura da tela).
static void text_centered(const char *s, int y, uint16_t color) {
    int len = 0;
    while (s[len]) len++;

    int x = (SCREEN_SIZE - len * FONT_SIZE) / 2;
    *DRAW_COLORS = color;
    text(s, x, y);
}

// Desenha um texto com uma caixa de fundo própria, ligeiramente maior que
// o texto, ao invés de depender do fundo embutido do text() (a cor 2 do
// DRAW_COLORS), que cobre só o tamanho exato de cada caractere e deixa o
// texto colado na borda da caixa, prejudicando a leitura.
// fg/bg vão de 1 a 4 (cor da paleta) — não use valores em nibble aqui.
static void draw_text_box(const char *s, int x, int y, uint16_t fg, uint16_t bg) {
    int len = 0;
    while (s[len]) len++;

    const uint32_t pad_x = 3;
    const uint32_t pad_y = 2;
    uint32_t w = (uint32_t)len * FONT_SIZE + pad_x * 2;
    uint32_t h = FONT_SIZE + pad_y * 2;

    *DRAW_COLORS = bg;
    rect(x - (int)pad_x, y - (int)pad_y, w, h);

    *DRAW_COLORS = fg; // cor 2 fica 0 (transparente), então não sobrepõe a caixa
    text(s, x, y);
}

// ─── Caixinha de fala da história ──────────────
// Desenha um balão retangular com borda, com um pequeno "rabinho"
static void draw_speech_box(const char *l1, const char *l2, const char *l3,
                             int x, int y, uint32_t w, uint32_t h) {
    // fundo do balão (branco, cor 4)
    *DRAW_COLORS = 4;
    rect(x, y, w, h);
 
    // borda externa (cor 2, azul médio) 2px
    *DRAW_COLORS = 2;
    rect(x, y, w, 2);                    // topo
    rect(x, y + (int)h - 2, w, 2);       // base
    rect(x, y, 2, h);                    // esquerda
    rect(x + (int)w - 2, y, 2, h);       // direita
 
    // "rabinho" do balão apontando pra baixo, no canto esquerdo
    int tx = x + 14;
    int ty = y + (int)h;
    rect(tx, ty, 10, 8);         // corpo do rabinho
    *DRAW_COLORS = 4;
    rect(tx + 2, ty, 6, 6);      // preenche por dentro, deixando borda de 2px
 
    // texto dentro do balão, uma linha por vez
    *DRAW_COLORS = 2;
    if (l1) text(l1, x + 8, y + 8);
    if (l2) text(l2, x + 8, y + 8 + (FONT_SIZE + 4));
    if (l3) text(l3, x + 8, y + 8 + (FONT_SIZE + 4) * 2);
}
 
// Lê um pixel (0-3) de uma sprite sheet 2bpp. Sprites do WASM-4 guardam o
// pixel mais à esquerda nos bits MAIS significativos de cada byte (ordem
// diferente do FRAMEBUFFER, que é LSB-first) — por isso o shift é invertido
// aqui em relação ao pget()/pset() do util.c.
static int tile_pixel(int x, int y, int width) {
    int idx = (y * width + x) >> 2;
    int shift = (3 - (x & 0b11)) << 1;
    int mask = 0b11 << shift;
    return (tiles4[idx] & mask) >> shift;
}
 
// Desenha uma sprite 8x8 da tiles4 ampliada por "scale", pixel a pixel,
// usando o MESMO mapeamento de cores (DRAW_COLORS) usado pelo sprite real
// no jogo — assim o retrato ampliado fica idêntico ao personagem/inimigo
// de verdade, em vez de ter cores inventadas à parte.
// colorMap segue o formato do DRAW_COLORS: nibble 0 = cor pro valor de
// pixel 0, nibble 1 = cor pro valor 1, e assim por diante (0 = transparente).
static void draw_sprite_scaled(int dx, int dy, int srcX, int srcY, int scale, uint16_t colorMap) {
    for (int sy = 0; sy < 8; sy++) {
        for (int sx = 0; sx < 8; sx++) {
            int v = tile_pixel(srcX + sx, srcY + sy, (int)tiles4Width);
            int color = (colorMap >> (v * 4)) & 0xF;
            if (color == 0) continue; // transparente
            *DRAW_COLORS = (uint16_t)color;
            rect(dx + sx * scale, dy + sy * scale, (uint32_t)scale, (uint32_t)scale);
        }
    }
}

// desenha o ninja com o mesmo mapeamento de cor do draw_player() (0x4230)
static void draw_hero_portrait(int x, int y, int scale) {
    int c = 96;
    int srcX = (c % 16) * 8;
    int srcY = (c / 16) * 8;
    draw_sprite_scaled(x, y, srcX, srcY, scale, 0x4230);
}

// desenha a caveirinha inimiga com o mesmo mapeamento de cor do
// draw_enemies() (0x4010), pra ficar idêntica ao inimigo real da dungeon
static void draw_skull_portrait(int x, int y, int scale, int frame) {
    int c = 112 + (frame % 2);
    int srcX = (c % 16) * 8;
    int srcY = (c / 16) * 8;
    draw_sprite_scaled(x, y, srcX, srcY, scale, 0x4010);
}


// ─── start() ─────────────────────────────────────────────
void start() {
    PALETTE[0] = COR_FUNDO;
    PALETTE[1] = COR_PAREDE;
    PALETTE[2] = COR_NINJA;
    PALETTE[3] = COR_BRANCO;

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

    // Inimigo 0: horizontal, linha 1, corredor cols 7..18 (12 tiles livres)
    enemies[0].col = 7; enemies[0].row = 2;
    enemies[0].dir = 0;  enemies[0].axis = 0; enemies[0].speed = 18;

    // Inimigo 1: vertical, col 1, corredor linhas 1..9 (9 tiles livres)
    enemies[1].col = 18;  enemies[1].row = 7;
    enemies[1].dir = 0;  enemies[1].axis = 1; enemies[1].speed = 20;

    // Inimigo 2: horizontal, linha 9, corredor cols 6..11
    enemies[2].col = 10;  enemies[2].row = 9;
    enemies[2].dir = 0;  enemies[2].axis = 0; enemies[2].speed = 16;

    // Inimigo 3: vertical, col 12, corredor linhas 11..15
    enemies[3].col = 12; enemies[3].row = 13;
    enemies[3].dir = 0;  enemies[3].axis = 1; enemies[3].speed = 20;

    // Inimigo 4: horizontal, linha 17, corredor cols 4..14 (11 tiles livres)
    enemies[4].col = 7;  enemies[4].row = 17;
    enemies[4].dir = 0;  enemies[4].axis = 0; enemies[4].speed = 14;

    // Inimigo 5: vertical, col 18, corredor linhas 1..9
    enemies[5].col = 14; enemies[5].row = 16;
    enemies[5].dir = 0;  enemies[5].axis = 1; enemies[5].speed = 17;

    for (int i = 0; i < NUM_ENEMIES; i++) {
        enemies[i].timer   = 0;
        enemies[i].chasing = 0;
    }
}

// ─── HISTORIA ────────────────────────────────────────────────
#define HIST_PAGINAS 3
 
// Layout: ninja ampliado no topo, caixa de fala abaixo dele, e uma
// decoraçãozinha (brilho/inimigo) posicionada fora da área da caixa,
// pra não ficar escondida atrás dela.
#define HIST_BOX_X 6
#define HIST_BOX_Y 48
#define HIST_BOX_W 148
#define HIST_BOX_H 54
#define HIST_DECO_X 128
#define HIST_DECO_Y 16
 
static void draw_historia_fundo(int pag) {
    // Fundo azul escuro do dungeon (cor 1) em toda a tela
    memset(FRAMEBUFFER, 0x00, 160*160/4);
 
    // Umas "pedras" decorando o topo e a base
    *DRAW_COLORS = 2;
    for (int c = 0; c < COLS; c++) {
        rect(c * TILE, 0, TILE - 1, TILE - 1);
        rect(c * TILE, 152, TILE - 1, TILE - 1);
    }
 
    // Um brilho de fragmento decorativo, sugerindo tesouro.
    // Posicionado fora da área da caixa de fala (que ocupa
    // y=HIST_BOX_Y..HIST_BOX_Y+HIST_BOX_H), pra não ficar escondido.
    if (pag == 1) {
        *DRAW_COLORS = 4;
        rect(HIST_DECO_X + 3, HIST_DECO_Y + 0, 2, 8);
        rect(HIST_DECO_X + 0, HIST_DECO_Y + 3, 8, 2);
        rect(HIST_DECO_X + 3, HIST_DECO_Y + 3, 2, 2);
    }
 
    // Um inimigo espreitando ao fundo — a mesma caveirinha da dungeon,
    // animada, em vez de um retângulo desenhado à mão.
    if (pag == 2) {
        draw_skull_portrait(HIST_DECO_X, HIST_DECO_Y, 2, frames / 10);
    }
}
 
static void update_historia() {
    uint8_t btn = *GAMEPAD1;
    uint8_t pressed = btn & ~(uint8_t)prevBtn;
    prevBtn = btn;
 
    draw_historia_fundo(historiaPag);
 
    // Ninja ampliado (3x = 24x24px), centralizado no topo da tela
    draw_hero_portrait(68, 14, 3);
 
    // Falas com no máximo 16 caracteres por linha, pra caber direitinho
    // dentro da largura da caixa (HIST_BOX_W - padding = 128px = 16 chars)
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
 
    text_centered("MAZE ESCAPE", 130, 4);
 
    if ((frames / 20) % 2 == 0) {
        text_centered("Press a button", 144, 3);
    }
 
    // qualquer botão avança a página (ou entra na dungeon na última)
    if (pressed != 0) {
        if (historiaPag < HIST_PAGINAS - 1) {
            historiaPag++;
            
        } else {
            telaAtual = TELA_DUNGEON;
            
        }
    }
}
 
static void update_menu() {
    uint8_t btn = *GAMEPAD1;
    uint8_t pressed = btn & ~(uint8_t)prevBtn;
    prevBtn = btn;
 
    // Preenche a tela inteira com a cor de fundo 2 (PALETTE[1], azul médio),
    // diferente do branco usado no texto, melhorando o contraste.
    // 0b01010101 == 0x55 (não 0x66!): cada byte guarda 4 pixels de 2 bits,
    // e repetir o valor bruto 1 (cor 2) em cada um deles dá 01 01 01 01 = 0x55.
    memset(FRAMEBUFFER, 0x55, 160*160/4);
 
    text_centered("------------", 22, 3);
    text_centered("MAZE ESCAPE", 32, 4);
    text_centered("------------", 42, 3);
 
    text_centered("CONTROLS:", 62, 4);
    // \x84 \x85 \x86 \x87 = setas esquerda/direita/cima/baixo, \x80 = botao X
    text("\x84\x85\x86\x87 - Move", 14, 80);
    text("\x80 - Collect shard", 14, 90);
 
    if ((frames / 20) % 2 == 0) {
        text_centered("Press \x80 to play", 118, 4);
    }
 
    if (pressed & BUTTON_1) {
        telaAtual = TELA_HISTORIA;
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
                // borda branca
                *DRAW_COLORS = 4;
                rect(sx, sy, TILE - 1, 1);
                rect(sx, sy, 1, TILE - 1);
            } else if (t == T_SHARD) {
                *DRAW_COLORS = 4; // branco (o 1o digito é o que rect() usa)
                rect(sx + 3, sy + 0, 2, 8);
                rect(sx + 0, sy + 3, 8, 2);
                rect(sx + 3, sy + 3, 2, 2);
            } else if (t == T_EXIT) {
                if ((frames / 14) % 2 == 0) { *DRAW_COLORS = 0x22; }
                else                         { *DRAW_COLORS = 0x44; }
                rect(sx, sy, TILE, TILE);
            }
        }
    }
}

static void draw_player() {
    int sx = pcol * TILE;
    int sy = prow * TILE;
    int c = 96 + (frames / 10) % 2; // alterna entre 2 quadros pra animar o passo

    // corpo do ninja na cor do personagem, detalhe na cor da parede e o
    // realce (faixa) na cor dos inimigos — nunca usa a cor do fundo, então
    // o personagem jamais fica igual ao fundo, mesmo com o fundo ciclando
    *DRAW_COLORS = 0x4230;
    blitSub(tiles4, sx, sy, 8, 8,
            (uint32_t)(c % 16) * 8, (uint32_t)(c / 16) * 8,
            tiles4Width, BLIT_2BPP);
}

static void draw_enemies() {
    for (int i = 0; i < NUM_ENEMIES; i++) {
        int sx = enemies[i].col * TILE;
        int sy = enemies[i].row * TILE;
        // caveirinhas do tiles4 (2 quadros de animação, tiles 112 e 113),
        // cada inimigo com uma fase diferente pra não animarem todos juntos
        int c = 112 + (frames / 10 + i) % 2;
        // crânio na cor dos inimigos (branco), olhos vazados na cor do fundo
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

        // Distância (em tiles) até o jogador, pra saber se ativa a perseguição.
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

        enemies[i].timer++;
        if (enemies[i].timer >= velocidade) {
            enemies[i].timer = 0;

            if (enemies[i].chasing == 1) {
                // Perseguição: anda um passo na direção do jogador.
                // Primeiro tenta alinhar a coluna, depois a linha.
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
                // Patrulha normal: anda sempre pra frente no mesmo eixo,
                // e vira quando bate numa parede.
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
            }
        }

        // colisão com jogador — checada TODO frame (não só quando o
        // inimigo se move), senão dá pra atravessar um inimigo parado
        // sem morrer até ele dar o próximo passo.
        if (enemies[i].col == pcol && enemies[i].row == prow) {
            telaAtual = TELA_GAMEOVER;
        }
    }
}

static void update_dungeon() {
    // fundo cicla lentamente por toda a paleta — nunca pode dar a mesma
    // cor do personagem, pois os índices reservados são pulados
    PALETTE[0] = COR_FUNDO;

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
    PALETTE[0] = COR_FUNDO; 
    *DRAW_COLORS = 0x11;
    rect(0, 0, 160, 160);

    text_centered("GAME OVER", 50, 2);
    text_centered("Caught by enemy!", 64, 3);

    // monta "Shards: N/6" numa string só, pra dar pra centralizar de uma vez
    char linha[24];
    int i = 0;
    const char *rotulo = "Shards: ";
    while (rotulo[i]) { linha[i] = rotulo[i]; i++; }
    char num[16];
    itoa(num, fragmentos);
    int j = 0;
    while (num[j]) { linha[i++] = num[j++]; }
    linha[i++] = '/';
    linha[i++] = '6';
    linha[i] = '\0';
    text_centered(linha, 90, 4);

    if ((frames / 20) % 2 == 0) {
        text_centered("Press \x80 to retry", 120, 4);
    }

    uint8_t btn = *GAMEPAD1;
    uint8_t pressed = btn & ~(uint8_t)prevBtn;
    prevBtn = btn;
    if (pressed & BUTTON_1) { start(); }
}

// ─── WIN ─────────────────────────────────────────────────
static void update_win() {
    PALETTE[0] = COR_FUNDO; // fundo fixo (quase preto) nesta tela
    *DRAW_COLORS = 0x11;
    rect(0, 0, 160, 160);

    uint16_t cor_piscando = ((frames / 10) % 2 == 0) ? 4 : 3;
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
    if (pressed & BUTTON_1) { start(); }
}

// ─── update() ────────────────────────────────────────────
void update() {
        frames++;
    if(telaAtual==TELA_MENU){
        update_menu();
    }else if(telaAtual==TELA_HISTORIA){
        update_historia();
    }else if(telaAtual==TELA_DUNGEON){
        update_dungeon();
    }else if(telaAtual==TELA_GAMEOVER){
        update_gameover();
    }else{
        update_win();
    }
}