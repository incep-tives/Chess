#define UNICODE
#define _UNICODE
#define ID_FILE_CLOSE 1001
#define ID_NEW_GAME   1002
#define ID_ABOUT      1003
#define ID_SAVE_GAME 1004
#define ID_OPEN_GAME 1005

#include <windows.h>
#include <dwmapi.h>
#include <windowsx.h>
#include <algorithm>
#include <gdiplus.h>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include "game/ChessGame.h"
#include "game/PromotionPopup.h"
#include "game/ChessAnimation.h"
#include <filesystem>
#include <iostream>
#include <mmsystem.h>
#include <ntdef.h>
#include "game/GameOverPopup.h"
#pragma comment(lib, "winmm.lib")
#pragma comment (lib,"Gdiplus.lib")
#pragma comment(lib, "dwmapi.lib")
using namespace Gdiplus;

const int BOARD_SIZE = 8;
GameOverPopup gameOverPopup;

#include "game/SingleplayerController.h"
SingleplayerController singleplayerController;

ChessGame singleplayerGame;

enum class GameMode { None, Singleplayer, Multiplayer };
GameMode gameMode = GameMode::None;
int stockfishElo = 1350;

PlayerColor playerColor = PlayerColor::None;

std::string GetFEN(const ChessGame& game) {
    return "startpos";
}

std::string ToUCIMove(int fromRow, int fromCol, int toRow, int toCol) {
    char files[] = "abcdefgh";
    char ranks[] = "87654321";
    std::string move;
    move += files[fromCol];
    move += ranks[fromRow];
    move += files[toCol];
    move += ranks[toRow];
    return move;
}

std::vector<std::string> GetMoveList(const ChessGame& game) {

    return {};
}


bool ApplyUCIMove(ChessGame& game, const std::string& uciMove) {

    return false;
}

PlayerColor ShowColorDialog(HWND parent) {
    int res = MessageBoxW(parent, L"Choose your color:\nYes = White\nNo = Black", L"Choose Color", MB_YESNO | MB_ICONQUESTION);
    if (res == IDYES) return PlayerColor::White;
    if (res == IDNO) return PlayerColor::Black;
    return PlayerColor::None;
}

int ShowDifficultyDialog(HWND parent) {
    const wchar_t* msg = L"Choose difficulty:\nYes = Easy\nNo = Medium\nCancel = Hard\nCustom = Custom";
    int res = MessageBoxW(parent, msg, L"Difficulty", MB_YESNOCANCEL | MB_ICONQUESTION | MB_DEFBUTTON1);
    if (res == IDYES) return 650;
    if (res == IDNO) return 1250;
    if (res == IDCANCEL) return 1750;
    wchar_t buf[16] = L"1350";
    if (DialogBoxParamW(nullptr, MAKEINTRESOURCEW(101), parent, [](HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam) -> INT_PTR {
        if (msg == WM_INITDIALOG) {
            SetDlgItemTextW(dlg, 100, (LPCWSTR)lParam);
            return TRUE;
        }
        if (msg == WM_COMMAND) {
            if (LOWORD(wParam) == IDOK) {
                EndDialog(dlg, 1);
                return TRUE;
            }
            if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(dlg, 0);
                return TRUE;
            }
        }
        return FALSE;
    }, (LPARAM)buf)) {
        GetDlgItemTextW(GetActiveWindow(), 100, buf, 16);
        return _wtoi(buf);
    }
    return 1350;
}

GameMode ShowGameModeDialog(HWND parent) {
    int res = MessageBoxW(parent, L"Choose game mode:\nYes = Singleplayer\nNo = Multiplayer", L"Game Mode", MB_YESNOCANCEL | MB_ICONQUESTION);
    if (res == IDYES) return GameMode::Singleplayer;
    if (res == IDNO) return GameMode::Multiplayer;
    return GameMode::None;
}

