# ProcessLister

A command-line tool that lists every running process on the system with its process ID, parent process ID, thread count, and executable name.

## Example output

Values vary from system to system.

```
PID        PPID       Threads    Executable Name
----       -----      -------    ---------------
0          0          8          [System Process]
4          0          240        System
188        4          4          Registry
612        4          2          smss.exe
884        876        12         csrss.exe
...
```

## How it works

1. `CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)` takes a current snapshot of all processes.
2. `Process32FirstW` reads the first entry into a `PROCESSENTRY32W` struct, and `Process32NextW` advances through the rest.
3. Each entry's `th32ProcessID`, `th32ParentProcessID`, `cntThreads`, and `szExeFile` fields are printed in aligned columns.
4. When the loop ends, `GetLastError()` is checked for `ERROR_NO_MORE_FILES` to tell a normal end of the loop apart from a failure.

## APIs used

| API | Purpose |
|---|---|
| `CreateToolhelp32Snapshot` | Snapshot of all processes in the system |
| `Process32FirstW` / `Process32NextW` | Iterate over each process in the snapshot |
| `PROCESSENTRY32W` | Per-process data: PID, parent PID, thread count, executable name |
| `GetLastError` | Used for error handling and reporting |
| `CloseHandle` | Release the snapshot handle |

## Things I learned

**`A`/`W` variants and the `UNICODE` macro:** The first version used `PROCESSENTRY32` and printed the name with `%s`. Since Visual Studio projects define `UNICODE`, `tlhelp32.h` silently remapped `PROCESSENTRY32` to `PROCESSENTRY32W`, making `szExeFile` a wide string. `printf`'s `%s` expects a narrow string, so every name printed only its first character (UTF-16 stores a zero byte after each ASCII character). I fixed this by naming the wide variants explicitly (`PROCESSENTRY32W`, `Process32FirstW`, `Process32NextW`) and printing with `%ls`, so the code behaves the same regardless of build settings.

**End of list versus API failure:** `Process32NextW` returns `FALSE` when it runs out of entries AND when an error occurs. My implementation saves `GetLastError()` and treats anything other than `ERROR_NO_MORE_FILES` as a failure.

**Failure values differ between APIs:** `CreateToolhelp32Snapshot` returns `INVALID_HANDLE_VALUE` on failure instead of `NULL`.

**Initializing `dwSize`:** `dwSize` must be set to `sizeof(PROCESSENTRY32W)` before the first call, or `Process32FirstW` fails.
