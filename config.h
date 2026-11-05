#pragma once
// Shared constants for the whole game. Every position is in screen pixels, (0, 0) = bottom-left.

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

const float PI = 3.14159265f;

// --- Playfield layout ---
const int LANE_COUNT = 4;                                        // the PDF needs at least 3 lanes
const float LANE_HEIGHT = 110.0f;
// y where the lowest lane starts (front sidewalk below)
const float FIELD_BOTTOM = 30.0f;
const float FIELD_TOP = FIELD_BOTTOM + LANE_COUNT * LANE_HEIGHT; // 470: background street starts above this
const float BASE_WIDTH = 170.0f;                                 // laboratory (home base) occupies x = 0 .. 170

// --- UI ---
const float UI_BAR_BOTTOM = 650.0f;                              // top bar occupies y = 650 .. 720
// Cards are taller than the bar and hang below it, over the 4-floor buildings. A 110 px bar would hide
// the formula bottle, the lab antenna and the roof props of the 3-floor buildings.
const float CARD_WIDTH = 160.0f;
const float CARD_HEIGHT = 96.0f;

const int TOTAL_WAVES = 3;

// ===================== Economy (team values) =====================
const int STARTING_COINS = 30;
// Buying price: paid for EVERY successful placement.
const int MELEE_PRICE = 10;
const int RANGED_PRICE = 20;
const int DEFENSE_PRICE = 30;
// Unlocking threshold (PDF): a card is unlocked while the balance is at least this value and locks again as soon
// as the balance falls below it (0 = always unlocked). Team decision: the threshold IS the price (2x and 3x the
// Melee price), so a character can be bought the moment it is affordable, in any wave.
const int MELEE_UNLOCK_AT = 0;
const int RANGED_UNLOCK_AT = RANGED_PRICE;     // 20
const int DEFENSE_UNLOCK_AT = DEFENSE_PRICE;   // 30
// Coins dropped by a defeated enemy (team decision: every enemy is worth 10)
const int EASY_REWARD = 10;
const int MEDIUM_REWARD = 10;
const int HARD_REWARD = 10;
const float COIN_PICKUP_RADIUS = 22.0f;  // the mouse collects a coin within this many pixels
const float COIN_RADIUS = 11.0f;         // drawn size of a dropped coin; a player character touching it collects it
const float MESSAGE_TIME = 1.5f;         // how long a placement message ("Not enough coins") stays

// ===================== Values FIXED by the PDF (do not change) =====================
const int MELEE_HEALTH = 5;        // "5 lives"
const int RANGED_HEALTH = 5;       // "5 lives"
const int DEFENSE_HEALTH = 7;      // "7 hits"
const int EASY_HEALTH = 5;
const int MEDIUM_HEALTH = 5;
const int HARD_HEALTH = 7;
const int PLAYER_DAMAGE = 1;       // melee and ranged players: 1 point per attack
const int ENEMY_DAMAGE = 1;        // easy and medium enemies
const int HARD_ENEMY_DAMAGE = 2;   // hard melee enemy

// ===================== Tuning (team table; adjust by playing) =====================
// Team values are in "screen widths per second"; multiplied by SCREEN_WIDTH to get pixels per second.
const float EASY_SPEED = 0.04f * SCREEN_WIDTH;
const float MEDIUM_SPEED = 0.05f * SCREEN_WIDTH;
const float HARD_SPEED = 0.06f * SCREEN_WIDTH;           // "runs"
const float ENEMY_ATTACK_INTERVAL = 3.0f;                // seconds between enemy attacks
const float PROJECTILE_SPEED = 0.45f * SCREEN_WIDTH;
const float JUMP_DURATION = 0.6f;
const float DUCK_DURATION = 0.6f;
// More tuning values (adjust by playing):
const float PLAYER_SPEED = 0.07f * SCREEN_WIDTH;         // walking speed of melee and ranged players
const float MELEE_ATTACK_INTERVAL = 0.7f;                // must stay longer than MELEE_SWING_TIME (game.cpp)
const float RANGED_ATTACK_INTERVAL = 1.0f;
const float SPAWN_INTERVAL = 6.5f;                       // seconds between enemy spawns inside a wave
const float WAVE_BREAK = 5.0f;                           // pause (with banner) before the next wave
const float JUMP_HEIGHT = 60.0f;                         // pixels at the top of a jump
