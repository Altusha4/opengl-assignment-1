Assignment 1 - Task 1: Getting Started
Course: Computer Graphics
Author: Altynay Yertay

DESCRIPTION
-----------
This folder contains two OpenGL programs:

  1. red_triangle  - displays a single red triangle
  2. blue_square   - displays a single blue square that fits
                     inside the window

Both open a 500x500 window with a black background.

blue_square was created by modifying red_triangle.cpp in three
places:

  1. the vertex array was changed from 3 triangle vertices to
     4 square corners at (-0.6, -0.6), (0.6, -0.6), (0.6, 0.6)
     and (-0.6, 0.6), so the square fits inside the window;
  2. the color array was changed from red to blue and extended
     from 3 to 4 vertices;
  3. the draw call was changed from GL_TRIANGLES with 3 vertices
     to GL_TRIANGLE_FAN with 4 vertices. GL_QUADS was not used
     because it is deprecated and unavailable in the core
     profile; GL_TRIANGLE_FAN builds the square from two
     triangles, (0,1,2) and (0,2,3).

No deprecated OpenGL functions are used. The programs use the
modern programmable pipeline only: vertex array objects (VAO),
vertex buffer objects (VBO), GLSL shaders and glDrawArrays.
Vertex positions and colors are passed to the vertex shader
through the attributes "vPosition" and "vColor".

FILES
-----
  red_triangle.cpp - program 1 (red triangle)
  blue_square.cpp  - program 2 (blue square)
  vshader.glsl     - vertex shader, shared by both programs
  fshader.glsl     - fragment shader, shared by both programs
  InitShader.cpp   - loads, compiles and links the shader files
  InitShader.h     - header for InitShader.cpp
  Makefile         - builds both programs
  README           - this file

HOW TO BUILD
------------
  cd into this folder and type:

      make

  The default target builds and links both programs.
  To remove the executables:

      make clean

HOW TO RUN
----------
  ./red_triangle
  ./blue_square

  Press Esc or Q to close a program window.

  IMPORTANT: run the programs from inside this folder. The
  shader files vshader.glsl and fshader.glsl are loaded using
  relative paths, so the working directory must be this folder.

OPERATING SYSTEM
----------------
  Developed and tested on macOS (Apple Silicon, OpenGL 4.1 core
  profile, GLSL 1.50). On macOS no extra libraries are needed:
  OpenGL and GLUT are provided by the system frameworks, and
  only the Xcode Command Line Tools are required.

  The Makefile also detects Linux automatically and links
  against -lGL -lGLEW -lglut. On Linux the packages
  freeglut3-dev and libglew-dev must be installed.
