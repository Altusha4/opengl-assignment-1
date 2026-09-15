#include "InitShader.h"
#include <cstdio>
#include <cstdlib>

static char* readShaderSource(const char* shaderFile)
{
    FILE* fp = fopen(shaderFile, "rb");
    if (fp == NULL) {
        fprintf(stderr, "Cannot open shader file: %s\n", shaderFile);
        fprintf(stderr, "Run the program from the folder containing the .glsl files.\n");
        return NULL;
    }

    fseek(fp, 0L, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0L, SEEK_SET);

    char* buf = new char[size + 1];
    size_t read = fread(buf, 1, size, fp);
    buf[read] = '\0';

    fclose(fp);
    return buf;
}

static GLuint compileShader(GLenum type, const char* shaderFile)
{
    char* source = readShaderSource(shaderFile);
    if (source == NULL) exit(EXIT_FAILURE);

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar**)&source, NULL);
    glCompileShader(shader);

    GLint compiled;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        fprintf(stderr, "Failed to compile shader %s:\n", shaderFile);
        GLint logSize;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize);
        char* log = new char[logSize + 1];
        glGetShaderInfoLog(shader, logSize, &logSize, log);
        fprintf(stderr, "%s\n", log);
        delete[] log;
        delete[] source;
        exit(EXIT_FAILURE);
    }

    delete[] source;
    return shader;
}

GLuint InitShader(const char* vertexShaderFile, const char* fragmentShaderFile)
{
    GLuint program = glCreateProgram();

    glAttachShader(program, compileShader(GL_VERTEX_SHADER, vertexShaderFile));
    glAttachShader(program, compileShader(GL_FRAGMENT_SHADER, fragmentShaderFile));

    glLinkProgram(program);

    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        fprintf(stderr, "Failed to link shader program:\n");
        GLint logSize;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize);
        char* log = new char[logSize + 1];
        glGetProgramInfoLog(program, logSize, &logSize, log);
        fprintf(stderr, "%s\n", log);
        delete[] log;
        exit(EXIT_FAILURE);
    }

    return program;
}