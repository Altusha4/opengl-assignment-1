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
const int MAX_CIRCLES = 40;
const int CIRCLE_SEGMENTS = 60;

struct Scene {
    GLfloat vertices[MAX_VERTICES][4];
    GLfloat colors[MAX_VERTICES][4];
    int numVertices;
    GLuint buffer;
    GLint thetaLoc, scaleLoc, offsetLoc;
};

Scene mainScene, subScene, scene2;

int mainWindow, subWindow, window2;

int squaresStart;
int ellipseStart, ellipseCount;
int circleStart, circleCount;
int triangleStart, triangleCount;

int clickStart;
int numClicks = 0;
GLfloat clickCenters[MAX_CIRCLES][2];

GLfloat subBackground[3] = { 0.1f, 0.15f, 0.4f };

float animTime = 0.0f;
int lastTime = 0;
float squareTheta = 0.0f;
float triangleTheta = 0.0f;
float breathScale = 1.0f;

enum { MENU_STOP, MENU_START };
enum { SQUARE_WHITE, SQUARE_RED, SQUARE_GREEN };
enum { BG_BLUE, BG_GREEN, BG_GRAY, BG_YELLOW };

void addVertex(Scene& s, GLfloat x, GLfloat y, GLfloat r, GLfloat g, GLfloat b)
{
    s.vertices[s.numVertices][0] = x;
    s.vertices[s.numVertices][1] = y;
    s.vertices[s.numVertices][2] = 0.0f;
    s.vertices[s.numVertices][3] = 1.0f;

    s.colors[s.numVertices][0] = r;
    s.colors[s.numVertices][1] = g;
    s.colors[s.numVertices][2] = b;
    s.colors[s.numVertices][3] = 1.0f;

    s.numVertices++;
}

void addEllipse(Scene& s, GLfloat rx, GLfloat ry, GLfloat r, GLfloat g, GLfloat b)
{
    addVertex(s, 0.0f, 0.0f, r, g, b);

    for (int i = 0; i <= CIRCLE_SEGMENTS; i++) {
        GLfloat angle = 2.0f * M_PI * i / CIRCLE_SEGMENTS;
        addVertex(s, rx * cos(angle), ry * sin(angle), r, g, b);
    }
}

void addTriangle(Scene& s, GLfloat radius, GLfloat r, GLfloat g, GLfloat b)
{
    GLfloat angles[3] = { 90.0f, 210.0f, 330.0f };

    for (int i = 0; i < 3; i++) {
        GLfloat angle = angles[i] * M_PI / 180.0f;
        addVertex(s, radius * cos(angle), radius * sin(angle), r, g, b);
    }
}

void addSquare(Scene& s, GLfloat radius, GLfloat shade)
{
    for (int i = 0; i < 4; i++) {
        GLfloat angle = M_PI / 4.0f + i * M_PI / 2.0f;
        addVertex(s, radius * cos(angle), radius * sin(angle), shade, shade, shade);
    }
}

void setColor(Scene& s, int start, int count, GLfloat r, GLfloat g, GLfloat b)
{
    for (int i = start; i < start + count; i++) {
        s.colors[i][0] = r;
        s.colors[i][1] = g;
        s.colors[i][2] = b;
    }
}

void uploadColors(Scene& s, int start, int count)
{
    glBindBuffer(GL_ARRAY_BUFFER, s.buffer);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(s.vertices) + start * sizeof(s.colors[0]),
                    count * sizeof(s.colors[0]), s.colors[start]);
}

void initScene(Scene& s)
{
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &s.buffer);
    glBindBuffer(GL_ARRAY_BUFFER, s.buffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(s.vertices) + sizeof(s.colors), NULL, GL_DYNAMIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(s.vertices), s.vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(s.vertices), sizeof(s.colors), s.colors);

    GLuint program = InitShader("vshaderA.glsl", "fshader.glsl");
    glUseProgram(program);

    GLint vPosition = glGetAttribLocation(program, "vPosition");
    glEnableVertexAttribArray(vPosition);
    glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);

    GLint vColor = glGetAttribLocation(program, "vColor");
    glEnableVertexAttribArray(vColor);
    glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(s.vertices));

    s.thetaLoc = glGetUniformLocation(program, "theta");
    s.scaleLoc = glGetUniformLocation(program, "scale");
    s.offsetLoc = glGetUniformLocation(program, "offset");
}

