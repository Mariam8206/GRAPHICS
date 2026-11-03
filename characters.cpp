#include "characters.h"
#include "renderer.h"
#include "text_labels.h"
#include <cmath>

// Colors are written as r, g, b in 0..1. A few shared ones:
static const float SKIN[3] = { 0.80f, 0.60f, 0.45f };

// One leg hanging from the hip at (hipX, hipY), swung by angleDegrees (positive = foot moves forward).
static void drawLeg(float hipX, float hipY, float angleDegrees, float length,
                    float r, float g, float b, float shoeR, float shoeG, float shoeB) {
    pushMatrix();
    translate(hipX, hipY);
    rotate(angleDegrees);
    drawRectangle(-3.5f, -length, 7, length, r, g, b);                 // leg
    drawRectangle(-4, -length - 3, 11, 4, shoeR, shoeG, shoeB);        // shoe, toe pointing forward
    popMatrix();
}

// Walking: legs swing opposite to each other. Standing: legs straight.
static float legSwing(float walkTime, bool moving) {
    return moving ? 25.0f * sin(walkTime * 9.0f) : 0.0f;
}

// Melee swing: arm goes from raised behind the head (160 deg) to struck forward (25 deg).
// attack < 0 means idle: the arm rests slightly forward and moves a little.
static float swingAngle(float attack, float walkTime) {
    if (attack < 0) return 30.0f + 5.0f * sin(walkTime * 3.0f);
    float eased = 1.0f - (1.0f - attack) * (1.0f - attack);           // fast start, slow end
    return 160.0f - 135.0f * eased;
}

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

// ===================== PLAYER 2: Beram (ranged scientist) =====================
void drawBeram(float walkTime, bool moving, float attack) {
    float hairR = 0.15f, hairG = 0.10f, hairB = 0.07f;
    float shirtR = 0.25f, shirtG = 0.60f, shirtB = 0.30f;   // green T-shirt
    float vestR = 0.42f, vestG = 0.42f, vestB = 0.45f;      // gray vest
    float swing = legSwing(walkTime, moving);
    float recoil = (attack > 0) ? attack : 0;               // 1 right after firing, fades to 0

    drawEllipse(-4, 56, 8, 10, hairR, hairG, hairB);       // long hair falling behind the shoulders

    pushMatrix();                                           // back arm
    translate(-3, 52);
    rotate(-swing * 0.6f);
    drawRectangle(-3, -8, 6, 8, shirtR * 0.8f, shirtG * 0.8f, shirtB * 0.8f);
    drawRectangle(-2.5f, -19, 5, 11, SKIN[0] * 0.85f, SKIN[1] * 0.85f, SKIN[2] * 0.85f);
    popMatrix();

    drawLeg(-4, 30, swing, 26, 0.15f, 0.20f, 0.35f, 0.35f, 0.22f, 0.12f);   // dark jeans, brown shoes
    drawLeg(4, 30, -swing, 26, 0.20f, 0.26f, 0.42f, 0.35f, 0.22f, 0.12f);

    drawQuad(-11, 29, 11, 29, 12, 55, -12, 55, shirtR, shirtG, shirtB);    // T-shirt
    drawQuad(-12, 30, -3, 30, -4, 54, -12, 54, vestR, vestG, vestB);       // vest, back panel
    drawQuad(5, 30, 12, 30, 12, 54, 6, 54, vestR, vestG, vestB);           // vest, front panel
    drawCircle(7, 44, 1, 0.2f, 0.2f, 0.2f);                                 // vest buttons
    drawCircle(7, 38, 1, 0.2f, 0.2f, 0.2f);

    // head: skin, beard, round glasses, hair on top
    drawCircle(1, 64, 9, SKIN[0], SKIN[1], SKIN[2]);
    drawEllipse(2, 57, 8, 6, hairR, hairG, hairB);                         // beard
    drawEllipse(5, 58, 2.5f, 1, 0.55f, 0.30f, 0.30f);                       // mouth inside the beard
    drawCircle(5.5f, 65, 1, 0.1f, 0.1f, 0.1f);                              // eye
    drawArc(5.5f, 65, 3.4f, 0, 360, 1.2f, 0.12f, 0.12f, 0.12f);             // round glasses
    drawLine(2, 65.5f, -4, 66, 1, 0.12f, 0.12f, 0.12f);                     // glasses arm
    drawEllipse(-1, 70, 10, 5, hairR, hairG, hairB);                        // hair on top
    drawRectangle(-10, 54, 6, 16, hairR, hairG, hairB);                     // hair down the back

    // front arm holding the El-Anab gun forward; it kicks back when firing
    pushMatrix();
    translate(3, 52);
    rotate(80 + recoil * 15);
    drawRectangle(-3, -8, 6, 8, shirtR, shirtG, shirtB);                    // sleeve
    drawRectangle(-2.5f, -19, 5, 11, SKIN[0], SKIN[1], SKIN[2]);
    translate(0, -21);
    rotate(-80);                                                            // keep the gun level
    drawRectangle(-2, -3, 18, 7, 0.55f, 0.56f, 0.60f);                      // gun body
    drawRectangle(16, -1.5f, 6, 4, 0.30f, 0.30f, 0.33f);                    // barrel
    drawRectangle(0, -9, 4, 7, 0.30f, 0.30f, 0.33f);                        // grip
    drawCircle(7, 8, 4.5f, 0.85f, 0.92f, 0.95f);                            // glass flask on top
    drawCircle(7, 7, 3.2f, 0.55f, 0.05f, 0.25f);                            // El-Anab liquid
    if (recoil > 0.5f) drawCircle(24, 0.5f, 5 * recoil, 0.85f, 0.15f, 0.45f, 0.8f);   // muzzle flash
    popMatrix();
}

