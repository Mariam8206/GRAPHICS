#pragma once
// Sareeh El-Feel, the MEDIUM enemy: rolls ballot boxes along the ground.
// Model drawn in its OWN coordinates: (0, 0) = between the feet, facing RIGHT (+x), about 76 pixels tall.
// The game moves, flips and squashes it with translate/scale. walkTime drives walking/idle, moving = legs swing,
// attack = progress of the current attack 0 -> 1 (negative = not attacking).
void drawSareeh(float walkTime, bool moving, float attack);
// His rolling ballot box, drawn centered on (0, 0).
void drawBallotBox(float rollAngle);
