#pragma once
// Dr. Beram, the RANGED player: hair, glasses, beard, green T-shirt, gray vest, El-Anab gun.
// Model drawn in its OWN coordinates: (0, 0) = between the feet, facing RIGHT (+x), about 76 pixels tall.
// The game moves, flips and squashes it with translate/scale. walkTime drives walking/idle, moving = legs swing,
// attack = progress of the current attack 0 -> 1 (negative = not attacking).
void drawBeram(float walkTime, bool moving, float attack);
// His shot, drawn centered on (0, 0): glowing maroon El-Anab energy.
void drawAnabEnergyBall(float time);
