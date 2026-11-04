#include "game.h"
#include "config.h"
#include "renderer.h"
#include "characters.h"
#include "lanes.h"
#include "text_labels.h"
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <string>

enum EnemyType { ENEMY_EASY, ENEMY_MEDIUM, ENEMY_HARD };
enum ProjectileKind { SHOT_ANAB_ENERGY, SHOT_WEB, SHOT_BALLOT_BOX };

const float MELEE_SWING_TIME = 0.4f;    // length of one swing animation (seconds)
const float FIRE_ANIM_TIME = 0.25f;     // length of the firing/recoil animation
const float HIT_FLASH_TIME = 0.15f;     // short extra red flash on every hit
const float MELEE_REACH = 12.0f;        // how far (pixels) past the body a close-range hit reaches
const float ENEMY_QUEUE_GAP = 6.0f;     // space kept between two enemies queued in the same lane
const float ENEMY_ENTRY_ZONE = 42.0f;   // widest enemy body (Thalma, 40 px) + the 2 px spawn margin

// ===================== Data =====================
struct Player {
    PlayerType type;
    int lane;                  // never changes after placement (PDF: no lane switching)
    float x;                   // center x in pixels
    int health, maxHealth;
    float attackTimer;         // counts down to the next attack
    float attackAnim;          // > 0 while an attack animation is playing
    bool damageDone;           // the current swing already hit (so one swing = one hit)
    float jumpTimer, duckTimer;
    float hitFlash;
    float walkTime;
    bool moving;
};

struct Enemy {
    EnemyType type;
    int lane;                  // chosen at random when it spawns, never changes
    float x;
    int health, maxHealth;
    float attackTimer;
    float attackAnim;
    bool damageDone;
    float hitFlash;
    float walkTime;
    bool moving;
};

struct Projectile {
    ProjectileKind kind;
    int lane;
    float x;
    float height;              // above the lane's ground: decides who can dodge it
    float speed;               // pixels/second; positive = to the right (player shots)
    float distance;            // how far it has flown
    int damage;
    bool fromPlayer;
    bool alive;
};

struct HitEffect {             // small expanding ring where something got hit
    float x, y, timer;
    float r, g, b;
};

struct CoinPickup {            // dropped where an enemy died; removed the moment it is collected
    float x, y;
    int value;
    float age;
};

struct FloatingText {          // "+10" rising from a collected coin
    float x, y, timer;
    std::string text;
};

static std::vector<Player> players;
static std::vector<Enemy> enemies;
static std::vector<Projectile> projectiles;
static std::vector<HitEffect> effects;
static std::vector<CoinPickup> coinPickups;
static std::vector<FloatingText> floatingTexts;

// Economy and placement (values in config.h). A card is unlocked exactly while the balance is at least its
// unlock threshold (PDF): it locks again as soon as a purchase takes the balance below it.
static const int PRICE[PLAYER_TYPE_COUNT] = { MELEE_PRICE, RANGED_PRICE, DEFENSE_PRICE };
static const int UNLOCK_AT[PLAYER_TYPE_COUNT] = { MELEE_UNLOCK_AT, RANGED_UNLOCK_AT, DEFENSE_UNLOCK_AT };
static const int FIELD_LIMIT[PLAYER_TYPE_COUNT] = { MELEE_FIELD_LIMIT, RANGED_FIELD_LIMIT, DEFENSE_FIELD_LIMIT };
static const char* PLAYER_NAME[PLAYER_TYPE_COUNT] = { "ANAB MAN", "BERAM", "TAYSEER" };
static int coins = STARTING_COINS;
static float cooldownLeft[PLAYER_TYPE_COUNT];
static int selectedCard = -1;

// Short message in the middle of the screen (placement refused, card unlocked)
static std::string message;                      // English text (logs, tests)
static TextLine messageLine(TC_L);                // what is drawn: text labels + numbers
static float messageTimer = 0;
static float messageR, messageG, messageB;

// Wave state
static int currentWave = 1;
static std::vector<EnemyType> spawnQueue;   // enemies of this wave that have not appeared yet
static bool waveSpawning = false;           // true from the wave's first spawn until its last spawn
static float spawnTimer = 0;
static float waveBreakTimer = -1;           // < 0 = no break running
static float bannerTimer = 0;
static bool gameWon = false, gameLost = false;

// ===================== Helpers =====================
static float playerHalfWidth(PlayerType type) { return type == PLAYER_DEFENSE ? 24.0f : 16.0f; }
static float enemyHalfWidth(EnemyType type) { return type == ENEMY_HARD ? 20.0f : 16.0f; }
static float laneGround(int lane) { return FIELD_BOTTOM + lane * LANE_HEIGHT + 14; }

// Height of the feet above the ground: a smooth arc while jumping, 0 otherwise.
static float jumpOffset(const Player& p) {
    if (p.jumpTimer <= 0) return 0;
    float progress = 1.0f - p.jumpTimer / JUMP_DURATION;   // 0 -> 1 during the jump
    return sin(PI * progress) * JUMP_HEIGHT;                // up and back down
}

