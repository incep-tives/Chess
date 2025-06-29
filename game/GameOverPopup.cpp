#include "GameOverPopup.h"
#include "ChessBoard.h"
#include "ChessGame.h"
#include <windows.h>

void GameOverPopup::reset() { result = GameResult::None; }

std::wstring GameOverPopup::getMessage() const {
    switch (result) {
        case GameResult::WhiteWins: return L"White wins by checkmate!";
        case GameResult::BlackWins: return L"Black wins by checkmate!";
        case GameResult::Stalemate: return L"Draw. Stalemate!";
        default: return L"";
    }
}

void GameOverPopup::show(GameResult r) {
    result = r;
    MessageBoxW(nullptr, getMessage().c_str(), L"Game Over", MB_OK | MB_ICONINFORMATION);
}

void GameOverPopup::checkGameEnd(ChessGame& game) {
    bool isWhiteTurn = game.whiteTurn;
    bool hasLegalMove = false;
    for (int r = 0; r < 8 && !hasLegalMove; ++r) {
        for (int c = 0; c < 8 && !hasLegalMove; ++c) {
            const Piece& p = game.board.board[r][c];
            if ((isWhiteTurn && game.board.isWhitePiece(p)) ||
                (!isWhiteTurn && game.board.isBlackPiece(p))) {
                auto moves = game.board.getLegalMoves(r, c, isWhiteTurn);
                for (const auto& move : moves) {
                    ChessBoard tempBoard = game.board;
                    tempBoard.movePiece(r, c, move.first, move.second, isWhiteTurn);
                    int kingRow = -1, kingCol = -1;
                    for (int tr = 0; tr < 8; ++tr)
                        for (int tc = 0; tc < 8; ++tc) {
                            const Piece& tp = tempBoard.board[tr][tc];
                            if ((isWhiteTurn && tp.name == "white_king") ||
                                (!isWhiteTurn && tp.name == "black_king")) {
                                kingRow = tr;
                                kingCol = tc;
                            }
                        }
                    if (kingRow != -1 && !tempBoard.isSquareAttacked(kingRow, kingCol, !isWhiteTurn)) {
                        hasLegalMove = true;
                        break;
                    }
                }
            }
        }
    }
    int kingRow = -1, kingCol = -1;
    for (int r = 0; r < 8; ++r)
        for (int c = 0; c < 8; ++c) {
            const Piece& p = game.board.board[r][c];
            if ((isWhiteTurn && p.name == "white_king") ||
                (!isWhiteTurn && p.name == "black_king")) {
                kingRow = r;
                kingCol = c;
            }
        }
    bool inCheck = (kingRow != -1) && game.board.isSquareAttacked(kingRow, kingCol, !isWhiteTurn);
    if (!hasLegalMove) {
        if (inCheck) {
            result = isWhiteTurn ? GameResult::BlackWins : GameResult::WhiteWins;
        } else {
            result = GameResult::Stalemate;
        }
        show(result);
    }
}