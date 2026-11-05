#pragma once
// Everything that draws: shader setup, basic shapes, text, and the 2D transformation stack.
// All coordinates are in pixels. Colors are r, g, b in 0..1 and an optional opacity a.

bool initRenderer();        // compiles the shader and creates the VAO/VBO. Call once after GLAD is loaded.
void beginFrame();          // clears the screen and resets the transformation. Call at the start of every frame.
void shutdownRenderer();    // frees the GPU objects

// --- Shapes ---
void drawRectangle(float x, float y, float width, float height, float r, float g, float b, float a = 1.0f);
void drawRectangleOutline(float x, float y, float width, float height, float thickness,
                          float r, float g, float b, float a = 1.0f);
void drawTriangle(float x1, float y1, float x2, float y2, float x3, float y3,
                  float r, float g, float b, float a = 1.0f);
void drawQuad(float x1, float y1, float x2, float y2, float x3, float y3, float x4, float y4,
              float r, float g, float b, float a = 1.0f);
void drawLine(float x1, float y1, float x2, float y2, float thickness, float r, float g, float b, float a = 1.0f);
void drawEllipse(float centerX, float centerY, float radiusX, float radiusY, float r, float g, float b, float a = 1.0f);
void drawCircle(float centerX, float centerY, float radius, float r, float g, float b, float a = 1.0f);
void drawArc(float centerX, float centerY, float radius, float startDegrees, float endDegrees, float thickness,
             float r, float g, float b, float a = 1.0f);

// Clipping rectangle in screen pixels (bottom-left x, y). clearClipRect() turns it off.
void setClipRect(float x, float y, float width, float height);
void clearClipRect();

// Damage feedback: 0 = normal colors, 1 = fully red. Stays active until changed (reset each frame).
void setDamageTint(float amount);
// Locked cards: 0 = normal colors, 1 = fully gray. Stays active until changed (reset each frame).
void setGrayscale(float amount);

// --- Images (PNG, loaded with stb_image) ---
struct Texture { unsigned int id = 0; int width = 0, height = 0; };
bool loadTexture(const char* path, Texture& texture);   // false if the file is missing or not a PNG
// (x, y) is the BOTTOM-LEFT corner. The image is multiplied by the color, so white text takes that color.
void drawImage(const Texture& texture, float x, float y, float width, float height,
               float r = 1, float g = 1, float b = 1, float a = 1);

// --- 2D transformations (each call acts on everything drawn after it, until popMatrix) ---
void pushMatrix();
void popMatrix();
void translate(float tx, float ty);
void rotate(float angleDegrees);
void scale(float sx, float sy);