static void addHitEffect(float x, float y, float r, float g, float b) {
    HitEffect e = { x, y, 0.3f, r, g, b };
    effects.push_back(e);
}

static void showMessage(const std::string& text, const TextLine& line, float r, float g, float b) {
    message = text;
    messageLine = line;
    messageTimer = MESSAGE_TIME;
    messageR = r; messageG = g; messageB = b;
}

// Every coin that comes in goes through here. When the balance climbs to a card's threshold, that card
// unlocks (see isUnlocked) and its "... unlocked!" message is shown - again after every relock.
static void addCoins(int amount) {
    int before = coins;
    coins += amount;
    for (int t = 0; t < PLAYER_TYPE_COUNT; t++)
        if (before < UNLOCK_AT[t] && coins >= UNLOCK_AT[t]) {
            static const char* UNLOCKED_ID[PLAYER_TYPE_COUNT] = { "msg_unlocked_melee", "msg_unlocked_ranged", "msg_unlocked_defense" };
            showMessage(std::string(PLAYER_NAME[t]) + " UNLOCKED!", TextLine(TC_L).label(UNLOCKED_ID[t]), 0.4f, 1.0f, 0.5f);
        }
}

static void collectAllCoins() {   // end of a wave: nothing the player missed is lost
    for (const CoinPickup& c : coinPickups) addCoins(c.value);
    coinPickups.clear();
}

static int countInLane(PlayerType type, int lane) {
    int count = 0;
    for (const Player& p : players)
        if (p.type == type && p.lane == lane) count++;
    return count;
}

static void hurtPlayer(Player& p, int damage) {
    p.health -= damage;
    p.hitFlash = HIT_FLASH_TIME;
    addHitEffect(p.x, laneGround(p.lane) + 45 + jumpOffset(p), 1.0f, 0.3f, 0.2f);
}

static void hurtEnemy(Enemy& e, int damage) {
    e.health -= damage;
    e.hitFlash = HIT_FLASH_TIME;
    addHitEffect(e.x, laneGround(e.lane) + 45, 0.95f, 0.35f, 0.65f);
}

// ===================== Waves =====================
// Approved wave table: easy / medium / hard per wave (PDF minimums: 3, 5, 10).
static const int WAVE_TABLE[TOTAL_WAVES][3] = {
    { 3, 0, 0 },
    { 3, 2, 0 },
    { 4, 3, 3 },
};

static void startWave(int wave) {
    currentWave = wave;
    spawnQueue.clear();
    for (int i = 0; i < WAVE_TABLE[wave - 1][0]; i++) spawnQueue.push_back(ENEMY_EASY);
    for (int i = 0; i < WAVE_TABLE[wave - 1][1]; i++) spawnQueue.push_back(ENEMY_MEDIUM);
    for (int i = 0; i < WAVE_TABLE[wave - 1][2]; i++) spawnQueue.push_back(ENEMY_HARD);
    for (int i = (int)spawnQueue.size() - 1; i > 0; i--)    // shuffle (Fisher-Yates): random order
        std::swap(spawnQueue[i], spawnQueue[rand() % (i + 1)]);
    waveSpawning = true;
    spawnTimer = 1.5f;
    bannerTimer = 2.5f;
}

// Runs once, the moment a wave (not the last one) is cleared: the next wave starts with empty lanes.
// Coins and unlocks are kept.
static void clearFieldForNextWave() {
    collectAllCoins();
    for (const Player& p : players)
        addHitEffect(p.x, laneGround(p.lane) + 35, 0.95f, 0.85f, 0.4f);   // they go back to the lab
    players.clear();
    projectiles.clear();
    for (int lane = 0; lane < LANE_COUNT; lane++) laneSelected[lane] = false;
    selectedCard = -1;
    for (int t = 0; t < PLAYER_TYPE_COUNT; t++) cooldownLeft[t] = 0;
}

// Returns false (nothing spawned) when every lane is crowded; the caller tries again next frame.
static bool spawnEnemy(EnemyType type) {
    // Random lane among those where no enemy has only just entered: spawning there would overlap it
    int freeLanes[LANE_COUNT], freeCount = 0;
    for (int lane = 0; lane < LANE_COUNT; lane++) {
        bool crowded = false;
        for (const Enemy& other : enemies)
            if (other.lane == lane && other.x > SCREEN_WIDTH - 120) crowded = true;
        if (!crowded) freeLanes[freeCount++] = lane;
    }
    if (freeCount == 0) return false;
    int lane = freeLanes[rand() % freeCount];
    Enemy e = {};
    e.type = type;
    e.lane = lane;
    e.x = SCREEN_WIDTH - enemyHalfWidth(type) - 2;    // appears at the right edge
    e.maxHealth = (type == ENEMY_HARD) ? HARD_HEALTH : (type == ENEMY_MEDIUM ? MEDIUM_HEALTH : EASY_HEALTH);
    e.health = e.maxHealth;
    e.attackTimer = 0.5f;                             // starts attacking right after appearing
    e.moving = true;
    enemies.push_back(e);
    return true;
}

