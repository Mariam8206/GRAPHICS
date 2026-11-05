#pragma once
// Top UI bar (PDF 4.2: collected coins shown at the top; player cards).
// Coin counter, wave label and the three character cards (lane first, then card).

void drawTopBar(int coins, int wave, int hoveredCard, float time);

// Which card contains the point (x, y) in game pixels: 0 = Anab Man, 1 = Beram, 2 = Tayseer, -1 = none.
int getCardAt(float x, float y);
