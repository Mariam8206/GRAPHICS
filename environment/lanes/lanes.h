#pragma once
// Lane hover / selection with the mouse (PDF 3.2: hover highlights the whole lane,
// left click toggles selection, several lanes can be selected at once).
// Placement is lane first, then card: the lane clicked last (while it is selected) is the "placement lane";
// clicking a character card places that character there.

#include "config.h"

struct GLFWwindow;   // forward declaration: we only need the name, not the whole GLFW header

extern bool laneSelected[LANE_COUNT];   // true = lane is locked-highlighted (W/S jump and duck act on these lanes)
extern int placementLane;               // where the next card click places a character; -1 = no lane chosen
extern float placementX;                // x where the placement lane was clicked (Tayseer stands there)

// A click on a lane: if it is the placement lane, it is deselected (no placement lane any more). Any other lane
// becomes the placement lane with one click and is selected for jump/duck (it stays selected if it already was);
// the other selected lanes do not change.
void clickLane(int lane, float x);
// No lane selected and no placement lane (new game, next wave)
void clearLaneSelection();

// GLFW calls this when a mouse button changes; it only REMEMBERS the click.
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

// Returns true once per left click (and forgets it), so each click is handled exactly once.
bool takeLeftClick();

// Which lane contains the point (x, y) in game pixels, or -1 if the point is not on a lane.
int getLaneAt(float x, float y);

// Selected lanes: gold tint + gold border. The placement lane also gets a white inner border.
// Hovered lane: light white tint. A lane can be both.
void drawLaneHighlights(int hoveredLane);
