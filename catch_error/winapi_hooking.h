#include "stdafx.h"
#pragma once

BOOL InstallHook(LPCSTR module, LPCSTR function, void* hook, void** original);

template<typename T>
BOOL InstallHookTyped(LPCSTR module, LPCSTR function, T proto, T hook, T* out_orig) {
	return InstallHook(module, function, hook, (void**)out_orig);
}

#define WinFuncHook(ret_type, name, ...)\
	ret_type(WINAPI* name##_base)(__VA_ARGS__) = 0;\
	ret_type WINAPI  name##_hook (__VA_ARGS__)
#define WinFuncHookSet(module, name)\
	if (!InstallHookTyped(module, #name, name, name##_hook, &name##_base)) trace("Failed to hook " #name "!");