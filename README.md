# 3d CPU Renderer

A CPU-based 3d rasterizer that includes textures, camera, and clipping. It loads OBJ
files to display any model the user has. This is done with no GPU or graphics 
API to assist, making it based on first principles. 

## Different components of the model.

### Faces
~gif here

### Vertices
~gif here

### Edges
~gif here

### Backface Culling
~gif here

One interesting thing about rendering on CPU is that performance is extreamly 
important. You can see in the above gif how backface culling will speed up
performance substantually.

## Texture mapping and flat shading
~gif here

## Exporting and loading OBJ
~ Exporting gif here
~ Showing Model gif here

## Prerequisites
- make
- a C compiler
- SDL2

## Installation
1. Just clone the repo. 
2. run make.
3. run the binary found in /bin/

