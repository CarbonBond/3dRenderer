#ifndef TRIANGLE_H
#define TRIANGLE_H
#include "vector.h"
#include "display.h"
#include <stdint.h>

typedef struct {
  int a;
  int b;
  int c;
  uint32_t color;
} face_t;

typedef struct {
  float avg_depth;
  uint32_t color;
  vec2_t points[3];
} triangle_t;

void int_swap(int* a, int* b);
void fill_flat_top_triangle   (int x0,int y0,int x1,int y1,int x2,int y2, uint32_t color);
void fill_flat_bottom_triangle(int x1,int y1,int mx,int my,int x2,int y2, uint32_t color);
void draw_filled_triangle(int x0, int y0, int x1, int y1, int x2, int y2, uint32_t color); 

#endif // !TRIANGLE_H
