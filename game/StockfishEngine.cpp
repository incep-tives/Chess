#include "StockfishEngine.h"

StockfishEngine::StockfishEngine() {}

StockfishEngine::~StockfishEngine() {
    if (Engine_Destroy && engine)
        Engine_Destroy(engine);
    if (dll)
        FreeLibrary(dll);
}

bool StockfishEngine::start(int elo) {
    dll = LoadLibraryA("Engine.dll");
    if (!dll) { MessageBoxA(nullptr, "Failed to load Engine.dll", "Error", MB_ICONERROR); return false; }

    Engine_Create = (Engine_Create_t)GetProcAddress(dll, "Engine_Create");
    Engine_Destroy = (Engine_Destroy_t)GetProcAddress(dll, "Engine_Destroy");
    Engine_SetElo = (Engine_SetElo_t)GetProcAddress(dll, "Engine_SetElo");
    Engine_SetPosition = (Engine_SetPosition_t)GetProcAddress(dll, "Engine_SetPosition");
    Engine_GetBestMove = (Engine_GetBestMove_t)GetProcAddress(dll, "Engine_GetBestMove");

    if (!Engine_Create || !Engine_Destroy || !Engine_SetElo || !Engine_SetPosition || !Engine_GetBestMove) {
        MessageBoxA(nullptr, "One or more Engine.dll exports not found", "Error", MB_ICONERROR);
        return false;
    }

    engine = Engine_Create();
    if (!engine) { MessageBoxA(nullptr, "Engine_Create failed", "Error", MB_ICONERROR); return false; }

    Engine_SetElo(engine, elo);

    return true;
}

std::string StockfishEngine::getBestMove(const std::string& fen, const std::vector<std::string>& moves) {
    if (!engine) return "";

    std::vector<const char*> c_moves;
    for (const auto& m : moves)
        c_moves.push_back(m.c_str());

    Engine_SetPosition(engine, fen.c_str(), c_moves.data(), (int)c_moves.size());
    const char* move = Engine_GetBestMove(engine, 1000);
    return move ? std::string(move) : "";
}