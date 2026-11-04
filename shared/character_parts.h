#pragma once
// Body parts and animation helpers used by BOTH the player models (players/) and the enemy models (enemies/).
// They are kept here, outside both folders, because both sides need them.

// Colors are written as r, g, b in 0..1.
const float SKIN[3] = { 0.80f, 0.60f, 0.45f };   // skin color shared by several characters

// One leg hanging from the hip at (hipX, hipY), swung by angleDegrees (positive = foot moves forward).
void drawLeg(float hipX, float hipY, float angleDegrees, float length,
             float r, float g, float b, float shoeR, float shoeG, float shoeB);
// Walking: legs swing opposite to each other. Standing: legs straight.
float legSwing(float walkTime, bool moving);
// Melee swing angle of the arm: attack goes 0 -> 1 during a swing; a negative value means idle.
float swingAngle(float attack, float walkTime);
