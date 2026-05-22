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

#include "creature.h"
#include "engine.h"
#include "light.h"
#include "map.h"
#include "tile.h"
#include "game.h"

double Creature::speedA = 857.36;
double Creature::speedB = 261.29;
double Creature::speedC = -4795.01;

extern Engine g_engine;
extern Map g_map;
extern ThingManager g_thingManager;
extern LightSystem g_light;
extern Game g_game;
extern Uint32 g_frameTime;

void Creature::preMove(const Position& fromPos, const Position& toPos)
{
	Tile* oldTile = g_map.getTile(fromPos);
	Tile* newTile = g_map.getTile(toPos);
	if(!oldTile || !newTile)
		return;

	move(fromPos, toPos, oldTile, newTile, true);
}

void Creature::move(const Position& fromPos, const Position& toPos, Tile* oldTile, Tile* newTile, bool preMove)
{
	if(!preMove)
	{
		if(!oldTile->removeThing(this, true))
			return;

		newTile->addThing(this, (g_clientVersion < 853), true);
	}
	if(fromPos.x > toPos.x)
	{
		m_direction = DIRECTION_WEST;
		m_turnDirection = DIRECTION_WEST;
		if(fromPos.y > toPos.y)
		{
			m_walkDirection = DIRECTION_NORTHWEST;
			m_walkOffsetX = 32;
			m_walkOffsetY = 32;
			addToDrawnTile(oldTile);
		}
		else if(fromPos.y < toPos.y)
		{
			m_walkDirection = DIRECTION_SOUTHWEST;

			Position newPos = Position(toPos.x + 1, toPos.y, toPos.z);
			Tile* newestTile = g_map.getTile(newPos);
			if(newestTile)
			{
				m_walkOffsetX = 32;
				m_walkOffsetY = 0;
				addToDrawnTile(newestTile);
			}
			else
			{
				//If somehow failed let's keep the "glitchy" movement
				m_walkOffsetX = 0;
				m_walkOffsetY = 0;
				addToDrawnTile(newTile);
			}
		}
		else
		{
			m_walkDirection = DIRECTION_WEST;
			m_walkOffsetX = 32;
			m_walkOffsetY = 0;
			addToDrawnTile(oldTile);
		}
	}
	else if(fromPos.x < toPos.x)
	{
		m_direction = DIRECTION_EAST;
		m_turnDirection = DIRECTION_EAST;
		if(fromPos.y > toPos.y)
		{
			m_walkDirection = DIRECTION_NORTHEAST;

			Position newPos = Position(fromPos.x + 1, fromPos.y, fromPos.z);
			Tile* newestTile = g_map.getTile(newPos);
			if(newestTile)
			{
				m_walkOffsetX = 0;
				m_walkOffsetY = 32;
				addToDrawnTile(newestTile);
			}
			else
			{
				//If somehow failed let's keep the "glitchy" movement
				m_walkOffsetX = -32;
				m_walkOffsetY = 32;
				addToDrawnTile(oldTile);
			}
		}
		else if(fromPos.y < toPos.y)
		{
			m_walkDirection = DIRECTION_SOUTHEAST;
			m_walkOffsetX = 0;
			m_walkOffsetY = 0;
			addToDrawnTile(newTile);
		}
		else
		{
			m_walkDirection = DIRECTION_EAST;
			m_walkOffsetX = 0;
			m_walkOffsetY = 0;
			addToDrawnTile(newTile);
		}
	}
	else if(fromPos.y > toPos.y)
	{
		m_direction = DIRECTION_NORTH;
		m_turnDirection = DIRECTION_NORTH;
		m_walkDirection = DIRECTION_NORTH;
		m_walkOffsetX = 0;
		m_walkOffsetY = 32;
		addToDrawnTile(oldTile);
	}
	else if(fromPos.y < toPos.y)
	{
		m_direction = DIRECTION_SOUTH;
		m_turnDirection = DIRECTION_SOUTH;
		m_walkDirection = DIRECTION_SOUTH;
		m_walkOffsetX = 0;
		m_walkOffsetY = 0;
		addToDrawnTile(newTile);
	}

	if(!Position::areInRange<1, 1, 0>(fromPos, toPos))
	{
		m_walking = false;
		m_preWalking = false;
		m_updateTime = 0;
		m_currentFrame = ThingFrameGroup_Idle;
		g_game.stopAutoWalk();
		return;
	}
	else if(!preMove)
	{
		if(m_preWalking)
		{
			m_preWalking = false;
			return;
		}
		else if(m_isLocalCreature)
			g_game.checkServerMovement(m_walkDirection);
	}

	Sint32 groundSpeed = SDL_static_cast(Sint32, newTile->getGroundSpeed());
	m_walking = true;
	m_preWalking = preMove;
	m_updateTime = 0;
	m_walkStartTime = g_frameTime;
	m_walkEndTime = g_frameTime + getStepDuration(groundSpeed, false);
	m_walkTime = getStepDuration(groundSpeed, true);
	m_walkedPixels = 0;
	if(!g_game.hasGameFeature(GAME_FEATURE_FRAMEGROUPS) || (m_thingType && m_thingType->hasFlag(ThingAttribute_AnimateAlways)))
		m_currentFrame = ThingFrameGroup_Idle;
	else
		m_currentFrame = ThingFrameGroup_Moving;
}

void Creature::stopMove()
{
	m_walking = false;
	m_preWalking = false;
	m_updateTime = 0;
	m_currentFrame = ThingFrameGroup_Idle;
	if(m_isLocalCreature)
		g_game.checkLocalCreatureMovement();
	
	if(m_turnDirection != m_direction)
		m_direction = m_turnDirection;
}

void Creature::addToDrawnTile(Tile* tile)
{
	removeFromDrawnTile();

	m_drawnTile = tile;
	m_drawnTile->addWalkCreature(this);
}

