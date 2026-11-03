#include "screens.h"
#include "config.h"
#include "renderer.h"
#include "characters.h"
#include "text_labels.h"
#include "cutscene.h"
#include "game.h"
#include <cmath>

// ===================== Buttons =====================
struct Button { MenuButton id; const char* label; float centerX, bottomY; };
static const float BUTTON_W = 300, BUTTON_H = 58;

static const Button MAIN_BUTTONS[] = { { BTN_START, "menu_start", 640, 300 }, { BTN_EXIT, "menu_exit", 640, 222 } };
static const Button PAUSE_BUTTONS[] = { { BTN_CONTINUE, "menu_continue", 640, 360 }, { BTN_RESTART, "menu_restart", 640, 285 },
                                        { BTN_MAIN_MENU, "menu_main", 640, 210 } };
static const Button END_BUTTONS[] = { { BTN_RESTART, "menu_restart", 640, 262 }, { BTN_MAIN_MENU, "menu_main", 640, 187 } };

static bool inside(const Button& b, float x, float y) {
    return x >= b.centerX - BUTTON_W / 2 && x <= b.centerX + BUTTON_W / 2 && y >= b.bottomY && y <= b.bottomY + BUTTON_H;
}

template <int N> static MenuButton findButton(const Button (&buttons)[N], float x, float y) {
    for (const Button& b : buttons) if (inside(b, x, y)) return b.id;
    return BTN_NONE;
}

MenuButton buttonAt(Screen screen, bool gameOver, float x, float y) {
    if (screen == SCREEN_MAIN_MENU) return findButton(MAIN_BUTTONS, x, y);
    if (screen == SCREEN_PAUSED) return findButton(PAUSE_BUTTONS, x, y);
    if (screen == SCREEN_PLAYING && gameOver) return findButton(END_BUTTONS, x, y);
    return BTN_NONE;
}

// ===================== Moves between screens =====================
void pressButton(App& app, MenuButton button) {
    switch (button) {
    case BTN_START:     app.screen = SCREEN_CUTSCENE; startCutscene(); break;      // PDF: Start = cutscene, then game
    case BTN_EXIT:      app.quit = true; break;
    case BTN_CONTINUE:  app.screen = SCREEN_PLAYING; break;
    case BTN_RESTART:   resetGame(); app.screen = SCREEN_PLAYING; break;
    case BTN_MAIN_MENU: resetGame(); app.screen = SCREEN_MAIN_MENU; break;
    case BTN_NONE:      break;
    }
}

void pressEscape(App& app, bool gameOver) {
    if (app.screen == SCREEN_PLAYING && !gameOver) app.screen = SCREEN_PAUSED;
    else if (app.screen == SCREEN_PAUSED) app.screen = SCREEN_PLAYING;
}

void finishCutscene(App& app) {
    resetGame();
    app.screen = SCREEN_PLAYING;
}

// ===================== Drawing =====================
static void drawButton(const Button& b, float mouseX, float mouseY) {
    bool hover = inside(b, mouseX, mouseY);
    float left = b.centerX - BUTTON_W / 2;
    drawRectangle(left + 4, b.bottomY - 4, BUTTON_W, BUTTON_H, 0, 0, 0, 0.45f);                        // shadow
    drawRectangle(left, b.bottomY, BUTTON_W, BUTTON_H, hover ? 0.50f : 0.32f, hover ? 0.18f : 0.12f, hover ? 0.30f : 0.20f);
    drawRectangleOutline(left, b.bottomY, BUTTON_W, BUTTON_H, hover ? 4.0f : 2.0f, 0.95f, 0.75f, 0.25f);
    TextLine(TC_BTN).label(b.label).drawCentered(b.centerX, b.bottomY + 8, 1.0f, hover ? 1.0f : 0.92f, hover ? 0.85f : 0.70f);
}

static void drawPanel(float centerX, float bottomY, float width, float height) {
    drawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0.55f);                                 // dim the game
    drawRectangle(centerX - width / 2, bottomY, width, height, 0.14f, 0.05f, 0.10f, 0.95f);
    drawRectangleOutline(centerX - width / 2, bottomY, width, height, 3, 0.95f, 0.75f, 0.25f);
}

static void drawMainMenu(float mouseX, float mouseY, float time) {
    drawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.05f, 0.02f, 0.06f, 0.55f);
    // the hero and one villain stand on either side of the menu, idling
    pushMatrix(); translate(330, 150); scale(2.2f, 2.2f); drawAnabMan(time, false, -1); popMatrix();
    pushMatrix(); translate(950, 150); scale(-2.2f, 2.2f); drawThalma(time, false, -1); popMatrix();
    // the last anab bottle, glowing
    float pulse = 0.5f + 0.5f * sin(time * 3.0f);
    drawCircle(640, 455, 30 + 6 * pulse, 0.85f, 0.15f, 0.45f, 0.25f);
    drawRectangle(633, 470, 14, 18, 0.80f, 0.90f, 0.95f);                                             // neck
    drawCircle(640, 452, 20, 0.80f, 0.90f, 0.95f);                                                    // glass
    drawCircle(640, 449, 15, 0.55f, 0.05f, 0.25f);                                                    // anab inside
    drawRectangle(634, 486, 12, 6, 0.55f, 0.35f, 0.18f);                                              // cork
    TextLine(TC_XXL).label("title_game").drawCentered(640, 515, 1.0f, 0.85f, 0.30f);
    TextLine(TC_M).label("team").drawCentered(640, 615, 0.95f, 0.85f, 0.60f);
    for (const Button& b : MAIN_BUTTONS) drawButton(b, mouseX, mouseY);
}

void drawMenus(Screen screen, bool won, bool lost, float mouseX, float mouseY, float time) {
    if (screen == SCREEN_MAIN_MENU) {
        drawMainMenu(mouseX, mouseY, time);
    } else if (screen == SCREEN_PAUSED) {
        drawPanel(640, 185, 480, 410);
        TextLine(TC_XXL).label("pause_title").drawCentered(640, 500, 1.0f, 0.85f, 0.30f);
        for (const Button& b : PAUSE_BUTTONS) drawButton(b, mouseX, mouseY);
    } else if (screen == SCREEN_PLAYING && (won || lost)) {
        drawPanel(640, 165, 760, 260);                                  // wide enough for "You win! The formula is safe"
        if (won) TextLine(TC_XXL).label("ban_win").drawCentered(640, 335, 0.4f, 1.0f, 0.5f);
        else     TextLine(TC_XXL).label("ban_lose").drawCentered(640, 335, 1.0f, 0.3f, 0.3f);
        for (const Button& b : END_BUTTONS) drawButton(b, mouseX, mouseY);
    }
}
