# ElfBot compatibility test plan

## Startup

- [ ] TFC opens without ElfBot.
- [ ] TFC opens with ElfBot closed.
- [ ] `elfbot_compat.log` is created beside `Tibia.exe`.
- [ ] The log shows `TFC PID`.
- [ ] The log shows whether `TibiaClient` was registered.
- [ ] The log shows whether `elfload.dll`, `elfbot.dll` or `USkin.dll` are loaded.
- [ ] ElfBot opens and finds the client.
- [ ] ElfBot does not stay in `Startup` after login.

## Player memory

- [ ] ElfBot reads player name.
- [ ] ElfBot reads player id after login.
- [ ] ElfBot reads level.
- [ ] ElfBot reads magic level.
- [ ] ElfBot reads HP and HP max.
- [ ] HP changes after damage/heal.
- [ ] ElfBot reads mana and mana max.
- [ ] Mana changes after spell/cast.
- [ ] ElfBot reads soul, stamina and capacity if shown.

## Position and battle list

- [ ] BattleList slot 0 is the local player.
- [ ] Slot 0 id equals `g_game.getPlayerID()`.
- [ ] Slot 0 name equals the local player name.
- [ ] Slot 0 X/Y/Z updates when walking north/south/east/west.
- [ ] `PLAYER_Z` and BattleList slot 0 Z match after floor changes.
- [ ] Nearby monsters appear in slots 1..249.
- [ ] Monster HP percent updates while attacking.
- [ ] Creature leaves the list after it leaves the visible area.

## Target and follow

- [ ] Attacking from normal TFC updates ElfBot target id.
- [ ] Cancelling attack from normal TFC clears ElfBot target id.
- [ ] Attack requested by ElfBot makes TFC attack the creature.
- [ ] Follow requested by ElfBot makes TFC follow the creature.
- [ ] Cancelling from ElfBot clears attack/follow safely.

## Cavebot and autowalk

- [ ] ElfBot Cavebot waypoint writes a target position.
- [ ] Log shows old-client autowalk or goto writeback.
- [ ] TFC calls `startAutoWalk()` for a plausible same-floor target.
- [ ] Invalid/stale far targets are ignored and logged.
- [ ] Single walk packets are bridged to TFC walk actions.

## Hotkeys and text

- [ ] F1..F12 key events are forwarded.
- [ ] WASD letters are not forwarded as hotkeys.
- [ ] ElfBot hotkey `say test` sends `test` once.
- [ ] Auto hotkey repeats only at its configured interval.
- [ ] Server/status/look text is not echoed as player speech.

## Inventory and containers

- [ ] Equipment slot reads item id/count.
- [ ] Moving/removing equipment updates the slot.
- [ ] Opening a backpack marks the container open.
- [ ] Container name/capacity/item count are mirrored.
- [ ] First item id/count update on add/remove.
- [ ] Closing a container clears the open flag.

## Stability

- [ ] Relog does not crash.
- [ ] Closing ElfBot does not crash TFC.
- [ ] Closing TFC exits or detaches the stub path cleanly if the stub path is active.
- [ ] Logs do not spam every frame.
- [ ] Win32 Release build succeeds.
- [ ] Win32 Debug build succeeds if available.