// Removes defeated units. A defeated enemy drops its coin here, in the same step that removes it, so it can
// only drop once. The coin is kept left of the right-edge entry zone, where a waiting player can still touch it.
static void removeDefeated() {
    static const int REWARD[3] = { EASY_REWARD, MEDIUM_REWARD, HARD_REWARD };
    for (const Enemy& e : enemies)
        if (e.health <= 0) {
            addHitEffect(e.x, laneGround(e.lane) + 35, 1.0f, 0.85f, 0.3f);
            CoinPickup c = { std::min(e.x, SCREEN_WIDTH - ENEMY_ENTRY_ZONE), laneGround(e.lane) + 22, REWARD[e.type], 0 };
            coinPickups.push_back(c);
        }
    for (const Player& p : players)
        if (p.health <= 0) addHitEffect(p.x, laneGround(p.lane) + 35, 0.6f, 0.6f, 0.6f);
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(), [](const Enemy& e) { return e.health <= 0; }), enemies.end());
    players.erase(std::remove_if(players.begin(), players.end(), [](const Player& p) { return p.health <= 0; }), players.end());
}

// ===================== Public functions =====================
void resetGame() {
    players.clear();
    enemies.clear();
    projectiles.clear();
    effects.clear();
    coinPickups.clear();
    floatingTexts.clear();
    gameWon = false;
    gameLost = false;
    waveBreakTimer = -1;
    coins = STARTING_COINS;                       // 30: only Anab Man (threshold 0) is unlocked at the start
    for (int t = 0; t < PLAYER_TYPE_COUNT; t++) cooldownLeft[t] = 0;
    selectedCard = -1;
    messageTimer = 0;
    for (int lane = 0; lane < LANE_COUNT; lane++) laneSelected[lane] = false;
    startWave(1);
    spawnTimer = 2.0f;
}

// The checks, in order. Nothing is changed here.
static PlaceResult checkPlacement(PlayerType type, int lane) {
    if (gameWon || gameLost) return PLACE_GAME_OVER;
    if (!isUnlocked(type)) return PLACE_LOCKED;
    if (coins < PRICE[type]) return PLACE_NO_COINS;
    if (cooldownLeft[type] > 0) return PLACE_COOLDOWN;
    if (countOnField(type) >= FIELD_LIMIT[type]) return PLACE_LIMIT;
    if (lane < 0 || lane >= LANE_COUNT) return PLACE_BAD_LANE;
    if (type == PLAYER_DEFENSE && countInLane(type, lane) >= DEFENSE_PER_LANE_LIMIT) return PLACE_LANE_TAKEN;
    return PLACE_OK;
}

static void showRefusal(PlayerType type, PlaceResult result) {
    std::string text;
    TextLine line(TC_L);
    std::string countText = std::to_string(countOnField(type)) + "/" + std::to_string(FIELD_LIMIT[type]);
    if (result == PLACE_GAME_OVER) { text = "GAME OVER - PRESS R TO RESTART"; line.label("msg_gameover"); }
    else if (result == PLACE_LOCKED) { text = "LOCKED - UNLOCKS AT " + std::to_string(UNLOCK_AT[type]) + " COINS"; line.label("msg_locked").number(UNLOCK_AT[type]).label("msg_coins"); }
    else if (result == PLACE_NO_COINS) { text = "NOT ENOUGH COINS (NEED " + std::to_string(PRICE[type]) + ")"; line.label("msg_nocoins").number(PRICE[type]); }
    else if (result == PLACE_COOLDOWN) { text = "COOLDOWN"; line.label("msg_cooldown"); }
    else if (result == PLACE_LIMIT) { text = "LIMIT REACHED (" + countText + ")"; line.label("msg_limit").number(countText); }
    else if (result == PLACE_BAD_LANE) { text = "INVALID LANE"; line.label("msg_badlane"); }
    else if (result == PLACE_LANE_TAKEN) { text = "ONLY ONE TAYSEER PER LANE"; line.label("msg_lanetaken"); }
    showMessage(text, line, 1.0f, 0.45f, 0.4f);
}

void clickCard(PlayerType type) {
    if (gameWon || gameLost) return;
    if (selectedCard == type) { selectedCard = -1; return; }   // second click on the same card cancels
    // A locked, unaffordable or full card cannot be selected. A card that is only cooling down can:
    // the placement itself then waits for the cooldown.
    PlaceResult problem = PLACE_OK;
    if (!isUnlocked(type)) problem = PLACE_LOCKED;
    else if (coins < PRICE[type]) problem = PLACE_NO_COINS;
    else if (countOnField(type) >= FIELD_LIMIT[type]) problem = PLACE_LIMIT;
    if (problem != PLACE_OK) { showRefusal(type, problem); return; }
    selectedCard = type;
}

