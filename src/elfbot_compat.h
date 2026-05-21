/*
  The Forgotten Client - ElfBot 8.60 memory compatibility layer

  Reserves the exact virtual address ranges Tibia.exe 8.60 used for its
  global data (player stats, creature list, containers, VIP list, etc.)
  and mirrors The Forgotten Client's live state into those addresses each
  frame, so ElfBot can read/write at its hard-coded offsets and see
  sensible data.

  Scope of compatibility:
    * DATA addresses (Player.Experience, BattleList.Start, Map.MapPointer,
      Container.Start, Vip.Start, Hotkey.*, etc.) -- fully supported.
      ElfBot reads stats, scans the creature list, lists VIPs, reads
      containers, etc.
    * CODE addresses (Client.DecryptCall, Client.ParserFunc,
      ContextMenus.AddContextMenuPtr, DrawItem.DrawItemFunc, etc.) --
      stubbed only. ElfBot features that *call* into Tibia code or *patch*
      Tibia code (custom RSA via Login patch, packet decryption hook, etc.)
      will NOT work, because there is no real Tibia code at those
      addresses in our binary. ElfBot will not crash because the bytes
      there are writable, but the calls go nowhere.

  IMPORTANT BUILD REQUIREMENT (Win32 only):
    The PE image base must be moved OUT of the Tibia 8.60 data range
    (0x400000 - 0x800000). The visual-studio.vcxproj sets
    <BaseAddress>0x10000000</BaseAddress> for Win32 builds so our own
    .text/.data sections don't collide with the regions we reserve here.
    ASLR (RandomizedBaseAddress) must also be disabled.
*/

#ifndef __FILE_ELFBOT_COMPAT_h_
#define __FILE_ELFBOT_COMPAT_h_

#include "defines.h"

namespace ElfbotCompat
{
	// -- Lifecycle ------------------------------------------------------
	// Call once, very early in main(), BEFORE SDL_CreateWindow. Reserves
	// the fixed virtual address regions and writes the static layout
	// (RSA modulus, server list, etc.).
	bool init();

	// Call once, very early in main(), BEFORE SDL_CreateWindow. Registers
	// the Win32 window class "TibiaClient" so tibiaelf.exe finds us.
	void registerTibiaWindowClass();

	// Call once per frame from the main loop, AFTER g_engine.update().
	// Refreshes the ElfBot-shaped memory from g_game / g_map / g_engine.
	void sync();

	// Call before exit, to release the reserved regions.
	void shutdown();

	// True if init() succeeded. If false, sync() is a no-op.
	bool isActive();

	// Mirror incoming client text into Tibia 8.60 text buffers so ElfBot
	// features that parse look/status messages can see them.
	void recordTextMessage(const char* text, const char* author, bool isLookMessage);

	// Mirror the current protocol XTEA key into Tibia 8.60 memory so
	// ElfBot's direct packet sender can encrypt packets with the same key
	// as the real TFC connection.
	void setXteaKey(const Uint32 keys[4]);

	// Forward SDL key events to the hidden TibiaClient helper window.
	// ElfBot's hotkey editor/hooks are Win32-message based, while this
	// client normally consumes keyboard input through SDL.
	void forwardKeyEvent(int keycode, int scancode, unsigned int modifiers, bool down);

	// -- Diagnostics ----------------------------------------------------
	// Append a printf-style line to elfbot_compat.log (next to the .exe).
	// Safe to call from any thread; calls before init() are silently
	// dropped. Includes a millisecond timestamp.
	void log(const char* fmt, ...);

	// Install SetUnhandledExceptionFilter so any AV/illegal-instruction
	// originating from elfbot.dll (or anywhere else) writes a crash
	// dump line to elfbot_compat.log before the process dies. Called
	// automatically from init().
	void installCrashHandler();