void Creature::resetDrawnTile()
{
	m_walking = false;
	m_preWalking = false;
	m_updateTime = 0;
	m_currentFrame = ThingFrameGroup_Idle;
	m_drawnTile = NULL;
}

void Creature::removeFromDrawnTile()
{
	if(m_drawnTile)
	{
		m_drawnTile->removeWalkCreature(this);
		m_drawnTile = NULL;
	}
}

void Creature::updateFrameGroup(Uint8 frameGroup)
{
	if(m_animationFrame != frameGroup)
	{
		m_animationFrame = frameGroup;
		if(m_outfitAnimator[frameGroup] && (frameGroup != ThingFrameGroup_Moving || m_outfitAnimation[frameGroup].m_lastPhaseTicks + CREATURE_ANIMATION_DELAY_RESET < g_frameTime))
			m_outfitAnimator[frameGroup]->resetAnimation(m_outfitAnimation[frameGroup], AnimationPhase_Invalid);

		if(m_mountAnimator[frameGroup] && (frameGroup != ThingFrameGroup_Moving || m_mountAnimation[frameGroup].m_lastPhaseTicks + CREATURE_ANIMATION_DELAY_RESET < g_frameTime))
			m_mountAnimator[frameGroup]->resetAnimation(m_mountAnimation[frameGroup], AnimationPhase_Invalid);
	}
}

void Creature::update()
{
	if(m_updateTime != g_frameTime)
	{
		m_updateTime = g_frameTime;
		if(m_walking)
		{
			float walkTicks = (m_walkTime * 0.03125f);
			m_walkedPixels = UTIL_min<Sint32>(SDL_static_cast(Sint32, (g_frameTime - m_walkStartTime) / walkTicks), 32);
			if(m_walkedPixels >= 32)
			{
				if(!m_preWalking || (g_frameTime - m_walkStartTime) >= 1000)//If we don't get the walk packet in 1s just end prewalking
				{
					if(g_frameTime >= m_walkEndTime)
						stopMove();
					else
						m_currentFrame = ThingFrameGroup_Idle;//Since we don't move keep idle animation
				}
				else
					m_currentFrame = ThingFrameGroup_Idle;//Since we don't move keep idle animation
			}
		}
		updateFrameGroup(m_currentFrame);
		if(m_thingType)
		{
			if(m_outfitAnimator[m_currentFrame])
			{
				//Calculate with new animation
				m_outfitAnim = SDL_static_cast(Uint8, m_outfitAnimator[m_currentFrame]->getPhase(m_outfitAnimation[m_currentFrame], (m_currentFrame == ThingFrameGroup_Idle ? 0 : m_walkTime)));
			}
			else
			{
				//Calculate with old animation
				if(m_thingType->m_category == ThingCategory_Effect)
				{
					Uint8 animCount = m_thingType->m_frameGroup[m_currentFrame].m_animCount;
					if(animCount > 2)
						m_outfitAnim = (SDL_static_cast(Uint8, (g_frameTime / ITEM_TICKS_PER_FRAME)) % (animCount - 2)) + 1;
					else
						m_outfitAnim = 0;
				}
				else if(m_thingType->m_category == ThingCategory_Item)
					m_outfitAnim = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / ITEM_TICKS_PER_FRAME)), m_thingType->m_frameGroup[m_currentFrame].m_animCount);
				else if(m_thingType->hasFlag(ThingAttribute_AnimateAlways))
					m_outfitAnim = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / CREATURE_TICKS_PER_FRAME)), m_thingType->m_frameGroup[m_currentFrame].m_animCount);
				else if(m_walking)
				{
					Uint8 animCount = m_thingType->m_frameGroup[m_currentFrame].m_animCount;
					if(animCount > 1 && m_walkedPixels < 32)
						m_outfitAnim = (SDL_static_cast(Uint8, m_walkedPixels / 8) % (animCount - 1)) + 1;
					else
						m_outfitAnim = 0;
				}
				else
					m_outfitAnim = 0;
			}
		}
		if(m_mountType)
		{
			if(m_mountAnimator[m_currentFrame])
			{
				//Calculate with new animation
				m_mountAnim = SDL_static_cast(Uint8, m_mountAnimator[m_currentFrame]->getPhase(m_mountAnimation[m_currentFrame], (m_currentFrame == ThingFrameGroup_Idle ? 0 : m_walkTime)));
			}
			else
			{
				//Calculate with old animation
				if(m_mountType->hasFlag(ThingAttribute_AnimateAlways))
					m_mountAnim = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / CREATURE_TICKS_PER_FRAME)), m_mountType->m_frameGroup[m_currentFrame].m_animCount);
				else if(m_walking)
				{
					Uint8 animCount = m_mountType->m_frameGroup[m_currentFrame].m_animCount;
					if(animCount > 1 && m_walkedPixels < 32)
						m_mountAnim = (SDL_static_cast(Uint8, m_walkedPixels / 8) % (animCount - 1)) + 1;
					else
						m_mountAnim = 0;
				}
				else
					m_mountAnim = 0;
			}
		}
	}
}

