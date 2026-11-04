#include "players/defense/tayseer.h"
#include "shared/character_parts.h"
#include "renderer/renderer.h"
#include "ui/text_labels.h"   // the POLICE label on his shield
#include <cmath>

// ===================== PLAYER 3: Tayseer Pasha (defense, police uniform) =====================
// Animation: he stands guard (legs never swing), breathes slightly, and is pushed back up to 3 pixels when hit.
static CharacterPose tayseerPose(float walkTime, float hitAmount) {
    CharacterPose pose = {};
    pose.breathe = 1.0f + 0.02f * sin(walkTime * 2.0f);
    pose.flinch = (hitAmount > 0) ? -3.0f * hitAmount : 0;
    return pose;
}

void drawTayseer(float walkTime, float hitAmount) {
    CharacterPose pose = tayseerPose(walkTime, hitAmount);

    pushMatrix();
    translate(pose.flinch, 0);
    // legs: a wider stance than the standard hips (BACK_HIP_X / FRONT_HIP_X), and they never swing
    drawLeg(-6, HIP_Y, 0, 26, 0.10f, 0.10f, 0.12f, 0.05f, 0.05f, 0.05f);   // black trousers, boots
    drawLeg(5, HIP_Y, 0, 26, 0.12f, 0.12f, 0.14f, 0.05f, 0.05f, 0.05f);

    // big white uniform shirt (wide belly) with black belt
    pushMatrix();
    scale(1, pose.breathe);
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
