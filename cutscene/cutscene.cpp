#include "cutscene/cutscene.h"
#include "config.h"
#include "renderer/renderer.h"
#include "players/players.h"
#include "enemies/enemies.h"
#include "environment/environment.h"
#include "ui/text_labels.h"
#include <algorithm>
#include <cmath>

static float t = 0;   // seconds since the cutscene started

void startCutscene() { t = 0; }
bool updateCutscene(float dt) { t += dt; return t >= CUTSCENE_LENGTH; }
float cutsceneTime() { return t; }

// ---------- small timing helpers ----------
static float clamp01(float v) { return std::max(0.0f, std::min(1.0f, v)); }
static float progress(float start, float end) { return clamp01((t - start) / (end - start)); }   // 0 before, 1 after
static float smooth(float u) { return u * u * (3 - 2 * u); }
static float lerp(float a, float b, float u) { return a + (b - a) * u; }
static bool between(float start, float end) { return t >= start && t < end; }

const float STAGE_GROUND = 150;     // feet of everyone on the stage (the second street lane)
const float STAGE_SCALE = 1.5f;      // characters are shown larger than in the game

// Puts a character on the stage: (x, y) = feet; enemies face left; squashY < 1 = ducking; tint = red flash when hit.
// Draw the character right after this call, then call endStageCharacter().
static void beginStageCharacter(float x, float y, bool facesLeft, float squashY, float tint) {
    drawEllipse(x, STAGE_GROUND, 16 * STAGE_SCALE, 4 * STAGE_SCALE, 0, 0, 0, 0.3f);    // shadow
    pushMatrix();
    translate(x, y);
    scale(facesLeft ? -STAGE_SCALE : STAGE_SCALE, STAGE_SCALE * squashY);
    setDamageTint(tint);
}

static void endStageCharacter() {
    setDamageTint(0);
    popMatrix();
}

// A subtitle in the bottom letterbox bar, fading in and out
static void subtitle(const char* id, float start, float end) {
    if (!between(start, end)) return;
    float a = std::min(clamp01((t - start) / 0.25f), clamp01((end - t) / 0.25f));
    TextLine(TC_L).label(id).drawCentered(SCREEN_WIDTH / 2.0f, 18, 1.0f, 0.95f, 0.85f, a);
}