void setTransform(Scene& s, GLfloat theta, GLfloat scale, GLfloat x, GLfloat y)
{
    glUniform1f(s.thetaLoc, theta);
    glUniform1f(s.scaleLoc, scale);
    glUniform2f(s.offsetLoc, x, y);
}

void buildMainScene()
{
    squaresStart = mainScene.numVertices;
    GLfloat radii[6] = { 0.806f, 0.672f, 0.537f, 0.403f, 0.269f, 0.134f };
    for (int i = 0; i < 6; i++) {
        GLfloat shade = (i % 2 == 0) ? 1.0f : 0.0f;
        addSquare(mainScene, radii[i], shade);
    }
    clickStart = mainScene.numVertices;
}

void buildSubScene()
{
    ellipseStart = subScene.numVertices;
    addEllipse(subScene, 0.7f, 0.65f, 1.0f, 0.0f, 0.0f);
    ellipseCount = subScene.numVertices - ellipseStart;
}

void buildScene2()
{
    circleStart = scene2.numVertices;
    addEllipse(scene2, 0.3f, 0.3f, 1.0f, 1.0f, 1.0f);
    circleCount = scene2.numVertices - circleStart;

    triangleStart = scene2.numVertices;
    addTriangle(scene2, 0.35f, 1.0f, 1.0f, 1.0f);
    triangleCount = scene2.numVertices - triangleStart;
}

void displayMain()
{
    glClear(GL_COLOR_BUFFER_BIT);

    setTransform(mainScene, squareTheta, 0.85f, 0.0f, 0.0f);
    for (int i = 0; i < 6; i++)
        glDrawArrays(GL_TRIANGLE_FAN, squaresStart + i * 4, 4);

    for (int i = 0; i < numClicks; i++) {
        setTransform(mainScene, 0.0f, breathScale, clickCenters[i][0], clickCenters[i][1]);
        glDrawArrays(GL_TRIANGLE_FAN, clickStart + i * (CIRCLE_SEGMENTS + 2), CIRCLE_SEGMENTS + 2);
    }

    glutSwapBuffers();
}

void displaySub()
{
    glClearColor(subBackground[0], subBackground[1], subBackground[2], 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    setTransform(subScene, 0.0f, 1.0f, 0.0f, 0.0f);
    glDrawArrays(GL_TRIANGLE_FAN, ellipseStart, ellipseCount);

    glutSwapBuffers();
}

void display2()
{
    glClear(GL_COLOR_BUFFER_BIT);

    setTransform(scene2, 0.0f, breathScale, -0.45f, 0.0f);
    glDrawArrays(GL_TRIANGLE_FAN, circleStart, circleCount);

    setTransform(scene2, triangleTheta, 1.0f, 0.45f, 0.0f);
    glDrawArrays(GL_TRIANGLES, triangleStart, triangleCount);

    glutSwapBuffers();
}

void idle()
{
    int now = glutGet(GLUT_ELAPSED_TIME);
    animTime += (now - lastTime) / 1000.0f;
    lastTime = now;

    squareTheta = 1.0f * animTime;
    triangleTheta = -1.5f * animTime;
    breathScale = 1.0f + 0.25f * sin(3.0f * animTime);

    glutPostWindowRedisplay(mainWindow);
    glutPostWindowRedisplay(window2);
}

void startAnimation()
{
    lastTime = glutGet(GLUT_ELAPSED_TIME);
    glutIdleFunc(idle);
}

void mainMenu(int id)
{
    if (id == MENU_STOP)
        glutIdleFunc(NULL);
    else if (id == MENU_START)
        startAnimation();
}

void squareColorMenu(int id)
{
    GLfloat colors[3][3] = {
        { 1.0f, 1.0f, 1.0f },
        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f }
    };

    glutSetWindow(mainWindow);
    for (int i = 0; i < 6; i += 2)
        setColor(mainScene, squaresStart + i * 4, 4, colors[id][0], colors[id][1], colors[id][2]);
    uploadColors(mainScene, squaresStart, 24);
    glutPostRedisplay();
}

void subMenu(int id)
{
    GLfloat colors[4][3] = {
        { 0.1f, 0.15f, 0.4f },
        { 0.05f, 0.35f, 0.15f },
        { 0.5f, 0.5f, 0.5f },
        { 1.0f, 0.9f, 0.5f }
    };

    for (int i = 0; i < 3; i++)
        subBackground[i] = colors[id][i];
    glutPostRedisplay();
}

