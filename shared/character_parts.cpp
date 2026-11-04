#include "shared/character_parts.h"
#include "renderer/renderer.h"
#include <cmath>

// One leg hanging from the hip at (hipX, hipY), swung by angleDegrees (positive = foot moves forward).
void drawLeg(float hipX, float hipY, float angleDegrees, float length,
                    float r, float g, float b, float shoeR, float shoeG, float shoeB) {
    pushMatrix();
    translate(hipX, hipY);
    rotate(angleDegrees);
    drawRectangle(-3.5f, -length, 7, length, r, g, b);                 // leg
    drawRectangle(-4, -length - 3, 11, 4, shoeR, shoeG, shoeB);        // shoe, toe pointing forward
    popMatrix();
}

// Walking: legs swing opposite to each other. Standing: legs straight.
float legSwing(float walkTime, bool moving) {
    return moving ? 25.0f * sin(walkTime * 9.0f) : 0.0f;
}

// Melee swing: arm goes from raised behind the head (160 deg) to struck forward (25 deg).
// attack < 0 means idle: the arm rests slightly forward and moves a little.
float swingAngle(float attack, float walkTime) {
    if (attack < 0) return 30.0f + 5.0f * sin(walkTime * 3.0f);
    float eased = 1.0f - (1.0f - attack) * (1.0f - attack);           // fast start, slow end
    return 160.0f - 135.0f * eased;
}
