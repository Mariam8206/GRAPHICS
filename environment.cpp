#include "environment.h"
#include "config.h"
#include "renderer.h"
#include "text_labels.h"
#include <cmath>

// Vertical layout (pixels):
//   0 .. 30     front sidewalk
//   30 .. 470   the 4 street lanes (FIELD_BOTTOM .. FIELD_TOP)
//   470 .. 488  back sidewalk, where buildings, streetlights and the kiosk stand
//   488 .. 650  building facades and sky; the UI bar covers 650 .. 720
const float BACK_SIDEWALK_TOP = 488.0f;
const float FLOOR_HEIGHT = 36.0f;

// ===================== Small props =====================

// Black plastic water tank on two metal legs (very common on Egyptian roofs).
static void drawWaterTank(float x, float roofY) {
    drawLine(x - 9, roofY, x - 7, roofY + 10, 2, 0.35f, 0.35f, 0.38f);           // legs
    drawLine(x + 9, roofY, x + 7, roofY + 10, 2, 0.35f, 0.35f, 0.38f);
    drawRectangle(x - 12, roofY + 10, 24, 18, 0.12f, 0.12f, 0.14f);              // tank body
    drawEllipse(x, roofY + 28, 12, 3, 0.22f, 0.22f, 0.25f);                      // round lid seen from the side
    drawRectangle(x - 12, roofY + 15, 24, 2, 0.25f, 0.25f, 0.28f);               // ribs
    drawRectangle(x - 12, roofY + 21, 24, 2, 0.25f, 0.25f, 0.28f);
    drawLine(x + 12, roofY + 14, x + 18, roofY, 1.5f, 0.6f, 0.6f, 0.6f);        // pipe down to the roof
}

// Satellite dish: pole + tilted dish + receiver arm.
static void drawSatelliteDish(float x, float roofY) {
    drawRectangle(x - 1.5f, roofY, 3, 14, 0.5f, 0.5f, 0.52f);                    // pole
    pushMatrix();
    translate(x, roofY + 16);
    rotate(35);                                                                  // dish faces up-right
    drawEllipse(0, 0, 4, 12, 0.85f, 0.85f, 0.88f);                               // dish
    drawEllipse(-1, 0, 2.5f, 9, 0.70f, 0.70f, 0.74f);                            // inner shading
    drawLine(0, 0, 12, 0, 1.2f, 0.4f, 0.4f, 0.42f);                              // receiver arm
    drawCircle(12, 0, 2, 0.2f, 0.2f, 0.2f);
    popMatrix();
}

// One window with wooden shutters (and an optional air-conditioner box below it).
static void drawWindow(float x, float y, bool hasAC) {
    drawRectangle(x - 2, y - 2, 22, 26, 0.95f, 0.92f, 0.85f);                    // frame
    drawRectangle(x, y, 18, 22, 0.25f, 0.35f, 0.45f);                            // glass
    drawRectangle(x + 8, y, 2, 22, 0.95f, 0.92f, 0.85f);                         // middle bar
    drawRectangle(x - 7, y, 5, 22, 0.45f, 0.30f, 0.18f);                         // left shutter
    drawRectangle(x + 20, y, 5, 22, 0.45f, 0.30f, 0.18f);                        // right shutter
    for (int i = 0; i < 5; i++) {                                                // shutter slats
        drawRectangle(x - 7, y + 3 + i * 4, 5, 1, 0.35f, 0.22f, 0.12f);
        drawRectangle(x + 20, y + 3 + i * 4, 5, 1, 0.35f, 0.22f, 0.12f);
    }
    if (hasAC) {
        drawRectangle(x + 1, y - 12, 16, 9, 0.88f, 0.88f, 0.86f);                // AC box
        drawCircle(x + 6, y - 7.5f, 3, 0.55f, 0.55f, 0.55f);                     // fan
    }
}

