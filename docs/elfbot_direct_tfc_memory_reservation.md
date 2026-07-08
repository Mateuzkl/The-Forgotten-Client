# ElfBot Direct TFC Memory Reservation

## Mode

- Default mode: `ELFBOT_COMPAT_DIRECT_TFC=1`
- Stub mode: `ELFBOT_COMPAT_USE_STUB=0`
- No `TibiaStub.exe` launch
- No `Local\TibiaShim860` IPC mapping

## Win32 image layout

Direct TFC mode cannot rely only on a TLS `VirtualAlloc` when the EXE is
linked at `0x10000000`. On this machine the loader/import path already
placed private heaps and NLS mappings inside `0x00400000..0x00800000`
before the EXE TLS callback ran, causing `VirtualAlloc(..., 0x00440000, ...)`
and `VirtualAlloc(..., 0x00630000, ...)` to fail with `GetLastError=487`.

- TFC Win32 image base: `0x00140000`
- Early reservation: `elfbot_shadow.cpp` maps `.text$aaa` from `0x00141000..0x00801000`
- ASLR: disabled with `RandomizedBaseAddress=false`
- Fixed base: enabled with `FixedBaseAddress=true`
- Target machine: `MachineX86`
- Link option: `/SECTION:.text,ERW`

This is still direct TFC mode: no `TibiaStub.exe`, no IPC, and no helper
process. The shadow section is part of the real `Tibia.exe` image and makes
the process own the old Tibia addresses before imported DLL initialization.

## Direct reservations

The image shadow owns the Tibia 8.60 ranges before the TLS callback. The TLS
callback verifies that ownership and only attempts `VirtualAlloc` as a fallback:

- Text/code/RSA range: `0x00440000..0x005D0000`, size `0x00190000`, protection `PAGE_EXECUTE_READWRITE`
- Data/client range: `0x00630000..0x007F0000`, size `0x001C0000`, protection `PAGE_READWRITE`

`ElfbotCompat::init()` verifies those reservations again, applies the required
page protections, and fails hard if they are not committed/writable.

## Required runtime probes

Startup logs must include:

- `[ElfBotDirect] 0x0063FE94 committed writable yes`
- `[ElfBotDirect] 0x0063FEF8 committed writable yes`
- `[ElfBotDirect] 0x00799F08 committed writable yes`

If either reservation fails, the log includes:

- `[ElfBotDirect] VirtualAlloc failed region text base=0x00440000 size=... GetLastError=...`
- `[ElfBotDirect] VirtualAlloc failed region data base=0x00630000 size=... GetLastError=...`

Do not fall back to Stub mode when this happens. Investigate which module
occupied the low range before the TLS callback.