void Creature::render(Sint32 posX, Sint32 posY)
{
	if(!m_thingType)
		return;

	auto& renderer = g_engine.getRender();
	update();

	posX += getOffsetX();
	posY += getOffsetY();

	Uint8 zPattern = 0;
	if(m_thingType->m_category != ThingCategory_Creature)
	{
		if(m_thingType->m_category == ThingCategory_Effect)
		{
			posX -= 8;
			posY -= 8;
		}
	}
	else
	{
		if(m_mountType)
		{
			posX -= m_mountType->m_displacement[0];
			posY -= m_mountType->m_displacement[1];
		}
		else
		{
			posX -= m_thingType->m_displacement[0];
			posY -= m_thingType->m_displacement[1];
		}
	}

	if(g_engine.getLightMode() != CLIENT_LIGHT_MODE_NONE)
	{
		Uint16* light = m_thingType->m_light;
		if(m_light[0] > light[0])
			light = m_light;

		if(m_isLocalCreature && (g_light.getLightIntensity() < 64 || m_position.z > 7))
		{
			if(light[0] < 2)
				light[0] = 2;
			if(light[1] == 0)
				light[1] = 215;
		}

		if(light[0] > 0)
			g_light.addLightSource(posX, posY, light);
	}

	Sint32 squareX = posX;
	Sint32 squareY = posY;
	Sint32 squareW = 32;
	Sint32 squareH = 32;
	if(g_game.getAttackID() == m_id)
	{
		if(g_game.getSelectID() == m_id)
			renderer->drawRectangle(squareX, squareY, squareW, squareH, 2, 248, 164, 164, 255);
		else
			renderer->drawRectangle(squareX, squareY, squareW, squareH, 2, 224, 64, 64, 255);

		squareX += 2;
		squareY += 2;
		squareW -= 4;
		squareH -= 4;
	}
	else if(g_game.getFollowID() == m_id)
	{
		if(g_game.getSelectID() == m_id)
			renderer->drawRectangle(squareX, squareY, squareW, squareH, 2, 180, 248, 180, 255);
		else
			renderer->drawRectangle(squareX, squareY, squareW, squareH, 2, 64, 224, 64, 255);

		squareX += 2;
		squareY += 2;
		squareW -= 4;
		squareH -= 4;
	}
	else if(g_game.getSelectID() == m_id)
	{
		renderer->drawRectangle(squareX, squareY, squareW, squareH, 2, 248, 248, 248, 255);
		squareX += 2;
		squareY += 2;
		squareW -= 4;
		squareH -= 4;
	}
	if(m_showStaticSquare && g_engine.hasShowPvPFrames())
	{
		renderer->drawRectangle(squareX, squareY, squareW, squareH, 2, m_staticSquareRed, m_staticSquareGreen, m_staticSquareBlue, 255);
		squareX += 2;
		squareY += 2;
		squareW -= 4;
		squareH -= 4;
	}
	if(m_showTimedSquare)
	{
		if(g_frameTime - m_timedSquareStartTime >= 1000)
			removeTimedSquare();
		else
			renderer->drawRectangle(squareX, squareY, squareW, squareH, 2, m_timedSquareRed, m_timedSquareGreen, m_timedSquareBlue, 255);
	}

	if(m_mountType && m_thingType->m_category == ThingCategory_Creature)
	{
		Sint32 posYc = posY;
		for(Uint8 y = 0; y < m_mountType->m_frameGroup[m_currentFrame].m_height; ++y)
		{
			Sint32 posXc = posX;
			for(Uint8 x = 0; x < m_mountType->m_frameGroup[m_currentFrame].m_width; ++x)
			{
				Uint32 sprite = m_mountType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 0, m_direction, 0, 0, m_mountAnim);
				if(sprite != 0)
					renderer->drawSprite(sprite, posXc, posYc);
				posXc -= 32;
			}
			posYc -= 32;
		}
		zPattern = UTIL_min<Uint8>(1, m_thingType->m_frameGroup[m_currentFrame].m_patternZ - 1);
	}

	if(m_thingType->m_frameGroup[m_currentFrame].m_layers > 1)
	{
		for(Uint8 y = 0; y < m_thingType->m_frameGroup[m_currentFrame].m_height; ++y)
		{
			Sint32 posXc = posX;
			for(Uint8 x = 0; x < m_thingType->m_frameGroup[m_currentFrame].m_width; ++x)
			{
				Uint32 sprite = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 0, m_direction, 0, zPattern, m_outfitAnim);
				Uint32 spriteMask = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 1, m_direction, 0, zPattern, m_outfitAnim);
				if(sprite != 0)
				{
					if(spriteMask != 0)
						renderer->drawSpriteMask(sprite, spriteMask, posXc, posY, m_outfit);
					else
						renderer->drawSprite(sprite, posXc, posY);
				}
				if(m_thingType->m_frameGroup[m_currentFrame].m_patternY > 1)
				{
					if(m_lookAddons & 1)//First addon
					{
						sprite = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 0, m_direction, 1, zPattern, m_outfitAnim);
						spriteMask = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 1, m_direction, 1, zPattern, m_outfitAnim);
						if(sprite != 0)
						{
							if(spriteMask != 0)
								renderer->drawSpriteMask(sprite, spriteMask, posXc, posY, m_outfit);
							else
								renderer->drawSprite(sprite, posXc, posY);
						}
					}
					if(m_lookAddons & 2)//Second addon
					{
						sprite = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 0, m_direction, 2, zPattern, m_outfitAnim);
						spriteMask = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 1, m_direction, 2, zPattern, m_outfitAnim);
						if(sprite != 0)
						{
							if(spriteMask != 0)
								renderer->drawSpriteMask(sprite, spriteMask, posXc, posY, m_outfit);
							else
								renderer->drawSprite(sprite, posXc, posY);
						}
					}
				}
				posXc -= 32;
			}
			posY -= 32;
		}
	}
	else
	{
		for(Uint8 y = 0; y < m_thingType->m_frameGroup[m_currentFrame].m_height; ++y)
		{
			Sint32 posXc = posX;
			for(Uint8 x = 0; x < m_thingType->m_frameGroup[m_currentFrame].m_width; ++x)
			{
				Uint32 sprite = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 0, m_direction, 0, zPattern, m_outfitAnim);
				if(sprite != 0)
					renderer->drawSprite(sprite, posXc, posY);
				if(m_thingType->m_frameGroup[m_currentFrame].m_patternY > 1)
				{
					if(m_lookAddons & 1)//First addon
					{
						sprite = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 0, m_direction, 1, zPattern, m_outfitAnim);
						if(sprite != 0)
							renderer->drawSprite(sprite, posXc, posY);
					}
					if(m_lookAddons & 2)//Second addon
					{
						sprite = m_thingType->getSprite(SDL_static_cast(ThingFrameGroup, m_currentFrame), x, y, 0, m_direction, 2, zPattern, m_outfitAnim);
						if(sprite != 0)
							renderer->drawSprite(sprite, posXc, posY);
					}
				}
				posXc -= 32;
			}
			posY -= 32;
		}
	}
}