int getSelectedCard() { return selectedCard; }

PlaceResult tryPlacePlayer(PlayerType type, int lane) {
    PlaceResult result = checkPlacement(type, lane);
    if (result != PLACE_OK) {
        showRefusal(type, result);
        return result;                                 // nothing created, no coins taken
    }
    Player p = {};
    p.type = type;
    p.lane = lane;
    p.x = BASE_WIDTH + playerHalfWidth(type) + 4;     // leftmost position of the lane
    p.maxHealth = (type == PLAYER_MELEE) ? MELEE_HEALTH : (type == PLAYER_RANGED ? RANGED_HEALTH : DEFENSE_HEALTH);
    p.health = p.maxHealth;
    p.attackTimer = 0.3f;                             // starts attacking right after placement
    p.moving = (type != PLAYER_DEFENSE);
    players.push_back(p);
    coins -= PRICE[type];                              // paid only now that the character exists
    cooldownLeft[type] = PLACEMENT_COOLDOWN;
    selectedCard = -1;                                 // back to lane selection for jump/duck
    return PLACE_OK;
}

// The only way a coin is credited (mouse or character): it is removed in the same step, so never twice.
static void collectCoin(size_t i) {
    const CoinPickup& c = coinPickups[i];
    FloatingText t = { c.x, c.y + 10, 0.8f, "+" + std::to_string(c.value) };
    floatingTexts.push_back(t);
    addCoins(c.value);
    coinPickups.erase(coinPickups.begin() + i);
}

void collectCoinsAt(float x, float y) {
    if (gameWon || gameLost) return;                   // frozen
    for (size_t i = 0; i < coinPickups.size(); ) {
        float dx = coinPickups[i].x - x, dy = coinPickups[i].y - y;
        if (dx * dx + dy * dy <= COIN_PICKUP_RADIUS * COIN_PICKUP_RADIUS) collectCoin(i);
        else i++;
    }
}

// A player character whose body touches a coin collects it (a jumping player passes over it).
static void collectCoinsTouchedByPlayers() {
    for (size_t i = 0; i < coinPickups.size(); ) {
        const CoinPickup& c = coinPickups[i];
        bool touched = false;
        for (const Player& p : players) {
            if (p.health <= 0) continue;
            float feet = laneGround(p.lane) + jumpOffset(p);
            float top = feet + (p.duckTimer > 0 && p.type != PLAYER_DEFENSE ? 38.0f : 75.0f);
            if (fabs(c.x - p.x) <= playerHalfWidth(p.type) + COIN_RADIUS &&
                c.y + COIN_RADIUS >= feet && c.y - COIN_RADIUS <= top) touched = true;
        }
        if (touched) collectCoin(i);
        else i++;
    }
}

int getCoins() { return coins; }
int getPrice(PlayerType type) { return PRICE[type]; }
int getUnlockThreshold(PlayerType type) { return UNLOCK_AT[type]; }
int getFieldLimit(PlayerType type) { return FIELD_LIMIT[type]; }
bool isUnlocked(PlayerType type) { return coins >= UNLOCK_AT[type]; }   // relocks below the threshold (PDF)
float getCooldownLeft(PlayerType type) { return cooldownLeft[type] > 0 ? cooldownLeft[type] : 0.0f; }

int countOnField(PlayerType type) {
    int count = 0;
    for (const Player& p : players)
        if (p.type == type) count++;
    return count;
}

void jumpSelectedLanes() {
    for (Player& p : players)
        if (laneSelected[p.lane] && p.type != PLAYER_DEFENSE && p.jumpTimer <= 0) {
            p.jumpTimer = JUMP_DURATION;
            p.duckTimer = 0;
        }
}

void duckSelectedLanes() {
    for (Player& p : players)
        if (laneSelected[p.lane] && p.type != PLAYER_DEFENSE && p.jumpTimer <= 0)
            p.duckTimer = DUCK_DURATION;
}

int getCurrentWave() { return currentWave; }
bool isGameWon() { return gameWon; }
bool isGameLost() { return gameLost; }

// ===================== Update =====================
// One frame is done in steps: waves, players, enemies, projectiles, then cleanup. updateGame (at the end
// of this section) calls them in that order.

// Waves: spawn the queue one by one; the next wave starts only after the field is clear.
// Sets gameWon when the last wave has been cleared.
static void updateWaves(float dt) {
    if (waveSpawning) {
        spawnTimer -= dt;
        if (spawnTimer <= 0 && spawnEnemy(spawnQueue.back())) {   // all lanes crowded: retried every frame
            spawnQueue.pop_back();
            spawnTimer = SPAWN_INTERVAL;
            if (spawnQueue.empty()) waveSpawning = false;   // the wave's last enemy has appeared
        }
    } else if (enemies.empty()) {
        if (currentWave >= TOTAL_WAVES) { collectAllCoins(); gameWon = true; return; }
        if (waveBreakTimer < 0) {                     // the wave was cleared just now
            clearFieldForNextWave();
            waveBreakTimer = WAVE_BREAK;              // players may already place characters during the break
        }
        waveBreakTimer -= dt;
        if (waveBreakTimer <= 0) {
            waveBreakTimer = -1;
            startWave(currentWave + 1);
        }
    }
}

