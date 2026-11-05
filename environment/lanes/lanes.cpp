#include "environment/lanes/lanes.h"
#include "renderer/renderer.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

bool laneSelected[LANE_COUNT] = { false };
int placementLane = -1;
float placementX = -1;
static bool leftClickHappened = false;   // set by the callback, consumed by takeLeftClick()

void clickLane(int lane, float x) {
    if (lane == placementLane) {         // clicking the placement lane again deselects it
        laneSelected[lane] = false;
        placementLane = -1;
    } else {                             // any other lane: one click selects it (if it was not yet) and makes it
        laneSelected[lane] = true;       // the placement lane; the other selected lanes stay selected for jump/duck
        placementLane = lane;
        placementX = x;
    }
}

void clearLaneSelection() {
    for (int lane = 0; lane < LANE_COUNT; lane++) laneSelected[lane] = false;
    placementLane = -1;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
        leftClickHappened = true;
}

bool takeLeftClick() {
    bool clicked = leftClickHappened;
    leftClickHappened = false;   // forget it, so the next frame does not handle it again
    return clicked;
}

int getLaneAt(float x, float y) {
    if (x < BASE_WIDTH || x > SCREEN_WIDTH) return -1;     // over the home base or outside
    if (y < FIELD_BOTTOM || y >= FIELD_TOP) return -1;     // below or above the lanes
    return (int)((y - FIELD_BOTTOM) / LANE_HEIGHT);        // 0 = bottom lane
}

void drawLaneHighlights(int hoveredLane) {
    float laneWidth = SCREEN_WIDTH - BASE_WIDTH;
    for (int lane = 0; lane < LANE_COUNT; lane++) {
        float laneY = FIELD_BOTTOM + lane * LANE_HEIGHT;
        if (laneSelected[lane]) {
            drawRectangle(BASE_WIDTH, laneY, laneWidth, LANE_HEIGHT, 1.0f, 0.85f, 0.2f, 0.25f);
            drawRectangleOutline(BASE_WIDTH, laneY, laneWidth, LANE_HEIGHT, 4, 1.0f, 0.75f, 0.1f, 0.9f);
        }
        if (lane == placementLane)       // the next character goes here: white border inside the gold one
            drawRectangleOutline(BASE_WIDTH + 4, laneY + 4, laneWidth - 8, LANE_HEIGHT - 8, 3, 1.0f, 1.0f, 1.0f, 0.9f);
        if (lane == hoveredLane)
            drawRectangle(BASE_WIDTH, laneY, laneWidth, LANE_HEIGHT, 1.0f, 1.0f, 1.0f, 0.2f);
    }
}
