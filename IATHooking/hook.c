#include <stdio.h>
#include <Windows.h>

// define MessageBoxW prototype
typedef int (WINAPI* MessageBoxW_t) (HWND, LPCWSTR, LPCWSTR, UINT);

// remember the address to the original MessageBoxW function
static MessageBoxW_t originalMessageBoxW = NULL;

// hooked function with additional behavior that will call the original MessageBoxW function
int WINAPI hookedMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	originalMessageBoxW(NULL, L"Get Hooked!", L"LMAO", MB_OK);

	// call the original MessageBoxW function
	return originalMessageBoxW(hWnd, lpText, lpCaption, uType);
}

int main() {

	// save original address
	originalMessageBoxW = MessageBoxW;

	// before IAT hooking
	MessageBoxW(NULL, L"Before IAT Hooking", L"Before Hooking", MB_OK);

	LPVOID imageBase = GetModuleHandleW(NULL);
	PIMAGE_DOS_HEADER dosHeaders = (PIMAGE_DOS_HEADER)imageBase;
	PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)imageBase + dosHeaders->e_lfanew);


	// after IAT hooking
	MessageBoxW(NULL, L"After IAT Hooking", L"After Hooking", MB_OK);
}
