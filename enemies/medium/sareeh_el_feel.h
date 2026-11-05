#pragma once
// Sareeh El-Feel, the MEDIUM enemy: rolls ballot boxes along the ground.
// Own coordinates: (0, 0) = between the feet, facing right (+x), about 76 px tall; the game moves, flips and
// squashes it. walkTime drives walking/idle, moving = legs swing, attack = 0 -> 1 (negative = not attacking).
void drawSareeh(float walkTime, bool moving, float attack);
// His rolling ballot box, drawn centered on (0, 0).
void drawBallotBox(float rollAngle);
