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
#include <cmath>
#include "InitShader.h"

const int MAX_VERTICES = 2000;

GLfloat vertices[MAX_VERTICES][4];
GLfloat colors[MAX_VERTICES][4];
int numVertices = 0;

int ellipseStart, ellipseCount;
int triangleStart, triangleCount;
int circleStart, circleCount;
int squaresStart;

void addVertex(GLfloat x, GLfloat y, GLfloat r, GLfloat g, GLfloat b)
{
    vertices[numVertices][0] = x;
    vertices[numVertices][1] = y;
    vertices[numVertices][2] = 0.0f;
    vertices[numVertices][3] = 1.0f;

    colors[numVertices][0] = r;
    colors[numVertices][1] = g;
    colors[numVertices][2] = b;
    colors[numVertices][3] = 1.0f;

    numVertices++;
}

void addEllipse(GLfloat cx, GLfloat cy, GLfloat rx, GLfloat ry,
                GLfloat r, GLfloat g, GLfloat b, int segments)
{
    addVertex(cx, cy, r, g, b);

    for (int i = 0; i <= segments; i++) {
        GLfloat angle = 2.0f * M_PI * i / segments;
        GLfloat x = cx + rx * cos(angle);
        GLfloat y = cy + ry * sin(angle);
        addVertex(x, y, r, g, b);
    }
}

void addTriangle(GLfloat cx, GLfloat cy, GLfloat radius)
{
    GLfloat angles[3] = { 90.0f, 210.0f, 330.0f };

    GLfloat triColors[3][3] = {
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 1.0f }
    };

    for (int i = 0; i < 3; i++) {
        GLfloat angle = angles[i] * M_PI / 180.0f;
        GLfloat x = cx + radius * cos(angle);
        GLfloat y = cy + radius * sin(angle);
        addVertex(x, y, triColors[i][0], triColors[i][1], triColors[i][2]);
    }
}

void addShadedCircle(GLfloat cx, GLfloat cy, GLfloat radius, int segments)
{
    addVertex(cx, cy, 0.15f, 0.0f, 0.0f);

    for (int i = 0; i <= segments; i++) {
        GLfloat angle = 2.0f * M_PI * i / segments;
        GLfloat x = cx + radius * cos(angle);
        GLfloat y = cy + radius * sin(angle);
        GLfloat red = angle / (2.0f * M_PI);
        addVertex(x, y, red, 0.0f, 0.0f);
    }
}

void addSquare(GLfloat cx, GLfloat cy, GLfloat radius, GLfloat shade)
{
    for (int i = 0; i < 4; i++) {
        GLfloat angle = M_PI / 4.0f + i * M_PI / 2.0f;
        GLfloat x = cx + radius * cos(angle);
        GLfloat y = cy + radius * sin(angle);
        addVertex(x, y, shade, shade, shade);
    }
}

void buildScene()
{
    ellipseStart = numVertices;
    addEllipse(-0.6f, 0.73f, 0.2f, 0.12f, 1.0f, 0.0f, 0.0f, 60);
    ellipseCount = numVertices - ellipseStart;

    triangleStart = numVertices;
    addTriangle(0.0f, 0.78f, 0.24f);
    triangleCount = numVertices - triangleStart;

    circleStart = numVertices;
    addShadedCircle(0.6f, 0.72f, 0.2f, 60);
    circleCount = numVertices - circleStart;

    squaresStart = numVertices;
    GLfloat radii[6] = { 0.806f, 0.672f, 0.537f, 0.403f, 0.269f, 0.134f };
    for (int i = 0; i < 6; i++) {
        GLfloat shade = (i % 2 == 0) ? 1.0f : 0.0f;
        addSquare(0.0f, -0.2f, radii[i], shade);
    }
}

void init()
{
    buildScene();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), colors);

    GLuint program = InitShader("vshader.glsl", "fshader.glsl");
    glUseProgram(program);

    GLint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

    GLint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(vertices));

    glClearColor(0.0, 0.0, 0.0, 1.0);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_FAN, ellipseStart, ellipseCount);
    glDrawArrays(GL_TRIANGLES, triangleStart, triangleCount);
    glDrawArrays(GL_TRIANGLE_FAN, circleStart, circleCount);

    for (int i = 0; i < 6; i++)
        glDrawArrays(GL_TRIANGLE_FAN, squaresStart + i * 4, 4);

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
    glutCreateWindow("Assignment 1 - Task 2");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}