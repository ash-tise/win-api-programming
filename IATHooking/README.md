# IATHooking

A program that hooks `MessageBoxW` in its own process by overwriting its entry in the Import Address Table (IAT). After the hook is installed every call the program makes to `MessageBoxW` first runs my hooked function. After my added code runs, then the real `MessageBoxW` runs.

This code is based on the [ired.team IAT hooking walkthrough](https://www.ired.team/offensive-security/code-injection-process-injection/import-adress-table-iat-hooking), rewritten in C.

## Purpose

The program shows three message boxes:

1. **"Before IAT Hooking"**: a normal call to the function, before the hook exists.
2. **"Get Hooked!"**: shown by the hook function when the hooked call is made.
3. **"After IAT Hooking"**: the original call, passed through by the hook to the real `MessageBoxW`.

`main` calls `MessageBoxW` the same way before and after hooking. What changes is the address stored in the IAT slot.

## How it works

1. `GetModuleHandleW(NULL)` returns the executable's base address in memory.
2. The DOS header (`IMAGE_DOS_HEADER`) at the base gives `e_lfanew`, the offset to the NT headers (`IMAGE_NT_HEADERS`).
3. The optional header's data directories give the RVA of the import table which is an array of `IMAGE_IMPORT_DESCRIPTOR`s. There's one per imported DLL.
4. For each DLL, there are two parallel thunk arrays that are walked together:
   - `OriginalFirstThunk` (the Import Name Table) holds each function's name.
   - `FirstThunk` (the Import Address Table) holds each function's resolved address.
5. Once the function with name `MessageBoxW` is found, the corresponding IAT slot is made writable with `VirtualProtect`. I then overwrite it with the address of `hookedMessageBoxW`. Its original protection is then restored.
6. The hook does whatever work I want it to do, then calls the real `MessageBoxW` through a saved function pointer.

Every Relative Virtual Address (RVA) in the PE structures is an offset from the image base, so each one is converted to a usable pointer with `base + RVA`.

## APIs and structures used

| Name | Purpose |
|---|---|
| `GetModuleHandleW` | Base address of the module |
| `IMAGE_DOS_HEADER` | `e_lfanew`: offset to the NT headers |
| `IMAGE_NT_HEADERS` | Optional header and its data directories |
| `IMAGE_IMPORT_DESCRIPTOR` | One entry per imported DLL |
| `IMAGE_THUNK_DATA` | One entry per imported function (INT and IAT) |
| `IMAGE_IMPORT_BY_NAME` | Gets an imported function's name |
| `VirtualProtect` | Temporarily makes the IAT slot writable |

## Cool things I learned

- Calling `MessageBoxW` inside `hookedMessageBoxW` goes back through the hooked IAT slot into the hook itself, recursing until the stack overflows. The hook calls the real function through the `originalMessageBoxW` pointer.

- `firstThunk->u1.Function` is the value in the IAT slot (a function's address) and `&firstThunk->u1.Function` is where the slot is stored. I forgot the `&` and realized `VirtualProtect` needs the slot's location.

- Some functions are imported by number instead of by name. For those, the thunk's top bit is set and `AddressOfData` is not a valid RVA. `IMAGE_SNAP_BY_ORDINAL` is checked to ensure there's a name to read.

- Adding a byte offset like `e_lfanew` requires an integer type (`DWORD_PTR`), because adding to a typed pointer scales by the size of the type. That same scaling is what makes `firstThunk++` move to the next 8-byte thunk.

## Limitations of IAT hooking

An IAT hook only catches calls that go through this module's IAT. It does not affect:

- Calls through addresses obtained with `GetProcAddress` because the call does not go through the IAT slot. The address is read directly from the DLL's export table and is stored in the caller's variable.
- Calls made by other DLLs through their own import tables. We are only patching the IAT of our current module.
- Calls to `MessageBoxA` because it's a separate function and therefore has its own IAT slot that would need to be patched.
- Code that calls a lower-level function beneath the hooked function because it'll skip calling the higher-level function (that we hooked) entirely.