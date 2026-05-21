#ifdef _WIN32

// EXE image base = 0x00140000. This array is placed in .text$aaa which
// the linker merges BEFORE the normal .text section, so the shadow
// starts at RVA 0x1000 = absolute address 0x00141000.
//
// We must size the shadow so that its END is past 0x00800000 -- the
// highest Tibia 8.60 address ElfBot writes to. Otherwise the back half
// of the Tibia address range (0x005B8980 RSA, 0x0063FE94 Player.Health,
// 0x0063FEF8 BattleList, 0x0079CF28 Client.Status, etc.) ends up
// overlapping TFC's REAL .text/.rdata/.data sections that come after
// the shadow, and our Tibia-style writes silently corrupt TFC's own
// globals (g_game, g_map, vtables, string literals). Symptom: writes
// "succeed" (memHealth matches shim->health in the log) but ElfBot's
// HUD reads back garbage values because TFC's data layout is being
// scrambled.
//
// Required end address >= 0x00800000  ==> end RVA >= 0x006C0000
//                                      ==> size >= 0x006BF000
// We round up to 0x006C0000 (7,077,888 bytes = 6.75 MB) for clean
// page alignment.
//
// Because the array is uninitialised volatile memory, the linker
// records the size in section headers but emits zero raw bytes
// (it's effectively BSS in .text$aaa), so the file size on disk
// barely grows.
#pragma section(".text$aaa", read, execute)
__declspec(allocate(".text$aaa")) __declspec(align(4096))
volatile unsigned char g_tibia860ImageShadow[0x006C0000];

#endif
