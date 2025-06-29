#include "library.h"
#include <windows.h>

extern "C" {

    DLL_EXPORT const void* Assets_LoadResource(int resId, const wchar_t* type, unsigned* outSize) {
        HMODULE hModule = GetModuleHandleW(L"Assets.dll");
        if (!hModule) hModule = GetModuleHandleW(nullptr);

        HRSRC hRes = FindResourceW(hModule, MAKEINTRESOURCEW(resId), type);
        if (!hRes) return nullptr;
        HGLOBAL hData = LoadResource(hModule, hRes);
        if (!hData) return nullptr;
        DWORD size = SizeofResource(hModule, hRes);
        if (outSize) *outSize = size;
        return LockResource(hData);
    }

}