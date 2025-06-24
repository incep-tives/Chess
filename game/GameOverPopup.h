#pragma once
#include <string>
#include <windows.h>
#include <gdiplus.h>
#include "ChessGame.h"

enum class GameResult { None, WhiteWins, BlackWins, Stalemate };

class GameOverPopup {
public:
    GameResult result = GameResult::None;
    void show(GameResult r);
    void reset();
    std::wstring getMessage() const;
    void checkGameEnd(ChessGame& game);
};