// Balcony: concrete slab + railing with vertical bars.
static void drawBalcony(float x, float y, float width) {
    drawRectangle(x, y, width, 4, 0.70f, 0.66f, 0.60f);                          // slab
    drawRectangle(x, y + 14, width, 2, 0.20f, 0.20f, 0.22f);                     // top rail
    for (float bx = x + 2; bx < x + width; bx += 5)
        drawRectangle(bx, y + 4, 1.2f, 10, 0.20f, 0.20f, 0.22f);                 // bars
}

// Clothesline between two points, sagging in the middle, with laundry swinging in the wind.
static void drawClothesline(float x1, float x2, float y, float time) {
    float midX = (x1 + x2) / 2, sag = 6;
    drawLine(x1, y, midX, y - sag, 1, 0.9f, 0.9f, 0.9f);
    drawLine(midX, y - sag, x2, y, 1, 0.9f, 0.9f, 0.9f);
    float colors[4][3] = { {0.85f, 0.2f, 0.2f}, {0.2f, 0.45f, 0.85f}, {0.95f, 0.85f, 0.3f}, {0.3f, 0.7f, 0.4f} };
    int count = 4;
    for (int i = 0; i < count; i++) {
        float t = (i + 1.0f) / (count + 1);                                      // 0..1 along the line
        float cx = x1 + (x2 - x1) * t;
        float cy = y - sag * (1 - fabs(2 * t - 1));                              // follow the sag
        pushMatrix();
        translate(cx, cy);
        rotate(6.0f * sin(time * 2.0f + i));                                     // swing from the line
        if (i % 2 == 0) {                                                        // shirt: body + sleeves
            drawRectangle(-5, -14, 10, 14, colors[i][0], colors[i][1], colors[i][2]);
            drawRectangle(-8, -6, 3, 6, colors[i][0], colors[i][1], colors[i][2]);
            drawRectangle(5, -6, 3, 6, colors[i][0], colors[i][1], colors[i][2]);
        } else {                                                                 // trousers: two legs
            drawRectangle(-5, -16, 4, 16, colors[i][0], colors[i][1], colors[i][2]);
            drawRectangle(1, -16, 4, 16, colors[i][0], colors[i][1], colors[i][2]);
        }
        drawRectangle(-1, -1, 2, 3, 0.6f, 0.4f, 0.2f);                           // peg
        popMatrix();
    }
}

// Election poster of Sareeh El-Feel: bald head, round light glasses, dark suit, gray scarf.
static void drawElectionPoster(float x, float y) {
    drawRectangle(x, y, 34, 44, 0.80f, 0.12f, 0.12f);                            // red border
    drawRectangle(x + 2, y + 2, 30, 40, 0.97f, 0.95f, 0.90f);                    // paper
    drawTriangle(x + 6, y + 12, x + 28, y + 12, x + 17, y + 24, 0.18f, 0.18f, 0.22f);   // suit shoulders
    drawRectangle(x + 6, y + 10, 22, 4, 0.18f, 0.18f, 0.22f);
    drawRectangle(x + 15, y + 13, 4, 9, 0.45f, 0.45f, 0.45f);                    // scarf
    drawCircle(x + 17, y + 28, 7, 0.85f, 0.65f, 0.50f);                          // face
    drawEllipse(x + 17, y + 33, 6, 2, 0.95f, 0.80f, 0.68f);                      // shiny bald top
    drawCircle(x + 14, y + 28, 2.2f, 0.85f, 0.75f, 0.55f);                       // glasses (light frames)
    drawCircle(x + 20, y + 28, 2.2f, 0.85f, 0.75f, 0.55f);
    drawCircle(x + 14, y + 28, 1.3f, 0.85f, 0.65f, 0.50f);
    drawCircle(x + 20, y + 28, 1.3f, 0.85f, 0.65f, 0.50f);
    drawLabelAt("sign_vote", x + 7, y + 2, 9, 0.80f, 0.12f, 0.12f);   // "VOTE!"
}

