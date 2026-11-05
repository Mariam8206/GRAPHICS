#include "renderer/renderer.h"
#include "config.h"
#include <glad/glad.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>                    // matrices, as in Lab 3 and Lab 4
#include <glm/gtc/matrix_transform.hpp>   // glm::translate, glm::rotate, glm::scale, glm::ortho
#include <glm/gtc/type_ptr.hpp>           // glm::value_ptr: a matrix as 16 floats for glUniformMatrix4fv
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"        // public-domain PNG loader, used for the UI text images

// ===================== Shaders =====================
static const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;   // vertex position in the model's OWN pixels (local space)
layout (location = 1) in vec2 aUV;    // texture coordinate (only used by images)
uniform mat4 uModel;                  // moves/rotates/scales the model into the screen (pixels)
uniform mat4 uProjection;             // screen pixels (0..1280, 0..720) -> NDC (-1..1), made with glm::ortho
out vec2 vUV;

void main()
{
    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);
    vUV = aUV;
}
)";

// Damage tint: mixing toward red (not multiplying) also reddens black and dark parts; alpha is not touched, so the
// shape stays the same. Grayscale is applied after the tint, so anything drawn with uGrayscale = 1 ends up fully gray.
static const char* fragmentShaderSource = R"(
#version 330 core
in vec2 vUV;
out vec4 FragColor;
uniform vec4 uColor;                  // color of the shape being drawn, sent from C++
uniform float uDamageTint;            // 0 = normal colors, 1 = fully red (damage feedback)
uniform float uGrayscale;             // 0 = normal colors, 1 = fully gray (locked cards)
uniform int uUseTexture;              // 1 = multiply by the image (white UI text gets uColor)
uniform sampler2D uTexture;

void main()
{
    vec4 image = (uUseTexture == 1) ? texture(uTexture, vUV) : vec4(1.0);
    vec3 damageRed = vec3(0.9, 0.05, 0.05);
    // mix(a, b, t) = a * (1 - t) + b * t : slides the color toward red as t grows
    vec3 color = mix(uColor.rgb * image.rgb, damageRed, uDamageTint);
    float gray = dot(color, vec3(0.299, 0.587, 0.114));   // brightness as the eye sees it
    FragColor = vec4(mix(color, vec3(gray), uGrayscale), uColor.a * image.a);
}
)";

// --- GPU objects, created once in initRenderer(). The *Location values are the shader uniform locations. ---
static GLuint shaderProgram;
static GLuint VAO, VBO;
static GLint colorLocation;
static GLint modelLocation;
static GLint projectionLocation;
static GLint damageTintLocation;
static GLint grayscaleLocation;
static GLint useTextureLocation;
static GLuint imageVAO, imageVBO; // images: 4 floats per corner (x, y, u, v)

static GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compile error:\n" << infoLog << std::endl;
    }
    return shader;
}

// ===================== 2D transformations (glm, as in Lab 3 and Lab 4) =====================
// Our world is flat, so z stays 0: translations use (tx, ty, 0), rotations turn around the z axis,
// scales keep z at 1. glm::mat4(1.0f) is the identity ("do nothing") matrix.

// The transformation applied to everything we draw right now, plus saved copies of it.
static glm::mat4 currentModel = glm::mat4(1.0f);
static std::vector<glm::mat4> matrixStack;

void pushMatrix() { matrixStack.push_back(currentModel); }
void popMatrix()  { currentModel = matrixStack.back(); matrixStack.pop_back(); }

