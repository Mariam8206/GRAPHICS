#pragma once
// Tayseer, the DEFENSE player: police uniform, cap, mustache, riot shield. He never walks and never attacks.
// Model drawn in its OWN coordinates: (0, 0) = between the feet, facing RIGHT (+x), about 76 pixels tall.
// The game moves it with translate/scale. walkTime drives his slight breathing; hitAmount = how fresh the last
// hit is, 1 right after a hit fading to 0 (he is pushed back a little), 0 = not hit.
void drawTayseer(float walkTime, float hitAmount);
