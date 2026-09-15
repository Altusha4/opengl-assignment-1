# Assignment 1 — Computer Graphics

**Author:** Altynay Yertay

---

## Description

Four OpenGL programs written with the modern programmable pipeline.

| Program | Task | What it displays |
|---|---|---|
| `red_triangle` | 1 | A single red triangle |
| `blue_square` | 1 | A single blue square that fits inside the window |
| `picture` | 2, part 1 | The reference picture: ellipse, triangle, shaded circle, concentric squares |
| `creative` | 2, part 2 | An original picture with six objects |

All windows are 500×500 pixels.

### Task 1 — from red_triangle to blue_square

`blue_square.cpp` was created by modifying `red_triangle.cpp` in three places:

1. **The vertex array** changed from 3 triangle vertices to 4 square corners
   at `(-0.6, -0.6)`, `(0.6, -0.6)`, `(0.6, 0.6)` and `(-0.6, 0.6)`, so the
   square fits inside the window.

2. **The color array** changed from red to blue and grew from 3 to 4 vertices.

3. **The draw call** changed from `GL_TRIANGLES` with 3 vertices to
   `GL_TRIANGLE_FAN` with 4. `GL_QUADS` was not used because it is deprecated
   and unavailable in the core profile; `GL_TRIANGLE_FAN` builds the square
   from two triangles, `(0,1,2)` and `(0,2,3)`.

### Task 2 part 1 — picture

All vertices of all four objects are packed into a single VBO, and each object
is drawn with its own `glDrawArrays` call at a different offset.

- **Ellipse** — parametric circle `x = cos(a)`, `y = sin(a)` with the y radius
  scaled to 60% of the x radius, drawn as a `GL_TRIANGLE_FAN` whose first
  vertex is the centre. The first perimeter point is repeated at the end to
  close the fan.
- **Triangle** — three vertices placed at 90°, 210° and 330° on a circle.
  The corners are red, green and blue; the rasteriser interpolates the rest.
- **Shaded circle** — same fan construction, but the red channel of each
  perimeter vertex is `angle / (2π)`, which produces a smooth ramp with a
  seam on the right where the angle wraps.
- **Concentric squares** — six squares drawn largest to smallest, alternating
  white and black, each corner computed from the angles π/4, 3π/4, 5π/4, 7π/4.

### Task 2 part 2 — creative

An original picture combining every technique from part 1:

- gradient background drawn as a `GL_TRIANGLE_STRIP` of colour-interpolated bands
- speech bubble outlined by drawing a black ellipse and a slightly smaller
  white one on top, the same layering trick used for the concentric squares
- the Greek letter sigma assembled from four quads
- a custom 3×5 bitmap font: every letter is a grid of small squares, and the
  whole caption is drawn with a single `GL_TRIANGLES` call

### Modern OpenGL only

No deprecated OpenGL functions are used anywhere. The programs rely on vertex
array objects (VAO), vertex buffer objects (VBO), GLSL shaders and
`glDrawArrays`. Vertex positions and colors reach the vertex shader through
the attributes `vPosition` and `vColor`.

---

## Files

| File | Purpose |
|---|---|
| `red_triangle.cpp` | Task 1, program 1 |
| `blue_square.cpp` | Task 1, program 2 |
| `picture.cpp` | Task 2, part 1 |
| `creative.cpp` | Task 2, part 2 |
| `vshader.glsl` | Vertex shader, shared by all programs |
| `fshader.glsl` | Fragment shader, shared by all programs |
| `InitShader.cpp` | Reads, compiles and links the shader files |
| `InitShader.h` | Header for `InitShader.cpp` |
| `Makefile` | Builds all four programs |
| `README.md` | This file |

---

## How to build

From inside this folder:

```sh
make
```

The default target builds and links all four programs. To remove the
executables:

```sh
make clean
```

## How to run

```sh
./red_triangle
./blue_square
./picture
./creative
```

Press <kbd>Esc</kbd> or <kbd>Q</kbd> to close a window.

> **Important:** run the programs from inside this folder. `vshader.glsl` and
> `fshader.glsl` are loaded using relative paths, so the working directory
> must be this folder.

---

## Operating system

Developed and tested on **macOS** (Apple Silicon, OpenGL 4.1 core profile,
GLSL 1.50). No extra libraries are needed there — OpenGL and GLUT come from
the system frameworks, and only the Xcode Command Line Tools are required.

The Makefile detects **Linux** automatically and links against
`-lGL -lGLEW -lglut`. There the packages `freeglut3-dev` and `libglew-dev`
must be installed.