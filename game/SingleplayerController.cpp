#include "SingleplayerController.h"

SingleplayerController::SingleplayerController() {}

void SingleplayerController::start(PlayerColor color, int elo_) {
    playerColor = color;
    elo = elo_;
    moveHistory.clear();
    engine.start(elo);
}

bool SingleplayerController::isBotTurn(const ChessGame& game) const {
    return (playerColor == PlayerColor::White && !game.whiteTurn) ||
           (playerColor == PlayerColor::Black && game.whiteTurn);
}

std::string SingleplayerController::getFEN(const ChessGame& game) const {
    return "startpos";
}

bool SingleplayerController::applyUCIMove(ChessGame& game, const std::string& uciMove) {
    if (uciMove.length() < 4) return false;
    int fromCol = uciMove[0] - 'a';
    int fromRow = '8' - uciMove[1];
    int toCol = uciMove[2] - 'a';
    int toRow = '8' - uciMove[3];
    return game.board.movePiece(fromRow, fromCol, toRow, toCol, !game.whiteTurn);
}

void SingleplayerController::onPlayerMove(ChessGame& game, GameOverPopup& popup, HWND hwnd) {
    if (!isBotTurn(game)) return;
    std::string fen = getFEN(game);
    std::string bestMove = engine.getBestMove(fen, moveHistory);
    if (!bestMove.empty()) {
        applyUCIMove(game, bestMove);
        moveHistory.push_back(bestMove);
        game.whiteTurn = !game.whiteTurn;
        InvalidateRect(hwnd, nullptr, TRUE);
        popup.checkGameEnd(game);
    }
}