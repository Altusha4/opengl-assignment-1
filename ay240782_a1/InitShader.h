#ifndef INIT_SHADER_H
#define INIT_SHADER_H

#ifdef __APPLE__
  #define GL_SILENCE_DEPRECATION
  #include <OpenGL/gl3.h>
#else
  #include <GL/glew.h>
#endif

GLuint InitShader(const char* vertexShaderFile, const char* fragmentShaderFile);

#endif