void Creature::renderInformations(Sint32 posX, Sint32 posY, Sint32 drawX, Sint32 drawY, float scale, bool visible)
{
	setNeedUpdate(true);
	if(m_health < 1)
		return;

	Uint8 red = m_red, green = m_green, blue = m_blue;
	if(!visible)
	{
		red = 191;
		green = 191;
		blue = 191;
	}

	drawX += getOffsetX();
	drawY += getOffsetY();
	if(m_thingType)
	{
		if(m_thingType->m_category != ThingCategory_Creature)
		{
			if(m_thingType->m_category == ThingCategory_Effect)
			{
				drawX -= 8;
				drawY -= 8;
			}
		}
		else
		{
			if(m_mountType)
			{
				drawX -= SDL_static_cast(Sint32, m_mountType->m_displacement[0]);
				drawY -= SDL_static_cast(Sint32, m_mountType->m_displacement[1]);
			}
			else
			{
				drawX -= SDL_static_cast(Sint32, m_thingType->m_displacement[0]);
				drawY -= SDL_static_cast(Sint32, m_thingType->m_displacement[1]);
			}
		}
	}
	posX += SDL_static_cast(Sint32, drawX * scale);
	posY += SDL_static_cast(Sint32, drawY * scale) - 16;
	g_engine.drawFont(CLIENT_FONT_OUTLINED, posX - m_nameLen, posY, m_name, red, green, blue, CLIENT_FONT_ALIGN_LEFT);

	auto& renderer = g_engine.getRender();
	renderer->fillRectangle(posX - 14, posY + 12, 28, 4, 0, 0, 0, 255);
	renderer->fillRectangle(posX - 13, posY + 13, SDL_static_cast(Sint32, m_health * 0.26f), 2, red, green, blue, 255);

	Sint32 POSX = posX + 9;
	Sint32 POSY = posY + 18;
	if(g_engine.hasShowMarks())
	{
		if(m_shield != SHIELD_NONE)
		{
			if(m_shield == SHIELD_BLUE_NOSHAREDEXP_BLINK || m_shield == SHIELD_YELLOW_NOSHAREDEXP_BLINK)
			{
				if(g_frameTime - m_shieldTime >= CREATURE_SHIELD_BLINK_TICKS)
				{
					m_shieldTime = g_frameTime;
					m_showShield = !m_showShield;
				}
				if(m_showShield)
					renderer->drawPicture(GUI_UI_IMAGE, m_shieldX, m_shieldY, POSX, POSY, GUI_UI_ICON_SHIELD_BLUE_W, GUI_UI_ICON_SHIELD_BLUE_H);
			}
			else
				renderer->drawPicture(GUI_UI_IMAGE, m_shieldX, m_shieldY, POSX, POSY, GUI_UI_ICON_SHIELD_BLUE_W, GUI_UI_ICON_SHIELD_BLUE_H);

			POSX += 13;
		}

		if(m_emblem != GUILDEMBLEM_NONE)
			renderer->drawPicture(GUI_UI_IMAGE, m_emblemX, m_emblemY, POSX, POSY + 13, GUI_UI_ICON_GUILDWAR_ALLY_W, GUI_UI_ICON_GUILDWAR_ALLY_H);

		if(m_skull != SKULL_NONE)
		{
			renderer->drawPicture(GUI_UI_IMAGE, m_skullX, m_skullY, POSX, POSY, GUI_UI_ICON_YELLOWSKULL_W, GUI_UI_ICON_YELLOWSKULL_H);
			POSX += 13;
		}
	}

	if(m_type == CREATURETYPE_SUMMON_OWN || m_type == CREATURETYPE_SUMMON_OTHERS)
	{
		renderer->drawPicture(GUI_UI_IMAGE, m_typeX, m_typeY, POSX, POSY, GUI_UI_ICON_SUMMONOWN_W, GUI_UI_ICON_SUMMONOWN_H);
		POSX += 13;
	}

	if(m_icon != CREATUREICON_NONE && g_engine.hasShowIcons())
	{
		renderer->drawPicture(GUI_UI_IMAGE, m_iconX, m_iconY, POSX, POSY, GUI_UI_ICON_BUBBLE_SPEECH_W, GUI_UI_ICON_BUBBLE_SPEECH_H);
		POSX += 20;
	}
}

