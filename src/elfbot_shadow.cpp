#if defined(_WIN32) && !defined(_WIN64)

// Direct TFC ElfBot compatibility needs the real process to own Tibia
// 8.60's absolute address range before imported DLLs, heaps, NLS maps, or
// SDL can place anything there. A TLS VirtualAlloc is still too late on
// some Windows setups because imported DLL process-attach code runs first.
//
// The Win32 EXE is linked at 0x00140000 and this .text$aaa block is merged
// before the normal .text section. It starts at RVA 0x1000, so it covers:
//
//   0x00141000 .. 0x00801000
//
// That includes every 8.60 address ElfBot touches:
//   0x00440000 text/code/RSA
//   0x00630000 player/battlelist/client globals
//   0x00799F08 hotkey text
//
// The linker records this as virtual image size, not raw file bytes, so the
// executable does not grow by the full 6.75 MB on disk.
#pragma section(".text$aaa", read, write, execute)
__declspec(allocate(".text$aaa")) __declspec(align(4096))
volatile unsigned char g_tibia860ImageShadow[0x006C0000];

#endif
