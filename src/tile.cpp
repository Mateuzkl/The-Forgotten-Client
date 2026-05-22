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

#include "tile.h"
#include "item.h"
#include "creature.h"
#include "effect.h"
#include "thingManager.h"

extern std::vector<Effect*> g_effects;

Tile::~Tile()
{
	for(std::vector<Creature*>::iterator it = m_walkCreatures.begin(), end = m_walkCreatures.end(); it != end; ++it)
		(*it)->resetDrawnTile();
	
	m_walkCreatures.clear();
	reset();
}

void Tile::reset()
{
	for(std::vector<Effect*>::iterator it = m_effects.begin(), end = m_effects.end(); it != end; ++it)
		delete (*it);

	for(std::vector<Item*>::iterator it = m_topItems.begin(), end = m_topItems.end(); it != end; ++it)
		delete (*it);

	for(std::vector<Item*>::iterator it = m_downItems.begin(), end = m_downItems.end(); it != end; ++it)
		delete (*it);

	if(m_ground)
	{
		delete m_ground;
		m_ground = NULL;
	}

	m_topItems.clear();
	m_downItems.clear();
	m_creatures.clear();
	m_effects.clear();
}

void Tile::render(Sint32 posX, Sint32 posY, bool visible_tile)
{
	Sint32 displacedItems = 0;
	m_tileElevation = 0;
	if(m_ground)
	{
		if(visible_tile)
			m_ground->render(posX, posY, visible_tile);

		if(m_ground->hasElevation() && m_tileElevation < ITEM_MAX_ELEVATION)
		{
			m_tileElevation = UTIL_min<Sint32>(ITEM_MAX_ELEVATION, m_tileElevation + m_ground->getElevation());
			visible_tile = true; //Make sure we get drawing everything
		}
	}

	for(std::vector<Item*>::iterator it = m_topItems.begin(), end = m_topItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getTopOrder() != 3)
		{
			item->render(posX - m_tileElevation, posY - m_tileElevation, visible_tile);
			if(item->hasElevation() && displacedItems < ITEM_MAX_ELEVATION)
			{
				m_tileElevation = UTIL_min<Sint32>(ITEM_MAX_ELEVATION, m_tileElevation + item->getElevation());
				visible_tile = true; //Make sure we get drawing everything
			}
		}
	}

	for(std::vector<Item*>::reverse_iterator it = m_downItems.rbegin(), end = m_downItems.rend(); it != end; ++it)
	{
		Item* item = (*it);
		item->render(posX - m_tileElevation, posY - m_tileElevation, visible_tile);
		if(item->hasElevation() && displacedItems < ITEM_MAX_ELEVATION)
		{
			m_tileElevation = UTIL_min<Sint32>(ITEM_MAX_ELEVATION, m_tileElevation + item->getElevation());
			visible_tile = true; //Make sure we get drawing everything
		}
	}

	posX -= m_tileElevation;
	posY -= m_tileElevation;
	for(std::vector<Creature*>::reverse_iterator it = m_creatures.rbegin(), end = m_creatures.rend(); it != end; ++it)
	{
		Creature* creature = (*it);
		if(!creature->isWalkingCheck())
			creature->render(posX, posY);
	}

	for(std::vector<Creature*>::iterator it = m_walkCreatures.begin(), end = m_walkCreatures.end(); it != end; ++it)
	{
		Creature* creature = (*it);
		if(creature->isWalkingCheck())
			creature->render(posX - creature->getWalkOffsetX(), posY - creature->getWalkOffsetY());
	}

	for(std::vector<Effect*>::iterator it = m_effects.begin(); it != m_effects.end();)
	{
		Effect* effect = (*it);
		if(effect->isDelayed())
		{
			++it;
			continue;
		}
		
		if(effect->canBeDeleted())
		{
			delete effect;
			it = m_effects.erase(it);
		}
		else
		{
			effect->setCachedX(posX);
			effect->setCachedY(posY);
			g_effects.push_back(effect);
			++it;
		}
	}

	for(std::vector<Item*>::iterator it = m_topItems.begin(), end = m_topItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getTopOrder() == 3)
			item->render(posX + m_tileElevation, posY + m_tileElevation, visible_tile);
	}
}