int ShowEloDialog(HWND parent) {
    wchar_t buf[16] = L"1350";
    if (DialogBoxParamW(nullptr, MAKEINTRESOURCEW(101), parent, [](HWND dlg, UINT msg, WPARAM wParam, LPARAM lParam) -> INT_PTR {
        if (msg == WM_INITDIALOG) {
            SetDlgItemTextW(dlg, 100, (LPCWSTR)lParam);
            return TRUE;
        }
        if (msg == WM_COMMAND) {
            if (LOWORD(wParam) == IDOK) {
                EndDialog(dlg, 1);
                return TRUE;
            }
            if (LOWORD(wParam) == IDCANCEL) {
                EndDialog(dlg, 0);
                return TRUE;
            }
        }
        return FALSE;
    }, (LPARAM)buf)) {
        GetDlgItemTextW(GetActiveWindow(), 100, buf, 16);
        return _wtoi(buf);
    }
    return 1350;
}

std::unique_ptr<Image> boardImage;
std::map<std::string, std::unique_ptr<Image>>& getPieceImages() {
    static std::map<std::string, std::unique_ptr<Image>> pieceImages;
    return pieceImages;
}
ChessGame game;
PromotionPopup promotion;
ChessAnimation animState;

struct PendingMove {
    int fromRow = -1, fromCol = -1, toRow = -1, toCol = -1;
    std::string piece;
    bool isPending = false;
};
PendingMove pendingMove;

LARGE_INTEGER freqQPC{};

