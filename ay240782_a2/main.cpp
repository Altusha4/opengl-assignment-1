#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <random>
#include <vector>

constexpr int   WIN_W   = 1000;
constexpr int   WIN_H   = 700;
constexpr float SCENE_W = 1000.0f;
constexpr float SCENE_H = 700.0f;
constexpr float HORIZON = 330.0f;
constexpr float SUN_R   = 32.0f;
constexpr float PI      = 3.14159265f;

static const char* VERTEX_SHADER = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec4 aDayColor;
layout(location = 2) in vec4 aNightColor;

uniform vec2  uOffset;
uniform float uNight;
uniform float uBrightness;
uniform float uAlpha;
uniform float uPointSize;

out vec4 vColor;

void main()
{
    vec2 p = aPos + uOffset;
    gl_Position  = vec4(p.x / 500.0 - 1.0, p.y / 350.0 - 1.0, 0.0, 1.0);
    gl_PointSize = uPointSize;

    vec4 c = mix(aDayColor, aNightColor, uNight);
    vColor = vec4(c.rgb * uBrightness, c.a * uAlpha);
}
)";

static const char* FRAGMENT_SHADER = R"(
#version 330 core
in vec4 vColor;
out vec4 FragColor;

uniform bool uRoundPoints;

void main()
{
    vec4 c = vColor;
    if (uRoundPoints) {
        float r = length(gl_PointCoord - vec2(0.5));
        if (r > 0.5) discard;
        c.a *= smoothstep(0.5, 0.1, r);
    }
    FragColor = c;
}
)";

static GLuint compileShader(GLenum type, const char* src)
{
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        std::fprintf(stderr, "Shader compile error:\n%s\n", log);
        std::exit(EXIT_FAILURE);
    }
    return s;
}

static GLuint createProgram()
{
    GLuint vs = compileShader(GL_VERTEX_SHADER, VERTEX_SHADER);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER);
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    GLint ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(p, sizeof(log), nullptr, log);
        std::fprintf(stderr, "Program link error:\n%s\n", log);
        std::exit(EXIT_FAILURE);
    }
    glDeleteShader(vs);
    glDeleteShader(fs);
    return p;
}

struct Vec2  { float x, y; };
struct Color { float r, g, b, a; };
struct Tone  { Color day, night; };

static Color rgb(float r, float g, float b, float a = 1.0f) { return {r, g, b, a}; }

static Color lerp(const Color& a, const Color& b, float t)
{
    return {a.r + (b.r - a.r) * t, a.g + (b.g - a.g) * t,
            a.b + (b.b - a.b) * t, a.a + (b.a - a.a) * t};
}

static Tone lerp(const Tone& a, const Tone& b, float t)
{
    return {lerp(a.day, b.day, t), lerp(a.night, b.night, t)};
}

static Tone fade(Tone t, float k)
{
    t.day.a *= k;
    t.night.a *= k;
    return t;
}

static float smooth01(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

struct Vertex { float x, y; Color day, night; };

struct Mesh {
    GLenum mode;
    std::vector<Vertex> verts;
    GLuint vao = 0, vbo = 0;

    explicit Mesh(GLenum m = GL_TRIANGLES) : mode(m) {}

    void vertex(Vec2 p, const Tone& t) { verts.push_back({p.x, p.y, t.day, t.night}); }

    void point(Vec2 p, const Tone& t) { vertex(p, t); }

    void line(Vec2 a, Vec2 b, const Tone& ta, const Tone& tb, float width = 1.0f)
    {
        float dx = b.x - a.x, dy = b.y - a.y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len < 1e-4f) return;
        Vec2 n = {-dy / len, dx / len};
        int strands = std::max(1, (int)std::lround(width * 2.0f));
        for (int i = 0; i < strands; ++i) {
            float o = (i - (strands - 1) * 0.5f) * 0.5f;
            vertex({a.x + n.x * o, a.y + n.y * o}, ta);
            vertex({b.x + n.x * o, b.y + n.y * o}, tb);
        }
    }

    void triangle(Vec2 a, Vec2 b, Vec2 c, const Tone& ta, const Tone& tb, const Tone& tc)
    {
        vertex(a, ta);
        vertex(b, tb);
        vertex(c, tc);
    }

    void quad(Vec2 bl, Vec2 br, Vec2 tr, Vec2 tl,
              const Tone& tbl, const Tone& tbr, const Tone& ttr, const Tone& ttl)
    {
        triangle(bl, br, tr, tbl, tbr, ttr);
        triangle(bl, tr, tl, tbl, ttr, ttl);
    }

    void circle(Vec2 c, float r, const Tone& center, const Tone& edge, int seg = 48)
    {
        for (int i = 0; i < seg; ++i) {
            float a0 = 2.0f * PI * i / seg, a1 = 2.0f * PI * (i + 1) / seg;
            triangle(c, {c.x + r * std::cos(a0), c.y + r * std::sin(a0)},
                        {c.x + r * std::cos(a1), c.y + r * std::sin(a1)}, center, edge, edge);
        }
    }

    void softEllipse(Vec2 c, float rx, float ry, const Tone& core, const Tone& edge,
                     float inner = 0.5f, int seg = 36)
    {
        for (int i = 0; i < seg; ++i) {
            float a0 = 2.0f * PI * i / seg, a1 = 2.0f * PI * (i + 1) / seg;
            Vec2 d0 = {rx * std::cos(a0), ry * std::sin(a0)};
            Vec2 d1 = {rx * std::cos(a1), ry * std::sin(a1)};
            Vec2 in0 = {c.x + d0.x * inner, c.y + d0.y * inner};
            Vec2 in1 = {c.x + d1.x * inner, c.y + d1.y * inner};
            Vec2 out0 = {c.x + d0.x, c.y + d0.y}, out1 = {c.x + d1.x, c.y + d1.y};
            triangle(c, in0, in1, core, core, core);
            quad(in0, out0, out1, in1, core, edge, edge, core);
        }
    }

    void upload()
    {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);

        const GLsizei stride = sizeof(Vertex);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, x));
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, day));
        glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (void*)offsetof(Vertex, night));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glEnableVertexAttribArray(2);
        glBindVertexArray(0);
    }

    void draw(GLint first, GLsizei count) const
    {
        glBindVertexArray(vao);
        glDrawArrays(mode, first, count);
    }
    void draw() const { draw(0, (GLsizei)verts.size()); }

    void release()
    {
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);
    }
};