// Streetlight standing on the back sidewalk.
static void drawStreetLight(float x) {
    drawRectangle(x - 3, 472, 6, 4, 0.25f, 0.25f, 0.28f);                        // base
    drawRectangle(x - 1.5f, 476, 3, 110, 0.30f, 0.30f, 0.33f);                   // pole
    drawLine(x, 586, x + 18, 592, 2.5f, 0.30f, 0.30f, 0.33f);                    // arm
    drawQuad(x + 12, 588, x + 28, 588, x + 26, 594, x + 14, 594, 0.25f, 0.25f, 0.28f); // lamp head
    drawEllipse(x + 20, 587, 6, 2, 1.0f, 0.95f, 0.70f);                          // bulb
    drawTriangle(x + 20, 586, x + 4, 520, x + 36, 520, 1.0f, 0.95f, 0.6f, 0.08f); // faint light cone
}

// Street kiosk: booth, striped awning, goods on the counter, a small fridge.
static void drawKiosk(float x) {
    float y = 472;
    drawRectangle(x, y, 90, 52, 0.15f, 0.45f, 0.35f);                            // booth
    drawRectangle(x + 8, y + 20, 74, 26, 0.10f, 0.12f, 0.14f);                   // open window (dark inside)
    float goods[5][3] = { {0.9f,0.2f,0.2f}, {0.2f,0.5f,0.9f}, {0.95f,0.8f,0.2f}, {0.3f,0.75f,0.3f}, {0.9f,0.5f,0.1f} };
    for (int i = 0; i < 5; i++) {
        drawRectangle(x + 12 + i * 14, y + 22, 8, 12, goods[i][0], goods[i][1], goods[i][2]);   // bottles/boxes
        drawRectangle(x + 13 + i * 14, y + 34, 6, 3, 0.9f, 0.9f, 0.9f);                        // caps
    }
    drawRectangle(x + 4, y + 18, 82, 4, 0.55f, 0.40f, 0.25f);                    // counter
    for (int i = 0; i < 6; i++) {                                                // striped awning
        float sx = x - 4 + i * 16.3f;
        drawQuad(sx, y + 56, sx + 16.3f, y + 56, sx + 16.3f, y + 64, sx, y + 64,
                 i % 2 ? 0.95f : 0.85f, i % 2 ? 0.95f : 0.15f, i % 2 ? 0.95f : 0.15f);
    }
    drawRectangle(x - 4, y + 52, 98, 4, 0.45f, 0.10f, 0.10f);
    drawRectangle(x + 92, y, 22, 44, 0.85f, 0.15f, 0.15f);                       // drinks fridge
    drawRectangle(x + 95, y + 6, 16, 32, 0.70f, 0.85f, 0.95f, 0.8f);
    TextLine(TC_S).label("sign_kiosk").drawCentered(x + 45, y + 1, 0.95f, 0.95f, 0.85f);   // "KIOSK"
}

// Apartment building: ground-floor shop shutter, windows with shutters, balconies, roof props.
static void drawBuilding(float x, float width, int floors, float r, float g, float b,
                         bool tank, bool dish, bool clothes, bool poster, float time) {
    float baseY = BACK_SIDEWALK_TOP;
    float height = floors * FLOOR_HEIGHT + 8;
    drawRectangle(x, baseY, width, height, r, g, b);                             // facade
    drawRectangle(x + width - 6, baseY, 6, height, r * 0.85f, g * 0.85f, b * 0.85f);   // shaded side edge
    drawRectangle(x - 2, baseY + height, width + 4, 4, r * 0.75f, g * 0.75f, b * 0.75f); // roof parapet

    // Ground floor: rolled metal shop shutter
    drawRectangle(x + 10, baseY, width - 20, FLOOR_HEIGHT - 8, 0.55f, 0.56f, 0.58f);
    for (float sy = baseY + 3; sy < baseY + FLOOR_HEIGHT - 8; sy += 4)
        drawRectangle(x + 10, sy, width - 20, 1, 0.42f, 0.43f, 0.45f);
    if (poster) drawElectionPoster(x + width / 2 - 17, baseY + 2);

    // Upper floors: windows (+ balcony on every second floor)
    int windows = (int)(width / 45);
    float spacing = width / windows;
    for (int floor = 1; floor < floors; floor++) {
        float fy = baseY + floor * FLOOR_HEIGHT;
        for (int w = 0; w < windows; w++)
            drawWindow(x + spacing * w + spacing / 2 - 9, fy + 6, (floor + w) % 3 == 0);
        if (floor % 2 == 1)
            drawBalcony(x + spacing / 2 - 20, fy + 2, spacing * (windows - 1) + 40);
    }
    if (clothes && floors >= 3)
        drawClothesline(x + 8, x + width - 8, baseY + 2 * FLOOR_HEIGHT + 32, time);

    float roofY = baseY + height + 4;
    if (tank) drawWaterTank(x + width * 0.3f, roofY);
    if (dish) drawSatelliteDish(x + width * 0.7f, roofY);
}

