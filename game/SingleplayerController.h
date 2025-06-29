#pragma once
#include "ChessGame.h"
#include "StockfishEngine.h"
#include "GameOverPopup.h"
#include <vector>
#include <string>

enum class PlayerColor { White, Black, None };

class SingleplayerController {
public:
    SingleplayerController();
    void start(PlayerColor color, int elo);
    void onPlayerMove(ChessGame& game, GameOverPopup& popup, HWND hwnd);

    PlayerColor playerColor = PlayerColor::None;
    std::vector<std::string> moveHistory;

    bool isBotTurn(const ChessGame& game) const;

private:
    StockfishEngine engine;
    int elo = 1350;
    std::string getFEN(const ChessGame& game) const;
    bool applyUCIMove(ChessGame& game, const std::string& uciMove);
};