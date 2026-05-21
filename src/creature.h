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

#ifndef __FILE_CREATURE_h_
#define __FILE_CREATURE_h_

#include "thing.h"
#include "thingManager.h"
#include "animator.h"

class ThingType;
class Tile;
class Creature : public Thing
{
	public:
		Creature() = default;

		// non-copyable
		Creature(const Creature&) = delete;
		Creature& operator=(const Creature&) = delete;

		// non-moveable
		Creature(Creature&&) = delete;
		Creature& operator=(Creature&&) = delete;

		static double speedA, speedB, speedC;

		virtual Creature* getCreature() {return this;}
		virtual const Creature* getCreature() const {return this;}

		virtual bool isCreature() {return true;}

		bool isPlayer() {return (m_type == CREATURETYPE_PLAYER);}
		bool isMonster() {return (m_type == CREATURETYPE_MONSTER || m_type == CREATURETYPE_SUMMON_OWN || m_type == CREATURETYPE_SUMMON_OTHERS);}
		bool isNPC() {return (m_type == CREATURETYPE_NPC);}

		void preMove(const Position& fromPos, const Position& toPos);
		void move(const Position& fromPos, const Position& toPos, Tile* oldTile, Tile* newTile, bool preMove = false);
		void stopMove();

		void addToDrawnTile(Tile* tile);
		void resetDrawnTile();
		void removeFromDrawnTile();

		void updateFrameGroup(Uint8 frameGroup);
		void update();
		void render(Sint32 posX, Sint32 posY);
		void renderInformations(Sint32 posX, Sint32 posY, Sint32 drawX, Sint32 drawY, float scale, bool visible);
		void renderOnBattle(Sint32 posX, Sint32 posY, bool renderManaBar);

		void addTimedSquare(Uint8 color);
		void removeTimedSquare();
		void showStaticSquare(Uint8 color);
		void hideStaticSquare();

		SDL_INLINE void addWalkTime(Uint32 walkTime) {m_walkEndTime += walkTime;}
		SDL_INLINE void setId(Uint32 cid) {m_id = cid;}
		void setName(const std::string name);
		void setHealth(Uint8 health);
		void turnDirection(Direction direction);
		void setOutfit(Uint16 lookType, Uint16 lookTypeEx, Uint8 lookHead, Uint8 lookBody, Uint8 lookLegs, Uint8 lookFeet, Uint8 lookAddons, Uint16 lookMount);
		void setSpeed(Uint16 speed);
		SDL_INLINE void setHelpers(Uint16 helpers) {m_helpers = helpers;}
		void setSkull(Uint8 skull);
		void setShield(Uint8 shield);
		SDL_INLINE void setUnpassable(bool unpassable) {m_unpassable = unpassable;}
		SDL_INLINE void setLight(Uint16 light[2]) {m_light[0] = light[0]; m_light[1] = light[1];}
		void setEmblem(Uint8 emblem);
		void setType(Uint8 type);
		void setIcon(Uint8 icon);
		void setVisible(bool visible);
		SDL_INLINE void setShowStatus(bool showStatus) {m_showStatus = showStatus;}
		SDL_INLINE void setManaPercent(Uint8 percent) {m_manaPercent = percent;}
		SDL_INLINE void setVocation(Uint8 vocation) {m_vocation = vocation;}
		SDL_INLINE void setNeedUpdate(bool needUpdate) {m_needUpdate = needUpdate;}
		SDL_INLINE void setLocalCreature(bool localCreature) {m_isLocalCreature = localCreature;}
		SDL_INLINE Uint32 getId() {return m_id;}
		SDL_INLINE std::string& getName() {return m_name;}
		SDL_INLINE Uint8 getHealth() {return m_health;}
		SDL_INLINE Uint16 getSpeed() {return m_speed;}
		SDL_INLINE Uint16 getHelpers() {return m_helpers;}
		SDL_INLINE Uint8 getSkull() {return m_skull;}
		SDL_INLINE Uint8 getShield() {return m_shield;}
		SDL_INLINE bool isUnpassable() {return m_unpassable;}
		SDL_INLINE Uint8 getEmblem() {return m_emblem;}
		SDL_INLINE Uint8 getType() {return m_type;}
		SDL_INLINE Uint8 getIcon() {return m_icon;}
		SDL_INLINE bool getShowStatus() {return m_showStatus;}
		SDL_INLINE Uint8 getManaPercent() {return m_manaPercent;}
		SDL_INLINE Uint8 getVocation() {return m_vocation;}
		SDL_INLINE Uint32 getVisibleTime() {return m_visibleTime;}
		SDL_INLINE bool isVisible() {return m_isVisible;}
		SDL_INLINE bool hasNeedUpdate() {return m_needUpdate;}
		SDL_INLINE bool isLocalCreature() {return m_isLocalCreature;}
		
		SDL_INLINE ThingType* getThingType() {return m_thingType;}
		SDL_INLINE ThingType* getMountType() {return m_mountType;}

