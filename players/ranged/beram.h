#pragma once
// Dr. Beram, the RANGED player: hair, glasses, beard, green T-shirt, gray vest, El-Anab gun.
// Own coordinates: (0, 0) = between the feet, facing right (+x), about 76 px tall; the game moves, flips and
// squashes it. walkTime drives walking/idle, moving = legs swing, attack = 0 -> 1 (negative = not attacking).
void drawBeram(float walkTime, bool moving, float attack);
// His shot, drawn centered on (0, 0): glowing maroon El-Anab energy.
void drawAnabEnergyBall(float time);
