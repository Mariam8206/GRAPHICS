#include "enemies/hard/abla_thalma.h"
#include "shared/character_parts.h"
#include "renderer/renderer.h"
#include <cmath>

// ===================== ENEMY 3: Abla Thalma (hard, charges and hits with a slipper) =====================
void drawThalma(float walkTime, bool moving, float attack) {
    float dressR = 0.45f, dressG = 0.25f, dressB = 0.65f;    // colorful patterned dress (purple base)
    float hairR = 0.45f, hairG = 0.22f, hairB = 0.10f;       // brown hair
    float swing = moving ? 35.0f * sin(walkTime * 13.0f) : 0; // running: faster, bigger steps

    pushMatrix();
    if (moving) rotate(-8);                                   // leans forward while charging

    drawEllipse(-3, 60, 9, 11, hairR, hairG, hairB);          // hair behind the head, to the shoulders

    pushMatrix();                                             // back arm, reaching forward while running
    translate(-3, 52);
    rotate(moving ? 70.0f : -10.0f);
    drawRectangle(-3, -18, 6, 18, SKIN[0] * 0.9f, SKIN[1] * 0.9f, SKIN[2] * 0.9f);
    popMatrix();

    drawLeg(-4, 30, swing, 26, SKIN[0] * 0.9f, SKIN[1] * 0.9f, SKIN[2] * 0.9f, 0.55f, 0.20f, 0.30f);
    drawLeg(4, 30, -swing, 26, SKIN[0], SKIN[1], SKIN[2], 0.55f, 0.20f, 0.30f);

    drawQuad(-15, 22, 15, 22, 10, 50, -10, 50, dressR, dressG, dressB);     // dress
    drawRectangle(-10, 48, 20, 8, dressR, dressG, dressB);
    float dots[8][2] = { {-9,27},{-2,31},{6,26},{10,34},{-6,39},{3,42},{-3,49},{7,48} };
    for (int i = 0; i < 8; i++) {                                            // floral pattern
        if (i % 3 == 0) drawCircle(dots[i][0], dots[i][1], 2.2f, 0.95f, 0.85f, 0.25f);
        else if (i % 3 == 1) drawCircle(dots[i][0], dots[i][1], 2.2f, 0.35f, 0.75f, 0.45f);
        else drawCircle(dots[i][0], dots[i][1], 2.2f, 0.95f, 0.95f, 0.95f);
    }
    drawArc(1, 56, 6, 200, 340, 1, 0.85f, 0.75f, 0.30f);                     // necklace
    drawCircle(1, 50.5f, 1.6f, 0.20f, 0.55f, 0.85f);                         // pendant

    // head: bangs, black rectangular glasses, hoop earring
    drawCircle(1, 64, 9, SKIN[0], SKIN[1], SKIN[2]);
    drawEllipse(0, 70, 10, 5, hairR, hairG, hairB);                          // top of the hair
    drawRectangle(-2, 66, 11, 4, hairR, hairG, hairB);                       // bangs
    drawRectangleOutline(3, 61, 7, 4.5f, 1.2f, 0.05f, 0.05f, 0.05f);         // glasses
    drawLine(3, 63, -3, 64, 1, 0.05f, 0.05f, 0.05f);
    drawCircle(6.5f, 63, 0.9f, 0.1f, 0.1f, 0.1f);                            // eye
    drawArc(-3, 57, 2.4f, 0, 360, 1, 0.90f, 0.75f, 0.25f);                   // hoop earring
    drawLine(4, 57.5f, 8, 57, 1.2f, 0.70f, 0.20f, 0.25f);                    // lips

    // front arm with the slipper: raised behind, then smacks forward
    pushMatrix();
    translate(3, 52);
    rotate(swingAngle(attack, walkTime) + (moving && attack < 0 ? 50.0f : 0.0f));
    drawRectangle(-3, -18, 6, 18, SKIN[0], SKIN[1], SKIN[2]);
    drawCircle(0, -19, 3.2f, SKIN[0], SKIN[1], SKIN[2]);
    drawEllipse(9, -20, 11, 4, 0.20f, 0.55f, 0.90f);                         // slipper sole
    drawRectangle(4, -18, 8, 3, 0.95f, 0.85f, 0.25f);                        // strap
    popMatrix();
    popMatrix();
}