		// Read-only accessors for the outfit fields. The ElfBot 8.60
		// compatibility layer needs these to populate offsets +96..+124
		// of the BattleList creature record (looktype + 4 color bytes +
		// addon + light radius + light color). m_outfit is packed as:
		//   bits  0..7  = head colour
		//   bits  8..15 = body colour
		//   bits 16..23 = legs colour
		//   bits 24..31 = feet colour
		SDL_INLINE Uint32 getOutfitPacked()    {return m_outfit;}
		SDL_INLINE Uint8  getOutfitHead()      {return  m_outfit        & 0xFF;}
		SDL_INLINE Uint8  getOutfitBody()      {return (m_outfit >>  8) & 0xFF;}
		SDL_INLINE Uint8  getOutfitLegs()      {return (m_outfit >> 16) & 0xFF;}
		SDL_INLINE Uint8  getOutfitFeet()      {return (m_outfit >> 24) & 0xFF;}
		SDL_INLINE Uint8  getLookAddons()      {return m_lookAddons;}
		SDL_INLINE Uint16 getLightRadius()     {return m_light[0];}
		SDL_INLINE Uint16 getLightColor()      {return m_light[1];}
		SDL_INLINE bool isWalkingCheck() {return (m_walking && (m_walkedPixels != 32 || m_preWalking));}
		SDL_INLINE bool isWalking() {return m_walking;}
		SDL_INLINE bool isPreWalking() {return m_preWalking;}
		SDL_INLINE Sint32 getWalkOffsetX() {return m_walkOffsetX;}
		SDL_INLINE Sint32 getWalkOffsetY() {return m_walkOffsetY;}
		SDL_INLINE Direction getDirection() {return m_direction;}
		SDL_INLINE Direction getWalkDirection() {return m_walkDirection;}

		std::pair<Sint32, Sint32> getDisplacement();
		Sint32 getOffsetX(bool checkPreWalk = false);
		Sint32 getOffsetY(bool checkPreWalk = false);
		Sint32 getStepDuration(Sint32 groundSpeed, bool ignoreDiagonal);

	protected:
		std::string m_name;
		ThingType* m_thingType = NULL;
		ThingType* m_mountType = NULL;
		Animator* m_outfitAnimator[ThingFrameGroup_Last] = {};
		Animator* m_mountAnimator[ThingFrameGroup_Last] = {};
		Animation m_outfitAnimation[ThingFrameGroup_Last];
		Animation m_mountAnimation[ThingFrameGroup_Last];

		Tile* m_drawnTile = NULL;

		Uint32 m_nameLen = 0;
		Uint32 m_updateTime = 0;
		Uint32 m_walkStartTime = 0;
		Uint32 m_walkEndTime = 0;
		Uint32 m_walkTime = 0;
		Uint32 m_timedSquareStartTime = 0;
		Uint32 m_shieldTime = 0;
		Uint32 m_visibleTime = 0;
		Uint32 m_cacheFormulatedSpeed = 0;

		Sint32 m_walkedPixels = 0;
		Sint32 m_walkOffsetX = 0;
		Sint32 m_walkOffsetY = 0;

		Sint32 m_shieldX = -1;
		Sint32 m_shieldY = -1;
		Sint32 m_skullX = -1;
		Sint32 m_skullY = -1;
		Sint32 m_typeX = -1;
		Sint32 m_typeY = -1;
		Sint32 m_iconX = -1;
		Sint32 m_iconY = -1;
		Sint32 m_emblemX = -1;
		Sint32 m_emblemY = -1;

		Uint32 m_id = 0;
		Uint32 m_outfit = 0x4C3A454E;

		Uint16 m_light[2] = {};
		Uint16 m_speed = 220;
		Uint16 m_helpers = 0;

		Uint8 m_lookAddons = 0;
		Uint8 m_health = 100;
		Uint8 m_skull = SKULL_NONE;
		Uint8 m_shield = SHIELD_NONE;
		Uint8 m_emblem = GUILDEMBLEM_NONE;
		Uint8 m_type = CREATURETYPE_NPC;
		Uint8 m_icon = CREATUREICON_NONE;

		Uint8 m_timedSquareRed = 0;
		Uint8 m_timedSquareGreen = 0;
		Uint8 m_timedSquareBlue = 0;
		Uint8 m_staticSquareRed = 0;
		Uint8 m_staticSquareGreen = 0;
		Uint8 m_staticSquareBlue = 0;
		Uint8 m_red = 0;
		Uint8 m_green = 188;
		Uint8 m_blue = 0;
		Uint8 m_manaPercent = 100;
		Uint8 m_vocation = 0;

		Direction m_direction = DIRECTION_SOUTH;
		Direction m_turnDirection = DIRECTION_SOUTH;
		Direction m_walkDirection = DIRECTION_SOUTH;

		Uint8 m_outfitAnim = 0;
		Uint8 m_mountAnim = 0;
		Uint8 m_currentFrame = ThingFrameGroup_Idle;
		Uint8 m_animationFrame = ThingFrameGroup_Idle;

		bool m_walking = false;
		bool m_preWalking = false;
		bool m_isLocalCreature = false;
		bool m_unpassable = true;
		bool m_showTimedSquare = false;
		bool m_showStaticSquare = false;
		bool m_showShield = false;
		bool m_showStatus = true;
		bool m_isVisible = false;
		bool m_needUpdate = false;
};

#endif /* __FILE_CREATURE_h_ */
