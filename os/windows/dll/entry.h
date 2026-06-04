#ifndef ENTRY_H
#define ENTRY_H

typedef int BOOL;
typedef void* HINSTANCE;
typedef unsigned long DWORD;
typedef void* LPVOID;

__declspec(dllexport) BOOL __stdcall _DllMainCRTStartup(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved); // NOLINT(bugprone-reserved-identifier)

#endif
