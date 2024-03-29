#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "display.h"
#include "triangle.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"
#include "matrix.h"
#include "light.h"

#define MATH_PI 3.1415926535897932384 


/*NOTE(Brandon):Array of triangles  that should be rendered frame by frame */
triangle_t* triangles_to_render = NULL;

vec3_t camera_position = {.x = 0, .y = 0, .z = 0};
mat4_t proj_matrix;


bool is_running = false;
int previous_frame_time = 0;

render_mode render_options = {1,1,1,1};

int cmpfunc (const void* a, const void* b) {
  return ( *(int*)b - *(int*)a);
} 

void setup(void){
  /*TODO(Brandon):CHECK FOR ERROR */
  color_buffer = (uint32_t*) malloc(sizeof(uint32_t) * window_height * window_width);
  if(color_buffer == NULL){
    fprintf(stderr, "Error: Couldn't Allocate color_buffer");
    exit(1);
  }

  color_buffer_texture = SDL_CreateTexture(
      renderer,
      SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_STREAMING,
      window_width,
      window_height
      );

  float fov = (100./180)*MATH_PI; //100deg in radians 
  float aspect = ((float)window_height/(float)window_width);
  proj_matrix = mat4_make_perspective(fov,
                                      aspect,
                                      0.1, //Znear
                                      100.); //Zfar

  //load_cube_mesh_data();
//load_obj_file_data("./asset/cube.obj", &mesh);
  load_obj_file_data("./asset/suzanne.obj", &mesh);
}

void process_input(void) {
  SDL_Event event;
  SDL_PollEvent(&event);

  switch (event.type) {
    case SDL_QUIT:
    {
      is_running = false;
      break;
    }
    case SDL_KEYDOWN:
    {
      switch(event.key.keysym.sym){
        case SDLK_ESCAPE:
        {
          is_running = false;
        } break;
        case SDLK_1:
        {
           render_options.vertex = !render_options.vertex;
        } break;
        case SDLK_2:
        {
           render_options.wireframe = !render_options.wireframe;
        } break;
        case SDLK_3:
        {
           render_options.triangles = !render_options.triangles;
        } break;
        case SDLK_c:
        {
           render_options.backface = !render_options.backface;
        } break;
      }
    }
  }
}


