#pragma once

#ifdef _WIN32
#define DLL_EXPORT __declspec(dllexport)
#else
#define DLL_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif
    DLL_EXPORT const void* Assets_LoadResource(int resId, const wchar_t* type, unsigned* outSize);

#ifdef __cplusplus
}
#endif