#pragma once
// Which screen the game is on, the three menus (PDF 4.3) and the moves between screens:
// main menu -> cutscene -> playing <-> paused (Esc); playing -> win/lose menu when the game ends.

enum Screen { SCREEN_MAIN_MENU, SCREEN_CUTSCENE, SCREEN_PLAYING, SCREEN_PAUSED };
enum MenuButton { BTN_NONE, BTN_START, BTN_EXIT, BTN_CONTINUE, BTN_RESTART, BTN_MAIN_MENU };

struct App {
    Screen screen = SCREEN_MAIN_MENU;
    bool quit = false;                // Exit was chosen: main.cpp closes the window
};

// The button under (x, y) on the current screen. gameOver = the win/lose menu is showing (while playing).
MenuButton buttonAt(Screen screen, bool gameOver, float x, float y);
void pressButton(App& app, MenuButton button);
void pressEscape(App& app, bool gameOver);   // playing -> paused, paused -> playing
void finishCutscene(App& app);               // the cutscene ended (or was skipped): a fresh game starts

// Draws the menu that belongs to the screen (nothing while playing a running game)
void drawMenus(Screen screen, bool won, bool lost, float mouseX, float mouseY, float time);