// Players: walk right until they meet an enemy or the right edge, and attack on their own timers.
static void updatePlayers(float dt) {
    for (Player& p : players) {
        p.walkTime += dt;
        p.hitFlash -= dt;
        p.attackAnim -= dt;
        if (p.jumpTimer > 0) p.jumpTimer -= dt;
        if (p.duckTimer > 0) p.duckTimer -= dt;

        // Find the closest enemy in front of this player, in the same lane
        float half = playerHalfWidth(p.type);
        Enemy* target = nullptr;
        float targetGap = 1e9f;
        bool touching = false;
        for (Enemy& e : enemies) {
            if (e.lane != p.lane || e.health <= 0) continue;
            float eHalf = enemyHalfWidth(e.type);
            if (e.x + eHalf < p.x - half) continue;               // enemy is already behind this player
            float gap = (e.x - eHalf) - (p.x + half);             // space between the two bodies
            if (gap <= 2) touching = true;                        // they collide
            if (gap < targetGap) { targetGap = gap; target = &e; }
        }

        // Movement: walk right until touching an enemy or reaching the right edge (then stay there).
        // The edge stop leaves room for the widest enemy to appear in front: spawning on top of a waiting player
        // made the two overlap by 30-38 px.
        float rightLimit = SCREEN_WIDTH - ENEMY_ENTRY_ZONE - half;
        p.moving = (p.type != PLAYER_DEFENSE) && !touching && (p.x < rightLimit);
        if (p.moving) p.x = std::min(p.x + PLAYER_SPEED * dt, rightLimit);

        // Attacks start immediately and repeat, even with no enemy around (PDF 3.1)
        if (p.type == PLAYER_MELEE) {
            p.attackTimer -= dt;
            if (p.attackTimer <= 0) {
                p.attackTimer = MELEE_ATTACK_INTERVAL;
                p.attackAnim = MELEE_SWING_TIME;
                p.damageDone = false;
            }
            // the hit lands in the middle of the swing, only if an enemy is within reach
            if (p.attackAnim > 0 && p.attackAnim <= MELEE_SWING_TIME / 2 && !p.damageDone) {
                p.damageDone = true;
                if (target && targetGap <= MELEE_REACH) hurtEnemy(*target, PLAYER_DAMAGE);
            }
        } else if (p.type == PLAYER_RANGED) {
            p.attackTimer -= dt;
            if (p.attackTimer <= 0) {
                p.attackTimer = RANGED_ATTACK_INTERVAL;
                p.attackAnim = FIRE_ANIM_TIME;
                Projectile shot = {};
                shot.kind = SHOT_ANAB_ENERGY;
                shot.lane = p.lane;
                shot.x = p.x + half + 10;
                shot.height = (p.duckTimer > 0 ? 25.0f : 44.0f) + jumpOffset(p);
                shot.speed = PROJECTILE_SPEED;
                shot.damage = PLAYER_DAMAGE;
                shot.fromPlayer = true;
                shot.alive = true;
                projectiles.push_back(shot);
            }
        }
        // PLAYER_DEFENSE: no attack; it blocks (see updateEnemies and updateProjectiles)
    }
}

