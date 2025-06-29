#pragma once
#include <windows.h>
#include <string>
#include <vector>

class StockfishEngine {
public:
    StockfishEngine();
    ~StockfishEngine();

    bool start(int elo);
    std::string getBestMove(const std::string& fen, const std::vector<std::string>& moves);

private:
    HMODULE dll = nullptr;
    void* engine = nullptr;

    using Engine_Create_t = void* (*)();
    using Engine_Destroy_t = void (*)(void*);
    using Engine_SetElo_t = void (*)(void*, int);
    using Engine_SetPosition_t = void (*)(void*, const char*, const char**, int);
    using Engine_GetBestMove_t = const char* (*)(void*, int);

    Engine_Create_t Engine_Create = nullptr;
    Engine_Destroy_t Engine_Destroy = nullptr;
    Engine_SetElo_t Engine_SetElo = nullptr;
    Engine_SetPosition_t Engine_SetPosition = nullptr;
    Engine_GetBestMove_t Engine_GetBestMove = nullptr;
};