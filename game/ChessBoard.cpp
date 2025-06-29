#include "ChessBoard.h"
#include <algorithm>
#include <vector>
#include <utility>

int ChessBoard::getPieceBaseValue(const std::string& name) {
    if (name.find("pawn") != std::string::npos) return 1;
    if (name.find("knight") != std::string::npos) return 3;
    if (name.find("bishop") != std::string::npos) return 3;
    if (name.find("rook") != std::string::npos) return 5;
    if (name.find("queen") != std::string::npos) return 9;
    return 0;
}

Piece ChessBoard::fromString(const std::string& s) {
    auto pos = s.find(':');
    if (pos == std::string::npos) return Piece(s, getPieceBaseValue(s));
    std::string name = s.substr(0, pos);
    int value = std::stoi(s.substr(pos + 1));
    return Piece(name, value);
}

std::string ChessBoard::toString(const Piece& p) {
    if (p.empty()) return ":0";
    return p.name + ":" + std::to_string(p.value);
}

ChessBoard::ChessBoard() {
    board = {{
        { Piece("black_rook", 5), Piece("black_knight", 3), Piece("black_bishop", 3), Piece("black_queen", 9), Piece("black_king", 0), Piece("black_bishop", 3), Piece("black_knight", 3), Piece("black_rook", 5) },
        { Piece("black_pawn", 1), Piece("black_pawn", 1), Piece("black_pawn", 1), Piece("black_pawn", 1), Piece("black_pawn", 1), Piece("black_pawn", 1), Piece("black_pawn", 1), Piece("black_pawn", 1) },
        { Piece(), Piece(), Piece(), Piece(), Piece(), Piece(), Piece(), Piece() },
        { Piece(), Piece(), Piece(), Piece(), Piece(), Piece(), Piece(), Piece() },
        { Piece(), Piece(), Piece(), Piece(), Piece(), Piece(), Piece(), Piece() },
        { Piece(), Piece(), Piece(), Piece(), Piece(), Piece(), Piece(), Piece() },
        { Piece("white_pawn", 1), Piece("white_pawn", 1), Piece("white_pawn", 1), Piece("white_pawn", 1), Piece("white_pawn", 1), Piece("white_pawn", 1), Piece("white_pawn", 1), Piece("white_pawn", 1) },
        { Piece("white_rook", 5), Piece("white_knight", 3), Piece("white_bishop", 3), Piece("white_queen", 9), Piece("white_king", 0), Piece("white_bishop", 3), Piece("white_knight", 3), Piece("white_rook", 5) }
    }};
}

bool ChessBoard::isWhitePiece(const Piece& piece) const {
    return !piece.empty() && piece.name.find("white") == 0;
}
bool ChessBoard::isBlackPiece(const Piece& piece) const {
    return !piece.empty() && piece.name.find("black") == 0;
}

bool ChessBoard::isSquareAttacked(int row, int col, bool byWhite) const {
    for (int r = 0; r < 8; ++r) {
        for (int c = 0; c < 8; ++c) {
            const Piece& p = board[r][c];
            if (p.empty()) continue;
            if ((byWhite && isWhitePiece(p)) || (!byWhite && isBlackPiece(p))) {
                if (p.name.find("king") != std::string::npos) {
                    if (std::abs(r - row) <= 1 && std::abs(c - col) <= 1 && (r != row || c != col))
                        return true;
                } else {
                    auto moves = getLegalMoves(r, c, byWhite);
                    if (std::find(moves.begin(), moves.end(), std::make_pair(row, col)) != moves.end())
                        return true;
                }
            }
        }
    }
    return false;
}