// Enemies: queue up and walk left, attack the front-most player, and set gameLost if one reaches the lab.
static void updateEnemies(float dt) {
    for (Enemy& e : enemies) {
        if (e.health <= 0) continue;                  // killed by a player this frame: no more moves or attacks
        e.walkTime += dt;
        e.hitFlash -= dt;
        e.attackAnim -= dt;

        // Find the FRONT-most player touching this enemy. A Defense player standing in front
        // is reached first, so players behind it are protected.
        float half = enemyHalfWidth(e.type);
        Player* target = nullptr;
        bool touching = false;
        for (Player& p : players) {
            if (p.lane != e.lane || p.health <= 0) continue;
            float pHalf = playerHalfWidth(p.type);
            if (p.x - pHalf > e.x + half) continue;               // player already behind this enemy
            float gap = (e.x - half) - (p.x + pHalf);
            if (gap <= 2) touching = true;
            if (gap <= MELEE_REACH && (!target || p.x > target->x)) target = &p;
        }

        // Queue behind the closest living enemy ahead in this lane: never step closer than ENEMY_QUEUE_GAP.
        // (Same x: the one earlier in the list counts as ahead, so two can never walk merged.)
        float room = 1e9f;
        for (const Enemy& other : enemies) {
            if (&other == &e || other.lane != e.lane || other.health <= 0) continue;
            if (other.x > e.x || (other.x == e.x && &other > &e)) continue;   // behind this enemy
            float gap = (e.x - half) - (other.x + enemyHalfWidth(other.type));
            room = std::min(room, gap - ENEMY_QUEUE_GAP);
        }
        float speed = (e.type == ENEMY_HARD) ? HARD_SPEED : (e.type == ENEMY_MEDIUM ? MEDIUM_SPEED : EASY_SPEED);
        float stepX = touching ? 0.0f : std::max(0.0f, std::min(speed * dt, room));
        e.moving = stepX > 0;
        e.x -= stepX;

        // Lose: one enemy reaching the home base is enough
        if (e.x - half <= BASE_WIDTH) { gameLost = true; break; }

        e.attackTimer -= dt;
        if (e.attackTimer <= 0) {
            e.attackTimer = ENEMY_ATTACK_INTERVAL;
            e.attackAnim = MELEE_SWING_TIME;
            e.damageDone = false;
            if (e.type != ENEMY_HARD) {                             // ranged enemies shoot every interval
                Projectile shot = {};
                shot.kind = (e.type == ENEMY_EASY) ? SHOT_WEB : SHOT_BALLOT_BOX;
                shot.lane = e.lane;
                shot.x = e.x - half - 6;
                shot.height = (e.type == ENEMY_EASY) ? 95.0f : 8.0f;   // web from above / box along the ground
                shot.speed = -PROJECTILE_SPEED;
                shot.damage = ENEMY_DAMAGE;
                shot.fromPlayer = false;
                shot.alive = true;
                projectiles.push_back(shot);
            }
        }
        // Hard enemy: close-range slipper hit in the middle of its swing, 2 damage
        if (e.type == ENEMY_HARD && e.attackAnim > 0 && e.attackAnim <= MELEE_SWING_TIME / 2 && !e.damageDone) {
            e.damageDone = true;
            if (target) hurtPlayer(*target, HARD_ENEMY_DAMAGE);
        }
    }
}

// Projectiles: move, then hit the first body on the path flown this frame (from -> to), so a fast shot
// at a low frame rate cannot jump over a body between two frames.
static void updateProjectiles(float dt) {
    for (Projectile& shot : projectiles) {
        float from = shot.x;
        float step = shot.speed * dt;
        shot.x += step;
        shot.distance += fabs(step);
        float pathLeft = std::min(from, shot.x), pathRight = std::max(from, shot.x);
        if (shot.kind == SHOT_WEB)                              // comes down from above, then flies at head height
            shot.height = std::max(50.0f, 95.0f - shot.distance * 0.35f);

        if (shot.fromPlayer) {
            Enemy* hit = nullptr;                               // first enemy on its path (moving right)
            for (Enemy& e : enemies) {
                if (e.lane != shot.lane || e.health <= 0) continue;
                float eHalf = enemyHalfWidth(e.type);
                if (pathRight > e.x - eHalf && pathLeft < e.x + eHalf && (!hit || e.x - eHalf < hit->x - enemyHalfWidth(hit->type)))
                    hit = &e;
            }
            if (hit) {
                hurtEnemy(*hit, shot.damage);
                shot.alive = false;                             // disappears on collision
            }
            if (shot.x > SCREEN_WIDTH + 20) shot.alive = false;
            // player shots never check players, so they can never harm a player (PDF)
        } else {
            // Which player does it hit? The front-most one that is in the way at this height.
            Player* hit = nullptr;
            for (Player& p : players) {
                if (p.lane != shot.lane || p.health <= 0) continue;
                float pHalf = playerHalfWidth(p.type);
                if (pathRight < p.x - pHalf || pathLeft > p.x + pHalf) continue;   // not on its path
                bool inTheWay;
                if (p.type == PLAYER_DEFENSE) {
                    inTheWay = true;                            // the shield blocks high and low attacks
                } else {
                    float feet = jumpOffset(p);
                    float top = feet + (p.duckTimer > 0 ? 38.0f : 75.0f);
                    inTheWay = (shot.height + 6 >= feet) && (shot.height - 6 <= top);
                    // jumping lifts the feet over a low box; ducking lowers the head under a web
                }
                if (inTheWay && (!hit || p.x > hit->x)) hit = &p;
            }
            if (hit) {
                hurtPlayer(*hit, shot.damage);
                shot.alive = false;
            } else if (shot.x < BASE_WIDTH) {
                addHitEffect(BASE_WIDTH, laneGround(shot.lane) + shot.height, 0.8f, 0.8f, 0.8f);   // splats on the lab wall
                shot.alive = false;
            }
        }
    }
}

