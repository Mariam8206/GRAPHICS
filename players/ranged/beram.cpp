#include "players/ranged/beram.h"
#include "shared/character_parts.h"
#include "renderer/renderer.h"
#include <cmath>

// ===================== PLAYER 2: Beram (ranged scientist) =====================
// Animation: legs walk, the back arm swings against the legs, the front arm holds the gun forward (80 degrees)
// and kicks up a little more (up to 15 degrees) right after firing.
static CharacterPose beramPose(float walkTime, bool moving, float attack) {
    CharacterPose pose = {};
    pose.legSwing = legSwing(walkTime, moving);
    pose.backArmAngle = -pose.legSwing * 0.6f;
    pose.recoil = (attack > 0) ? attack : 0;               // 1 right after firing, fades to 0
    pose.frontArmAngle = 80 + pose.recoil * 15;
    return pose;
}

void drawBeram(float walkTime, bool moving, float attack) {
    float hairR = 0.15f, hairG = 0.10f, hairB = 0.07f;
    float shirtR = 0.25f, shirtG = 0.60f, shirtB = 0.30f;   // green T-shirt
    float vestR = 0.42f, vestG = 0.42f, vestB = 0.45f;      // gray vest
    CharacterPose pose = beramPose(walkTime, moving, attack);

    drawEllipse(-4, 56, 8, 10, hairR, hairG, hairB);       // long hair falling behind the shoulders

    pushMatrix();                                           // back arm
    translate(BACK_SHOULDER_X, SHOULDER_Y);
    rotate(pose.backArmAngle);
    drawRectangle(-3, -8, 6, 8, shirtR * 0.8f, shirtG * 0.8f, shirtB * 0.8f);
    drawRectangle(-2.5f, -19, 5, 11, SKIN[0] * 0.85f, SKIN[1] * 0.85f, SKIN[2] * 0.85f);
    popMatrix();

    // dark jeans, brown shoes
    drawLeg(BACK_HIP_X, HIP_Y, pose.legSwing, 26, 0.15f, 0.20f, 0.35f, 0.35f, 0.22f, 0.12f);
    drawLeg(FRONT_HIP_X, HIP_Y, -pose.legSwing, 26, 0.20f, 0.26f, 0.42f, 0.35f, 0.22f, 0.12f);

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
    translate(FRONT_SHOULDER_X, SHOULDER_Y);
    rotate(pose.frontArmAngle);
    drawRectangle(-3, -8, 6, 8, shirtR, shirtG, shirtB);                    // sleeve
    drawRectangle(-2.5f, -19, 5, 11, SKIN[0], SKIN[1], SKIN[2]);
    translate(0, -21);
    rotate(-80);                                                            // keep the gun level
    drawRectangle(-2, -3, 18, 7, 0.55f, 0.56f, 0.60f);                      // gun body
    drawRectangle(16, -1.5f, 6, 4, 0.30f, 0.30f, 0.33f);                    // barrel
    drawRectangle(0, -9, 4, 7, 0.30f, 0.30f, 0.33f);                        // grip
    drawCircle(7, 8, 4.5f, 0.85f, 0.92f, 0.95f);                            // glass flask on top
    drawCircle(7, 7, 3.2f, 0.55f, 0.05f, 0.25f);                            // El-Anab liquid
    if (pose.recoil > 0.5f) drawCircle(24, 0.5f, 5 * pose.recoil, 0.85f, 0.15f, 0.45f, 0.8f);   // muzzle flash
    popMatrix();
}

void drawAnabEnergyBall(float time) {
    float pulse = 1.0f + 0.15f * sin(time * 25.0f);
    drawEllipse(-14, 0, 14, 3, 0.85f, 0.15f, 0.45f, 0.35f);                  // trail
    drawCircle(0, 0, 11 * pulse, 0.85f, 0.15f, 0.45f, 0.30f);                // glow
    drawCircle(0, 0, 6.5f, 0.55f, 0.05f, 0.25f);                             // El-Anab core
    drawCircle(-1.5f, 1.5f, 2.5f, 1.0f, 0.75f, 0.85f);                       // bright center
}
