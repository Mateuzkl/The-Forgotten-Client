# ElfBot compatibility current state

Branch base: `edbf8424643d7a796f88c5082313098736b18feb`.

## What works today

- The client identifies itself as Tibia through the normal product/window path.
- `ElfbotCompat::registerTibiaWindowClass()` registers the Win32 class name `TibiaClient` before SDL creates the real window.
- `ElfbotCompat::init()` runs very early from `main.cpp`, before the SDL window is created.
- `ElfbotCompat::sync()` runs once per frame from the main loop after engine update.
- The fixed Tibia 8.60 address range is reserved through the TLS startup path and then populated by `ElfbotCompat`.
- ElfBot can find the process/window and can read the primary Tibia 8.60 memory layout when injected into the real client.

## Window and process discovery

ElfBot looks for a Tibia-style client window. TFC preserves that path by:

- registering class `TibiaClient` with `SDL_RegisterApp`;
- creating the visible SDL window with product name `Tibia`;
- updating the title to `Tibia - <player name>` after login.

The current code path is primarily in-process mode: ElfBot attaches to the real TFC/Tibia process. `TibiaStub.exe` and `Local\TibiaShim860` still exist in the tree, but the current `init()` path does not launch the stub or create the IPC mapping.

## TibiaStub architecture

The stub still documents and implements the older two-process architecture:

- `TibiaStub.exe` has a fixed low image layout shaped like Tibia 8.60.
- It creates/registers a hidden `TibiaClient` window.
- It opens `Local\TibiaShim860`.
- It copies `TibiaShimBlock` fields into absolute Tibia 8.60 addresses.

Because the current client path uses direct in-process memory mirroring, `spawnStub()` and `createShim()` are present but dormant. This is important for future debugging: if ElfBot is attached to the stub, IPC must be re-enabled; if ElfBot is attached to TFC, the in-process mirror is the active path.

## Mirrored 8.60 addresses

Primary player block:

- `PLAYER_ID` at `0x63FE98`
- `PLAYER_HEALTH` at `0x63FE94`
- `PLAYER_HEALTH_MAX` at `0x63FE90`
- `PLAYER_LEVEL` at `0x63FE88`
- `PLAYER_MAGIC_LEVEL` at `0x63FE84`
- `PLAYER_LEVEL_PERCENT` at `0x63FE80`
- `PLAYER_MAGIC_PERCENT` at `0x63FE7C`
- `PLAYER_MANA` at `0x63FE78`
- `PLAYER_MANA_MAX` at `0x63FE74`
- `PLAYER_SOUL` at `0x63FE70`
- `PLAYER_STAMINA` at `0x63FE6C`
- `PLAYER_CAPACITY` at `0x63FE68`
- `PLAYER_EXPERIENCE` at `0x63FE8C`

Battle list:

- `BATTLELIST_START` at `0x63FEF8`
- stride `0xA8`
- max 250 slots
- slot 0 is forced to the local player when available

Other mirrored areas:

- inventory slots at `0x64CC98`
- containers at `0x64CD10`
- VIPs at `0x63DBB8`
- target/follow ids at `0x63FE64` and `0x63FE60`
- game-ready gate at `0x64A9C0`
- hotkey buffers around `0x799D30..0x799F08`
- text/status/look buffers around `0x7E07B0..0x7E0DA8`

## Data sources

The mirror uses live TFC state:

- `g_game` for player id, stats, skills, inventory, target/follow and containers;
- `g_map` for local player, known creatures and current positions;
- `g_engine` for modes, ingame state and the real SDL/Win32 window.

The 8.60 offsets were checked against the cloned TibiaAPI `Version860.cs` reference and the local constants in `elfbot_compat.h`.

## Already implemented bridges

- Incoming text is mirrored with `recordTextMessage()`.
- XTEA keys are captured with `setXteaKey()`.
- SDL key events are forwarded to the Win32 window for function-key hotkeys.
- Old-client packet/action hooks queue attack, follow, walk and autowalk requests for main-thread consumption.
- `processElfbotWriteback()` consumes target/follow/goto/say writes only from the main thread.

## Suspicious or pending areas

- The source still contains both in-process mode and TibiaStub IPC code. The active path should be confirmed in logs for every test.
- `createShim()` and `spawnStub()` are currently not called by `init()`.
- The battle-list writer filled outfit colors/addon/lightColor in the shim, but older copy paths did not write every field to 8.60 memory.
- Container support currently mirrors the first item directly and leaves full slot mirroring for later hardening.
- Packet/XTEA hooks should stay last; basic memory, battle list, target/follow and autowalk must be validated first.

## Working assumptions for next tests

- ElfBot is attached to the real TFC process unless logs show `TibiaStub.exe` as the target.
- Slot 0 must always be the local player after login.
- The game-ready gate must stay exactly `31` while ingame.
- No hook should call `g_game` directly from a foreign thread; pending flags are consumed during `sync()`.
