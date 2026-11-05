#pragma once
// Abla Thalma, the HARD enemy: charges and hits with a slipper.
// Own coordinates: (0, 0) = between the feet, facing right (+x), about 76 px tall; the game moves, flips and
// squashes it. walkTime drives walking/idle, moving = legs swing, attack = 0 -> 1 (negative = not attacking).
void drawThalma(float walkTime, bool moving, float attack);
