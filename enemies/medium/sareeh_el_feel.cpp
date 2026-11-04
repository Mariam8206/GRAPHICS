#include "enemies/medium/sareeh_el_feel.h"
#include "shared/character_parts.h"
#include "renderer/renderer.h"
#include <cmath>

// ===================== ENEMY 2: Sareeh El-Feel (medium, attacks from below) =====================
// Animation: legs walk, the back arm swings against the legs. The front arm points forward (95 degrees) and sweeps
// down from 100 to 20 degrees during a throw, rolling the ballot box along the ground.
static CharacterPose sareehPose(float walkTime, bool moving, float attack) {
    CharacterPose pose = {};
    pose.legSwing = legSwing(walkTime, moving);
    pose.backArmAngle = -pose.legSwing * 0.6f;
    pose.frontArmAngle = (attack >= 0) ? 100.0f - 80.0f * attack : 95.0f;
    return pose;
}

void drawSareeh(float walkTime, bool moving, float attack) {
    float suitR = 0.20f, suitG = 0.20f, suitB = 0.24f;     // dark suit
    CharacterPose pose = sareehPose(walkTime, moving, attack);

    pushMatrix();                                           // back arm
    translate(BACK_SHOULDER_X, SHOULDER_Y);
    rotate(pose.backArmAngle);
    drawRectangle(-3.5f, -20, 7, 20, suitR * 0.8f, suitG * 0.8f, suitB * 0.8f);
    drawCircle(0, -21, 3.5f, SKIN[0] * 0.9f, SKIN[1] * 0.9f, SKIN[2] * 0.9f);
    popMatrix();

    drawLeg(BACK_HIP_X, HIP_Y, pose.legSwing, 26, suitR * 0.8f, suitG * 0.8f, suitB * 0.8f, 0.08f, 0.08f, 0.08f);
    drawLeg(FRONT_HIP_X, HIP_Y, -pose.legSwing, 26, suitR, suitG, suitB, 0.08f, 0.08f, 0.08f);

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
    pushMatrix();
    translate(FRONT_SHOULDER_X, SHOULDER_Y);
    rotate(pose.frontArmAngle);
    drawRectangle(-3.5f, -20, 7, 20, suitR, suitG, suitB);
    drawCircle(0, -21, 3.8f, SKIN[0], SKIN[1], SKIN[2]);
    drawLine(0, -24, 0, -29, 1.6f, SKIN[0], SKIN[1], SKIN[2]);               // pointing finger
    popMatrix();
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
