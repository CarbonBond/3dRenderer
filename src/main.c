#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_pixels.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

#include "display.h"
#include "triangle.h"
#include "vector.h"
#include "mesh.h"
#include "array.h"

/*NOTE(Brandon):Array of triangles  that should be rendered frame by frame */
triangle_t* triangles_to_render = NULL;

vec3_t camera_position = {.x = 0, .y = 0, .z = 0};

int fov_factor = 640;

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


  //load_cube_mesh_data();
  load_obj_file_data("./asset/cube.obj", &mesh);
  //load_obj_file_data("./asset/suzanne.obj", &mesh);
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

vec2_t project(vec3_t point) {
  vec2_t projected_point = {
    .x = ((point.x * fov_factor) / point.z) ,
    .y = ((point.y * fov_factor) / point.z)   };
  return projected_point;
}

void update(void){

  int time_to_wait = FRAME_TARGET_TIME - (SDL_GetTicks() - previous_frame_time);

  if(time_to_wait > 0 && time_to_wait <= FRAME_TARGET_TIME) 
    SDL_Delay(time_to_wait);

  triangles_to_render = NULL;

  mesh.rotation.y += 0.002;
  mesh.rotation.x += 0.003;
  mesh.rotation.z += 0.004;


  /*NOTE(Brandon): Loop over all the faces */
  for (int i = 0; i < (array_length(mesh.faces)); i++) {
    face_t triangle_face = mesh.faces[i];

    vec3_t face_vertices[3];
    face_vertices[0] = mesh.vertices[triangle_face.a - 1];
    face_vertices[1] = mesh.vertices[triangle_face.b - 1];
    face_vertices[2] = mesh.vertices[triangle_face.c - 1];

    vec3_t transformed_vertices[3];


    for (int j = 0; j < 3; j++) {
      vec3_t transformed_vertex = face_vertices[j];
      
      transformed_vertex = vec3_rotate_x(transformed_vertex, mesh.rotation.x);
      transformed_vertex = vec3_rotate_y(transformed_vertex, mesh.rotation.y);
      transformed_vertex = vec3_rotate_z(transformed_vertex, mesh.rotation.z);

      /*NOTE(Brandon):Translate the vertex away from the camera */
      transformed_vertex.z += 5;
      transformed_vertices[j] = transformed_vertex;
    }

    
    /*
     * NOTE(Brandon): Get the Normal vector 
     * */
    vec3_t vector_a = transformed_vertices[0];
    vec3_t vector_b = transformed_vertices[1];
    vec3_t vector_c = transformed_vertices[2];
    vec3_t vector_ab = vec3_sub(vector_b,vector_a);
    vec3_t vector_ac = vec3_sub(vector_c, vector_a);
    /*
     * NOTE(Brandon):Compute normal vector, (Left handed) 
     * */
    vec3_normlize(&vector_ab);
    vec3_normlize(&vector_ac);
    vec3_t normal = vec3_cross(vector_ab, vector_ac);

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


    vec2_t projected_points[3];
    for(int j = 0; j < 3; j++){
      projected_points[j] = project(transformed_vertices[j]);
      /*NOTE(Brandon): Scale and translate the points to the middle of the
       *               screen*/
      projected_points[j].x += (window_width/2.);
      projected_points[j].y += (window_height/2.);
    }

    //Note(Brandon): Calculate avg depth of face
    float avg_depth = (transformed_vertices[0].z +
                       transformed_vertices[1].z+
                       transformed_vertices[2].z) / 3; 

    triangle_t projected_triangle = {
      .points = {
        { projected_points[0].x, projected_points[0].y },
        { projected_points[1].x, projected_points[1].y },
        { projected_points[2].x, projected_points[2].y },
      },
      .avg_depth = avg_depth,
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
                           0xAA00AA00);
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
  clear_color_buffer(0xFF888888);

  SDL_RenderPresent(renderer);
}

void free_resources(void) {
  array_free(mesh.faces);
  array_free(mesh.vertices);
  free(color_buffer);
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
