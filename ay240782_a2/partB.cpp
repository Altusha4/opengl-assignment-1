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

struct mat4 {
    GLfloat m[4][4];
};

const int NUM_VERTICES = 36;

GLfloat points[NUM_VERTICES][4];
GLfloat colors[NUM_VERTICES][4];
int numPoints = 0;

GLfloat cubeVertices[8][3] = {
    { -0.5f, -0.5f,  0.5f },
    { -0.5f,  0.5f,  0.5f },
    {  0.5f,  0.5f,  0.5f },
    {  0.5f, -0.5f,  0.5f },
    { -0.5f, -0.5f, -0.5f },
    { -0.5f,  0.5f, -0.5f },
    {  0.5f,  0.5f, -0.5f },
    {  0.5f, -0.5f, -0.5f }
};

GLfloat vertexColors[8][3] = {
    { 0.0f, 0.0f, 0.0f },
    { 1.0f, 0.0f, 0.0f },
    { 1.0f, 1.0f, 0.0f },
    { 0.0f, 1.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f },
    { 1.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f },
    { 0.0f, 1.0f, 1.0f }
};

enum { SCALE, ROTATE, TRANSLATE };

const GLfloat DEFAULT_DELTA[3] = { 0.1f, 10.0f, 0.1f };

int mode = ROTATE;
GLfloat scaleXYZ[3];
GLfloat rotateXYZ[3];
GLfloat translateXYZ[3];
GLfloat delta[3];

GLint modelLoc;

void addPoint(int v)
{
    for (int i = 0; i < 3; i++) {
        points[numPoints][i] = cubeVertices[v][i];
        colors[numPoints][i] = vertexColors[v][i];
    }
    points[numPoints][3] = 1.0f;
    colors[numPoints][3] = 1.0f;
    numPoints++;
}

void quad(int a, int b, int c, int d)
{
    addPoint(a);
    addPoint(b);
    addPoint(c);
    addPoint(a);
    addPoint(c);
    addPoint(d);
}

void colorcube()
{
    quad(1, 0, 3, 2);
    quad(2, 3, 7, 6);
    quad(3, 0, 4, 7);
    quad(6, 5, 1, 2);
    quad(4, 5, 6, 7);
    quad(5, 4, 0, 1);
}

mat4 identity()
{
    mat4 r;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            r.m[i][j] = (i == j) ? 1.0f : 0.0f;
    return r;
}

mat4 multiply(const mat4& a, const mat4& b)
{
    mat4 r;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            r.m[i][j] = 0.0f;
            for (int k = 0; k < 4; k++)
                r.m[i][j] += a.m[i][k] * b.m[k][j];
        }
    return r;
}

mat4 scaleMatrix(GLfloat sx, GLfloat sy, GLfloat sz)
{
    mat4 r = identity();
    r.m[0][0] = sx;
    r.m[1][1] = sy;
    r.m[2][2] = sz;
    return r;
}

mat4 translateMatrix(GLfloat tx, GLfloat ty, GLfloat tz)
{
    mat4 r = identity();
    r.m[0][3] = tx;
    r.m[1][3] = ty;
    r.m[2][3] = tz;
    return r;
}

mat4 rotateX(GLfloat degrees)
{
    GLfloat a = degrees * M_PI / 180.0f;
    mat4 r = identity();
    r.m[1][1] = cos(a);
    r.m[1][2] = -sin(a);
    r.m[2][1] = sin(a);
    r.m[2][2] = cos(a);
    return r;
}

mat4 rotateY(GLfloat degrees)
{
    GLfloat a = degrees * M_PI / 180.0f;
    mat4 r = identity();
    r.m[0][0] = cos(a);
    r.m[0][2] = sin(a);
    r.m[2][0] = -sin(a);
    r.m[2][2] = cos(a);
    return r;
}

mat4 rotateZ(GLfloat degrees)
{
    GLfloat a = degrees * M_PI / 180.0f;
    mat4 r = identity();
    r.m[0][0] = cos(a);
    r.m[0][1] = -sin(a);
    r.m[1][0] = sin(a);
    r.m[1][1] = cos(a);
    return r;
}