void Creature::renderOnBattle(Sint32 posX, Sint32 posY, bool renderManaBar)
{
	auto& renderer = g_engine.getRender();
	if(m_thingType)
	{
		Uint8 animation;
		if(m_outfitAnimator[ThingFrameGroup_Idle])
		{
			//Calculate with new animation
			animation = SDL_static_cast(Uint8, m_outfitAnimator[ThingFrameGroup_Idle]->getPhase(m_outfitAnimation[ThingFrameGroup_Idle]));
		}
		else
		{
			//Calculate with old animation
			if(m_thingType->m_category == ThingCategory_Effect)
			{
				Uint8 animCount = m_thingType->m_frameGroup[ThingFrameGroup_Idle].m_animCount;
				if(animCount > 2)
					animation = (SDL_static_cast(Uint8, (g_frameTime / ITEM_TICKS_PER_FRAME)) % (animCount - 2)) + 1;
				else
					animation = 0;
			}
			else if(m_thingType->m_category == ThingCategory_Item)
				animation = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / ITEM_TICKS_PER_FRAME)), m_thingType->m_frameGroup[ThingFrameGroup_Idle].m_animCount);
			else if(m_thingType->hasFlag(ThingAttribute_AnimateAlways))
				animation = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / CREATURE_TICKS_PER_FRAME)), m_thingType->m_frameGroup[ThingFrameGroup_Idle].m_animCount);
			else
				animation = 0;
		}
		g_engine.drawOutfit(m_thingType, posX, posY, 20, DIRECTION_SOUTH, 0, 0, animation, m_outfit);
	}

	Uint8 red, green, blue;
	if(m_isVisible)
	{
		red = 192;
		green = 192;
		blue = 192;
	}
	else
	{
		red = 128;
		green = 128;
		blue = 128;
	}

	Sint32 squareX = posX;
	Sint32 squareY = posY;
	Sint32 squareW = 20;
	Sint32 squareH = 20;
	if(g_game.getAttackID() == m_id)
	{
		if(g_game.getSelectID() == m_id)
		{
			red = 248;
			green = 164;
			blue = 164;
			renderer->drawRectangle(squareX, squareY, squareW, squareH, 1, red, green, blue, 255);
		}
		else
		{
			red = 224;
			green = 64;
			blue = 64;
			renderer->drawRectangle(squareX, squareY, squareW, squareH, 1, red, green, blue, 255);
		}

		squareX += 1;
		squareY += 1;
		squareW -= 2;
		squareH -= 2;
	}
	else if(g_game.getFollowID() == m_id)
	{
		if(g_game.getSelectID() == m_id)
		{
			red = 180;
			green = 248;
			blue = 180;
			renderer->drawRectangle(squareX, squareY, squareW, squareH, 1, red, green, blue, 255);
		}
		else
		{
			red = 64;
			green = 224;
			blue = 64;
			renderer->drawRectangle(squareX, squareY, squareW, squareH, 1, red, green, blue, 255);
		}

		squareX += 1;
		squareY += 1;
		squareW -= 2;
		squareH -= 2;
	}
	else if(g_game.getSelectID() == m_id)
	{
		red = 248;
		green = 248;
		blue = 248;
		renderer->drawRectangle(squareX, squareY, squareW, squareH, 1, red, green, blue, 255);
		squareX += 1;
		squareY += 1;
		squareW -= 2;
		squareH -= 2;
	}
	if(m_showStaticSquare && g_engine.hasShowPvPFrames())
	{
		renderer->drawRectangle(squareX, squareY, squareW, squareH, 1, m_staticSquareRed, m_staticSquareGreen, m_staticSquareBlue, 255);
		squareX += 1;
		squareY += 1;
		squareW -= 2;
		squareH -= 2;
	}
	if(m_showTimedSquare)
		renderer->drawRectangle(squareX, squareY, squareW, squareH, 1, m_timedSquareRed, m_timedSquareGreen, m_timedSquareBlue, 255);

	posX += 22;
	posY += 4;
	g_engine.drawFont(CLIENT_FONT_NONOUTLINED, posX, posY, m_name, red, green, blue, CLIENT_FONT_ALIGN_LEFT);
	if(m_showStatus)
	{
		renderer->drawRectangle(posX - 1, posY + 14, 132, 5, 1, 0, 0, 0, 255);
		renderer->fillRectangle(posX, posY + 15, SDL_static_cast(Sint32, m_health) * 130 / 100, 3, m_red, m_green, m_blue, 255);
		if(renderManaBar)
		{
			renderer->drawRectangle(posX - 1, posY + 20, 132, 5, 1, 0, 0, 0, 255);
			renderer->fillRectangle(posX, posY + 21, SDL_static_cast(Sint32, m_manaPercent) * 130 / 100, 3, 0, 0, 255, 255);
		}
	}
	else
	{
		renderer->drawRectangle(posX - 1, posY + 14, 132, 5, 1, 0, 0, 0, 255);
		renderer->fillRectangle(posX, posY + 15, 130, 3, 112, 112, 112, 255);
		if(renderManaBar)
		{
			renderer->drawRectangle(posX - 1, posY + 20, 132, 5, 1, 0, 0, 0, 255);
			renderer->fillRectangle(posX, posY + 21, 130, 3, 112, 112, 112, 255);
		}
	}

	posX += 133;
	if(m_skull != SKULL_NONE)
	{
		posX -= 13;
		renderer->drawPicture(GUI_UI_IMAGE, m_skullX, m_skullY, posX, posY, GUI_UI_ICON_YELLOWSKULL_W, GUI_UI_ICON_YELLOWSKULL_H);
	}

	if(m_shield != SHIELD_NONE)
	{
		posX -= 13;
		if(m_shield == SHIELD_BLUE_NOSHAREDEXP_BLINK || m_shield == SHIELD_YELLOW_NOSHAREDEXP_BLINK)
		{
			if(g_frameTime - m_shieldTime >= CREATURE_SHIELD_BLINK_TICKS)
			{
				m_shieldTime = g_frameTime;
				m_showShield = !m_showShield;
			}
			if(m_showShield)
				renderer->drawPicture(GUI_UI_IMAGE, m_shieldX, m_shieldY, posX, posY, GUI_UI_ICON_SHIELD_BLUE_W, GUI_UI_ICON_SHIELD_BLUE_H);
		}
		else
			renderer->drawPicture(GUI_UI_IMAGE, m_shieldX, m_shieldY, posX, posY, GUI_UI_ICON_SHIELD_BLUE_W, GUI_UI_ICON_SHIELD_BLUE_H);
	}

	if(m_emblem != GUILDEMBLEM_NONE)
	{
		posX -= 13;
		renderer->drawPicture(GUI_UI_IMAGE, m_emblemX, m_emblemY, posX, posY, GUI_UI_ICON_GUILDWAR_ALLY_W, GUI_UI_ICON_GUILDWAR_ALLY_H);
	}

	if(m_type == CREATURETYPE_SUMMON_OWN || m_type == CREATURETYPE_SUMMON_OTHERS)
	{
		posX -= 13;
		renderer->drawPicture(GUI_UI_IMAGE, m_typeX, m_typeY, posX, posY, GUI_UI_ICON_SUMMONOWN_W, GUI_UI_ICON_SUMMONOWN_H);
	}
}

