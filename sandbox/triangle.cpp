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

GLfloat vertices[3][4] = {
    {  0.0f,  0.8f, 0.0f, 1.0f },
    { -0.8f, -0.8f, 0.0f, 1.0f },
    {  0.8f, -0.8f, 0.0f, 1.0f }
};

GLfloat colors[3][4] = {
    { 1.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 1.0f, 1.0f }
};

const char* vertexShaderSource = R"(#version 150

in  vec4 vPosition;
in  vec4 vColor;
out vec4 color;

void main()
{
    color = vColor;
    gl_Position = vPosition;
}
)";

const char* fragmentShaderSource = R"(#version 150

in  vec4 color;
out vec4 fColor;

void main()
{
    fColor = color;
}
)";

GLuint createShaderProgram(const char* vsSource, const char* fsSource)
{
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vsSource, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fsSource, NULL);
    glCompileShader(fs);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint ok;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        fprintf(stderr, "Shader error:\n%s\n", log);
        exit(EXIT_FAILURE);
    }
    return program;
}

void init()
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), colors);

    GLuint program = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    glUseProgram(program);

    glClearColor(0.0, 0.0, 0.0, 1.0);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
    if (key == 033 || key == 'q' || key == 'Q')
        exit(EXIT_SUCCESS);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);

#ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    glutInitContextVersion(3, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);
#endif

    glutInitWindowSize(500, 500);
    glutCreateWindow("Step 2: Triangle");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}