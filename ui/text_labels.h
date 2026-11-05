#pragma once
// UI text. Every label is a PNG made by ui/assets/labels/make_labels.ps1 (white text, Dubai Bold), drawn tinted.
// Numbers are drawn with digit images. The game needs the folder ui/assets/labels: without it, it shows an error
// and stops.
#include <string>
#include <vector>

// Size classes (all images of one class have the same height, so labels and digits share a baseline)
enum TextClass { TC_XS, TC_S, TC_M, TC_N, TC_L, TC_XL, TC_XXL, TC_BTN, TEXT_CLASS_COUNT };   // TC_BTN: menu buttons

bool initTextLabels();           // loads ui/assets/labels/*.png; call once after initRenderer(). false = images missing
void showTextImagesError();      // error box for a failed initTextLabels (the console lists the missing files)
std::string textAssetFolder();   // where the images were found ("" if not found)

// One line of text, written in reading order (left to right on screen): labels (by id) and numbers.
// Numbers are plain ASCII: digits and "/", "+", "." (e.g. "40", "+10").
class TextLine {
public:
    explicit TextLine(TextClass textClass) : cls(textClass) {}
    TextLine& label(const char* id);
    TextLine& number(const std::string& digits);
    TextLine& number(int value) { return number(std::to_string(value)); }
    TextLine& outlined() { outline = true; return *this; }   // dark 1 px outline instead of a drop shadow
                                                             // (for text over busy backgrounds: "+10", health)
    float width() const;
    // (rightX, bottomY) = bottom-right corner; shadow = dark copy 1 px down-right for readability
    void drawRight(float rightX, float bottomY, float r, float g, float b, float a = 1, bool shadow = true) const;
    void drawCentered(float centerX, float bottomY, float r, float g, float b, float a = 1, bool shadow = true) const;
    void drawLeft(float leftX, float bottomY, float r, float g, float b, float a = 1, bool shadow = true) const;
    bool empty() const { return pieces.empty(); }
private:
    enum PieceKind { PIECE_LABEL, PIECE_NUMBER };
    struct Piece { PieceKind kind; std::string text; };   // text = the label id, or the digits of a number
    TextClass cls;
    bool outline = false;
    std::vector<Piece> pieces;
    float pieceWidth(const Piece& piece) const;
    void drawPieces(float rightX, float bottomY, float r, float g, float b, float a) const;
};

// Single label drawn at its native size; (x, y) = bottom-left. Used for signs inside transformed models.
void drawLabelAt(const char* id, float x, float y, float height, float r, float g, float b, float a = 1);
