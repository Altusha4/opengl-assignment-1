# Assignment 2. Computer Graphics

**Author:** Altynay Yertay, SE-2416

## Description

Two OpenGL programs built on the code of Assignment 1 (GLUT, `InitShader`,
GLSL shaders, VAO and VBO).

| Program | Part | What it does |
|---|---|---|
| `partA` | A | Main window with the rotating "black & white" square, a subwindow with an ellipse, and "window 2" with a breathing circle and a clockwise triangle. Menus, a submenu, keyboard colours, Stop/Start animation and circles added by left clicks |
| `partB` | B | The colour cube with depth testing. A menu chooses SCALE, ROTATE or TRANSLATE, and keys change X, Y, Z and the delta. The model matrix is `M = T * R * S` with `R = Rz * Ry * Rx` |

The full report is in [`report.md`](report.md).

## How to build

```sh
make
```

To remove the executables:

```sh
make clean
```

## How to run

```sh
./partA
./partB
```

> **Important:** run the programs from inside this folder. The shader files are
> loaded using relative paths.

## Controls

**Part A**

* Main window, right button: Stop Animation, Start Animation, Square Colors → White / Red / Green
* Main window, left click: add a breathing circle of a random colour
* Subwindow, right button: background colour menu
* Window 2: `r` `g` `b` `y` `o` `p` `w` change the colour of the circle and the triangle
* `Esc` or `q` in the main window or window 2: quit

**Part B** (the same list is printed when the program starts)

* Right button: SCALE, ROTATE, TRANSLATE
* `q`/`a`, `w`/`s`, `e`/`d`: increase / decrease X, Y, Z
* `+`/`-`: double / halve the delta
* `r`: reset, `Esc`: quit

## Operating system

Developed and tested on **macOS** (Apple Silicon, OpenGL core profile, GLSL
1.50). OpenGL and GLUT come from the system frameworks. On **Linux** the Makefile
links against `-lGL -lGLEW -lglut`, which needs `freeglut3-dev` and `libglew-dev`.
