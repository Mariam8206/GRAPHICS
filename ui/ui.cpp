#include "ui/ui.h"
#include "config.h"
#include "renderer/renderer.h"
#include "players/players.h"
#include "game/collectibles/coin.h"
#include "game/game.h"
#include "ui/text_labels.h"
#include <cmath>

// Cards sit over the three 4-floor buildings (x 180-345, 500-680, 830-1005), which have no roof props,
// so hanging below the bar hides nothing important.
static const float CARD_X[PLAYER_TYPE_COUNT] = { 182, 512, 838 };
static const float CARD_Y = SCREEN_HEIGHT - CARD_HEIGHT - 2;   // bottom edge of every card
static const char* CARD_NAME_ID[PLAYER_TYPE_COUNT] = { "name_melee", "name_ranged", "name_defense" };
static const int CARD_HEALTH[PLAYER_TYPE_COUNT] = { MELEE_HEALTH, RANGED_HEALTH, DEFENSE_HEALTH };

// Layout inside a card, measured from the card's bottom-left corner (x, y).
// Portrait panel on the left: sky with a strip of street at its bottom, the character standing on it.
const float PORTRAIT_LEFT = 4, PORTRAIT_BOTTOM = 22, PORTRAIT_WIDTH = 54, PORTRAIT_HEIGHT = CARD_HEIGHT - 26;
const float STREET_STRIP_HEIGHT = 10;
// Text column on the right, right-aligned 6 px from the card's right edge. Rows (bottom of each line):
const float TEXT_MARGIN_RIGHT = 6;
const float NAME_ROW = 73;        // character name
const float STATS_ROW = 57;       // "HP n" and, left of it, "DMG n" or "Blocks"
const float COIN_ICON_ROW = 48;   // center of the small coin next to the price
const float PRICE_ROW = 37;       // price
const float FIELD_ROW = 21;       // "On field n"
const float STATUS_ROW = 3;       // status strip along the bottom edge
const float STATUS_HEIGHT = 17;

int getCardAt(float x, float y) {
    for (int i = 0; i < PLAYER_TYPE_COUNT; i++)
        if (x >= CARD_X[i] && x <= CARD_X[i] + CARD_WIDTH && y >= CARD_Y && y <= CARD_Y + CARD_HEIGHT)
            return i;
    return -1;
}

static void drawPadlock(float x, float y) {
    drawArc(x, y + 8, 7, 0, 180, 3, 0.75f, 0.75f, 0.75f);                    // shackle
    drawRectangle(x - 10, y - 6, 20, 15, 0.55f, 0.55f, 0.55f);               // body
    drawCircle(x, y + 2, 2.5f, 0.15f, 0.15f, 0.15f);                         // keyhole
    drawRectangle(x - 1, y - 3, 2, 4, 0.15f, 0.15f, 0.15f);
}

// Portrait: a small version of the real model, idling, clipped to its panel (Beram's ray gun would cover the text).
// A card that cannot be used gets a dark overlay; a locked one also gets a padlock.
static void drawCardPortrait(PlayerType type, float x, float y, bool locked, bool available, float time) {
    float left = x + PORTRAIT_LEFT, bottom = y + PORTRAIT_BOTTOM;
    float centerX = left + PORTRAIT_WIDTH / 2;
    drawRectangle(left, bottom, PORTRAIT_WIDTH, PORTRAIT_HEIGHT, 0.72f, 0.85f, 0.93f);       // sky
    drawRectangle(left, bottom, PORTRAIT_WIDTH, STREET_STRIP_HEIGHT, 0.40f, 0.40f, 0.42f);   // street

    setClipRect(left, bottom, PORTRAIT_WIDTH, PORTRAIT_HEIGHT);
    pushMatrix();
    translate(centerX, y + 28);                                                              // feet on the street
    scale(0.78f, 0.78f);
    if (type == PLAYER_MELEE) drawAnabMan(time, false, -1);
    else if (type == PLAYER_RANGED) drawBeram(time, false, 0);
    else drawTayseer(time, 0);
    popMatrix();
    clearClipRect();

    if (!available) drawRectangle(left, bottom, PORTRAIT_WIDTH, PORTRAIT_HEIGHT, 0.0f, 0.0f, 0.0f, 0.35f);
    if (locked) drawPadlock(centerX, y + 52);
}