void resetTransformations()
{
    for (int i = 0; i < 3; i++) {
        scaleXYZ[i] = 1.0f;
        rotateXYZ[i] = 0.0f;
        translateXYZ[i] = 0.0f;
        delta[i] = DEFAULT_DELTA[i];
    }
}

void updateTitle()
{
    const char* titles[3] = {
        "Assignment 2 - Part B - SCALE",
        "Assignment 2 - Part B - ROTATE",
        "Assignment 2 - Part B - TRANSLATE"
    };
    glutSetWindowTitle(titles[mode]);
}

void printInstructions()
{
    printf("Assignment 2 - Part B: 3D cube transformations\n");
    printf("\n");
    printf("Right mouse button: menu to choose the transformation\n");
    printf("  SCALE, ROTATE or TRANSLATE (the current one is shown in the title)\n");
    printf("\n");
    printf("Keys for the current transformation:\n");
    printf("  q / a   increase / decrease X\n");
    printf("  w / s   increase / decrease Y\n");
    printf("  e / d   increase / decrease Z\n");
    printf("  + / -   double / halve the delta of the current transformation\n");
    printf("  r       reset all transformations and deltas\n");
    printf("  Esc     quit\n");
    printf("\n");
    printf("Default deltas: scale 0.1, rotate 10 degrees, translate 0.1\n");
    printf("The model matrix is M = T * R * S with R = Rz * Ry * Rx\n");
    fflush(stdout);
}

void init()
{
    colorcube();

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    GLuint buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(points), points);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors);

    GLuint program = InitShader("vshaderB.glsl", "fshader.glsl");
    glUseProgram(program);

    GLint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

    GLint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(points));

    modelLoc = glGetUniformLocation(program, "model");

    glEnable(GL_DEPTH_TEST);
    glClearColor(0.2, 0.2, 0.2, 1.0);

    resetTransformations();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    mat4 S = scaleMatrix(scaleXYZ[0], scaleXYZ[1], scaleXYZ[2]);
    mat4 R = multiply(rotateZ(rotateXYZ[2]), multiply(rotateY(rotateXYZ[1]), rotateX(rotateXYZ[0])));
    mat4 T = translateMatrix(translateXYZ[0], translateXYZ[1], translateXYZ[2]);
    mat4 M = multiply(T, multiply(R, S));

    glUniformMatrix4fv(modelLoc, 1, GL_TRUE, &M.m[0][0]);
    glDrawArrays(GL_TRIANGLES, 0, NUM_VERTICES);

    glutSwapBuffers();
}

void keyboard(unsigned char key, int x, int y)
{
    GLfloat* values[3] = { scaleXYZ, rotateXYZ, translateXYZ };
    GLfloat* v = values[mode];

    switch (key) {
    case 'q': v[0] += delta[mode]; break;
    case 'a': v[0] -= delta[mode]; break;
    case 'w': v[1] += delta[mode]; break;
    case 's': v[1] -= delta[mode]; break;
    case 'e': v[2] += delta[mode]; break;
    case 'd': v[2] -= delta[mode]; break;
    case '+':
    case '=': delta[mode] *= 2.0f; break;
    case '-': delta[mode] *= 0.5f; break;
    case 'r': resetTransformations(); break;
    case 033: exit(EXIT_SUCCESS);
    default: return;
    }

    glutPostRedisplay();
}

void menu(int id)
{
    mode = id;
    updateTitle();
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);

#ifdef __APPLE__
    glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
#else
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitContextVersion(3, 2);
    glutInitContextProfile(GLUT_CORE_PROFILE);
#endif

    glutInitWindowSize(600, 600);
    glutCreateWindow("Assignment 2 - Part B - ROTATE");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);

    glutCreateMenu(menu);
    glutAddMenuEntry("SCALE", SCALE);
    glutAddMenuEntry("ROTATE", ROTATE);
    glutAddMenuEntry("TRANSLATE", TRANSLATE);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    printInstructions();

    glutMainLoop();
    return 0;
}