	// ------------------------------------------------------------------
	// Tibia 8.60 address constants (mirrored from the user-supplied
	// Tibia.Addresses C# layout). Grouped by region.
	// ------------------------------------------------------------------
	namespace Addr
	{
		// --- Player block (data) -------------------------------------
		// Addresses verified against TibiaAPI Version860.cs.
		// Anchor is Experience at 0x63FE8C. This ElfBot layout treats it
		// as a 32-bit value; HealthMax and Health occupy +4 and +8.
		static const Uint32 PLAYER_EXPERIENCE      = 0x63FE8C;  // Uint32
		static const Uint32 PLAYER_FLAGS           = 0x63FE20;  // Exp - 108
		static const Uint32 PLAYER_ID              = 0x63FE98;  // Exp + 12
		static const Uint32 PLAYER_HEALTH          = 0x63FE94;  // Exp + 8
		static const Uint32 PLAYER_HEALTH_MAX      = 0x63FE90;  // Exp + 4
		static const Uint32 PLAYER_LEVEL           = 0x63FE88;  // Exp - 4
		static const Uint32 PLAYER_MAGIC_LEVEL     = 0x63FE84;  // Exp - 8
		static const Uint32 PLAYER_LEVEL_PERCENT   = 0x63FE80;  // Exp - 12
		static const Uint32 PLAYER_MAGIC_PERCENT   = 0x63FE7C;  // Exp - 16
		static const Uint32 PLAYER_MANA            = 0x63FE78;  // Exp - 20
		static const Uint32 PLAYER_MANA_MAX        = 0x63FE74;  // Exp - 24
		static const Uint32 PLAYER_SOUL            = 0x63FE70;  // Exp - 28
		static const Uint32 PLAYER_STAMINA         = 0x63FE6C;  // Exp - 32
		static const Uint32 PLAYER_CAPACITY        = 0x63FE68;  // Exp - 36

		static const Uint32 PLAYER_FIST_PERCENT    = 0x63FE24;
		static const Uint32 PLAYER_SKILLS_BASE     = 0x63FE24;   // 14 dwords

		static const Uint32 PLAYER_SLOT_HEAD       = 0x64CC98;   // 10 slots, 12 bytes each

		static const Uint32 PLAYER_TILES_CURRENT   = 0x63FEA0;
		static const Uint32 PLAYER_TILES_TOGO      = 0x63FEA4;

		// --- Player position ------------------------------------------
		// Confirmed by both TibiaAPI and Tibia.exe 8.60 disassembly:
		// the position fields the original client reads for the *local*
		// player are the X/Y/Z dwords of BattleList[0]. There is NO
		// separate Player.X/Y/Z global -- the player is always slot 0
		// of the creature array.
		//   PLAYER_X        = BATTLELIST_START + 0*0xA8 + 36  = 0x63FF1C
		//   PLAYER_Y        = BATTLELIST_START + 0*0xA8 + 40  = 0x63FF20
		//   PLAYER_BATTLE_Z = BATTLELIST_START + 0*0xA8 + 44  = 0x63FF24
		// (BattleList.Start = 0x63FEF8; +0x24/0x28/0x2C are X/Y/Z within
		//  a creature slot, see C_OFF_X/Y/Z below.)
		static const Uint32 PLAYER_X               = 0x63FF1C;  // BattleList[0].X
		static const Uint32 PLAYER_Y               = 0x63FF20;  // BattleList[0].Y
		static const Uint32 PLAYER_BATTLE_Z        = 0x63FF24;  // BattleList[0].Z
		static const Uint32 PLAYER_GOTO_X          = 0x63FE8C + 80;
		static const Uint32 PLAYER_GOTO_Y          = (0x63FE8C + 80) - 4;
		static const Uint32 PLAYER_GOTO_Z          = (0x63FE8C + 80) - 8;

		static const Uint32 PLAYER_RED_SQUARE      = 0x63FE64;  // target id
		static const Uint32 PLAYER_GREEN_SQUARE    = 0x63FE64 - 4;
		static const Uint32 PLAYER_TARGET_BATTLELIST_ID   = 0x63FE5C;  // TargetId - 8
		static const Uint32 PLAYER_TARGET_BATTLELIST_TYPE = 0x63FE5F;  // TargetId - 5
		static const Uint32 PLAYER_TARGET_TYPE            = 0x63FE67;  // TargetId + 3
		static const Uint32 PLAYER_Z               = 0x64F600;
		static const Uint32 PLAYER_ATTACK_COUNT    = 0x63DA40;
		static const Uint32 PLAYER_FOLLOW_COUNT    = 0x63DA40 + 0x20;

