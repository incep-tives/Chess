#pragma once
#include "ChessBoard.h"
#include <string>

class ChessGame {
public:
    ChessBoard board;
    bool whiteTurn = true;
    int selectedRow = -1, selectedCol = -1;
    int lastMoveFromRow = -1, lastMoveFromCol = -1;
    int lastMoveToRow = -1, lastMoveToCol = -1;
    bool inCheck = false;
    int checkedKingRow = -1, checkedKingCol = -1;
    bool flashCheck = false;
    unsigned int flashStart = 0;

    ChessGame();
    void selectOrMove(int row, int col);
    void resetSelection();
    std::string serialize() const;
    void deserialize(const std::string& data);
};