void mainMouse(int button, int state, int x, int y)
{
    if (button != GLUT_LEFT_BUTTON || state != GLUT_DOWN || numClicks == MAX_CIRCLES)
        return;

    int width = glutGet(GLUT_WINDOW_WIDTH);
    int height = glutGet(GLUT_WINDOW_HEIGHT);
    clickCenters[numClicks][0] = 2.0f * x / width - 1.0f;
    clickCenters[numClicks][1] = 1.0f - 2.0f * y / height;

    GLfloat r = rand() / (GLfloat)RAND_MAX;
    GLfloat g = rand() / (GLfloat)RAND_MAX;
    GLfloat b = rand() / (GLfloat)RAND_MAX;

    int start = mainScene.numVertices;
    addEllipse(mainScene, 0.08f, 0.08f, r, g, b);
    int count = mainScene.numVertices - start;

    glBindBuffer(GL_ARRAY_BUFFER, mainScene.buffer);
    glBufferSubData(GL_ARRAY_BUFFER, start * sizeof(mainScene.vertices[0]),
                    count * sizeof(mainScene.vertices[0]), mainScene.vertices[start]);
    uploadColors(mainScene, start, count);

    numClicks++;
    glutPostRedisplay();
}

void keyboard2(unsigned char key, int x, int y)
{
    if (key == 033 || key == 'q' || key == 'Q')
        exit(EXIT_SUCCESS);

    GLfloat r, g, b;
    switch (key) {
    case 'r': r = 1.0f; g = 0.0f; b = 0.0f; break;
    case 'g': r = 0.0f; g = 1.0f; b = 0.0f; break;
    case 'b': r = 0.0f; g = 0.0f; b = 1.0f; break;
    case 'y': r = 1.0f; g = 1.0f; b = 0.0f; break;
    case 'o': r = 1.0f; g = 0.5f; b = 0.0f; break;
    case 'p': r = 0.5f; g = 0.0f; b = 0.5f; break;
    case 'w': r = 1.0f; g = 1.0f; b = 1.0f; break;
    default: return;
    }

    setColor(scene2, 0, scene2.numVertices, r, g, b);
    uploadColors(scene2, 0, scene2.numVertices);
    glutPostRedisplay();
}

void keyboardMain(unsigned char key, int x, int y)
{
    if (key == 033 || key == 'q' || key == 'Q')
        exit(EXIT_SUCCESS);
}

void createMainWindow()
{
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(100, 100);
    mainWindow = glutCreateWindow("Assignment 2 - Part A");

    buildMainScene();
    initScene(mainScene);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    glutDisplayFunc(displayMain);
    glutKeyboardFunc(keyboardMain);
    glutMouseFunc(mainMouse);

    int colorMenu = glutCreateMenu(squareColorMenu);
    glutAddMenuEntry("White", SQUARE_WHITE);
    glutAddMenuEntry("Red", SQUARE_RED);
    glutAddMenuEntry("Green", SQUARE_GREEN);

    glutCreateMenu(mainMenu);
    glutAddMenuEntry("Stop Animation", MENU_STOP);
    glutAddMenuEntry("Start Animation", MENU_START);
    glutAddSubMenu("Square Colors", colorMenu);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void createSubWindow()
{
    subWindow = glutCreateSubWindow(mainWindow, 10, 10, 170, 110);

    buildSubScene();
    initScene(subScene);

    glutDisplayFunc(displaySub);

    glutCreateMenu(subMenu);
    glutAddMenuEntry("Dark Blue", BG_BLUE);
    glutAddMenuEntry("Dark Green", BG_GREEN);
    glutAddMenuEntry("Gray", BG_GRAY);
    glutAddMenuEntry("Light Yellow", BG_YELLOW);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void createWindow2()
{
    glutInitWindowSize(600, 600);
    glutInitWindowPosition(700, 100);
    window2 = glutCreateWindow("window 2");

    buildScene2();
    initScene(scene2);
    glClearColor(0.0, 0.0, 0.0, 1.0);

    glutDisplayFunc(display2);
    glutKeyboardFunc(keyboard2);
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

    createMainWindow();
    createSubWindow();
    createWindow2();

    startAnimation();

    glutMainLoop();
    return 0;
}
