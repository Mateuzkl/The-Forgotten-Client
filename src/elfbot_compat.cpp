/*
  The Forgotten Client - ElfBot 8.60 memory compatibility layer

  Implementation. See elfbot_compat.h for the public interface and the
  description of what is / isn't faked.

  Architecture:
    1. init() reserves two large virtual address ranges at fixed
       locations using VirtualAlloc(MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE).
       Those ranges cover every data and code address Tibia 8.60 placed
       in its .data/.text sections, so any read or write ElfBot performs
       lands on writable memory rather than crashing.

    2. registerTibiaWindowClass() pre-registers a Win32 window class
       called "TibiaClient" via SDL_RegisterApp(), so SDL2's subsequent
       call to RegisterClassEx (inside SDL_CreateWindow) uses our class
       name. This makes tibiaelf.exe's process scan find the window.

    3. sync() runs once per frame. It snapshots TFC's live state from
       g_game / g_map / g_engine and writes it into the fixed addresses
       in the Tibia 8.60 layout. Concretely it writes:
         * Player.* block at 0x63FE8C
         * BattleList creatures at 0x63FEF8 (168-byte stride)
         * Container array at 0x64CD10 (492-byte stride)
         * Inventory slots at 0x64CC98
         * Player.Z at 0x64F600
*/

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#include <SDL2/SDL_keycode.h>
#include <SDL2/SDL_main.h>
#include <SDL2/SDL_syswm.h>
#pragma comment(lib, "psapi.lib")
#endif

#include "elfbot_compat.h"
#include "game.h"
#include "map.h"
#include "creature.h"
#include "thingManager.h"   // for ThingType::m_id (looktype) read in shimWriteCreature
#include "container.h"
#include "engine.h"
#include "protocolgame.h"
#include "GUI/itemUI.h"

// Shared-memory IPC contract with TibiaStub.exe. The stub is a separate
// process whose PE image covers the Tibia 8.60 address range, since
// Win10's process layout makes that range unreachable from inside TFC.
#include "../tibiastub/tibia_shim_ipc.h"

#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <string>
#include <vector>
#ifdef _WIN32
#include <share.h>
#endif
#include <climits>

#if defined(_MSC_VER)
#pragma warning(disable: 4505)
#endif

extern Game   g_game;
extern Map    g_map;
extern Engine g_engine;

// Defined in main.cpp -- TFC's own FPS / frame-time globals. We mirror
// them into the Tibia 8.60 FPS variable so ElfBot's HUD sees the live
// framerate from TFC. Must be declared at file scope (NOT inside
// namespace ElfbotCompat) so the linker resolves them to the unqualified
// ::g_lastFrames / ::g_frameDiff symbols, not ElfbotCompat::g_*.
extern Uint16 g_lastFrames;
extern Uint32 g_frameDiff;

#ifdef _WIN32
// Size MUST match the definition in elfbot_shadow.cpp. The shadow needs
// to be large enough that its END is past 0x00800000 (the highest Tibia
// 8.60 address), otherwise late Tibia writes silently corrupt TFC's
// real .text/.rdata/.data sections that come after the shadow.
extern volatile unsigned char g_tibia860ImageShadow[0x006C0000];
#endif

namespace ElfbotCompat
{
	// ---- state --------------------------------------------------------
	static bool      s_active        = false;
	static void*     s_regionText    = NULL;
	static void*     s_regionData    = NULL;
	static char      s_lastMessageText[256] = {0};
	static char      s_lastMessageAuthor[40] = {0};
	static char      s_lastLookText[256] = {0};

	static void copyBounded(char* destination, size_t destinationSize, const char* source)
	{
		if(!destination || destinationSize == 0)
			return;

		std::snprintf(destination, destinationSize, "%s", (source ? source : ""));
		destination[destinationSize - 1] = '\0';
	}

	// Counters populated by the TLS callback at process startup, then
	// logged by init() once CRT I/O is safe.
	struct TlsResult { Uint32 ok; Uint32 fail; uintptr_t firstFail; };
	static TlsResult s_tlsText  = {0, 0, 0};
	static TlsResult s_tlsDlow  = {0, 0, 0};
	static TlsResult s_tlsDhigh = {0, 0, 0};
	static bool s_tlsRan = false;
#ifdef _WIN32
	static HWND      s_tibiaHwnd     = NULL;  // hidden helper window
	static FILE*     s_logFile       = NULL;
	static CRITICAL_SECTION s_logLock;
	static bool      s_logInited     = false;
	static LPTOP_LEVEL_EXCEPTION_FILTER s_prevFilter = NULL;
	static bool      s_hasMirroredTargets = false;
	static Uint32    s_mirroredAttackId = 0;
	static Uint32    s_mirroredFollowId = 0;
	static bool      s_tibiaHwndIsRealWindow = false;
	static bool      s_hasMirroredGoto = false;
	static Uint32    s_mirroredGotoX = 0;
	static Uint32    s_mirroredGotoY = 0;
	static Uint32    s_mirroredGotoZ = 0;
	static bool      s_watchSnapshotsReady = false;
	static volatile LONG s_pendingOldWalk = 0;
	static volatile LONG s_pendingOldAttack = 0;
	static volatile LONG s_pendingOldFollow = 0;
	static volatile LONG s_pendingPacketWalk = 0;
	static volatile LONG s_pendingPacketAutoWalk = 0;
	static volatile LONG s_pendingPacketAttack = 0;
	static volatile LONG s_pendingPacketFollow = 0;
	static Uint32    s_pendingOldWalkX = 0;
	static Uint32    s_pendingOldWalkY = 0;
	static Uint32    s_pendingOldWalkZ = 0;
	static Uint32    s_pendingOldAttackId = 0;
	static Uint32    s_pendingOldFollowId = 0;
	static Direction s_pendingPacketWalkDir = DIRECTION_INVALID;
	static Uint8     s_pendingPacketAutoWalkCount = 0;
	static Direction s_pendingPacketAutoWalkDirs[128] = {};
	static Uint32    s_pendingPacketAttackId = 0;
	static Uint32    s_pendingPacketFollowId = 0;
	static bool      s_hasXteaKey = false;
	static Uint32    s_xteaKey[4] = {0, 0, 0, 0};

	struct WatchRange
	{
		Uint32 base;
		Uint32 size;
		const char* name;
		std::vector<unsigned char> bytes;
	};
	static std::vector<WatchRange> s_watchRanges;

	// Stub IPC state.
	static HANDLE              s_shimMapping    = NULL;   // file mapping
	static TibiaShimBlock*     s_shim           = NULL;   // mapped view, writable
	static PROCESS_INFORMATION s_stubProcInfo   = {0};    // spawned stub
	static bool                s_stubLaunched   = false;
	static Uint32              s_frameCounter   = 0;

	static void dumpElfbotRelatedModules(const char* tag);
	static void dumpAndScanElfbotImageOnce();
	static void probeElfbotInternalState(const TibiaShimBlock* shim, bool force);
	static bool isPlausiblePosition(Uint32 x, Uint32 y, Uint32 z);
	static Uint32 deltaAbs(Uint32 a, Uint32 b);
#endif

#ifdef _WIN32
	static LRESULT CALLBACK tibiaClientWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
	{
		return DefWindowProcW(hwnd, msg, wp, lp);
	}

	static UINT mapSdlKeyToVirtualKey(int keycode)
	{
		// Do not synthesize letter keys into ElfBot's helper window.
		// The client uses WASD movement, and ElfBot's key editor polls
		// those messages as hotkey candidates; forwarding letters makes
		// the editor repeatedly capture W. Function keys still forward.
		if((keycode >= 'a' && keycode <= 'z') ||
		   (keycode >= 'A' && keycode <= 'Z'))
			return 0;
		if(keycode >= '0' && keycode <= '9')
			return static_cast<UINT>(keycode);
		if(keycode >= SDLK_F1 && keycode <= SDLK_F12)
			return static_cast<UINT>(VK_F1 + (keycode - SDLK_F1));

		switch(keycode)
		{
			case SDLK_ESCAPE:    return VK_ESCAPE;
			case SDLK_TAB:       return VK_TAB;
			case SDLK_RETURN:    return VK_RETURN;
			case SDLK_BACKSPACE: return VK_BACK;
			case SDLK_SPACE:     return VK_SPACE;
			case SDLK_LEFT:      return VK_LEFT;
			case SDLK_RIGHT:     return VK_RIGHT;
			case SDLK_UP:        return VK_UP;
			case SDLK_DOWN:      return VK_DOWN;
			case SDLK_HOME:      return VK_HOME;
			case SDLK_END:       return VK_END;
			case SDLK_PAGEUP:    return VK_PRIOR;
			case SDLK_PAGEDOWN:  return VK_NEXT;
			case SDLK_INSERT:    return VK_INSERT;
			case SDLK_DELETE:    return VK_DELETE;
			case SDLK_LSHIFT:
			case SDLK_RSHIFT:    return VK_SHIFT;
			case SDLK_LCTRL:
			case SDLK_RCTRL:     return VK_CONTROL;
			case SDLK_LALT:
			case SDLK_RALT:      return VK_MENU;
			case SDLK_KP_0:      return VK_NUMPAD0;
			case SDLK_KP_1:      return VK_NUMPAD1;
			case SDLK_KP_2:      return VK_NUMPAD2;
			case SDLK_KP_3:      return VK_NUMPAD3;
			case SDLK_KP_4:      return VK_NUMPAD4;
			case SDLK_KP_5:      return VK_NUMPAD5;
			case SDLK_KP_6:      return VK_NUMPAD6;
			case SDLK_KP_7:      return VK_NUMPAD7;
			case SDLK_KP_8:      return VK_NUMPAD8;
			case SDLK_KP_9:      return VK_NUMPAD9;
			case SDLK_KP_ENTER:  return VK_RETURN;
			default:             return 0;
		}
	}
#endif

	void forwardKeyEvent(int keycode, int scancode, unsigned int modifiers, bool down)
	{
#ifdef _WIN32
		(void)scancode;
		if(!s_tibiaHwnd || !IsWindow(s_tibiaHwnd))
			return;
		if(s_tibiaHwndIsRealWindow)
			return;

		UINT vk = mapSdlKeyToVirtualKey(keycode);
		if(vk == 0)
			return;

		UINT scan = MapVirtualKeyA(vk, MAPVK_VK_TO_VSC);
		LPARAM lp = 1 | (static_cast<LPARAM>(scan) << 16);
		if(!down)
			lp |= 0xC0000000;

		bool alt = (modifiers & KMOD_ALT) != 0 || vk == VK_MENU;
		UINT msg = down ? (alt ? WM_SYSKEYDOWN : WM_KEYDOWN)
		                : (alt ? WM_SYSKEYUP   : WM_KEYUP);
		PostMessageA(s_tibiaHwnd, msg, static_cast<WPARAM>(vk), lp);
#else
		(void)keycode;
		(void)scancode;
		(void)modifiers;
		(void)down;
#endif
	}

	// -- diagnostics ---------------------------------------------------
	void log(const char* fmt, ...)
	{
#ifdef _WIN32
		if(!s_logInited || !s_logFile) return;
		EnterCriticalSection(&s_logLock);
		SYSTEMTIME st;
		GetLocalTime(&st);
		std::fprintf(s_logFile, "[%02d:%02d:%02d.%03d] ",
			st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);
		va_list ap;
		va_start(ap, fmt);
		std::vfprintf(s_logFile, fmt, ap);
		va_end(ap);
		std::fputc('\n', s_logFile);
		std::fflush(s_logFile);
		LeaveCriticalSection(&s_logLock);
#else
		(void)fmt;
#endif
	}

	void setXteaKey(const Uint32 keys[4])
	{
#ifdef _WIN32
		if(!keys)
			return;

		for(int i = 0; i < 4; ++i)
			s_xteaKey[i] = keys[i];
		s_hasXteaKey = true;

		log("xtea key captured for old client memory");
#else
		(void)keys;
#endif
	}

	void recordTextMessage(const char* text, const char* author, bool isLookMessage)
	{
		if(!text)
			return;

		copyBounded(s_lastMessageText, sizeof(s_lastMessageText), text);

		if(author)
			copyBounded(s_lastMessageAuthor, sizeof(s_lastMessageAuthor), author);
		else
			s_lastMessageAuthor[0] = '\0';

		if(isLookMessage)
		{
			copyBounded(s_lastLookText, sizeof(s_lastLookText), text);
			log("look text captured: %s", s_lastLookText);
		}
	}

#ifdef _WIN32
	static void openLogFile()
	{
		if(s_logInited) return;
		InitializeCriticalSection(&s_logLock);
		// Write the log right next to the running .exe so the user
		// finds it without hunting through %APPDATA%.
		char exePath[MAX_PATH];
		DWORD n = GetModuleFileNameA(NULL, exePath, MAX_PATH);
		std::string logPath;
		if(n > 0 && n < MAX_PATH)
		{
			logPath = exePath;
			size_t slash = logPath.find_last_of("\\/");
			if(slash != std::string::npos)
				logPath.resize(slash + 1);
			else
				logPath.clear();
		}
		logPath += "elfbot_compat.log";
		// Allow external tools / PowerShell to read the log while the
		// client is running. fopen_s opens with an exclusive share mode
		// on MSVC, which made live diagnostics impossible.
		s_logFile = _fsopen(logPath.c_str(), "w", _SH_DENYNO);
		s_logInited = true;
		if(s_logFile)
		{
			SYSTEMTIME st;
			GetLocalTime(&st);
			std::fprintf(s_logFile,
				"=== ElfBot compat log opened %04d-%02d-%02d %02d:%02d:%02d ===\n",
				st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);
			std::fflush(s_logFile);
		}
	}

	// Identify the module (DLL/EXE) that contains an arbitrary address.
	// Writes "ModName.dll +0xOFFSET" into out. Out must be at least
	// MAX_PATH + 32 bytes.
	static void describeAddress(void* addr, char* out, size_t outSize)
	{
		HMODULE hMod = NULL;
		if(GetModuleHandleExA(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
			GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			reinterpret_cast<LPCSTR>(addr), &hMod) && hMod)
		{
			char modPath[MAX_PATH] = {0};
			GetModuleFileNameA(hMod, modPath, MAX_PATH);
			const char* leaf = std::strrchr(modPath, '\\');
			leaf = leaf ? leaf + 1 : modPath;
			uintptr_t off = reinterpret_cast<uintptr_t>(addr)
			              - reinterpret_cast<uintptr_t>(hMod);
			_snprintf_s(out, outSize, _TRUNCATE,
				"%s+0x%IX (base 0x%p)", leaf, off, hMod);
		}
		else
		{
			_snprintf_s(out, outSize, _TRUNCATE,
				"<no module> 0x%p", addr);
		}
	}

