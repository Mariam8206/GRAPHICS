#pragma once
// Gameplay: players, enemies, projectiles, combat, waves, win/lose, coins and placement.

enum PlayerType { PLAYER_MELEE, PLAYER_RANGED, PLAYER_DEFENSE };
const int PLAYER_TYPE_COUNT = 3;

// Result of a placement attempt. Only PLACE_OK creates a character and takes coins.
// PLACE_GAME_OVER and PLACE_BAD_LANE are safety checks only: refused like the others, but no message is shown.
// PLACE_NO_ROOM: a Tayseer finds no free spot in the lane (Tayseers and enemies fill it); also no message.
// There is no limit on how many characters of a type can be placed: as many as the player can afford.
enum PlaceResult { PLACE_OK, PLACE_GAME_OVER, PLACE_LOCKED, PLACE_NO_COINS,
                   PLACE_BAD_LANE, PLACE_NO_ROOM };

void resetGame();                  // clears everything (coins back to start, Anab Man first again) and starts wave 1
void updateGame(float deltaTime);  // moves and fights; does nothing once the game is won or lost (freeze)
void drawGame(float time);         // characters, coins, projectiles, health bars, messages, banners

// --- Placement: card first, then lane ---
void clickCard(PlayerType type);                      // selects the card, or cancels if it was already selected
int getSelectedCard();                                // -1 = no card selected
// Checks everything; pays only if the character is placed. Anab Man and Beram appear at the left end of the lane;
// Tayseer (Defense) stands where the lane was clicked (clickX, in pixels; -1 = left end).
PlaceResult tryPlacePlayer(PlayerType type, int lane, float clickX = -1.0f);

void jumpSelectedLanes();          // W / Up: every non-defense player in a selected lane jumps
void duckSelectedLanes();          // S / Down: every non-defense player in a selected lane ducks
void collectCoinsAt(float x, float y);   // mouse hover: collects every coin pickup near (x, y)

// --- Information for the top bar and the cards ---
int getCoins();
int getPrice(PlayerType type);
int getUnlockThreshold(PlayerType type);
int countOnField(PlayerType type);
bool isUnlocked(PlayerType type);
bool isWaitingForFirstMelee();   // true until the first Anab Man of the game is placed (Beram and Tayseer stay locked)

int getCurrentWave();
bool isGameWon();
bool isGameLost();