void updateGame(float dt) {
    messageTimer -= dt;                               // messages still fade out after the game ends
    if (gameWon || gameLost) return;                  // PDF: on a loss everything stops (we also freeze on a win)
    bannerTimer -= dt;
    for (int t = 0; t < PLAYER_TYPE_COUNT; t++) cooldownLeft[t] -= dt;

    updateWaves(dt);
    if (gameWon) return;
    updatePlayers(dt);
    updateEnemies(dt);
    // Lose: everything stops. Units defeated earlier in this frame are still removed, so none stays on the
    // loss screen with 0 health.
    if (gameLost) { removeDefeated(); return; }
    updateProjectiles(dt);
    collectCoinsTouchedByPlayers();   // before new drops, so a fresh coin is visible for at least one frame

    // Remove what is finished
    removeDefeated();
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(), [](const Projectile& s) { return !s.alive; }), projectiles.end());
    for (HitEffect& fx : effects) fx.timer -= dt;
    effects.erase(std::remove_if(effects.begin(), effects.end(), [](const HitEffect& fx) { return fx.timer <= 0; }), effects.end());
    for (CoinPickup& c : coinPickups) c.age += dt;
    for (FloatingText& t : floatingTexts) { t.timer -= dt; t.y += 40 * dt; }
    floatingTexts.erase(std::remove_if(floatingTexts.begin(), floatingTexts.end(), [](const FloatingText& t) { return t.timer <= 0; }), floatingTexts.end());
}

// ===================== Drawing =====================
// Red damage feedback: deeper red as health drops, plus a short flash on every hit.
static float damageTintFor(int health, int maxHealth, float hitFlash) {
    float tint = (1.0f - (float)health / maxHealth) * 0.6f;
    if (hitFlash > 0) tint += 0.35f;
    return tint;
}

// Segmented health bar: one segment per health point, plus the number. Players: green with a gold frame;
// enemies: red with a dark frame (so the two sides never look alike).
static void drawHealthBar(float centerX, float y, int health, int maxHealth, bool isPlayer) {
    float segment = 7, gap = 1.5f;
    float width = maxHealth * segment + (maxHealth - 1) * gap;
    float left = centerX - width / 2;
    if (isPlayer) drawRectangle(left - 3, y - 3, width + 6, 12, 0.95f, 0.75f, 0.25f);
    else          drawRectangle(left - 3, y - 3, width + 6, 12, 0.10f, 0.05f, 0.08f);
    drawRectangle(left - 1.5f, y - 1.5f, width + 3, 9, 0.15f, 0.15f, 0.15f);
    for (int i = 0; i < maxHealth; i++) {
        float sx = left + i * (segment + gap);
        if (i < health) {
            if (isPlayer) drawRectangle(sx, y, segment, 6, 0.30f, 0.85f, 0.30f);
            else          drawRectangle(sx, y, segment, 6, 0.90f, 0.20f, 0.25f);
        } else {
            drawRectangle(sx, y, segment, 6, 0.35f, 0.35f, 0.35f);
        }
    }
    TextLine(TC_S).outlined().number(health).drawLeft(left + width + 5, y - 5, 1.0f, 1.0f, 1.0f);   // centered on the bar
}

static void drawPlayerUnit(const Player& p, float time) {
    float ground = laneGround(p.lane);
    float lift = jumpOffset(p);
    drawEllipse(p.x, ground, 16 - lift * 0.1f, 4, 0, 0, 0, 0.3f);           // shadow stays on the ground
    pushMatrix();
    translate(p.x, ground + lift);
    if (p.duckTimer > 0 && p.type != PLAYER_DEFENSE) scale(1.1f, 0.55f);     // ducking = squashed down
    setDamageTint(damageTintFor(p.health, p.maxHealth, p.hitFlash));
    if (p.type == PLAYER_MELEE) {
        float attack = (p.attackAnim > 0) ? 1.0f - p.attackAnim / MELEE_SWING_TIME : -1.0f;
        drawAnabMan(p.walkTime, p.moving, attack);
    } else if (p.type == PLAYER_RANGED) {
        float recoil = (p.attackAnim > 0) ? p.attackAnim / FIRE_ANIM_TIME : 0.0f;
        drawBeram(p.walkTime, p.moving, recoil);
    } else {
        drawTayseer(p.walkTime, p.hitFlash > 0 ? p.hitFlash / HIT_FLASH_TIME : 0.0f);
    }
    setDamageTint(0);
    popMatrix();
}

static void drawEnemyUnit(const Enemy& e, float time) {
    float ground = laneGround(e.lane);
    drawEllipse(e.x, ground, 16, 4, 0, 0, 0, 0.3f);
    pushMatrix();
    translate(e.x, ground);
    scale(-1, 1);                                                             // enemies face left
    setDamageTint(damageTintFor(e.health, e.maxHealth, e.hitFlash));
    float attack = (e.attackAnim > 0) ? 1.0f - e.attackAnim / MELEE_SWING_TIME : -1.0f;
    if (e.type == ENEMY_EASY) drawSpiderMan(e.walkTime, e.moving, attack);
    else if (e.type == ENEMY_MEDIUM) drawSareeh(e.walkTime, e.moving, attack);
    else drawThalma(e.walkTime, e.moving, attack);
    setDamageTint(0);
    popMatrix();
}

// Each health bar starts above its unit's head and moves up one row (13 px) until it no longer overlaps a bar
// already placed in the same lane, so crowded fights (players AND enemies) stay readable.
struct PlacedBar { int lane; float left, right, y; };

