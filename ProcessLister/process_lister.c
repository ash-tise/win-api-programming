#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>

int main() {

	int status = 0;

	// snapshot all processes in the system
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	// error check
	if (hSnapshot == INVALID_HANDLE_VALUE) {
		printf("CreateToolhelp32Snapshot Failed: %lu\n", GetLastError());
		return 1;
	}

	PROCESSENTRY32W pe32;
	pe32.dwSize = sizeof(pe32);

	// grab info for first process
	if (!Process32FirstW(hSnapshot, &pe32)) {
		printf("Process32First Failed: %lu\n", GetLastError());
		CloseHandle(hSnapshot);
		return 1;
	}

	// print header information
	printf("%-10s %-10s %-10s %s\n", "PID", "PPID", "Threads", "Executable Name");
	printf("%-10s %-10s %-10s %s\n", "----", "-----", "-------", "---------------");

	// list out each process in the snapshot
	do {
		printf("%-10lu %-10lu %-10lu %ls\n", pe32.th32ProcessID, pe32.th32ParentProcessID, pe32.cntThreads, pe32.szExeFile);
	} while (Process32NextW(hSnapshot, &pe32));

	// error check
	DWORD err = GetLastError();
	if (err != ERROR_NO_MORE_FILES) {
		printf("Process32Next failed: %lu\n", err);
		status = 1;
	}

	// close handle and exit
	CloseHandle(hSnapshot);
	return status;
}