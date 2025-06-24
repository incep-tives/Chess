#pragma once
#include <objidl.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include <map>
#include <memory>

enum class PromotionState { None, Choosing };

class ChessGame;
class GameOverPopup;
class Piece;

class PromotionPopup {
public:
    PromotionState state = PromotionState::None;
    int row = -1, col = -1;
    bool white = true;
    std::vector<std::string> options;
    int selected = -1;

    void reset();
    void start(int r, int c, bool w);
    bool handleClick(int x, int y, float squareSize, ChessGame& game, GameOverPopup& gameOverPopup);
    void draw(Gdiplus::Graphics& graphics, float squareSize, const std::map<std::string, std::unique_ptr<Gdiplus::Image>>& pieceImages) const;
};