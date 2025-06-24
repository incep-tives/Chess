cmake_minimum_required(VERSION 3.31)
project(FXChess)

set(CMAKE_CXX_STANDARD 23)

add_executable(FXChess main.cpp
        game/ChessBoard.cpp
        game/ChessBoard.h
        game/ChessGame.cpp
        game/ChessGame.h
        game/PromotionPopup.cpp
        game/ChessAnimation.cpp
        game/GameOverPopup.cpp
        icon.rc
)
set_target_properties(FXChess PROPERTIES WIN32_EXECUTABLE TRUE)

target_link_libraries(FXChess PRIVATE gdiplus winmm dwmapi)