void Tile::reRenderTile(Sint32 posX, Sint32 posY)
{
	posX -= m_tileElevation;
	posY -= m_tileElevation;
	for(std::vector<Creature*>::reverse_iterator it = m_creatures.rbegin(), end = m_creatures.rend(); it != end; ++it)
	{
		Creature* creature = (*it);
		if(!creature->isWalkingCheck())
			creature->render(posX, posY);
	}

	for(std::vector<Creature*>::iterator it = m_walkCreatures.begin(), end = m_walkCreatures.end(); it != end; ++it)
	{
		Creature* creature = (*it);
		if(creature->isWalkingCheck())
			creature->render(posX - creature->getWalkOffsetX(), posY - creature->getWalkOffsetY());
	}

	for(std::vector<Item*>::iterator it = m_topItems.begin(), end = m_topItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getTopOrder() == 3)
			item->render(posX + m_tileElevation, posY + m_tileElevation, true);
	}
}

void Tile::renderInformations(Sint32 posX, Sint32 posY, Sint32 drawX, Sint32 drawY, float scale, bool visible)
{
	drawX -= m_tileElevation;
	drawY -= m_tileElevation;
	for(std::vector<Creature*>::reverse_iterator it = m_creatures.rbegin(), end = m_creatures.rend(); it != end; ++it)
	{
		Creature* creature = (*it);
		if(!creature->isWalkingCheck())
			creature->renderInformations(posX, posY, drawX, drawY, scale, visible);
	}

	for(std::vector<Creature*>::iterator it = m_walkCreatures.begin(), end = m_walkCreatures.end(); it != end; ++it)
	{
		Creature* creature = (*it);
		if(creature->isWalkingCheck())
			creature->renderInformations(posX, posY, drawX - creature->getWalkOffsetX(), drawY - creature->getWalkOffsetY(), scale, visible);
	}
}

Uint32 Tile::getThingStackPos(Thing* thing)
{
	Uint32 index = 0;
	if(m_ground)
	{
		if(m_ground == thing)
			return index;

		++index;
	}

	for(std::vector<Item*>::iterator it = m_topItems.begin(), end = m_topItems.end(); it != end; ++it)
	{
		if((*it) == thing)
			return index;

		++index;
	}

	for(std::vector<Creature*>::iterator it = m_creatures.begin(), end = m_creatures.end(); it != end; ++it)
	{
		if((*it) == thing)
			return index;

		++index;
	}

	for(std::vector<Item*>::iterator it = m_downItems.begin(), end = m_downItems.end(); it != end; ++it)
	{
		if((*it) == thing)
			return index;

		++index;
	}
	return index;
}

Uint32 Tile::getUseStackPos(Thing* thing)
{
	if(g_clientVersion < 1230)
		return getThingStackPos(thing);

	Uint32 index = 0;
	if(m_ground)
	{
		if(m_ground == thing)
			return index;

		++index;
	}

	for(std::vector<Item*>::iterator it = m_topItems.begin(), end = m_topItems.end(); it != end; ++it)
	{
		if((*it) == thing)
			return index;

		++index;
	}

	for(std::vector<Item*>::iterator it = m_downItems.begin(), end = m_downItems.end(); it != end; ++it)
	{
		if((*it) == thing)
			return index;

		++index;
	}
	return index;
}

Thing* Tile::getThingByStackPos(Uint32 index)
{
	if(m_ground)
	{
		if(index == 0)
			return m_ground;

		--index;
	}

	Uint32 topItemSize = SDL_static_cast(Uint32, m_topItems.size());
	if(index < topItemSize)
		return m_topItems[index];

	index -= topItemSize;

	Uint32 creatureSize = SDL_static_cast(Uint32, m_creatures.size());
	if(index < creatureSize)
		return m_creatures[index];

	index -= creatureSize;

	Uint32 downItemSize = SDL_static_cast(Uint32, m_downItems.size());
	if(index < downItemSize)
		return m_downItems[index];

	//index -= downItemSize;
	return NULL;
}