void Creature::addTimedSquare(Uint8 color)
{
	m_timedSquareStartTime = g_frameTime;
	colorFrom8bit(color, m_timedSquareRed, m_timedSquareGreen, m_timedSquareBlue);
	m_showTimedSquare = true;
}

void Creature::removeTimedSquare()
{
	m_showTimedSquare = false;
}

void Creature::showStaticSquare(Uint8 color)
{
	colorFrom8bit(color, m_staticSquareRed, m_staticSquareGreen, m_staticSquareBlue);
	m_showStaticSquare = true;
}

void Creature::hideStaticSquare()
{
	m_showStaticSquare = false;
}

void Creature::setName(const std::string name)
{
	m_name = std::move(name);
	m_nameLen = g_engine.calculateFontWidth(CLIENT_FONT_OUTLINED, m_name) / 2;
}

void Creature::setHealth(Uint8 health)
{
	m_health = health;
	if(m_health < 4)
	{
		m_red = 96;
		m_green = 0;
		m_blue = 0;
	}
	else if(m_health < 10)
	{
		m_red = 192;
		m_green = 0;
		m_blue = 0;
	}
	else if(m_health < 30)
	{
		m_red = 192;
		m_green = 48;
		m_blue = 48;
	}
	else if(m_health < 60)
	{
		m_red = 192;
		m_green = 192;
		m_blue = 0;
	}
	else if(m_health < 95)
	{
		m_red = 96;
		m_green = 192;
		m_blue = 96;
	}
	else
	{
		m_red = 0;
		m_green = 192;
		m_blue = 0;
	}
}

void Creature::turnDirection(Direction direction)
{
	if(m_walking)
		m_turnDirection = direction;//Schedule turn direction after walk complete
	else
	{
		m_turnDirection = direction;
		m_direction = direction;//Change our direction right away if we not walking
	}
}

void Creature::setOutfit(Uint16 lookType, Uint16 lookTypeEx, Uint8 lookHead, Uint8 lookBody, Uint8 lookLegs, Uint8 lookFeet, Uint8 lookAddons, Uint16 lookMount)
{
	if(lookType == 0)
	{
		if(lookTypeEx == 0)
			m_thingType = g_thingManager.getThingType(ThingCategory_Effect, 13);
		else
			m_thingType = g_thingManager.getThingType(ThingCategory_Item, lookTypeEx);
	}
	else
		m_thingType = g_thingManager.getThingType(ThingCategory_Creature, lookType);

	if(lookMount == 0)
		m_mountType = NULL;
	else
		m_mountType = g_thingManager.getThingType(ThingCategory_Creature, lookMount);

	for(Sint32 i = ThingFrameGroup_Default; i < ThingFrameGroup_Last; ++i)
	{
		m_outfitAnimator[i] = (m_thingType ? m_thingType->m_frameGroup[i].m_animator : NULL);
		if(m_outfitAnimator[i])
			m_outfitAnimator[i]->resetAnimation(m_outfitAnimation[i], AnimationPhase_Invalid);

		m_mountAnimator[i] = (m_mountType ? m_mountType->m_frameGroup[i].m_animator : NULL);
		if(m_mountAnimator[i])
			m_mountAnimator[i]->resetAnimation(m_mountAnimation[i], AnimationPhase_Invalid);
	}

	m_outfit = (lookFeet << 24) | (lookLegs << 16) | (lookBody << 8) | (lookHead);
	m_lookAddons = lookAddons;
}

void Creature::setSpeed(Uint16 speed)
{
	m_speed = speed;
	if(g_game.hasGameFeature(GAME_FEATURE_NEWSPEED_LAW))
	{
		Sint32 stepSpeed = SDL_static_cast(Sint32, m_speed);
		stepSpeed = (stepSpeed < 1 ? 1 : stepSpeed);//Make sure we don't get div by 0 exception

		double formulatedSpeed = 1.0;
		if(stepSpeed > -Creature::speedB)
		{
			formulatedSpeed = SDL_floor((Creature::speedA * SDL_log(stepSpeed + Creature::speedB) + Creature::speedC) + 0.5);
			if(formulatedSpeed < 1.0)
				formulatedSpeed = 1.0;
		}
		m_cacheFormulatedSpeed = SDL_static_cast(Uint32, formulatedSpeed);
	}
}

void Creature::setSkull(Uint8 skull)
{
	m_skull = skull;
	switch(skull)
	{
		case SKULL_YELLOW:
		{
			m_skullX = GUI_UI_ICON_YELLOWSKULL_X;
			m_skullY = GUI_UI_ICON_YELLOWSKULL_Y;
		}
		break;
		case SKULL_GREEN:
		{
			m_skullX = GUI_UI_ICON_GREENSKULL_X;
			m_skullY = GUI_UI_ICON_GREENSKULL_Y;
		}
		break;
		case SKULL_WHITE:
		{
			m_skullX = GUI_UI_ICON_WHITESKULL_X;
			m_skullY = GUI_UI_ICON_WHITESKULL_Y;
		}
		break;
		case SKULL_RED:
		{
			m_skullX = GUI_UI_ICON_REDSKULL_X;
			m_skullY = GUI_UI_ICON_REDSKULL_Y;
		}
		break;
		case SKULL_BLACK:
		{
			m_skullX = GUI_UI_ICON_BLACKSKULL_X;
			m_skullY = GUI_UI_ICON_BLACKSKULL_Y;
		}
		break;
		case SKULL_ORANGE:
		{
			m_skullX = GUI_UI_ICON_ORANGOSKULL_X;
			m_skullY = GUI_UI_ICON_ORANGOSKULL_Y;
		}
		break;
		default: m_skull = SKULL_NONE; break;
	}
}