namespace pal {
const Tone skyHorizon = {rgb(1.00f, 0.75f, 0.52f), rgb(0.15f, 0.14f, 0.28f)};
const Tone skyPeach   = {rgb(0.99f, 0.70f, 0.56f), rgb(0.09f, 0.09f, 0.22f)};
const Tone skyPink    = {rgb(0.92f, 0.64f, 0.64f), rgb(0.05f, 0.06f, 0.16f)};
const Tone skyLilac   = {rgb(0.76f, 0.60f, 0.69f), rgb(0.03f, 0.04f, 0.11f)};
const Tone skyTop     = {rgb(0.56f, 0.51f, 0.63f), rgb(0.01f, 0.02f, 0.07f)};

const Tone seaHorizon = {rgb(0.60f, 0.54f, 0.60f), rgb(0.12f, 0.13f, 0.26f)};
const Tone seaFar     = {rgb(0.30f, 0.42f, 0.50f), rgb(0.05f, 0.08f, 0.17f)};
const Tone seaMid     = {rgb(0.15f, 0.40f, 0.45f), rgb(0.03f, 0.06f, 0.12f)};
const Tone seaNear    = {rgb(0.24f, 0.55f, 0.56f), rgb(0.05f, 0.10f, 0.16f)};

const Tone foam       = {rgb(0.97f, 0.95f, 0.96f), rgb(0.70f, 0.76f, 0.88f)};
const Tone silhouette = {rgb(0.25f, 0.20f, 0.28f), rgb(0.02f, 0.02f, 0.06f)};
}

static Tone shade(Tone t, float k)
{
    for (Color* c : {&t.day, &t.night}) {
        c->r = std::min(1.0f, c->r * k);
        c->g = std::min(1.0f, c->g * k);
        c->b = std::min(1.0f, c->b * k);
    }
    return t;
}

struct Row { GLint first; GLsizei count; float depth; };

struct Cloud {
    Mesh mesh;
    float x, y, speed, margin;
};

struct Scene {
    Mesh sky, sunGlow, sun, moonCraters, sea, swells, wash, rocks, boat;
    Mesh stars{GL_POINTS}, bigStars{GL_POINTS};
    Mesh sunPath{GL_LINES}, wavesA{GL_LINES}, wavesB{GL_LINES}, foam{GL_LINES};
    Mesh rockCracks{GL_LINES}, boatRig{GL_LINES}, birdsUp{GL_LINES}, birdsDown{GL_LINES};
    std::vector<Cloud> cloudsBack, cloudsFront;
    std::vector<Row> pathRows;
    std::vector<Row> swellRows;

    std::vector<Mesh*> all()
    {
        std::vector<Mesh*> list = {&sky, &sunGlow, &sun, &moonCraters, &sea, &swells, &wash,
                                   &rocks, &boat, &stars, &bigStars, &sunPath, &wavesA, &wavesB,
                                   &foam, &rockCracks, &boatRig, &birdsUp, &birdsDown};
        for (Cloud& c : cloudsBack) list.push_back(&c.mesh);
        for (Cloud& c : cloudsFront) list.push_back(&c.mesh);
        return list;
    }
};

static Tone skyTone(float t)
{
    if (t < 0.10f) return lerp(pal::skyHorizon, pal::skyPeach, t / 0.10f);
    if (t < 0.35f) return lerp(pal::skyPeach, pal::skyPink, (t - 0.10f) / 0.25f);
    if (t < 0.65f) return lerp(pal::skyPink, pal::skyLilac, (t - 0.35f) / 0.30f);
    return lerp(pal::skyLilac, pal::skyTop, (t - 0.65f) / 0.35f);
}

static void buildSky(Mesh& m)
{
    const int bands = 32;
    for (int i = 0; i < bands; ++i) {
        float t0 = (float)i / bands, t1 = (float)(i + 1) / bands;
        float y0 = HORIZON - 2.0f + t0 * (SCENE_H - HORIZON + 2.0f);
        float y1 = HORIZON - 2.0f + t1 * (SCENE_H - HORIZON + 2.0f);
        Tone a = skyTone(t0), b = skyTone(t1);
        m.quad({0, y0}, {SCENE_W, y0}, {SCENE_W, y1}, {0, y1}, a, a, b, b);
    }
}