		// --- BattleList (data) ---------------------------------------
		static const Uint32 BATTLELIST_START       = 0x63FEF8;
		static const Uint32 BATTLELIST_STEP        = 0xA8;       // 168 bytes
		static const Uint32 BATTLELIST_MAX         = 250;
		static const Uint32 BATTLELIST_END         = 0x63FEF8 + (0xA8 * 250);

		// --- Creature struct field offsets (within a battle-list slot)
		static const Uint32 C_OFF_ID               = 0;
		static const Uint32 C_OFF_TYPE             = 3;
		static const Uint32 C_OFF_NAME             = 4;     // 32-char string
		static const Uint32 C_OFF_X                = 36;
		static const Uint32 C_OFF_Y                = 40;
		static const Uint32 C_OFF_Z                = 44;
		static const Uint32 C_OFF_SCR_OFFSET_H     = 48;
		static const Uint32 C_OFF_SCR_OFFSET_V     = 52;
		static const Uint32 C_OFF_IS_WALKING       = 76;
		static const Uint32 C_OFF_DIRECTION        = 80;
		static const Uint32 C_OFF_OUTFIT           = 96;
		static const Uint32 C_OFF_COLOR_HEAD       = 100;
		static const Uint32 C_OFF_COLOR_BODY       = 104;
		static const Uint32 C_OFF_COLOR_LEGS       = 108;
		static const Uint32 C_OFF_COLOR_FEET       = 112;
		static const Uint32 C_OFF_ADDON            = 116;
		static const Uint32 C_OFF_LIGHT            = 120;
		static const Uint32 C_OFF_LIGHT_COLOR      = 124;
		static const Uint32 C_OFF_BLACK_SQUARE     = 132;
		static const Uint32 C_OFF_HP_BAR           = 136;     // percent 0..100
		static const Uint32 C_OFF_WALK_SPEED       = 140;
		static const Uint32 C_OFF_IS_VISIBLE       = 144;
		static const Uint32 C_OFF_SKULL            = 148;
		static const Uint32 C_OFF_PARTY            = 152;
		static const Uint32 C_OFF_WAR_ICON         = 160;
		static const Uint32 C_OFF_IS_BLOCKING      = 164;

		// --- Container array (data) ----------------------------------
		static const Uint32 CONTAINER_START        = 0x64CD10;
		static const Uint32 CONTAINER_STEP         = 492;
		static const Uint32 CONTAINER_STEP_SLOT    = 12;
		static const Uint32 CONTAINER_MAX          = 16;
		static const Uint32 CONTAINER_MAX_STACK    = 100;
		static const Uint32 CONTAINER_OFF_ISOPEN   = 0;
		static const Uint32 CONTAINER_OFF_ID       = 4;
		static const Uint32 CONTAINER_OFF_NAME     = 16;    // 32-byte string
		static const Uint32 CONTAINER_OFF_VOLUME   = 48;
		static const Uint32 CONTAINER_OFF_AMOUNT   = 56;
		static const Uint32 CONTAINER_OFF_ITEMID   = 60;    // first item id
		static const Uint32 CONTAINER_OFF_ITEMCNT  = 64;    // first item count

		// --- VIP list (data) -----------------------------------------
		static const Uint32 VIP_START              = 0x63DBB8;
		static const Uint32 VIP_STEP               = 0x2C;
		static const Uint32 VIP_MAX                = 200;
		static const Uint32 VIP_OFF_ID             = 0;
		static const Uint32 VIP_OFF_NAME           = 4;
		static const Uint32 VIP_OFF_STATUS         = 34;
		static const Uint32 VIP_OFF_ICON           = 40;

		// --- Map / world (data) --------------------------------------
		static const Uint32 MAP_POINTER            = 0x654118;

