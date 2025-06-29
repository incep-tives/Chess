#include "PromotionPopup.h"
#include <windows.h>
#include <mmsystem.h>
#include <gdiplus.h>
#include "GameOverPopup.h"
#include "ChessBoard.h"
#include "ChessGame.h"

void PromotionPopup::reset() {
    state = PromotionState::None;
    row = col = -1;
    white = true;
    options.clear();
    selected = -1;
}

void PromotionPopup::start(int r, int c, bool w) {
    state = PromotionState::Choosing;
    row = r;
    col = c;
    white = w;
    options = white
        ? std::vector<std::string>{"white_queen", "white_rook", "white_bishop", "white_knight"}
        : std::vector<std::string>{"black_queen", "black_rook", "black_bishop", "black_knight"};
    selected = -1;
}

bool PromotionPopup::handleClick(int x, int y, float squareSize, ChessGame& game, GameOverPopup& gameOverPopup) {
    if (state != PromotionState::Choosing) return false;
    int popupX = int((col + 0.5f) * squareSize - squareSize / 2);
    int popupY = int(row * squareSize);
    if (x >= popupX && x < popupX + int(squareSize) && y >= popupY && y < popupY + int(4 * squareSize)) {
        int idx = (y - popupY) / int(squareSize);
        if (idx >= 0 && idx < 4) {
            if (game.lastMoveFromRow != -1 && game.lastMoveFromCol != -1) {
                game.board.board[game.lastMoveFromRow][game.lastMoveFromCol] = Piece();
            }
            game.board.board[row][col] = Piece(options[idx], white ? 1 : 0);
            PlaySoundW(L"assets\\sounds\\promote.wav", nullptr, SND_FILENAME | SND_ASYNC);
            reset();
            game.whiteTurn = !game.whiteTurn;
            gameOverPopup.checkGameEnd(game);
            bool isWhite = game.whiteTurn;
            game.inCheck = false;
            for (int r = 0; r < 8; ++r)
                for (int c = 0; c < 8; ++c) {
                    const Piece& p = game.board.board[r][c];
                    if ((isWhite && p.name == "white_king") || (!isWhite && p.name == "black_king")) {
                        if (game.board.isSquareAttacked(r, c, !isWhite)) {
                            game.inCheck = true;
                            game.checkedKingRow = r;
                            game.checkedKingCol = c;
                            game.flashCheck = true;
                            game.flashStart = GetTickCount();
                            PlaySoundW(L"assets\\sounds\\illegal.wav", nullptr, SND_FILENAME | SND_ASYNC);
                        }
                    }
                }
            return true;
        }
    }
    return false;
}

void PromotionPopup::draw(Gdiplus::Graphics& graphics, float squareSize, const std::map<std::string, std::unique_ptr<Gdiplus::Image>>& pieceImages) const {
    if (state != PromotionState::Choosing) return;
    float popupX = static_cast<float>((col + 0.5f) * squareSize - squareSize / 2);
    float popupY = static_cast<float>(row * squareSize);
    Gdiplus::SolidBrush bg(Gdiplus::Color(220, 240, 240, 240));
    graphics.FillRectangle(&bg, popupX, popupY, squareSize, 4 * squareSize);
    Gdiplus::Pen border(Gdiplus::Color(255, 80, 80, 80), 2);
    graphics.DrawRectangle(&border, popupX, popupY, squareSize, 4 * squareSize);
    for (int i = 0; i < 4; ++i) {
        auto it = pieceImages.find(options[i]);
        if (it != pieceImages.end() && it->second) {
            graphics.DrawImage(
                it->second.get(),
                popupX, popupY + i * squareSize,
                squareSize, squareSize
            );
        }
    }
}