static void buildStars(Mesh& small, Mesh& big)
{
    std::mt19937 rng(7);
    std::uniform_real_distribution<float> rx(0.0f, SCENE_W), ry(HORIZON + 40.0f, SCENE_H), rc(0.0f, 1.0f);
    for (int i = 0; i < 190; ++i) {
        Vec2 p = {rx(rng), ry(rng)};
        Color c = lerp(rgb(0.80f, 0.87f, 1.0f), rgb(1.0f, 0.95f, 0.80f), rc(rng));
        c.a = 0.55f + 0.45f * (p.y - HORIZON) / (SCENE_H - HORIZON);
        (i % 5 == 0 ? big : small).point(p, {c, c});
    }
}

static void buildSun(Mesh& glow, Mesh& disk, Mesh& craters)
{
    const Tone warm   = {rgb(1.00f, 0.86f, 0.45f, 0.70f), rgb(0.75f, 0.82f, 1.00f, 0.30f)};
    const Tone gapSun = {rgb(1.00f, 0.90f, 0.45f, 0.85f), rgb(0.75f, 0.82f, 1.00f, 0.12f)};
    glow.softEllipse({0, 0}, 330, 48, warm, fade(warm, 0), 0.15f, 64);
    glow.softEllipse({0, 58}, 210, 20, gapSun, fade(gapSun, 0), 0.3f, 48);
    glow.circle({0, 0}, SUN_R * 4.0f, warm, fade(warm, 0), 64);
    glow.circle({0, 0}, SUN_R * 1.8f, warm, fade(warm, 0), 64);

    const Tone center = {rgb(1.00f, 1.00f, 0.90f), rgb(0.98f, 0.98f, 0.94f)};
    const Tone edge   = {rgb(1.00f, 0.80f, 0.28f), rgb(0.80f, 0.83f, 0.90f)};
    disk.circle({0, 0}, SUN_R, center, edge, 64);

    const Tone crater = {rgb(1.0f, 0.8f, 0.5f, 0.0f), rgb(0.66f, 0.69f, 0.76f, 0.6f)};
    const Tone rim    = {rgb(1.0f, 0.8f, 0.5f, 0.0f), rgb(0.74f, 0.77f, 0.84f, 0.2f)};
    craters.circle({-10, 8}, 8, crater, rim, 24);
    craters.circle({11, -6}, 6, crater, rim, 24);
    craters.circle({-4, -14}, 4, crater, rim, 20);
    craters.circle({14, 12}, 3, crater, rim, 16);
}

struct CloudStyle { Tone core, lit; };

static Cloud makeCloud(std::mt19937& rng, const CloudStyle& st, float x, float y,
                       float width, float thick, int puffs, float speed)
{
    Cloud c{Mesh(GL_TRIANGLES), x, y, speed, width * 0.8f};
    std::uniform_real_distribution<float> r01(0.0f, 1.0f);
    struct Puff { float x, y, rx, ry; };
    std::vector<Puff> list;
    for (int i = 0; i < puffs; ++i)
        list.push_back({(r01(rng) - 0.5f) * width, (r01(rng) - 0.5f) * thick * 0.6f,
                        width * (0.15f + 0.15f * r01(rng)), thick * (0.4f + 0.3f * r01(rng))});
    for (const Puff& p : list)
        c.mesh.softEllipse({p.x, p.y - p.ry * 0.4f}, p.rx * 0.9f, p.ry * 0.7f, st.lit, fade(st.lit, 0));
    for (const Puff& p : list)
        c.mesh.softEllipse({p.x, p.y}, p.rx, p.ry, st.core, fade(st.core, 0));
    return c;
}

static void buildClouds(std::vector<Cloud>& back, std::vector<Cloud>& front)
{
    const CloudStyle pink   = {{rgb(0.95f, 0.72f, 0.70f, 0.55f), rgb(0.14f, 0.14f, 0.24f, 0.55f)},
                               {rgb(1.00f, 0.84f, 0.72f, 0.45f), rgb(0.22f, 0.24f, 0.36f, 0.35f)}};
    const CloudStyle lilac  = {{rgb(0.64f, 0.56f, 0.66f, 0.60f), rgb(0.09f, 0.09f, 0.17f, 0.65f)},
                               {rgb(0.96f, 0.74f, 0.70f, 0.40f), rgb(0.20f, 0.22f, 0.34f, 0.30f)}};
    const CloudStyle dark   = {{rgb(0.43f, 0.37f, 0.47f, 0.85f), rgb(0.05f, 0.05f, 0.11f, 0.85f)},
                               {rgb(0.98f, 0.66f, 0.52f, 0.70f), rgb(0.26f, 0.28f, 0.40f, 0.45f)}};
    const CloudStyle streak = {{rgb(0.58f, 0.46f, 0.52f, 0.80f), rgb(0.07f, 0.07f, 0.14f, 0.80f)},
                               {rgb(1.00f, 0.74f, 0.42f, 0.75f), rgb(0.28f, 0.30f, 0.44f, 0.45f)}};
    std::mt19937 rng(21);

    back.push_back(makeCloud(rng, pink,  150, 655, 460, 80, 10, 4.0f));
    back.push_back(makeCloud(rng, pink,  620, 675, 500, 60, 10, 4.0f));
    back.push_back(makeCloud(rng, lilac, 960, 625, 380, 70,  9, 4.0f));
    back.push_back(makeCloud(rng, lilac, 360, 590, 450, 55, 10, 5.0f));
    back.push_back(makeCloud(rng, pink,  820, 560, 420, 50,  9, 5.0f));
    back.push_back(makeCloud(rng, dark,  200, 490, 520, 45, 12, 6.0f));
    back.push_back(makeCloud(rng, dark,  700, 475, 560, 40, 12, 6.0f));
    back.push_back(makeCloud(rng, dark, 1020, 505, 300, 35,  8, 6.0f));

    front.push_back(makeCloud(rng, streak, 300, 402, 460, 18, 10, 8.0f));
    front.push_back(makeCloud(rng, streak, 830, 412, 380, 16,  9, 8.0f));
    front.push_back(makeCloud(rng, streak, 520, 386, 300, 10,  8, 9.0f));
    front.push_back(makeCloud(rng, streak,  90, 372, 260, 10,  7, 9.0f));
}

