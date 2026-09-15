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

const int MAX_VERTICES = 3000;

GLfloat vertices[MAX_VERTICES][4];
GLfloat colors[MAX_VERTICES][4];
int numVertices = 0;

int backgroundStart, backgroundCount;
int bubbleOuterStart, bubbleOuterCount;
int bubbleInnerStart, bubbleInnerCount;
int tailOuterStart, tailInnerStart;
int sigmaStart;
int textStart, textCount;

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

void addBackground(int bands)
{
    for (int i = 0; i <= bands; i++) {
        GLfloat t = (GLfloat)i / bands;
        GLfloat y = 1.0f - 2.0f * t;
        GLfloat r = 0.85f + 0.15f * t;
        GLfloat g = 0.82f + 0.18f * t;
        GLfloat b = 0.95f + 0.05f * t;
        addVertex(-1.0f, y, r, g, b);
        addVertex( 1.0f, y, r, g, b);
    }
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

void addTriangleShape(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2,
                      GLfloat x3, GLfloat y3, GLfloat r, GLfloat g, GLfloat b)
{
    addVertex(x1, y1, r, g, b);
    addVertex(x2, y2, r, g, b);
    addVertex(x3, y3, r, g, b);
}

void addQuad(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2,
             GLfloat x3, GLfloat y3, GLfloat x4, GLfloat y4,
             GLfloat r, GLfloat g, GLfloat b)
{
    addVertex(x1, y1, r, g, b);
    addVertex(x2, y2, r, g, b);
    addVertex(x3, y3, r, g, b);
    addVertex(x4, y4, r, g, b);
}

void addSigma(GLfloat cx, GLfloat cy, GLfloat s)
{
    GLfloat black = 0.0f;

    addQuad(cx - 0.75f * s, cy + 0.78f * s,
            cx + 0.75f * s, cy + 0.78f * s,
            cx + 0.75f * s, cy + 1.00f * s,
            cx - 0.75f * s, cy + 1.00f * s,
            black, black, black);

    addQuad(cx - 0.75f * s, cy + 1.00f * s,
            cx + 0.10f * s, cy + 0.00f * s,
            cx + 0.40f * s, cy + 0.00f * s,
            cx - 0.45f * s, cy + 1.00f * s,
            black, black, black);

    addQuad(cx - 0.75f * s, cy - 1.00f * s,
            cx + 0.10f * s, cy + 0.00f * s,
            cx + 0.40f * s, cy + 0.00f * s,
            cx - 0.45f * s, cy - 1.00f * s,
            black, black, black);

    addQuad(cx - 0.75f * s, cy - 1.00f * s,
            cx + 0.75f * s, cy - 1.00f * s,
            cx + 0.75f * s, cy - 0.78f * s,
            cx - 0.75f * s, cy - 0.78f * s,
            black, black, black);
}

const char* glyph(char c)
{
    switch (c) {
        case 'I': return "###.#..#..#.###";
        case 'F': return "####..####..#..";
        case 'E': return "####..####..###";
        case 'L': return "#..#..#..#..###";
        case 'S': return "####..###..####";
        case 'O': return "####.##.##.####";
        case 'G': return "####..#.##.####";
        case 'M': return "#.########.##.#";
        case 'A': return "####.#####.##.#";
        default:  return "...............";
    }
}

void addPixel(GLfloat x, GLfloat y, GLfloat size, GLfloat shade)
{
    addVertex(x,        y,        shade, shade, shade);
    addVertex(x + size, y,        shade, shade, shade);
    addVertex(x + size, y - size, shade, shade, shade);

    addVertex(x,        y,        shade, shade, shade);
    addVertex(x + size, y - size, shade, shade, shade);
    addVertex(x,        y - size, shade, shade, shade);
}

void addText(const char* text, GLfloat centerX, GLfloat topY, GLfloat px)
{
    int length = 0;
    while (text[length] != '\0') length++;

    GLfloat startX = centerX - (length * 4 - 1) * px / 2.0f;

    for (int i = 0; i < length; i++) {
        const char* bits = glyph(text[i]);
        for (int row = 0; row < 5; row++)
            for (int col = 0; col < 3; col++)
                if (bits[row * 3 + col] == '#')
                    addPixel(startX + (i * 4 + col) * px, topY - row * px, px, 0.1f);
    }
}

void buildScene()
{
    backgroundStart = numVertices;
    addBackground(20);
    backgroundCount = numVertices - backgroundStart;

    bubbleOuterStart = numVertices;
    addEllipse(0.0f, 0.62f, 0.62f, 0.28f, 0.0f, 0.0f, 0.0f, 60);
    bubbleOuterCount = numVertices - bubbleOuterStart;

    tailOuterStart = numVertices;
    addTriangleShape(0.06f, 0.38f, 0.26f, 0.38f, 0.14f, 0.12f,
                     0.0f, 0.0f, 0.0f);

    bubbleInnerStart = numVertices;
    addEllipse(0.0f, 0.62f, 0.60f, 0.26f, 1.0f, 1.0f, 1.0f, 60);
    bubbleInnerCount = numVertices - bubbleInnerStart;

    tailInnerStart = numVertices;
    addTriangleShape(0.085f, 0.37f, 0.235f, 0.37f, 0.145f, 0.17f,
                     1.0f, 1.0f, 1.0f);

    textStart = numVertices;
    addText("I FEEL",   0.0f, 0.78f, 0.030f);
    addText("SO SIGMA", 0.0f, 0.57f, 0.030f);
    textCount = numVertices - textStart;

    sigmaStart = numVertices;
    addSigma(0.0f, -0.42f, 0.45f);
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

    glClearColor(1.0, 1.0, 1.0, 1.0);
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLE_STRIP, backgroundStart, backgroundCount);

    glDrawArrays(GL_TRIANGLE_FAN, bubbleOuterStart, bubbleOuterCount);
    glDrawArrays(GL_TRIANGLES, tailOuterStart, 3);
    glDrawArrays(GL_TRIANGLE_FAN, bubbleInnerStart, bubbleInnerCount);
    glDrawArrays(GL_TRIANGLES, tailInnerStart, 3);

    glDrawArrays(GL_TRIANGLES, textStart, textCount);

    for (int i = 0; i < 4; i++)
        glDrawArrays(GL_TRIANGLE_FAN, sigmaStart + i * 4, 4);

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
    glutCreateWindow("Assignment 1 - Task 2 Part 2");

    init();

    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);

    glutMainLoop();
    return 0;
}