Creature* Tile::getTopCreature(Sint32 x, Sint32 y, iRect& rect, float scale)
{
	for(std::vector<Creature*>::reverse_iterator it = m_walkCreatures.rbegin(), end = m_walkCreatures.rend(); it != end; ++it)
	{
		Creature* creature = (*it);
		if(creature->isWalkingCheck())
		{
			std::pair<Sint32, Sint32> displacement = creature->getDisplacement();
			Sint32 xOffset = SDL_static_cast(Sint32, scale * (creature->getOffsetX() - creature->getWalkOffsetX() - displacement.first));
			Sint32 yOffset = SDL_static_cast(Sint32, scale * (creature->getOffsetY() - creature->getWalkOffsetY() - displacement.second));
			iRect irect = iRect(rect.x1 + xOffset, rect.y1 + yOffset, rect.x2, rect.y2);
			if(irect.isPointInside(x, y))
				return creature;
		}
	}

	for(std::vector<Creature*>::iterator it = m_creatures.begin(), end = m_creatures.end(); it != end; ++it)
	{
		Creature* creature = (*it);
		if(!creature->isWalkingCheck())
		{
			std::pair<Sint32, Sint32> displacement = creature->getDisplacement();
			Sint32 xOffset = SDL_static_cast(Sint32, scale * displacement.first);
			Sint32 yOffset = SDL_static_cast(Sint32, scale * displacement.second);
			iRect irect = iRect(rect.x1 - xOffset, rect.y1 - yOffset, rect.x2, rect.y2);
			if(irect.isPointInside(x, y))
				return creature;
		}
	}
	return NULL;
}

Creature* Tile::getTopCreature()
{
	if(!m_creatures.empty())
		return (*m_creatures.begin());

	return NULL;
}

Thing* Tile::getTopLookThing()
{
	for(std::vector<Item*>::iterator it = m_downItems.begin(), end = m_downItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(!item->getThingType() || !item->getThingType()->hasFlag(ThingAttribute_LookThrough))
			return item;
	}

	for(std::vector<Item*>::reverse_iterator it = m_topItems.rbegin(), end = m_topItems.rend(); it != end; ++it)
	{
		Item* item = (*it);
		if(!item->getThingType() || !item->getThingType()->hasFlag(ThingAttribute_LookThrough))
			return item;
	}
	return m_ground;
}

Thing* Tile::getTopUseThing()
{
	if(m_ground && m_ground->getThingType() && m_ground->getThingType()->hasFlag(ThingAttribute_ForceUse))
		return m_ground;

	for(std::vector<Item*>::reverse_iterator it = m_topItems.rbegin(), end = m_topItems.rend(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_ForceUse))
			return item;
	}

	if(!m_downItems.empty())
		return *(m_downItems.begin());

	if(!m_topItems.empty())
		return *(m_topItems.rbegin());

	return m_ground;
}

Thing* Tile::getTopMoveThing()
{
	for(std::vector<Item*>::iterator it = m_downItems.begin(), end = m_downItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && !item->getThingType()->hasFlag(ThingAttribute_NotMoveable))
			return item;
	}

	Creature* topCreature = getTopCreature();
	if(topCreature)
		return topCreature;

	for(std::vector<Item*>::reverse_iterator it = m_topItems.rbegin(), end = m_topItems.rend(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && !item->getThingType()->hasFlag(ThingAttribute_NotMoveable))
			return item;
	}
	return m_ground;
}

bool Tile::addThing(Thing* thing, bool insertThing, bool moveCreature)
{
	if(!thing)
		return false;

	if(thing->isCreature())
	{
		Creature* creature = thing->getCreature();
		if(insertThing)
			m_creatures.insert(m_creatures.begin(), creature);
		else
			m_creatures.push_back(creature);

		if(!moveCreature)
			creature->stopMove();

		creature->setCurrentPosition(m_position);
	}
	else if(thing->isItem())
	{
		Item* item = thing->getItem();
		if(!item->getThingType())
		{
			if(insertThing)
				m_downItems.insert(m_downItems.begin(), item);
			else
				m_downItems.push_back(item);

			if(getThingCount() > 10)
			{
				Thing* excessThing = getThingByStackPos(10);
				if(!removeThing(excessThing))
					return false;
			}
			return true;
		}

		if(item->getThingType()->hasFlag(ThingAttribute_Ground))
			m_ground = item;
		else
		{
			Sint32 order = item->getTopOrder();
			if(order == 0)
			{
				if(insertThing)
					m_downItems.insert(m_downItems.begin(), item);
				else
					m_downItems.push_back(item);
			}
			else
			{
				std::vector<Item*>::iterator it = m_topItems.begin(), end = m_topItems.end();
				for(; it != end; ++it)
				{
					Sint32 itOrder = (*it)->getTopOrder();
					if(insertThing)
					{
						if(itOrder >= order)
							break;
					}
					else
					{
						if(itOrder > order)
							break;
					}
				}
				m_topItems.insert(it, item);
			}
		}

		ThingType* tType = item->getThingType();
		if(tType)
		{
			if(tType->hasFlag(ThingAttribute_LyingCorpse))
				m_lyingCorpses = true;

			if(tType->hasFlag(ThingAttribute_Translucent))
				m_hasTranslucentLight = true;
		}
	}
	if(getThingCount() > 10)
	{
		Thing* excessThing = getThingByStackPos(10);
		if(!removeThing(excessThing))
			return false;
	}
	return true;
}

