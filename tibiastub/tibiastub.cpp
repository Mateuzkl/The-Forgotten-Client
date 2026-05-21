/*
  TibiaStub.exe - the "shape" of Tibia 8.60 for ElfBot to attach to.

  This program does almost nothing on its own. Its purpose is to be a
  Win32 process whose PE image covers the Tibia 8.60 data address range
  (0x440000 - 0x800000), so that when tibiaelf.exe injects elfbot.dll
  into this process, every hard-coded Tibia 8.60 address it dereferences
  lands on memory we own.

  How we cover the address range:
    - Linker BaseAddress  = 0x00400000
    - Linker FixedBaseAddress = true, RandomizedBaseAddress = false
    - A 4 MB uninitialized array `g_tibiaShadow` declared at file scope
      lives in .bss, extending the PE's ImageSize to >= 0x400000.

  The Windows loader does NtAllocateVirtualMemory for the entire
  ImageSize at process creation, BEFORE ntdll's heap exists. That
  reservation is permanent for the process. Anything (heap, apphelp
  shim DB, ole32 worker thread stacks) trying to use addresses in
  0x400000 - 0x800000 will fall back to other ranges instead.

  Runtime behavior:
    1. Open / wait for shared-memory section "Local\TibiaShim860"
       created by the parent Forgotten Client process.
    2. Register window class "TibiaClient" + create hidden 1x1 decoy
       window so tibiaelf.exe's FindWindow finds us.
    3. Spin in a 16 ms loop:
         a. Read the shim block
         b. Copy each field to its Tibia 8.60 absolute address
       Done forever, until the parent dies.
*/

#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#include "tibia_shim_ipc.h"

// ---------------------------------------------------------------------
// CRT replacements. We link with IgnoreAllDefaultLibraries=true so the
// usual msvcrt.lib / libcmt.lib isn't pulled in -- that keeps our PE
// tiny so .bss lands at a low RVA. But MSVC's optimizer still emits
// calls to memset/memcpy for zeroing loops and struct copies, so we
// supply minimal implementations here.
//
// MSVC has memset/memcpy/memmove/memcmp registered as intrinsics
// (auto-included by /Oi). Defining a function with those names is
// normally an error (C2169). The pragma below temporarily disables
// the intrinsic registration for those four names, letting us provide
// real out-of-line definitions that the linker can resolve.
// ---------------------------------------------------------------------
#pragma function(memset, memcpy, memmove, memcmp)

extern "C" void* __cdecl memset(void* dst, int v, size_t n)
{
	unsigned char* d = (unsigned char*)dst;
	while(n--) *d++ = (unsigned char)v;
	return dst;
}
extern "C" void* __cdecl memcpy(void* dst, const void* src, size_t n)
{
	unsigned char*       d = (unsigned char*)dst;
	const unsigned char* s = (const unsigned char*)src;
	while(n--) *d++ = *s++;
	return dst;
}
// MSVC may also emit `_memmove` for overlapping copies and `_memcmp` for
// equality checks. Add stubs preemptively so we don't get more LNK2001s.
extern "C" void* __cdecl memmove(void* dst, const void* src, size_t n)
{
	unsigned char*       d = (unsigned char*)dst;
	const unsigned char* s = (const unsigned char*)src;
	if(d < s) { while(n--) *d++ = *s++; }
	else      { d += n; s += n; while(n--) *--d = *--s; }
	return dst;
}
extern "C" int __cdecl memcmp(const void* a, const void* b, size_t n)
{
	const unsigned char* p = (const unsigned char*)a;
	const unsigned char* q = (const unsigned char*)b;
	while(n--) { if(*p != *q) return (int)*p - (int)*q; ++p; ++q; }
	return 0;
}

// ---------------------------------------------------------------------
// Minimal file logger (Win32 only, no CRT).
// Writes to TibiaStub.log next to the .exe.
// ---------------------------------------------------------------------
static HANDLE s_log = INVALID_HANDLE_VALUE;

static int my_strlen(const char* s) { int n = 0; while(s[n]) ++n; return n; }

static char* hex8(char* dst, unsigned long v)
{
	static const char H[] = "0123456789ABCDEF";
	for(int i = 7; i >= 0; --i) { dst[i] = H[v & 0xF]; v >>= 4; }
	return dst + 8;
}
static char* hex_var(char* dst, unsigned long v, int width)
{
	// Right-justified hex with leading zeros, up to 8 chars.
	static const char H[] = "0123456789ABCDEF";
	char buf[16]; int n = 0;
	if(!v) buf[n++] = '0';
	while(v) { buf[n++] = H[v & 0xF]; v >>= 4; }
	while(n < width) buf[n++] = '0';
	for(int i = 0; i < n; ++i) dst[i] = buf[n - 1 - i];
	return dst + n;
}
static char* udec(char* dst, unsigned long v)
{
	char buf[16]; int n = 0;
	if(!v) buf[n++] = '0';
	while(v) { buf[n++] = (char)('0' + (v % 10)); v /= 10; }
	for(int i = 0; i < n; ++i) dst[i] = buf[n - 1 - i];
	return dst + n;
}

