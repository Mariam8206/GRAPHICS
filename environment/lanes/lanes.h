#pragma once
// Lane hover / selection with the mouse (PDF 3.2: hover highlights the whole lane,
// left click toggles selection, several lanes can be selected at once).

#include "config.h"

struct GLFWwindow;   // forward declaration: we only need the name, not the whole GLFW header

extern bool laneSelected[LANE_COUNT];   // true = lane is locked-highlighted

// GLFW calls this when a mouse button changes; it only REMEMBERS the click.
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

// Returns true once per left click (and forgets it), so each click is handled exactly once.
bool takeLeftClick();

// Which lane contains the point (x, y) in game pixels, or -1 if the point is not on a lane.
int getLaneAt(float x, float y);

// Selected lanes: gold tint + gold border. Hovered lane: light white tint. A lane can be both.
void drawLaneHighlights(int hoveredLane);