static float findFreeBarHeight(std::vector<PlacedBar>& placedBars, int lane, float centerX, float startY, int maxHealth) {
    float halfWidth = (maxHealth * 8.5f + 22) / 2;                       // segments + frame + number
    float y = startY;
    bool moved = true;
    while (moved) {
        moved = false;
        for (const PlacedBar& b : placedBars) {
            float above = b.y + 13;
            // compare with the exact value we move to: with fabs(b.y - y) < 13, float rounding could
            // keep "overlapping" after the move and loop forever
            if (b.lane == lane && y > b.y - 13 && y < above && centerX - halfWidth < b.right && centerX + halfWidth > b.left) {
                y = above;
                moved = true;
            }
        }
    }
    PlacedBar bar = { lane, centerX - halfWidth, centerX + halfWidth, y };
    placedBars.push_back(bar);
    return y;
}

// Keeps the whole bar + number on screen (an enemy that just spawned stands at the right edge)
static float keepBarOnScreen(float x, int maxHealth) {
    float half = maxHealth * 4.25f;                                       // half the segments' width
    return std::min(std::max(x, half + 5), SCREEN_WIDTH - half - 22);     // 22: number on the right
}

void drawGame(float time) {
    // Back lanes first, so characters in lower (closer) lanes overlap the ones above them
    for (int lane = LANE_COUNT - 1; lane >= 0; lane--) {
        for (const Player& p : players) if (p.lane == lane) drawPlayerUnit(p, time);
        for (const Enemy& e : enemies) if (e.lane == lane) drawEnemyUnit(e, time);
        for (const Projectile& shot : projectiles) {
            if (shot.lane != lane) continue;
            pushMatrix();
            translate(shot.x, laneGround(lane) + shot.height);
            if (shot.kind == SHOT_ANAB_ENERGY) drawAnabEnergyBall(time);
            else if (shot.kind == SHOT_WEB) drawWebBall(time);
            else drawBallotBox(shot.distance * 2.5f);                        // rolls as it travels
            popMatrix();
        }
    }
    for (const HitEffect& fx : effects) {
        float t = fx.timer / 0.3f;                                           // 1 -> 0
        drawArc(fx.x, fx.y, 6 + (1 - t) * 14, 0, 360, 3, fx.r, fx.g, fx.b, t);
    }
    // Coin pickups: bob up and down and slowly "spin" (squeezed sideways) so they catch the eye
    for (const CoinPickup& c : coinPickups) {
        float bob = 3 * sin(c.age * 4.0f);
        drawCircle(c.x, c.y + bob, 16 + 2 * sin(c.age * 6.0f), 1.0f, 0.85f, 0.3f, 0.25f);   // glow
        pushMatrix();
        translate(c.x, c.y + bob);
        scale(0.75f + 0.25f * cos(c.age * 3.0f), 1);
        drawAnabCoin(COIN_RADIUS);
        popMatrix();
    }
    // Health bars last, so nothing covers them (PDF: health always visible).
    std::vector<PlacedBar> placedBars;
    for (const Player& p : players) {
        float x = keepBarOnScreen(p.x, p.maxHealth);
        float y = findFreeBarHeight(placedBars, p.lane, x, laneGround(p.lane) + jumpOffset(p) + 84, p.maxHealth);
        drawHealthBar(x, y, p.health, p.maxHealth, true);
    }
    for (const Enemy& e : enemies) {
        float x = keepBarOnScreen(e.x, e.maxHealth);
        float y = findFreeBarHeight(placedBars, e.lane, x, laneGround(e.lane) + 97, e.maxHealth);
        drawHealthBar(x, y, e.health, e.maxHealth, false);
    }

    for (const FloatingText& t : floatingTexts)
        TextLine(TC_L).outlined().number(t.text).drawCentered(t.x, t.y - 25, 1.0f, 0.85f, 0.25f, t.timer / 0.8f);

    // Placement / unlock message, on a dark strip just below the cards
    if (messageTimer > 0) {
        float alpha = messageTimer < 0.3f ? messageTimer / 0.3f : 1.0f;   // fades out at the end
        float width = messageLine.width();
        drawRectangle((SCREEN_WIDTH - width) / 2 - 14, 576, width + 28, 36, 0.05f, 0.03f, 0.05f, 0.75f * alpha);
        messageLine.drawCentered(SCREEN_WIDTH / 2.0f, 578, messageR, messageG, messageB, alpha);
    }

    // Wave banners (the win/lose banner is part of the win/lose menu, screens.cpp)
    float center = SCREEN_WIDTH / 2.0f;
    if (gameWon || gameLost) {
        // nothing here
    } else if (bannerTimer > 0) {
        TextLine(TC_XXL).label("ban_wave").number(currentWave).drawCentered(center, 290, 1.0f, 0.85f, 0.3f);
    } else if (waveBreakTimer > 0) {
        TextLine(TC_XXL).label("ban_cleared").drawCentered(center, 290, 0.5f, 1.0f, 0.6f);
    }
}
