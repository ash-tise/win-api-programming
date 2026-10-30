# ProcessLister

A command-line tool that lists every running process on the system with its process ID, parent process ID, thread count, executable name, and full executable path.

## Example output

Values vary from system to system.

```
PID        PPID       Threads    Executable Name                Full Path
----       -----      -------    ---------------                ---------
0          0          8          [System Process]               <error>: 87
4          0          240        System                         <access denied>
612        4          2          smss.exe                       <access denied>
884        876        12         csrss.exe                      <access denied>
5120       5088       61         explorer.exe                   C:\Windows\explorer.exe
...
```

## How it works

1. `CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)` takes a current snapshot of all processes.
2. `Process32FirstW` reads the first entry into a `PROCESSENTRY32W` struct, and `Process32NextW` advances through the rest.
3. For each process, a helper function, `GetImagePath`, opens the process with `OpenProcess` and retrieves its full path with `QueryFullProcessImageNameW`. If either call fails, it writes an error marker into the path column instead.
4. Each entry's `th32ProcessID`, `th32ParentProcessID`, `cntThreads`, `szExeFile`, and full path are printed in aligned columns.
5. When the loop ends, `GetLastError()` is checked for `ERROR_NO_MORE_FILES` to tell a normal end of the loop apart from a failure.

## APIs used

| API | Purpose |
|---|---|
| `CreateToolhelp32Snapshot` | Snapshot of all processes in the system |
| `Process32FirstW` / `Process32NextW` | Iterate over each process in the snapshot |
| `PROCESSENTRY32W` | Per-process data: PID, parent PID, thread count, executable name |
| `OpenProcess` | Open a handle to each process to query it |
| `QueryFullProcessImageNameW` | Retrieve a process's full executable path |
| `GetLastError` | Used for error handling and reporting |
| `CloseHandle` | Release the snapshot and process handles |

## Things I learned

**`A`/`W` variants and the `UNICODE` macro:** The first version used `PROCESSENTRY32` and printed the name with `%s`. Since Visual Studio projects define `UNICODE`, `tlhelp32.h` silently remapped `PROCESSENTRY32` to `PROCESSENTRY32W`, making `szExeFile` a wide string. `printf`'s `%s` expects a narrow string, so every name printed only its first character (UTF-16 stores a zero byte after each ASCII character). I fixed this by naming the wide variants explicitly (`PROCESSENTRY32W`, `Process32FirstW`, `Process32NextW`) and printing with `%ls`, so the code behaves the same regardless of build settings.

**End of list versus API failure:** `Process32NextW` returns `FALSE` when it runs out of entries AND when an error occurs. My implementation saves `GetLastError()` and treats anything other than `ERROR_NO_MORE_FILES` as a failure.

**Failure values differ between APIs:** `CreateToolhelp32Snapshot` returns `INVALID_HANDLE_VALUE` on failure while `OpenProcess` returns `NULL`.

**Initializing `dwSize`:** `dwSize` must be set to `sizeof(PROCESSENTRY32W)` before the first call or else `Process32FirstW` fails.

**OpenProcess:** If a request is denied when calling `OpenProcess` the entire call fails. Requesting `PROCESS_QUERY_LIMITED_INFORMATION` lets the tool get paths for more processes including some that are more protected.

**Always leave the buffer valid:** On failure, `GetImagePath` writes a marker such as `<access denied>` into the buffer. If this weren't the case, `main` would print uninitialized stack memory in the path column.