static void buildSea(Mesh& m)
{
    const int bands = 24;
    auto tone = [](float d) {
        if (d < 0.06f) return lerp(pal::seaHorizon, pal::seaFar, d / 0.06f);
        if (d < 0.35f) return lerp(pal::seaFar, pal::seaMid, (d - 0.06f) / 0.29f);
        return lerp(pal::seaMid, pal::seaNear, (d - 0.35f) / 0.65f);
    };
    for (int i = 0; i < bands; ++i) {
        float d0 = (float)i / bands, d1 = (float)(i + 1) / bands;
        float yTop = HORIZON * (1.0f - d0), yBot = HORIZON * (1.0f - d1);
        m.quad({0, yBot}, {SCENE_W, yBot}, {SCENE_W, yTop}, {0, yTop},
               tone(d1), tone(d1), tone(d0), tone(d0));
    }
}

static void buildSunPath(Mesh& m, std::vector<Row>& rows)
{
    const Tone gold = {rgb(1.00f, 0.86f, 0.60f, 0.80f), rgb(0.88f, 0.92f, 1.00f, 0.85f)};
    std::mt19937 rng(3);
    std::uniform_real_distribution<float> r01(0.0f, 1.0f);
    for (float y = HORIZON - 2.0f; y > 150.0f; y -= 2.5f) {
        float d = (HORIZON - y) / (HORIZON - 150.0f);
        float halfW = 18.0f + 80.0f * d;
        Tone t = fade(gold, 1.0f - 0.6f * d);
        Row row = {(GLint)m.verts.size(), 0, d};
        int dashes = 2 + (int)(r01(rng) * 3.0f);
        for (int i = 0; i < dashes; ++i) {
            float c = (r01(rng) * 2.0f - 1.0f) * halfW * (0.3f + 0.7f * r01(rng));
            float len = halfW * (0.15f + 0.45f * r01(rng)) * (1.0f - std::fabs(c) / halfW * 0.6f);
            float k = 1.0f - std::fabs(c) / halfW;
            Tone mid = fade(t, 0.35f + 0.65f * k);
            m.line({c - len, y}, {c, y}, fade(mid, 0.0f), mid, 1.0f);
            m.line({c, y}, {c + len, y}, mid, fade(mid, 0.0f), 1.0f);
        }
        row.count = (GLsizei)m.verts.size() - row.first;
        rows.push_back(row);
    }
}

static void buildWaves(Mesh& a, Mesh& b)
{
    const Tone whitecap = {rgb(0.86f, 0.90f, 0.92f, 0.55f), rgb(0.45f, 0.52f, 0.72f, 0.40f)};
    const Tone sheen    = {rgb(1.00f, 0.78f, 0.72f, 0.45f), rgb(0.30f, 0.35f, 0.55f, 0.30f)};
    const Tone trough   = {rgb(0.08f, 0.26f, 0.32f, 0.55f), rgb(0.01f, 0.03f, 0.07f, 0.50f)};
    std::mt19937 rng(11);
    std::uniform_real_distribution<float> rx(0.0f, SCENE_W), ry(150.0f, HORIZON - 3.0f);
    for (int i = 0; i < 320; ++i) {
        float x = rx(rng), y = ry(rng);
        float d = (HORIZON - y) / (HORIZON - 150.0f);
        float len = 4.0f + 30.0f * d;
        Tone t = (i % 3 == 0) ? whitecap : (i % 3 == 1) ? trough : sheen;
        t = fade(t, 0.5f + 0.5f * d);
        Mesh& m = (i % 2) ? a : b;
        float w = d > 0.5f ? 1.5f : 1.0f;
        m.line({x - len, y}, {x, y + 1.0f}, fade(t, 0.0f), t, w);
        m.line({x, y + 1.0f}, {x + len, y}, t, fade(t, 0.0f), w);
    }
}

