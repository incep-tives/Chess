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
        game/SingleplayerController.cpp
        game/StockfishEngine.cpp
        icon.rc
        stockfish/bitboard.cpp
        stockfish/evaluate.cpp
        stockfish/engine.cpp
        stockfish/misc.cpp
        stockfish/movegen.cpp
        stockfish/numa.h
        stockfish/perft.h
        stockfish/position.cpp
        stockfish/search.cpp
        stockfish/thread.cpp
        stockfish/tune.cpp
        stockfish/uci.cpp
        stockfish/ucioption.cpp
        stockfish/types.h
        stockfish/timeman.cpp
        stockfish/nnue/nnue_misc.cpp
        stockfish/nnue/nnue_feature_transformer.h
        stockfish/nnue/nnue_common.h
        stockfish/nnue/layers/affine_transform.h
        stockfish/nnue/layers/affine_transform_sparse_input.h
        stockfish/nnue/layers/clipped_relu.h
        stockfish/nnue/layers/simd.h
        stockfish/nnue/layers/sqr_clipped_relu.h
        stockfish/nnue/features/half_ka_v2_hm.cpp
        stockfish/incbin/incbin.h
        stockfish/nnue/nnue_architecture.h
        stockfish/thread_win32_osx.h
        stockfish/score.cpp
        stockfish/movepick.cpp
        stockfish/nnue/nnue_accumulator.cpp
        stockfish/memory.cpp
        stockfish/history.h
        stockfish/benchmark.cpp
        stockfish/tt.cpp
        stockfish/nnue/network.cpp
        stockfish/syzygy/tbprobe.cpp
)

add_library(Assets SHARED
        engine/library.cpp
        engine/library.h
        assets.rc
)

target_link_libraries(FXChess PRIVATE Assets)

target_include_directories(Assets PUBLIC
        assets/images
        assets/sounds
)

set_target_properties(FXChess PROPERTIES WIN32_EXECUTABLE TRUE)

add_custom_command(TARGET FXChess POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:FXChess>/assets
)

set_target_properties(Assets PROPERTIES
        OUTPUT_NAME "Assets"
        PREFIX ""
)

set_target_properties(FXChess PROPERTIES
        OUTPUT_NAME "Chess"
        PREFIX ""
)

set_target_properties(Assets PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

target_link_libraries(FXChess PRIVATE gdiplus winmm dwmapi)