// ===================== Whole scene =====================
void drawEnvironment(float time) {
    // Sky (two bands for a simple gradient) and the sun
    drawRectangle(0, FIELD_TOP, SCREEN_WIDTH, SCREEN_HEIGHT - FIELD_TOP, 0.60f, 0.78f, 0.92f);
    drawRectangle(0, FIELD_TOP, SCREEN_WIDTH, 90, 0.80f, 0.86f, 0.88f);
    drawCircle(1180, 620, 26, 1.0f, 0.92f, 0.60f);

    // Background buildings, left to right (x, width, floors, color, tank, dish, clothes, poster)
    // Roof props only on 3-floor buildings: on 4-floor ones the UI bar would hide them.
    drawBuilding(180, 165, 4, 0.86f, 0.76f, 0.60f, false, false, true, false, time);
    drawBuilding(350, 145, 3, 0.85f, 0.66f, 0.58f, true, true, false, true, time);
    drawBuilding(500, 180, 4, 0.90f, 0.80f, 0.56f, false, false, true, false, time);
    drawBuilding(685, 140, 3, 0.76f, 0.74f, 0.71f, true, true, false, true, time);
    drawBuilding(830, 175, 4, 0.93f, 0.88f, 0.76f, false, false, true, false, time);
    drawBuilding(1010, 125, 3, 0.80f, 0.62f, 0.50f, true, true, false, true, time);

    // Back sidewalk, streetlights and the kiosk
    drawRectangle(0, FIELD_TOP, SCREEN_WIDTH, BACK_SIDEWALK_TOP - FIELD_TOP, 0.72f, 0.70f, 0.66f);
    for (float x = 0; x < SCREEN_WIDTH; x += 40)
        drawRectangle(x, FIELD_TOP, 1, BACK_SIDEWALK_TOP - FIELD_TOP, 0.55f, 0.53f, 0.50f);
    drawStreetLight(345);
    drawStreetLight(680);
    drawStreetLight(1005);
    drawKiosk(1150);

    // Street lanes: asphalt in two shades, dashed white lines between lanes
    for (int lane = 0; lane < LANE_COUNT; lane++) {
        float laneY = FIELD_BOTTOM + lane * LANE_HEIGHT;
        float shade = (lane % 2 == 0) ? 0.40f : 0.36f;
        drawRectangle(0, laneY, SCREEN_WIDTH, LANE_HEIGHT, shade, shade, shade + 0.02f);
        if (lane > 0)
            for (float x = BASE_WIDTH + 20; x < SCREEN_WIDTH; x += 70)
                drawRectangle(x, laneY - 2, 36, 4, 0.92f, 0.92f, 0.88f);
    }
    // a few manhole covers and road patches (fixed positions, so they do not jump around)
    drawEllipse(520, 95, 16, 6, 0.25f, 0.25f, 0.27f);
    drawEllipse(520, 95, 12, 4, 0.30f, 0.30f, 0.32f);
    drawEllipse(940, 315, 16, 6, 0.25f, 0.25f, 0.27f);
    drawEllipse(940, 315, 12, 4, 0.30f, 0.30f, 0.32f);
    drawRectangle(760, 190, 60, 20, 0.33f, 0.33f, 0.35f);

    // Curb along the top of the street and the front sidewalk at the bottom
    drawRectangle(0, FIELD_TOP - 4, SCREEN_WIDTH, 4, 0.85f, 0.85f, 0.82f);
    drawRectangle(0, 0, SCREEN_WIDTH, FIELD_BOTTOM, 0.72f, 0.70f, 0.66f);
    for (float x = 0; x < SCREEN_WIDTH; x += 40)
        drawRectangle(x, 0, 1, FIELD_BOTTOM, 0.55f, 0.53f, 0.50f);
    drawRectangle(0, FIELD_BOTTOM - 4, SCREEN_WIDTH, 4, 0.85f, 0.85f, 0.82f);
}