// glm multiplies each new matrix on the RIGHT (currentModel * T), so the call written LAST acts on the shape FIRST.
void translate(float tx, float ty) { currentModel = glm::translate(currentModel, glm::vec3(tx, ty, 0.0f)); }
void rotate(float angleDegrees) {
    currentModel = glm::rotate(currentModel, glm::radians(angleDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
}
// negative = flip
void scale(float sx, float sy)     { currentModel = glm::scale(currentModel, glm::vec3(sx, sy, 1.0f)); }

// ===================== Setup / per frame =====================
bool initRenderer() {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    int success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader link error:\n" << infoLog << std::endl;
        return false;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Uniform locations: "addresses" of the uniforms inside the linked program
    projectionLocation = glGetUniformLocation(shaderProgram, "uProjection");
    colorLocation = glGetUniformLocation(shaderProgram, "uColor");
    modelLocation = glGetUniformLocation(shaderProgram, "uModel");
    damageTintLocation = glGetUniformLocation(shaderProgram, "uDamageTint");
    grayscaleLocation = glGetUniformLocation(shaderProgram, "uGrayscale");
    useTextureLocation = glGetUniformLocation(shaderProgram, "uUseTexture");
    glUseProgram(shaderProgram);
    glUniform1i(glGetUniformLocation(shaderProgram, "uTexture"), 0);   // images always use texture unit 0

    // One VAO + VBO, reused for every shape (data is uploaded in drawVertices)
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // attribute 0 = 2 floats (x, y) per vertex, tightly packed
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // A second VAO + VBO for images: attribute 0 = position, attribute 1 = texture coordinate
    glGenVertexArrays(1, &imageVAO);
    glGenBuffers(1, &imageVBO);
    glBindVertexArray(imageVAO);
    glBindBuffer(GL_ARRAY_BUFFER, imageVBO);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Blending: a color with a < 1 mixes with what is already on screen.
    // final = newColor * a + oldColor * (1 - a)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return true;
}

void beginFrame() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(shaderProgram);
    // Orthographic projection (Lab 4): x 0..1280 and y 0..720 (y up, origin bottom-left) fill the window
    glm::mat4 projection = glm::ortho(0.0f, (float)SCREEN_WIDTH, 0.0f, (float)SCREEN_HEIGHT);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));
    currentModel = glm::mat4(1.0f);    // start every frame with "no transformation"
    matrixStack.clear();
    setDamageTint(0);                  // and with normal colors (a missed reset cannot leak into the next frame)
    setGrayscale(0);
}

void shutdownRenderer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &imageVAO);
    glDeleteBuffers(1, &imageVBO);
    glDeleteProgram(shaderProgram);
}

// ===================== Images (PNG textures) =====================
bool loadTexture(const char* path, Texture& texture) {
    int width, height, channels;
    stbi_set_flip_vertically_on_load(1);               // PNG rows go top-down; our y goes up
    unsigned char* pixels = stbi_load(path, &width, &height, &channels, 4);   // always RGBA
    if (!pixels) return false;
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    stbi_image_free(pixels);
    texture.id = id;
    texture.width = width;
    texture.height = height;
    return true;
}

// (x, y) is the BOTTOM-LEFT corner. The image is multiplied by the color (white text takes the color).
void drawImage(const Texture& texture, float x, float y, float width, float height,
               float r, float g, float b, float a) {
    if (texture.id == 0) return;
    float vertices[] = {
        x,         y,          0, 0,
        x + width, y,          1, 0,
        x + width, y + height, 1, 1,
        x,         y,          0, 0,
        x + width, y + height, 1, 1,
        x,         y + height, 0, 1
    };
    glUniform4f(colorLocation, r, g, b, a);
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(currentModel));
    glUniform1i(useTextureLocation, 1);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glBindVertexArray(imageVAO);
    glBindBuffer(GL_ARRAY_BUFFER, imageVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glUniform1i(useTextureLocation, 0);                // back to plain colored shapes
}

// ===================== Shapes =====================
// The one function that talks to OpenGL. Every shape builds its corners and calls this.
// vertices    : x, y pairs in pixels
// vertexCount : number of corners (NOT number of floats)
// mode        : how OpenGL connects the corners (GL_TRIANGLES or GL_TRIANGLE_FAN)
static void drawVertices(const float* vertices, int vertexCount, GLenum mode, float r, float g, float b, float a) {
    if (vertexCount <= 0) return;
    glUniform4f(colorLocation, r, g, b, a);
    // GL_FALSE = no transpose: glm already stores the matrix column by column, as OpenGL expects
    glUniformMatrix4fv(modelLocation, 1, GL_FALSE, glm::value_ptr(currentModel));
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // size in bytes = corners * 2 floats per corner * bytes per float
    glBufferData(GL_ARRAY_BUFFER, vertexCount * 2 * sizeof(float), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(mode, 0, vertexCount);
}

// (x, y) is the BOTTOM-LEFT corner. A rectangle = 2 triangles = 6 corners.
void drawRectangle(float x, float y, float width, float height, float r, float g, float b, float a) {
    float vertices[] = {
        x,         y,              // triangle 1: bottom-left
        x + width, y,              //             bottom-right
        x + width, y + height,     //             top-right

        x,         y,              // triangle 2: bottom-left
        x + width, y + height,     //             top-right
        x,         y + height      //             top-left
    };
    drawVertices(vertices, 6, GL_TRIANGLES, r, g, b, a);
}

// Only the border: 4 thin rectangles (bottom, top, left, right) that do not overlap at the corners.
void drawRectangleOutline(float x, float y, float width, float height, float thickness,
                          float r, float g, float b, float a) {
    drawRectangle(x, y, width, thickness, r, g, b, a);                                                  // bottom
    drawRectangle(x, y + height - thickness, width, thickness, r, g, b, a);                             // top
    drawRectangle(x, y + thickness, thickness, height - 2 * thickness, r, g, b, a);                     // left
    drawRectangle(x + width - thickness, y + thickness, thickness, height - 2 * thickness, r, g, b, a); // right
}

void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3, float r, float g, float b, float a) {
    float vertices[] = { x1, y1,  x2, y2,  x3, y3 };
    drawVertices(vertices, 3, GL_TRIANGLES, r, g, b, a);
}

