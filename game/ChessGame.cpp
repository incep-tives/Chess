#include "ChessGame.h"
#include "PromotionPopup.h"
#include <windows.h>
#include <gdiplus.h>

ChessGame::ChessGame() : board(), whiteTurn(true), selectedRow(-1), selectedCol(-1),
                         lastMoveFromRow(-1), lastMoveFromCol(-1), lastMoveToRow(-1), lastMoveToCol(-1),
                         inCheck(false), checkedKingRow(-1), checkedKingCol(-1) {}

void ChessGame::selectOrMove(int row, int col) {
    const Piece& piece = board.board[row][col];
    if ((whiteTurn && board.isWhitePiece(piece)) || (!whiteTurn && board.isBlackPiece(piece))) {
        selectedRow = row;
        selectedCol = col;
    } else if (selectedRow == row && selectedCol == col) {
        resetSelection();
    }
}

void ChessGame::resetSelection() {
    selectedRow = selectedCol = -1;
}

std::string ChessGame::serialize() const {
    std::string out;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            out += ChessBoard::toString(board.board[r][c]);
            out += ',';
        }
    }
    out += (whiteTurn ? "W" : "B");
    return out;
}

void ChessGame::deserialize(const std::string& data) {
    size_t idx = 0, pos = 0;
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            pos = data.find(',', idx);
            board.board[r][c] = ChessBoard::fromString(data.substr(idx, pos - idx));
            idx = pos + 1;
        }
    }
    whiteTurn = (data[idx] == 'W');
}