#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

#include "config.h"
#include "renderer/renderer.h"
#include "environment/lanes/lanes.h"
#include "environment/environment.h"
#include "ui/ui.h"
#include "game/game.h"
#include "ui/text_labels.h"
#include "ui/screens.h"
#include "cutscene/cutscene.h"

// Keyboard: GLFW calls keyCallback when a key changes; we remember each PRESS once (like mouse clicks).
static bool keyWasPressed[GLFW_KEY_LAST + 1] = { false };

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS && key >= 0 && key <= GLFW_KEY_LAST)
        keyWasPressed[key] = true;
}

// true once per key press, then forgotten
static bool takeKey(int key) {
    bool pressed = keyWasPressed[key];
    keyWasPressed[key] = false;
    return pressed;
}

int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);   // OpenGL 3.3 core: the minimum the PDF requires
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);       // fixed size keeps our pixel layout simple

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "The Last Anab Bottle Crisis", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to init GLAD\n";
        return -1;
    }

    // The framebuffer can be bigger than the window on high-DPI screens, so ask GLFW for its real size
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    if (!initRenderer()) return -1;
    if (!initTextLabels()) {   // UI text images (ui/assets/labels): the game cannot show any text without them
        glfwHideWindow(window);   // only the error box stays on screen, not an empty game window
        showTextImagesError();
        glfwTerminate();
        return -1;
    }

    glfwSwapInterval(1);                                      // V-Sync: ~60 frames per second
    glfwSetMouseButtonCallback(window, mouseButtonCallback);  // clicks go to lanes.cpp
    glfwSetKeyCallback(window, keyCallback);

    srand((unsigned)time(nullptr));   // once per launch: enemy lanes and order differ between games
    resetGame();   // also sets the starting coins (see config.h)
    App app;       // starts on the main menu (PDF 4.3)

    double lastTime = glfwGetTime();

    // --- Game loop: 1) measure time  2) update  3) draw ---
    while (!glfwWindowShouldClose(window)) {
        double currentTime = glfwGetTime();
        float deltaTime = (float)(currentTime - lastTime);
        lastTime = currentTime;
        if (deltaTime > 0.05f) deltaTime = 0.05f;   // e.g. after dragging the window: avoid a huge jump
        float time = (float)currentTime;            // drives small animations (bottle glow, laundry)

        // Update: mouse. GLFW measures from the TOP-left with y going DOWN; our game uses y going UP
        double cursorX, cursorY;
        glfwGetCursorPos(window, &cursorX, &cursorY);
        float mouseX = (float)cursorX;
        float mouseY = SCREEN_HEIGHT - (float)cursorY;
        bool gameOver = isGameWon() || isGameLost();
        bool clicked = takeLeftClick();
        // every key press is taken each frame, so a press on one screen never acts later on another
        bool escapeKey = takeKey(GLFW_KEY_ESCAPE);
        bool skipKey = takeKey(GLFW_KEY_ENTER) | takeKey(GLFW_KEY_SPACE);
        bool jumpKey = takeKey(GLFW_KEY_W) | takeKey(GLFW_KEY_UP);     // "|" so both keys are always consumed
        bool duckKey = takeKey(GLFW_KEY_S) | takeKey(GLFW_KEY_DOWN);
        int hoveredLane = -1, hoveredCard = -1;

        // Update: depends on the screen
        if (app.screen == SCREEN_CUTSCENE) {
            if (updateCutscene(deltaTime) || skipKey) finishCutscene(app);
        } else if (app.screen == SCREEN_PLAYING && !gameOver && escapeKey) {
            pressEscape(app, gameOver);   // pause first: a click or key in the same frame is dropped
        } else if (app.screen == SCREEN_PLAYING && !gameOver) {
            hoveredLane = getLaneAt(mouseX, mouseY);
            hoveredCard = getCardAt(mouseX, mouseY);
            if (clicked) {
                if (hoveredCard != -1) {
                    clickCard((PlayerType)hoveredCard);                         // select / cancel a card
                } else if (hoveredLane != -1) {
                    if (getSelectedCard() != -1)
                        tryPlacePlayer((PlayerType)getSelectedCard(), hoveredLane, mouseX);   // card first, then lane (Tayseer: at the click)
                    else
                        laneSelected[hoveredLane] = !laneSelected[hoveredLane];   // no card: toggle for jump/duck
                }
            }
            collectCoinsAt(mouseX, mouseY);   // mouse hover collects coins (player characters touching them do too, in game.cpp)

            // PDF: W/Up = jump, S/Down = duck, for players in the selected lanes
            if (jumpKey) jumpSelectedLanes();
            if (duckKey) duckSelectedLanes();

            updateGame(deltaTime);   // freezes by itself once won or lost
        } else {
            // main menu, pause menu, win/lose menu: only the buttons react
            if (escapeKey) pressEscape(app, gameOver);
            if (clicked) pressButton(app, buttonAt(app.screen, gameOver, mouseX, mouseY));
        }
        if (app.quit) glfwSetWindowShouldClose(window, GLFW_TRUE);

        // Draw: back to front
        beginFrame();
        if (app.screen == SCREEN_CUTSCENE) {
            drawCutscene();
        } else {
            drawEnvironment(time);          // sky, Egyptian street, 4 lanes
            drawLaboratory(time);           // home base on the far left
            if (app.screen != SCREEN_MAIN_MENU) {
                drawLaneHighlights(hoveredLane);
                drawGame(time);             // characters, projectiles, health bars, wave banners
                drawTopBar(getCoins(), getCurrentWave(), hoveredCard, time);   // UI on top of everything
                TextLine(TC_M).label("hint").drawCentered((BASE_WIDTH + SCREEN_WIDTH) / 2, 4, 0.15f, 0.15f, 0.15f, 1, false);
            }
            drawMenus(app.screen, isGameWon(), isGameLost(), mouseX, mouseY, time);   // main / pause / win-lose menu
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    shutdownRenderer();
    glfwTerminate();
    return 0;
}