bool Tile::insertThing(Thing* thing, Uint32 stackpos)
{
	if(!thing)
		return false;

	if(thing->isCreature())
	{
		Creature* creature = thing->getCreature();
		stackpos -= SDL_static_cast(Uint32, m_topItems.size()) + (m_ground ? 1 : 0);
		if(stackpos >= SDL_static_cast(Uint32, m_creatures.size()))
			m_creatures.push_back(creature);
		else
			m_creatures.insert(m_creatures.begin() + stackpos, creature);

		creature->stopMove();
		creature->setCurrentPosition(m_position);
	}
	else if(thing->isItem())
	{
		Item* item = thing->getItem();
		if(!item->getThingType())
		{
			stackpos -= SDL_static_cast(Uint32, m_topItems.size()) + SDL_static_cast(Uint32, m_creatures.size()) + (m_ground ? 1 : 0);
			if(stackpos >= SDL_static_cast(Uint32, m_downItems.size()))
				m_downItems.push_back(item);
			else
				m_downItems.insert(m_downItems.begin() + stackpos, item);

			if(getThingCount() > 10)
			{
				Thing* excessThing = getThingByStackPos(10);
				if(!removeThing(excessThing))
					return false;
			}
			return true;
		}

		if(item->getThingType()->hasFlag(ThingAttribute_Ground))
		{
			if(stackpos != 0)
				return false;
			else
				m_ground = item;
		}
		else
		{
			Sint32 order = item->getTopOrder();
			if(order == 0)
			{
				stackpos -= SDL_static_cast(Uint32, m_topItems.size()) + SDL_static_cast(Uint32, m_creatures.size()) + (m_ground ? 1 : 0);
				if(stackpos >= SDL_static_cast(Uint32, m_downItems.size()))
					m_downItems.push_back(item);
				else
					m_downItems.insert(m_downItems.begin() + stackpos, item);
			}
			else
			{
				stackpos -= (m_ground ? 1 : 0);
				if(stackpos >= SDL_static_cast(Uint32, m_topItems.size()))
					m_topItems.push_back(item);
				else
					m_topItems.insert(m_topItems.begin() + stackpos, item);
			}
		}

		ThingType* tType = item->getThingType();
		if(tType)
		{
			if(tType->hasFlag(ThingAttribute_LyingCorpse))
				m_lyingCorpses = true;

			if(tType->hasFlag(ThingAttribute_Translucent))
				m_hasTranslucentLight = true;
		}
	}
	if(getThingCount() > 10)
	{
		Thing* excessThing = getThingByStackPos(10);
		if(!removeThing(excessThing))
			return false;
	}
	return true;
}

bool Tile::removeThing(Thing* thing, bool moveCreature)
{
	if(!thing)
		return false;

	if(thing->isCreature())
	{
		Creature* creature = thing->getCreature();
		std::vector<Creature*>::iterator it = std::find(m_creatures.begin(), m_creatures.end(), creature);
		if(it != m_creatures.end())
		{
			if(!moveCreature)
			{
				creature->removeFromDrawnTile();
				creature->stopMove();
			}

			m_creatures.erase(it);
			return true;
		}
	}
	else if(thing->isItem())
	{
		bool removed = false;
		Item* item = thing->getItem();
		if(item == m_ground)
		{
			m_ground = NULL;
			removed = true;
		}

		if(!removed)
		{
			std::vector<Item*>::iterator it = std::find(m_topItems.begin(), m_topItems.end(), item);
			if(it != m_topItems.end())
			{
				m_topItems.erase(it);
				removed = true;
			}
		}

		if(!removed)
		{
			std::vector<Item*>::iterator it = std::find(m_downItems.begin(), m_downItems.end(), item);
			if(it != m_downItems.end())
			{
				m_downItems.erase(it);
				removed = true;
			}
		}

		ThingType* tType = item->getThingType();
		if(tType)
		{
			if(tType->hasFlag(ThingAttribute_LyingCorpse))
				recacheLyingCorpses();

			if(tType->hasFlag(ThingAttribute_Translucent))
				recacheTranslucentLight();
		}

		if(removed)
			delete thing;

		return removed;
	}
	return false;
}