void Creature::setShield(Uint8 shield)
{
	if(m_shield != shield)
		UTIL_refreshPartyWindow();

	m_shield = shield;
	switch(shield)
	{
		case SHIELD_WHITEYELLOW:
		{
			m_shieldX = GUI_UI_ICON_SHIELD_WHITEYELLOW_X;
			m_shieldY = GUI_UI_ICON_SHIELD_WHITEYELLOW_Y;
		}
		break;
		case SHIELD_WHITEBLUE:
		{
			m_shieldX = GUI_UI_ICON_SHIELD_WHITEBLUE_X;
			m_shieldY = GUI_UI_ICON_SHIELD_WHITEBLUE_Y;
		}
		break;
		case SHIELD_BLUE:
		{
			m_shieldX = GUI_UI_ICON_SHIELD_BLUE_X;
			m_shieldY = GUI_UI_ICON_SHIELD_BLUE_Y;
		}
		break;
		case SHIELD_YELLOW:
		{
			m_shieldX = GUI_UI_ICON_SHIELD_YELLOW_X;
			m_shieldY = GUI_UI_ICON_SHIELD_YELLOW_Y;
		}
		break;
		case SHIELD_BLUE_SHAREDEXP:
		{
			m_shieldX = GUI_UI_ICON_SHIELD_BLUE_SHAREDEXP_X;
			m_shieldY = GUI_UI_ICON_SHIELD_BLUE_SHAREDEXP_Y;
		}
		break;
		case SHIELD_YELLOW_SHAREDEXP:
		{
			m_shieldX = GUI_UI_ICON_SHIELD_YELLOW_SHAREDEXP_X;
			m_shieldY = GUI_UI_ICON_SHIELD_YELLOW_SHAREDEXP_Y;
		}
		break;
		case SHIELD_BLUE_NOSHAREDEXP_BLINK:
		{
			m_shieldX = GUI_UI_ICON_SHIELD_BLUE_NOSHAREDEXP_X;
			m_shieldY = GUI_UI_ICON_SHIELD_BLUE_NOSHAREDEXP_Y;
			m_shieldTime = g_frameTime;
			m_showShield = true;
		}
		break;
		case SHIELD_YELLOW_NOSHAREDEXP_BLINK:
		{
			m_shieldX = GUI_UI_ICON_SHIELD_YELLOW_NOSHAREDEXP_X;
			m_shieldY = GUI_UI_ICON_SHIELD_YELLOW_NOSHAREDEXP_Y;
			m_shieldTime = g_frameTime;
			m_showShield = true;
		}
		break;
		case SHIELD_BLUE_NOSHAREDEXP:
		{
			m_shieldX = GUI_UI_ICON_SHIELD_BLUE_NOSHAREDEXP_X;
			m_shieldY = GUI_UI_ICON_SHIELD_BLUE_NOSHAREDEXP_Y;
		}
		break;
		case SHIELD_YELLOW_NOSHAREDEXP:
		{
			m_shieldX = GUI_UI_ICON_SHIELD_YELLOW_NOSHAREDEXP_X;
			m_shieldY = GUI_UI_ICON_SHIELD_YELLOW_NOSHAREDEXP_Y;
		}
		break;
		case SHIELD_GRAY:
		{
			m_shieldX = GUI_UI_ICON_SHIELD_GRAY_X;
			m_shieldY = GUI_UI_ICON_SHIELD_GRAY_Y;
		}
		break;
		default: m_shield = SHIELD_NONE; break;
	}
}

void Creature::setEmblem(Uint8 emblem)
{
	m_emblem = emblem;
	switch(emblem)
	{
		case GUILDEMBLEM_ALLY:
		{
			m_emblemX = GUI_UI_ICON_GUILDWAR_ALLY_X;
			m_emblemY = GUI_UI_ICON_GUILDWAR_ALLY_Y;
		}
		break;
		case GUILDEMBLEM_ENEMY:
		{
			m_emblemX = GUI_UI_ICON_GUILDWAR_ENEMY_X;
			m_emblemY = GUI_UI_ICON_GUILDWAR_ENEMY_Y;
		}
		break;
		case GUILDEMBLEM_NEUTRAL:
		{
			m_emblemX = GUI_UI_ICON_GUILDWAR_NEUTRAL_X;
			m_emblemY = GUI_UI_ICON_GUILDWAR_NEUTRAL_Y;
		}
		break;
		case GUILDEMBLEM_MEMBER:
		{
			m_emblemX = GUI_UI_ICON_GUILDWAR_MEMBER_X;
			m_emblemY = GUI_UI_ICON_GUILDWAR_MEMBER_Y;
		}
		break;
		case GUILDEMBLEM_OTHER:
		{
			m_emblemX = GUI_UI_ICON_GUILDWAR_OTHER_X;
			m_emblemY = GUI_UI_ICON_GUILDWAR_OTHER_Y;
		}
		break;
		default: m_emblem = GUILDEMBLEM_NONE; break;
	}
}