void update(void){

  int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

  if(time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) 
    SDL_Delay(time_to_wait);

  triangles_to_render = NULL;

  mesh.rotation.y += 0.002;
  mesh.rotation.x += 0.003;
  mesh.rotation.z += 0.004;

//mesh.scale.x = 2.0;
//mesh.scale.y = 1.5;
  mesh.translation.z = 5.0;

  mat4_t scale_matrix = mat4_make_scale(mesh.scale.x, mesh.scale.y, mesh.scale.z);
  mat4_t tranlation_matrix = mat4_make_transform(mesh.translation.x, mesh.translation.y, mesh.translation.z); 
  mat4_t rotation_z_matrix = mat4_make_rotation_z(mesh.rotation.z); 
  mat4_t rotation_x_matrix = mat4_make_rotation_x(mesh.rotation.x); 
  mat4_t rotation_y_matrix = mat4_make_rotation_y(mesh.rotation.y); 


  /*NOTE(Brandon): Loop over all the faces */
  for (int i = 0; i < (array_length(mesh.faces)); i++) {
    face_t triangle_face = mesh.faces[i];

    vec3_t face_vertices[3];
    face_vertices[0] = mesh.vertices[triangle_face.a - 1];
    face_vertices[1] = mesh.vertices[triangle_face.b - 1];
    face_vertices[2] = mesh.vertices[triangle_face.c - 1];

    vec4_t transformed_vertices[3];
    for (int j = 0; j < 3; j++) {
      vec4_t transformed_vertex = vec4_from_vec3(face_vertices[j]);

      mat4_t world_matrix = mat4_identity();
      world_matrix = mat4_mul_mat4(scale_matrix, world_matrix);

      //NOTE(Brandon): Order matters!!! Scale -> Rotate -> translate
      transformed_vertex = mat4_mul_vec4(scale_matrix, transformed_vertex);
      transformed_vertex = mat4_mul_vec4(rotation_z_matrix, transformed_vertex);
      transformed_vertex = mat4_mul_vec4(rotation_x_matrix, transformed_vertex);
      transformed_vertex = mat4_mul_vec4(rotation_y_matrix, transformed_vertex);
      transformed_vertex = mat4_mul_vec4(tranlation_matrix, transformed_vertex);

      transformed_vertex = mat4_mul_vec4(world_matrix, transformed_vertex);

      transformed_vertices[j] = transformed_vertex;
    }

    
    /*
     * NOTE(Brandon): Get the Normal vector 
     * */
    vec3_t vector_a = vec3_from_vec4(transformed_vertices[0]);
    vec3_t vector_b = vec3_from_vec4(transformed_vertices[1]);
    vec3_t vector_c = vec3_from_vec4(transformed_vertices[2]);
    vec3_t vector_ab = vec3_sub(vector_b,vector_a);
    vec3_t vector_ac = vec3_sub(vector_c, vector_a);
    /*
     * NOTE(Brandon):Compute normal vector, (Left handed) 
     * */
    vec3_normlize(&vector_ab);
    vec3_normlize(&vector_ac);
    vec3_t normal = vec3_cross(vector_ab, vector_ac);
    vec3_normlize(&normal);

    vec3_t camera_ray = vec3_sub(camera_position, vector_a);
    vec3_normlize(&camera_ray);

    /*
     * NOTE(Brandon):Calculate how alinged the camera ray is with the 
     *               face normals.
     * */
    float dot_normal_camera = vec3_dot(normal, camera_ray);

    /*
     * NOTE(Brandon):Bypass triagnles the do not face the camera 
     *                (Backface culling) 
     * */
    if(dot_normal_camera < 0 && render_options.backface) continue;


    vec4_t projected_points[3];
    for(int j = 0; j < 3; j++){
      projected_points[j] = mat4_mul_vec4_project(proj_matrix, transformed_vertices[j]);

      //Note(Brandon): Scale into the view
      projected_points[j].x *= (window_width / 2.);
      projected_points[j].y *= (window_height / 2.);

      //NOTE (Brandon): Invert y values to account for flipped screen y-cord
      projected_points[j].y *= -1;

      /*NOTE(Brandon): translate the points to the middle of the screen*/
      projected_points[j].x += (window_width / 2.);
      projected_points[j].y += (window_height / 2.);
    }

    //Note(Brandon): Calculate avg depth of face
    float avg_depth = (transformed_vertices[0].z +
                       transformed_vertices[1].z+
                       transformed_vertices[2].z) / 3; 

    //Note(Brandon) Light the faces of the triangle based on how they are 
    //              positioned from the sun.
    float light_amount = -vec3_dot(normal, light.direction);
    uint32_t lighted_color = light_apply_intensity(triangle_face.color, light_amount); 

    triangle_t projected_triangle = {
      .points = {
        { projected_points[0].x, projected_points[0].y },
        { projected_points[1].x, projected_points[1].y },
        { projected_points[2].x, projected_points[2].y },
      },
      .avg_depth = avg_depth,
      .color = lighted_color
    };

    /*NOTE(Brandon):Save the projected triangle ine the array
     *              of triangles to render */
    array_push(triangles_to_render, projected_triangle);
    qsort(triangles_to_render, array_length(triangles_to_render),sizeof(triangle_t), cmpfunc);
  }
  //NOTE(Brandon) Sort the triangles to render.
}
void render(void){

  draw_grid(0xFF333333, 10);


  for(int i = 0; i < (array_length(triangles_to_render)); i++){
    triangle_t triangle = triangles_to_render[i];

    if(render_options.triangles){
      draw_filled_triangle(triangle.points[0].x, triangle.points[0].y,
                           triangle.points[1].x, triangle.points[1].y,
                           triangle.points[2].x, triangle.points[2].y,
                           triangle.color);
    }
    if(render_options.wireframe){
      draw_triangle(triangle.points[0].x, triangle.points[0].y,
                    triangle.points[1].x, triangle.points[1].y,
                    triangle.points[2].x, triangle.points[2].y,
                    0xAA000000);
    }
    if(render_options.vertex){
      draw_rect(triangle.points[0].x - 3, triangle.points[0].y - 3, 6, 6, 0xFFFF00AA);
      draw_rect(triangle.points[1].x - 3, triangle.points[1].y - 3, 6, 6, 0xFFFF00AA);
      draw_rect(triangle.points[2].x - 3, triangle.points[2].y - 3, 6, 6, 0xFFFF00AA);
    }

  }

  array_free(triangles_to_render);

  render_color_buffer();
  clear_color_buffer(0xFFCCCC10);

  SDL_RenderPresent(renderer);
}

void free_resources(void) {
  array_free(mesh.faces);
  array_free(mesh.vertices);
};
int main(int argc, char *argv[])
{
  is_running = initialize_window();

  setup();

  while(is_running) {
    process_input();
    update();
    render();
  }

  destroy_window();
  free_resources();

  return 0;
}