void Tile::addWalkCreature(Creature* creature)
{
	m_walkCreatures.push_back(creature);
}

void Tile::removeWalkCreature(Creature* creature)
{
	std::vector<Creature*>::iterator it = std::find(m_walkCreatures.begin(), m_walkCreatures.end(), creature);
	if(it != m_walkCreatures.end())
		m_walkCreatures.erase(it);
}

void Tile::addEffect(Effect* effect)
{
	if(effect->isTopEffect())
		m_effects.insert(m_effects.begin(), effect);
	else
		m_effects.push_back(effect);
}

void Tile::removeMagicEffect(Effect* effect)
{
	std::vector<Effect*>::iterator it = std::find(m_effects.begin(), m_effects.end(), effect);
	if(it != m_effects.end())
		m_effects.erase(it);

	delete effect;
}

void Tile::removeMagicEffects(Uint16 effectId)
{
	for(std::vector<Effect*>::iterator it = m_effects.begin(); it != m_effects.end();)
	{
		if((*it)->getID() == effectId)
		{
			delete (*it);
			it = m_effects.erase(it);
		}
		else
			++it;
	}
}

bool Tile::isLookingPossible()
{
	if(m_ground && (!m_ground->getThingType() || !m_ground->getThingType()->hasFlag(ThingAttribute_LookThrough)))
		return true;

	for(std::vector<Item*>::iterator it = m_topItems.begin(), end = m_topItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(!item->getThingType() || !item->getThingType()->hasFlag(ThingAttribute_LookThrough))
			return true;
	}

	for(std::vector<Item*>::iterator it = m_downItems.begin(), end = m_downItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(!item->getThingType() || !item->getThingType()->hasFlag(ThingAttribute_LookThrough))
			return true;
	}
	return false;
}

bool Tile::isWalkable()
{
	if(!m_ground || (m_ground->getThingType() && m_ground->getThingType()->hasFlag(ThingAttribute_NotWalkable)))
		return false;

	for(std::vector<Creature*>::iterator it = m_creatures.begin(), end = m_creatures.end(); it != end; ++it)
	{
		Creature* creature = (*it);
		if(creature->isUnpassable())
			return false;
	}

	for(std::vector<Item*>::iterator it = m_topItems.begin(), end = m_topItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_NotWalkable))
			return false;
	}

	for(std::vector<Item*>::iterator it = m_downItems.begin(), end = m_downItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_NotWalkable))
			return false;
	}
	return true;
}

bool Tile::isPathable()
{
	if(m_ground && m_ground->getThingType() && m_ground->getThingType()->hasFlag(ThingAttribute_NotPathable))
		return false;

	for(std::vector<Item*>::iterator it = m_topItems.begin(), end = m_topItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_NotPathable))
			return false;
	}

	for(std::vector<Item*>::iterator it = m_downItems.begin(), end = m_downItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_NotPathable))
			return false;
	}
	return true;
}

bool Tile::mustHookEast()
{
	if(m_ground && m_ground->getThingType() && m_ground->getThingType()->hasFlag(ThingAttribute_HookEast))
		return true;

	for(std::vector<Item*>::iterator it = m_topItems.begin(), end = m_topItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_HookEast))
			return true;
	}

	for(std::vector<Item*>::iterator it = m_downItems.begin(), end = m_downItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_HookEast))
			return true;
	}
	return false;
}

bool Tile::mustHookSouth()
{
	if(m_ground && m_ground->getThingType() && m_ground->getThingType()->hasFlag(ThingAttribute_HookSouth))
		return true;

	for(std::vector<Item*>::iterator it = m_topItems.begin(), end = m_topItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_HookSouth))
			return true;
	}

	for(std::vector<Item*>::iterator it = m_downItems.begin(), end = m_downItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_HookSouth))
			return true;
	}
	return false;
}