// Text column, right-aligned: name, stats, price (red when it cannot be paid) and how many are on the field.
// A card that cannot be used has dimmer text.
static void drawCardText(PlayerType type, float x, float y, bool locked, bool affordable, bool available) {
    float dim = available ? 1.0f : 0.6f;
    float right = x + CARD_WIDTH - TEXT_MARGIN_RIGHT;

    TextLine(TC_N).label(CARD_NAME_ID[type]).drawRight(right, y + NAME_ROW, 1.0f * dim, 0.92f * dim, 0.70f * dim);

    TextLine hpLine(TC_S);
    hpLine.label("hp").number(CARD_HEALTH[type]);
    hpLine.drawRight(right, y + STATS_ROW, 0.55f * dim, 1.0f * dim, 0.55f * dim);
    float roleRight = right - hpLine.width() - 10;                     // left of the HP text
    if (type == PLAYER_DEFENSE)
        TextLine(TC_S).label("blocks").drawRight(roleRight, y + STATS_ROW, 0.6f * dim, 0.8f * dim, 1.0f * dim);
    else
        TextLine(TC_S).label("dmg").number(PLAYER_DAMAGE)
            .drawRight(roleRight, y + STATS_ROW, 1.0f * dim, 0.6f * dim, 0.6f * dim);

    pushMatrix();
    translate(right - 7, y + COIN_ICON_ROW);
    drawAnabCoin(7);
    popMatrix();
    TextLine price(TC_N);
    price.number(getPrice(type));
    if (affordable || locked) price.drawRight(right - 17, y + PRICE_ROW, 1.0f * dim, 0.88f * dim, 0.35f * dim);
    else                      price.drawRight(right - 17, y + PRICE_ROW, 1.0f, 0.25f, 0.25f);   // red: not enough coins

    TextLine(TC_S).label("field").number(countOnField(type))
        .drawRight(right, y + FIELD_ROW, 0.85f * dim, 0.85f * dim, 0.85f * dim);
}

// Status strip at the bottom: the one most important reason this card can or cannot be used
static void drawCardStatus(PlayerType type, float x, float y, bool locked, bool affordable) {
    TextLine status(TC_S);
    float r, g, b;
    if (locked && isWaitingForFirstMelee()) { status.label("st_meleefirst"); r = 0.8f; g = 0.8f; b = 0.8f; }
    else if (locked)      { status.label("st_unlocks").number(getUnlockThreshold(type)); r = 0.8f; g = 0.8f; b = 0.8f; }
    else if (!affordable) { status.label("st_need").number(getPrice(type)); r = 1.0f; g = 0.35f; b = 0.35f; }
    else                  { status.label("st_ready"); r = 0.45f; g = 1.0f; b = 0.5f; }
    drawRectangle(x + 4, y + STATUS_ROW, CARD_WIDTH - 8, STATUS_HEIGHT, 0.12f, 0.04f, 0.08f);   // 4 px in on each side
    status.drawCentered(x + CARD_WIDTH / 2, y + STATUS_ROW, r, g, b, 1, false);
}

// Border: hovered (usable cards only) = brighter
static void drawCardBorder(float x, float y, bool locked, bool hovered, bool available) {
    float w = CARD_WIDTH, h = CARD_HEIGHT;
    if (hovered && available) {
        drawRectangle(x, y, w, h, 1.0f, 1.0f, 1.0f, 0.12f);
        drawRectangleOutline(x, y, w, h, 2, 1.0f, 0.95f, 0.75f);
    } else {
        drawRectangleOutline(x, y, w, h, 2, 0.95f, 0.75f, 0.25f, locked ? 0.5f : 1.0f);
    }
}

static void drawCard(PlayerType type, bool hovered, float time) {
    float x = CARD_X[type], y = CARD_Y;
    bool locked = !isUnlocked(type);
    bool affordable = getCoins() >= getPrice(type);
    bool available = !locked && affordable;          // can be bought right now

    if (locked) setGrayscale(1);                     // the whole card loses its color
    drawRectangle(x - 2, y - 2, CARD_WIDTH + 4, CARD_HEIGHT + 4, 0.08f, 0.02f, 0.05f);   // dark frame
    drawRectangle(x, y, CARD_WIDTH, CARD_HEIGHT, 0.32f, 0.12f, 0.20f);                   // background
    drawCardPortrait(type, x, y, locked, available, time);
    drawCardText(type, x, y, locked, affordable, available);
    drawCardStatus(type, x, y, locked, affordable);
    drawCardBorder(x, y, locked, hovered, available);
    setGrayscale(0);
}

void drawTopBar(int coins, int wave, int hoveredCard, float time) {
    float barHeight = SCREEN_HEIGHT - UI_BAR_BOTTOM;
    drawRectangle(0, UI_BAR_BOTTOM, SCREEN_WIDTH, barHeight, 0.20f, 0.06f, 0.12f);   // el-anab maroon bar
    drawRectangle(0, UI_BAR_BOTTOM, SCREEN_WIDTH, 3, 0.95f, 0.75f, 0.25f);           // gold edge

    // Coin counter (top-left)
    pushMatrix();
    translate(40, UI_BAR_BOTTOM + barHeight / 2);
    drawAnabCoin(18);
    popMatrix();
    TextLine(TC_M).label("coins_label").drawLeft(66, UI_BAR_BOTTOM + 45, 0.95f, 0.85f, 0.60f);
    TextLine(TC_XL).number(coins).drawLeft(66, UI_BAR_BOTTOM + 13, 1.0f, 0.90f, 0.40f);

    for (int i = 0; i < PLAYER_TYPE_COUNT; i++)
        drawCard((PlayerType)i, i == hoveredCard, time);

    // Wave label (top-right): "WAVE 1 OF 3"
    TextLine(TC_XL).label("wave_label").number(wave).label("of").number(TOTAL_WAVES)
        .drawRight(SCREEN_WIDTH - 20, UI_BAR_BOTTOM + 19, 1.0f, 1.0f, 1.0f);
}
