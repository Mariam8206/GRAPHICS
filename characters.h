#pragma once
// The six character models and the three kinds of projectiles.
// Every model is drawn in its OWN coordinates: (0, 0) = between the feet, the character faces RIGHT (+x),
// and it is about 76 pixels tall. The game moves/flips/squashes them with translate/scale.
//
// Parameters used by all models:
//   walkTime : seconds the character has existed; drives the walking/idle animation
//   moving   : true while walking (legs swing), false while standing/fighting
//   attack   : progress of the current attack animation, 0 -> 1. A negative value = not attacking.

// --- Players ---
void drawAnabMan(float walkTime, bool moving, float attack);     // melee: maroon costume, "ain" emblem, swings a club
void drawBeram(float walkTime, bool moving, float attack);       // ranged: hair, glasses, beard, green T-shirt, gray vest, El-Anab gun
void drawTayseer(float walkTime, float attack);                  // defense: police uniform, cap, mustache, riot shield (never walks)

// --- Enemies ---
void drawSpiderMan(float walkTime, bool moving, float attack);   // easy: shoots webs from above
void drawSareeh(float walkTime, bool moving, float attack);      // medium: rolls ballot boxes along the ground
void drawThalma(float walkTime, bool moving, float attack);      // hard: charges and hits with a slipper

// --- Projectiles (drawn centered on (0, 0)) ---
void drawAnabEnergyBall(float time);   // Beram's shot: glowing maroon El-Anab energy
void drawWebBall(float time);          // Spider-Man's web
void drawBallotBox(float rollAngle);   // Sareeh's rolling ballot box

// --- Coin (drawn centered on (0, 0)): gold coin with the Arabic letter "ain" emblem ---
void drawAnabCoin(float radius);
