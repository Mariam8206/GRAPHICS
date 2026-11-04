#include "enemies/easy/spider_man.h"
#include "shared/character_parts.h"
#include "renderer/renderer.h"
#include <cmath>

// ===================== ENEMY 1: Spider-Man (easy, attacks from above) =====================
void drawSpiderMan(float walkTime, bool moving, float attack) {
    float redR = 0.85f, redG = 0.10f, redB = 0.12f;
    float blueR = 0.12f, blueG = 0.25f, blueB = 0.75f;
    float swing = legSwing(walkTime, moving);

    pushMatrix();                                                           // back arm
    translate(-3, 52);
    rotate(-swing * 0.6f);
    drawRectangle(-3, -20, 6, 20, blueR * 0.8f, blueG * 0.8f, blueB * 0.8f);
    drawCircle(0, -21, 3.3f, redR * 0.8f, redG, redB);
    popMatrix();

    drawLeg(-4, 30, swing, 26, blueR * 0.8f, blueG * 0.8f, blueB * 0.8f, redR * 0.8f, redG, redB);  // blue legs, red boots
    drawLeg(4, 30, -swing, 26, blueR, blueG, blueB, redR, redG, redB);

    drawQuad(-10, 29, 10, 29, 12, 55, -12, 55, blueR, blueG, blueB);       // blue sides
    drawQuad(-6, 29, 6, 29, 9, 55, -9, 55, redR, redG, redB);              // red front
    for (int i = 0; i < 4; i++)                                             // web pattern lines
        drawLine(-8, 33 + i * 6.0f, 8, 33 + i * 6.0f, 0.6f, 0.15f, 0.05f, 0.05f);
    drawLine(0, 29, 0, 55, 0.6f, 0.15f, 0.05f, 0.05f);
    drawEllipse(0, 45, 2, 3, 0.05f, 0.05f, 0.05f);                          // spider emblem
    for (int i = -1; i <= 1; i += 2) {
        drawLine(0, 46, 5.0f * i, 50, 0.8f, 0.05f, 0.05f, 0.05f);
        drawLine(0, 44, 5.0f * i, 40, 0.8f, 0.05f, 0.05f, 0.05f);
    }

    // red masked head with big white eye lenses outlined in black
    drawCircle(1, 64, 9.5f, redR, redG, redB);
    drawLine(-7, 64, 9, 64, 0.5f, 0.15f, 0.05f, 0.05f);                     // mask web lines
    drawLine(1, 55, 1, 73, 0.5f, 0.15f, 0.05f, 0.05f);
    pushMatrix();
    translate(5, 66);
    rotate(-25);
    drawEllipse(0, 0, 5, 3.4f, 0.05f, 0.05f, 0.05f);
    drawEllipse(0, 0, 4, 2.4f, 1.0f, 1.0f, 1.0f);
    popMatrix();

    // front arm: points up and forward to shoot webs from above
    float armAngle = (attack >= 0) ? 120.0f + 20.0f * (1 - attack) : 70.0f;
    pushMatrix();
    translate(3, 52);
    rotate(armAngle);
    drawRectangle(-3, -20, 6, 20, redR, redG, redB);
    drawCircle(0, -21, 3.5f, redR, redG, redB);
    drawLine(-1, -23, -1, -28, 1.5f, redR, redG, redB);                     // fingers in the web-shooting pose
    drawLine(2, -23, 2, -28, 1.5f, redR, redG, redB);
    if (attack >= 0 && attack < 0.4f) drawLine(0, -24, 0, -60, 1, 0.95f, 0.95f, 0.95f, 0.8f);   // web strand
    popMatrix();
}

void drawWebBall(float time) {
    drawCircle(0, 0, 8, 0.97f, 0.97f, 0.97f, 0.9f);
    for (int i = 0; i < 4; i++) {                                            // web lines
        pushMatrix();
        rotate(i * 45.0f + time * 90.0f);
        drawLine(-8, 0, 8, 0, 0.8f, 0.55f, 0.55f, 0.60f);
        popMatrix();
    }
    drawArc(0, 0, 4.5f, 0, 360, 0.8f, 0.55f, 0.55f, 0.60f);
}