static void buildSwells(Mesh& m, std::vector<Row>& rows)
{
    const Tone trough = {rgb(0.08f, 0.27f, 0.31f), rgb(0.02f, 0.04f, 0.09f)};
    const Tone face   = {rgb(0.30f, 0.66f, 0.60f), rgb(0.07f, 0.15f, 0.22f)};
    const float specs[][6] = {{272, 2.0f, 6, 4, 0.023f, 0.0f},
                              {242, 3.0f, 11, 8, 0.017f, 1.3f},
                              {205, 5.0f, 22, 18, 0.012f, 2.1f},
                              {168, 6.0f, 26, 22, 0.010f, 0.7f}};
    std::mt19937 rng(5);
    std::uniform_real_distribution<float> r01(0.0f, 1.0f);
    for (int i = 0; i < 4; ++i) {
        const float y = specs[i][0], amp = specs[i][1], faceH = specs[i][2], foamH = specs[i][3];
        const float k = specs[i][4], ph = specs[i][5];
        auto base  = [&](float x) { return y + amp * std::sin(k * x + ph); };
        auto crest = [&](float x) { return smooth01(1.2f * std::sin(k * 0.6f * x + ph * 2.3f)); };

        Row row = {(GLint)m.verts.size(), 0, i / 3.0f};
        const float step = 8.0f;
        float jag0 = r01(rng);
        for (float x = -80.0f; x < SCENE_W + 80.0f; x += step) {
            float x1 = x + step, jag1 = r01(rng);
            float b0 = base(x), b1 = base(x1), c0 = crest(x), c1 = crest(x1);
            float t0 = b0 + faceH * (0.2f + 0.8f * c0), t1 = b1 + faceH * (0.2f + 0.8f * c1);
            m.quad({x, b0 - faceH * 0.5f}, {x1, b1 - faceH * 0.5f}, {x1, b1}, {x, b0},
                   fade(trough, 0), fade(trough, 0), fade(trough, 0.9f), fade(trough, 0.9f));
            m.quad({x, b0}, {x1, b1}, {x1, t1}, {x, t0}, fade(trough, 0.9f), fade(trough, 0.9f),
                   fade(face, 0.3f + 0.7f * c1), fade(face, 0.3f + 0.7f * c0));
            m.quad({x, t0 - faceH * 0.7f * c0}, {x1, t1 - faceH * 0.7f * c1}, {x1, t1}, {x, t0},
                   fade(pal::foam, 0), fade(pal::foam, 0), fade(pal::foam, 0.6f * c1), fade(pal::foam, 0.6f * c0));
            float f0 = foamH * c0 * (0.5f + 0.5f * jag0), f1 = foamH * c1 * (0.5f + 0.5f * jag1);
            m.quad({x, t0 - 2.0f}, {x1, t1 - 2.0f}, {x1, t1 + f1}, {x, t0 + f0},
                   fade(pal::foam, 0.95f * c0), fade(pal::foam, 0.95f * c1), fade(pal::foam, 0), fade(pal::foam, 0));
            jag0 = jag1;
        }
        row.count = (GLsizei)m.verts.size() - row.first;
        rows.push_back(row);
    }
}

static void buildWash(Mesh& patches, Mesh& lace)
{
    std::mt19937 rng(9);
    std::uniform_real_distribution<float> r01(0.0f, 1.0f);
    for (int i = 0; i < 28; ++i) {
        Vec2 c = {-40.0f + r01(rng) * (SCENE_W + 80.0f), 120.0f + r01(rng) * 55.0f};
        Tone t = fade(pal::foam, 0.3f + 0.3f * r01(rng));
        patches.softEllipse(c, 30.0f + 60.0f * r01(rng), 4.0f + 5.0f * r01(rng), t, fade(t, 0), 0.25f, 28);
    }
    for (int i = 0; i < 70; ++i) {
        Vec2 a = {r01(rng) * SCENE_W, 130.0f + r01(rng) * 140.0f};
        float d = (270.0f - a.y) / 140.0f;
        Tone t = fade(pal::foam, 0.3f + 0.35f * d);
        for (int s = 0; s < 4; ++s) {
            Vec2 b = {a.x + 6.0f + 12.0f * d, a.y + (r01(rng) - 0.5f) * 3.0f};
            lace.line(a, b, t, t, d > 0.5f ? 1.5f : 1.0f);
            a = b;
        }
    }
}