void LoadImages() {
    boardImage = std::make_unique<Image>(L"assets\\images\\chessboard.jpg");
    std::vector<std::string> pieces = {
        "white_king", "white_queen", "white_rook", "white_bishop", "white_knight", "white_pawn",
        "black_king", "black_queen", "black_rook", "black_bishop", "black_knight", "black_pawn"
    };
    for (const auto& piece : pieces) {
        std::wstring wpath = L"assets\\images\\";
        wpath += std::wstring(piece.begin(), piece.end());
        wpath += L".png";
        auto img = std::make_unique<Image>(wpath.c_str());
        if (img->GetLastStatus() != Ok) {
            MessageBoxW(nullptr, (L"Failed to load " + wpath).c_str(), L"Error", MB_ICONERROR);
        }
        getPieceImages()[piece] = std::move(img);
    }
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    static int boardDrawSize = 500;


    ChessGame& currentGame = (gameMode == GameMode::Singleplayer) ? singleplayerGame : game;

    switch (msg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_SIZE: {
            int w = LOWORD(lParam);
            int h = HIWORD(lParam);
            boardDrawSize = (w < h ? w : h);
            InvalidateRect(hwnd, nullptr, TRUE);
            break;
        }
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_NEW_GAME:
                    if (gameMode == GameMode::Singleplayer) {
                        singleplayerGame = ChessGame();
                        singleplayerController.moveHistory.clear();
                        singleplayerController.start(playerColor, stockfishElo);
                    } else {
                        game = ChessGame();
                    }
                    promotion.reset();
                    gameOverPopup.result = GameResult::None;
                    InvalidateRect(hwnd, nullptr, TRUE);
                    break;
                case ID_FILE_CLOSE:
                    PostQuitMessage(0);
                    break;
                case ID_ABOUT:
                    MessageBoxW(hwnd, L"Chess Game\nby Noctify", L"About", MB_OK | MB_ICONINFORMATION);
                    break;
                case ID_SAVE_GAME: {
                    wchar_t fileName[MAX_PATH] = L"";
                    OPENFILENAMEW ofn = { sizeof(ofn) };
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFilter = L"Chess Files (*.chess)\0*.chess\0";
                    ofn.lpstrFile = fileName;
                    ofn.nMaxFile = MAX_PATH;
                    ofn.Flags = OFN_OVERWRITEPROMPT;
                    ofn.lpstrDefExt = L"chess";
                    if (GetSaveFileNameW(&ofn)) {
                        std::wofstream ofs(fileName);
                        if (!ofs) {
                            MessageBoxW(hwnd, L"Failed to save file.", L"Error", MB_ICONERROR);
                            break;
                        }
                        ofs << (currentGame.whiteTurn ? L"white" : L"black") << L"\n";
                        for (int r = 0; r < 8; ++r) {
                            for (int c = 0; c < 8; ++c) {
                                const Piece& p = currentGame.board.board[r][c];
                                if (!p.empty()) {
                                    ofs << L"piece " << r << L" " << c << L" "
                                        << std::wstring(p.name.begin(), p.name.end()) << L" "
                                        << p.value << L"\n";
                                }
                            }
                        }
                        for (const auto& p : currentGame.board.capturedWhite) {
                            ofs << L"captured_white "
                                << std::wstring(p.name.begin(), p.name.end()) << L" "
                                << p.value << L"\n";
                        }
                        for (const auto& p : currentGame.board.capturedBlack) {
                            ofs << L"captured_black "
                                << std::wstring(p.name.begin(), p.name.end()) << L" "
                                << p.value << L"\n";
                        }
                        ofs << L"lastmove "
                            << currentGame.lastMoveFromRow << L" " << currentGame.lastMoveFromCol << L" "
                            << currentGame.lastMoveToRow << L" " << currentGame.lastMoveToCol << L"\n";
                        ofs.close();
                        MessageBoxW(hwnd, L"Game saved successfully.", L"", MB_OK | MB_ICONINFORMATION);
                    }
                    break;
                }
                case ID_OPEN_GAME: {
                    wchar_t fileName[MAX_PATH] = L"";
                    OPENFILENAMEW ofn = { sizeof(ofn) };
                    ofn.hwndOwner = hwnd;
                    ofn.lpstrFilter = L"Chess Files (*.chess)\0*.chess\0";
                    ofn.lpstrFile = fileName;
                    ofn.nMaxFile = MAX_PATH;
                    ofn.Flags = OFN_FILEMUSTEXIST;
                    ofn.lpstrDefExt = L"chess";
                    if (GetOpenFileNameW(&ofn)) {
                        std::wifstream ifs(fileName);
                        if (!ifs) {
                            MessageBoxW(hwnd, L"Failed to open file.", L"Error", MB_ICONERROR);
                            break;
                        }
                        currentGame = ChessGame();
                        for (int r = 0; r < 8; ++r)
                            for (int c = 0; c < 8; ++c)
                                currentGame.board.board[r][c] = Piece();
                        promotion.reset();
                        gameOverPopup.result = GameResult::None;

                        std::wstring line;
                        while (std::getline(ifs, line)) {
                            std::wistringstream iss(line);
                            std::wstring token;
                            iss >> token;
                            if (token == L"white") {
                                currentGame.whiteTurn = true;
                            } else if (token == L"black") {
                                currentGame.whiteTurn = false;
                            } else if (token == L"piece") {
                                int r, c, value;
                                std::wstring name;
                                iss >> r >> c >> name >> value;
                                std::string sname(name.begin(), name.end());
                                int color = (sname.find("white") != std::string::npos) ? 1 : 0;
                                currentGame.board.board[r][c] = Piece(sname, color);
                                currentGame.board.board[r][c].value = value;
                            } else if (token == L"captured_white") {
                                std::wstring name;
                                int value;
                                iss >> name >> value;
                                std::string sname(name.begin(), name.end());
                                currentGame.board.capturedWhite.push_back(Piece(sname, 1));
                                currentGame.board.capturedWhite.back().value = value;
                            } else if (token == L"captured_black") {
                                std::wstring name;
                                int value;
                                iss >> name >> value;
                                std::string sname(name.begin(), name.end());
                                currentGame.board.capturedBlack.push_back(Piece(sname, 0));
                                currentGame.board.capturedBlack.back().value = value;
                            } else if (token == L"lastmove") {
                                iss >> currentGame.lastMoveFromRow >> currentGame.lastMoveFromCol >> currentGame.lastMoveToRow >> currentGame.lastMoveToCol;
                            }
                        }
                        ifs.close();
                        InvalidateRect(hwnd, nullptr, TRUE);
                        MessageBoxW(hwnd, L"Game loaded successfully.", L"", MB_OK | MB_ICONINFORMATION);
                    }
                    break;
                }
            }
            break;
        case WM_LBUTTONDOWN: {
            int x = LOWORD(lParam), y = HIWORD(lParam);
            int barHeight = 28;

            RECT rect;
            GetClientRect(hwnd, &rect);
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;
            int boardDrawSize = height - 2 * barHeight;
            int boardY = barHeight;
            float squareSize = static_cast<float>(boardDrawSize) / BOARD_SIZE;
            int col = int(x / squareSize);
            int row = int((y - barHeight) / squareSize);
            if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) break;

            if (gameOverPopup.result != GameResult::None) break;
            if (promotion.handleClick(x, y, squareSize, currentGame, gameOverPopup)) {
                InvalidateRect(hwnd, nullptr, TRUE);
                break;
            }

            if (gameMode == GameMode::Singleplayer && singleplayerController.isBotTurn(currentGame)) {
                break;
            }

            Piece& clickedPiece = currentGame.board.board[row][col];
            bool isOwnPiece = (currentGame.whiteTurn && currentGame.board.isWhitePiece(clickedPiece)) ||
                              (!currentGame.whiteTurn && currentGame.board.isBlackPiece(clickedPiece));

            if (currentGame.selectedRow != -1 && currentGame.selectedCol != -1) {
                if (currentGame.selectedRow == row && currentGame.selectedCol == col) {
                    currentGame.resetSelection();
                    InvalidateRect(hwnd, nullptr, TRUE);
                    break;
                }
                if (isOwnPiece) {
                    currentGame.selectOrMove(row, col);
                    InvalidateRect(hwnd, nullptr, TRUE);
                    break;
                }
                auto moves = currentGame.board.getLegalMoves(currentGame.selectedRow, currentGame.selectedCol, currentGame.whiteTurn);
                std::vector<std::pair<int, int>> legalMoves;
                for (const auto& move : moves) {
                    ChessBoard tempBoard = currentGame.board;
                    if (tempBoard.movePiece(currentGame.selectedRow, currentGame.selectedCol, move.first, move.second, currentGame.whiteTurn)) {
                        int kingRow = -1, kingCol = -1;
                        for (int r = 0; r < 8; ++r)
                            for (int c = 0; c < 8; ++c) {
                                const Piece& p = tempBoard.board[r][c];
                                if ((currentGame.whiteTurn && p.name == "white_king") || (!currentGame.whiteTurn && p.name == "black_king")) {
                                    kingRow = r;
                                    kingCol = c;
                                }
                            }
                        if (!tempBoard.isSquareAttacked(kingRow, kingCol, !currentGame.whiteTurn))
                            legalMoves.push_back(move);
                    }
                }
                bool isLegal = false;
                for (const auto& move : legalMoves) {
                    if (move.first == row && move.second == col) {
                        isLegal = true;
                        break;
                    }
                }
                if (isLegal) {
                    Piece& moving = currentGame.board.board[currentGame.selectedRow][currentGame.selectedCol];
                    if (moving.name.find("pawn") != std::string::npos &&
                        ((currentGame.whiteTurn && row == 0) || (!currentGame.whiteTurn && row == 7))) {
                        promotion.start(row, col, currentGame.whiteTurn);
                        currentGame.lastMoveFromRow = currentGame.selectedRow;
                        currentGame.lastMoveFromCol = currentGame.selectedCol;
                        currentGame.lastMoveToRow = row;
                        currentGame.lastMoveToCol = col;
                        currentGame.resetSelection();
                        InvalidateRect(hwnd, nullptr, TRUE);
                        break;
                    }
                    currentGame.board.movePiece(currentGame.selectedRow, currentGame.selectedCol, row, col, currentGame.whiteTurn);
                    currentGame.lastMoveFromRow = currentGame.selectedRow;
                    currentGame.lastMoveFromCol = currentGame.selectedCol;
                    currentGame.lastMoveToRow = row;
                    currentGame.lastMoveToCol = col;
                    currentGame.resetSelection();
                    currentGame.whiteTurn = !currentGame.whiteTurn;
                    gameOverPopup.checkGameEnd(currentGame);

                    if (gameMode == GameMode::Singleplayer) {
                        std::string uciMove = ToUCIMove(currentGame.lastMoveFromRow, currentGame.lastMoveFromCol, currentGame.lastMoveToRow, currentGame.lastMoveToCol);
                        singleplayerController.moveHistory.push_back(uciMove);
                        singleplayerController.onPlayerMove(currentGame, gameOverPopup, hwnd);
                    }

                    bool isWhite = currentGame.whiteTurn;
                    currentGame.inCheck = false;
                    for (int r = 0; r < 8; ++r)
                        for (int c = 0; c < 8; ++c) {
                            const Piece& p = currentGame.board.board[r][c];
                            if ((isWhite && p.name == "white_king") || (!isWhite && p.name == "black_king")) {
                                if (currentGame.board.isSquareAttacked(r, c, !isWhite)) {
                                    currentGame.inCheck = true;
                                    currentGame.checkedKingRow = r;
                                    currentGame.checkedKingCol = c;
                                    currentGame.flashCheck = true;
                                    currentGame.flashStart = GetTickCount();
                                    PlaySoundW(L"assets\\sounds\\illegal.wav", nullptr, SND_FILENAME | SND_ASYNC);
                                }
                            }
                        }
                    InvalidateRect(hwnd, nullptr, TRUE);
                } else {
                    PlaySoundW(L"assets\\sounds\\illegal.wav", nullptr, SND_FILENAME | SND_ASYNC);
                    currentGame.resetSelection();
                    InvalidateRect(hwnd, nullptr, TRUE);
                }
                break;
            }
            if (isOwnPiece) {
                currentGame.selectOrMove(row, col);
                InvalidateRect(hwnd, nullptr, TRUE);
            }
            break;
        }
        case WM_MOUSEMOVE:
        case WM_LBUTTONUP:
            break;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            RECT rect;
            GetClientRect(hwnd, &rect);
            int width = rect.right - rect.left;
            int height = rect.bottom - rect.top;

            const int barHeight = 28;
            const int boardDrawSize = height - 2 * barHeight;
            const int boardY = barHeight;
            const float squareSize = static_cast<float>(boardDrawSize) / BOARD_SIZE;

            HDC memDC = CreateCompatibleDC(hdc);
            HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
            HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

            Graphics graphics(memDC);

            graphics.SetSmoothingMode(SmoothingModeHighQuality);
            graphics.SetInterpolationMode(InterpolationModeHighQualityBicubic);
            graphics.SetCompositingQuality(CompositingQualityHighQuality);
            graphics.SetPixelOffsetMode(PixelOffsetModeHighQuality);
            graphics.SetTextRenderingHint(TextRenderingHintClearTypeGridFit);

            if (boardImage) {
                graphics.DrawImage(boardImage.get(), 0, boardY, boardDrawSize, boardDrawSize);
            }

            if (currentGame.lastMoveFromRow != -1 && currentGame.lastMoveFromCol != -1 &&
                currentGame.lastMoveToRow != -1 && currentGame.lastMoveToCol != -1) {
                Color highlightColor(180, 246, 246, 105);
                SolidBrush highlightBrush(highlightColor);
                graphics.FillRectangle(
                    &highlightBrush,
                    static_cast<REAL>(currentGame.lastMoveFromCol * squareSize),
                    static_cast<REAL>(currentGame.lastMoveFromRow * squareSize + boardY),
                    static_cast<REAL>(squareSize),
                    static_cast<REAL>(squareSize)
                );
                graphics.FillRectangle(
                    &highlightBrush,
                    static_cast<REAL>(currentGame.lastMoveToCol * squareSize),
                    static_cast<REAL>(currentGame.lastMoveToRow * squareSize + boardY),
                    static_cast<REAL>(squareSize),
                    static_cast<REAL>(squareSize)
                );
            }

            if (currentGame.inCheck && currentGame.checkedKingRow != -1 && currentGame.checkedKingCol != -1) {
                Color color = Color(180, 255, 0, 0);
                SolidBrush brush(color);
                graphics.FillRectangle(
                    &brush,
                    static_cast<REAL>(currentGame.checkedKingCol * squareSize),
                    static_cast<REAL>(currentGame.checkedKingRow * squareSize + boardY),
                    static_cast<REAL>(squareSize),
                    static_cast<REAL>(squareSize)
                );
            }

            if (currentGame.selectedRow != -1 && currentGame.selectedCol != -1) {
                float outlineThickness = 4.0f;
                Color outlineColor = Color(255, 0, 180, 255);
                Pen outlinePen(outlineColor, outlineThickness);
                outlinePen.SetAlignment(PenAlignmentInset);
                graphics.DrawRectangle(
                    &outlinePen,
                    static_cast<REAL>(currentGame.selectedCol * squareSize + outlineThickness / 2),
                    static_cast<REAL>(currentGame.selectedRow * squareSize + outlineThickness / 2 + boardY),
                    static_cast<REAL>(squareSize - outlineThickness),
                    static_cast<REAL>(squareSize - outlineThickness)
                );
            }

            if (currentGame.selectedRow != -1 && currentGame.selectedCol != -1) {
                auto moves = currentGame.board.getLegalMoves(currentGame.selectedRow, currentGame.selectedCol, currentGame.whiteTurn);
                std::vector<std::pair<int, int>> legalMoves;
                for (const auto& move : moves) {
                    ChessBoard tempBoard = currentGame.board;
                    if (tempBoard.movePiece(currentGame.selectedRow, currentGame.selectedCol, move.first, move.second, currentGame.whiteTurn)) {
                        int kingRow = -1, kingCol = -1;
                        for (int r = 0; r < 8; ++r)
                            for (int c = 0; c < 8; ++c) {
                                const Piece& p = tempBoard.board[r][c];
                                if ((currentGame.whiteTurn && p.name == "white_king") || (!currentGame.whiteTurn && p.name == "black_king")) {
                                    kingRow = r;
                                    kingCol = c;
                                }
                            }
                        if (!tempBoard.isSquareAttacked(kingRow, kingCol, !currentGame.whiteTurn))
                            legalMoves.push_back(move);
                    }
                }
                Color moveDotColor(128, 80, 80, 80);
                SolidBrush moveDotBrush(moveDotColor);
                float dotRadius = squareSize * 0.18f;
                for (const auto& move : legalMoves) {
                    float cx = move.second * squareSize + squareSize / 2.0f;
                    float cy = move.first * squareSize + squareSize / 2.0f + boardY;
                    graphics.FillEllipse(
                        &moveDotBrush,
                        static_cast<REAL>(cx - dotRadius),
                        static_cast<REAL>(cy - dotRadius),
                        static_cast<REAL>(dotRadius * 2),
                        static_cast<REAL>(dotRadius * 2)
                    );
                }
            }
            for (int i = 0; i < BOARD_SIZE; ++i) {
                for (int j = 0; j < BOARD_SIZE; ++j) {
                    const Piece& piece = currentGame.board.board[i][j];
                    if (!piece.empty() && getPieceImages()[piece.name]) {
                        graphics.DrawImage(
                            getPieceImages()[piece.name].get(),
                            static_cast<INT>(j * squareSize),
                            static_cast<INT>(i * squareSize + boardY),
                            static_cast<INT>(squareSize),
                            static_cast<INT>(squareSize)
                        );
                    }
                }
            }
            if (promotion.state == PromotionState::Choosing) {
                REAL popupX = static_cast<REAL>((promotion.col + 0.5f) * squareSize - squareSize / 2);
                REAL popupY = static_cast<REAL>(promotion.row * squareSize + boardY);
                SolidBrush bg(Color(220, 240, 240, 240));
                graphics.FillRectangle(&bg, popupX, popupY, static_cast<REAL>(squareSize), static_cast<REAL>(4 * squareSize));
                Pen border(Color(255, 80, 80, 80), 2);
                graphics.DrawRectangle(&border, popupX, popupY, static_cast<REAL>(squareSize), static_cast<REAL>(4 * squareSize));
                for (int i = 0; i < 4; ++i) {
                    if (getPieceImages()[promotion.options[i]]) {
                        graphics.DrawImage(
                            getPieceImages()[promotion.options[i]].get(),
                            popupX, popupY + static_cast<REAL>(i * squareSize),
                            static_cast<REAL>(squareSize), static_cast<REAL>(squareSize)
                        );
                    }
                }
            }
            int whiteScore = 0, blackScore = 0;
            for (int i = 0; i < BOARD_SIZE; ++i) {
                for (int j = 0; j < BOARD_SIZE; ++j) {
                    const Piece& piece = currentGame.board.board[i][j];
                    if (piece.empty()) continue;
                    if (piece.name.find("white") != std::string::npos)
                        whiteScore += piece.value;
                    else if (piece.name.find("black") != std::string::npos)
                        blackScore += piece.value;
                }
            }
            int scoreDiff = whiteScore - blackScore;

            const auto& blackCaptured = currentGame.board.capturedWhite;
            const auto& whiteCaptured = currentGame.board.capturedBlack;

            if (!blackCaptured.empty()) {
                float iconSize = barHeight - 8.0f;
                float iconSpacing = 4.0f;
                int numIcons = static_cast<int>(blackCaptured.size());
                float barPad = 10.0f;
                float barW = numIcons * iconSize + (numIcons - 1) * iconSpacing + 2 * barPad;
                float barH = iconSize + 8.0f;
                float barY = (barHeight - barH) / 2.0f;
                float barX = std::round((width - barW) / 2.0f);

                GraphicsPath barPath;
                barPath.AddArc(barX, barY, barH, barH, 90, 180);
                barPath.AddArc(barX + barW - barH, barY, barH, barH, 270, 180);
                barPath.CloseFigure();

                SolidBrush barBrush(Color(240, 80, 80, 80));
                Pen borderPen(Color(255, 180, 180, 180), 2.0f);
                graphics.FillPath(&barBrush, &barPath);
                graphics.DrawPath(&borderPen, &barPath);

                float x = barX + barPad;
                float y = barY + 4.0f;
                for (const auto& piece : blackCaptured) {
                    if (getPieceImages()[piece.name]) {
                        graphics.DrawImage(getPieceImages()[piece.name].get(), std::round(x), std::round(y), std::round(iconSize), std::round(iconSize));
                        x += iconSize + iconSpacing;
                    }
                }
                if (scoreDiff < 0) {
                    FontFamily fontFamily(L"Segoe UI");
                    Font font(&fontFamily, iconSize * 0.7f, FontStyleBold, UnitPixel);
                    SolidBrush brush(Color(255, 255, 255, 255));
                    WCHAR buf[8];
                    swprintf(buf, 8, L"+%d", -scoreDiff);
                    graphics.DrawString(buf, wcslen(buf), &font, PointF(width - 40.0f, y), &brush);
                }
            }

            if (!whiteCaptured.empty()) {
                float iconSize = barHeight - 8.0f;
                float iconSpacing = 4.0f;
                int numIcons = static_cast<int>(whiteCaptured.size());
                float barPad = 10.0f;
                float barW = numIcons * iconSize + (numIcons - 1) * iconSpacing + 2 * barPad;
                float barH = iconSize + 8.0f;
                float barY = height - barHeight + (barHeight - barH) / 2.0f;
                float barX = std::round((width - barW) / 2.0f);

                GraphicsPath barPath;
                barPath.AddArc(barX, barY, barH, barH, 90, 180);
                barPath.AddArc(barX + barW - barH, barY, barH, barH, 270, 180);
                barPath.CloseFigure();

                SolidBrush barBrush(Color(240, 80, 80, 80));
                Pen borderPen(Color(255, 180, 180, 180), 2.0f);
                graphics.FillPath(&barBrush, &barPath);
                graphics.DrawPath(&borderPen, &barPath);

                float x = barX + barPad;
                float y = barY + 4.0f;
                for (const auto& piece : whiteCaptured) {
                    if (getPieceImages()[piece.name]) {
                        graphics.DrawImage(getPieceImages()[piece.name].get(), std::round(x), std::round(y), std::round(iconSize), std::round(iconSize));
                        x += iconSize + iconSpacing;
                    }
                }
                if (scoreDiff > 0) {
                    FontFamily fontFamily(L"Segoe UI");
                    Font font(&fontFamily, iconSize * 0.7f, FontStyleBold, UnitPixel);
                    SolidBrush brush(Color(255, 255, 255, 255));
                    WCHAR buf[8];
                    swprintf(buf, 8, L"+%d", scoreDiff);
                    graphics.DrawString(buf, wcslen(buf), &font, PointF(width - 40.0f, y), &brush);
                }
            }

            BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);
            SelectObject(memDC, oldBitmap);
            DeleteObject(memBitmap);
            DeleteDC(memDC);
            EndPaint(hwnd, &ps);
            break;
        }
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);

    QueryPerformanceFrequency(&freqQPC);

    LoadImages();


    HICON hIcon = (HICON)LoadImageW(
        nullptr, L"icon.ico", IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE
    );

    HMENU hMenu = CreateMenu();
    HMENU hFileMenu = CreatePopupMenu();
    AppendMenuW(hFileMenu, MF_STRING, ID_FILE_CLOSE, L"Close");
    AppendMenuW(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, L"File");
    AppendMenuW(hMenu, MF_STRING, ID_NEW_GAME, L"New Game");
    AppendMenuW(hMenu, MF_STRING, ID_SAVE_GAME, L"Save Game");
    AppendMenuW(hMenu, MF_STRING, ID_OPEN_GAME, L"Open Game");
    AppendMenuW(hMenu, MF_STRING, ID_ABOUT, L"About");

    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"ChessWindow";
    wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = hIcon ? hIcon : LoadIcon(nullptr, IDI_APPLICATION);
    RegisterClass(&wc);

    RECT rc = { 0, 0, 700, 756 };
    AdjustWindowRectEx(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME, TRUE, 0);
    int winWidth = rc.right - rc.left;
    int winHeight = rc.bottom - rc.top;

    HWND hwnd = CreateWindowEx(
    0, L"ChessWindow", L"Chess",
    WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME,
    CW_USEDEFAULT, CW_USEDEFAULT, winWidth, winHeight,
    nullptr, hMenu, hInstance, nullptr
    );

    gameMode = ShowGameModeDialog(hwnd);
    if (gameMode == GameMode::None) return 0;
    if (gameMode == GameMode::Singleplayer) {
        playerColor = ShowColorDialog(hwnd);
        if (playerColor == PlayerColor::None) return 0;
        stockfishElo = ShowEloDialog(hwnd);
        singleplayerController.start(playerColor, stockfishElo);
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    return 0;
}