#include "players/melee/anab_man.h"
#include "shared/character_parts.h"
#include "renderer/renderer.h"
#include <cmath>

// ===================== PLAYER 1: El-Ragol El-Enab (melee) =====================
void drawAnabMan(float walkTime, bool moving, float attack) {
    float suitR = 0.45f, suitG = 0.13f, suitB = 0.25f;     // maroon/purple costume
    float darkR = 0.30f, darkG = 0.08f, darkB = 0.16f;
    float swing = legSwing(walkTime, moving);

    // back arm (darker, behind the body)
    pushMatrix();
    translate(-3, 52);
    rotate(-swing * 0.6f);
    drawRectangle(-3, -20, 6, 20, darkR, darkG, darkB);
    drawCircle(0, -21, 3.5f, 0.1f, 0.1f, 0.1f);                        // black glove
    popMatrix();

    drawLeg(-4, 30, swing, 26, darkR, darkG, darkB, 0.12f, 0.12f, 0.12f);
    drawLeg(4, 30, -swing, 26, suitR, suitG, suitB, 0.12f, 0.12f, 0.12f);

    // torso with silver side stripes and belt
    drawQuad(-11, 29, 11, 29, 13, 55, -13, 55, suitR, suitG, suitB);
    drawLine(-11, 31, -13, 53, 1.5f, 0.85f, 0.85f, 0.90f);
    drawLine(11, 31, 13, 53, 1.5f, 0.85f, 0.85f, 0.90f);
    drawRectangle(-11, 29, 22, 4, darkR, darkG, darkB);
    drawRectangle(-2, 29, 4, 4, 0.85f, 0.85f, 0.90f);                  // buckle

    // chest emblem: silver badge with the Arabic letter "ain" drawn with two arcs
    drawCircle(1, 44, 7, 0.85f, 0.85f, 0.90f);
    drawArc(2, 46.5f, 2.6f, 50, 300, 1.6f, suitR, suitG, suitB);      // small top hook
    drawArc(2.5f, 41.5f, 3.6f, 90, 280, 1.6f, suitR, suitG, suitB);   // bigger lower curve

    // head: lower face visible, the rest covered by the mask
    drawEllipse(3, 58, 6, 4.5f, SKIN[0], SKIN[1], SKIN[2]);
    drawCircle(0, 65, 9.5f, suitR, suitG, suitB);                      // mask
    pushMatrix();                                                      // white eye patch, tilted
    translate(5, 66);
    rotate(20);
    drawEllipse(0, 0, 4.5f, 2.6f, 0.95f, 0.95f, 0.97f);
    popMatrix();
    drawCircle(6, 66, 1.2f, 0.1f, 0.1f, 0.1f);                         // eye
    drawTriangle(-6, 64, -13, 72, -8, 60, 0.85f, 0.85f, 0.90f);        // silver wing on the mask side
    drawTriangle(-5, 68, -11, 76, -3, 72, 0.85f, 0.85f, 0.90f);
    drawLine(4, 56, 7, 56.5f, 1, 0.4f, 0.2f, 0.2f);                    // mouth

    // front arm swinging a wooden club (the close-range weapon animates in the hand)
    pushMatrix();
    translate(3, 52);
    rotate(swingAngle(attack, walkTime));
    drawRectangle(-3, -20, 6, 20, suitR, suitG, suitB);
    drawCircle(0, -21, 3.8f, 0.1f, 0.1f, 0.1f);                        // glove
    drawQuad(-2, -19, -2, -23, 28, -26, 28, -16, 0.55f, 0.35f, 0.18f); // club: thin handle, thick end
    drawRectangle(20, -25, 3, 9, 0.40f, 0.24f, 0.12f);                 // band on the club
    popMatrix();
}