bool ChessBoard::movePiece(int fromRow, int fromCol, int toRow, int toCol, bool whiteTurn) {
    Piece& src = board[fromRow][fromCol];
    if (src.empty()) return false;
    if (whiteTurn && !isWhitePiece(src)) return false;
    if (!whiteTurn && !isBlackPiece(src)) return false;

    auto legalMoves = getLegalMoves(fromRow, fromCol, whiteTurn);
    auto it = std::find(legalMoves.begin(), legalMoves.end(), std::make_pair(toRow, toCol));
    if (it == legalMoves.end()) return false;

    if (src.name.find("pawn") != std::string::npos && toCol != fromCol && board[toRow][toCol].empty()) {
        int capRow = whiteTurn ? toRow + 1 : toRow - 1;
        Piece capturedPawn = board[capRow][toCol];
        if (!capturedPawn.empty()) {
            if (isWhitePiece(capturedPawn))
                capturedWhite.push_back(capturedPawn);
            else
                capturedBlack.push_back(capturedPawn);
        }
        board[capRow][toCol] = Piece();
    }


    if (!board[toRow][toCol].empty()) {
        if (isWhitePiece(board[toRow][toCol]))
            capturedWhite.push_back(board[toRow][toCol]);
        else
            capturedBlack.push_back(board[toRow][toCol]);
    }

    if (src.name.find("pawn") != std::string::npos && std::abs(toRow - fromRow) == 2) {
        enPassantRow = (fromRow + toRow) / 2;
        enPassantCol = toCol;
    } else {
        enPassantRow = enPassantCol = -1;
    }

    if (src.name.find("king") != std::string::npos && std::abs(toCol - fromCol) == 2) {
        int homeRow = whiteTurn ? 7 : 0;
        if (toCol == 6) {
            board[homeRow][5] = board[homeRow][7];
            board[homeRow][7] = Piece();
        } else if (toCol == 2) {
            board[homeRow][3] = board[homeRow][0];
            board[homeRow][0] = Piece();
        }
    }

    if (src.name == "white_king") whiteKingMoved = true;
    if (src.name == "black_king") blackKingMoved = true;
    if (src.name == "white_rook" && fromRow == 7 && fromCol == 0) whiteQueensideRookMoved = true;
    if (src.name == "white_rook" && fromRow == 7 && fromCol == 7) whiteKingsideRookMoved = true;
    if (src.name == "black_rook" && fromRow == 0 && fromCol == 0) blackQueensideRookMoved = true;
    if (src.name == "black_rook" && fromRow == 0 && fromCol == 7) blackKingsideRookMoved = true;

    board[toRow][toCol] = src;
    board[fromRow][fromCol] = Piece();
    return true;
}