// Any 4-cornered shape (e.g. a trapezoid). Corners must be given in order around the shape.
void drawQuad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
              float r, float g, float b, float a) {
    float vertices[] = { x1, y1,  x2, y2,  x3, y3,     x1, y1,  x3, y3,  x4, y4 };
    drawVertices(vertices, 6, GL_TRIANGLES, r, g, b, a);
}

// A thick straight line = a thin rectangle turned to point from (x1, y1) to (x2, y2).
void drawLine(float x1, float y1, float x2, float y2, float thickness, float r, float g, float b, float a) {
    float dx = x2 - x1, dy = y2 - y1;
    float length = sqrt(dx * dx + dy * dy);
    if (length < 0.0001f) return;
    // (nx, ny) = a vector at 90 degrees to the line, half the thickness long
    float nx = -dy / length * thickness / 2;
    float ny = dx / length * thickness / 2;
    drawQuad(x1 + nx, y1 + ny, x2 + nx, y2 + ny, x2 - nx, y2 - ny, x1 - nx, y1 - ny, r, g, b, a);
}

// Triangle fan: one center corner + corners around the edge.
void drawEllipse(float centerX, float centerY, float radiusX, float radiusY, float r, float g, float b, float a) {
    const int SEGMENTS = 40;
    float vertices[(SEGMENTS + 2) * 2];   // center + (segments + 1) edge corners, 2 floats each
    vertices[0] = centerX;
    vertices[1] = centerY;
    for (int i = 0; i <= SEGMENTS; i++) { // "<=" repeats the first edge corner to close the shape
        float angle = 2.0f * PI * i / SEGMENTS;
        vertices[2 + i * 2]     = centerX + radiusX * cos(angle);
        vertices[2 + i * 2 + 1] = centerY + radiusY * sin(angle);
    }
    drawVertices(vertices, SEGMENTS + 2, GL_TRIANGLE_FAN, r, g, b, a);
}

void drawCircle(float centerX, float centerY, float radius, float r, float g, float b, float a) {
    drawEllipse(centerX, centerY, radius, radius, r, g, b, a);
}

// Part of a circle's outline, from startDegrees to endDegrees (counter-clockwise, 0 = pointing right),
// made of short thick lines. Used for glasses frames, smiles, the Arabic letter emblem, etc.
void drawArc(float centerX, float centerY, float radius, float startDegrees, float endDegrees, float thickness,
             float r, float g, float b, float a) {
    const int STEPS = 16;
    for (int i = 0; i < STEPS; i++) {
        float a1 = (startDegrees + (endDegrees - startDegrees) * i / STEPS) * PI / 180.0f;
        float a2 = (startDegrees + (endDegrees - startDegrees) * (i + 1) / STEPS) * PI / 180.0f;
        drawLine(centerX + radius * cos(a1), centerY + radius * sin(a1),
                 centerX + radius * cos(a2), centerY + radius * sin(a2), thickness, r, g, b, a);
    }
}

// Clipping: nothing is drawn outside this screen rectangle until clearClipRect(). Uses the viewport size,
// so it also works when the framebuffer is bigger than 1280x720 (high-DPI screens).
void setClipRect(float x, float y, float width, float height) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    float sx = viewport[2] / (float)SCREEN_WIDTH, sy = viewport[3] / (float)SCREEN_HEIGHT;
    glEnable(GL_SCISSOR_TEST);
    glScissor(viewport[0] + (GLint)(x * sx), viewport[1] + (GLint)(y * sy),
              (GLsizei)ceil(width * sx), (GLsizei)ceil(height * sy));
}
void clearClipRect() { glDisable(GL_SCISSOR_TEST); }

// Damage feedback: everything drawn after this call is pushed toward red by "amount" (0..1).
// A shader switch instead of a color argument: one call tints a whole character made of many shapes, and the
// model drawing functions stay unchanged. Callers set it back to 0 when the character is done.
void setDamageTint(float amount) {
    if (amount < 0) amount = 0;
    if (amount > 1) amount = 1;
    glUniform1f(damageTintLocation, amount);
}

// Locked cards: everything drawn after this call loses its color by "amount" (0..1).
// Same idea as setDamageTint: one call grays the whole card (frame, portrait, text images) at once.
void setGrayscale(float amount) {
    if (amount < 0) amount = 0;
    if (amount > 1) amount = 1;
    glUniform1f(grayscaleLocation, amount);
}
