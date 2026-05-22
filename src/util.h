/*
  The Forgotten Client
  Copyright (C) 2020 Saiyans King

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __FILE_UTIL_h_
#define __FILE_UTIL_h_

#include "defines.h"

typedef std::vector<std::string> StringVector;
typedef void (*EventHandlerFunction)(Uint32, Sint32);

const Uint32 xtea_delta = 0x61C88647;

Uint8 colorTo8bit(Uint8 r, Uint8 g, Uint8 b);
void colorFrom8bitFloat(Uint8 color, float& r, float& g, float& b);
void colorFrom8bit(Uint8 color, Uint8& r, Uint8& g, Uint8& b);
void getOutfitColorFloat(Uint8 color, float& r, float& g, float& b);
void getOutfitColorRGB(Uint8 color, Uint8& r, Uint8& g, Uint8& b);
void getTrianglePointFloat(const float v0[2], const float v1[2], const float v2[2], const float c0[3], const float c1[3], const float c2[3], const float p[2], float c[3]);

bool SDL_HasSSSE3();
bool SDL_HasFMA3();
bool SDL_HasFMA4();
bool SDL_HasXOP();
char* SDL_GetCPUName();

Uint32 UTIL_ctz(Uint32 mask);
Uint32 UTIL_clz(Uint32 mask);
SDL_FORCE_INLINE Uint32 UTIL_power_of_2(Uint32 input)
{
	Uint32 value = 1;
	while(value < input)
		value <<= 1;
	return value;
}
void UTIL_integer_scale(Sint32& w, Sint32& h, Sint32 expectW, Sint32 expectH, Sint32 limitW, Sint32 limitH);

std::string SDL_ReadLEString(SDL_RWops* src);
void SDL_WriteLEString(SDL_RWops* src, const std::string& text);

Uint32 SDL_ReadProtobufTag(SDL_RWops* src);
Uint32 SDL_ReadProtobufSize(SDL_RWops* src);
Uint64 SDL_ReadProtobufVariant(SDL_RWops* src);
std::string SDL_ReadProtobufString(SDL_RWops* src);

void UTIL_SetClipboardTextLatin1(const char* clipboardText);
char* UTIL_GetClipboardTextLatin1();

void UTIL_MessageBox(bool fatalError, const char* message);
std::string UTIL_protocolDebugLogPath();
void UTIL_protocolDebugLog(const char* scope, const char* format, ...);
void UTIL_protocolDebugDumpMessage(const char* scope, const char* direction, const Uint8* buffer, Uint16 size, Uint16 readPos, Uint8 opcode);
void UTIL_OpenURL(const char* url);
void UTIL_TakeScreenshot(const char* filename, Uint32 flags);
void UTIL_ResizeEvent(Uint32 windowId, Sint32 width, Sint32 height);
void UTIL_SafeEventHandler(EventHandlerFunction eHandler, Uint32 param, Sint32 status);
void UTIL_UpdatePanels(void* pPanel, Sint32 x, Sint32 y);
void UTIL_ResizePanel(void* pPanel, Sint32 x, Sint32 y);

void UTIL_replaceString(std::string& str, const std::string& sought, const std::string& replacement);
StringVector UTIL_explodeString(const std::string& inString, const std::string& separator, Sint32 limit = -1);
Sint32 UTIL_random(Sint32 min_range, Sint32 max_range);
Uint16 UTIL_parseModifiers(Uint16 mods);
std::string UTIL_ipv4_tostring(Uint32 ipV4);
std::string UTIL_formatCreatureName(const std::string& name);
std::string UTIL_formatConsoleText(const std::string& text);
std::string UTIL_formatStringCommas(const std::string& v);
std::string UTIL_formatDate(const char* format, time_t time);
void UTIL_parseSizedText(const std::string& text, size_t start, Uint8 fontId, Uint32 allowedWidth, void* _THIS, size_t (*callback)(void* __THIS, bool skipLine, size_t start, size_t length));

template<typename T>
SDL_FORCE_INLINE T UTIL_safeMod(T value, T denom) {return (value % (denom > 1 ? denom : 1));}
template <typename T>
SDL_FORCE_INLINE T UTIL_min(T a, T b) {return (a < b ? a : b);}
template <typename T>
SDL_FORCE_INLINE T UTIL_max(T a, T b) {return (a > b ? a : b);}

SDL_FORCE_INLINE bool UTIL_isPartyMember(Uint8 shield)
{
	switch(shield)
	{
		case SHIELD_WHITEYELLOW:
		case SHIELD_BLUE:
		case SHIELD_YELLOW:
		case SHIELD_BLUE_SHAREDEXP:
		case SHIELD_YELLOW_SHAREDEXP:
		case SHIELD_BLUE_NOSHAREDEXP_BLINK:
		case SHIELD_YELLOW_NOSHAREDEXP_BLINK:
		case SHIELD_BLUE_NOSHAREDEXP:
		case SHIELD_YELLOW_NOSHAREDEXP:
			return true;
		default: return false;
	}
}
SDL_FORCE_INLINE bool UTIL_isPartyLeader(Uint8 shield)
{
	switch(shield)
	{
		case SHIELD_WHITEYELLOW:
		case SHIELD_YELLOW:
		case SHIELD_YELLOW_SHAREDEXP:
		case SHIELD_YELLOW_NOSHAREDEXP_BLINK:
		case SHIELD_YELLOW_NOSHAREDEXP:
			return true;
		default: return false;
	}
}
SDL_FORCE_INLINE bool UTIL_isPartySharedEnabled(Uint8 shield)
{
	switch(shield)
	{
		case SHIELD_YELLOW_SHAREDEXP:
		case SHIELD_YELLOW_NOSHAREDEXP_BLINK:
		case SHIELD_YELLOW_NOSHAREDEXP:
		case SHIELD_BLUE_SHAREDEXP:
		case SHIELD_BLUE_NOSHAREDEXP_BLINK:
		case SHIELD_BLUE_NOSHAREDEXP:
			return true;
		default: return false;
	}
}

void UTIL_initSubsystem();

typedef size_t(*LPUTIL_Faststrstr)(const char* haystack, size_t haystackSize, const char* needle, size_t needleSize);
extern LPUTIL_Faststrstr UTIL_Faststrstr;
typedef bool(*LPUTIL_FastCopy)(Uint8*, const Uint8*, size_t);
extern LPUTIL_FastCopy UTIL_FastCopy;
typedef bool(*LPXTEA_DECRYPT)(Uint8*, size_t, const Uint32*);
extern LPXTEA_DECRYPT XTEA_decrypt;
typedef bool(*LPXTEA_ENCRYPT)(Uint8*, size_t, const Uint32*);
extern LPXTEA_ENCRYPT XTEA_encrypt;
typedef Uint32(*LPADLER32CHECKSUM)(const Uint8*, size_t);
extern LPADLER32CHECKSUM adler32Checksum;

#endif /* __FILE_UTIL_h_ */