		// --- Hotkeys (data) ------------------------------------------
		// Verified against a Tibia.exe 8.60 disassembly: the hotkey state
		// is stored as 5 PARALLEL arrays, one slot per F1..F12+Shift+Ctrl
		// hotkey (36 total). Each "object" array is 36 dwords (0x90 bytes).
		//
		//   0x00799D30  HOTKEY_OBJECT_USE        36 dwords  item id / spell id
		//   0x00799DC0  HOTKEY_OBJECT_COUNT      36 dwords  stack count / variant   (NEW)
		//   0x00799E50  HOTKEY_OBJECT_START      36 dwords  action mode (Use/UseOn/Equip)
		//   0x00799EE0  HOTKEY_AUTO_START        36 bytes   auto-send flag table
		//   0x00799F08  HOTKEY_TEXT_START        36 entries hotkey text buffers (zero-init)
		//
		// ElfBot reads ALL of these when a hotkey fires. If COUNT is
		// missing it ends up reading whatever junk is at 0x799DC0 and
		// either uses 0 (= no action) or a corrupt stack count (= refuses
		// to fire). That is the most likely reason hotkeys don't work in
		// our client: the existing header skipped the COUNT array.
		static const Uint32 HOTKEY_OBJECT_USE      = 0x799D30;  // 36 dwords
		static const Uint32 HOTKEY_OBJECT_COUNT    = 0x799DC0;  // 36 dwords (NEW)
		static const Uint32 HOTKEY_OBJECT_START    = 0x799E50;  // 36 dwords
		static const Uint32 HOTKEY_AUTO_START      = 0x799EE0;  // 36 bytes
		static const Uint32 HOTKEY_TEXT_START      = 0x799F08;  // 36 text slots
		static const Uint32 HOTKEY_MAX             = 36;
		static const Uint32 HOTKEY_STRIDE          = 4;          // dword stride within an array
		static const Uint32 HOTKEY_TEXT_STRIDE     = 0x100;      // 256-byte text slot

		// --- Client globals (data) -----------------------------------
		static const Uint32 CLIENT_STATUS          = 0x79CF28;
		static const Uint32 CLIENT_ACTION_STATE    = 0x79CF88;
		static const Uint32 CLIENT_MULTI_CLIENT    = 0x50BCA4;
		static const Uint32 CLIENT_SAFE_MODE       = 0x799CE4;
		static const Uint32 CLIENT_FOLLOW_MODE     = 0x799CE4 + 4;
		static const Uint32 CLIENT_ATTACK_MODE     = 0x799CE4 + 8;
		static const Uint32 CLIENT_START_TIME      = 0x7E0790;
		static const Uint32 CLIENT_LASTMSG_TEXT    = 0x7E0A00;
		static const Uint32 CLIENT_LASTMSG_AUTHOR  = 0x7E0A00 - 0x28;
		static const Uint32 CLIENT_SPEAK_TEXT_A    = 0x7E09D8;
		static const Uint32 CLIENT_SPEAK_TEXT_B    = 0x795098;
		static const Uint32 CLIENT_HOTKEY_TEXT_A   = 0x7E0A28;
		static const Uint32 CLIENT_HOTKEY_TEXT_B   = 0x7E0BC0;
		static const Uint32 CLIENT_HOTKEY_TEXT_C   = 0x7E1218;
		static const Uint32 CLIENT_STATUSBAR_TEXT  = 0x7E07B0;
		static const Uint32 CLIENT_LOOK_TEXT       = 0x7E0DA8;
		static const Uint32 CLIENT_STATUSBAR_TIME  = 0x7E07B0 - 4;
		static const Uint32 CLIENT_GAMEWIN_BAR     = 0x7E07A4;

		// --- FPS / framerate counter (Tibia 8.60) --------------------
		// Found via reverse-engineering the "%s %.02f fps" format string
		// in Tibia.exe: the formatter at 0x004BA028 writes EDX into
		// 0x0064AA2C and ESI into 0x0064AA34 right before the printf.
		// 0x0064AA34 holds the live FPS as a 32-bit float; 0x0064AA2C
		// holds the formatted string pointer (the "fps" suffix label).
		// The "Current framerate: %2.2f" console reader uses a
		// secondary copy at 0x0064BF44/0x0064BF48 (eax/ecx).
		// Bots that read FPS from the original client use 0x0064AA34.
		static const Uint32 CLIENT_FPS_VALUE       = 0x0064AA34;  // float, live FPS
		static const Uint32 CLIENT_FPS_VALUE_ALT   = 0x0064BF44;  // alt copy
		static const Uint32 CLIENT_FRAME_TIME_MS   = 0x0064BF48;  // ms/frame copy

