#pragma once
// Body parts and animation helpers used by BOTH the player models (players/) and the enemy models (enemies/).
// They are kept here, outside both folders, because both sides need them.

// Colors are written as r, g, b in 0..1.
const float SKIN[3] = { 0.80f, 0.60f, 0.45f };   // skin color shared by several characters

// Joints of the standard character skeleton, in the model's own coordinates ((0, 0) = between the feet,
// facing right). Body parts are drawn around these points and rotated there (pushMatrix, translate, rotate).
const float HIP_Y = 30;              // both legs hang from here
const float BACK_HIP_X = -4;
const float FRONT_HIP_X = 4;
const float SHOULDER_Y = 52;         // both arms hang from here
const float BACK_SHOULDER_X = -3;
const float FRONT_SHOULDER_X = 3;

// How a character is posed at one moment: the angles (degrees) its joints are turned by. Each character has one
// function that works these out from walkTime / moving / attack; its draw function only draws with them.
struct CharacterPose {
    float legSwing;        // back leg turns by +legSwing, front leg by -legSwing
    float backArmAngle;    // back arm, turned at BACK_SHOULDER
    float frontArmAngle;   // front arm and whatever it holds (weapon), turned at FRONT_SHOULDER
};

// One leg hanging from the hip at (hipX, hipY), swung by angleDegrees (positive = foot moves forward).
void drawLeg(float hipX, float hipY, float angleDegrees, float length,
             float r, float g, float b, float shoeR, float shoeG, float shoeB);
// Walking: legs swing opposite to each other. Standing: legs straight.
float legSwing(float walkTime, bool moving);
// Melee swing angle of the arm: attack goes 0 -> 1 during a swing; a negative value means idle.
float swingAngle(float attack, float walkTime);