void Creature::setType(Uint8 type)
{
	m_type = type;
	switch(type)
	{
		case CREATURETYPE_SUMMON_OWN:
		{
			m_typeX = GUI_UI_ICON_SUMMONOWN_X;
			m_typeY = GUI_UI_ICON_SUMMONOWN_Y;
		}
		break;
		case CREATURETYPE_SUMMON_OTHERS:
		{
			m_typeX = GUI_UI_ICON_SUMMONOTHERS_X;
			m_typeY = GUI_UI_ICON_SUMMONOTHERS_Y;
		}
		break;
		default: break;
	}
}

void Creature::setIcon(Uint8 icon)
{
	m_icon = icon;
	switch(icon)
	{
		case CREATUREICON_SPEECH:
		{
			m_iconX = GUI_UI_ICON_BUBBLE_SPEECH_X;
			m_iconY = GUI_UI_ICON_BUBBLE_SPEECH_Y;
		}
		break;
		case CREATUREICON_TRADE:
		{
			m_iconX = GUI_UI_ICON_BUBBLE_TRADE_X;
			m_iconY = GUI_UI_ICON_BUBBLE_TRADE_Y;
		}
		break;
		case CREATUREICON_QUEST:
		{
			m_iconX = GUI_UI_ICON_BUBBLE_QUEST_X;
			m_iconY = GUI_UI_ICON_BUBBLE_QUEST_Y;
		}
		break;
		case CREATUREICON_QUESTTRADER:
		{
			m_iconX = GUI_UI_ICON_BUBBLE_QUESTTRADER_X;
			m_iconY = GUI_UI_ICON_BUBBLE_QUESTTRADER_Y;
		}
		break;
		case CREATUREICON_SPEECH1:
		{
			m_iconX = GUI_UI_ICON_BUBBLE_SPEECH_X;
			m_iconY = GUI_UI_ICON_BUBBLE_SPEECH_Y;
		}
		break;
		case CREATUREICON_SPEECH2:
		{
			m_iconX = GUI_UI_ICON_BUBBLE_SPEECH_X;
			m_iconY = GUI_UI_ICON_BUBBLE_SPEECH_Y;
		}
		break;
		case CREATUREICON_SPEECH3:
		{
			m_iconX = GUI_UI_ICON_BUBBLE_SPEECH2_X;
			m_iconY = GUI_UI_ICON_BUBBLE_SPEECH2_Y;
		}
		break;
		default: m_icon = CREATUREICON_NONE; break;
	}
}

void Creature::setVisible(bool visible)
{
	m_isVisible = visible;
	if(m_isVisible)
		m_visibleTime = g_frameTime;
}

std::pair<Sint32, Sint32> Creature::getDisplacement()
{
	if(!m_thingType)
		return std::make_pair(0, 0);

	if(m_thingType->m_category != ThingCategory_Creature)
	{
		if(m_thingType->m_category == ThingCategory_Effect)
			return std::make_pair(8, 8);
		else
			return std::make_pair(0, 0);
	}
	else
	{
		if(m_mountType)
			return std::make_pair(SDL_static_cast(Sint32, m_mountType->m_displacement[0]), SDL_static_cast(Sint32, m_mountType->m_displacement[1]));
		else
			return std::make_pair(SDL_static_cast(Sint32, m_thingType->m_displacement[0]), SDL_static_cast(Sint32, m_thingType->m_displacement[1]));
	}
}

Sint32 Creature::getOffsetX(bool checkPreWalk)
{
	if(!m_walking) return 0;
	switch(m_walkDirection)
	{
		case DIRECTION_NORTH: return 0;
		case DIRECTION_EAST: return m_walkedPixels - ((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_SOUTH: return 0;
		case DIRECTION_WEST: return -m_walkedPixels + ((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_SOUTHWEST: return -m_walkedPixels + ((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_SOUTHEAST: return m_walkedPixels - ((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_NORTHWEST: return -m_walkedPixels + ((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_NORTHEAST: return m_walkedPixels - ((checkPreWalk && m_preWalking) ? 0 : 32);
		default: return 0;
	}
}

Sint32 Creature::getOffsetY(bool checkPreWalk)
{
	if(!m_walking) return 0;
	switch(m_walkDirection)
	{
		case DIRECTION_NORTH: return -m_walkedPixels + ((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_EAST: return 0;
		case DIRECTION_SOUTH: return m_walkedPixels - ((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_WEST: return 0;
		case DIRECTION_SOUTHWEST: return m_walkedPixels - ((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_SOUTHEAST: return m_walkedPixels - ((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_NORTHWEST: return -m_walkedPixels + ((checkPreWalk && m_preWalking) ? 0 : 32);
		case DIRECTION_NORTHEAST: return -m_walkedPixels + ((checkPreWalk && m_preWalking) ? 0 : 32);
		default: return 0;
	}
}

Sint32 Creature::getStepDuration(Sint32 groundSpeed, bool ignoreDiagonal)
{
	Sint32 speed = SDL_static_cast(Sint32, m_speed);
	speed = (speed < 1 ? 1 : speed);//Make sure we don't get div by 0 exception
	groundSpeed = (groundSpeed < 1 ? 150 : groundSpeed);

	Sint32 interval = 1000 * groundSpeed;
	if(g_game.hasGameFeature(GAME_FEATURE_NEWSPEED_LAW))
		interval /= (m_cacheFormulatedSpeed > 0 ? m_cacheFormulatedSpeed : 1);
	else
		interval /= speed;

	Sint32 serverBeat = SDL_static_cast(Sint32, g_game.getServerBeat());
	serverBeat = (serverBeat < 1 ? 1 : serverBeat);
	interval = ((interval + serverBeat - 1) / serverBeat) * serverBeat;
	if(!ignoreDiagonal && (m_walkDirection & DIRECTION_DIAGONAL_MASK) != 0)
		interval *= 3;

	return interval;
}
