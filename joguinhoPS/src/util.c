#include "wasm4.h"
#include "util.h"

int pget(int x, int y) {
  if (x < 0 || x > 159 || y < 0 || y > 159) { return 0; } // outside screen
  int idx = (y*160 + x) >> 2;
  int shift = (x & 0b11) << 1;
  int mask = 0b11 << shift;
  return ((FRAMEBUFFER[idx] & mask) >> shift) + 1;
}

void pset(int x, int y, int c) {
  if (x < 0 || x > 159 || y < 0 || y > 159) { return; } // outside screen
  if (c == 0) { return; } // ignore transparent color
  int idx = (y*160 + x) >> 2;
  int shift = (x & 0b11) << 1;
  int mask = 0b11 << shift;
  FRAMEBUFFER[idx] = (uint8_t) ((c-1) << shift) | (FRAMEBUFFER[idx] & ~mask);
}

int hit_point_point(Shape sh1, Shape sh2) {
  return sh2.x == sh1.x && sh2.y == sh1.y;
}

int hit_point_box(Shape sh1, Shape sh2) {
  return sh2.x < sh1.x && sh1.x < sh2.x + sh2.w &&
         sh2.y < sh1.y && sh1.y < sh2.y + sh2.h;
}

int hit_point_circle(Shape sh1, Shape sh2) {
  int dx = sh1.x - sh2.x;
  int dy = sh1.y - sh2.y;
  return dx * dx + dy * dy < sh2.w * sh2.w;
}

int hit_box_box(Shape sh1, Shape sh2) {
  return sh2.x < sh1.x + sh1.w && sh1.x < sh2.x + sh2.w &&
         sh2.y < sh1.y + sh1.h && sh1.y < sh2.y + sh2.h;
}

int hit_circle_circle(Shape sh1, Shape sh2) {
  int dx = sh1.x - sh2.x;
  int dy = sh1.y - sh2.y;
  int sr = sh1.w + sh2.w;
  return dx * dx + dy * dy < sr * sr;
}

// adapted from https://learnopengl.com/In-Practice/2D-Game/Collisions/Collision-Detection
int hit_circle_box(Shape sh1, Shape sh2) {
  int half_w = sh2.w / 2;
  int half_h = sh2.h / 2;
  int center_x = sh2.x + half_w;
  int center_y = sh2.y + half_h;
  // offset between circle and box centers
  int ox = sh1.x - center_x;
  int oy = sh1.y - center_y;
  // clamp offset inside box
  ox = MAX(-half_w, MIN(ox, half_w));
  oy = MAX(-half_h, MIN(oy, half_h));
  // get closest point inside box
  int px = center_x + ox;
  int py = center_y + oy;
  // check distance between circle center and closest point
  int dx = sh1.x - px;
  int dy = sh1.y - py;
  return dx * dx + dy * dy < sh1.w * sh1.w;
}

void itoa(char *buf, int n) {
  int i = 0;
  if (n == 0) {
    buf[i++] = '0';
    buf[i] = '\0';
    return;
  }
  while (n > 0) {
    buf[i++] = (n % 10) + '0';
    n /= 10;
  }
  buf[i] = '\0';
  for (int j = 0; j < i/2; j++) {
    char tmp = buf[j];
    buf[j] = buf[i-j-1];
    buf[i-j-1] = tmp;
  }
}