// ===================== PLAYER 3: Tayseer Pasha (defense, police uniform) =====================
void drawTayseer(float walkTime, float attack) {
    float breathe = 1.0f + 0.02f * sin(walkTime * 2.0f);     // standing guard, slight breathing
    float flinch = (attack > 0) ? -3.0f * attack : 0;        // pushed back a little when hit

    pushMatrix();
    translate(flinch, 0);
    drawLeg(-6, 30, 0, 26, 0.10f, 0.10f, 0.12f, 0.05f, 0.05f, 0.05f);      // black trousers, boots
    drawLeg(5, 30, 0, 26, 0.12f, 0.12f, 0.14f, 0.05f, 0.05f, 0.05f);

    // big white uniform shirt (wide belly) with black belt
    pushMatrix();
    scale(1, breathe);
    drawEllipse(1, 42, 15, 15, 0.95f, 0.95f, 0.95f);
    popMatrix();
    drawRectangle(-14, 29, 28, 4, 0.08f, 0.08f, 0.08f);
    drawRectangle(-2, 29, 5, 4, 0.85f, 0.70f, 0.20f);                       // belt buckle
    drawRectangle(-13, 52, 9, 4, 0.08f, 0.08f, 0.10f);                      // black epaulette
    drawCircle(-11, 54, 1, 0.95f, 0.80f, 0.25f);                            // gold stars
    drawCircle(-8, 54, 1, 0.95f, 0.80f, 0.25f);
    drawCircle(-5, 54, 1, 0.95f, 0.80f, 0.25f);
    drawRectangle(-1, 34, 2, 18, 0.80f, 0.80f, 0.82f);                      // shirt buttons line

    // chubby face, mustache, peaked police cap with gold badge
    drawCircle(2, 64, 10, SKIN[0], SKIN[1], SKIN[2]);
    drawEllipse(7, 60, 5, 2, 0.10f, 0.08f, 0.06f);                          // mustache
    drawCircle(7, 65, 1.2f, 0.1f, 0.1f, 0.1f);                              // eye
    drawLine(4, 68, 9, 68.5f, 1.2f, 0.10f, 0.08f, 0.06f);                   // eyebrow
    drawRectangle(-8, 70, 20, 7, 0.08f, 0.08f, 0.10f);                      // cap
    drawQuad(4, 70, 16, 68, 16, 70, 4, 72, 0.05f, 0.05f, 0.05f);            // visor
    drawCircle(3, 74, 2, 0.95f, 0.80f, 0.25f);                              // badge

    // riot shield held in front (the blocking equipment)
    drawRectangle(12, 4, 16, 70, 0.20f, 0.22f, 0.25f);                      // frame
    drawRectangle(14, 6, 12, 66, 0.55f, 0.72f, 0.90f, 0.85f);               // clear shield
    drawRectangle(15, 50, 3, 18, 1.0f, 1.0f, 1.0f, 0.5f);                   // shine
    pushMatrix();
    translate(25, 8);                                                       // low enough that "POLICE" ends below the hand
    rotate(90);
    drawLabelAt("shield_police", 0, 0, 10, 1.0f, 1.0f, 1.0f);   // "POLICE", reading up the shield
    popMatrix();
    drawCircle(14, 40, 3.5f, SKIN[0], SKIN[1], SKIN[2]);                    // hand gripping the shield
    popMatrix();
}

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