void drawCutscene() {
    drawEnvironment(t);
    drawLaboratory(t);

    // 0-3.6 s: what the heroes defend - the last anab bottle on the lab roof glows
    if (t < 3.6f) {
        float pulse = 0.5f + 0.5f * sin(t * 6.0f);
        float a = std::min(1.0f, (3.6f - t) / 0.4f);
        drawArc(80, 612, 34 + 6 * pulse, 0, 360, 4, 1.0f, 0.85f, 0.30f, a);
        drawArc(80, 612, 46 + 8 * pulse, 0, 360, 2, 1.0f, 0.85f, 0.30f, 0.5f * a);
    }

    // Dr. Beram stands by the lab door the whole time (aims his gun when the villains advance)
    beginStageCharacter(250, STAGE_GROUND, false, 1, 0);
    drawBeram(t, false, between(13.2f, 13.5f) ? 1.0f : 0.0f);
    endStageCharacter();

    // Sareeh: walks in 7-10 s, advances at the end
    float sareehX = lerp(1360, 1110, smooth(progress(7.0f, 10.0f)));
    sareehX = lerp(sareehX, 1020, smooth(progress(13.0f, 15.5f)));
    bool sareehMoving = between(7.0f, 10.0f) || between(13.0f, 15.5f);
    beginStageCharacter(sareehX, STAGE_GROUND, true, 1, 0);
    drawSareeh(t, sareehMoving, -1);
    endStageCharacter();

    // Abla Thalma: charges in 6.2-7.3 s, swings her slipper while threatening, advances at the end
    float thalmaX = lerp(1360, 960, progress(6.2f, 7.3f));
    thalmaX = lerp(thalmaX, 860, smooth(progress(13.0f, 15.0f)));
    bool thalmaMoving = between(6.2f, 7.3f) || between(13.0f, 15.0f);
    float swing = (t > 7.3f && t < 12.0f) ? fmod(t - 7.3f, 1.2f) / 0.4f : -1.0f;   // a swing every 1.2 s
    if (swing > 1) swing = -1;
    beginStageCharacter(thalmaX, STAGE_GROUND, true, 1, 0);
    drawThalma(t, thalmaMoving, swing);
    endStageCharacter();

    // Spider-Man: walks in 3.3-5.3 s, throws a web at 10.0 s, is hit and knocked back at 11.3 s, advances at the end
    float spiderX = lerp(1360, 820, smooth(progress(3.3f, 5.3f)));
    spiderX = lerp(spiderX, 880, smooth(progress(11.3f, 11.9f)));
    spiderX = lerp(spiderX, 760, smooth(progress(13.0f, 15.0f)));
    bool spiderMoving = between(3.3f, 5.3f) || between(13.0f, 15.0f);
    float throwAnim = between(9.7f, 10.1f) ? (t - 9.7f) / 0.4f : -1.0f;
    float spiderTint = between(11.3f, 11.7f) ? 0.7f : 0.0f;
    beginStageCharacter(spiderX, STAGE_GROUND, true, 1, spiderTint);
    drawSpiderMan(t, spiderMoving, throwAnim);
    endStageCharacter();

    // Anab Man: comes out of the lab 8-9.5 s, ducks under the web, then strikes Spider-Man
    float anabX = lerp(175, 430, smooth(progress(8.0f, 9.5f)));
    bool anabMoving = between(8.0f, 9.5f);
    bool ducking = between(10.35f, 10.95f);
    float strike = between(11.0f, 11.5f) ? (t - 11.0f) / 0.5f : (between(13.6f, 14.1f) ? (t - 13.6f) / 0.5f : -1.0f);
    if (t >= 8.0f) {
        beginStageCharacter(anabX, STAGE_GROUND, false, ducking ? 0.55f : 1.0f, 0);
        drawAnabMan(t, anabMoving, strike);
        endStageCharacter();
    }

    // The web: flies from Spider-Man's hands over the ducking Anab Man and splats on the lab wall
    if (between(10.0f, 10.95f)) {
        float webX = 820 - 36 - (t - 10.0f) * 700;
        float webY = STAGE_GROUND + 75 - std::min(25.0f, (t - 10.0f) * 60);
        pushMatrix(); translate(std::max(webX, 175.0f), webY); scale(1.4f, 1.4f); drawWebBall(t); popMatrix();
    } else if (between(10.95f, 11.4f)) {
        float u = (t - 10.95f) / 0.45f;
        drawArc(175, STAGE_GROUND + 50, 10 + 20 * u, 0, 360, 3, 0.9f, 0.9f, 0.9f, 1 - u);
    }
    if (between(11.25f, 11.6f)) {                                       // the hit
        float u = (t - 11.25f) / 0.35f;
        drawArc(spiderX - 20, STAGE_GROUND + 70, 10 + 25 * u, 0, 360, 4, 1.0f, 0.85f, 0.3f, 1 - u);
    }

    // Cinematic bars, subtitles and the title
    drawRectangle(0, 0, SCREEN_WIDTH, 62, 0, 0, 0, 0.92f);
    drawRectangle(0, SCREEN_HEIGHT - 50, SCREEN_WIDTH, 50, 0, 0, 0, 0.92f);
    TextLine(TC_S).label("cs_skip").drawLeft(16, SCREEN_HEIGHT - 34, 0.75f, 0.75f, 0.75f);
    subtitle("cs_beram", 0.4f, 3.4f);
    subtitle("cs_spider", 3.6f, 6.0f);
    subtitle("cs_thalma", 6.2f, 8.2f);
    subtitle("cs_sareeh", 8.4f, 10.0f);
    subtitle("cs_anab", 10.2f, 12.8f);
    if (t > 13.0f) {
        float a = clamp01((t - 13.0f) / 0.6f);
        TextLine(TC_XXL).label("cs_title").drawCentered(SCREEN_WIDTH / 2.0f, 420, 1.0f, 0.85f, 0.30f, a);
    }
    if (t > 15.2f) drawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0, 0, clamp01((t - 15.2f) / 0.8f));   // fade out
}
