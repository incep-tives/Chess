#pragma once
#include <string>
#include <vector>
#include <utility>
#include <array>

struct Piece {
    std::string name;
    int value;
    Piece() : name(""), value(0) {}
    Piece(const std::string& n, int v) : name(n), value(v) {}
    bool empty() const { return name.empty(); }
};

class ChessBoard {
public:
    std::array<std::array<Piece, 8>, 8> board;
    bool whiteKingMoved = false, blackKingMoved = false;
    bool whiteKingsideRookMoved = false, whiteQueensideRookMoved = false;
    bool blackKingsideRookMoved = false, blackQueensideRookMoved = false;

    int enPassantRow = -1;
    int enPassantCol = -1;

    std::vector<Piece> capturedWhite;
    std::vector<Piece> capturedBlack;

    ChessBoard();
    bool isWhitePiece(const Piece& piece) const;
    bool isBlackPiece(const Piece& piece) const;
    bool movePiece(int fromRow, int fromCol, int toRow, int toCol, bool whiteTurn);
    std::vector<std::pair<int, int>> getLegalMoves(int row, int col, bool whiteTurn) const;
    bool isSquareAttacked(int row, int col, bool byWhite) const;

    static int getPieceBaseValue(const std::string& name);
    static Piece fromString(const std::string& s);
    static std::string toString(const Piece& p);
};