void Tile::recacheTranslucentLight()
{
	m_hasTranslucentLight = false;
	if(m_position.z != GAME_PLAYER_FLOOR)
		return;

	if(m_ground && m_ground->getThingType() && m_ground->getThingType()->hasFlag(ThingAttribute_Translucent))
	{
		m_hasTranslucentLight = true;
		return;
	}

	for(std::vector<Item*>::iterator it = m_topItems.begin(), end = m_topItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_Translucent))
		{
			m_hasTranslucentLight = true;
			return;
		}
	}

	for(std::vector<Item*>::iterator it = m_downItems.begin(), end = m_downItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_Translucent))
		{
			m_hasTranslucentLight = true;
			return;
		}
	}
}

void Tile::recacheLyingCorpses()
{
	m_lyingCorpses = false;
	for(std::vector<Item*>::iterator it = m_topItems.begin(), end = m_topItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_LyingCorpse))
		{
			m_lyingCorpses = true;
			return;
		}
	}

	for(std::vector<Item*>::iterator it = m_downItems.begin(), end = m_downItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_LyingCorpse))
		{
			m_lyingCorpses = true;
			return;
		}
	}
}

size_t Tile::getThingCount()
{
	return (m_ground ? 1 : 0) + m_topItems.size() + m_downItems.size() + m_creatures.size();
}

Uint16 Tile::getGroundSpeed()
{
	if(m_ground && m_ground->getThingType() && m_ground->getThingType()->m_groundSpeed > 0)
		return m_ground->getThingType()->m_groundSpeed;

	return 150;
}

Uint16 Tile::getMinimapSpeed()
{
	if(!isWalkable())
		return 0;

	return getGroundSpeed();
}

Uint16 Tile::getMinimapColor()
{
	Uint16 color = 0;
	if(m_ground && m_ground->getThingType())
		color = m_ground->getThingType()->m_minimapColor;

	for(std::vector<Item*>::iterator it = m_topItems.begin(), end = m_topItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getTopOrder() != 3 && item->getThingType()->m_minimapColor)
			color = item->getThingType()->m_minimapColor;
	}

	for(std::vector<Item*>::iterator it = m_downItems.begin(), end = m_downItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getThingType()->m_minimapColor)
			color = item->getThingType()->m_minimapColor;
	}

	for(std::vector<Item*>::iterator it = m_topItems.begin(), end = m_topItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getTopOrder() == 3 && item->getThingType()->m_minimapColor)
			color = item->getThingType()->m_minimapColor;
	}
	return color;
}

bool Tile::hasground()
{
	if(m_ground)
		return true;

	return false;
}

bool Tile::isFullground()
{
	if(m_ground && m_ground->getThingType() && m_ground->getThingType()->hasFlag(ThingAttribute_FullGround))
		return true;

	return false;
}

bool Tile::isLookPossible()
{
	if(m_ground && m_ground->getThingType() && m_ground->getThingType()->hasFlag(ThingAttribute_BlockProjectile))
		return false;

	for(std::vector<Item*>::iterator it = m_topItems.begin(), end = m_topItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_BlockProjectile))
			return false;
	}

	for(std::vector<Item*>::iterator it = m_downItems.begin(), end = m_downItems.end(); it != end; ++it)
	{
		Item* item = (*it);
		if(item->getThingType() && item->getThingType()->hasFlag(ThingAttribute_BlockProjectile))
			return false;
	}
	return true;
}

bool Tile::limitsFloorsView(bool isFreeView)
{
	//Ground and walls limits the view
	Thing* firstThing = getThingByStackPos(0);
	if(!firstThing)
		return false;

	//Can be creature so check if item
	Item* item = firstThing->getItem();
	if(!item)
		return false;
	
	ThingType* ttype = item->getThingType();
	if(!ttype)
		return false;

	if(isFreeView)
	{
		if(!ttype->hasFlag(ThingAttribute_DontHide) && (ttype->hasFlag(ThingAttribute_Ground) || ttype->hasFlag(ThingAttribute_OnBottom)))
			return true;
	}
	else if(!ttype->hasFlag(ThingAttribute_DontHide) && (ttype->hasFlag(ThingAttribute_Ground) || (ttype->hasFlag(ThingAttribute_OnBottom) && ttype->hasFlag(ThingAttribute_BlockProjectile))))
		return true;

	return false;
}