// ===================== ENEMY 2: Sareeh El-Feel (medium, attacks from below) =====================
void drawSareeh(float walkTime, bool moving, float attack) {
    float suitR = 0.20f, suitG = 0.20f, suitB = 0.24f;     // dark suit
    float swing = legSwing(walkTime, moving);

    pushMatrix();                                           // back arm
    translate(-3, 52);
    rotate(-swing * 0.6f);
    drawRectangle(-3.5f, -20, 7, 20, suitR * 0.8f, suitG * 0.8f, suitB * 0.8f);
    drawCircle(0, -21, 3.5f, SKIN[0] * 0.9f, SKIN[1] * 0.9f, SKIN[2] * 0.9f);
    popMatrix();

    drawLeg(-4, 30, swing, 26, suitR * 0.8f, suitG * 0.8f, suitB * 0.8f, 0.08f, 0.08f, 0.08f);
    drawLeg(4, 30, -swing, 26, suitR, suitG, suitB, 0.08f, 0.08f, 0.08f);

    drawEllipse(1, 42, 14, 15, suitR, suitG, suitB);                         // suit jacket, round belly
    drawTriangle(-2, 56, 8, 56, 4, 44, 0.95f, 0.95f, 0.95f);                 // white shirt collar
    drawRectangle(-1, 30, 5, 26, 0.45f, 0.45f, 0.45f);                       // gray scarf hanging down
    drawRectangle(5, 34, 4, 22, 0.40f, 0.40f, 0.40f);
    for (float sy = 33; sy < 54; sy += 4)
        drawRectangle(-1, sy, 5, 1.2f, 0.30f, 0.30f, 0.30f);                 // scarf pattern
    drawRectangle(5, 34, 4, 2, 0.20f, 0.65f, 0.65f);                         // tag at the scarf end

    // bald head with gray fringe, round light-framed glasses, big smile
    drawCircle(1, 65, 10, SKIN[0], SKIN[1], SKIN[2]);
    drawEllipse(-1, 70, 6, 3, 0.95f, 0.80f, 0.68f);                          // shiny bald top
    drawEllipse(-7, 63, 3.5f, 5, 0.70f, 0.70f, 0.70f);                       // gray hair at the back
    drawArc(6, 65, 3.4f, 0, 360, 1.2f, 0.85f, 0.75f, 0.55f);                 // glasses
    drawLine(2.6f, 65.5f, -4, 66, 1, 0.85f, 0.75f, 0.55f);
    drawCircle(6, 65, 1, 0.1f, 0.1f, 0.1f);                                  // eye
    drawArc(5, 60, 3.5f, 200, 340, 1.4f, 0.45f, 0.15f, 0.15f);               // smile

    // front arm: points forward (like the photo); swings low to roll the ballot box
    float armAngle = (attack >= 0) ? 100.0f - 80.0f * attack : 95.0f;
    pushMatrix();
    translate(3, 52);
    rotate(armAngle);
    drawRectangle(-3.5f, -20, 7, 20, suitR, suitG, suitB);
    drawCircle(0, -21, 3.8f, SKIN[0], SKIN[1], SKIN[2]);
    drawLine(0, -24, 0, -29, 1.6f, SKIN[0], SKIN[1], SKIN[2]);               // pointing finger
    popMatrix();
}

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

// ===================== Projectiles =====================
void drawAnabEnergyBall(float time) {
    float pulse = 1.0f + 0.15f * sin(time * 25.0f);
    drawEllipse(-14, 0, 14, 3, 0.85f, 0.15f, 0.45f, 0.35f);                  // trail
    drawCircle(0, 0, 11 * pulse, 0.85f, 0.15f, 0.45f, 0.30f);                // glow
    drawCircle(0, 0, 6.5f, 0.55f, 0.05f, 0.25f);                             // El-Anab core
    drawCircle(-1.5f, 1.5f, 2.5f, 1.0f, 0.75f, 0.85f);                       // bright center
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

void drawBallotBox(float rollAngle) {
    pushMatrix();
    rotate(rollAngle);
    drawRectangle(-8, -7, 16, 14, 0.96f, 0.96f, 0.96f);                      // box
    drawRectangle(-8, 4, 16, 3, 0.15f, 0.35f, 0.75f);                        // blue lid
    drawRectangle(-4, 5, 8, 1, 0.05f, 0.05f, 0.05f);                         // ballot slot
    drawCircle(0, -2, 2.5f, 0.80f, 0.15f, 0.15f);                            // red seal
    popMatrix();
}

// ===================== Coin =====================
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
