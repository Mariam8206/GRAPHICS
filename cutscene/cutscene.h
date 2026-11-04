#pragma once
// The 16-second animated introduction (PDF section 2): Dr. Beram shows the last anab bottle, three villains
// arrive and threaten to take it, Spider-Man throws a web that Anab Man ducks, Anab Man hits back, and the
// villains advance as the game begins. Subtitles (text images) support the action; Enter/Space skips.

const float CUTSCENE_LENGTH = 16.0f;   // seconds (PDF: 10-20)

void startCutscene();
bool updateCutscene(float dt);   // true once the cutscene has finished
float cutsceneTime();
void drawCutscene();