std::vector<std::pair<int, int>> ChessBoard::getLegalMoves(int row, int col, bool whiteTurn) const {
    std::vector<std::pair<int, int>> moves;
    const Piece& piece = board[row][col];
    if (piece.empty()) return moves;

    int dir = isWhitePiece(piece) ? -1 : 1;
    if (piece.name.find("pawn") != std::string::npos) {
        int nextRow = row + dir;
        if (nextRow >= 0 && nextRow < 8 && board[nextRow][col].empty())
            moves.emplace_back(nextRow, col);
        if ((row == 6 && whiteTurn) || (row == 1 && !whiteTurn)) {
            int doubleRow = row + 2 * dir;
            if (board[nextRow][col].empty() && board[doubleRow][col].empty())
                moves.emplace_back(doubleRow, col);
        }
        for (int dc : {-1, 1}) {
            int nc = col + dc;
            if (nc >= 0 && nc < 8 && nextRow >= 0 && nextRow < 8) {
                if (!board[nextRow][nc].empty() &&
                    ((whiteTurn && isBlackPiece(board[nextRow][nc])) || (!whiteTurn && isWhitePiece(board[nextRow][nc]))))
                    moves.emplace_back(nextRow, nc);
                if (nextRow == enPassantRow && nc == enPassantCol &&
                    board[row][nc].name.find(whiteTurn ? "black_pawn" : "white_pawn") != std::string::npos) {
                    moves.emplace_back(nextRow, nc);
                }
            }
        }
    } else if (piece.name.find("rook") != std::string::npos) {
        for (int dr : {-1, 1}) {
            for (int r = row + dr; r >= 0 && r < 8; r += dr) {
                if (!board[r][col].empty()) {
                    if ((whiteTurn && isBlackPiece(board[r][col])) || (!whiteTurn && isWhitePiece(board[r][col])))
                        moves.emplace_back(r, col);
                    break;
                }
                moves.emplace_back(r, col);
            }
        }
        for (int dc : {-1, 1}) {
            for (int c = col + dc; c >= 0 && c < 8; c += dc) {
                if (!board[row][c].empty()) {
                    if ((whiteTurn && isBlackPiece(board[row][c])) || (!whiteTurn && isWhitePiece(board[row][c])))
                        moves.emplace_back(row, c);
                    break;
                }
                moves.emplace_back(row, c);
            }
        }
    } else if (piece.name.find("knight") != std::string::npos) {
        int d[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
        for (auto& off : d) {
            int nr = row + off[0], nc = col + off[1];
            if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8 &&
                (board[nr][nc].empty() ||
                 (whiteTurn && isBlackPiece(board[nr][nc])) ||
                 (!whiteTurn && isWhitePiece(board[nr][nc]))))
                moves.emplace_back(nr, nc);
        }
    } else if (piece.name.find("bishop") != std::string::npos) {
        for (int dr : {-1, 1}) for (int dc : {-1, 1}) {
            int r = row + dr, c = col + dc;
            while (r >= 0 && r < 8 && c >= 0 && c < 8) {
                if (!board[r][c].empty()) {
                    if ((whiteTurn && isBlackPiece(board[r][c])) || (!whiteTurn && isWhitePiece(board[r][c])))
                        moves.emplace_back(r, c);
                    break;
                }
                moves.emplace_back(r, c);
                r += dr; c += dc;
            }
        }
    } else if (piece.name.find("queen") != std::string::npos) {
        for (int dr : {-1, 1}) {
            for (int r = row + dr; r >= 0 && r < 8; r += dr) {
                if (!board[r][col].empty()) {
                    if ((whiteTurn && isBlackPiece(board[r][col])) || (!whiteTurn && isWhitePiece(board[r][col])))
                        moves.emplace_back(r, col);
                    break;
                }
                moves.emplace_back(r, col);
            }
        }
        for (int dc : {-1, 1}) {
            for (int c = col + dc; c >= 0 && c < 8; c += dc) {
                if (!board[row][c].empty()) {
                    if ((whiteTurn && isBlackPiece(board[row][c])) || (!whiteTurn && isWhitePiece(board[row][c])))
                        moves.emplace_back(row, c);
                    break;
                }
                moves.emplace_back(row, c);
            }
        }
        for (int dr : {-1, 1}) for (int dc : {-1, 1}) {
            int r = row + dr, c = col + dc;
            while (r >= 0 && r < 8 && c >= 0 && c < 8) {
                if (!board[r][c].empty()) {
                    if ((whiteTurn && isBlackPiece(board[r][c])) || (!whiteTurn && isWhitePiece(board[r][c])))
                        moves.emplace_back(r, c);
                    break;
                }
                moves.emplace_back(r, c);
                r += dr; c += dc;
            }
        }
    } else if (piece.name.find("king") != std::string::npos) {
        for (int dr = -1; dr <= 1; ++dr) for (int dc = -1; dc <= 1; ++dc) {
            if (dr == 0 && dc == 0) continue;
            int nr = row + dr, nc = col + dc;
            if (nr >= 0 && nr < 8 && nc >= 0 && nc < 8 &&
                (board[nr][nc].empty() ||
                 (whiteTurn && isBlackPiece(board[nr][nc])) ||
                 (!whiteTurn && isWhitePiece(board[nr][nc]))))
                moves.emplace_back(nr, nc);
        }
        int homeRow = whiteTurn ? 7 : 0;
        bool kingMoved = whiteTurn ? whiteKingMoved : blackKingMoved;
        bool kingsideRookMoved = whiteTurn ? whiteKingsideRookMoved : blackKingsideRookMoved;
        bool queensideRookMoved = whiteTurn ? whiteQueensideRookMoved : blackQueensideRookMoved;

        if (!kingMoved && !kingsideRookMoved &&
            board[homeRow][5].empty() && board[homeRow][6].empty() &&
            !isSquareAttacked(homeRow, 4, !whiteTurn) &&
            !isSquareAttacked(homeRow, 5, !whiteTurn) &&
            !isSquareAttacked(homeRow, 6, !whiteTurn)) {
            if ((whiteTurn && board[homeRow][7].name == "white_rook") ||
                (!whiteTurn && board[homeRow][7].name == "black_rook")) {
                moves.emplace_back(homeRow, 6);
            }
        }
        if (!kingMoved && !queensideRookMoved &&
            board[homeRow][1].empty() && board[homeRow][2].empty() && board[homeRow][3].empty() &&
            !isSquareAttacked(homeRow, 4, !whiteTurn) &&
            !isSquareAttacked(homeRow, 3, !whiteTurn) &&
            !isSquareAttacked(homeRow, 2, !whiteTurn)) {
            if ((whiteTurn && board[homeRow][0].name == "white_rook") ||
                (!whiteTurn && board[homeRow][0].name == "black_rook")) {
                moves.emplace_back(homeRow, 2);
            }
        }
    }
    return moves;
}