	// Walk the process VAS via VirtualQuery and log every region from
	// `from` to `to`. Lets us see what is reserving the addresses we
	// want to use, before we even attempt the alloc.
	static void dumpAddressSpace(uintptr_t from, uintptr_t to, const char* tag)
	{
		log("--- address space map %s [0x%IX..0x%IX] ---", tag, from, to);
		MEMORY_BASIC_INFORMATION mbi;
		uintptr_t addr = from;
		int rows = 0;
		while(addr < to && rows < 256)
		{
			SIZE_T r = VirtualQuery(reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi));
			if(r == 0) break;
			const char* state =
				(mbi.State == MEM_FREE)    ? "FREE  " :
				(mbi.State == MEM_RESERVE) ? "RESRV " :
				(mbi.State == MEM_COMMIT)  ? "COMMIT" : "?     ";
			const char* type =
				(mbi.Type == MEM_IMAGE)   ? "IMAGE  " :
				(mbi.Type == MEM_MAPPED)  ? "MAPPED " :
				(mbi.Type == MEM_PRIVATE) ? "PRIVATE" : "       ";
			char info[MAX_PATH + 32] = {0};
			if(mbi.Type == MEM_IMAGE)
				describeAddress(mbi.AllocationBase, info, sizeof(info));
			log("  0x%08IX +0x%08IX  %s %s prot=0x%08lX  %s",
				reinterpret_cast<uintptr_t>(mbi.BaseAddress),
				static_cast<uintptr_t>(mbi.RegionSize),
				state, type, mbi.Protect, info);
			addr = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
			++rows;
		}
	}

	// Reserve+commit a single contiguous range page-by-page (64KB
	// granularity). Pages that conflict with existing reservations
	// are skipped, not fatal. Returns the number of pages successfully
	// reserved.
	static Uint32 reservePagewise(uintptr_t base, Uint32 size, DWORD protect, const char* tag)
	{
		const Uint32 GRAN = 0x10000;  // 64KB Windows allocation granularity
		Uint32 ok = 0, fail = 0;
		uintptr_t firstFail = 0;
		log("reservePagewise '%s' base=0x%IX size=0x%X protect=0x%X",
			tag, base, size, protect);
		for(Uint32 off = 0; off < size; off += GRAN)
		{
			LPVOID p = VirtualAlloc(
				reinterpret_cast<LPVOID>(base + off),
				GRAN,
				MEM_RESERVE | MEM_COMMIT,
				protect);
			if(p)
			{
				++ok;
			}
			else
			{
				++fail;
				if(!firstFail) firstFail = base + off;
			}
		}
		log("  '%s' result: ok=%u fail=%u firstFail=0x%IX",
			tag, ok, fail, firstFail);
		return ok;
	}

	static void dumpLoadedModules(const char* tag)
	{
		HMODULE mods[256];
		DWORD needed = 0;
		HANDLE hProc = GetCurrentProcess();
		if(!EnumProcessModules(hProc, mods, sizeof(mods), &needed))
			return;
		DWORD count = needed / sizeof(HMODULE);
		if(count > 256) count = 256;
		log("--- loaded modules (%s) count=%lu ---", tag, count);
		for(DWORD i = 0; i < count; ++i)
		{
			char modPath[MAX_PATH] = {0};
			GetModuleFileNameA(mods[i], modPath, MAX_PATH);
			MODULEINFO mi = {0};
			GetModuleInformation(hProc, mods[i], &mi, sizeof(mi));
			log("  %p..%p  %s",
				mi.lpBaseOfDll,
				reinterpret_cast<void*>(
					reinterpret_cast<uintptr_t>(mi.lpBaseOfDll) + mi.SizeOfImage),
				modPath);
		}
	}

	static LONG WINAPI crashFilter(EXCEPTION_POINTERS* ep)
	{
		if(!ep || !ep->ExceptionRecord || !ep->ContextRecord)
			return EXCEPTION_CONTINUE_SEARCH;

		DWORD code = ep->ExceptionRecord->ExceptionCode;
		void* eipAddr = ep->ExceptionRecord->ExceptionAddress;

		log("============================================================");
		log("CRASH: exception 0x%08lX at EIP=0x%p", code, eipAddr);

		const char* name = "unknown";
		switch(code)
		{
			case EXCEPTION_ACCESS_VIOLATION:     name = "ACCESS_VIOLATION"; break;
			case EXCEPTION_ILLEGAL_INSTRUCTION:  name = "ILLEGAL_INSTRUCTION"; break;
			case EXCEPTION_PRIV_INSTRUCTION:     name = "PRIV_INSTRUCTION"; break;
			case EXCEPTION_INT_DIVIDE_BY_ZERO:   name = "INT_DIVIDE_BY_ZERO"; break;
			case EXCEPTION_STACK_OVERFLOW:       name = "STACK_OVERFLOW"; break;
			case EXCEPTION_IN_PAGE_ERROR:        name = "IN_PAGE_ERROR"; break;
			case EXCEPTION_BREAKPOINT:           name = "BREAKPOINT"; break;
			case 0xC0000409:                     name = "STACK_BUFFER_OVERRUN"; break;
		}
		log("  type: %s", name);

		if(code == EXCEPTION_ACCESS_VIOLATION || code == EXCEPTION_IN_PAGE_ERROR)
		{
			ULONG_PTR rw      = ep->ExceptionRecord->ExceptionInformation[0];
			ULONG_PTR badAddr = ep->ExceptionRecord->ExceptionInformation[1];
			const char* op = (rw == 0) ? "READ"
			               : (rw == 1) ? "WRITE"
			               : (rw == 8) ? "DEP/EXECUTE"
			               : "?";
			log("  bad access: %s at 0x%p", op, reinterpret_cast<void*>(badAddr));
		}

		char where[MAX_PATH + 64] = {0};
		describeAddress(eipAddr, where, sizeof(where));
		log("  EIP in: %s", where);

		// Register dump
		CONTEXT* c = ep->ContextRecord;
#ifdef _WIN64
		log("  RAX=0x%016IX RBX=0x%016IX RCX=0x%016IX RDX=0x%016IX",
			(uintptr_t)c->Rax, (uintptr_t)c->Rbx, (uintptr_t)c->Rcx, (uintptr_t)c->Rdx);
		log("  RSI=0x%016IX RDI=0x%016IX RBP=0x%016IX RSP=0x%016IX",
			(uintptr_t)c->Rsi, (uintptr_t)c->Rdi, (uintptr_t)c->Rbp, (uintptr_t)c->Rsp);
		log("  R8 =0x%016IX R9 =0x%016IX R10=0x%016IX R11=0x%016IX",
			(uintptr_t)c->R8,  (uintptr_t)c->R9,  (uintptr_t)c->R10, (uintptr_t)c->R11);
		log("  RIP=0x%016IX EFL=0x%08lX",
			(uintptr_t)c->Rip, c->EFlags);
#else
		log("  EAX=0x%08lX EBX=0x%08lX ECX=0x%08lX EDX=0x%08lX",
			c->Eax, c->Ebx, c->Ecx, c->Edx);
		log("  ESI=0x%08lX EDI=0x%08lX EBP=0x%08lX ESP=0x%08lX",
			c->Esi, c->Edi, c->Ebp, c->Esp);
		log("  EIP=0x%08lX EFL=0x%08lX",
			c->Eip, c->EFlags);

		// Cheap return-address scrape from the stack (skip pure stack
		// walk -- needs DbgHelp). Walk 16 stack slots and log any value
		// that lands inside a known module's .text range.
		log("  stack candidates (return addresses):");
		const DWORD* sp = reinterpret_cast<const DWORD*>(c->Esp);
		__try {
			for(int i = 0; i < 32; ++i)
			{
				DWORD val = sp[i];
				if(val < 0x10000) continue;
				char info[MAX_PATH + 64] = {0};
				describeAddress(reinterpret_cast<void*>(val), info, sizeof(info));
				if(std::strstr(info, "<no module>") == NULL)
					log("    [esp+0x%02X] = 0x%08lX  %s", i * 4, val, info);
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER) {
			log("    (stack walk faulted)");
		}
#endif

		dumpLoadedModules("at crash");
		log("============================================================");
		if(s_logFile) std::fflush(s_logFile);

		// Let the previous handler (or default WER) run.
		return s_prevFilter ? s_prevFilter(ep) : EXCEPTION_CONTINUE_SEARCH;
	}
#endif // _WIN32

	void installCrashHandler()
	{
#ifdef _WIN32
		openLogFile();
		s_prevFilter = SetUnhandledExceptionFilter(crashFilter);
		log("crash handler installed (prev=%p)", s_prevFilter);
#endif
	}

	// Helpers: write a value at an absolute virtual address. The address
	// MUST live inside one of our reserved regions or this crashes.
	template <typename T>
	static SDL_INLINE void poke(Uint32 addr, T value)
	{
		*reinterpret_cast<T*>(static_cast<uintptr_t>(addr)) = value;
	}

	template <typename T>
	static SDL_INLINE T peek(Uint32 addr)
	{
		return *reinterpret_cast<volatile T*>(static_cast<uintptr_t>(addr));
	}

	// Write a C-string with a fixed total field width; pads with zeros.
	static SDL_INLINE void pokeFixedString(Uint32 addr, const char* src, size_t maxLen)
	{
		char* dst = reinterpret_cast<char*>(static_cast<uintptr_t>(addr));
		std::memset(dst, 0, maxLen);
		if(src)
		{
			size_t srcLen = std::strlen(src);
			if(srcLen > maxLen - 1)
				srcLen = maxLen - 1;
			std::memcpy(dst, src, srcLen);
		}
	}

	static SDL_INLINE void pokeFixedString(Uint32 addr, const std::string& s, size_t maxLen)
	{
		pokeFixedString(addr, s.c_str(), maxLen);
	}

	static void zeroRegion(void* base, size_t len)
	{
		if(base)
			std::memset(base, 0, len);
	}

	static bool isCommittedRange(uintptr_t base, SIZE_T size);

	static bool isLowTibiaRangeReady()
	{
		struct Range { uintptr_t base; SIZE_T size; const char* name; };
		static const Range ranges[] = {
			{ 0x00440000, 0x00190000, "text/code hooks" },
			{ 0x005B0000, 0x00090000, "rdata/RSA/DAT" },
			{ 0x00630000, 0x00040000, "player/battlelist/map" },
			{ 0x00790000, 0x00070000, "client globals/messages" },
		};

		for(size_t i = 0; i < sizeof(ranges) / sizeof(ranges[0]); ++i)
		{
			if(!isCommittedRange(ranges[i].base, ranges[i].size))
			{
				log("low Tibia range missing: %s at 0x%IX size=0x%IX",
					ranges[i].name, ranges[i].base, ranges[i].size);
				return false;
			}
		}
		return true;
	}

	static bool isCommittedRange(uintptr_t base, SIZE_T size)
	{
		MEMORY_BASIC_INFORMATION mbi;
		uintptr_t addr = base;
		const uintptr_t end = base + size;
		while(addr < end)
		{
			if(VirtualQuery(reinterpret_cast<LPCVOID>(addr), &mbi, sizeof(mbi)) == 0)
				return false;
			if(mbi.State != MEM_COMMIT)
				return false;
			if(mbi.Protect == PAGE_NOACCESS)
				return false;
			addr = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
		}
		return true;
	}

	static bool protectLowTibiaShadow()
	{
		// The shadow lives in .text$aaa for Tibia 8.60 address layout,
		// but MSVC/linker may keep .text read/execute. Make just this
		// backing range writable at runtime so TFC can start safely.
		const SIZE_T size = 0x006C0000;
		const volatile void* shadow = static_cast<const volatile void*>(&g_tibia860ImageShadow[0]);
		void* base = const_cast<void*>(shadow);
		DWORD oldProtect = 0;
		if(!VirtualProtect(base, size, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			log("protectLowTibiaShadow: VirtualProtect failed GLE=%lu", GetLastError());
			return false;
		}
		return true;
	}

	static bool initFakeDatPointer()
	{
		// Tibia 8.60's DAT begins at item ID 100. ElfBot's init loops
		// iterate item IDs starting around 100 up through ~12k, so the
		// first id MUST be 100 (not some huge sentinel). With a wrong
		// first id, the function at elfbot.dll+0x3C496 computes
		//   esi = (itemId - first_id) * 0x4C
		// which wraps modulo 2^32 to a random offset; writes land in
		// random places of our buffer, corrupting the per-item flag
		// tables ElfBot relies on. Symptoms when wrong: ElfBot doesn't
		// crash but its HUD / Lists / Looting tabs show no item data.
		const Uint32 kFirstThingId = 100u;
		const Uint32 kThingCount = 0x10000u;
		const SIZE_T kRecordSize = 0x4C;
		const SIZE_T kRecordsSize = static_cast<SIZE_T>(kThingCount) * kRecordSize;

		Uint8* records = static_cast<Uint8*>(VirtualAlloc(NULL, kRecordsSize,
			MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
		Uint32* descriptor = static_cast<Uint32*>(VirtualAlloc(NULL, 0x1000,
			MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
		if(!records || !descriptor)
		{
			log("initFakeDatPointer: VirtualAlloc failed");
			return false;
		}

		descriptor[0] = kFirstThingId;
		descriptor[1] = kFirstThingId + kThingCount - 1;
		descriptor[2] = static_cast<Uint32>(reinterpret_cast<uintptr_t>(records));
		poke<Uint32>(Addr::CLIENT_DAT_POINTER,
			static_cast<Uint32>(reinterpret_cast<uintptr_t>(descriptor)));
		return true;
	}

	static int __stdcall lowTibiaRecvHook(Uint32 socket, char* buffer, int len, int flags)
	{
		(void)socket;
		(void)buffer;
		(void)flags;
		static DWORD s_lastLogTick = 0;
		DWORD now = GetTickCount();
		if(now - s_lastLogTick > 1000)
		{
			s_lastLogTick = now;
			log("low-client recv hook called len=%d flags=%d", len, flags);
		}
		return -1;
	}

	static Direction directionFromOldWalkByte(Uint8 b)
	{
		switch(b)
		{
			case 1: return DIRECTION_EAST;
			case 2: return DIRECTION_NORTHEAST;
			case 3: return DIRECTION_NORTH;
			case 4: return DIRECTION_NORTHWEST;
			case 5: return DIRECTION_WEST;
			case 6: return DIRECTION_SOUTHWEST;
			case 7: return DIRECTION_SOUTH;
			case 8: return DIRECTION_SOUTHEAST;
			default: return DIRECTION_INVALID;
		}
	}

	static bool xteaDecryptBlock(unsigned char* data, size_t len, const Uint32 key[4])
	{
		if(!data || !key || len == 0 || (len % 8) != 0)
			return false;

		for(size_t off = 0; off < len; off += 8)
		{
			Uint32 v0 = 0, v1 = 0;
			std::memcpy(&v0, data + off + 0, sizeof(v0));
			std::memcpy(&v1, data + off + 4, sizeof(v1));

			Uint32 sum = 0xC6EF3720u;
			for(int i = 0; i < 32; ++i)
			{
				v1 -= (((v0 << 4) ^ (v0 >> 5)) + v0) ^ (sum + key[(sum >> 11) & 3]);
				sum -= 0x9E3779B9u;
				v0 -= (((v1 << 4) ^ (v1 >> 5)) + v1) ^ (sum + key[sum & 3]);
			}

			std::memcpy(data + off + 0, &v0, sizeof(v0));
			std::memcpy(data + off + 4, &v1, sizeof(v1));
		}
		return true;
	}

	static Uint16 readLe16(const unsigned char* p)
	{
		return static_cast<Uint16>(p[0] | (p[1] << 8));
	}

	static Uint32 readLe32(const unsigned char* p)
	{
		return static_cast<Uint32>(p[0] |
			(p[1] << 8) |
			(p[2] << 16) |
			(p[3] << 24));
	}

	static void queueOldEncryptedPacketAction(const unsigned char* packet, int len)
	{
		if(!s_hasXteaKey || !packet || len < 10)
			return;

		// Tibia 8.60 uses: [u16 packetLen][u32 checksum][xtea block...].
		// The decrypted block starts with [u16 plainLen][payload...].
		const int encryptedOffset = 6;
		const int encryptedLen = len - encryptedOffset;
		if(encryptedLen <= 0 || (encryptedLen % 8) != 0)
			return;

		std::vector<unsigned char> plain(static_cast<size_t>(encryptedLen));
		std::memcpy(&plain[0], packet + encryptedOffset, static_cast<size_t>(encryptedLen));
		if(!xteaDecryptBlock(&plain[0], plain.size(), s_xteaKey))
			return;

		Uint16 plainLen = readLe16(&plain[0]);
		if(plainLen == 0 || static_cast<size_t>(plainLen) + 2u > plain.size())
			return;

		const unsigned char* payload = &plain[2];
		Uint8 opcode = payload[0];
		log("old encrypted packet decrypted: opcode=0x%02X plainLen=%u", opcode, plainLen);

		if(opcode == GameAutowalkOpcode && plainLen >= 2)
		{
			Uint8 count = payload[1];
			if(count > 127)
				count = 127;
			if(static_cast<Uint16>(count) + 2u > plainLen)
				count = static_cast<Uint8>(plainLen > 2 ? plainLen - 2 : 0);

			Uint8 outCount = 0;
			for(Uint8 i = 0; i < count; ++i)
			{
				Direction d = directionFromOldWalkByte(payload[2 + i]);
				if(d != DIRECTION_INVALID)
					s_pendingPacketAutoWalkDirs[outCount++] = d;
			}
			if(outCount != 0)
			{
				s_pendingPacketAutoWalkCount = outCount;
				InterlockedExchange(&s_pendingPacketAutoWalk, 1);
				log("old packet queued autowalk directions=%u", outCount);
			}
		}
		else if(opcode >= GameWalkNorthOpcode && opcode <= GameWalkNorthWestOpcode)
		{
			Direction d = DIRECTION_INVALID;
			switch(opcode)
			{
				case GameWalkNorthOpcode:     d = DIRECTION_NORTH; break;
				case GameWalkEastOpcode:      d = DIRECTION_EAST; break;
				case GameWalkSouthOpcode:     d = DIRECTION_SOUTH; break;
				case GameWalkWestOpcode:      d = DIRECTION_WEST; break;
				case GameWalkNorthEastOpcode: d = DIRECTION_NORTHEAST; break;
				case GameWalkSouthEastOpcode: d = DIRECTION_SOUTHEAST; break;
				case GameWalkSouthWestOpcode: d = DIRECTION_SOUTHWEST; break;
				case GameWalkNorthWestOpcode: d = DIRECTION_NORTHWEST; break;
				default: break;
			}
			if(d != DIRECTION_INVALID)
			{
				s_pendingPacketWalkDir = d;
				InterlockedExchange(&s_pendingPacketWalk, 1);
				log("old packet queued single walk opcode=0x%02X", opcode);
			}
		}
		else if(opcode == GameAttackOpcode && plainLen >= 5)
		{
			s_pendingPacketAttackId = readLe32(payload + 1);
			InterlockedExchange(&s_pendingPacketAttack, 1);
			log("old packet queued attack id=%u", s_pendingPacketAttackId);
		}
		else if(opcode == GameFollowOpcode && plainLen >= 5)
		{
			s_pendingPacketFollowId = readLe32(payload + 1);
			InterlockedExchange(&s_pendingPacketFollow, 1);
			log("old packet queued follow id=%u", s_pendingPacketFollowId);
		}
	}

	static int __stdcall lowTibiaSendHook(Uint32 socket, const char* buffer, int len, int flags)
	{
		(void)socket;
		static unsigned int s_callCount = 0;
		++s_callCount;

		char hex[3 * 64 + 1] = {0};
		int n = len;
		if(n < 0)
			n = 0;
		if(n > 64)
			n = 64;

		for(int i = 0; i < n; ++i)
		{
			unsigned char ch = 0;
			__try {
				ch = static_cast<unsigned char>(buffer[i]);
			}
			__except(EXCEPTION_EXECUTE_HANDLER) {
				break;
			}
			_snprintf_s(hex + i * 3, sizeof(hex) - i * 3, _TRUNCATE, "%02X ", ch);
		}

		log("low-client send hook called #%u len=%d flags=%d bytes=%s",
			s_callCount, len, flags, hex);
		if(buffer && len > 0)
			queueOldEncryptedPacketAction(reinterpret_cast<const unsigned char*>(buffer), len);

		// Pretend the old Tibia socket accepted the packet. The real TFC
		// network path is separate; this hook is only to prove whether
		// ElfBot is reaching the original-client send path.
		return len > 0 ? len : 0;
	}

	static int __cdecl oldTibiaAutoWalkHook(Uint32 x, Uint32 y, Uint32 z, Uint32 mode, Uint32 flags)
	{
		// ElfBot calls Tibia.exe!0x00464070 when Cavebot wants to walk to
		// a waypoint. The original function depends on Tibia's map/path
		// internals, so queue the request for TFC's main loop instead.
		s_pendingOldWalkX = x;
		s_pendingOldWalkY = y;
		s_pendingOldWalkZ = z;
		poke<Uint32>(Addr::PLAYER_GOTO_X, x);
		poke<Uint32>(Addr::PLAYER_GOTO_Y, y);
		poke<Uint32>(Addr::PLAYER_GOTO_Z, z);
		InterlockedExchange(&s_pendingOldWalk, 1);
		log("old-tibia hook: autowalk x=%u y=%u z=%u mode=%u flags=%u",
			x, y, z, mode, flags);

		// Returning 0 prevents ElfBot from continuing into old Tibia's
		// follow-up packet/UI functions. TFC sends the real walk.
		return 0;
	}

	static void __cdecl oldTibiaSetAttackHook(Uint32 creatureId)
	{
		poke<Uint32>(Addr::PLAYER_RED_SQUARE, creatureId);
		poke<Uint32>(Addr::PLAYER_GREEN_SQUARE, 0);
		s_pendingOldAttackId = creatureId;
		InterlockedExchange(&s_pendingOldAttack, 1);
		log("old-tibia hook: attack id=%u", creatureId);
	}

	static void __cdecl oldTibiaSetFollowHook(Uint32 creatureId)
	{
		poke<Uint32>(Addr::PLAYER_GREEN_SQUARE, creatureId);
		poke<Uint32>(Addr::PLAYER_RED_SQUARE, 0);
		s_pendingOldFollowId = creatureId;
		InterlockedExchange(&s_pendingOldFollow, 1);
		log("old-tibia hook: follow id=%u", creatureId);
	}

	static void __cdecl oldTibiaSetTargetHook(Uint32 creatureId)
	{
		poke<Uint32>(Addr::PLAYER_TARGET_BATTLELIST_ID, creatureId);
		log("old-tibia hook: target-battlelist id=%u", creatureId);
	}

	static bool patchOldTibiaFunction(Uint32 address, void* hook, const char* name)
	{
		if(!isCommittedRange(address, 8))
		{
			log("old-tibia hook install skipped %s at 0x%08X (not committed)",
				name, address);
			return false;
		}

		unsigned char* dst = reinterpret_cast<unsigned char*>(
			static_cast<uintptr_t>(address));
		INT_PTR rel = reinterpret_cast<INT_PTR>(hook)
		            - (static_cast<INT_PTR>(address) + 5);
		if(rel < INT_MIN || rel > INT_MAX)
		{
			log("old-tibia hook install failed %s at 0x%08X (jump out of range)",
				name, address);
			return false;
		}

		DWORD oldProtect = 0;
		if(!VirtualProtect(dst, 8, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			log("old-tibia hook install failed %s at 0x%08X GLE=%lu",
				name, address, GetLastError());
			return false;
		}

		dst[0] = 0xE9;
		*reinterpret_cast<LONG*>(dst + 1) = static_cast<LONG>(rel);
		for(int i = 5; i < 8; ++i)
			dst[i] = 0x90;

		DWORD ignored = 0;
		VirtualProtect(dst, 8, oldProtect, &ignored);
		FlushInstructionCache(GetCurrentProcess(), dst, 8);
		log("old-tibia hook installed: %s 0x%08X -> %p", name, address, hook);
		return true;
	}

	static void installOldTibiaActionHooks(bool tibiaLoaded)
	{
		if(!tibiaLoaded)
		{
			log("old-tibia hooks skipped: external image disabled");
			return;
		}

		patchOldTibiaFunction(0x00464070, reinterpret_cast<void*>(&oldTibiaAutoWalkHook), "autowalk");
		patchOldTibiaFunction(0x0045D810, reinterpret_cast<void*>(&oldTibiaSetAttackHook), "set-attack");
		patchOldTibiaFunction(0x0045D830, reinterpret_cast<void*>(&oldTibiaSetFollowHook), "set-follow");
		patchOldTibiaFunction(0x0045D850, reinterpret_cast<void*>(&oldTibiaSetTargetHook), "set-target");
	}

	static bool initTibiaPointerSlots(bool tibiaLoaded)
	{
		const SIZE_T kSafe = 0x100000;
		void* safe = VirtualAlloc(NULL, kSafe, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
		if(!safe)
		{
			log("initTibiaPointerSlots: safe VirtualAlloc failed");
			return false;
		}

		Uint32 selfAddr = static_cast<Uint32>(reinterpret_cast<uintptr_t>(safe));
		Uint32* p = static_cast<Uint32*>(safe);
		for(SIZE_T i = 0; i < kSafe / sizeof(Uint32); ++i)
			p[i] = selfAddr;

		if(!tibiaLoaded)
		{
			log("initTibiaPointerSlots: low memory bulk prefill skipped");
		}

		// Original Tibia wrappers call these WS2_32 IAT slots:
		//   0x0057D503 -> [0x005B85E4] recv
		//   0x0057D51B -> [0x005B8610] send
		// Point them at logging hooks instead of the safe pointer so we
		// can prove when ElfBot reaches the old-client network path.
		poke<Uint32>(0x005B85E4,
			static_cast<Uint32>(reinterpret_cast<uintptr_t>(&lowTibiaRecvHook)));
		poke<Uint32>(0x005B8610,
			static_cast<Uint32>(reinterpret_cast<uintptr_t>(&lowTibiaSendHook)));
		poke<Uint32>(Addr::MAP_POINTER, selfAddr);
		poke<Uint32>(0x0079DA74, selfAddr);  // Client.FrameRatePointer
		poke<Uint32>(0x00799890, selfAddr);  // Client.SocketStruct
		poke<Uint32>(0x0064C25C, selfAddr);  // Client.GameWindowRectPointer
		poke<Uint32>(0x0064F5C4, selfAddr);  // Client.DialogPointer
		poke<Uint32>(0x00795070, selfAddr);  // Client.LastRcvPacket
		poke<Uint32>(0x007998AC, selfAddr);  // Client.RecvStream
		poke<Uint32>(0x0051F650, selfAddr);  // Client.EventTriggerPointer
		poke<Uint32>(0x0051D200, selfAddr);  // Client.ActionStateFreezer

		initFakeDatPointer();
		log("initTibiaPointerSlots: pointer slots initialised");
		return true;
	}

	static void pokeStringRaw(Uint32 addr, const char* src, size_t maxLen)
	{
		pokeFixedString(addr, src, maxLen);
	}

	static std::string readStringRaw(Uint32 addr, size_t maxLen)
	{
		const char* src = reinterpret_cast<const char*>(static_cast<uintptr_t>(addr));
		std::string out;
		for(size_t i = 0; i < maxLen; ++i)
		{
			char ch = src[i];
			if(ch == '\0')
				break;

			unsigned char uch = static_cast<unsigned char>(ch);
			if(uch < 32 || uch > 126)
				break;

			out.push_back(ch);
		}
		return out;
	}

	static std::string trimAscii(const std::string& s)
	{
		size_t first = 0;
		while(first < s.size() && (s[first] == ' ' || s[first] == '\t' || s[first] == '\r' || s[first] == '\n'))
			++first;

		size_t last = s.size();
		while(last > first && (s[last - 1] == ' ' || s[last - 1] == '\t' || s[last - 1] == '\r' || s[last - 1] == '\n'))
			--last;

		return s.substr(first, last - first);
	}

	static std::string lowerAscii(std::string s)
	{
		for(size_t i = 0; i < s.size(); ++i)
		{
			if(s[i] >= 'A' && s[i] <= 'Z')
				s[i] = static_cast<char>(s[i] - 'A' + 'a');
		}
		return s;
	}

	static bool parseElfbotSayCommand(const std::string& raw, std::string& message, Uint32& intervalMs, bool& isAuto)
	{
		std::string s = trimAscii(raw);
		if(s.empty() || s.size() > 200)
			return false;

		std::string lower = lowerAscii(s);
		intervalMs = 0;
		isAuto = false;

		if(lower.compare(0, 5, "auto ") == 0)
		{
			isAuto = true;
			size_t pos = 5;
			Uint32 parsedInterval = 0;
			while(pos < lower.size() && lower[pos] >= '0' && lower[pos] <= '9')
			{
				parsedInterval = parsedInterval * 10 + static_cast<Uint32>(lower[pos] - '0');
				++pos;
			}

			while(pos < lower.size() && lower[pos] == ' ')
				++pos;

			if(lower.compare(pos, 4, "say ") != 0)
				return false;

			intervalMs = parsedInterval > 0 ? parsedInterval : 200;
			if(intervalMs < 100)
				intervalMs = 100;
			message = trimAscii(s.substr(pos + 4));
			return !message.empty();
		}

		if(lower.compare(0, 4, "say ") == 0)
		{
			message = trimAscii(s.substr(4));
			return !message.empty();
		}

		// Some Tibia paths contain only the final text, not the "say" verb.
		if(lower.find("auto ") == std::string::npos)
		{
			message = s;
			return !message.empty();
		}

		return false;
	}

#ifdef _WIN32
	static bool isReadablePointer(const void* ptr, size_t len)
	{
		MEMORY_BASIC_INFORMATION mbi;
		if(VirtualQuery(ptr, &mbi, sizeof(mbi)) == 0)
			return false;

		if(mbi.State != MEM_COMMIT || (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)))
			return false;

		const uintptr_t start = reinterpret_cast<uintptr_t>(ptr);
		const uintptr_t end = start + len;
		const uintptr_t regionEnd = reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize;
		return end >= start && end <= regionEnd;
	}

	static bool isScanReadableProtect(DWORD protect)
	{
		if(protect & (PAGE_NOACCESS | PAGE_GUARD))
			return false;

		DWORD p = protect & 0xFF;
		return p == PAGE_READONLY ||
		       p == PAGE_READWRITE ||
		       p == PAGE_WRITECOPY ||
		       p == PAGE_EXECUTE_READ ||
		       p == PAGE_EXECUTE_READWRITE ||
		       p == PAGE_EXECUTE_WRITECOPY;
	}

	static bool isScanWritableProtect(DWORD protect)
	{
		if(protect & (PAGE_NOACCESS | PAGE_GUARD))
			return false;

		DWORD p = protect & 0xFF;
		return p == PAGE_READWRITE ||
		       p == PAGE_WRITECOPY ||
		       p == PAGE_EXECUTE_READWRITE ||
		       p == PAGE_EXECUTE_WRITECOPY;
	}

	static const char* memoryTypeName(DWORD type)
	{
		switch(type)
		{
			case MEM_IMAGE:   return "image";
			case MEM_MAPPED:  return "mapped";
			case MEM_PRIVATE: return "private";
			default:          return "unknown";
		}
	}

	static std::string readAsciiBounded(const char* ptr, const char* limit, size_t maxLen)
	{
		std::string out;
		while(ptr < limit && out.size() < maxLen)
		{
			char ch = *ptr++;
			if(ch == '\0')
				break;

			unsigned char uch = static_cast<unsigned char>(ch);
			if(uch < 32 || uch > 126)
				break;

			out.push_back(ch);
		}
		return out;
	}

	static void logSingleModule(const char* name)
	{
		HMODULE mod = GetModuleHandleA(name);
		if(!mod)
		{
			log("  %s: not loaded", name);
			return;
		}

		char path[MAX_PATH] = {0};
		GetModuleFileNameA(mod, path, MAX_PATH);
		MODULEINFO mi = {0};
		GetModuleInformation(GetCurrentProcess(), mod, &mi, sizeof(mi));
		log("  %s: %p..%p size=0x%lX path=%s",
			name,
			mi.lpBaseOfDll,
			reinterpret_cast<void*>(
				reinterpret_cast<uintptr_t>(mi.lpBaseOfDll) + mi.SizeOfImage),
			mi.SizeOfImage,
			path);
	}

	static void dumpElfbotRelatedModules(const char* tag)
	{
		log("--- ElfBot related modules (%s) ---", tag ? tag : "");
		logSingleModule("elfload.dll");
		logSingleModule("USkin.dll");
		logSingleModule("elfbot.dll");
	}

	static std::string getExeDirectory()
	{
		char exePath[MAX_PATH] = {0};
		DWORD n = GetModuleFileNameA(NULL, exePath, MAX_PATH);
		if(n == 0 || n >= MAX_PATH)
			return std::string();

		std::string dir = exePath;
		size_t slash = dir.find_last_of("\\/");
		if(slash != std::string::npos)
			dir.resize(slash + 1);
		else
			dir.clear();
		return dir;
	}

	static bool writeZeroBytes(FILE* f, SIZE_T bytes)
	{
		unsigned char zeros[4096] = {0};
		while(bytes != 0)
		{
			SIZE_T n = bytes > sizeof(zeros) ? sizeof(zeros) : bytes;
			if(std::fwrite(zeros, 1, n, f) != n)
				return false;
			bytes -= n;
		}
		return true;
	}

	static void scanElfbotImageForConstants(HMODULE elfbot, SIZE_T imageSize)
	{
		struct ConstNeedle { const char* name; Uint32 value; };
		static const ConstNeedle needles[] = {
			{ "CLIENT_STATUS",          Addr::CLIENT_STATUS },
			{ "GAMETICK_COUNTER",       Addr::CLIENT_GAMETICK_COUNTER },
			{ "PLAYER_ID",              Addr::PLAYER_ID },
			{ "PLAYER_EXPERIENCE",      Addr::PLAYER_EXPERIENCE },
			{ "PLAYER_LEVEL",           Addr::PLAYER_LEVEL },
			{ "PLAYER_RED_SQUARE",      Addr::PLAYER_RED_SQUARE },
			{ "PLAYER_TARGET_ID",       Addr::PLAYER_TARGET_BATTLELIST_ID },
			{ "PLAYER_X",               Addr::PLAYER_X },
			{ "PLAYER_Y",               Addr::PLAYER_Y },
			{ "PLAYER_BATTLE_Z",        Addr::PLAYER_BATTLE_Z },
			{ "PLAYER_Z",               Addr::PLAYER_Z },
			{ "LEGACY_POS_X",           0x0064F608 },
			{ "LEGACY_POS_Y",           0x0064F604 },
			{ "LEGACY_POS_Z",           0x0064F600 },
			{ "LEGACY_POS2_X",          0x006405AC },
			{ "LEGACY_POS2_Y",          0x006405B0 },
			{ "LEGACY_POS2_Z",          0x006405B4 },
			{ "BATTLELIST_START",       Addr::BATTLELIST_START },
			{ "CONTAINER_START",        Addr::CONTAINER_START },
			{ "HOTKEY_OBJECT_USE",      Addr::HOTKEY_OBJECT_USE },
			{ "HOTKEY_OBJECT_COUNT",    Addr::HOTKEY_OBJECT_COUNT },
			{ "HOTKEY_OBJECT_START",    Addr::HOTKEY_OBJECT_START },
			{ "HOTKEY_AUTO_START",      Addr::HOTKEY_AUTO_START },
			{ "HOTKEY_TEXT_START",      Addr::HOTKEY_TEXT_START },
			{ "STATUSBAR_TEXT",         Addr::CLIENT_STATUSBAR_TEXT },
			{ "HOTKEY_TEXT_A",          Addr::CLIENT_HOTKEY_TEXT_A },
			{ "HOTKEY_TEXT_B",          Addr::CLIENT_HOTKEY_TEXT_B },
			{ "HOTKEY_TEXT_C",          Addr::CLIENT_HOTKEY_TEXT_C },
			{ "OLD_SEND_IAT",           0x005B8610 },
			{ "OLD_SEND_WRAPPER",       0x0057D51B },
			{ "OLD_RECV_IAT",           0x005B85E4 },
			{ "OLD_RECV_WRAPPER",       0x0057D503 },
		};

		struct StringNeedle { const char* name; const char* text; };
		static const StringNeedle strings[] = {
			{ "startup", "Startup" },
			{ "slot", "Slot" },
			{ "please-run-tibia", "Please run Tibia first" },
			{ "auto-on", "Auto ON" },
			{ "cavebot", "Cavebot" },
			{ "targeting", "Targeting" },
			{ "hotkeys", "Hotkeys" },
			{ "stand", "Stand" },
			{ "ascar", "www.ascar.us" },
		};

		uintptr_t imageBase = reinterpret_cast<uintptr_t>(elfbot);
		uintptr_t cursor = imageBase;
		uintptr_t imageEnd = imageBase + imageSize;
		unsigned int constCounts[sizeof(needles) / sizeof(needles[0])] = {0};
		unsigned int stringCounts[sizeof(strings) / sizeof(strings[0])] = {0};

		while(cursor < imageEnd)
		{
			MEMORY_BASIC_INFORMATION mbi;
			if(VirtualQuery(reinterpret_cast<LPCVOID>(cursor), &mbi, sizeof(mbi)) == 0)
				break;

			uintptr_t base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
			uintptr_t end = base + mbi.RegionSize;
			if(end <= cursor)
				break;

			uintptr_t scanStart = cursor > base ? cursor : base;
			uintptr_t scanEnd = end < imageEnd ? end : imageEnd;
			if(mbi.State == MEM_COMMIT && isScanReadableProtect(mbi.Protect) && scanEnd > scanStart)
			{
				const unsigned char* bytes = reinterpret_cast<const unsigned char*>(scanStart);
				SIZE_T len = scanEnd - scanStart;
				for(SIZE_T off = 0; off + sizeof(Uint32) <= len; ++off)
				{
					Uint32 v = 0;
					std::memcpy(&v, bytes + off, sizeof(v));
					for(size_t i = 0; i < sizeof(needles) / sizeof(needles[0]); ++i)
					{
						if(v != needles[i].value)
							continue;
						++constCounts[i];
						if(constCounts[i] <= 16)
							log("elfbot const-ref %-20s 0x%08X at +0x%08IX",
								needles[i].name, needles[i].value,
								(scanStart + off) - imageBase);
					}
				}

				for(SIZE_T off = 0; off < len; ++off)
				{
					for(size_t i = 0; i < sizeof(strings) / sizeof(strings[0]); ++i)
					{
						size_t slen = std::strlen(strings[i].text);
						if(off + slen > len)
							continue;
						if(std::memcmp(bytes + off, strings[i].text, slen) != 0)
							continue;
						++stringCounts[i];
						if(stringCounts[i] <= 16)
							log("elfbot string %-18s at +0x%08IX text='%s'",
								strings[i].name, (scanStart + off) - imageBase, strings[i].text);
					}
				}
			}

			cursor = end;
		}

		for(size_t i = 0; i < sizeof(needles) / sizeof(needles[0]); ++i)
		{
			if(constCounts[i] > 16)
				log("elfbot const-ref %-20s total=%u", needles[i].name, constCounts[i]);
		}
		for(size_t i = 0; i < sizeof(strings) / sizeof(strings[0]); ++i)
		{
			if(stringCounts[i] > 16)
				log("elfbot string %-18s total=%u", strings[i].name, stringCounts[i]);
		}
	}

	static void dumpAndScanElfbotImageOnce()
	{
		static bool s_done = false;
		if(s_done)
			return;

		HMODULE elfbot = GetModuleHandleA("elfbot.dll");
		if(!elfbot)
			return;
		s_done = true;

		MODULEINFO mi = {0};
		if(!GetModuleInformation(GetCurrentProcess(), elfbot, &mi, sizeof(mi)) ||
		   !mi.lpBaseOfDll || mi.SizeOfImage == 0)
		{
			log("elfbot dump: GetModuleInformation failed");
			return;
		}

		std::string path = getExeDirectory();
		char fileName[64] = {0};
		_snprintf_s(fileName, sizeof(fileName), _TRUNCATE,
			"elfbot_unpacked_%08IX.bin", reinterpret_cast<uintptr_t>(mi.lpBaseOfDll));
		path += fileName;

		FILE* f = NULL;
		if(fopen_s(&f, path.c_str(), "wb") != 0 || !f)
		{
			log("elfbot dump: failed to open %s", path.c_str());
			scanElfbotImageForConstants(elfbot, mi.SizeOfImage);
			return;
		}

		uintptr_t imageBase = reinterpret_cast<uintptr_t>(mi.lpBaseOfDll);
		uintptr_t cursor = imageBase;
		uintptr_t imageEnd = imageBase + mi.SizeOfImage;
		bool ok = true;
		while(cursor < imageEnd)
		{
			MEMORY_BASIC_INFORMATION mbi;
			if(VirtualQuery(reinterpret_cast<LPCVOID>(cursor), &mbi, sizeof(mbi)) == 0)
			{
				ok = false;
				break;
			}

			uintptr_t base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
			uintptr_t end = base + mbi.RegionSize;
			if(end <= cursor)
			{
				ok = false;
				break;
			}

			uintptr_t chunkStart = cursor > base ? cursor : base;
			uintptr_t chunkEnd = end < imageEnd ? end : imageEnd;
			SIZE_T chunkSize = chunkEnd - chunkStart;
			if(mbi.State == MEM_COMMIT && isScanReadableProtect(mbi.Protect))
			{
				if(std::fwrite(reinterpret_cast<const void*>(chunkStart), 1, chunkSize, f) != chunkSize)
				{
					ok = false;
					break;
				}
			}
			else if(!writeZeroBytes(f, chunkSize))
			{
				ok = false;
				break;
			}

			cursor = chunkEnd;
		}

		std::fclose(f);
		log("elfbot dump: %s size=0x%lX ok=%d", path.c_str(), mi.SizeOfImage, ok ? 1 : 0);
		scanElfbotImageForConstants(elfbot, mi.SizeOfImage);
	}

	static bool shouldScanRegion(const MEMORY_BASIC_INFORMATION& mbi, uintptr_t elfbotBase, SIZE_T scannedBytes, bool deepProbe)
	{
		if(mbi.State != MEM_COMMIT || !isScanReadableProtect(mbi.Protect))
			return false;

		uintptr_t base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
		uintptr_t end = base + mbi.RegionSize;

		// Skip the known low Tibia mirror. We already log these absolute
		// addresses directly; this probe is for ElfBot's own image/heap state.
		if(base < 0x00820000 && end > 0x00400000)
			return false;

		if(mbi.Type == MEM_IMAGE)
			return reinterpret_cast<uintptr_t>(mbi.AllocationBase) == elfbotBase;

		if(!deepProbe || mbi.Type != MEM_PRIVATE)
			return false;

		// Scan only bounded private heap. ElfBot stores its Cavebot
		// waypoint list, Targeting entries, and enabled flags in heap
		// allocations, not in the unpacked image. A full process heap
		// sweep can stall the client, so cap both per-region and total.
		if(mbi.RegionSize > 0x00400000)
			return false;
		if(scannedBytes >= 0x01800000)
			return false;

		return true;
	}

	static bool isElfbotDeepProbeEnabled()
	{
		static int s_cached = -1;
		static DWORD s_lastCheck = 0;
		DWORD now = GetTickCount();
		if(s_cached >= 0 && now - s_lastCheck < 1000)
			return s_cached != 0;
		s_lastCheck = now;

		char exePath[MAX_PATH] = {0};
		DWORD n = GetModuleFileNameA(NULL, exePath, MAX_PATH);
		if(n == 0 || n >= MAX_PATH)
		{
			s_cached = 0;
			return false;
		}

		std::string marker = exePath;
		size_t slash = marker.find_last_of("\\/");
		if(slash != std::string::npos)
			marker.resize(slash + 1);
		else
			marker.clear();
		marker += "elfbot_deep_probe.txt";

		DWORD attrs = GetFileAttributesA(marker.c_str());
		s_cached = (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) ? 1 : 0;
		return s_cached != 0;
	}

	static void logMatchAddress(const char* label, uintptr_t addr, const MEMORY_BASIC_INFORMATION& mbi, const char* detail)
	{
		char where[MAX_PATH + 64] = {0};
		describeAddress(reinterpret_cast<void*>(addr), where, sizeof(where));
		log("  %s addr=0x%08IX alloc=0x%08IX type=%s prot=0x%lX %s%s%s",
			label,
			addr,
			reinterpret_cast<uintptr_t>(mbi.AllocationBase),
			memoryTypeName(mbi.Type),
			mbi.Protect,
			where,
			detail && detail[0] ? " " : "",
			detail && detail[0] ? detail : "");
	}

	static void probeElfbotInternalState(const TibiaShimBlock* shim, bool force)
	{
		if(!shim || shim->status != 8 || shim->creatureCount == 0)
			return;

		HMODULE elfbot = GetModuleHandleA("elfbot.dll");
		if(!elfbot)
			return;

		static DWORD s_lastProbeTick = 0;
		DWORD now = GetTickCount();
		if(!force && now - s_lastProbeTick < 5000)
			return;
		s_lastProbeTick = now;
		const bool deepProbe = force || isElfbotDeepProbeEnabled();

		const TibiaShimCreature& self = shim->creatures[0];
		const char* playerName = self.name;
		size_t playerNameLen = playerName[0] ? std::strlen(playerName) : 0;
		if(playerNameLen > 31)
			playerNameLen = 31;

		Uint32 x = self.x;
		Uint32 y = self.y;
		Uint32 z = self.z;
		Uint32 id = shim->player.id;

		log("elfbot probe: status=%u id=%u level=%u pos=%u,%u,%u name=%s",
			shim->status, id, shim->player.level, x, y, z,
			playerName[0] ? playerName : "<empty>");

		uintptr_t elfbotBase = reinterpret_cast<uintptr_t>(elfbot);
		uintptr_t cursor = 0x00100000;
		const uintptr_t maxAddr = 0x70000000;
		SIZE_T scannedBytes = 0;
		unsigned int nameMatches = 0;
		unsigned int creatureNameMatches = 0;
		unsigned int posMatches = 0;
		unsigned int posXYZeroMatches = 0;
		unsigned int nearPosMatches = 0;
		unsigned int idMatches = 0;
		unsigned int scriptMatches = 0;

		while(cursor < maxAddr)
		{
			MEMORY_BASIC_INFORMATION mbi;
			if(VirtualQuery(reinterpret_cast<LPCVOID>(cursor), &mbi, sizeof(mbi)) == 0)
				break;

			uintptr_t base = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
			uintptr_t end = base + mbi.RegionSize;
			if(end <= cursor)
				break;

			if(shouldScanRegion(mbi, elfbotBase, scannedBytes, deepProbe))
			{
				const unsigned char* bytes = reinterpret_cast<const unsigned char*>(base);
				SIZE_T len = mbi.RegionSize;
				if(scannedBytes + len > 0x01800000)
					len = 0x01800000 - scannedBytes;
				scannedBytes += len;

				if(playerNameLen >= 2)
				{
					for(SIZE_T off = 0; off + playerNameLen <= len; ++off)
					{
						if(std::memcmp(bytes + off, playerName, playerNameLen) != 0)
							continue;

						++nameMatches;
						if(nameMatches <= 16)
						{
							std::string s = readAsciiBounded(
								reinterpret_cast<const char*>(bytes + off),
								reinterpret_cast<const char*>(bytes + len),
								64);
							char detail[96] = {0};
							_snprintf_s(detail, sizeof(detail), _TRUNCATE, "text='%s'", s.c_str());
							logMatchAddress("name-match", base + off, mbi, detail);
						}
					}
				}

				for(Uint32 ci = 1; ci < shim->creatureCount && ci < 16; ++ci)
				{
					const char* creatureName = shim->creatures[ci].name;
					size_t creatureNameLen = creatureName[0] ? std::strlen(creatureName) : 0;
					if(creatureNameLen < 2 || creatureNameLen > 31)
						continue;

					for(SIZE_T off = 0; off + creatureNameLen <= len; ++off)
					{
						if(std::memcmp(bytes + off, creatureName, creatureNameLen) != 0)
							continue;

						++creatureNameMatches;
						if(creatureNameMatches <= 24)
						{
							std::string s = readAsciiBounded(
								reinterpret_cast<const char*>(bytes + off),
								reinterpret_cast<const char*>(bytes + len),
								64);
							char detail[128] = {0};
							_snprintf_s(detail, sizeof(detail), _TRUNCATE,
								"name='%s' text='%s'", creatureName, s.c_str());
							logMatchAddress("creature-name-match", base + off, mbi, detail);
						}
					}
				}

				for(SIZE_T off = 0; off + 12 <= len; off += 4)
				{
					Uint32 a = 0, b = 0, c = 0;
					std::memcpy(&a, bytes + off + 0, sizeof(Uint32));
					std::memcpy(&b, bytes + off + 4, sizeof(Uint32));
					std::memcpy(&c, bytes + off + 8, sizeof(Uint32));

					if(a == x && b == y && c == z)
					{
						++posMatches;
						if(posMatches <= 24)
							logMatchAddress("pos-xyz-match", base + off, mbi, "");
					}
					else if(a == x && b == y && c == 0)
					{
						++posXYZeroMatches;
						if(posXYZeroMatches <= 12)
							logMatchAddress("pos-xy0-match", base + off, mbi, "");
					}
					else if(isPlausiblePosition(a, b, c) &&
					        c == z &&
					        deltaAbs(a, x) <= 100 &&
					        deltaAbs(b, y) <= 100)
					{
						++nearPosMatches;
						if(nearPosMatches <= 32)
						{
							char detail[96] = {0};
							_snprintf_s(detail, sizeof(detail), _TRUNCATE,
								"triple=%u,%u,%u", a, b, c);
							logMatchAddress("near-pos-triple", base + off, mbi, detail);
						}
					}

					if(id != 0 && a == id)
					{
						++idMatches;
						if(idMatches <= 12)
							logMatchAddress("id-match", base + off, mbi, "");
					}
				}

				for(SIZE_T off = 0; off + 5 < len; ++off)
				{
					bool script = std::memcmp(bytes + off, "auto ", 5) == 0 ||
					              std::memcmp(bytes + off, "say ", 4) == 0;
					if(!script)
						continue;

					++scriptMatches;
					if(scriptMatches <= 24)
					{
						std::string s = readAsciiBounded(
							reinterpret_cast<const char*>(bytes + off),
							reinterpret_cast<const char*>(bytes + len),
							140);
						char detail[180] = {0};
						_snprintf_s(detail, sizeof(detail), _TRUNCATE, "text='%s'", s.c_str());
						logMatchAddress("script-match", base + off, mbi, detail);
					}
				}
			}

			cursor = end;
		}

		log("elfbot probe summary: scanned=%Iu name=%u creatureNames=%u posXYZ=%u posXY0=%u nearPos=%u id=%u scripts=%u",
			scannedBytes,
			nameMatches,
			creatureNameMatches,
			posMatches,
			posXYZeroMatches,
			nearPosMatches,
			idMatches,
			scriptMatches);
	}
#endif

	// ------------------------------------------------------------------
	// Alternate "0x636XXX" player layout.
	//
	// The ElfBot build the user is running reads player stats from a
	// completely different address table than the TibiaAPI Version860.cs
	// layout we initially targeted. The alternate table (sourced from
	// the user's spec) places Health, Mana, Experience, Level, Position,
	// Target ID etc. in the 0x6366xx-0x6368xx range -- much lower than
	// TibiaAPI's 0x63FExx. Both ranges fall inside our 6.75 MB shadow,
	// so writing to both layouts is safe and free.
	//
	// Player.Name in this layout is referenced via a POINTER at
	// 0x006368C8. We park a dedicated 64-byte name buffer at a fixed
	// shadow offset, write the name string there, and set the pointer
	// to that buffer's address.
	// ------------------------------------------------------------------
	static const Uint32 ALT_HEALTH       = 0x006366DC;
	static const Uint32 ALT_HEALTH_MAX   = 0x006366E0;  // assumed +4 after HP
	static const Uint32 ALT_MANA         = 0x006366E4;
	static const Uint32 ALT_MANA_MAX     = 0x006366E8;
	static const Uint32 ALT_MAGIC_LEVEL  = 0x006366EC;
	static const Uint32 ALT_LEVEL        = 0x00636734;
	static const Uint32 ALT_CAPACITY     = 0x006367EC;
	static const Uint32 ALT_SOUL         = 0x00636830;
	static const Uint32 ALT_EXPERIENCE   = 0x00636898;
	static const Uint32 ALT_X            = 0x006366C8;
	static const Uint32 ALT_Y            = 0x006366CC;
	static const Uint32 ALT_Z            = 0x006366D0;
	static const Uint32 ALT_CONDITIONS   = 0x00636744;
	static const Uint32 ALT_FISHING      = 0x00636784;
	static const Uint32 ALT_FOOD_TIMER   = 0x00636750;
	static const Uint32 ALT_NAME_POINTER = 0x006368C8;
	static const Uint32 ALT_TARGET_ID    = 0x006368D0;
	static const Uint32 ALT_FOLLOW_ID    = 0x006368D4;
	static const Uint32 ALT_ATTACK_MODE  = 0x006368D8;

	// 64-byte scratch buffer for the player name string. Lives inside
	// the shadow at a fixed, well-clear-of-anything-else slot.
	static const Uint32 ALT_NAME_BUFFER  = 0x00637E00;

	// Extra original-client position mirror confirmed by repeated scans.
	static const Uint32 LEGACY_POS_X     = 0x0064F608;
	static const Uint32 LEGACY_POS_Y     = 0x0064F604;
	static const Uint32 LEGACY_POS_Z     = 0x0064F600;
	static const Uint32 LEGACY_POS2_X    = 0x006405AC;
	static const Uint32 LEGACY_POS2_Y    = 0x006405B0;
	static const Uint32 LEGACY_POS2_Z    = 0x006405B4;
	static const Uint32 CLIENT_POS_Y1    = 0x0079CF34;
	static const Uint32 CLIENT_POS_X1    = 0x0079CF38;
	static const Uint32 CLIENT_POS_Y2    = 0x0079CF44;
	static const Uint32 CLIENT_POS_X2    = 0x0079CF48;
	static const Uint32 CLIENT_POS_Y3    = 0x0079CF60;
	static const Uint32 CLIENT_POS_X3    = 0x0079CF64;

	static void applyShimAlternateLayout(const TibiaShimBlock* shim)
	{
		// Stats (treated as direct 32-bit values).
		poke<Uint32>(ALT_HEALTH,       shim->player.health);
		poke<Uint32>(ALT_HEALTH_MAX,   shim->player.healthMax);
		poke<Uint32>(ALT_MANA,         shim->player.mana);
		poke<Uint32>(ALT_MANA_MAX,     shim->player.manaMax);
		poke<Uint32>(ALT_LEVEL,        shim->player.level);
		poke<Uint32>(ALT_MAGIC_LEVEL,  shim->player.magicLevel);
		poke<Uint32>(ALT_CAPACITY,     shim->player.capacity);
		poke<Uint32>(ALT_SOUL,         shim->player.soul);
		poke<Uint64>(ALT_EXPERIENCE,   shim->player.experience);

		// Position. Read from BattleList[0] (self) which already has it.
		const TibiaShimCreature& selfCr = shim->creatures[0];
		poke<Uint32>(ALT_X, selfCr.x);
		poke<Uint32>(ALT_Y, selfCr.y);
		poke<Uint32>(ALT_Z, selfCr.z);

		// Targeting / following.
		poke<Uint32>(ALT_TARGET_ID,    shim->player.targetId);
		poke<Uint32>(ALT_FOLLOW_ID,    shim->player.followId);

		// Name lives at the buffer; the pointer at ALT_NAME_POINTER
		// dereferences to it. Write the name once per frame (cheap)
		// and set the pointer.
		pokeStringRaw(ALT_NAME_BUFFER, selfCr.name, 31);
		poke<Uint32>(ALT_NAME_POINTER, ALT_NAME_BUFFER);
	}

	static void patchElfbotCachedPlayerName(const TibiaShimBlock* shim)
	{
		if(shim->status != 8 || shim->creatures[0].name[0] == '\0')
			return;

		HMODULE elfbot = GetModuleHandleA("elfbot.dll");
		if(!elfbot)
			return;

		// In this ElfBot build, the header/title player-name cache sits at
		// elfbot.dll+0x5F7B1. Verified against original Tibia.exe where the
		// same slot contained "GM Snarkox" beside the www.ascar.us strings.
		static const uintptr_t kElfbotPlayerNameOffset = 0x5F7B1;
		char* dst = reinterpret_cast<char*>(
			reinterpret_cast<uintptr_t>(elfbot) + kElfbotPlayerNameOffset);

		DWORD oldProtect = 0;
		if(!VirtualProtect(dst, 32, PAGE_READWRITE, &oldProtect))
			return;

		static char s_lastName[32] = {0};
		if(std::strncmp(s_lastName, shim->creatures[0].name, sizeof(s_lastName) - 1) != 0)
		{
			std::memset(dst, 0, 32);
			strncpy_s(dst, 32, shim->creatures[0].name, _TRUNCATE);
			strncpy_s(s_lastName, sizeof(s_lastName), shim->creatures[0].name, _TRUNCATE);
			s_lastName[sizeof(s_lastName) - 1] = '\0';
			log("elfbot cached name patched: base=%p addr=%p name=%s",
				elfbot, dst, s_lastName);
		}

		DWORD ignored = 0;
		VirtualProtect(dst, 32, oldProtect, &ignored);
	}

	static void patchElfbotCurrentPositionCache(const TibiaShimBlock* shim)
	{
		if(shim->status != 8 || shim->creatureCount == 0)
			return;

		HMODULE elfbot = GetModuleHandleA("elfbot.dll");
		if(!elfbot)
			return;

		const TibiaShimCreature& self = shim->creatures[0];
		if(!(self.x > 0 && self.x < 0xFFFFu &&
		     self.y > 0 && self.y < 0xFFFFu &&
		     self.z <= 15))
			return;

		struct PosSlot { uintptr_t offset; Uint32 value; };
		PosSlot slots[] = {
			// Static current-position cache groups. Do not include dynamic
			// cavebot row memory. The first group is from the user's CE
			// scans of ElfBot's own current-position copies:
			//   elfbot.dll+0x52768 = X
			//   elfbot.dll+0x5276C = Y
			//   elfbot.dll+0x52770 = Z
			// The previous 0x5E268 value was a typo and never touched the
			// cache used by Cavebot's "Stand" button.
			{0x052768, self.x},
			{0x05276C, self.y},
			{0x052770, self.z},
			// Keep the previous guessed group too in case this packed build
			// moves one of the copies by one page.
			{0x05F268, self.x},
			{0x05F27C, self.y},
			{0x05F270, self.z},
			{0x05F854, self.x},
			{0x05F858, self.y},
			{0x05F85C, self.z},
			{0x05F860, self.x},
			{0x05F864, self.y},
			{0x05F868, self.z},
		};

		for(size_t i = 0; i < sizeof(slots) / sizeof(slots[0]); ++i)
		{
			Uint32* dst = reinterpret_cast<Uint32*>(
				reinterpret_cast<uintptr_t>(elfbot) + slots[i].offset);

			DWORD oldProtect = 0;
			if(!VirtualProtect(dst, sizeof(Uint32), PAGE_READWRITE, &oldProtect))
				continue;

			*dst = slots[i].value;

			DWORD ignored = 0;
			VirtualProtect(dst, sizeof(Uint32), oldProtect, &ignored);
		}

		static Uint32 s_lastX = 0xFFFFFFFFu;
		static Uint32 s_lastY = 0xFFFFFFFFu;
		static Uint32 s_lastZ = 0xFFFFFFFFu;
		if(s_lastX != self.x || s_lastY != self.y || s_lastZ != self.z)
		{
			s_lastX = self.x;
			s_lastY = self.y;
			s_lastZ = self.z;
			log("elfbot current position cache patched: base=%p pos=%u,%u,%u",
				elfbot, self.x, self.y, self.z);
		}
	}

	static void resetElfbotWriteWatch()
	{
#ifdef _WIN32
		s_watchRanges.clear();
		s_watchSnapshotsReady = false;
#endif
	}

	static void initElfbotWriteWatch()
	{
#ifdef _WIN32
		if(s_watchSnapshotsReady)
			return;

		struct WatchSpec { Uint32 base; Uint32 size; const char* name; };
		static const WatchSpec specs[] = {
			{ 0x0063DA00, 0x00000500, "target-vip" },
			{ 0x0063FE00, 0x0000B000, "player-battlelist" },
			{ 0x0064CD00, 0x00002000, "containers" },
			{ 0x0064F500, 0x00000200, "legacy-position" },
			{ 0x00795000, 0x00000200, "speak-alt" },
			{ 0x00799C00, 0x00000400, "hotkeys-modes" },
			{ 0x0079CE00, 0x00000300, "client-status" },
			{ 0x007E0700, 0x00001500, "messages-compose" },
		};

		s_watchRanges.clear();
		for(size_t i = 0; i < sizeof(specs) / sizeof(specs[0]); ++i)
		{
			const void* ptr = reinterpret_cast<const void*>(
				static_cast<uintptr_t>(specs[i].base));
			if(!isCommittedRange(specs[i].base, specs[i].size))
			{
				log("write-watch skipped %s 0x%08X size=0x%X (not readable)",
					specs[i].name, specs[i].base, specs[i].size);
				continue;
			}

			WatchRange r;
			r.base = specs[i].base;
			r.size = specs[i].size;
			r.name = specs[i].name;
			r.bytes.resize(specs[i].size);
			std::memcpy(&r.bytes[0], ptr, specs[i].size);
			s_watchRanges.push_back(r);
		}

		s_watchSnapshotsReady = true;
		log("write-watch armed: %u ranges", static_cast<unsigned>(s_watchRanges.size()));
#endif
	}

	static void refreshElfbotWriteWatch()
	{
#ifdef _WIN32
		initElfbotWriteWatch();
		for(size_t i = 0; i < s_watchRanges.size(); ++i)
		{
			WatchRange& r = s_watchRanges[i];
			const void* ptr = reinterpret_cast<const void*>(
				static_cast<uintptr_t>(r.base));
			if(r.bytes.empty() || !isCommittedRange(r.base, r.size))
			{
				resetElfbotWriteWatch();
				return;
			}
			std::memcpy(&r.bytes[0], ptr, r.size);
		}
#endif
	}

	static void logElfbotOldClientWrites()
	{
#ifdef _WIN32
		if(!s_watchSnapshotsReady)
			return;

		HMODULE elfbot = GetModuleHandleA("elfbot.dll");
		if(!elfbot)
			return;

		for(size_t i = 0; i < s_watchRanges.size(); ++i)
		{
			WatchRange& r = s_watchRanges[i];
			const unsigned char* cur = reinterpret_cast<const unsigned char*>(
				static_cast<uintptr_t>(r.base));
			if(r.bytes.empty() || !isCommittedRange(r.base, r.size))
			{
				resetElfbotWriteWatch();
				return;
			}

			unsigned int changedBytes = 0;
			unsigned int loggedRows = 0;
			for(Uint32 off = 0; off < r.size; ++off)
			{
				if(cur[off] == r.bytes[off])
					continue;

				if(changedBytes == 0)
					log("old-client writes detected in %s", r.name);

				if(loggedRows < 48)
				{
					Uint32 aligned = off & ~3u;
					Uint32 oldDword = 0;
					Uint32 newDword = 0;
					if(aligned + sizeof(Uint32) <= r.size)
					{
						std::memcpy(&oldDword, &r.bytes[aligned], sizeof(Uint32));
						std::memcpy(&newDword, cur + aligned, sizeof(Uint32));
					}

					log("  addr=0x%08X old=%02X new=%02X dword@0x%08X: 0x%08X -> 0x%08X",
						r.base + off,
						static_cast<unsigned>(r.bytes[off]),
						static_cast<unsigned>(cur[off]),
						r.base + aligned,
						oldDword,
						newDword);
					++loggedRows;
				}

				++changedBytes;
				off |= 3u;
			}

			if(changedBytes != 0)
			{
				log("  total changed dword groups=%u", changedBytes);
				std::memcpy(&r.bytes[0], cur, r.size);
			}
		}
#endif
	}

	static void applyShimInProcess(const TibiaShimBlock* shim)
	{
		poke<Uint32>(Addr::CLIENT_STATUS, shim->status);
		poke<Uint32>(Addr::CLIENT_ACTION_STATE, 0);
		poke<Uint32>(Addr::CLIENT_MULTI_CLIENT, 1);
		poke<Uint32>(Addr::CLIENT_SAFE_MODE, g_engine.getSecureMode());
		poke<Uint32>(Addr::CLIENT_FOLLOW_MODE, g_engine.getChaseMode());
		poke<Uint32>(Addr::CLIENT_ATTACK_MODE, g_engine.getAttackMode());

		poke<Uint32>(Addr::PLAYER_EXPERIENCE,
			static_cast<Uint32>(shim->player.experience > 0xFFFFFFFFULL
				? 0xFFFFFFFFULL : shim->player.experience));
		poke<Uint32>(Addr::PLAYER_FLAGS, g_game.getIcons());
		poke<Uint32>(Addr::PLAYER_ID, shim->player.id);
		poke<Uint32>(Addr::PLAYER_HEALTH, shim->player.health);
		poke<Uint32>(Addr::PLAYER_HEALTH_MAX, shim->player.healthMax);
		poke<Uint32>(Addr::PLAYER_LEVEL, shim->player.level);
		poke<Uint32>(Addr::PLAYER_MAGIC_LEVEL, shim->player.magicLevel);
		poke<Uint32>(Addr::PLAYER_LEVEL_PERCENT, shim->player.levelPercent);
		poke<Uint32>(Addr::PLAYER_MAGIC_PERCENT, shim->player.magicLevelPercent);
		poke<Uint32>(Addr::PLAYER_MANA, shim->player.mana);
		poke<Uint32>(Addr::PLAYER_MANA_MAX, shim->player.manaMax);
		poke<Uint32>(Addr::PLAYER_SOUL, shim->player.soul);
		poke<Uint32>(Addr::PLAYER_STAMINA, shim->player.stamina);
		poke<Uint32>(Addr::PLAYER_CAPACITY, shim->player.capacity);

		for(int i = 0; i < 7; ++i)
		{
			poke<Uint32>(Addr::PLAYER_SKILLS_BASE + i * 4, shim->player.skillPercent[i]);
			poke<Uint32>(Addr::PLAYER_SKILLS_BASE + 28 + i * 4, shim->player.skillLevel[i]);
		}

		poke<Uint32>(Addr::PLAYER_RED_SQUARE, shim->player.targetId);
		poke<Uint32>(Addr::PLAYER_GREEN_SQUARE, shim->player.followId);
		poke<Uint32>(Addr::PLAYER_TARGET_BATTLELIST_ID, shim->player.targetId);
		poke<Uint8 >(Addr::PLAYER_TARGET_BATTLELIST_TYPE,
			static_cast<Uint8>((shim->player.targetId >> 24) & 0xFF));
		poke<Uint8 >(Addr::PLAYER_TARGET_TYPE,
			static_cast<Uint8>((shim->player.targetId >> 24) & 0xFF));
		poke<Uint32>(Addr::PLAYER_Z, shim->player.z);
		const TibiaShimCreature& selfPos = shim->creatures[0];
		poke<Uint32>(Addr::PLAYER_X, selfPos.x);
		poke<Uint32>(Addr::PLAYER_Y, selfPos.y);
		poke<Uint32>(Addr::PLAYER_BATTLE_Z, selfPos.z);

		// Keep only confirmed original-client read-only position mirrors.
		// Do not write ElfBot's dynamic 0x2Exxxxxx cavebot list memory here;
		// those addresses change per waypoint/list and are owned by ElfBot.
		if(selfPos.x > 0 && selfPos.x < 0xFFFFu &&
		   selfPos.y > 0 && selfPos.y < 0xFFFFu &&
		   selfPos.z <= 15)
		{
			poke<Uint32>(LEGACY_POS_X, selfPos.x);
			poke<Uint32>(LEGACY_POS_Y, selfPos.y);
			poke<Uint32>(LEGACY_POS_Z, selfPos.z);
			poke<Uint32>(CLIENT_POS_X1, selfPos.x);
			poke<Uint32>(CLIENT_POS_Y1, selfPos.y);
			poke<Uint32>(CLIENT_POS_X2, selfPos.x);
			poke<Uint32>(CLIENT_POS_Y2, selfPos.y);
			poke<Uint32>(CLIENT_POS_X3, selfPos.x);
			poke<Uint32>(CLIENT_POS_Y3, selfPos.y);

			// Do not mirror PLAYER_GOTO_X/Y/Z here. Those addresses are
			// action request slots in the old client. If we keep writing
			// the current tile into them, Cavebot can see its walk target
			// as already reached and never emit a real movement request.
			// Current position is mirrored through BattleList[0], legacy
			// 0x64F60x, client 0x79CFxx and ElfBot's own cache instead.
		}

		for(int i = 0; i < 10; ++i)
		{
			Uint32 base = Addr::PLAYER_SLOT_HEAD + i * 12;
			poke<Uint16>(base + 0, shim->player.inventory[i].id);
			poke<Uint16>(base + 2, 0);
			poke<Uint32>(base + 4, shim->player.inventory[i].count);
			poke<Uint32>(base + 8, 0);
		}

		for(Uint32 i = 0; i < Addr::BATTLELIST_MAX; ++i)
		{
			Uint32 base = Addr::BATTLELIST_START + i * Addr::BATTLELIST_STEP;
			if(i >= shim->creatureCount)
			{
				// Clear the WHOLE 168-byte slot, not just the id. Otherwise
				// stale name/position from previous frames sticks around --
				// you can find it in Cheat Engine and ElfBot may see it as
				// a "ghost" creature at that position.
				std::memset(reinterpret_cast<void*>(static_cast<uintptr_t>(base)),
				            0, Addr::BATTLELIST_STEP);
				continue;
			}
			const TibiaShimCreature& c = shim->creatures[i];
			poke<Uint32>(base + Addr::C_OFF_ID, c.id);
			poke<Uint8 >(base + Addr::C_OFF_TYPE, c.type);
			pokeStringRaw(base + Addr::C_OFF_NAME, c.name, 32);
			poke<Uint32>(base + Addr::C_OFF_X, c.x);
			poke<Uint32>(base + Addr::C_OFF_Y, c.y);
			poke<Uint32>(base + Addr::C_OFF_Z, c.z);
			poke<Uint32>(base + Addr::C_OFF_SCR_OFFSET_H, c.screenOffH);
			poke<Uint32>(base + Addr::C_OFF_SCR_OFFSET_V, c.screenOffV);
			poke<Uint32>(base + Addr::C_OFF_IS_WALKING, c.isWalking);
			poke<Uint32>(base + Addr::C_OFF_DIRECTION, c.direction);
			poke<Uint32>(base + Addr::C_OFF_OUTFIT, c.outfit);
			poke<Uint32>(base + Addr::C_OFF_LIGHT, c.light);
			poke<Uint32>(base + Addr::C_OFF_BLACK_SQUARE, c.blackSquare);
			poke<Uint8 >(base + Addr::C_OFF_HP_BAR, c.hpBar);
			poke<Uint32>(base + Addr::C_OFF_WALK_SPEED, c.walkSpeed);
			poke<Uint32>(base + Addr::C_OFF_IS_VISIBLE, c.isVisible);
			poke<Uint8 >(base + Addr::C_OFF_SKULL, c.skull);
			poke<Uint32>(base + Addr::C_OFF_PARTY, c.party);
			poke<Uint32>(base + Addr::C_OFF_WAR_ICON, c.warIcon);
			poke<Uint32>(base + Addr::C_OFF_IS_BLOCKING, c.isBlocking);
		}

		// These addresses overlap the original 8.60 battlelist area.
		// Write them after the battlelist cleanup, otherwise empty slot
		// zeroing immediately erases them and Cavebot still sees 0,0,0.
		if(selfPos.x > 0 && selfPos.x < 0xFFFFu &&
		   selfPos.y > 0 && selfPos.y < 0xFFFFu &&
		   selfPos.z <= 15)
		{
			poke<Uint32>(LEGACY_POS2_X, selfPos.x);
			poke<Uint32>(LEGACY_POS2_Y, selfPos.y);
			poke<Uint32>(LEGACY_POS2_Z, selfPos.z);
		}

		for(int i = 0; i < 16; ++i)
		{
			Uint32 base = Addr::CONTAINER_START + i * Addr::CONTAINER_STEP;
			const TibiaShimContainer& c = shim->containers[i];
			poke<Uint32>(base + Addr::CONTAINER_OFF_ISOPEN, c.isOpen);
			poke<Uint32>(base + Addr::CONTAINER_OFF_ID, c.id);
			pokeStringRaw(base + Addr::CONTAINER_OFF_NAME, c.name, 32);
			poke<Uint32>(base + Addr::CONTAINER_OFF_VOLUME, c.volume);
			poke<Uint32>(base + Addr::CONTAINER_OFF_AMOUNT, c.amount);
			poke<Uint16>(base + Addr::CONTAINER_OFF_ITEMID, c.firstItemId);
			poke<Uint32>(base + Addr::CONTAINER_OFF_ITEMCNT, c.firstItemCount);
		}

		for(Uint32 i = 0; i < Addr::VIP_MAX; ++i)
		{
			Uint32 base = Addr::VIP_START + i * Addr::VIP_STEP;
			if(i >= shim->vipCount)
			{
				poke<Uint32>(base + Addr::VIP_OFF_ID, 0);
				continue;
			}
			const TibiaShimVip& v = shim->vips[i];
			poke<Uint32>(base + Addr::VIP_OFF_ID, v.id);
			pokeStringRaw(base + Addr::VIP_OFF_NAME, v.name, 30);
			poke<Uint16>(base + Addr::VIP_OFF_STATUS, v.status);
			poke<Uint32>(base + Addr::VIP_OFF_ICON, v.icon);
		}

		// CHAT/MESSAGE BUFFER POLICY -- READ THIS BEFORE CHANGING ANY LINE BELOW
		//
		// In Tibia 8.60 the text buffers split into two groups:
		//   INBOUND  (server -> client, "stuff the player just received")
		//     0x7E0A00  CLIENT_LASTMSG_TEXT    - last chat line received
		//     0x7E09D8  CLIENT_LASTMSG_AUTHOR  - speaker name of the line above
		//     0x7E07B0  CLIENT_STATUSBAR_TEXT  - yellow status-bar text
		//                                       (cancel msgs, boosted-creature
		//                                        notices, "Sorry not possible")
		//     0x7E0DA8  CLIENT_LOOK_TEXT       - look-action result text
		//   OUTBOUND (client -> server, "what the player is about to say")
		//     0x7E09D8  CLIENT_SPEAK_TEXT_A    - chat box compose buffer
		//     0x795098  CLIENT_SPEAK_TEXT_B    - mirror used by send-speak
		//     0x7E0A28  CLIENT_HOTKEY_TEXT_A   - hotkey chat compose
		//     0x7E0BC0  CLIENT_HOTKEY_TEXT_B   - hotkey chat compose
		//     0x7E1218  CLIENT_HOTKEY_TEXT_C   - hotkey chat compose
		//
		// Previously this code wrote shim->lastMessageText (inbound server
		// text) into ALL of the OUTBOUND buffers too. That caused a serious
		// bug: when the server sent a cancel message like "Today's boosted
		// monsters are necromancer and hellhound", we pumped that string
		// into CLIENT_SPEAK_TEXT_A. ElfBot (or any code polling that buffer
		// for "what is the player typing") then relayed it through the send
		// path, so the *character* ended up announcing the server's status
		// message in chat. The server saw it in its log as a normal speak
		// packet originating from the player, because that's exactly what
		// happened over the wire.
		//
		// Rule: only write INBOUND text into INBOUND buffers.
		//       Never touch the OUTBOUND speak/hotkey compose buffers from
		//       sync() -- they are owned by the player (and by ElfBot when
		//       it dispatches a hotkey). Writing to them turns every server
		//       chat line into auto-speech.
		// In real Tibia 8.60 the adjacent string buffers are tightly
		// packed -- only 40 bytes between LASTMSG_AUTHOR / LASTMSG_TEXT /
		// HOTKEY_TEXT_A. Writing 256 bytes per buffer (as we did before)
		// memsets and overwrites the neighbouring buffers as a side
		// effect, which is what caused server cancel-message text to
		// leak into HOTKEY_TEXT_A. Cap each write to 40 bytes so the
		// poke stays inside its own slot.
		pokeStringRaw(Addr::CLIENT_LASTMSG_TEXT,    shim->lastMessageText,    40);
		pokeStringRaw(Addr::CLIENT_LASTMSG_AUTHOR,  shim->lastMessageAuthor,  40);
		pokeStringRaw(Addr::CLIENT_STATUSBAR_TEXT,  shim->lastMessageText,    40);
		pokeStringRaw(Addr::CLIENT_LOOK_TEXT,       s_lastLookText,          200);
		// DO NOT WRITE:
		//   CLIENT_SPEAK_TEXT_A / CLIENT_SPEAK_TEXT_B
		//   CLIENT_HOTKEY_TEXT_A / _B / _C
		// (outbound compose buffers -- see comment above)

		// FPS / frame timing -- the user explicitly wants ElfBot's HUD to
		// see the *cipsoft client's* FPS variable populated, not our own
		// copy. The address layout was reverse-engineered from Tibia.exe
		// (see CLIENT_FPS_VALUE comment in elfbot_compat.h). We mirror
		// TFC's g_lastFrames (Uint16 FPS) and g_frameDiff (Uint32 ms per
		// frame) into the original Tibia globals.
		{
			// g_lastFrames / g_frameDiff are defined at GLOBAL scope in
			// main.cpp. They are forward-declared at file scope just
			// before namespace ElfbotCompat { ... } below, so they
			// reference the global symbols, not ElfbotCompat::...
			float fpsAsFloat = static_cast<float>(::g_lastFrames);
			poke<float>(Addr::CLIENT_FPS_VALUE,     fpsAsFloat);
			poke<float>(Addr::CLIENT_FPS_VALUE_ALT, fpsAsFloat);
			poke<Uint32>(Addr::CLIENT_FRAME_TIME_MS, ::g_frameDiff);
		}

		// Status-bar DOUBLE copies (Tibia 8.60). The original client
		// keeps the displayed "Cap:" and "Soul:" values as 8-byte
		// doubles in a separate region 0x0064ADxx so the UI formatter
		// can call printf(%.02f) on them. Memory-scanning bots
		// (including ElfBot variants and some bots like NeoBot) read
		// these doubles, not the integer source in the player struct,
		// so we mirror them here for completeness. Cheap (16 bytes
		// per frame) and read-only otherwise.
		{
			poke<double>(Addr::CLIENT_DISP_CAPACITY,
			             static_cast<double>(shim->player.capacity));
			poke<double>(Addr::CLIENT_DISP_SOUL,
			             static_cast<double>(shim->player.soul));
		}

		// ElfBot "game ready" gate. ElfBot's title-formatter at
		// elfbot.dll +0x5DEC does:
		//      cmp dword [0x0064A9C0], 0x1F   ; literal 31, not >=
		//      je  skip_startup_append        ; jump if EXACTLY equal
		//      push "Startup"
		// Earlier I wrote 0x80 here, which kept the JE from firing
		// (cmp 0x80, 0x1F is NOT equal), so ElfBot still appended
		// "Startup". The correct value is exactly 31. We pin it to 31
		// every frame so even if some other path increments it later
		// our write wins on the next sync tick.
		poke<Uint32>(Addr::CLIENT_GAMETICK_COUNTER, 0x1F);
		if(s_hasXteaKey)
		{
			for(int i = 0; i < 4; ++i)
				poke<Uint32>(Addr::CLIENT_XTEAKEY + i * 4, s_xteaKey[i]);
		}

		// Also write to the alternate 0x636XXX layout so the ElfBot
		// build that reads from THAT table sees our data too. Cheap,
		// inside our shadow, no conflict with the TibiaAPI layout
		// because the two address ranges don't overlap.
		applyShimAlternateLayout(shim);
		patchElfbotCachedPlayerName(shim);
		patchElfbotCurrentPositionCache(shim);
		probeElfbotInternalState(shim, false);

		s_mirroredAttackId = shim->player.targetId;
		s_mirroredFollowId = shim->player.followId;
		s_hasMirroredTargets = true;
		refreshElfbotWriteWatch();
	}

	static bool isPlausibleCreatureId(Uint32 id)
	{
		return id != 0 && id != 0xFFFFFFFFu && id != 0xCCCCCCCCu && id < 0xF0000000u;
	}

	static bool isPlausiblePosition(Uint32 x, Uint32 y, Uint32 z)
	{
		return x > 0 && x < 0xFFFFu && y > 0 && y < 0xFFFFu && z <= 15;
	}

	static void processElfbotSayWriteback()
	{
#ifdef _WIN32
		struct TextSlot { Uint32 addr; size_t readLen; size_t clearLen; const char* name; bool acceptPlainText; };
		static const TextSlot slots[] = {
			// Only OUTBOUND compose/hotkey buffers. Do not read LASTMSG_TEXT
			// or CLIENT_SPEAK_TEXT_A: those overlap inbound author/message
			// storage in the 8.60 layout and caused server text to be echoed.
			{ Addr::CLIENT_HOTKEY_TEXT_C, 128, 128, "hotkey_c", true  },
			{ Addr::CLIENT_HOTKEY_TEXT_B, 128, 128, "hotkey_b", true  },
			{ Addr::CLIENT_HOTKEY_TEXT_A,  40,  40, "hotkey_a", true  },
			{ Addr::CLIENT_SPEAK_TEXT_B,  128, 128, "speak_b",  false },
		};

		static std::string s_lastCommandRaw;
		static DWORD s_lastCommandTick = 0;

		DWORD now = GetTickCount();
		for(size_t i = 0; i < sizeof(slots) / sizeof(slots[0]); ++i)
		{
			std::string raw = readStringRaw(slots[i].addr, slots[i].readLen);
			raw = trimAscii(raw);
			if(raw.empty())
				continue;

			if(raw == s_lastMessageText || raw == s_lastLookText || raw == s_lastMessageAuthor)
				continue;

			std::string lower = lowerAscii(raw);
			bool explicitCommand = (lower.compare(0, 5, "auto ") == 0 || lower.compare(0, 4, "say ") == 0);
			if(!explicitCommand && !slots[i].acceptPlainText)
				continue;

			std::string message;
			Uint32 intervalMs = 0;
			bool isAuto = false;
			if(!parseElfbotSayCommand(raw, message, intervalMs, isAuto))
				continue;

			bool shouldSend = false;
			if(raw != s_lastCommandRaw)
				shouldSend = true;
			else if(isAuto && now - s_lastCommandTick >= intervalMs)
				shouldSend = true;

			if(!shouldSend)
				continue;

			s_lastCommandRaw = raw;
			s_lastCommandTick = now;
			log("writeback: say from %s 0x%08X raw='%s' message='%s'",
				slots[i].name, slots[i].addr, raw.c_str(), message.c_str());
			g_game.sendSay(MessageSay, 0, std::string(), message);

			// Clear the old-client compose buffer after consuming it. If
			// ElfBot's auto loop really fires every N ms, it will write the
			// same text again and we will forward that next write. If it was
			// stale memory, it will not repeat forever.
			std::memset(reinterpret_cast<void*>(static_cast<uintptr_t>(slots[i].addr)),
			            0, slots[i].clearLen);
			break;
		}
#endif
	}

	static size_t getModuleImageSize(HMODULE module)
	{
#ifdef _WIN32
		if(!module)
			return 0;

		const IMAGE_DOS_HEADER* dos = reinterpret_cast<const IMAGE_DOS_HEADER*>(module);
		if(!isReadablePointer(dos, sizeof(*dos)) || dos->e_magic != IMAGE_DOS_SIGNATURE)
			return 0;

		const IMAGE_NT_HEADERS* nt = reinterpret_cast<const IMAGE_NT_HEADERS*>(
			reinterpret_cast<uintptr_t>(module) + dos->e_lfanew);
		if(!isReadablePointer(nt, sizeof(*nt)) || nt->Signature != IMAGE_NT_SIGNATURE)
			return 0;

		return nt->OptionalHeader.SizeOfImage;
#else
		(void)module;
		return 0;
#endif
	}

	static std::string readAsciiFromMemory(const char* ptr, const char* limit, size_t maxLen)
	{
		std::string out;
		while(ptr < limit && out.size() < maxLen)
		{
			char ch = *ptr++;
			if(ch == '\0')
				break;

			unsigned char uch = static_cast<unsigned char>(ch);
			if(uch < 32 || uch > 126)
				break;

			out.push_back(ch);
		}
		return out;
	}

	static void processElfbotAutoScripts()
	{
#ifdef _WIN32
		HMODULE elfbot = GetModuleHandleA("elfbot.dll");
		if(!elfbot)
			return;

		struct AutoScriptState
		{
			std::string raw;
			DWORD lastRun;
			DWORD lastSeen;
		};
		static std::vector<AutoScriptState> s_scripts;
		static DWORD s_lastScan = 0;

		DWORD now = GetTickCount();
		if(now - s_lastScan >= 1000)
		{
			s_lastScan = now;
			uintptr_t base = reinterpret_cast<uintptr_t>(elfbot);
			size_t imageSize = getModuleImageSize(elfbot);
			uintptr_t end = base + imageSize;
			uintptr_t cursor = base;

			while(imageSize != 0 && cursor < end)
			{
				MEMORY_BASIC_INFORMATION mbi;
				if(VirtualQuery(reinterpret_cast<const void*>(cursor), &mbi, sizeof(mbi)) == 0)
					break;

				uintptr_t pageStart = reinterpret_cast<uintptr_t>(mbi.BaseAddress);
				uintptr_t pageEnd = pageStart + mbi.RegionSize;
				if(pageEnd <= cursor)
					break;

				uintptr_t scanStart = cursor > pageStart ? cursor : pageStart;
				uintptr_t scanEnd = pageEnd < end ? pageEnd : end;
				bool readable = (mbi.State == MEM_COMMIT && !(mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)));
				if(readable && scanEnd > scanStart + 5)
				{
					const char* p = reinterpret_cast<const char*>(scanStart);
					const char* limit = reinterpret_cast<const char*>(scanEnd);
					for(; p + 5 < limit; ++p)
					{
						if(std::memcmp(p, "auto ", 5) != 0)
							continue;

						std::string raw = readAsciiFromMemory(p, limit, 180);
						std::string message;
						Uint32 intervalMs = 0;
						bool isAuto = false;
						if(!parseElfbotSayCommand(raw, message, intervalMs, isAuto) || !isAuto)
							continue;

						bool found = false;
						for(size_t i = 0; i < s_scripts.size(); ++i)
						{
							if(s_scripts[i].raw == raw)
							{
								s_scripts[i].lastSeen = now;
								found = true;
								break;
							}
						}
						if(!found)
						{
							AutoScriptState state;
							state.raw = raw;
							state.lastRun = 0;
							state.lastSeen = now;
							s_scripts.push_back(state);
							log("elfbot auto script discovered: raw='%s'", raw.c_str());
							g_game.processTextMessage(MessageFailure,
								std::string("Auto ON: ") + raw);
						}
					}
				}

				cursor = pageEnd;
			}

			for(size_t i = 0; i < s_scripts.size();)
			{
				if(now - s_scripts[i].lastSeen > 5000)
				{
					log("elfbot auto script expired: raw='%s'", s_scripts[i].raw.c_str());
					g_game.processTextMessage(MessageFailure,
						std::string("Auto OFF: ") + s_scripts[i].raw);
					s_scripts.erase(s_scripts.begin() + i);
				}
				else
				{
					++i;
				}
			}
		}

		for(size_t i = 0; i < s_scripts.size(); ++i)
		{
			std::string message;
			Uint32 intervalMs = 0;
			bool isAuto = false;
			if(!parseElfbotSayCommand(s_scripts[i].raw, message, intervalMs, isAuto) || !isAuto)
				continue;

			if(s_scripts[i].lastRun != 0 && now - s_scripts[i].lastRun < intervalMs)
				continue;

			s_scripts[i].lastRun = now;
			log("writeback: elfbot auto script raw='%s' message='%s'",
				s_scripts[i].raw.c_str(), message.c_str());
			g_game.sendSay(MessageSay, 0, std::string(), message);
		}
#endif
	}

	static Uint32 deltaAbs(Uint32 a, Uint32 b)
	{
		return (a > b) ? (a - b) : (b - a);
	}

	static void processOldTibiaHookActions()
	{
#ifdef _WIN32
		Creature* self = g_map.getLocalCreature();
		if(!self)
			return;

		if(InterlockedExchange(&s_pendingPacketAutoWalk, 0) != 0)
		{
			std::vector<Direction> path;
			Uint8 count = s_pendingPacketAutoWalkCount;
			if(count > 127)
				count = 127;
			for(Uint8 i = 0; i < count; ++i)
			{
				if(s_pendingPacketAutoWalkDirs[i] != DIRECTION_INVALID)
					path.push_back(s_pendingPacketAutoWalkDirs[i]);
			}
			if(!path.empty())
			{
				log("writeback: old encrypted packet autowalk directions=%u",
					static_cast<unsigned int>(path.size()));
				g_game.sendAutoWalk(path);
			}
		}

		if(InterlockedExchange(&s_pendingPacketWalk, 0) != 0)
		{
			Direction d = s_pendingPacketWalkDir;
			if(d != DIRECTION_INVALID)
			{
				log("writeback: old encrypted packet single walk dir=%d", static_cast<int>(d));
				g_game.sendWalk(d);
			}
		}

		if(InterlockedExchange(&s_pendingPacketAttack, 0) != 0)
		{
			Uint32 id = s_pendingPacketAttackId;
			Creature* c = g_map.getCreatureById(id);
			if(c)
			{
				log("writeback: old encrypted packet attack id=%u name=%s",
					id, c->getName().c_str());
				g_game.sendAttack(c);
			}
			else
				log("writeback: old encrypted packet attack id=%u not in TFC creature map", id);
		}

		if(InterlockedExchange(&s_pendingPacketFollow, 0) != 0)
		{
			Uint32 id = s_pendingPacketFollowId;
			Creature* c = g_map.getCreatureById(id);
			if(c)
			{
				log("writeback: old encrypted packet follow id=%u name=%s",
					id, c->getName().c_str());
				g_game.sendFollow(c);
			}
			else
				log("writeback: old encrypted packet follow id=%u not in TFC creature map", id);
		}

		if(InterlockedExchange(&s_pendingOldWalk, 0) != 0)
		{
			Position current = self->getCurrentPosition();
			Uint32 gx = s_pendingOldWalkX;
			Uint32 gy = s_pendingOldWalkY;
			Uint32 gz = s_pendingOldWalkZ;
			if(gz == 0 && current.z != 0)
				gz = current.z;

			if(isPlausiblePosition(gx, gy, gz))
			{
				Position target(static_cast<Uint16>(gx), static_cast<Uint16>(gy), static_cast<Uint8>(gz));
				if(target != current &&
				   target.z == current.z &&
				   deltaAbs(target.x, current.x) <= 50 &&
				   deltaAbs(target.y, current.y) <= 50)
				{
					static Position s_lastHookWalk(0xFFFF, 0xFFFF, 0xFF);
					static DWORD s_lastHookWalkTick = 0;
					DWORD now = GetTickCount();
					if(target != s_lastHookWalk || now - s_lastHookWalkTick > 1000)
					{
						s_lastHookWalk = target;
						s_lastHookWalkTick = now;
						log("writeback: old-tibia autowalk target=%u,%u,%u current=%u,%u,%u",
							target.x, target.y, target.z, current.x, current.y, current.z);
						g_game.startAutoWalk(target);
					}
				}
				else
				{
					log("writeback: ignored old-tibia autowalk target=%u,%u,%u current=%u,%u,%u",
						gx, gy, gz, current.x, current.y, current.z);
				}
			}
			else
			{
				log("writeback: ignored invalid old-tibia autowalk target=%u,%u,%u", gx, gy, gz);
			}

			poke<Uint32>(Addr::PLAYER_GOTO_X, 0);
			poke<Uint32>(Addr::PLAYER_GOTO_Y, 0);
			poke<Uint32>(Addr::PLAYER_GOTO_Z, 0);
		}

		if(InterlockedExchange(&s_pendingOldAttack, 0) != 0)
		{
			Uint32 id = s_pendingOldAttackId;
			if(id == 0)
			{
				log("writeback: old-tibia cancel attack/follow");
				g_game.sendCancelAttackAndFollow();
			}
			else if(id != g_game.getAttackID() && isPlausibleCreatureId(id))
			{
				Creature* c = g_map.getCreatureById(id);
				if(c)
				{
					log("writeback: old-tibia attack id=%u name=%s", id, c->getName().c_str());
					g_game.sendAttack(c);
				}
				else
				{
					log("writeback: old-tibia attack id=%u not in TFC creature map", id);
				}
			}
		}

		if(InterlockedExchange(&s_pendingOldFollow, 0) != 0)
		{
			Uint32 id = s_pendingOldFollowId;
			if(id == 0)
			{
				log("writeback: old-tibia cancel follow");
				g_game.sendCancelAttackAndFollow();
			}
			else if(id != g_game.getFollowID() && isPlausibleCreatureId(id))
			{
				Creature* c = g_map.getCreatureById(id);
				if(c)
				{
					log("writeback: old-tibia follow id=%u name=%s", id, c->getName().c_str());
					g_game.sendFollow(c);
				}
				else
				{
					log("writeback: old-tibia follow id=%u not in TFC creature map", id);
				}
			}
		}
#endif
	}

	static void processElfbotAutowalkWriteback()
	{
#ifdef _WIN32
		Creature* self = g_map.getLocalCreature();
		if(!self)
			return;

		Position current = self->getCurrentPosition();
		Uint32 gx = peek<Uint32>(Addr::PLAYER_GOTO_X);
		Uint32 gy = peek<Uint32>(Addr::PLAYER_GOTO_Y);
		Uint32 gz = peek<Uint32>(Addr::PLAYER_GOTO_Z);

		if(s_hasMirroredGoto &&
		   gx == s_mirroredGotoX &&
		   gy == s_mirroredGotoY &&
		   gz == s_mirroredGotoZ)
			return;

		if(gx == 0 && gy == 0 && gz == 0)
			return;

		// Several ElfBot cavebot paths leave Z as zero even when X/Y are
		// valid. Treat zero Z as "same floor" unless the player is really
		// on floor 0.
		if(gz == 0 && current.z != 0)
			gz = current.z;

		if(!isPlausiblePosition(gx, gy, gz))
			return;

		Position target(static_cast<Uint16>(gx), static_cast<Uint16>(gy), static_cast<Uint8>(gz));
		if(target == current)
		{
			poke<Uint32>(Addr::PLAYER_GOTO_X, 0);
			poke<Uint32>(Addr::PLAYER_GOTO_Y, 0);
			poke<Uint32>(Addr::PLAYER_GOTO_Z, 0);
			return;
		}

		// Keep this strictly local. A bad stale pointer should not send the
		// player running across the map or flipping floors.
		if(target.z != current.z || deltaAbs(target.x, current.x) > 50 || deltaAbs(target.y, current.y) > 50)
		{
			log("writeback: ignored autowalk target=%u,%u,%u current=%u,%u,%u",
				target.x, target.y, target.z, current.x, current.y, current.z);
			poke<Uint32>(Addr::PLAYER_GOTO_X, 0);
			poke<Uint32>(Addr::PLAYER_GOTO_Y, 0);
			poke<Uint32>(Addr::PLAYER_GOTO_Z, 0);
			return;
		}

		static Position s_lastGotoRequest(0xFFFF, 0xFFFF, 0xFF);
		static DWORD s_lastGotoTick = 0;
		DWORD now = GetTickCount();
		if(target != s_lastGotoRequest || now - s_lastGotoTick > 1000)
		{
			s_lastGotoRequest = target;
			s_lastGotoTick = now;
			log("writeback: autowalk target=%u,%u,%u current=%u,%u,%u",
				target.x, target.y, target.z, current.x, current.y, current.z);
			g_game.startAutoWalk(target);
		}

		poke<Uint32>(Addr::PLAYER_GOTO_X, 0);
		poke<Uint32>(Addr::PLAYER_GOTO_Y, 0);
		poke<Uint32>(Addr::PLAYER_GOTO_Z, 0);
#endif
	}

	static void processElfbotWriteback()
	{
		if(!g_engine.isIngame())
			return;

		Creature* self = g_map.getLocalCreature();
		if(!self)
			return;

		if(!s_hasMirroredTargets)
			return;

		processOldTibiaHookActions();
		logElfbotOldClientWrites();
		processElfbotSayWriteback();
		processElfbotAutowalkWriteback();

		// Read attack/follow from BOTH address layouts. Only treat a
		// value as an ElfBot request when it differs from the last value
		// we mirrored ourselves. This avoids interpreting a one-frame-old
		// mirror as a "cancel" when TFC changes attack/follow internally.
		Uint32 memAttackPrimary = peek<Uint32>(Addr::PLAYER_RED_SQUARE);
		Uint32 memAttackBattle = peek<Uint32>(Addr::PLAYER_TARGET_BATTLELIST_ID);
		Uint32 memAttackAlt = peek<Uint32>(ALT_TARGET_ID);
		Uint32 memFollowPrimary = peek<Uint32>(Addr::PLAYER_GREEN_SQUARE);
		Uint32 memFollowAlt = peek<Uint32>(ALT_FOLLOW_ID);

		bool attackWritten = false;
		bool followWritten = false;
		Uint32 memAttack = s_mirroredAttackId;
		Uint32 memFollow = s_mirroredFollowId;

		if(memAttackPrimary != s_mirroredAttackId)
		{
			memAttack = memAttackPrimary;
			attackWritten = true;
		}
		else if(memAttackBattle != s_mirroredAttackId)
		{
			memAttack = memAttackBattle;
			attackWritten = true;
		}
		else if(memAttackAlt != s_mirroredAttackId)
		{
			memAttack = memAttackAlt;
			attackWritten = true;
		}

		if(memFollowPrimary != s_mirroredFollowId)
		{
			memFollow = memFollowPrimary;
			followWritten = true;
		}
		else if(memFollowAlt != s_mirroredFollowId)
		{
			memFollow = memFollowAlt;
			followWritten = true;
		}

		static Uint32 s_lastAttackRequest = 0xFFFFFFFFu;
		static Uint32 s_lastFollowRequest = 0xFFFFFFFFu;
		static Position s_lastGotoRequest(0xFFFF, 0xFFFF, 0xFF);

		if(attackWritten && memAttack != g_game.getAttackID() && memAttack != s_lastAttackRequest)
		{
			s_lastAttackRequest = memAttack;
			if(memAttack == 0)
			{
				log("writeback: cancel attack/follow");
				g_game.sendCancelAttackAndFollow();
			}
			else if(isPlausibleCreatureId(memAttack))
			{
				Creature* c = g_map.getCreatureById(memAttack);
				if(c)
				{
					log("writeback: attack id=%u name=%s", memAttack, c->getName().c_str());
					g_game.sendAttack(c);
				}
				else
				{
					log("writeback: attack id=%u not in TFC creature map", memAttack);
				}
			}
		}

		if(followWritten && memFollow != g_game.getFollowID() && memFollow != s_lastFollowRequest)
		{
			s_lastFollowRequest = memFollow;
			if(memFollow == 0)
			{
				log("writeback: cancel follow");
				g_game.sendCancelAttackAndFollow();
			}
			else if(isPlausibleCreatureId(memFollow))
			{
				Creature* c = g_map.getCreatureById(memFollow);
				if(c)
				{
					log("writeback: follow id=%u name=%s", memFollow, c->getName().c_str());
					g_game.sendFollow(c);
				}
				else
				{
					log("writeback: follow id=%u not in TFC creature map", memFollow);
				}
			}
		}

		(void)s_lastGotoRequest;
	}

	static void logShimSnapshot(const TibiaShimBlock* shim)
	{
		static Uint32 s_lastStatus = 0xFFFFFFFFu;
		static Uint32 s_lastPlayerId = 0xFFFFFFFFu;
		static Uint32 s_lastCreatureCount = 0xFFFFFFFFu;
		static Uint32 s_lastPosX = 0xFFFFFFFFu;
		static Uint32 s_lastPosY = 0xFFFFFFFFu;
		static Uint32 s_lastPosZ = 0xFFFFFFFFu;

		if(shim->status == s_lastStatus &&
		   shim->player.id == s_lastPlayerId &&
		   shim->creatureCount == s_lastCreatureCount &&
		   shim->creatures[0].x == s_lastPosX &&
		   shim->creatures[0].y == s_lastPosY &&
		   shim->creatures[0].z == s_lastPosZ)
			return;

		s_lastStatus = shim->status;
		s_lastPlayerId = shim->player.id;
		s_lastCreatureCount = shim->creatureCount;
		s_lastPosX = shim->creatures[0].x;
		s_lastPosY = shim->creatures[0].y;
		s_lastPosZ = shim->creatures[0].z;

		log("shim data: status=%u memStatus=%u action=%u flags=0x%08X id=%u memId=%u level=%u hp=%u creatures=%u battle0=%u name=%s",
			shim->status,
			peek<Uint32>(Addr::CLIENT_STATUS),
			peek<Uint32>(Addr::CLIENT_ACTION_STATE),
			peek<Uint32>(Addr::PLAYER_FLAGS),
			shim->player.id,
			peek<Uint32>(Addr::PLAYER_ID),
			static_cast<unsigned>(shim->player.level),
			shim->player.health,
			shim->creatureCount,
			peek<Uint32>(Addr::BATTLELIST_START + Addr::C_OFF_ID),
			shim->creatures[0].name);

		// Per-slot dump of every populated BattleList entry. Lets us
		// see exactly what an external memory reader (Cheat Engine /
		// ElfBot HUD / the user's Python tibia_player_reader) should
		// be able to fetch via ReadProcessMemory.
		{
			Uint32 nSlots = shim->creatureCount;
			if(nSlots > 12) nSlots = 12;  // cap log noise; usually <8
			for(Uint32 s = 0; s < nSlots; ++s)
			{
				Uint32 base = Addr::BATTLELIST_START + s * Addr::BATTLELIST_STEP;
				const char* nm = reinterpret_cast<const char*>(
					static_cast<uintptr_t>(base + Addr::C_OFF_NAME));
				char nameSafe[33] = {0};
				for(int k = 0; k < 32 && nm[k]; ++k)
				{
					unsigned char ch = static_cast<unsigned char>(nm[k]);
					nameSafe[k] = (ch >= 32 && ch < 127) ? static_cast<char>(ch) : '?';
				}
				log("battle[%u] @ 0x%08X id=%u type=%u name='%s' "
				    "pos=%u,%u,%u outfit=%u colors=%u/%u/%u/%u addon=%u "
				    "hp%%=%u walking=%u dir=%u visible=%u",
					s, base,
					peek<Uint32>(base + Addr::C_OFF_ID),
					peek<Uint8 >(base + Addr::C_OFF_TYPE),
					nameSafe,
					peek<Uint32>(base + Addr::C_OFF_X),
					peek<Uint32>(base + Addr::C_OFF_Y),
					peek<Uint32>(base + Addr::C_OFF_Z),
					peek<Uint32>(base + Addr::C_OFF_OUTFIT),
					peek<Uint32>(base + Addr::C_OFF_COLOR_HEAD),
					peek<Uint32>(base + Addr::C_OFF_COLOR_BODY),
					peek<Uint32>(base + Addr::C_OFF_COLOR_LEGS),
					peek<Uint32>(base + Addr::C_OFF_COLOR_FEET),
					peek<Uint32>(base + Addr::C_OFF_ADDON),
					peek<Uint8 >(base + Addr::C_OFF_HP_BAR),
					peek<Uint32>(base + Addr::C_OFF_IS_WALKING),
					peek<Uint32>(base + Addr::C_OFF_DIRECTION),
					peek<Uint32>(base + Addr::C_OFF_IS_VISIBLE));
			}
		}
		log("elfbot gate: [0x%08X]=%u (target=31, will appear as Slot N when matched)",
			Addr::CLIENT_GAMETICK_COUNTER,
			peek<Uint32>(Addr::CLIENT_GAMETICK_COUNTER));
		log("shim target slots: red=0x%08X green=0x%08X targetBattle=0x%08X fishing=%u",
			peek<Uint32>(Addr::PLAYER_RED_SQUARE),
			peek<Uint32>(Addr::PLAYER_GREEN_SQUARE),
			peek<Uint32>(Addr::PLAYER_TARGET_BATTLELIST_ID),
			peek<Uint32>(Addr::PLAYER_SKILLS_BASE + 13 * 4));
		log("shim position: battle=%u,%u,%u legacy=%u,%u,%u pos2=%u,%u,%u client=%u,%u/%u,%u/%u,%u goto=%u,%u,%u alt=%u,%u,%u playerZ=%u",
			peek<Uint32>(Addr::PLAYER_X),
			peek<Uint32>(Addr::PLAYER_Y),
			peek<Uint32>(Addr::PLAYER_BATTLE_Z),
			peek<Uint32>(LEGACY_POS_X),
			peek<Uint32>(LEGACY_POS_Y),
			peek<Uint32>(LEGACY_POS_Z),
			peek<Uint32>(LEGACY_POS2_X),
			peek<Uint32>(LEGACY_POS2_Y),
			peek<Uint32>(LEGACY_POS2_Z),
			peek<Uint32>(CLIENT_POS_X1),
			peek<Uint32>(CLIENT_POS_Y1),
			peek<Uint32>(CLIENT_POS_X2),
			peek<Uint32>(CLIENT_POS_Y2),
			peek<Uint32>(CLIENT_POS_X3),
			peek<Uint32>(CLIENT_POS_Y3),
			peek<Uint32>(Addr::PLAYER_GOTO_X),
			peek<Uint32>(Addr::PLAYER_GOTO_Y),
			peek<Uint32>(Addr::PLAYER_GOTO_Z),
			peek<Uint32>(ALT_X),
			peek<Uint32>(ALT_Y),
			peek<Uint32>(ALT_Z),
			peek<Uint32>(Addr::PLAYER_Z));
	}

	// ---- helper: spawn TibiaStub.exe ----------------------------------
	// Looks for TibiaStub.exe next to TFC.exe. Launches it as a child.
	// The stub will OpenFileMapping our shim section, then sit forever
	// copying shim -> Tibia 8.60 absolute addresses inside ITS process.
	static void terminateExistingStubs(const char* stubPath)
	{
		DWORD pids[1024];
		DWORD needed = 0;
		if(!EnumProcesses(pids, sizeof(pids), &needed))
			return;

		DWORD count = needed / sizeof(DWORD);
		for(DWORD i = 0; i < count; ++i)
		{
			DWORD pid = pids[i];
			if(pid == 0 || pid == GetCurrentProcessId())
				continue;

			HANDLE h = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_TERMINATE, FALSE, pid);
			if(!h)
				continue;

			char path[MAX_PATH] = {0};
			if(GetModuleFileNameExA(h, NULL, path, MAX_PATH) && _stricmp(path, stubPath) == 0)
			{
				log("spawnStub: terminating stale stub PID=%lu", pid);
				TerminateProcess(h, 0);
				WaitForSingleObject(h, 1000);
			}
			CloseHandle(h);
		}
	}

	static bool spawnStub()
	{
		char tfcPath[MAX_PATH] = {0};
		if(!GetModuleFileNameA(NULL, tfcPath, MAX_PATH))
		{
			log("spawnStub: GetModuleFileNameA failed (GLE=%lu)", GetLastError());
			return false;
		}
		char* slash = std::strrchr(tfcPath, '\\');
		if(!slash) slash = std::strrchr(tfcPath, '/');
		if(slash) *(slash + 1) = '\0';
		else      tfcPath[0]   = '\0';

		char stubPath[MAX_PATH] = {0};
		_snprintf_s(stubPath, sizeof(stubPath), _TRUNCATE,
			"%sTibiaStub.exe", tfcPath);

		log("spawnStub: launching %s", stubPath);
		terminateExistingStubs(stubPath);

		STARTUPINFOA si;
		std::memset(&si, 0, sizeof(si));
		si.cb = sizeof(si);
		std::memset(&s_stubProcInfo, 0, sizeof(s_stubProcInfo));

		BOOL ok = CreateProcessA(
			stubPath,    // lpApplicationName
			NULL,        // lpCommandLine
			NULL, NULL,
			FALSE,       // bInheritHandles
			0,           // dwCreationFlags
			NULL,        // lpEnvironment
			tfcPath,     // lpCurrentDirectory
			&si,
			&s_stubProcInfo);
		if(!ok)
		{
			log("spawnStub: CreateProcess failed (GLE=%lu) -- ElfBot will "
			    "have to be pointed at this TFC process directly, but it "
			    "will then crash for the usual Win10 layout reasons.",
			    GetLastError());
			return false;
		}
		log("spawnStub: stub PID=%lu", s_stubProcInfo.dwProcessId);
		return true;
	}

	// ---- helper: create the shared-memory shim section ----------------
	static bool createShim()
	{
		s_shimMapping = CreateFileMappingA(
			INVALID_HANDLE_VALUE,    // page-file backed (no real file)
			NULL,
			PAGE_READWRITE,
			0, sizeof(TibiaShimBlock),
			TIBIA_SHIM_NAME);
		if(!s_shimMapping)
		{
			log("createShim: CreateFileMapping failed (GLE=%lu)", GetLastError());
			return false;
		}
		bool created = (GetLastError() != ERROR_ALREADY_EXISTS);

		s_shim = static_cast<TibiaShimBlock*>(MapViewOfFile(
			s_shimMapping, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TibiaShimBlock)));
		if(!s_shim)
		{
			log("createShim: MapViewOfFile failed (GLE=%lu)", GetLastError());
			CloseHandle(s_shimMapping); s_shimMapping = NULL;
			return false;
		}

		if(created)
			std::memset(s_shim, 0, sizeof(TibiaShimBlock));

		s_shim->magic   = 0x48535442u;  // 'TBSH' little-endian
		s_shim->version = TIBIA_SHIM_VERSION;
		s_shim->tfcPid  = GetCurrentProcessId();
		s_shim->frame   = 0;
		s_shim->status  = 0;

		log("createShim: section=%p view=%p size=0x%X", s_shimMapping, s_shim,
			static_cast<unsigned>(sizeof(TibiaShimBlock)));
		return true;
	}

	// ---- init --------------------------------------------------------
	bool init()
	{
#ifdef _WIN32
		// Open the log file and install the crash filter FIRST so any
		// fault during the rest of init() is captured.
		openLogFile();
		installCrashHandler();
		dumpLoadedModules("init() start");

		log("TLS low Tibia range: ok=%u fail=%u firstFail=0x%IX",
			s_tlsText.ok, s_tlsText.fail, s_tlsText.firstFail);

		// The in-process ElfBot emulation is always enabled.
		// TFC registers the "TibiaClient" window class so tibiaelf.exe
		// injects elfbot.dll directly into this process; init() must
		// always complete so ElfBot finds valid Tibia memory structures
		// instead of zero-initialised BSS (which causes an immediate
		// NULL-dereference crash inside elfbot.dll at startup).
		if(!protectLowTibiaShadow())
		{
			log("FATAL: Tibia shadow could not be made writable.");
			s_active = false;
			return false;
		}

		if(!isLowTibiaRangeReady())
		{
			log("FATAL: 0x00400000..0x00800000 was not reserved inside Tibia.exe.");
			s_active = false;
			return false;
		}

		if(!initTibiaPointerSlots(false))
		{
			log("FATAL: Tibia pointer slots could not be initialised.");
			s_active = false;
			return false;
		}

		// Install JMP hooks over the Tibia 8.60 function stubs in the
		// shadow so ElfBot's cavebot (AutoWalk) and combat (SetAttack,
		// SetFollow) calls are intercepted and forwarded to TFC.
		// Pass true = shadow is ready (protectLowTibiaShadow succeeded).
		installOldTibiaActionHooks(true);

		s_active = true;
		log("init() OK -- in-process ElfBot mode; s_active=true");
		return true;
#else
		// Non-Windows: ElfBot is Win32-only, so this layer is a no-op.
		s_active = false;
		return false;
#endif
	}

	void shutdown()
	{
#ifdef _WIN32
		if(s_shim)        { UnmapViewOfFile(s_shim); s_shim = NULL; }
		if(s_shimMapping) { CloseHandle(s_shimMapping); s_shimMapping = NULL; }
		if(s_tibiaHwnd && !s_tibiaHwndIsRealWindow)
			DestroyWindow(s_tibiaHwnd);
		s_tibiaHwnd = NULL;
		s_tibiaHwndIsRealWindow = false;

		s_regionText = NULL;
		s_regionData = NULL;
		s_hasMirroredGoto = false;
		resetElfbotWriteWatch();
		if(s_logFile) { std::fflush(s_logFile); std::fclose(s_logFile); s_logFile = NULL; }
#endif
		s_active = false;
	}

	bool isActive() { return s_active; }

	// ---- window-class shim -------------------------------------------
	// Make SDL create the real game window with Tibia 8.60's class name
	// so tibiaelf.exe injects into this Tibia.exe process directly.
	void registerTibiaWindowClass()
	{
#ifdef _WIN32
		HINSTANCE hInst = GetModuleHandleW(NULL);
		static bool s_attemptedRegister = false;
		if(!s_attemptedRegister)
		{
			s_attemptedRegister = true;
			char className[] = "TibiaClient";
			if(SDL_RegisterApp(className, CS_BYTEALIGNCLIENT, hInst) == 0)
				log("registerTibiaWindowClass: SDL_RegisterApp(TibiaClient) OK");
			else
				log("registerTibiaWindowClass: SDL_RegisterApp failed: %s", SDL_GetError());
		}

		WNDCLASSEXW wc;
		std::memset(&wc, 0, sizeof(wc));
		wc.cbSize = sizeof(wc);
		if(GetClassInfoExW(hInst, L"TibiaClient", &wc))
			log("registerTibiaWindowClass: class TibiaClient is registered");
		else
			log("registerTibiaWindowClass: class TibiaClient missing GLE=%lu", GetLastError());

		// Do not create a fake 1x1 TibiaClient window here. ElfBot binds
		// game state to the HWND it finds; if it finds a decoy instead of
		// the real SDL window it behaves as if the client is still at the
		// login screen. shimWriteClientWindow() records the real HWND once
		// SDL_CreateWindow has completed.
#endif
	}

	// ---- per-frame sync into the shared-memory shim ------------------
	// Each frame we serialize g_game's player state, the known creature
	// list, containers, etc. into the TibiaShimBlock the stub maps. The
	// stub copies it from there to the Tibia 8.60 absolute addresses
	// inside its own process.

	static void writeFixedStr(char* dst, const std::string& s, size_t maxLen)
	{
		std::memset(dst, 0, maxLen);
		size_t n = s.size();
		if(n > maxLen - 1) n = maxLen - 1;
		std::memcpy(dst, s.data(), n);
	}

	static void shimWriteClientWindow(TibiaShimBlock* shim)
	{
		shim->clientHwnd = 0;
		shim->clientX = shim->clientY = shim->clientW = shim->clientH = 0;
		shim->clientMinimized = 0;
		shim->clientVisible = 0;

		if(!g_engine.m_window)
			return;

		SDL_SysWMinfo info;
		SDL_VERSION(&info.version);
		if(!SDL_GetWindowWMInfo(g_engine.m_window, &info))
			return;

		HWND hwnd = info.info.win.window;
		if(!hwnd || !IsWindow(hwnd))
			return;

		if(s_tibiaHwnd != hwnd || !s_tibiaHwndIsRealWindow)
		{
			s_tibiaHwnd = hwnd;
			s_tibiaHwndIsRealWindow = true;
			char cls[64] = {0};
			GetClassNameA(hwnd, cls, sizeof(cls));
			log("real client HWND=%p class='%s'", hwnd, cls);
		}

		RECT r;
		if(GetWindowRect(hwnd, &r))
		{
			shim->clientX = r.left;
			shim->clientY = r.top;
			shim->clientW = r.right - r.left;
			shim->clientH = r.bottom - r.top;
		}

		shim->clientHwnd = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(hwnd));
		shim->clientMinimized = IsIconic(hwnd) ? 1u : 0u;
		shim->clientVisible = IsWindowVisible(hwnd) ? 1u : 0u;
	}

	static void updateNativeClientTitle(const TibiaShimBlock* shim)
	{
		if(!g_engine.m_window)
			return;

		char title[128];
		if(shim->status == 8 && shim->creatures[0].name[0] != '\0')
			_snprintf_s(title, sizeof(title), _TRUNCATE, "Tibia - %s", shim->creatures[0].name);
		else
			_snprintf_s(title, sizeof(title), _TRUNCATE, "Tibia");

		static char s_lastTitle[128] = {0};
		if(std::strcmp(s_lastTitle, title) == 0)
			return;

		SDL_SetWindowTitle(g_engine.m_window, title);
		strncpy_s(s_lastTitle, sizeof(s_lastTitle), title, _TRUNCATE);
		log("client title: %s", title);
	}

	static void shimWritePlayer(TibiaShimBlock* shim)
	{
		TibiaShimPlayer& p = shim->player;
		p.experience        = g_game.getPlayerExperience();
		p.id                = g_game.getPlayerID();
		p.health            = g_game.getPlayerHealth();
		p.healthMax         = g_game.getPlayerMaxHealth();
		p.mana              = g_game.getPlayerMana();
		p.manaMax           = g_game.getPlayerMaxMana();
		p.soul              = g_game.getPlayerSoul();
		p.stamina           = g_game.getPlayerStamina();
		p.capacity          = static_cast<uint32_t>(g_game.getPlayerCapacity());
		p.level             = g_game.getPlayerLevel();
		p.magicLevel        = g_game.getPlayerMagicLevel();
		p.levelPercent      = g_game.getPlayerLevelPercent();
		p.magicLevelPercent = g_game.getPlayerMagicLevelPercent();

		Creature* self = g_map.getLocalCreature();
		p.z = (self ? static_cast<uint8_t>(self->getCurrentPosition().z) : 0);

		static const Skills kSkillOrder[7] = {
			Skills_Fist, Skills_Club, Skills_Sword, Skills_Axe,
			Skills_Distance, Skills_Shielding, Skills_Fishing
		};
		for(int i = 0; i < 7; ++i)
		{
			p.skillPercent[i] = static_cast<uint8_t>(
				g_game.getPlayerSkillLevelPercent(kSkillOrder[i]));
			p.skillLevel[i]   = g_game.getPlayerSkillLevel(kSkillOrder[i]);
		}

		p.targetId = g_game.getAttackID();
		p.followId = g_game.getFollowID();
		p.selectId = g_game.getSelectID();

		for(int slot = 0; slot < 10 && slot < SLOT_LAST; ++slot)
		{
			ItemUI* item = g_game.getInventoryItem(static_cast<Uint8>(slot));
			if(item)
			{
				p.inventory[slot].id    = item->getID();
				p.inventory[slot].count = item->getItemCount();
			}
			else
			{
				p.inventory[slot].id    = 0;
				p.inventory[slot].count = 0;
			}
		}
	}

	static void shimWriteCreature(TibiaShimCreature& dst, Creature* c)
	{
		std::memset(&dst, 0, sizeof(dst));
		if(!c) return;
		Position& pos = c->getCurrentPosition();
		dst.id          = c->getId();
		// Tibia 8.60 stores the creature category in byte +3, which is
		// also the high byte of the 32-bit creature id. Do not write TFC's
		// CREATURETYPE_* enum here, or ElfBot will see a different battle
		// list id than Player.Id and refuse to bind to the local player.
		dst.type        = static_cast<uint8_t>((dst.id >> 24) & 0xFF);
		writeFixedStr(dst.name, c->getName(), sizeof(dst.name));
		dst.x           = pos.x;
		dst.y           = pos.y;
		dst.z           = pos.z;
		dst.isWalking   = c->isWalking() ? 1 : 0;
		dst.direction   = static_cast<uint32_t>(c->getDirection());

		// --- Outfit (offsets +96..+124) -------------------------------
		// In Tibia 8.60 the looktype lives in byte +96 (with the rest of
		// the dword reserved for flags), the four colour bytes follow at
		// +100/+104/+108/+112, addon at +116, light radius at +120 and
		// light colour at +124. Memory-scanning bots (Cheat-Engine-style
		// readers) read these exact offsets to find outfit / addon /
		// light state of each battle-list entry.
		//
		// TFC stores looktype as a pointer to ThingType, not as an
		// integer. m_thingType->m_id holds the underlying numeric id
		// the protocol sent; that's what 8.60 expects in the outfit
		// slot. Falls back to 0 (invisible) if the thing manager has
		// not resolved the id yet, e.g. for one frame right after a
		// setOutfit() with an unknown looktype.
		ThingType* tt = c->getThingType();
		dst.outfit      = tt ? tt->m_id : 0;
		dst.colorHead   = c->getOutfitHead();
		dst.colorBody   = c->getOutfitBody();
		dst.colorLegs   = c->getOutfitLegs();
		dst.colorFeet   = c->getOutfitFeet();
		dst.addon       = c->getLookAddons();
		dst.light       = c->getLightRadius();
		dst.lightColor  = c->getLightColor();

		dst.hpBar       = c->getHealth();
		dst.walkSpeed   = c->getSpeed();
		// TFC's Creature::isVisible() is a render-state flag and is often
		// false for valid known creatures in the battle list. Tibia 8.60's
		// battle-list +144 flag means "entry is usable/visible to client";
		// ElfBot targeting ignores entries with this byte cleared.
		dst.isVisible   = (dst.id != 0 && dst.name[0] != '\0' && dst.hpBar > 0) ? 1 : 0;
		dst.skull       = c->getSkull();
		dst.party       = c->getShield();
		dst.isBlocking  = c->isUnpassable() ? 1 : 0;
	}

	static void shimWriteBattleList(TibiaShimBlock* shim)
	{
		uint32_t slot = 0;
		Creature* self = g_map.getLocalCreature();

		// Fallback: if g_map hasn't recorded the local creature yet
		// (happens for a few frames right after login, before the
		// protocol code calls setLocalCreature), look it up by the
		// known player id. This guarantees the local player lands at
		// slot 0 from the very first frame ElfBot can see us --
		// otherwise the player gets placed at some arbitrary slot via
		// the knownCreatures iteration below and ElfBot never binds.
		if(!self)
		{
			Uint32 localId = g_game.getPlayerID();
			if(localId != 0)
				self = g_map.getCreatureById(localId);
		}

		if(self)
		{
			shimWriteCreature(shim->creatures[0], self);
			slot = 1;
		}

		knownCreatures& kc = g_map.getKnownCreatures();
		for(knownCreatures::iterator it = kc.begin(); it != kc.end(); ++it)
		{
			if(slot >= 250) break;
			Creature* c = it->second;
			if(!c || c == self) continue;
			shimWriteCreature(shim->creatures[slot], c);
			++slot;
		}

		// Zero remaining slots so the stub can copy a known-clean block.
		for(uint32_t i = slot; i < 250; ++i)
			std::memset(&shim->creatures[i], 0, sizeof(TibiaShimCreature));

		shim->creatureCount = slot;
	}

	static void shimWriteContainers(TibiaShimBlock* shim)
	{
		for(uint8_t cid = 0; cid < 16 && cid < GAME_MAX_CONTAINERS; ++cid)
		{
			TibiaShimContainer& d = shim->containers[cid];
			Container* c = g_game.findContainer(cid);
			if(!c)
			{
				std::memset(&d, 0, sizeof(d));
				continue;
			}
			d.isOpen = 1;
			d.id     = c->getCid();
			writeFixedStr(d.name, c->getName(), sizeof(d.name));
			d.volume = c->getCapacity();

			std::vector<ItemUI*>& items = c->getItems();
			d.amount = static_cast<uint32_t>(items.size());
			if(!items.empty() && items[0])
			{
				d.firstItemId    = items[0]->getID();
				d.firstItemCount = items[0]->getItemCount();
			}
			else
			{
				d.firstItemId    = 0;
				d.firstItemCount = 0;
			}
		}
	}

	void sync()
	{
		if(!s_active) return;
#ifdef _WIN32
		static TibiaShimBlock localShim;
		static bool localShimInit = false;
		static bool hasPublishedFrame = false;
		static bool s_prevIngame = false;
		if(!localShimInit)
		{
			std::memset(&localShim, 0, sizeof(localShim));
			localShim.magic = 0x48535442; // 'TBSH'
			localShim.version = TIBIA_SHIM_VERSION;
			localShim.tfcPid = GetCurrentProcessId();
			localShimInit = true;
		}

		// --- Session-boundary reset ------------------------------------
		// On every logout->login OR login->logout transition, scrub
		// per-session static state so stale mirrored values can't be read
		// against a fresh g_game. Symptom of skipping this: random crash
		// on relog because applyShim writes BattleList entries / target
		// IDs based on the *previous* character's data.
		bool ingame = g_engine.isIngame();
		if(ingame != s_prevIngame)
		{
			s_hasMirroredTargets = false;
			s_mirroredAttackId   = 0;
			s_mirroredFollowId   = 0;
			s_hasMirroredGoto    = false;
			resetElfbotWriteWatch();
			s_lastMessageText[0]   = '\0';
			s_lastMessageAuthor[0] = '\0';
			s_lastLookText[0]      = '\0';
			std::memset(&localShim.player,  0, sizeof(localShim.player));
			std::memset(&localShim.creatures, 0, sizeof(localShim.creatures));
			localShim.creatureCount = 0;
			// Reset the ElfBot "game ready" gate so a relog cleanly
			// re-arms it. Otherwise the in-game tick counter is left
			// at 0x80 across logout and ElfBot keeps painting the title
			// as "Slot N" for one or two frames after the player has
			// already returned to the character list.
			poke<Uint32>(Addr::CLIENT_GAMETICK_COUNTER, 0);
			poke<Uint32>(Addr::PLAYER_GOTO_X, 0);
			poke<Uint32>(Addr::PLAYER_GOTO_Y, 0);
			poke<Uint32>(Addr::PLAYER_GOTO_Z, 0);
			// Don't touch g_tibia860ImageShadow directly here -- the
			// next applyShim()/shimWriteBattleList() pass will overwrite
			// the slots it owns. Anything we don't own (Tibia's static
			// resource tables, RSA bytes etc.) must remain intact.
			s_prevIngame = ingame;
			log("session reset: ingame=%d", ingame ? 1 : 0);
		}

		// While logged out, do NOT publish a frame. The shadow contains
		// stale player/battlelist data from the previous character; any
		// writeback poll against it would dispatch garbage to the next
		// session.
		if(!ingame)
		{
			hasPublishedFrame = false;
			return;
		}

		if(hasPublishedFrame)
			processElfbotWriteback();

		// Detect when elfbot.dll has been injected into the real client.
		{
			static bool s_loggedElfbot = false;
			if(!s_loggedElfbot)
			{
				if(GetModuleHandleA("elfbot.dll"))
				{
					s_loggedElfbot = true;
					log("DETECTED: elfbot.dll loaded into Tibia.exe");
					dumpElfbotRelatedModules("elfbot detected");
					dumpAndScanElfbotImageOnce();
				}
			}
		}

		// HOTKEY-FIRING DIAGNOSTIC ---------------------------------------
		// When ElfBot dispatches a hotkey it writes a feedback string of
		// the form "Auto ON: ..." or "Auto OFF: ..." (or a one-shot
		// command echo) into the status-bar buffer at 0x7E07B0. If we
		// see that string appear after the user presses the hotkey,
		// ElfBot's keyboard hook + dispatch are working internally and
		// the only thing left to wire is the WinSock send hook.
		// If we never see it, the keypress isn't reaching ElfBot (SDL2
		// is eating it before Windows can deliver WM_HOTKEY to ElfBot's
		// RegisterHotKey'd window).
		{
			static char s_prevStatusBar[64] = {0};
			char now[64] = {0};
			const char* src = reinterpret_cast<const char*>(
				static_cast<uintptr_t>(Addr::CLIENT_STATUSBAR_TEXT));
			for(int i = 0; i < 63; ++i)
			{
				char c = src[i];
				if(c == '\0') break;
				if(c < 32 || c > 126) { now[i] = '?'; }
				else                  { now[i] = c;   }
			}
			if(now[0] && std::strcmp(now, s_prevStatusBar) != 0)
			{
				// Only log strings we did NOT write ourselves. Our own
				// inbound-mirror writes shim->lastMessageText to the
				// status bar; anything else came from ElfBot (or another
				// in-process writer).
				if(std::strcmp(now, s_lastMessageText) != 0)
					log("STATUSBAR changed (likely ElfBot): '%s'", now);
				strncpy_s(s_prevStatusBar, sizeof(s_prevStatusBar), now, _TRUNCATE);
			}
		}

		shimWriteClientWindow(&localShim);
		shimWritePlayer    (&localShim);
		shimWriteBattleList(&localShim);
		shimWriteContainers(&localShim);
		writeFixedStr(localShim.lastMessageText, s_lastMessageText, sizeof(localShim.lastMessageText));
		writeFixedStr(localShim.lastMessageAuthor, s_lastMessageAuthor, sizeof(localShim.lastMessageAuthor));

		localShim.status = (g_game.getPlayerID() != 0) ? 8u : 0u;
		updateNativeClientTitle(&localShim);

		MemoryBarrier();
		localShim.frame = ++s_frameCounter;
		applyShimInProcess(&localShim);
		hasPublishedFrame = true;
		logShimSnapshot(&localShim);
#endif
	}
}

#ifdef _WIN32
// ----------------------------------------------------------------------
// TLS callback: process-startup early reservation.
//
// Placed at the very bottom of this translation unit so that the
// static members of namespace ElfbotCompat it references (s_tlsRan,
// s_tlsText, etc.) have already been declared at this point. The
// callback itself must be at file scope so the .CRT$XLB section
// pointer below resolves.
//
// ntdll's loader invokes it during LdrpInitializeProcess, after the
// EXE's static-import DLLs have run their DllMain(DLL_PROCESS_ATTACH)
// but BEFORE the EXE entry point (mainCRTStartup -> CRT init ->
// global C++ constructors -> main()). This is our one shot at
// claiming the Tibia 8.60 DATA address ranges before global ctors
// (g_game, g_map, etc.) or auxiliary thread stacks land on them.
//
// Constraints inside a TLS callback:
//   - The C/C++ runtime is NOT yet initialized -> no CRT calls.
//   - We must not throw C++ exceptions.
//   - VirtualAlloc / GetLastError / kernel32 functions ARE safe.
//
// Per-cluster ok/fail counts are recorded into ElfbotCompat::s_tlsX so
// init() can log them once CRT I/O is safe.
// ----------------------------------------------------------------------
static void NTAPI elfbotTlsCallback(PVOID /*hModule*/, DWORD reason, PVOID /*reserved*/)
{
	if(reason != DLL_PROCESS_ATTACH) return;
	if(ElfbotCompat::s_tlsRan) return;
	ElfbotCompat::s_tlsRan = true;

	const SIZE_T GRAN = 0x10000;
	// MUST match the array declaration in elfbot_shadow.cpp.
	const SIZE_T size = 0x006C0000;

	const volatile void* shadow = static_cast<const volatile void*>(&g_tibia860ImageShadow[0]);
	void* shadowBase = const_cast<void*>(shadow);
	DWORD oldProtect = 0;
	VirtualProtect(shadowBase, size, PAGE_EXECUTE_READWRITE, &oldProtect);

	g_tibia860ImageShadow[0] = 0;
	g_tibia860ImageShadow[size - 1] = 0;
	ElfbotCompat::s_tlsText.ok = static_cast<Uint32>(size / GRAN);
}

// Register the TLS callback.
//   - .CRT$XLB pointer entry (CRT's own callbacks sit in .CRT$XLC; XLB
//     sorts before XLC alphabetically, so ours runs first).
//   - /INCLUDE:__tls_used forces the linker to emit the PE TLS directory.
//   - /INCLUDE:_g_elfbotTlsCallbackPtr prevents the linker from stripping
//     our pointer (32-bit symbol decoration adds a leading underscore).
#pragma section(".CRT$XLB", long, read)
extern "C" __declspec(allocate(".CRT$XLB"))
PIMAGE_TLS_CALLBACK g_elfbotTlsCallbackPtr = elfbotTlsCallback;

#ifdef _WIN64
#pragma comment(linker, "/INCLUDE:_tls_used")
#pragma comment(linker, "/INCLUDE:g_elfbotTlsCallbackPtr")
#else
#pragma comment(linker, "/INCLUDE:__tls_used")
#pragma comment(linker, "/INCLUDE:_g_elfbotTlsCallbackPtr")
#endif
#endif // _WIN32
