#include "ui/screens.h"
#include "config.h"
#include "renderer/renderer.h"
#include "players/players.h"
#include "enemies/enemies.h"
#include "ui/text_labels.h"
#include "cutscene/cutscene.h"
#include "game/game.h"
#include <cmath>

// ===================== Buttons =====================
struct Button { MenuButton id; const char* label; float centerX, bottomY; };
static const float BUTTON_W = 300, BUTTON_H = 58;
static const float MENU_CENTER_X = SCREEN_WIDTH / 2.0f;   // every menu is centered on the screen (x = 640)

static const int MAIN_BUTTON_COUNT = 2, PAUSE_BUTTON_COUNT = 3, END_BUTTON_COUNT = 2;
static const Button MAIN_BUTTONS[MAIN_BUTTON_COUNT] = {
    { BTN_START, "menu_start", MENU_CENTER_X, 300 },
    { BTN_EXIT, "menu_exit", MENU_CENTER_X, 222 } };
static const Button PAUSE_BUTTONS[PAUSE_BUTTON_COUNT] = {
    { BTN_CONTINUE, "menu_continue", MENU_CENTER_X, 360 },
    { BTN_RESTART, "menu_restart", MENU_CENTER_X, 285 },
    { BTN_MAIN_MENU, "menu_main", MENU_CENTER_X, 210 } };
static const Button END_BUTTONS[END_BUTTON_COUNT] = {     // win/lose menu
    { BTN_RESTART, "menu_restart", MENU_CENTER_X, 262 },
    { BTN_MAIN_MENU, "menu_main", MENU_CENTER_X, 187 } };

static bool inside(const Button& b, float x, float y) {
    return x >= b.centerX - BUTTON_W / 2 && x <= b.centerX + BUTTON_W / 2
        && y >= b.bottomY && y <= b.bottomY + BUTTON_H;
}

// The button of this menu under (x, y), or BTN_NONE
static MenuButton findButton(const Button buttons[], int count, float x, float y) {
    for (int i = 0; i < count; i++)
        if (inside(buttons[i], x, y)) return buttons[i].id;
    return BTN_NONE;
}

MenuButton buttonAt(Screen screen, bool gameOver, float x, float y) {
    if (screen == SCREEN_MAIN_MENU) return findButton(MAIN_BUTTONS, MAIN_BUTTON_COUNT, x, y);
    if (screen == SCREEN_PAUSED) return findButton(PAUSE_BUTTONS, PAUSE_BUTTON_COUNT, x, y);
    if (screen == SCREEN_PLAYING && gameOver) return findButton(END_BUTTONS, END_BUTTON_COUNT, x, y);
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
    drawRectangle(left, b.bottomY, BUTTON_W, BUTTON_H,
                  hover ? 0.50f : 0.32f, hover ? 0.18f : 0.12f, hover ? 0.30f : 0.20f);
    drawRectangleOutline(left, b.bottomY, BUTTON_W, BUTTON_H, hover ? 4.0f : 2.0f, 0.95f, 0.75f, 0.25f);
    TextLine(TC_BTN).label(b.label).drawCentered(b.centerX, b.bottomY + 8,
                                                 1.0f, hover ? 1.0f : 0.92f, hover ? 0.85f : 0.70f);
}

static void drawPanel(float centerX, float bottomY, float width, float height) {
    drawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, 0.55f);                                 // dim the game
    drawRectangle(centerX - width / 2, bottomY, width, height, 0.14f, 0.05f, 0.10f, 0.95f);
    drawRectangleOutline(centerX - width / 2, bottomY, width, height, 3, 0.95f, 0.75f, 0.25f);
}

static void drawMainMenu(float mouseX, float mouseY, float time) {
    drawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0.05f, 0.02f, 0.06f, 0.55f);
    // the hero and one villain stand on either side of the menu, idling (the villain mirrored to face him)
    pushMatrix();
    translate(330, 150);
    scale(2.2f, 2.2f);
    drawAnabMan(time, false, -1);
    popMatrix();

    pushMatrix();
    translate(950, 150);
    scale(-2.2f, 2.2f);
    drawThalma(time, false, -1);
    popMatrix();

    // the last anab bottle, glowing
    float pulse = 0.5f + 0.5f * sin(time * 3.0f);
    drawCircle(MENU_CENTER_X, 455, 30 + 6 * pulse, 0.85f, 0.15f, 0.45f, 0.25f);                      // glow
    drawRectangle(MENU_CENTER_X - 7, 470, 14, 18, 0.80f, 0.90f, 0.95f);                               // neck
    drawCircle(MENU_CENTER_X, 452, 20, 0.80f, 0.90f, 0.95f);                                          // glass
    drawCircle(MENU_CENTER_X, 449, 15, 0.55f, 0.05f, 0.25f);                                          // anab inside
    drawRectangle(MENU_CENTER_X - 6, 486, 12, 6, 0.55f, 0.35f, 0.18f);                                // cork
    TextLine(TC_XXL).label("title_game").drawCentered(MENU_CENTER_X, 515, 1.0f, 0.85f, 0.30f);
    TextLine(TC_M).label("team").drawCentered(MENU_CENTER_X, 615, 0.95f, 0.85f, 0.60f);
    for (const Button& b : MAIN_BUTTONS) drawButton(b, mouseX, mouseY);
}

void drawMenus(Screen screen, bool won, bool lost, float mouseX, float mouseY, float time) {
    if (screen == SCREEN_MAIN_MENU) {
        drawMainMenu(mouseX, mouseY, time);
    } else if (screen == SCREEN_PAUSED) {
        drawPanel(MENU_CENTER_X, 185, 480, 410);
        TextLine(TC_XXL).label("pause_title").drawCentered(MENU_CENTER_X, 500, 1.0f, 0.85f, 0.30f);
        for (const Button& b : PAUSE_BUTTONS) drawButton(b, mouseX, mouseY);
    } else if (screen == SCREEN_PLAYING && (won || lost)) {
        drawPanel(MENU_CENTER_X, 165, 760, 260);                 // wide enough for "You win! The formula is safe"
        if (won) TextLine(TC_XXL).label("ban_win").drawCentered(MENU_CENTER_X, 335, 0.4f, 1.0f, 0.5f);
        else     TextLine(TC_XXL).label("ban_lose").drawCentered(MENU_CENTER_X, 335, 1.0f, 0.3f, 0.3f);
        for (const Button& b : END_BUTTONS) drawButton(b, mouseX, mouseY);
    }
}