// ===================== Home base: Dr. Beram's laboratory =====================

// Erlenmeyer flask: cone body + neck, with colored liquid inside.
static void drawFlask(float x, float y, float r, float g, float b) {
    drawTriangle(x - 9, y, x + 9, y, x, y + 18, 0.85f, 0.93f, 0.97f);            // glass
    drawRectangle(x - 2.5f, y + 12, 5, 9, 0.85f, 0.93f, 0.97f);                  // neck
    drawQuad(x - 7, y + 1, x + 7, y + 1, x + 3.5f, y + 8, x - 3.5f, y + 8, r, g, b);  // liquid
}

// The last bottle of El-Anab formula: glowing glass bottle on a metal stand.
static void drawFormulaBottle(float x, float y, float time) {
    float pulse = 0.5f + 0.5f * sin(time * 3.0f);                                // 0..1, repeating
    drawCircle(x, y + 22, 34 + 4 * pulse, 0.65f, 0.05f, 0.25f, 0.15f);           // glow
    drawRectangle(x - 22, y - 8, 44, 8, 0.45f, 0.47f, 0.50f);                    // stand
    drawRectangle(x - 16, y - 14, 4, 6, 0.35f, 0.37f, 0.40f);                    // stand legs
    drawRectangle(x + 12, y - 14, 4, 6, 0.35f, 0.37f, 0.40f);
    drawCircle(x, y + 18, 20, 0.80f, 0.90f, 0.95f, 0.6f);                        // glass body
    drawEllipse(x, y + 14, 16, 13, 0.50f, 0.04f, 0.20f);                         // el-anab liquid
    drawRectangle(x - 5, y + 34, 10, 14, 0.80f, 0.90f, 0.95f, 0.6f);             // neck
    drawRectangle(x - 6, y + 46, 12, 6, 0.55f, 0.35f, 0.20f);                    // cork
    drawEllipse(x - 7, y + 22, 3, 6, 1.0f, 1.0f, 1.0f, 0.6f);                    // shine
    drawCircle(x + 4, y + 10 + 6 * pulse, 2, 0.9f, 0.5f, 0.7f, 0.8f);            // rising bubble
}

static void drawHazardSign(float x, float y) {
    drawTriangle(x, y, x + 24, y, x + 12, y + 21, 0.15f, 0.15f, 0.15f);
    drawTriangle(x + 3, y + 2, x + 21, y + 2, x + 12, y + 17, 1.0f, 0.82f, 0.1f);
    drawRectangle(x + 11, y + 7, 2, 6, 0.1f, 0.1f, 0.1f);                        // "!"
    drawRectangle(x + 11, y + 4, 2, 2, 0.1f, 0.1f, 0.1f);
}