static void log_open()
{
	wchar_t path[MAX_PATH];
	DWORD n = GetModuleFileNameW(NULL, path, MAX_PATH);
	if(n == 0 || n >= MAX_PATH) return;
	for(DWORD i = n; i > 0; --i)
	{
		if(path[i - 1] == L'\\' || path[i - 1] == L'/')
		{
			static const wchar_t kFile[] = L"TibiaStub.log";
			int j = 0;
			while(kFile[j]) { path[i + j] = kFile[j]; ++j; }
			path[i + j] = 0;
			break;
		}
	}
	s_log = CreateFileW(path, GENERIC_WRITE, FILE_SHARE_READ, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
}

// log("plain text line\n"); writes the string verbatim. No formatting.
static void log_str(const char* s)
{
	if(s_log == INVALID_HANDLE_VALUE) return;
	DWORD w = 0;
	WriteFile(s_log, s, (DWORD)my_strlen(s), &w, NULL);
	FlushFileBuffers(s_log);
}

// log_kv("key", value_hex_8); writes "key=0xVVVVVVVV\n".
static void log_kv(const char* key, unsigned long v)
{
	char buf[128];
	int i = 0;
	while(key[i] && i < 64) { buf[i] = key[i]; ++i; }
	buf[i++] = '='; buf[i++] = '0'; buf[i++] = 'x';
	char* p = hex8(buf + i, v);
	*p++ = '\n'; *p = 0;
	log_str(buf);
}

static void log_shim_snapshot(const TibiaShimBlock* shim)
{
	char buf[256];
	int i = 0;
	const char prefix[] = "shim data: status=";
	for(int k = 0; prefix[k]; ++k) buf[i++] = prefix[k];
	char* p = udec(buf + i, shim->status);
	i = (int)(p - buf);
	const char mid1[] = " id=";
	for(int k = 0; mid1[k]; ++k) buf[i++] = mid1[k];
	p = udec(buf + i, shim->player.id);
	i = (int)(p - buf);
	const char mid2[] = " level=";
	for(int k = 0; mid2[k]; ++k) buf[i++] = mid2[k];
	p = udec(buf + i, shim->player.level);
	i = (int)(p - buf);
	const char mid3[] = " hp=";
	for(int k = 0; mid3[k]; ++k) buf[i++] = mid3[k];
	p = udec(buf + i, shim->player.health);
	i = (int)(p - buf);
	const char mid4[] = " creatures=";
	for(int k = 0; mid4[k]; ++k) buf[i++] = mid4[k];
	p = udec(buf + i, shim->creatureCount);
	i = (int)(p - buf);
	const char mid5[] = " name=";
	for(int k = 0; mid5[k]; ++k) buf[i++] = mid5[k];
	for(int k = 0; k < 32 && shim->creatures[0].name[k] && i < (int)sizeof(buf) - 2; ++k)
		buf[i++] = shim->creatures[0].name[k];
	buf[i++] = '\n';
	buf[i] = 0;
	log_str(buf);
}

static void log_wpath(const char* prefix, const wchar_t* path)
{
	char buf[320];
	int i = 0;
	while(prefix[i] && i < (int)sizeof(buf) - 1) { buf[i] = prefix[i]; ++i; }
	if(path)
	{
		for(int j = 0; path[j] && i < (int)sizeof(buf) - 2; ++j)
		{
			wchar_t ch = path[j];
			buf[i++] = (ch >= 32 && ch <= 126) ? (char)ch : '?';
		}
	}
	buf[i++] = '\n';
	buf[i] = 0;
	log_str(buf);
}

// Module enumeration -- log every loaded DLL with its base + name.
static void log_modules(const char* tag)
{
	HMODULE mods[256]; DWORD needed = 0;
	HANDLE hp = GetCurrentProcess();
	if(!EnumProcessModules(hp, mods, sizeof(mods), &needed)) return;
	DWORD count = needed / sizeof(HMODULE);
	if(count > 256) count = 256;
	char header[64] = "--- modules: ";
	int hi = 0; while(header[hi]) ++hi;
	while(*tag && hi < 60) header[hi++] = *tag++;
	header[hi++] = '\n'; header[hi] = 0;
	log_str(header);
	for(DWORD k = 0; k < count; ++k)
	{
		char name[MAX_PATH] = {0};
		GetModuleFileNameA(mods[k], name, MAX_PATH);
		char line[MAX_PATH + 32];
		int li = 0; line[li++] = ' '; line[li++] = ' ';
		line[li++] = '0'; line[li++] = 'x';
		char* p = hex8(line + li, (unsigned long)(uintptr_t)mods[k]);
		li = (int)(p - line);
		line[li++] = ' '; line[li++] = ' ';
		for(int j = 0; name[j] && li < (int)sizeof(line) - 2; ++j) line[li++] = name[j];
		line[li++] = '\n'; line[li] = 0;
		log_str(line);
	}
}

// SetUnhandledExceptionFilter target: log AV details.
static LONG WINAPI stub_crash_filter(EXCEPTION_POINTERS* ep)
{
	if(!ep || !ep->ExceptionRecord) return EXCEPTION_CONTINUE_SEARCH;
	log_str("\n*** STUB CRASH ***\n");
	log_kv("ExceptionCode  ", ep->ExceptionRecord->ExceptionCode);
	log_kv("ExceptionAddr  ", (unsigned long)(uintptr_t)ep->ExceptionRecord->ExceptionAddress);
	if(ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION
	   && ep->ExceptionRecord->NumberParameters >= 2)
	{
		log_kv("AccessType     ", (unsigned long)ep->ExceptionRecord->ExceptionInformation[0]);
		log_kv("BadAddress     ", (unsigned long)ep->ExceptionRecord->ExceptionInformation[1]);
	}
	if(ep->ContextRecord)
	{
		log_kv("EIP            ", ep->ContextRecord->Eip);
		log_kv("EAX            ", ep->ContextRecord->Eax);
		log_kv("EBX            ", ep->ContextRecord->Ebx);
		log_kv("ECX            ", ep->ContextRecord->Ecx);
		log_kv("EDX            ", ep->ContextRecord->Edx);
		log_kv("ESI            ", ep->ContextRecord->Esi);
		log_kv("EDI            ", ep->ContextRecord->Edi);
		log_kv("EBP            ", ep->ContextRecord->Ebp);
		log_kv("ESP            ", ep->ContextRecord->Esp);
	}
	log_modules("at crash");
	return EXCEPTION_EXECUTE_HANDLER;
}

// ---------------------------------------------------------------------
// Set once by initTibiaPointerSlots(), then read by applyShim() to
// fill "safe sentinel" values into otherwise-unused fields. Keeping
// this as a non-zero, self-referential pointer makes ElfBot's reads
// of these fields land in our 1 MB safe region instead of crashing.
// ---------------------------------------------------------------------
static uint32_t g_safeAddr = 0;

static bool initFakeDatPointer()
{
	// ElfBot calls small DAT flag helpers copied from Tibia 8.60. Those
	// helpers expect Client.DatPointer to reference a descriptor:
	//   +0 = first encoded thing id, +4 = last encoded thing id,
	//   +8 = record array, 0x4C bytes per record.
	// The item ids passed here are encoded as 0x40000000 | itemId.
	const uint32_t kFirstThingId = 0x40000000u;
	const uint32_t kThingCount = 0x10000u;
	const SIZE_T kRecordSize = 0x4C;
	const SIZE_T kRecordsSize = (SIZE_T)kThingCount * kRecordSize;

	uint8_t* records = (uint8_t*)VirtualAlloc(NULL, kRecordsSize,
		MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	uint32_t* descriptor = (uint32_t*)VirtualAlloc(NULL, 0x1000,
		MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if(!records || !descriptor)
	{
		log_str("initFakeDatPointer: VirtualAlloc failed\n");
		return false;
	}

	descriptor[0] = kFirstThingId;
	descriptor[1] = kFirstThingId + kThingCount - 1;
	descriptor[2] = (uint32_t)(uintptr_t)records;
	*(volatile uint32_t*)(uintptr_t)0x007998DC = (uint32_t)(uintptr_t)descriptor;

	log_kv("fake dat desc  ", (unsigned long)(uintptr_t)descriptor);
	log_kv("fake dat recs  ", (unsigned long)(uintptr_t)records);
	return true;
}

// ---------------------------------------------------------------------
// THE SHADOW.
// 3.5 MB of uninitialized memory. The MSVC linker places this in .bss
// after .text/.rdata/.data. With a tiny .text (a few KB), .bss begins
// at a low RVA, and the array spans up to RVA 0x400000, giving us a
// committed PE-image-backed mapping for the entire Tibia 8.60 range.
//
// `volatile` prevents the compiler from optimizing the array away if
// it ever "looks" unused; `static` keeps it internal-linkage.
// ---------------------------------------------------------------------
static volatile char g_tibiaShadow[3 * 1024 * 1024 + 1024 * 1024];  // 4 MB

// ---------------------------------------------------------------------
// Tibia 8.60 absolute addresses we write to. Verified against TibiaAPI
// Version860.cs. Each helper writes one field type to the address.
// ---------------------------------------------------------------------
static inline void poke_u8 (uintptr_t addr, uint8_t  v) { *(volatile uint8_t* )addr = v; }
static inline void poke_u16(uintptr_t addr, uint16_t v) { *(volatile uint16_t*)addr = v; }
static inline void poke_u32(uintptr_t addr, uint32_t v) { *(volatile uint32_t*)addr = v; }
static inline void poke_u64(uintptr_t addr, uint64_t v) { *(volatile uint64_t*)addr = v; }
static inline void poke_str(uintptr_t addr, const char* src, size_t maxLen)
{
	char* dst = (char*)addr;
	size_t i = 0;
	for(; i < maxLen - 1 && src[i]; ++i) dst[i] = src[i];
	for(; i < maxLen; ++i) dst[i] = 0;
}

// ---------------------------------------------------------------------
// Apply one frame of shim data to Tibia 8.60 addresses.
// ---------------------------------------------------------------------
static void applyShim(const TibiaShimBlock* shim)
{
	// Client.Status (0x79CF28) -- 8 = in game in Tibia 8.60.
	poke_u32(0x0079CF28, shim->status);

	// Player block (anchor: Experience at 0x63FE8C).
	poke_u64(0x0063FE8C, shim->player.experience);   // experience (u64)
	poke_u32(0x0063FE98, shim->player.id);            // id (Exp+12)
	poke_u32(0x0063FE94, shim->player.health);        // hp (Exp+8)
	poke_u32(0x0063FE90, shim->player.healthMax);     // hp max (Exp+4)
	poke_u32(0x0063FE88, (uint32_t)shim->player.level);            // level (Exp-4)
	poke_u32(0x0063FE84, (uint32_t)shim->player.magicLevel);       // mlvl (Exp-8)
	poke_u32(0x0063FE80, (uint32_t)shim->player.levelPercent);     // %lvl (Exp-12)
	poke_u32(0x0063FE7C, (uint32_t)shim->player.magicLevelPercent);// %mlvl (Exp-16)
	poke_u32(0x0063FE78, shim->player.mana);          // mana (Exp-20)
	poke_u32(0x0063FE74, shim->player.manaMax);       // mana max (Exp-24)
	poke_u32(0x0063FE70, (uint32_t)shim->player.soul);             // soul (Exp-28)
	poke_u32(0x0063FE6C, shim->player.stamina);       // stamina (Exp-32)
	poke_u32(0x0063FE68, shim->player.capacity);      // cap (Exp-36)

	// Skills (anchor: FistPercent at 0x63FE24).
	for(int i = 0; i < 7; ++i)
	{
		poke_u32(0x0063FE24 + i * 4,        shim->player.skillPercent[i]);
		poke_u32(0x0063FE24 + 28 + i * 4,   shim->player.skillLevel[i]);
	}

	// Target / follow IDs.
	poke_u32(0x0063FE64, shim->player.targetId);  // RedSquare
	poke_u32(0x0063FE60, shim->player.followId);  // GreenSquare (RedSq-4)
	poke_u32(0x0063FE5C, shim->player.selectId);  // TargetBattlelistId (RedSq-8)

	// Player Z (separate slot at 0x64F600).
	poke_u32(0x0064F600, (uint32_t)shim->player.z);

	// Inventory slots (0x64CC98, 10 slots * 12 bytes).
	for(int i = 0; i < 10; ++i)
	{
		uintptr_t base = 0x0064CC98 + i * 12;
		poke_u16(base + 0, shim->player.inventory[i].id);
		poke_u16(base + 2, 0);
		poke_u32(base + 4, shim->player.inventory[i].count);
		poke_u32(base + 8, 0);
	}

	// Battle list (0x63FEF8, 168-byte stride, up to 250 entries).
	for(uint32_t i = 0; i < 250; ++i)
	{
		uintptr_t base = 0x0063FEF8 + i * 0xA8;
		if(i >= shim->creatureCount)
		{
			// Mark slot empty by writing 0 to the id field. Leave the
			// rest of the 168-byte slot alone -- it was prefilled with
			// the safe-target address by initTibiaPointerSlots(), so
			// any read through it lands in the self-referential safe
			// region rather than AVing on garbage. (Zeroing the whole
			// slot here would re-create the original null-deref crash
			// on the next ElfBot iteration.)
			*(volatile uint32_t*)(base + 0) = 0;
			continue;
		}
		const TibiaShimCreature& c = shim->creatures[i];
		poke_u32(base + 0,   c.id);
		poke_u8 (base + 3,   c.type);
		poke_str(base + 4,   c.name, 32);
		poke_u32(base + 36,  c.x);
		poke_u32(base + 40,  c.y);
		poke_u32(base + 44,  c.z);
		poke_u32(base + 48,  c.screenOffH);
		poke_u32(base + 52,  c.screenOffV);
		poke_u32(base + 76,  c.isWalking);
		poke_u32(base + 80,  c.direction);
		poke_u32(base + 96,  c.outfit);
		poke_u32(base + 120, c.light);
		poke_u32(base + 132, c.blackSquare);
		poke_u8 (base + 136, c.hpBar);
		poke_u32(base + 140, c.walkSpeed);
		poke_u32(base + 144, c.isVisible);
		poke_u8 (base + 148, c.skull);
		poke_u32(base + 152, c.party);
		poke_u32(base + 160, c.warIcon);
		poke_u32(base + 164, c.isBlocking);
	}

	// Containers (0x64CD10, 492-byte stride, 16 slots).
	for(int i = 0; i < 16; ++i)
	{
		uintptr_t base = 0x0064CD10 + i * 492;
		const TibiaShimContainer& c = shim->containers[i];
		// isOpen=0 marks slot closed. We don't touch the rest of the
		// 492-byte slot for the same reason as the battle list: it
		// was prefilled with the safe-target pointer, and overwriting
		// it with zeros would re-introduce null-deref AVs.
		poke_u32(base + 0,  c.isOpen);
		poke_u32(base + 4,  c.id);
		poke_str(base + 16, c.name, 32);
		poke_u32(base + 48, c.volume);
		poke_u32(base + 56, c.amount);
		poke_u16(base + 60, c.firstItemId);
		poke_u32(base + 64, c.firstItemCount);
	}

	// VIP list (0x63DBB8, 44-byte stride, 200 slots).
	for(uint32_t i = 0; i < 200; ++i)
	{
		uintptr_t base = 0x0063DBB8 + i * 0x2C;
		if(i >= shim->vipCount)
		{
			// id=0 marks slot empty; leave rest as safe-target prefill.
			poke_u32(base, 0);
			continue;
		}
		const TibiaShimVip& v = shim->vips[i];
		poke_u32(base + 0,  v.id);
		poke_str(base + 4,  v.name, 30);
		poke_u16(base + 34, v.status);
		poke_u32(base + 40, v.icon);
	}

	// Last chat message (0x7E0A00 text, 0x7E0A00-0x28 = 0x7E09D8 author).
	poke_str(0x007E0A00, shim->lastMessageText,   256);
	poke_str(0x007E09D8, shim->lastMessageAuthor,  40);
}

// ---------------------------------------------------------------------
// Load the WHOLE Tibia.exe PE image into our shadow.
//
// We tried embedding individual function blobs (kept below as backup),
// but ElfBot reads / calls many more Tibia addresses than just the
// handful we identified. The robust answer: pull the entire genuine
// Tibia.exe from disk, parse its PE headers, and copy each section
// to its virtual address inside our 4 MB shadow. After this, every
// byte ElfBot reads or calls in the 0x401000-0x7FFFFF range is
// IDENTICAL to what it would see attached to a real Tibia.exe.
//
// We skip our own stub's .text (RVA 0x1000) and .rdata (RVA 0x2000)
// since those contain THIS code -- overwriting them mid-execution
// would crash us. The skipped range is only 8 KB at Tibia's image
// start, which is just early CRT/loader code ElfBot doesn't care
// about.
// ---------------------------------------------------------------------
static const wchar_t* kTibiaPathCandidates[] = {
	L"Tibia860.exe",          // next to TibiaStub.exe (preferred)
	L"D:\\Tibia FILES\\Clients\\RL-Tibia Clients\\8.60\\Tibia.exe",
	NULL
};

static bool loadTibiaImage()
{
	HANDLE hFile = INVALID_HANDLE_VALUE;
	const wchar_t* foundPath = NULL;
	for(int i = 0; kTibiaPathCandidates[i]; ++i)
	{
		hFile = CreateFileW(kTibiaPathCandidates[i], GENERIC_READ,
			FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
		if(hFile != INVALID_HANDLE_VALUE)
		{
			foundPath = kTibiaPathCandidates[i];
			break;
		}
	}
	if(hFile == INVALID_HANDLE_VALUE)
	{
		log_str("loadTibiaImage: Tibia.exe not found at any candidate path\n");
		return false;
	}
	log_wpath("loadTibiaImage: source=", foundPath);

	LARGE_INTEGER fsz;
	if(!GetFileSizeEx(hFile, &fsz) || fsz.QuadPart < 0x1000 || fsz.QuadPart > 0x800000)
	{
		log_str("loadTibiaImage: bogus Tibia.exe size\n");
		CloseHandle(hFile);
		return false;
	}
	SIZE_T fileSize = (SIZE_T)fsz.QuadPart;

	uint8_t* fbuf = (uint8_t*)VirtualAlloc(NULL, fileSize,
		MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if(!fbuf) { CloseHandle(hFile); return false; }

	DWORD got = 0;
	if(!ReadFile(hFile, fbuf, (DWORD)fileSize, &got, NULL) || got != fileSize)
	{
		log_str("loadTibiaImage: ReadFile failed\n");
		VirtualFree(fbuf, 0, MEM_RELEASE);
		CloseHandle(hFile);
		return false;
	}
	CloseHandle(hFile);

	// Validate MZ + PE
	if(fbuf[0] != 'M' || fbuf[1] != 'Z')
	{
		log_str("loadTibiaImage: not an MZ file\n");
		VirtualFree(fbuf, 0, MEM_RELEASE);
		return false;
	}
	uint32_t peOff = *(uint32_t*)(fbuf + 0x3C);
	if(fbuf[peOff] != 'P' || fbuf[peOff + 1] != 'E')
	{
		log_str("loadTibiaImage: missing PE signature\n");
		VirtualFree(fbuf, 0, MEM_RELEASE);
		return false;
	}
	uint16_t numSec  = *(uint16_t*)(fbuf + peOff + 6);
	uint16_t optHdr  = *(uint16_t*)(fbuf + peOff + 20);
	uint32_t imgBase = *(uint32_t*)(fbuf + peOff + 24 + 28);
	uint8_t* secHdr  = fbuf + peOff + 24 + optHdr;

	log_kv("Tibia.exe imgBase", imgBase);
	log_kv("Tibia.exe sections", numSec);

	// We are ONLY allowed to write inside g_tibiaShadow. Anything
	// below g_tibiaShadow is either our PE header, .text/.rdata
	// (currently executing), or the .data globals that hold our
	// log handle, safe-target pointer, and other state. Writing
	// there silently destroys our own runtime state and the stub
	// dies immediately on the next log call.
	const uintptr_t shadowStart = (uintptr_t)&g_tibiaShadow[0];
	const uintptr_t shadowEnd   = shadowStart + sizeof(g_tibiaShadow);
	log_kv("shadow start", (uint32_t)shadowStart);
	log_kv("shadow end  ", (uint32_t)shadowEnd);

	for(int i = 0; i < numSec; ++i)
	{
		uint8_t* sh = secHdr + i * 40;
		char name[9] = {0};
		for(int j = 0; j < 8; ++j) name[j] = (char)sh[j];
		uint32_t vSize = *(uint32_t*)(sh + 8);
		uint32_t vAddr = *(uint32_t*)(sh + 12);
		uint32_t rSize = *(uint32_t*)(sh + 16);
		uint32_t rAddr = *(uint32_t*)(sh + 20);

		uint32_t copyBytes = (rSize < vSize) ? rSize : vSize;

		// Compute the absolute destination range Tibia wants.
		uintptr_t dstAbs    = (uintptr_t)imgBase + vAddr;
		uintptr_t dstAbsEnd = dstAbs + copyBytes;

		// Clamp into g_tibiaShadow.
		if(dstAbsEnd <= shadowStart) continue;  // entire section below shadow
		if(dstAbs    >= shadowEnd)   continue;  // entire section above shadow

		uintptr_t srcSkip = 0;
		if(dstAbs < shadowStart)
		{
			srcSkip   = shadowStart - dstAbs;
			copyBytes -= (uint32_t)srcSkip;
			dstAbs    = shadowStart;
		}
		if(dstAbs + copyBytes > shadowEnd)
		{
			copyBytes = (uint32_t)(shadowEnd - dstAbs);
		}
		if(copyBytes == 0) continue;

		uint8_t* dst = (uint8_t*)dstAbs;
		uint8_t* src = fbuf + rAddr + srcSkip;

		// Make destination pages writable+executable temporarily.
		DWORD oldProt;
		VirtualProtect(dst, copyBytes, PAGE_EXECUTE_READWRITE, &oldProt);

		// Manual byte copy.
		for(uint32_t k = 0; k < copyBytes; ++k) dst[k] = src[k];

		log_str("  copied section ");
		log_str(name);
		log_str("\n");
		log_kv("    dst", (uint32_t)(uintptr_t)dst);
		log_kv("    sz ", copyBytes);
	}

	VirtualFree(fbuf, 0, MEM_RELEASE);
	log_str("loadTibiaImage: complete\n");
	return true;
}

// ---------------------------------------------------------------------
// Real Tibia 8.60 function entry bytes (kept as a fallback if
// loadTibiaImage() can't find Tibia.exe).
// ---------------------------------------------------------------------
static const uint8_t kFunc_0x44E960[128] = {
	0x55,0x8B,0xEC,0x6A,0xFF,0x68,0x7E,0xCE,0x58,0x00,0x64,0xA1,0x00,0x00,0x00,0x00,
	0x50,0x64,0x89,0x25,0x00,0x00,0x00,0x00,0x81,0xEC,0x78,0x06,0x00,0x00,0x53,0x56,
	0x57,0x89,0x65,0xF0,0x8B,0xF1,0x33,0xDB,0x89,0x5D,0xFC,0x8B,0x7D,0x08,0x8D,0x87,
	0xEF,0xD8,0xFF,0xFF,0x3D,0xCA,0x00,0x00,0x00,0x0F,0x87,0xF9,0x06,0x00,0x00,0x0F,
	0xB6,0x80,0x0C,0xF3,0x44,0x00,0xFF,0x24,0x85,0xA0,0xF2,0x44,0x00,0x8B,0x4E,0x40,
	0x51,0x8B,0x56,0x34,0x52,0x8B,0x46,0x4C,0x50,0x8B,0x4E,0x48,0x51,0x8B,0x56,0x44,
	0x52,0xE8,0x4A,0x88,0xFB,0xFF,0x83,0xC4,0x14,0xE9,0xB0,0x06,0x00,0x00,0x53,0x8B,
	0x46,0x5C,0x50,0x8B,0x4E,0x50,0x51,0x8B,0x56,0x68,0x52,0x8B,0x46,0x64,0x50,0x8B,
};
static const uint8_t kFunc_0x452320[128] = {
	0x55,0x8B,0xEC,0x6A,0xFF,0x68,0xFD,0xD7,0x58,0x00,0x64,0xA1,0x00,0x00,0x00,0x00,
	0x50,0x64,0x89,0x25,0x00,0x00,0x00,0x00,0x81,0xEC,0xFC,0x03,0x00,0x00,0x53,0x56,
	0x57,0x89,0x65,0xF0,0x8B,0xF1,0x33,0xDB,0x8B,0x45,0x0C,0x3B,0xC3,0x0F,0x85,0x01,
	0x01,0x00,0x00,0xBE,0x0F,0x00,0x00,0x00,0x89,0xB5,0xCC,0xFE,0xFF,0xFF,0x89,0x9D,
	0xC8,0xFE,0xFF,0xFF,0x88,0x9D,0xB8,0xFE,0xFF,0xFF,0x6A,0x0A,0x68,0x64,0xA8,0x5B,
	0x00,0x8D,0x8D,0xB4,0xFE,0xFF,0xFF,0xE8,0x04,0x0F,0xFB,0xFF,0x89,0x5D,0xFC,0x89,
	0xB5,0x74,0xFF,0xFF,0xFF,0x89,0x9D,0x70,0xFF,0xFF,0xFF,0x88,0x9D,0x60,0xFF,0xFF,
	0xFF,0x53,0x68,0xCB,0xA7,0x5B,0x00,0x8D,0x8D,0x5C,0xFF,0xFF,0xFF,0xE8,0xDE,0x0E,
};
static const uint8_t kFunc_0x45A194[128] = {
	0x0F,0x84,0xCD,0x00,0x00,0x00,0xB9,0x04,0x00,0x00,0x00,0x89,0x4D,0x80,0x89,0x4D,
	0xCC,0xC7,0x85,0x60,0xFF,0xFF,0xFF,0x0E,0x00,0x00,0x00,0xBE,0xC8,0x00,0x00,0x00,
	0x89,0xB5,0x64,0xFF,0xFF,0xFF,0x8B,0xFE,0x89,0xBD,0x68,0xFF,0xFF,0xFF,0x8B,0xDE,
	0x89,0x9D,0x6C,0xFF,0xFF,0xFF,0xA1,0x6C,0xDA,0x79,0x00,0x83,0xE8,0x00,0x74,0x25,
	0x48,0x74,0x16,0x48,0x74,0x07,0x68,0xC4,0xE8,0x5B,0x00,0xEB,0x1D,0x68,0xC0,0xE8,
	0x5B,0x00,0x51,0x8D,0x4D,0xDC,0x51,0xEB,0x16,0x68,0xBC,0xE8,0x5B,0x00,0x51,0x8D,
	0x55,0xDC,0x52,0xEB,0x0A,0x68,0xB8,0xE8,0x5B,0x00,0x8D,0x45,0xDC,0x51,0x50,0xE8,
	0xA8,0xC8,0x0F,0x00,0x83,0xC4,0x0C,0xE8,0x80,0x88,0x0C,0x00,0x8B,0x10,0x8B,0xC8,
};
static const uint8_t kFunc_0x45A258[128] = {
	0xE8,0x73,0xAB,0x05,0x00,0x83,0xC4,0x24,0xC7,0x45,0xCC,0x12,0x00,0x00,0x00,0xE8,
	0xF4,0x25,0x0B,0x00,0x84,0xC0,0x0F,0x85,0xD6,0x00,0x00,0x00,0xE8,0xE7,0x2F,0x0C,
	0x00,0x8B,0xF8,0x89,0x7D,0xA0,0xE8,0xAD,0x2F,0x0C,0x00,0x85,0xC0,0x0F,0x8C,0xBF,
	0x00,0x00,0x00,0xE8,0xB0,0x2F,0x0C,0x00,0x85,0xC0,0x0F,0x8C,0xB2,0x00,0x00,0x00,
	0xE8,0x93,0x2F,0x0C,0x00,0x8B,0xF0,0x89,0x75,0xE4,0xE8,0x99,0x2F,0x0C,0x00,0x89,
	0x45,0xEC,0x8D,0x4F,0xFF,0x83,0xF9,0x05,0x77,0x70,0xFF,0x24,0x8D,0x3C,0xA6,0x45,
	0x00,0x2B,0x35,0xF0,0xA9,0x64,0x00,0x89,0x75,0xE4,0x8B,0x0D,0xF4,0xA9,0x64,0x00,
	0xEB,0x53,0x2B,0x35,0xF8,0xA9,0x64,0x00,0x89,0x75,0xE4,0x8B,0x0D,0xFC,0xA9,0x64,
};
static const uint8_t kFunc_0x45C370[128] = {
	0x55,0x8B,0xEC,0x6A,0xFF,0x68,0x97,0xF3,0x58,0x00,0x64,0xA1,0x00,0x00,0x00,0x00,
	0x50,0x64,0x89,0x25,0x00,0x00,0x00,0x00,0x81,0xEC,0xE0,0x03,0x00,0x00,0x53,0x56,
	0x57,0x89,0x65,0xF0,0xC7,0x45,0xEC,0xFF,0xFF,0xFF,0xFF,0x33,0xFF,0x89,0x7D,0xFC,
	0xBB,0x06,0x00,0x00,0x00,0xE8,0xF6,0xD4,0x09,0x00,0x8B,0xF0,0x89,0x75,0xEC,0x83,
	0xFE,0xFF,0x75,0x14,0x89,0x45,0xFC,0x8B,0x4D,0xF4,0x64,0x89,0x0D,0x00,0x00,0x00,
	0x00,0x5F,0x5E,0x5B,0x8B,0xE5,0x5D,0xC3,0x56,0xE8,0xC2,0xD0,0xFF,0xFF,0x83,0xC4,
	0x04,0xE8,0x0A,0x12,0x0C,0x00,0x83,0xF8,0x03,0x75,0x0A,0x6A,0x04,0xE8,0xDE,0x11,
	0x0C,0x00,0x83,0xC4,0x04,0xE8,0xF6,0x11,0x0C,0x00,0x83,0xF8,0x04,0x0F,0x85,0x55,
};
static const uint8_t kFunc_0x45C3A5[128] = {
	0xE8,0xF6,0xD4,0x09,0x00,0x8B,0xF0,0x89,0x75,0xEC,0x83,0xFE,0xFF,0x75,0x14,0x89,
	0x45,0xFC,0x8B,0x4D,0xF4,0x64,0x89,0x0D,0x00,0x00,0x00,0x00,0x5F,0x5E,0x5B,0x8B,
	0xE5,0x5D,0xC3,0x56,0xE8,0xC2,0xD0,0xFF,0xFF,0x83,0xC4,0x04,0xE8,0x0A,0x12,0x0C,
	0x00,0x83,0xF8,0x03,0x75,0x0A,0x6A,0x04,0xE8,0xDE,0x11,0x0C,0x00,0x83,0xC4,0x04,
	0xE8,0xF6,0x11,0x0C,0x00,0x83,0xF8,0x04,0x0F,0x85,0x55,0x01,0x00,0x00,0x8D,0x46,
	0xF6,0x83,0xF8,0x5A,0x0F,0x87,0x9F,0x00,0x00,0x00,0x0F,0xB6,0x80,0x74,0xCD,0x45,
	0x00,0xFF,0x24,0x85,0x54,0xCD,0x45,0x00,0xE8,0x8E,0xF5,0xFA,0xFF,0x89,0x35,0x4C,
	0x9F,0x63,0x00,0xEB,0x8B,0xE8,0x41,0xF7,0xFA,0xFF,0x89,0x35,0x4C,0x9F,0x63,0x00,
};
static const uint8_t kFunc_0x4B4DD0[128] = {
	0x55,0x8B,0xEC,0x6A,0xFF,0x68,0xCD,0xEC,0x59,0x00,0x64,0xA1,0x00,0x00,0x00,0x00,
	0x50,0x64,0x89,0x25,0x00,0x00,0x00,0x00,0x81,0xEC,0x98,0x01,0x00,0x00,0x53,0x56,
	0x57,0x89,0x65,0xF0,0x33,0xC9,0x89,0x4D,0xFC,0x33,0xC0,0x89,0x45,0xD0,0x89,0x4D,
	0xD4,0xBA,0xFF,0xFF,0xFF,0x1F,0x89,0x55,0xD8,0x8B,0xF2,0x89,0x75,0xDC,0x83,0xEC,
	0x10,0x8B,0xFC,0x89,0x07,0x89,0x4F,0x04,0x89,0x57,0x08,0x89,0x77,0x0C,0x8B,0x45,
	0x28,0x50,0x68,0xFF,0xFF,0xFF,0x7F,0x8B,0x55,0x24,0x52,0x51,0x83,0xEC,0x0C,0x8B,
	0xC4,0x8B,0x4D,0x18,0x89,0x08,0x8B,0x55,0x1C,0x89,0x50,0x04,0x8B,0x4D,0x20,0x89,
	0x48,0x08,0x8B,0x55,0x14,0x52,0x8B,0x45,0x10,0x50,0x8B,0x4D,0x0C,0x51,0x8B,0x55,
};
static const uint8_t kFunc_0x4B5990[128] = {
	0x55,0x8B,0xEC,0x6A,0xFF,0x68,0x44,0xEF,0x59,0x00,0x64,0xA1,0x00,0x00,0x00,0x00,
	0x50,0x64,0x89,0x25,0x00,0x00,0x00,0x00,0x81,0xEC,0x88,0x01,0x00,0x00,0x53,0x56,
	0x57,0x89,0x65,0xF0,0x33,0xC9,0x39,0x4D,0x18,0x0F,0x84,0xD1,0x02,0x00,0x00,0x89,
	0x4D,0xEC,0xB8,0x01,0x00,0x00,0x00,0x89,0x45,0xDC,0x89,0x45,0xE0,0x83,0xCF,0xFF,
	0x89,0x7D,0xC4,0x89,0x7D,0xCC,0x89,0x7D,0xD4,0x89,0x7D,0xC8,0x89,0x7D,0xE8,0x89,
	0x7D,0xD0,0x89,0x7D,0xE4,0x89,0x7D,0xD8,0x89,0x4D,0xFC,0x89,0x45,0xEC,0x6A,0x05,
	0x8B,0x45,0x18,0x50,0xE8,0xC7,0xB4,0x04,0x00,0x83,0xC4,0x08,0x84,0xC0,0x74,0x14,
	0x8D,0x4D,0xE0,0x51,0x8D,0x55,0xDC,0x52,0x8D,0x45,0x18,0x50,0xE8,0x7F,0x0A,0x05,
};
static const uint8_t kFunc_0x4B96E0[128] = {
	0x55,0x8B,0xEC,0x6A,0xFF,0x68,0x3E,0xF4,0x59,0x00,0x64,0xA1,0x00,0x00,0x00,0x00,
	0x50,0x64,0x89,0x25,0x00,0x00,0x00,0x00,0x81,0xEC,0x20,0x07,0x00,0x00,0x53,0x56,
	0x57,0x89,0x65,0xF0,0x8B,0x75,0x1C,0x85,0xF6,0x0F,0x84,0x65,0x05,0x00,0x00,0x8B,
	0x4D,0x14,0x85,0xC9,0x0F,0x84,0x5A,0x05,0x00,0x00,0x8B,0x5D,0x18,0x85,0xDB,0x0F,
	0x84,0x4F,0x05,0x00,0x00,0x8B,0x7D,0x0C,0x85,0xFF,0x0F,0x8D,0xAB,0x00,0x00,0x00,
	0x68,0x1C,0xB8,0x5B,0x00,0x8D,0x8D,0x58,0xFD,0xFF,0xFF,0xE8,0xE0,0xA6,0xF4,0xFF,
	0xC7,0x45,0xFC,0x00,0x00,0x00,0x00,0x68,0xCB,0xA7,0x5B,0x00,0x8D,0x8D,0xB0,0xFC,
	0xFF,0xFF,0xE8,0xC9,0xA6,0xF4,0xFF,0xC6,0x45,0xFC,0x01,0x68,0x2C,0xAE,0x5B,0x00,
};
static const uint8_t kFunc_0x4F5823[128] = {
	0xE8,0xA8,0xF5,0xFB,0xFF,0x83,0xC4,0x24,0xE9,0x8E,0x01,0x00,0x00,0x89,0x95,0x40,
	0xFF,0xFF,0xFF,0x83,0xC7,0xFC,0x89,0xBD,0x4C,0xFF,0xFF,0xFF,0x8B,0x46,0x1C,0x89,
	0x85,0x24,0xFC,0xFF,0xFF,0x83,0xC0,0xE4,0x50,0x6A,0x01,0x8D,0x85,0x40,0xFF,0xFF,
	0xFF,0x50,0xE8,0x56,0x63,0xF3,0xFF,0x8B,0x46,0x20,0x89,0x85,0xC0,0xFC,0xFF,0xFF,
	0x83,0xC0,0xFB,0x50,0x6A,0x01,0x8D,0x8D,0x4C,0xFF,0xFF,0xFF,0x51,0xE8,0x3B,0x63,
	0xF3,0xFF,0xE8,0x16,0xD2,0x02,0x00,0x33,0xFF,0x89,0xBD,0x10,0xFE,0xFF,0xFF,0x89,
	0xBD,0x14,0xFE,0xFF,0xFF,0x89,0xBD,0x18,0xFE,0xFF,0xFF,0x8B,0x95,0x4C,0xFF,0xFF,
	0xFF,0x8B,0x4D,0x10,0x03,0xCA,0x89,0x8D,0xB0,0xFE,0xFF,0xFF,0x89,0x8D,0xE4,0xFE,
};
static const uint8_t kFunc_0x4F8E40[128] = {
	0x55,0x8B,0xEC,0x6A,0xFF,0x68,0x40,0x8D,0x5A,0x00,0x64,0xA1,0x00,0x00,0x00,0x00,
	0x50,0x64,0x89,0x25,0x00,0x00,0x00,0x00,0x81,0xEC,0x1C,0x03,0x00,0x00,0x53,0x56,
	0x57,0x89,0x65,0xF0,0xA1,0x90,0x98,0x79,0x00,0x85,0xC0,0x0F,0x84,0x90,0x02,0x00,
	0x00,0xC7,0x45,0xFC,0x00,0x00,0x00,0x00,0x8A,0x45,0x08,0x84,0xC0,0xA1,0xA8,0x98,
	0x79,0x00,0x0F,0x84,0x16,0x02,0x00,0x00,0x89,0x45,0xD4,0x8D,0x70,0xFE,0x89,0x75,
	0xE8,0x8D,0x7E,0xFA,0x89,0x7D,0xCC,0xEB,0x07,0x8D,0xA4,0x24,0x00,0x00,0x00,0x00,
	0x8D,0x46,0xFC,0x25,0x07,0x00,0x00,0x80,0x79,0x05,0x48,0x83,0xC8,0xF8,0x40,0x74,
	0x16,0xE8,0xBA,0x89,0x04,0x00,0x50,0xB9,0xA0,0x98,0x79,0x00,0xE8,0x7F,0x06,0x06,
};

// ---------------------------------------------------------------------
// Pre-populate Tibia 8.60 pointer slots with a self-referential "safe
// target" region.
//
// Real Tibia.exe stores many pointers in its .data section. ElfBot
// reads those pointers and dereferences them to reach the actual game
// data structures. In our stub, every Tibia address starts as zero,
// so ElfBot's first dereference null-AVs (which is exactly what the
// crash log shows: BadAddress=0x00000000 at elfbot.dll+0x3C4A3).
//
// Fix: allocate a fresh 1 MB region, fill it with its own base
// address, then write that base address into every known pointer
// slot. Now ElfBot dereferences chain like:
//   *(0x654118)      = SAFE_BASE
//   *(SAFE_BASE)     = SAFE_BASE     (self-pointer)
//   *(SAFE_BASE+N)   = SAFE_BASE     (every dword is SAFE_BASE)
// so every dereference, no matter how many levels deep, lands inside
// the safe region. No more null AVs.
//
// Done once at startup, BEFORE tibiaelf.exe has a chance to inject.
// ---------------------------------------------------------------------
// Called with `tibiaLoaded=true` when loadTibiaImage() succeeded -- in
// that case we SKIP the shadow prefill and per-function blob copies
// (they'd overwrite real Tibia bytes with garbage). We always do the
// safe-target allocation and pointer-slot fixups since Tibia's
// initialised pointer values reference runtime allocations that
// don't exist in our process.
static void initTibiaPointerSlots(bool tibiaLoaded)
{
	// 1) Allocate 1 MB outside g_tibiaShadow -- the self-referential
	//    "safe target" region for any pointer dereferences ElfBot does
	//    on unmapped/uninitialised Tibia memory.
	const SIZE_T kSafe = 0x100000;  // 1 MB
	LPVOID safe = VirtualAlloc(NULL, kSafe, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if(!safe) { log_str("initTibiaPointerSlots: VirtualAlloc failed\n"); return; }

	uint32_t selfAddr = (uint32_t)(uintptr_t)safe;
	g_safeAddr = selfAddr;
	uint32_t* p = (uint32_t*)safe;
	for(SIZE_T i = 0; i < kSafe / sizeof(uint32_t); ++i) p[i] = selfAddr;
	log_kv("safe target    ", selfAddr);

	// 2) Shadow prefill -- only when we DIDN'T load the full Tibia
	//    image. Otherwise we'd erase the genuine Tibia bytes we just
	//    laid down.
	if(!tibiaLoaded)
	{
		uint32_t* shadow = (uint32_t*)&g_tibiaShadow[0];
		const SIZE_T shadowDwords = sizeof(g_tibiaShadow) / sizeof(uint32_t);
		for(SIZE_T i = 0; i < shadowDwords; ++i) shadow[i] = selfAddr;
		log_str("shadow prefilled with safe target\n");
	}
	else
	{
		log_str("shadow prefill SKIPPED (Tibia.exe was loaded)\n");
	}

	// 3) Pointer slots -- always override these. The values Tibia
	//    sets for them refer to its own runtime allocations (Map*,
	//    Socket*, etc.) which don't exist in our process. Pointing
	//    them at the safe region prevents the first AV chain we saw.
	#define POKE_PTR(addr)  *(volatile uint32_t*)(uintptr_t)(addr) = selfAddr
	POKE_PTR(0x005B85E4);  // Client.RecvPointer
	POKE_PTR(0x005B8610);  // Client.SendPointer
	POKE_PTR(0x00654118);  // Map.MapPointer
	POKE_PTR(0x0079DA74);  // Client.FrameRatePointer
	POKE_PTR(0x00799890);  // Client.SocketStruct
	POKE_PTR(0x0064C25C);  // Client.GameWindowRectPointer
	POKE_PTR(0x0064F5C4);  // Client.DialogPointer
	POKE_PTR(0x00795070);  // Client.LastRcvPacket
	POKE_PTR(0x007998AC);  // Client.RecvStream
	POKE_PTR(0x0051F650);  // Client.EventTriggerPointer
	POKE_PTR(0x0051D200);  // Client.ActionStateFreezer
	#undef POKE_PTR
	initFakeDatPointer();
	log_str("Tibia pointer slots initialised\n");

	// 4) If Tibia.exe wasn't loaded, fall back to per-function blob
	//    copies + RSA + small validation bytes. Loaded-Tibia already
	//    has all of these in place.
	if(!tibiaLoaded)
	{
		*(volatile uint16_t*)(uintptr_t)0x004F2809 = 19061;   // NameSpy1
		*(volatile uint16_t*)(uintptr_t)0x004F2813 = 16501;   // NameSpy2
		{
			static const uint8_t kLevelSpy[6] = { 0x89, 0x86, 0x88, 0x2A, 0x00, 0x00 };
			uint8_t* a = (uint8_t*)(uintptr_t)0x004F46BA;
			uint8_t* b = (uint8_t*)(uintptr_t)0x004F47BF;
			uint8_t* c = (uint8_t*)(uintptr_t)0x004F4840;
			for(int i = 0; i < 6; ++i) { a[i] = kLevelSpy[i]; b[i] = kLevelSpy[i]; c[i] = kLevelSpy[i]; }
		}
		*(volatile uint16_t*)(uintptr_t)0x004EAFA9 = 0x057E;
		*(volatile uint8_t*) (uintptr_t)0x004EAFAC = 0x80;

		DWORD oldProt;
		VirtualProtect((LPVOID)(uintptr_t)0x00440000, 0xC0000,
		               PAGE_EXECUTE_READWRITE, &oldProt);
		#define COPY_FUNC(addr, blob) \
			do { \
				uint8_t* dst = (uint8_t*)(uintptr_t)(addr); \
				for(size_t i = 0; i < sizeof(blob); ++i) dst[i] = (blob)[i]; \
			} while(0)
		COPY_FUNC(0x0044E960, kFunc_0x44E960);
		COPY_FUNC(0x00452320, kFunc_0x452320);
		COPY_FUNC(0x0045A194, kFunc_0x45A194);
		COPY_FUNC(0x0045A258, kFunc_0x45A258);
		COPY_FUNC(0x0045C370, kFunc_0x45C370);
		COPY_FUNC(0x0045C3A5, kFunc_0x45C3A5);
		COPY_FUNC(0x004B4DD0, kFunc_0x4B4DD0);
		COPY_FUNC(0x004B5990, kFunc_0x4B5990);
		COPY_FUNC(0x004B96E0, kFunc_0x4B96E0);
		COPY_FUNC(0x004F5823, kFunc_0x4F5823);
		COPY_FUNC(0x004F8E40, kFunc_0x4F8E40);
		#undef COPY_FUNC

		static const char kRealTibiaRsa[] =
			"124710459426827943004376449897985582167801707960697037164044904862"
			"948569380850421396904597686953877022394604239428185498284169068581"
			"802277612081027966724336319448537811441719076484340922854929273517"
			"308661370727105382899118999403808045846444647284499123164879035103"
			"627004668521005328367415259939915284902061793";
		char* dst = (char*)(uintptr_t)0x005B8980;
		int k = 0;
		while(kRealTibiaRsa[k]) { dst[k] = kRealTibiaRsa[k]; ++k; }
		dst[k] = 0;

		log_str("fallback function blobs + RSA + validation bytes written\n");
	}
}

// ---------------------------------------------------------------------
// Window-class registration so tibiaelf.exe finds us via FindWindow.
// Same pattern as TFC's hidden decoy, kept self-contained here.
// ---------------------------------------------------------------------
static HWND registerTibiaWindow()
{
	HINSTANCE hInst = GetModuleHandleW(NULL);
	WNDCLASSEXW wc = {0};
	wc.cbSize        = sizeof(wc);
	wc.style         = CS_BYTEALIGNCLIENT;
	wc.lpfnWndProc   = DefWindowProcW;
	wc.hInstance     = hInst;
	wc.hIcon         = LoadIconW(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursorW(NULL, IDC_ARROW);
	wc.lpszClassName = L"TibiaClient";
	RegisterClassExW(&wc);

	return CreateWindowExW(
		0, L"TibiaClient", L"Tibia",
		WS_POPUP,        // not WS_VISIBLE -- invisible decoy
		0, 0, 1, 1,
		NULL, NULL, hInst, NULL);
}

struct WindowBindContext
{
	DWORD pid;
	HWND owner;
	HWND tibiaHwnd;
	bool clientMinimized;
	bool clientVisible;
	bool becameMinimized;
	bool becameRestored;
};

struct TrackedWindow
{
	HWND hwnd;
	bool lastVisible;
	bool restoreOnClientShow;
};

static TrackedWindow s_trackedWindows[64];

static TrackedWindow* trackWindow(HWND hwnd)
{
	for(int i = 0; i < 64; ++i)
	{
		if(s_trackedWindows[i].hwnd == hwnd)
			return &s_trackedWindows[i];
	}
	for(int i = 0; i < 64; ++i)
	{
		if(!s_trackedWindows[i].hwnd || !IsWindow(s_trackedWindows[i].hwnd))
		{
			s_trackedWindows[i].hwnd = hwnd;
			s_trackedWindows[i].lastVisible = false;
			s_trackedWindows[i].restoreOnClientShow = false;
			return &s_trackedWindows[i];
		}
	}
	return NULL;
}

static BOOL CALLBACK bindProcessWindowProc(HWND hwnd, LPARAM lp)
{
	WindowBindContext* ctx = (WindowBindContext*)lp;
	if(hwnd == ctx->tibiaHwnd)
		return TRUE;

	DWORD pid = 0;
	GetWindowThreadProcessId(hwnd, &pid);
	if(pid != ctx->pid)
		return TRUE;

	if(GetWindow(hwnd, GW_OWNER) != ctx->owner)
		SetWindowLongPtrW(hwnd, GWLP_HWNDPARENT, (LONG_PTR)ctx->owner);

	TrackedWindow* tracked = trackWindow(hwnd);
	const bool visibleNow = (IsWindowVisible(hwnd) != FALSE);
	if(tracked && visibleNow && !ctx->clientMinimized && ctx->clientVisible)
		tracked->lastVisible = true;

	if(ctx->becameMinimized || !ctx->clientVisible)
	{
		if(tracked && (tracked->lastVisible || visibleNow))
			tracked->restoreOnClientShow = true;
		if(visibleNow)
			ShowWindow(hwnd, SW_HIDE);
	}
	else if(ctx->becameRestored && ctx->clientVisible)
	{
		if(tracked && tracked->restoreOnClientShow)
		{
			ShowWindow(hwnd, SW_SHOWNOACTIVATE);
			SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0,
				SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
			tracked->restoreOnClientShow = false;
			tracked->lastVisible = true;
		}
	}

	return TRUE;
}

static void syncClientWindowBridge(HWND tibiaHwnd, const TibiaShimBlock* shim)
{
	static HWND s_owner = NULL;
	static bool s_haveClientState = false;
	static bool s_wasClientMinimized = false;
	if(!shim || !shim->clientHwnd)
		return;

	HWND owner = (HWND)(uintptr_t)shim->clientHwnd;
	if(!IsWindow(owner))
		return;

	if(s_owner != owner)
	{
		s_owner = owner;
		SetWindowLongPtrW(tibiaHwnd, GWLP_HWNDPARENT, (LONG_PTR)owner);
		log_kv("client HWND   ", (unsigned long)(uintptr_t)owner);
		log_str("window bridge: stub windows now owned by client\n");
	}

	WindowBindContext ctx;
	ctx.pid = GetCurrentProcessId();
	ctx.owner = owner;
	ctx.tibiaHwnd = tibiaHwnd;
	ctx.clientMinimized = (shim->clientMinimized != 0);
	ctx.clientVisible = (shim->clientVisible != 0);
	ctx.becameMinimized = false;
	ctx.becameRestored = false;

	if(!s_haveClientState)
	{
		s_haveClientState = true;
		s_wasClientMinimized = ctx.clientMinimized;
	}
	else if(s_wasClientMinimized != ctx.clientMinimized)
	{
		ctx.becameMinimized = ctx.clientMinimized;
		ctx.becameRestored = !ctx.clientMinimized;
		s_wasClientMinimized = ctx.clientMinimized;
		log_str(ctx.clientMinimized ?
			"window bridge: client minimized, hiding ElfBot windows\n" :
			"window bridge: client restored, restoring ElfBot windows\n");
	}

	EnumWindows(bindProcessWindowProc, (LPARAM)&ctx);
}

// ---------------------------------------------------------------------
// Open the shared-memory mapping created by TFC. Polls every 100 ms
// until it exists (TFC must be running first).
// ---------------------------------------------------------------------
static const TibiaShimBlock* openShim(HANDLE* outMapping)
{
	*outMapping = NULL;
	for(int tries = 0; tries < 600; ++tries)  // 60s timeout
	{
		HANDLE h = OpenFileMappingA(FILE_MAP_READ, FALSE, TIBIA_SHIM_NAME);
		if(h)
		{
			void* view = MapViewOfFile(h, FILE_MAP_READ, 0, 0, sizeof(TibiaShimBlock));
			if(view)
			{
				const TibiaShimBlock* shim = (const TibiaShimBlock*)view;
				if(shim->magic == 0x48535442 /*'TBSH' LE*/
				   && shim->version == TIBIA_SHIM_VERSION)
				{
					*outMapping = h;
					return shim;
				}
				UnmapViewOfFile(view);
			}
			CloseHandle(h);
		}
		Sleep(100);
	}
	return NULL;
}

// ---------------------------------------------------------------------
// Entry point. /SUBSYSTEM:WINDOWS so no console pops up.
// ---------------------------------------------------------------------
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	log_open();
	log_str("=== TibiaStub started ===\n");
	SetUnhandledExceptionFilter(stub_crash_filter);
	log_kv("ImageBase      ", (unsigned long)(uintptr_t)GetModuleHandleA(NULL));
	log_kv("g_tibiaShadow  ", (unsigned long)(uintptr_t)&g_tibiaShadow[0]);
	log_kv("shadow size    ", (unsigned long)sizeof(g_tibiaShadow));
	log_modules("startup");

	// Force the shadow array into the working set by touching it.
	g_tibiaShadow[0] = 0;
	g_tibiaShadow[sizeof(g_tibiaShadow) - 1] = 0;
	log_str("shadow touched\n");

	// IMPORTANT: register the window class FIRST, before the slow
	// loadTibiaImage() call. If tibiaelf.exe runs while we're still
	// initialising, FindWindow("TibiaClient") returns NULL and the
	// user sees "Please run Tibia first". With this order, the
	// window exists almost immediately. We DO NOT proceed past
	// registerTibiaWindow until init is complete, so any elfbot.dll
	// injection attempt before then will block on OpenProcess /
	// fail safely.
	//
	// However, the window being visible to tibiaelf BEFORE we've
	// finished init means tibiaelf might inject elfbot.dll into a
	// half-set-up stub. To prevent that, we mark the decoy window
	// with a placeholder title until init is finished, and ElfBot's
	// injection requires the window's PID to match -- which is
	// already true as soon as the window exists, so there's no
	// perfect way to gate this from the stub side. The user must
	// wait until the "STUB READY" log line before launching
	// tibiaelf.exe.
	HWND hwnd = registerTibiaWindow();
	if(!hwnd)
	{
		log_str("FATAL: registerTibiaWindow failed\n");
		return 1;
	}
	log_kv("decoy HWND     ", (unsigned long)(uintptr_t)hwnd);

	// Load the WHOLE Tibia.exe image into our shadow. ElfBot's reads
	// of any Tibia 8.60 address (whether code or data) will now return
	// genuine Tibia bytes. If Tibia.exe isn't found on disk, we fall
	// back to the smaller per-function blob copies in initTibiaPointerSlots.
	bool tibiaLoaded = loadTibiaImage();
	if(tibiaLoaded)
		log_str("Tibia.exe fully loaded into shadow\n");
	else
		log_str("Tibia.exe load failed -- using function-blob fallback\n");

	// Pointer-slot init: overrides Tibia's own globals with our safe
	// target where needed.
	initTibiaPointerSlots(tibiaLoaded);

	log_str("\n========================================\n");
	log_str("STUB READY -- safe to launch tibiaelf.exe\n");
	log_str("========================================\n\n");

	HANDLE hMapping = NULL;
	const TibiaShimBlock* shim = openShim(&hMapping);
	if(!shim)
	{
		log_str("WARN: openShim returned NULL (TFC never made the section). "
			"Idling so tibiaelf can still attach.\n");
		MSG msg;
		while(GetMessageW(&msg, NULL, 0, 0)) DispatchMessageW(&msg);
		return 0;
	}
	log_kv("shim view      ", (unsigned long)(uintptr_t)shim);

	// Main loop: copy shim -> Tibia 8.60 addresses every ~16 ms.
	// Watch for elfbot.dll / USkin.dll loading and log when they do.
	DWORD lastFrame = 0;
	uint32_t lastLoggedStatus = 0xFFFFFFFFu;
	uint32_t lastLoggedPlayerId = 0xFFFFFFFFu;
	uint32_t lastLoggedCreatureCount = 0xFFFFFFFFu;
	bool sawElfbot = false, sawUskin = false;
	int  modulePollTick = 0;
	for(;;)
	{
		MSG msg;
		while(PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message == WM_QUIT) { log_str("WM_QUIT received\n"); return 0; }
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}

		syncClientWindowBridge(hwnd, shim);

		// Detect tibiaelf's injection ASAP.
		if(!sawElfbot && GetModuleHandleA("elfbot.dll"))
		{
			sawElfbot = true;
			log_str("DETECTED: elfbot.dll loaded into stub\n");
			log_modules("after elfbot.dll");
		}
		if(!sawUskin && GetModuleHandleA("USkin.dll"))
		{
			sawUskin = true;
			log_str("DETECTED: USkin.dll loaded into stub\n");
		}

		// Detect parent death.
		HANDLE hParent = OpenProcess(SYNCHRONIZE, FALSE, shim->tfcPid);
		if(hParent)
		{
			DWORD wait = WaitForSingleObject(hParent, 0);
			CloseHandle(hParent);
			if(wait == WAIT_OBJECT_0)
			{
				log_str("parent TFC died, exiting\n");
				break;
			}
		}

		if(shim->frame != lastFrame)
		{
			lastFrame = shim->frame;
			applyShim(shim);
			if(shim->status != lastLoggedStatus ||
			   shim->player.id != lastLoggedPlayerId ||
			   shim->creatureCount != lastLoggedCreatureCount)
			{
				lastLoggedStatus = shim->status;
				lastLoggedPlayerId = shim->player.id;
				lastLoggedCreatureCount = shim->creatureCount;
				log_shim_snapshot(shim);
			}
		}

		// Every ~5 seconds re-log the module list so we catch late loads.
		if(++modulePollTick >= 312)  // 312 * 16ms ~= 5s
		{
			modulePollTick = 0;
			if(sawElfbot && !sawUskin)
				log_str("(still no USkin.dll loaded after elfbot)\n");
		}

		Sleep(16);
	}

	log_str("=== TibiaStub exiting cleanly ===\n");
	UnmapViewOfFile((LPCVOID)shim);
	CloseHandle(hMapping);
	if(s_log != INVALID_HANDLE_VALUE) CloseHandle(s_log);
	return 0;
}