		// --- Status-bar display copies (Tibia 8.60) ------------------
		// The status-bar formatter at 0x004BA000 reads the integer player
		// stats (0x63FE74..0x63FEA4) and writes formatted DOUBLE copies
		// into UI cells. Reverse-engineered by tracing `push "Cap:"`
		// (VA 0x005BC458) and `push "Soul:"` (0x005BC460):
		//   "Cap:"  formatter writes EAX/ECX -> 0x0064ADD0/D4 (double)
		//   "Soul:" formatter writes EAX/ECX -> 0x0064AE18/1C (double)
		// Memory-scanning bots read these copies, not the integer source.
		// We mirror them so the values match across both layouts.
		static const Uint32 CLIENT_DISP_CAPACITY   = 0x0064ADD0;  // double, displayed cap
		static const Uint32 CLIENT_DISP_SOUL       = 0x0064AE18;  // double, displayed soul

		// --- ElfBot "game ready" gate (Tibia 8.60) -------------------
		// Reverse-engineered from elfbot.dll's title-bar formatter at
		// VA 0x10005DEC:
		//
		//      mov eax, 0x0064A9C0     ; load counter address
		//      cmp dword [eax], 0x1F   ; compare to 31
		//      je  0x10006B5C          ; equal -> SKIP appending "Startup"
		//      push "Startup"          ; otherwise stamp "Startup" into title
		//
		// Inside Tibia.exe this counter is:
		//      mov dword [0x0064A9C0], 0   ; init  @ 0x004B76E0
		//      inc dword [0x0064A9C0]      ; bumped @ 0x004C5DD9 each tick
		// of the in-game state machine. ElfBot watches it as a "Tibia has
		// finished its login handshake" proxy. When it stays at zero (as
		// it does in TFC because we never tick the in-game heartbeat),
		// the title bar reads
		//    "<server> - <name> - Startup - 0 ms - 0 exp/hour"
		// and ElfBot keeps cavebot / hotkey dispatch in cold-start state.
		// Writing >= 31 here flips the title to
		//    "<server> - <name> - Slot N - <ping> ms - <exp>/hour"
		// and unlocks cavebot / targeting / hotkey processing.
		static const Uint32 CLIENT_GAMETICK_COUNTER = 0x0064A9C0;
		static const Uint32 CLIENT_LOGIN_PASSWORD  = 0x79CEE4;
		static const Uint32 CLIENT_LOGIN_ACCOUNT   = 0x79CEE4 + 32;
		static const Uint32 CLIENT_LOGIN_SEL_CHAR  = 0x79CED8;
		static const Uint32 CLIENT_LOGIN_CHARLIST  = 0x79CEDC;
		static const Uint32 CLIENT_LOGIN_CHARLEN   = 0x79CEE0;
		static const Uint32 CLIENT_LOGINSRV_START  = 0x7947F8;
		static const Uint32 CLIENT_LOGINSRV_STEP   = 112;
		static const Uint32 CLIENT_LOGINSRV_MAX    = 10;
		static const Uint32 CLIENT_XTEAKEY         = 0x7998BC;
		static const Uint32 CLIENT_RSA_MODULUS     = 0x5B8980;    // 309-byte string
		static const Uint32 CLIENT_DAT_POINTER     = 0x7998DC;

		// --- Reservation regions (must cover all above) --------------
		// We allocate two large regions; both with PAGE_READWRITE.
		// Code-section addresses (Client.DecryptCall, etc.) live in
		// REGION_TEXT and are NOT executable -- ElfBot writes to them
		// without crashing but the calls/patches do not function.
		static const Uint32 REGION_TEXT_BASE       = 0x00440000;  // 64KB-aligned
		static const Uint32 REGION_TEXT_SIZE       = 0x00190000;  // up to 0x5D0000
		static const Uint32 REGION_DATA_BASE       = 0x00630000;
		static const Uint32 REGION_DATA_SIZE       = 0x001C0000;  // up to 0x7F0000
	}
}

#endif /* __FILE_ELFBOT_COMPAT_h_ */
