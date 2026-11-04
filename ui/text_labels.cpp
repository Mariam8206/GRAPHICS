#include "ui/text_labels.h"
#include "renderer/renderer.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>   // GetModuleFileNameA: find the assets next to the .exe as well; MessageBoxA for the error

// Every label id (file ui/assets/labels/<id>.png, text in ui/assets/labels/labels.tsv) with its size class.
struct LabelInfo { const char* id; TextClass cls; };
static const LabelInfo LABELS[] = {
    { "coins_label", TC_M },  { "wave_label", TC_XL }, { "of", TC_XL },
    { "name_melee", TC_N },   { "name_ranged", TC_N }, { "name_defense", TC_N },
    { "hp", TC_S },           { "dmg", TC_S },         { "blocks", TC_S },       { "field", TC_S },
    { "st_unlocks", TC_S },   { "st_limit", TC_S },    { "st_need", TC_S },      { "st_meleefirst", TC_S },
    { "st_click", TC_S },     { "st_ready", TC_S },
    { "msg_gameover", TC_L }, { "msg_locked", TC_L },  { "msg_meleefirst", TC_L }, { "msg_coins", TC_L },
    { "msg_nocoins", TC_L },  { "msg_limit", TC_L },   { "msg_badlane", TC_L },  { "msg_lanetaken", TC_L },
    { "msg_unlocked_melee", TC_L }, { "msg_unlocked_ranged", TC_L }, { "msg_unlocked_defense", TC_L },
    { "ban_wave", TC_XXL },   { "ban_cleared", TC_XXL }, { "ban_win", TC_XXL },  { "ban_lose", TC_XXL },
    { "hint", TC_M },         { "sign_lab", TC_M },    { "sign_kiosk", TC_S },   { "sign_vote", TC_XS },
    { "shield_police", TC_XS },
    // menus
    { "title_game", TC_XXL }, { "team", TC_M },
    { "menu_start", TC_BTN }, { "menu_exit", TC_BTN }, { "menu_continue", TC_BTN },
    { "menu_restart", TC_BTN }, { "menu_main", TC_BTN }, { "pause_title", TC_XXL },
    // cutscene subtitles
    { "cs_beram", TC_L }, { "cs_spider", TC_L }, { "cs_thalma", TC_L }, { "cs_sareeh", TC_L }, { "cs_anab", TC_L },
    { "cs_title", TC_XXL }, { "cs_skip", TC_S },
};
static const char* CLASS_PREFIX[TEXT_CLASS_COUNT] = { "xs", "s", "m", "n", "l", "xl", "xxl", "btn" };
static const float CLASS_HEIGHT[TEXT_CLASS_COUNT] = { 14, 17, 22, 23, 33, 32, 83, 43 };   // = image heights (make_labels.ps1)
static const char* DIGIT_NAME[13] = { "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "slash", "plus", "dot" };

static std::map<std::string, Texture> labelTextures;
static Texture digitTextures[TEXT_CLASS_COUNT][13];
static std::string folder;

static const LabelInfo* findLabel(const std::string& id) {
    for (const LabelInfo& info : LABELS) if (id == info.id) return &info;
    std::cerr << "Unknown UI label id: " << id << std::endl;
    return nullptr;
}

static bool fileExists(const std::string& path) { return std::ifstream(path, std::ios::binary).good(); }

// The game normally starts in the project folder (Visual Studio, build_and_run.bat). If it is started
// from somewhere else, look next to the .exe and up to two folders above it (x64\Debug -> project).
static std::string findAssetFolder() {
    const std::string LABELS = "ui/assets/labels/";
    std::vector<std::string> candidates = { LABELS };
    char exePath[MAX_PATH] = {};
    if (GetModuleFileNameA(nullptr, exePath, MAX_PATH)) {
        std::string dir(exePath);
        dir = dir.substr(0, dir.find_last_of("\\/") + 1);
        candidates.push_back(dir + LABELS);
        candidates.push_back(dir + "../" + LABELS);
        candidates.push_back(dir + "../../" + LABELS);
    }
    for (const std::string& c : candidates)
        if (fileExists(c + "hp.png")) return c;
    return "";
}

static int digitIndex(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c == '/') return 10;
    if (c == '+') return 11;
    if (c == '.') return 12;
    return -1;
}

