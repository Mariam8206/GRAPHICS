#pragma once
// Anab Man (El-Ragol El-Enab), the MELEE player: maroon costume, "ain" emblem, swings a club.
// Model drawn in its OWN coordinates: (0, 0) = between the feet, facing RIGHT (+x), about 76 pixels tall.
// The game moves, flips and squashes it with translate/scale. walkTime drives walking/idle, moving = legs swing,
// attack = progress of the current attack 0 -> 1 (negative = not attacking).
void drawAnabMan(float walkTime, bool moving, float attack);
