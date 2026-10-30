#include <Windows.h>
#include <TlHelp32.h>
#include <stdio.h>

static BOOL GetImagePath(DWORD pid, WCHAR* buf, DWORD cchbuf) {
	// get process handle
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
	if (hProcess == NULL) {
		DWORD err = GetLastError();
		if (err == ERROR_ACCESS_DENIED) {
			swprintf_s(buf, cchbuf, L"<access denied>");
		}
		else {
			swprintf_s(buf, cchbuf, L"<error>: %lu", err);
		}
		return FALSE;
	}

	DWORD cch = cchbuf;

	// grab full path to the executable
	if (!QueryFullProcessImageNameW(hProcess, 0, buf, &cch)) {
		DWORD err = GetLastError();
		if (err == ERROR_ACCESS_DENIED) {
			swprintf_s(buf, cchbuf, L"<access denied>");
		}
		else {
			swprintf_s(buf, cchbuf, L"<error>: %lu", err);
		}
		CloseHandle(hProcess);
		return FALSE;
	}
	CloseHandle(hProcess);
	return TRUE;
}

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
	printf("%-10s %-10s %-10s %-30s %s\n", "PID", "PPID", "Threads", "Executable Name", "Full Path");
	printf("%-10s %-10s %-10s %-30s %s\n", "----", "-----", "-------", "---------------", "---------");

	// list out each process in the snapshot
	do {

		WCHAR path[MAX_PATH];
		GetImagePath(pe32.th32ProcessID, path, _countof(path));
		printf("%-10lu %-10lu %-10lu %-30ls %ls\n", pe32.th32ProcessID, pe32.th32ParentProcessID, pe32.cntThreads, pe32.szExeFile, path);
	
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