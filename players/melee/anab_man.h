#pragma once
// Anab Man (El-Ragol El-Enab), the MELEE player: maroon costume, "ain" emblem, swings a club.
// Own coordinates: (0, 0) = between the feet, facing right (+x), about 76 px tall; the game moves, flips and
// squashes it. walkTime drives walking/idle, moving = legs swing, attack = 0 -> 1 (negative = not attacking).
void drawAnabMan(float walkTime, bool moving, float attack);