bool initTextLabels() {
    folder = findAssetFolder();
    if (folder.empty()) {
        std::cerr << "ERROR: the folder ui/assets/labels (UI text images) was not found" << std::endl;
        return false;
    }
    bool ok = true;
    for (const LabelInfo& info : LABELS) {
        Texture t;
        if (loadTexture((folder + info.id + ".png").c_str(), t)) labelTextures[info.id] = t;
        else { std::cerr << "ERROR: missing UI image " << folder << info.id << ".png" << std::endl; ok = false; }
    }
    for (int c = 0; c < TEXT_CLASS_COUNT; c++)
        for (int d = 0; d < 13; d++) {
            std::string path = folder + CLASS_PREFIX[c] + "_digit_" + DIGIT_NAME[d] + ".png";
            if (!loadTexture(path.c_str(), digitTextures[c][d])) { std::cerr << "ERROR: missing UI image " << path << std::endl; ok = false; }
        }
    return ok;
}

void showTextImagesError() {
    MessageBoxA(nullptr,
        "The game cannot start because its text images are missing.\n\n"
        "Keep the folder ui\\assets\\labels inside the project folder and start the game from there "
        "(or from x64\\Debug / x64\\Release). The console lists the missing files.",
        "The Last Anab Bottle Crisis", MB_OK | MB_ICONERROR);
}

std::string textAssetFolder() { return folder; }

// ---------- TextLine ----------
TextLine& TextLine::label(const char* id) { if (findLabel(id)) pieces.push_back({ 0, id, 0 }); return *this; }
TextLine& TextLine::number(const std::string& digits) { pieces.push_back({ 1, digits, 0 }); return *this; }
TextLine& TextLine::gap(float pixels) { pieces.push_back({ 2, "", pixels }); return *this; }

static float pieceWidth(int kind, const std::string& text, float pixels, TextClass cls) {
    if (kind == 2) return pixels;
    if (kind == 0) return (float)labelTextures[text].width;
    float w = 0;
    for (char ch : text) { int d = digitIndex(ch); if (d >= 0) w += digitTextures[cls][d].width; }
    return w;
}

static float spacing(TextClass cls) { return std::round(CLASS_HEIGHT[cls] * 0.22f); }   // between two pieces

float TextLine::width() const {
    float w = 0;
    for (size_t i = 0; i < pieces.size(); i++) {
        w += pieceWidth(pieces[i].kind, pieces[i].text, pieces[i].pixels, cls);
        if (i + 1 < pieces.size() && pieces[i].kind != 2 && pieces[i + 1].kind != 2) w += spacing(cls);
    }
    return w;
}

float TextLine::height() const { return CLASS_HEIGHT[cls]; }

void TextLine::drawPieces(float rightX, float bottomY, float r, float g, float b, float a) const {
    float x = std::round(rightX - width()), y = std::round(bottomY);   // whole pixels: the images stay sharp
    for (size_t i = 0; i < pieces.size(); i++) {                       // the first piece is the leftmost
        const Piece& p = pieces[i];
        if (p.kind == 0) {
            const Texture& t = labelTextures[p.text];
            drawImage(t, x, y, (float)t.width, (float)t.height, r, g, b, a);
        } else if (p.kind == 1) {
            float dx = x;
            for (char ch : p.text) {
                int d = digitIndex(ch);
                if (d < 0) continue;
                const Texture& t = digitTextures[cls][d];
                drawImage(t, dx, y, (float)t.width, (float)t.height, r, g, b, a);
                dx += t.width;
            }
        }
        x += pieceWidth(p.kind, p.text, p.pixels, cls);
        if (i + 1 < pieces.size() && p.kind != 2 && pieces[i + 1].kind != 2) x += spacing(cls);
    }
}

void TextLine::drawRight(float rightX, float bottomY, float r, float g, float b, float a, bool shadow) const {
    if (outline) {
        const float OFFSETS[8][2] = { {-1,0}, {1,0}, {0,-1}, {0,1}, {-1,-1}, {1,-1}, {-1,1}, {1,1} };
        for (const auto& o : OFFSETS) drawPieces(rightX + o[0], bottomY + o[1], 0.08f, 0.04f, 0.06f, a * 0.9f);
    } else if (shadow) {
        float off = (cls == TC_XXL) ? 3.0f : 1.0f;
        drawPieces(rightX + off, bottomY - off, 0, 0, 0, a * 0.75f);
    }
    drawPieces(rightX, bottomY, r, g, b, a);
}
void TextLine::drawCentered(float centerX, float bottomY, float r, float g, float b, float a, bool shadow) const {
    drawRight(centerX + width() / 2, bottomY, r, g, b, a, shadow);
}
void TextLine::drawLeft(float leftX, float bottomY, float r, float g, float b, float a, bool shadow) const {
    drawRight(leftX + width(), bottomY, r, g, b, a, shadow);
}

void drawLabelAt(const char* id, float x, float y, float height, float r, float g, float b, float a) {
    if (!findLabel(id)) return;
    const Texture& t = labelTextures[id];
    float s = height / t.height;
    drawImage(t, x, y, t.width * s, height, r, g, b, a);
}