static void addRock(Mesh& m, Mesh& cracks, std::mt19937& rng, Vec2 c, float rx, float ry, float light)
{
    const Tone lit    = shade({rgb(0.80f, 0.72f, 0.64f), rgb(0.30f, 0.32f, 0.40f)}, light);
    const Tone shadow = shade({rgb(0.22f, 0.19f, 0.20f), rgb(0.05f, 0.05f, 0.08f)}, light);
    std::uniform_real_distribution<float> r01(0.0f, 1.0f);
    const float p1 = r01(rng) * 2 * PI, p2 = r01(rng) * 2 * PI, p3 = r01(rng) * 2 * PI;

    auto radius = [&](float a) {
        return 1.0f + 0.10f * std::sin(2 * a + p1) + 0.07f * std::sin(3 * a + p2) + 0.04f * std::sin(7 * a + p3);
    };
    auto toneAt = [&](float y) { return lerp(shadow, lit, smooth01((y - (c.y - ry)) / (2.0f * ry))); };
    const int seg = 32;
    for (int i = 0; i < seg; ++i) {
        float a0 = 2.0f * PI * i / seg, a1 = 2.0f * PI * (i + 1) / seg;
        Vec2 q0 = {c.x + rx * radius(a0) * std::cos(a0), c.y + ry * radius(a0) * std::sin(a0)};
        Vec2 q1 = {c.x + rx * radius(a1) * std::cos(a1), c.y + ry * radius(a1) * std::sin(a1)};
        m.triangle(c, q0, q1, toneAt(c.y + 0.3f * ry), toneAt(q0.y), toneAt(q1.y));
    }
    const Tone shine = shade({rgb(0.96f, 0.88f, 0.80f, 0.40f), rgb(0.45f, 0.48f, 0.58f, 0.25f)}, light);
    m.softEllipse({c.x - rx * 0.2f, c.y + ry * 0.45f}, rx * 0.5f, ry * 0.28f, shine, fade(shine, 0), 0.3f, 24);

    const Tone crack = {rgb(0.18f, 0.15f, 0.16f, 0.7f), rgb(0.02f, 0.02f, 0.04f, 0.7f)};
    for (int k = 0; k < 2; ++k) {
        Vec2 a = {c.x + (r01(rng) - 0.5f) * rx, c.y + ry * (0.1f + 0.5f * r01(rng))};
        for (int s = 0; s < 3; ++s) {
            Vec2 b = {a.x + (r01(rng) - 0.3f) * rx * 0.25f, a.y - ry * (0.12f + 0.15f * r01(rng))};
            cracks.line(a, b, crack, crack, 1.0f);
            a = b;
        }
    }
}

static void buildRocks(Mesh& m, Mesh& cracks)
{
    const Tone deep = {rgb(0.16f, 0.13f, 0.14f), rgb(0.03f, 0.03f, 0.05f)};
    m.quad({0, 0}, {SCENE_W, 0}, {SCENE_W, 85}, {0, 85}, deep, deep, deep, deep);

    const float rocks[][5] = {
        {40, 128, 110, 40, 0.75f},  {235, 118, 105, 36, 0.70f}, {430, 110, 95, 32, 0.75f},
        {620, 115, 110, 36, 0.70f}, {810, 128, 115, 42, 0.75f}, {975, 150, 85, 58, 0.80f},
        {70, 55, 150, 70, 1.05f},   {300, 48, 120, 60, 0.90f},  {480, 70, 95, 45, 0.85f},
        {650, 45, 130, 62, 0.95f},  {850, 72, 135, 66, 0.90f},  {990, 40, 95, 65, 1.00f},
        {160, -5, 125, 45, 1.15f},  {560, -5, 115, 40, 1.05f},  {880, -10, 135, 45, 1.10f}};
    std::mt19937 rng(17);
    for (const auto& r : rocks) addRock(m, cracks, rng, {r[0], r[1]}, r[2], r[3], r[4]);
}

static void buildBoat(Mesh& body, Mesh& rig, float s)
{
    const Tone hullTop = {rgb(0.40f, 0.26f, 0.28f), rgb(0.10f, 0.09f, 0.15f)};
    const Tone hullBot = {rgb(0.20f, 0.13f, 0.17f), rgb(0.04f, 0.04f, 0.08f)};
    body.quad({-30 * s, 0}, {28 * s, 0}, {44 * s, 12 * s}, {-44 * s, 12 * s}, hullBot, hullBot, hullTop, hullTop);

    const Tone sailTop  = {rgb(1.00f, 0.95f, 0.86f), rgb(0.70f, 0.73f, 0.84f)};
    const Tone sailBase = {rgb(0.96f, 0.70f, 0.66f), rgb(0.36f, 0.39f, 0.52f)};
    const Tone jibTop   = {rgb(0.98f, 0.84f, 0.80f), rgb(0.60f, 0.63f, 0.75f)};
    const Tone jibBase  = {rgb(0.82f, 0.56f, 0.60f), rgb(0.30f, 0.33f, 0.45f)};
    body.triangle({3 * s, 18 * s}, {3 * s, 88 * s}, {40 * s, 18 * s}, sailBase, sailTop, sailBase);
    body.triangle({-3 * s, 20 * s}, {-3 * s, 76 * s}, {-32 * s, 20 * s}, jibBase, jibTop, jibBase);

    const Tone mast = {rgb(0.30f, 0.20f, 0.22f), rgb(0.06f, 0.06f, 0.10f)};
    const Tone flag = {rgb(0.90f, 0.30f, 0.32f), rgb(0.25f, 0.20f, 0.30f)};
    rig.line({0, 12 * s}, {0, 94 * s}, mast, mast, 1.5f);
    rig.line({0, 94 * s}, {12 * s, 90 * s}, flag, flag, 1.5f);
    rig.line({12 * s, 90 * s}, {0, 87 * s}, flag, flag, 1.5f);
}

static void buildBirds(Mesh& up, Mesh& down)
{
    const float birds[][3] = {{0, 0, 1.0f}, {30, 12, 0.8f}, {-28, 16, 0.85f},
                              {54, -6, 0.7f}, {16, -20, 0.75f}};
    for (const auto& b : birds) {
        float x = b[0], y = b[1], s = b[2];
        auto add = [&](Mesh& m, float wingY, float tipY) {
            Vec2 L = {x - 13 * s, y + tipY * s}, LM = {x - 6 * s, y + wingY * s}, C = {x, y};
            Vec2 RM = {x + 6 * s, y + wingY * s}, R = {x + 13 * s, y + tipY * s};
            m.line(L, LM, pal::silhouette, pal::silhouette, 1.5f);
            m.line(LM, C, pal::silhouette, pal::silhouette, 1.5f);
            m.line(C, RM, pal::silhouette, pal::silhouette, 1.5f);
            m.line(RM, R, pal::silhouette, pal::silhouette, 1.5f);
        };
        add(up, 6.0f, 3.0f);
        add(down, 2.0f, -5.0f);
    }
}

