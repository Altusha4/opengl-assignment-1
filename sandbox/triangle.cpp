#ifdef __APPLE__
  #define GL_SILENCE_DEPRECATION
  #define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
  #include <OpenGL/gl3.h>
  #include <GLUT/glut.h>
#else
  #include <GL/glew.h>
  #include <GL/freeglut.h>
#endif

#include <cstdlib>
#include <cstdio>