void drawLaboratory(float time) {
    float labTop = 560;
    // Building body: clean white walls with teal trim, so it stands out from the beige street
    drawRectangle(0, 0, BASE_WIDTH, labTop, 0.92f, 0.94f, 0.95f);
    drawRectangle(BASE_WIDTH - 8, 0, 8, labTop, 0.10f, 0.55f, 0.55f);            // teal corner pillar
    drawRectangle(0, labTop, BASE_WIDTH + 4, 8, 0.10f, 0.45f, 0.45f);            // roof edge
    drawRectangle(0, FIELD_TOP - 4, BASE_WIDTH, 10, 0.10f, 0.55f, 0.55f);        // floor band

    // Pipe running up the wall, with red valves
    drawRectangle(12, 0, 8, labTop, 0.60f, 0.62f, 0.65f);
    for (float py = 60; py < labTop; py += 110)
        drawCircle(16, py, 6, 0.80f, 0.15f, 0.15f);

    // One door per lane on the street side: this is where the defenders come out
    for (int lane = 0; lane < LANE_COUNT; lane++) {
        float laneY = FIELD_BOTTOM + lane * LANE_HEIGHT;
        drawRectangle(118, laneY + 6, 44, 76, 0.10f, 0.45f, 0.45f);              // door frame
        drawRectangle(122, laneY + 6, 36, 70, 0.20f, 0.24f, 0.28f);              // door
        drawRectangle(128, laneY + 48, 24, 20, 0.55f, 0.75f, 0.85f);             // door window
        drawCircle(152, laneY + 38, 2.5f, 0.9f, 0.8f, 0.3f);                     // handle
        drawRectangle(36, laneY + 40, 60, 44, 0.80f, 0.82f, 0.84f);              // side window frame
        drawRectangle(40, laneY + 44, 52, 36, 0.55f, 0.75f, 0.85f);              // glass
        for (float by = laneY + 48; by < laneY + 80; by += 6)                    // blinds
            drawRectangle(40, by, 52, 2, 0.90f, 0.92f, 0.93f, 0.8f);
        if (lane % 2 == 0) drawHazardSign(54, laneY + 12);
    }

    // Upper floor: big window showing lab equipment
    drawRectangle(26, 488, 130, 62, 0.25f, 0.28f, 0.32f);                        // frame
    drawRectangle(30, 492, 122, 54, 0.75f, 0.88f, 0.92f);                        // glass
    drawRectangle(30, 492, 122, 4, 0.55f, 0.40f, 0.25f);                         // shelf
    drawFlask(46, 496, 0.3f, 0.85f, 0.3f);
    drawFlask(72, 496, 0.55f, 0.05f, 0.25f);
    drawRectangle(92, 496, 30, 4, 0.40f, 0.30f, 0.20f);                          // test-tube rack
    float tubeColors[4][3] = { {0.9f,0.2f,0.3f}, {0.2f,0.6f,0.9f}, {0.95f,0.8f,0.2f}, {0.55f,0.05f,0.25f} };
    for (int i = 0; i < 4; i++) {
        drawRectangle(95 + i * 7.0f, 500, 4, 16, 0.85f, 0.93f, 0.97f);
        drawRectangle(95 + i * 7.0f, 500, 4, 8, tubeColors[i][0], tubeColors[i][1], tubeColors[i][2]);
    }
    drawCircle(140, 506, 9, 0.85f, 0.93f, 0.97f);                                // round flask
    drawCircle(140, 504, 6, 0.3f, 0.85f, 0.3f);
    drawRectangle(138, 514, 4, 12, 0.85f, 0.93f, 0.97f);

    // Name board: "Dr. Beram's Lab"
    drawRectangle(30, FIELD_TOP - 27, 112, 22, 0.12f, 0.12f, 0.15f);
    TextLine(TC_M).label("sign_lab").drawCentered(86, FIELD_TOP - 27, 0.4f, 1.0f, 0.6f, 1, false);

    // Roof: antenna with blinking light, and the last formula bottle (what the players defend)
    drawLine(150, labTop + 8, 150, labTop + 60, 2, 0.4f, 0.4f, 0.42f);
    drawLine(142, labTop + 45, 158, labTop + 45, 1.5f, 0.4f, 0.4f, 0.42f);
    if (sin(time * 4.0f) > 0) drawCircle(150, labTop + 62, 3, 1.0f, 0.2f, 0.2f);
    drawFormulaBottle(80, labTop + 22, time);
}