struct State {
    bool  nightTarget = false;
    float night = 0.0f;
    float sunX = 500.0f, sunY = 348.0f;
    float brightness = 1.0f;
    bool  screenshotRequested = false;
} g;

struct Uniforms { GLint offset, night, brightness, alpha, pointSize, roundPoints; } U;

static void keyCallback(GLFWwindow* win, int key, int, int action, int)
{
    if (action != GLFW_PRESS) return;
    if (key == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(win, GLFW_TRUE);
    if (key == GLFW_KEY_SPACE) g.nightTarget = !g.nightTarget;
    if (key == GLFW_KEY_P) g.screenshotRequested = true;
}

static void update(GLFWwindow* win, float dt)
{
    const float speed = 200.0f;
    if (glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS)  g.sunX -= speed * dt;
    if (glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS) g.sunX += speed * dt;
    if (glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS)    g.sunY += speed * dt;
    if (glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS)  g.sunY -= speed * dt;
    g.sunX = std::clamp(g.sunX, 40.0f, SCENE_W - 40.0f);
    g.sunY = std::clamp(g.sunY, HORIZON - 2.0f * SUN_R, SCENE_H - 50.0f);

    if (glfwGetKey(win, GLFW_KEY_W) == GLFW_PRESS) g.brightness += 0.8f * dt;
    if (glfwGetKey(win, GLFW_KEY_S) == GLFW_PRESS) g.brightness -= 0.8f * dt;
    g.brightness = std::clamp(g.brightness, 0.3f, 1.8f);

    float target = g.nightTarget ? 1.0f : 0.0f;
    float step = dt / 2.0f;
    g.night += std::clamp(target - g.night, -step, step);
}

static void drawMesh(const Mesh& m, float ox = 0, float oy = 0, float alpha = 1.0f)
{
    glUniform2f(U.offset, ox, oy);
    glUniform1f(U.alpha, alpha);
    m.draw();
}

static float wrapX(float start, float speed, float t, float margin)
{
    return std::fmod(start + speed * t + margin, SCENE_W + 2 * margin) - margin;
}

static void render(Scene& s, float t, float pixelScale)
{
    const float night = smooth01(g.night);
    glUniform1f(U.night, night);
    glUniform1f(U.brightness, g.brightness);

    drawMesh(s.sky);
    if (night > 0.01f) {
        glUniform1i(U.roundPoints, 1);
        glUniform1f(U.pointSize, 2.5f * pixelScale);
        drawMesh(s.stars, 0, 0, night * (0.8f + 0.2f * std::sin(t * 1.7f)));
        glUniform1f(U.pointSize, 4.0f * pixelScale);
        drawMesh(s.bigStars, 0, 0, night * (0.75f + 0.25f * std::sin(t * 2.3f + 1.0f)));
        glUniform1i(U.roundPoints, 0);
    }

    const float glowA = smooth01((g.sunY - (HORIZON - 1.5f * SUN_R)) / (2.5f * SUN_R));
    const float reflA = smooth01((g.sunY - (HORIZON - SUN_R)) / (1.5f * SUN_R));
    drawMesh(s.sunGlow, g.sunX, g.sunY, glowA);
    drawMesh(s.sun, g.sunX, g.sunY);
    drawMesh(s.moonCraters, g.sunX, g.sunY);

    for (const Cloud& c : s.cloudsBack) drawMesh(c.mesh, wrapX(c.x, c.speed, t, c.margin), c.y);
    for (const Cloud& c : s.cloudsFront) drawMesh(c.mesh, wrapX(c.x, c.speed, t, c.margin), c.y);
    drawMesh(s.sunGlow, g.sunX, g.sunY, 0.25f * glowA);

    drawMesh(s.sea);

    if (reflA > 0.01f) {
        glUniform1f(U.alpha, reflA);
        for (size_t i = 0; i < s.pathRows.size(); ++i) {
            const Row& r = s.pathRows[i];
            float jitter = std::sin(t * 2.2f + i * 0.9f) * (1.5f + 5.0f * r.depth);
            glUniform2f(U.offset, g.sunX + jitter, 0.0f);
            s.sunPath.draw(r.first, r.count);
        }
    }

    drawMesh(s.wavesA, 6.0f * std::sin(t * 0.8f), 0);
    drawMesh(s.wavesB, -6.0f * std::sin(t * 0.8f), 0);

    float boatX = wrapX(640, 14.0f, t, 60), boatY = 286.0f + 1.5f * std::sin(t * 1.6f);
    drawMesh(s.boat, boatX, boatY);
    drawMesh(s.boatRig, boatX, boatY);

    glUniform1f(U.alpha, 1.0f);
    for (size_t i = 0; i < s.swellRows.size(); ++i) {
        const Row& r = s.swellRows[i];
        glUniform2f(U.offset, (6.0f + 4.0f * i) * std::sin(t * 0.35f + i * 1.7f),
                              (1.0f + 1.2f * i) * std::sin(t * 0.9f + i * 1.1f));
        s.swells.draw(r.first, r.count);
    }

    float washX = 6.0f * std::sin(t * 0.4f), washY = 3.0f * std::sin(t * 0.9f);
    drawMesh(s.foam, washX, washY);
    drawMesh(s.wash, washX, washY);
    drawMesh(s.rocks);
    drawMesh(s.rockCracks);

    const Mesh& birds = std::fmod(t * 2.5f, 1.0f) < 0.5f ? s.birdsUp : s.birdsDown;
    drawMesh(birds, wrapX(300, 30.0f, t, 120), 560 + 12.0f * std::sin(t * 0.7f));
}

static void saveScreenshot(const char* path)
{
    GLint vp[4];
    glGetIntegerv(GL_VIEWPORT, vp);
    const int w = vp[2], h = vp[3];
    std::vector<unsigned char> px((size_t)w * h * 3);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_BGR, GL_UNSIGNED_BYTE, px.data());

    unsigned char header[18] = {};
    header[2] = 2;
    header[12] = w & 0xFF; header[13] = (w >> 8) & 0xFF;
    header[14] = h & 0xFF; header[15] = (h >> 8) & 0xFF;
    header[16] = 24;
    if (FILE* f = std::fopen(path, "wb")) {
        std::fwrite(header, 1, sizeof(header), f);
        std::fwrite(px.data(), 1, px.size(), f);
        std::fclose(f);
        std::printf("Saved %s (%dx%d)\n", path, w, h);
    }
}

