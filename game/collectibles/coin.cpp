#include "game/collectibles/coin.h"
#include "renderer/renderer.h"

// Designed at radius 10, then scaled. The Arabic letter "ain" uses the same two arcs as Anab Man's badge.
void drawAnabCoin(float radius) {
    pushMatrix();
    scale(radius / 10, radius / 10);
    drawCircle(0, 0, 10, 0.80f, 0.60f, 0.10f);                               // rim
    drawCircle(0, 0, 8.3f, 0.98f, 0.80f, 0.25f);                             // face
    drawArc(0, 0, 7, 0, 360, 0.8f, 0.85f, 0.62f, 0.12f);                     // inner ring
    drawArc(0.3f, 2.1f, 2.2f, 50, 300, 1.4f, 0.50f, 0.05f, 0.22f);           // "ain": small top hook
    drawArc(0.7f, -2.1f, 3.1f, 90, 280, 1.4f, 0.50f, 0.05f, 0.22f);          // "ain": bigger lower curve
    drawEllipse(-4.5f, 4.5f, 1.4f, 2.4f, 1.0f, 1.0f, 0.9f, 0.6f);            // shine
    popMatrix();
}
