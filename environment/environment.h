#pragma once
// The game world: an Egyptian street (background + 4 street lanes) and Dr. Beram's laboratory (home base).
//
// Environmental elements (PDF 4.2 needs at least 5):
//   1. Egyptian apartment facades   2. Balconies        3. Rooftop water tanks
//   4. Satellite dishes             5. Clotheslines     6. Streetlights
//   7. Kiosk                        8. Election posters of Sareeh El-Feel

void drawEnvironment(float time);   // sky, buildings and street; call first every frame
void drawLaboratory(float time);    // the home base on the far left, with the last formula bottle
