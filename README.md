A collection of tools written in C using the Win32 API. The goal is to better understand Windows internals regarding processes, handles, tokens, memory, as well how APIs work with them.

# win-api-programming

Small Windows tools written in C against the Win32 API, built to learn how Windows works under the hood: processes, handles, memory, tokens, and the APIs that expose them.

Each tool is checked against an established reference tool (Task Manager, System Informer, WinDbg, Sysinternals) to confirm its output is correct.

## Tools

| Tool | Description | Key APIs |
|---|---|---|
| [ProcessLister](ProcessLister/) | Lists running processes with PID, parent PID, thread count, and executable name | `CreateToolhelp32Snapshot`, `Process32FirstW`, `Process32NextW` |

## Building

**Requirements:** Visual Studio with the **Desktop development with C++** workload (in order to support `.slnx` solution files).

**In Visual Studio:**

1. Open `win-api-programming.slnx`
2. Set the platform to **x64** and choose **Debug** or **Release**
3. **Build → Build Solution** (Ctrl+Shift+B)

**From the command line:**

```
msbuild win-api-programming.slnx /p:Configuration=Release /p:Platform=x64
```

Executables are written to `x64\<Configuration>\` in the repository root.

## Conventions

- Written in C (C17)
- Wide-character (`W`) APIs are explicitly called so behavior doesn't depend on whether `UNICODE` is defined
