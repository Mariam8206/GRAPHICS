#include "ui/ui.h"
#include "config.h"
#include "renderer/renderer.h"
#include "players/players.h"
#include "game/collectibles/coin.h"
#include "game/game.h"
#include "ui/text_labels.h"
#include <string>
#include <cstdio>
#include <cmath>

// Cards sit over the three 4-floor buildings (x 180-345, 500-680, 830-1005), which have no roof props,
// so hanging below the bar hides nothing important.
static const float CARD_X[PLAYER_TYPE_COUNT] = { 182, 512, 838 };
static const float CARD_Y = SCREEN_HEIGHT - CARD_HEIGHT - 2;   // bottom edge of every card
static const char* CARD_NAME_ID[PLAYER_TYPE_COUNT] = { "name_melee", "name_ranged", "name_defense" };

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

static void drawCard(int i, bool hovered, float time) {
    PlayerType type = (PlayerType)i;
    float x = CARD_X[i], y = CARD_Y, w = CARD_WIDTH, h = CARD_HEIGHT;
    bool locked = !isUnlocked(type);
    bool affordable = getCoins() >= getPrice(type);
    int count = countOnField(type), limit = getFieldLimit(type);
    bool full = count >= limit;
    bool selected = (getSelectedCard() == i);
    bool available = !locked && affordable && !full;
    float textDim = available ? 1.0f : 0.6f;

    if (locked) setGrayscale(1);                                             // the whole card loses its color

    // Frame, background and portrait panel (sky + street)
    drawRectangle(x - 2, y - 2, w + 4, h + 4, 0.08f, 0.02f, 0.05f);
    drawRectangle(x, y, w, h, 0.32f, 0.12f, 0.20f);
    drawRectangle(x + 4, y + 22, 54, h - 26, 0.72f, 0.85f, 0.93f);
    drawRectangle(x + 4, y + 22, 54, 10, 0.40f, 0.40f, 0.42f);

    // Small version of the real model, idling (clipped to its panel: Beram's ray gun would cover the text)
    setClipRect(x + 4, y + 22, 54, h - 26);
    pushMatrix();
    translate(x + 31, y + 28);
    scale(0.78f, 0.78f);
    if (type == PLAYER_MELEE) drawAnabMan(time, false, -1);
    else if (type == PLAYER_RANGED) drawBeram(time, false, 0);
    else drawTayseer(time, 0);
    popMatrix();
    clearClipRect();
    if (!available) drawRectangle(x + 4, y + 22, 54, h - 26, 0.0f, 0.0f, 0.0f, 0.35f);
    if (locked) drawPadlock(x + 31, y + 52);

    // Text column right of the portrait, right-aligned:
    // name, health + damage (or role), price, how many are on the field
    float right = x + w - 6;
    TextLine(TC_N).label(CARD_NAME_ID[i]).drawRight(right, y + 73, 1.0f * textDim, 0.92f * textDim, 0.70f * textDim);
    TextLine hpLine(TC_S);
    hpLine.label("hp").number(i == PLAYER_DEFENSE ? DEFENSE_HEALTH : (i == PLAYER_RANGED ? RANGED_HEALTH : MELEE_HEALTH));
    hpLine.drawRight(right, y + 57, 0.55f * textDim, 1.0f * textDim, 0.55f * textDim);
    float roleRight = right - hpLine.width() - 10;
    if (type == PLAYER_DEFENSE) TextLine(TC_S).label("blocks").drawRight(roleRight, y + 57, 0.6f * textDim, 0.8f * textDim, 1.0f * textDim);
    else TextLine(TC_S).label("dmg").number(PLAYER_DAMAGE).drawRight(roleRight, y + 57, 1.0f * textDim, 0.6f * textDim, 0.6f * textDim);

    pushMatrix();
    translate(right - 7, y + 48);
    drawAnabCoin(7);
    popMatrix();
    TextLine price(TC_N);
    price.number(getPrice(type));
    if (affordable || locked) price.drawRight(right - 17, y + 37, 1.0f * textDim, 0.88f * textDim, 0.35f * textDim);
    else                      price.drawRight(right - 17, y + 37, 1.0f, 0.25f, 0.25f);   // red: not enough coins

    std::string countText = std::to_string(count) + "/" + std::to_string(limit);
    TextLine(TC_S).label("field").number(countText).drawRight(right, y + 21,
        full ? 1.0f : 0.85f * textDim, full ? 0.55f : 0.85f * textDim, full ? 0.2f : 0.85f * textDim);

    // Status strip at the bottom: the one most important reason this card can or cannot be used
    TextLine status(TC_S);
    float sr, sg, sb;
    if (locked && isWaitingForFirstMelee()) { status.label("st_meleefirst"); sr = 0.8f; sg = 0.8f; sb = 0.8f; }
    else if (locked)          { status.label("st_unlocks").number(getUnlockThreshold(type)); sr = 0.8f; sg = 0.8f; sb = 0.8f; }
    else if (full)            { status.label("st_limit").number(countText); sr = 1.0f; sg = 0.55f; sb = 0.2f; }
    else if (!affordable)     { status.label("st_need").number(getPrice(type)); sr = 1.0f; sg = 0.35f; sb = 0.35f; }
    else if (selected)        { status.label("st_click"); sr = 1.0f; sg = 0.85f; sb = 0.3f; }
    else                      { status.label("st_ready"); sr = 0.45f; sg = 1.0f; sb = 0.5f; }
    drawRectangle(x + 4, y + 3, w - 8, 17, 0.12f, 0.04f, 0.08f);
    status.drawCentered(x + w / 2, y + 3, sr, sg, sb, 1, false);

    // Border: hovered (available cards only) = brighter; selected = thick pulsing gold
    if (hovered && available && !selected) {
        drawRectangle(x, y, w, h, 1.0f, 1.0f, 1.0f, 0.12f);
        drawRectangleOutline(x, y, w, h, 2, 1.0f, 0.95f, 0.75f);
    } else {
        drawRectangleOutline(x, y, w, h, 2, 0.95f, 0.75f, 0.25f, locked ? 0.5f : 1.0f);
    }
    if (selected) {
        float pulse = 0.5f + 0.5f * sin(time * 6.0f);
        drawRectangleOutline(x, y, w, h, 4, 1.0f, 0.82f, 0.15f);
        drawRectangleOutline(x - 5, y - 5, w + 10, h + 10, 3, 1.0f, 0.85f, 0.2f, 0.35f + 0.4f * pulse);
    }
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
        drawCard(i, i == hoveredCard, time);

    // Wave label (top-right): "WAVE 1 OF 3"
    TextLine(TC_XL).label("wave_label").number(wave).label("of").number(TOTAL_WAVES)
        .drawRight(SCREEN_WIDTH - 20, UI_BAR_BOTTOM + 19, 1.0f, 1.0f, 1.0f);
}
