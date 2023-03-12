#ifndef MESH_H
#define MESH_H

#include "vector.h"
#include "triangle.h"

#define N_CUBE_VERTICES 8
extern vec3_t cube_vertices[N_CUBE_VERTICES];  

#define N_CUBE_FACES (6 * 2)
extern face_t cube_faces[N_CUBE_FACES];

typedef struct {
  /*NOTE(Brandon): Dynamic array of vertices and faces */
  vec3_t *vertices; 
  face_t *faces;
  vec3_t rotation;
  vec3_t translation;
  vec3_t scale;
} mesh_t;

extern mesh_t mesh;

void load_cube_mesh_data(void);
void load_obj_file_data(char* filename, mesh_t *obj);

#endif // !MESH_H