int main(int argc, char** argv)
{
    const bool screenshotMode = argc > 1 && std::strcmp(argv[1], "--screenshots") == 0;

    if (!glfwInit()) {
        std::fprintf(stderr, "Failed to initialise GLFW\n");
        return EXIT_FAILURE;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    glfwWindowHint(GLFW_SAMPLES, 4);

    GLFWwindow* win = glfwCreateWindow(WIN_W, WIN_H, "Sunset by the Sea - OpenGL Postcard",
                                       nullptr, nullptr);
    if (!win) {
        std::fprintf(stderr, "Failed to create an OpenGL 3.3 window\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(win);
    glfwSwapInterval(1);
    glfwSetKeyCallback(win, keyCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::fprintf(stderr, "Failed to load OpenGL functions with GLAD\n");
        return EXIT_FAILURE;
    }

    GLuint program = createProgram();
    glUseProgram(program);
    U.offset      = glGetUniformLocation(program, "uOffset");
    U.night       = glGetUniformLocation(program, "uNight");
    U.brightness  = glGetUniformLocation(program, "uBrightness");
    U.alpha       = glGetUniformLocation(program, "uAlpha");
    U.pointSize   = glGetUniformLocation(program, "uPointSize");
    U.roundPoints = glGetUniformLocation(program, "uRoundPoints");
    glUniform1f(U.pointSize, 1.0f);
    glUniform1i(U.roundPoints, 0);

    Scene scene;
    buildSky(scene.sky);
    buildStars(scene.stars, scene.bigStars);
    buildSun(scene.sunGlow, scene.sun, scene.moonCraters);
    buildClouds(scene.cloudsBack, scene.cloudsFront);
    buildSea(scene.sea);
    buildSunPath(scene.sunPath, scene.pathRows);
    buildWaves(scene.wavesA, scene.wavesB);
    buildSwells(scene.swells, scene.swellRows);
    buildWash(scene.wash, scene.foam);
    buildRocks(scene.rocks, scene.rockCracks);
    buildBoat(scene.boat, scene.boatRig, 0.6f);
    buildBirds(scene.birdsUp, scene.birdsDown);
    for (Mesh* m : scene.all()) m->upload();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glEnable(GL_MULTISAMPLE);

    auto beginFrame = [&]() -> float {
        int fbW, fbH;
        glfwGetFramebufferSize(win, &fbW, &fbH);
        glViewport(0, 0, fbW, fbH);
        glClearColor(0, 0, 0, 1);
        glClear(GL_COLOR_BUFFER_BIT);
        return (float)fbW / WIN_W;
    };

    if (screenshotMode) {
        const float t = 6.0f;
        for (int i = 0; i < 3; ++i) glfwPollEvents();
        g.night = 0.0f;
        render(scene, t, beginFrame());
        saveScreenshot("screenshot_sunset.tga");
        glfwSwapBuffers(win);
        g.night = 1.0f;
        g.sunX = 640.0f;
        g.sunY = 540.0f;
        render(scene, t, beginFrame());
        saveScreenshot("screenshot_night.tga");
        glfwSwapBuffers(win);
    } else {
        double last = glfwGetTime();
        while (!glfwWindowShouldClose(win)) {
            double now = glfwGetTime();
            float dt = (float)(now - last);
            last = now;

            glfwPollEvents();
            update(win, dt);
            render(scene, (float)now, beginFrame());
            if (g.screenshotRequested) {
                saveScreenshot("screenshot.tga");
                g.screenshotRequested = false;
            }
            glfwSwapBuffers(win);
        }
    }

    for (Mesh* m : scene.all()) m->release();
    glDeleteProgram(program);
    glfwDestroyWindow(win);
    glfwTerminate();
    return EXIT_SUCCESS;
}
