#include <stdio.h>
#include <string.h>
#include <Windows.h>

// define MessageBoxW prototype
typedef int (WINAPI* MessageBoxW_t) (HWND, LPCWSTR, LPCWSTR, UINT);

// remember the address to the original MessageBoxW function
static MessageBoxW_t originalMessageBoxW = NULL;

// hooked function with additional behavior that will call the original MessageBoxW function
int WINAPI hookedMessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType)
{
	// add your own code here
	originalMessageBoxW(NULL, L"Get Hooked!", L"LMAO", MB_OK);

	// call the original MessageBoxW function
	return originalMessageBoxW(hWnd, lpText, lpCaption, uType);
}

int main() {

	// save original address
	originalMessageBoxW = MessageBoxW;

	// before IAT hooking
	MessageBoxW(NULL, L"Before IAT Hooking", L"Before Hooking", MB_OK);

	// get the base address of the current module
	LPVOID imageBase = GetModuleHandleW(NULL);
	// get DOS header
	PIMAGE_DOS_HEADER dosHeaders = (PIMAGE_DOS_HEADER)imageBase;
	// get NT headers
	PIMAGE_NT_HEADERS ntHeaders = (PIMAGE_NT_HEADERS)((DWORD_PTR)imageBase + dosHeaders->e_lfanew);

	// get import descriptor for the current module
	PIMAGE_IMPORT_DESCRIPTOR importDescriptor = NULL;
	IMAGE_DATA_DIRECTORY importsDirectory = ntHeaders->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	importDescriptor = (PIMAGE_IMPORT_DESCRIPTOR)((DWORD_PTR)imageBase + importsDirectory.VirtualAddress);

	while (importDescriptor->Name != 0)
	{

		PIMAGE_THUNK_DATA originalFirstThunk = NULL, firstThunk = NULL;
		originalFirstThunk = (PIMAGE_THUNK_DATA)((DWORD_PTR)imageBase + importDescriptor->OriginalFirstThunk);
		firstThunk = (PIMAGE_THUNK_DATA)((DWORD_PTR)imageBase + importDescriptor->FirstThunk);

		while (originalFirstThunk->u1.AddressOfData != 0)
		{
			// ensure there is a name to compare
			if (!IMAGE_SNAP_BY_ORDINAL(originalFirstThunk->u1.Ordinal))
			{

				PIMAGE_IMPORT_BY_NAME functionName = (PIMAGE_IMPORT_BY_NAME)((DWORD_PTR)imageBase + originalFirstThunk->u1.AddressOfData);

				// find MessageBoxW in IAT
				if (strcmp(functionName->Name, "MessageBoxW") == 0)
				{
					DWORD oldProtect = 0;
					VirtualProtect((LPVOID)(&firstThunk->u1.Function), 8, PAGE_READWRITE, &oldProtect);

					// replace the address with our new function
					firstThunk->u1.Function = (DWORD_PTR)hookedMessageBoxW;

					// restore original protection
					VirtualProtect((LPVOID)(&firstThunk->u1.Function), 8, oldProtect, &oldProtect);
				}
			}
			originalFirstThunk++;
			firstThunk++;
		}
		importDescriptor++;
	}

	// after IAT hooking
	MessageBoxW(NULL, L"After IAT Hooking", L"After Hooking", MB_OK);

	return 0;
}
