#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// returns the color from pixel (x,y)
int pget(int x, int y);

// set pixel (x,y) with color c
void pset(int x, int y, int c);

// Shape structure:
//   (x,y) is a point
//   (x,y,w) is a circle with radius 'w'
//   (x,y,w,h) is a rectangle with width 'w' and height 'h'
typedef struct {
  int x, y, w, h;
} Shape;

// collision functions
int hit_point_point(Shape sh1, Shape sh2);
int hit_point_box(Shape sh1, Shape sh2);
int hit_point_circle(Shape sh1, Shape sh2);
int hit_box_box(Shape sh1, Shape sh2);
int hit_circle_circle(Shape sh1, Shape sh2);
int hit_circle_box(Shape sh1, Shape sh2);

// converts integer 'n' to a string inside 'buf'
void itoa(char *buf, int n);

#endif // UTIL_H
