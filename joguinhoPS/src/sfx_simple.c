#include "wasm4.h"

const char* const names[16] = {
  "beep",       "chime up",    "drop",      "laser",
  "zap",        "explosion",   "hurt",      "powerup blip",
  "menu move",  "menu select", "bass thud", "click",
  "alarm tone", "bell",        "whoosh",    "coin blip",
};

// tone(frequency, duration, volume, flags) -> https://wasm4.org/docs/guides/audio
void play(int id) {
  switch (id) {
  case 0:  // beep            A4 (69), 50% duty
    tone(440, 20 | (10 << 8), 80, TONE_PULSE1 | TONE_MODE3);
    break;
  case 1:  // chime up        C5->C6 (72->84)
    tone(523 | (1047 << 16), 30 | (20 << 8), 90, TONE_PULSE1 | TONE_MODE2);
    break;
  case 2:  // drop            slide down
    tone(600 | (120 << 16), 35 | (10 << 8), 100, TONE_TRIANGLE);
    break;
  case 3:  // laser           fast slide down
    tone(1200 | (200 << 16), 14, 85, TONE_PULSE2 | TONE_MODE1);
    break;
  case 4:  // zap             noise burst
    tone(800, 10 | (6 << 8), 80, TONE_NOISE);
    break;
  case 5:  // explosion       noise sweep down, long release
    tone(160 | (40 << 16), 12 | (45 << 8), 100, TONE_NOISE);
    break;
  case 6:  // hurt            A4->A3 (69->57)
    tone(440 | (220 << 16), 18 | (10 << 8), 90, TONE_PULSE1 | TONE_MODE2);
    break;
  case 7:  // powerup blip    E4->E5 (64->76), peak-attack envelope
    tone(330 | (660 << 16), (6 << 24) | 25 | (8 << 8),
         (100 << 8) | 70, TONE_PULSE1 | TONE_MODE3);
    break;
  case 8:  // menu move       short triangle tick
    tone(700, 6, 70, TONE_TRIANGLE);
    break;
  case 9:  // menu select     A5 (81)
    tone(880, 12 | (6 << 8), 80, TONE_PULSE1 | TONE_MODE3);
    break;
  case 10: // bass thud       low triangle
    tone(200 | (100 << 16), 16 | (8 << 8), 100, TONE_TRIANGLE);
    break;
  case 11: // click           very short noise tick
    tone(1200, 3, 60, TONE_NOISE);
    break;
  case 12: // alarm tone      steady A5 ~1s
    tone(880, 60, 50, TONE_PULSE1 | TONE_MODE3);
    break;
  case 13: // bell            G6 (91), long release
    tone(1568, 10 | (70 << 8), 90, TONE_TRIANGLE);
    break;
  case 14: // whoosh          noise sweep up
    tone(200 | (1600 << 16), 40, 60, TONE_NOISE);
    break;
  case 15: // coin blip       B5->E6 (83->88)
    tone(988 | (1319 << 16), 8 | (18 << 8), 90, TONE_PULSE1 | TONE_MODE2);
    break;
  }
}

void draw_int(int n, int x, int y) {
  char buf[12];
  int i = (int)sizeof(buf) - 1;
  buf[i--] = '\0';
  if (n == 0) {
    buf[i--] = '0';
  } else {
    unsigned u = (unsigned)n;
    while (u && i >= 0) { buf[i--] = (char)('0' + (u % 10)); u /= 10; }
  }
  text(&buf[i + 1], x, y);
}

int active = 0;
uint8_t gamepad_old;

void update() {
  uint8_t gamepad = *GAMEPAD1;
  uint8_t gamepad_now = gamepad & (gamepad ^ gamepad_old); // pressionado agora
  gamepad_old = gamepad;

  if (gamepad_now & BUTTON_LEFT)  { active = (active + 15) % 16; }
  if (gamepad_now & BUTTON_RIGHT) { active = (active +  1) % 16; }
  if (gamepad_now & (BUTTON_1 | BUTTON_2)) { play(active); }

  *DRAW_COLORS = 2;
  text("SFX DEMO", 16, 16);
  *DRAW_COLORS = 4;
  text("SFX", 16, 64);
  draw_int(active, 48, 64);
  *DRAW_COLORS = 2;
  text(names[active], 16, 80);
  *DRAW_COLORS = 3;
  text("\x84 \x85 select", 16, 120);
  text("\x80 \x81 play", 16, 136);
}
