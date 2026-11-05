#pragma once
// Spider-Man, the EASY enemy: attacks from above with webs.
// Own coordinates: (0, 0) = between the feet, facing right (+x), about 76 px tall; the game moves, flips and
// squashes it. walkTime drives walking/idle, moving = legs swing, attack = 0 -> 1 (negative = not attacking).
void drawSpiderMan(float walkTime, bool moving, float attack);
// His web, drawn centered on (0, 0).
void drawWebBall(float time);
