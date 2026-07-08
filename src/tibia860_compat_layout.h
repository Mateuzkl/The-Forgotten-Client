/*
  Tibia 8.60 compatibility mirror layout used by ElfBotCompat.

  TFC builds one snapshot per frame from live g_game/g_map/g_engine state,
  then writes that snapshot into the fixed Tibia 8.60 memory addresses that
  ElfBot reads. Byte layout MUST stay stable: use fixed-width integer types
  and pack the structs so compiler padding cannot move fields.
*/

#ifndef TIBIA860_COMPAT_LAYOUT_H_
#define TIBIA860_COMPAT_LAYOUT_H_

#include <stdint.h>

#define TIBIA_SHIM_VERSION 0x00010002u

#pragma pack(push, 1)

// Matches Tibia 8.60's per-creature record (BattleList.StepCreatures = 0xA8).
// Field offsets must match TibiaAPI Version860.cs Creature.*.
struct TibiaShimCreature
{
	uint32_t id;                 // +0
	uint8_t  type;               // +4 (lives at +3 in real Tibia; padded here)
	uint8_t  _pad0[3];           // align name to +4 (Tibia uses +4 too)
	char     name[32];           // +8 ... copied to the real +4 name slot
	uint32_t x;                  // +40
	uint32_t y;                  // +44
	uint32_t z;                  // +48
	uint32_t screenOffH;         // +52
	uint32_t screenOffV;         // +56
	uint8_t  _pad1[16];          // 60..75
	uint32_t isWalking;          // +76
	uint32_t direction;          // +80
	uint8_t  _pad2[12];          // 84..95
	uint32_t outfit;             // +96
	uint32_t colorHead;          // +100
	uint32_t colorBody;          // +104
	uint32_t colorLegs;          // +108
	uint32_t colorFeet;          // +112
	uint32_t addon;              // +116
	uint32_t light;              // +120
	uint32_t lightColor;         // +124
	uint8_t  _pad3[4];           // 128..131
	uint32_t blackSquare;        // +132
	uint8_t  hpBar;              // +136
	uint8_t  _pad4[3];           // 137..139
	uint32_t walkSpeed;          // +140
	uint32_t isVisible;          // +144
	uint8_t  skull;              // +148
	uint8_t  _pad5[3];           // 149..151
	uint32_t party;              // +152
	uint8_t  _pad6[4];           // 156..159
	uint32_t warIcon;            // +160
	uint32_t isBlocking;         // +164
};
typedef char TibiaShimCreature_size_must_be_0xA8[
	(sizeof(TibiaShimCreature) == 0xA8) ? 1 : -1];

// Matches Tibia 8.60 container record (Container.StepContainer = 492).
struct TibiaShimContainer
{
	uint32_t isOpen;             // +0
	uint32_t id;                 // +4
	uint8_t  _pad0[8];           // 8..15
	char     name[32];           // +16
	uint32_t volume;             // +48
	uint32_t amount;             // +52
	uint8_t  _pad1[4];           // 56..59
	uint16_t firstItemId;        // +60
	uint16_t _pad2;              // 62..63
	uint32_t firstItemCount;     // +64
	uint8_t  slots[492 - 68];
};
typedef char TibiaShimContainer_size_must_be_492[
	(sizeof(TibiaShimContainer) == 492) ? 1 : -1];

// Matches Tibia 8.60 VIP record (Vip.StepPlayers = 0x2C = 44).
struct TibiaShimVip
{
	uint32_t id;                 // +0
	char     name[30];           // +4
	uint16_t status;             // +34
	uint8_t  _pad0[4];           // 36..39
	uint32_t icon;               // +40
};
typedef char TibiaShimVip_size_must_be_0x2C[
	(sizeof(TibiaShimVip) == 0x2C) ? 1 : -1];

struct TibiaShimPlayer
{
	uint64_t experience;
	uint32_t id;
	uint32_t health;
	uint32_t healthMax;
	uint32_t mana;
	uint32_t manaMax;
	uint32_t soul;
	uint32_t stamina;
	uint32_t capacity;
	uint16_t level;
	uint16_t magicLevel;
	uint8_t  levelPercent;
	uint8_t  magicLevelPercent;
	uint8_t  z;
	uint8_t  _pad0;

	uint8_t  skillPercent[7];
	uint8_t  _pad1;
	uint16_t skillLevel[7];
	uint8_t  _pad2[2];

	uint32_t targetId;
	uint32_t followId;
	uint32_t selectId;

	struct { uint16_t id; uint16_t count; } inventory[10];
};

// One complete frame snapshot before it is copied into 8.60 addresses.
struct TibiaShimBlock
{
	uint32_t magic;              // 'TBSH' (0x48535442)
	uint32_t version;            // TIBIA_SHIM_VERSION
	uint32_t frame;
	uint32_t tfcPid;

	uint32_t status;             // 0 = offline, 8 = ingame

	uint32_t clientHwnd;
	int32_t  clientX;
	int32_t  clientY;
	int32_t  clientW;
	int32_t  clientH;
	uint32_t clientMinimized;
	uint32_t clientVisible;

	TibiaShimPlayer player;

	uint32_t creatureCount;
	TibiaShimCreature creatures[250];

	TibiaShimContainer containers[16];

	uint32_t vipCount;
	TibiaShimVip vips[200];

	char lastMessageText[256];
	char lastMessageAuthor[40];
};

#pragma pack(pop)

#endif // TIBIA860_COMPAT_LAYOUT_H_
