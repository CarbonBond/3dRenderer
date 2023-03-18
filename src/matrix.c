#include "matrix.h"
#include "vector.h"
#include <math.h>

mat4_t mat4_identity(void) {
  mat4_t m = {{
    {1, 0, 0, 0},
    {0, 1, 0, 0},
    {0, 0, 1, 0},
    {0, 0, 0, 1},
  }};
  return m;
}
mat4_t mat4_make_transform(float tx, float ty, float tz) {
  mat4_t m = mat4_identity();
  m.m[0][3] = tx;
  m.m[1][3] = ty;
  m.m[2][3] = tz;
  return m;
}
mat4_t mat4_make_scale(float sx, float sy, float sz) {
  mat4_t m = mat4_identity();
  m.m[0][0] = sx;
  m.m[1][1] = sy;
  m.m[2][2] = sz;
  return m;
}

vec4_t mat4_mul_vec4(mat4_t m, vec4_t v) {
  vec4_t result;
  result.x = m.m[0][0] * v.x + m.m[0][1] * v.y + m.m[0][2] * v.z + m.m[0][3] * v.w; 
  result.y = m.m[1][0] * v.x + m.m[1][1] * v.y + m.m[1][2] * v.z + m.m[1][3] * v.w; 
  result.z = m.m[2][0] * v.x + m.m[2][1] * v.y + m.m[2][2] * v.z + m.m[2][3] * v.w; 
  result.w = m.m[3][0] * v.x + m.m[3][1] * v.y + m.m[3][2] * v.z + m.m[3][3] * v.w; 

  return result;
}
mat4_t mat4_make_rotation_x(float angle){
  float c = cos(angle);
  float s = sin(angle);

  mat4_t m = mat4_identity();
  m.m[1][1] = c;
  m.m[1][2] = s;
  m.m[2][1] = -s;
  m.m[2][2] = c;
  return m;
}
mat4_t mat4_make_rotation_y(float angle){
  float c = cos(angle);
  float s = sin(angle);

  mat4_t m = mat4_identity();
  m.m[0][0] = c;
  m.m[0][2] = -s;
  m.m[2][0] = s;
  m.m[2][2] = c;
  return m;
}
mat4_t mat4_make_rotation_z(float angle){
  float c = cos(angle);
  float s = sin(angle);

  mat4_t m = mat4_identity();
  m.m[0][0] = c;
  m.m[0][1] = s;
  m.m[1][0] = -s;
  m.m[1][1] = c;
  return m;
}

mat4_t mat4_mul_mat4(mat4_t a, mat4_t b) {
  mat4_t r;
  for (int i = 0; i < 4; i++){
    for (int j = 0; j < 4; j++){
      r.m[i][j] = a.m[i][0] * b.m[0][j] +
                  a.m[i][1] * b.m[1][j] +
                  a.m[i][2] * b.m[2][j] +
                  a.m[i][3] * b.m[3][j];
    }
  }
  return r;
}
mat4_t mat4_make_perspective(float fov, float aspect, float znear, float zfar) {
  mat4_t r = {{{ 0 }}};
  double t = 1 / tan(fov / 2);
  r.m[0][0] = aspect * t;
  r.m[1][1] = t;
  r.m[2][2] = zfar / (zfar - znear);
  r.m[2][3] = -((zfar*znear)/(zfar-znear));
  r.m[3][2] = 1.;
  return r;
}

vec4_t mat4_mul_vec4_project(mat4_t mat_proj, vec4_t v){
  vec4_t r = mat4_mul_vec4(mat_proj, v);

  //Note(Brandon) perspective device)
  if(r.w != 0.0){
    r.x /= r.w;
    r.y /= r.w;
    r.z /= r.w;
  }

  return r;
}
