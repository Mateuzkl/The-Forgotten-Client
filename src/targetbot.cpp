/*
  The Forgotten Client
  Native targetbot module.
*/

#include "targetbot.h"
#include "creature.h"
#include "game.h"
#include "map.h"

#include <cctype>

extern Game g_game;
extern Map g_map;

static const Uint32 TARGETBOT_SCAN_INTERVAL = 400;

static std::string botLowerName(const std::string& value)
{
	std::string lower = value;
	for(size_t i = 0, end = lower.length(); i < end; ++i)
		lower[i] = SDL_static_cast(char, std::tolower(SDL_static_cast(unsigned char, lower[i])));
	return lower;
}

TargetBot::TargetBot()
{
	reset();
}

void TargetBot::reset()
{
	m_targetNames.clear();
	m_statusText = "Target: ready";
	m_nextTargetTick = 0;
	m_currentTargetId = 0;
	m_maxDistance = 7;
	m_enabled = true;
	m_followTarget = true;
}

void TargetBot::addTargetName(const std::string& name)
{
	if(name.empty())
		return;

	std::string lowerName = botLowerName(name);
	for(std::vector<std::string>::iterator it = m_targetNames.begin(), end = m_targetNames.end(); it != end; ++it)
	{
		if(botLowerName(*it) == lowerName)
			return;
	}
	m_targetNames.push_back(name);
}

bool TargetBot::eraseTargetName(size_t index)
{
	if(index >= m_targetNames.size())
		return false;

	m_targetNames.erase(m_targetNames.begin() + index);
	return true;
}

void TargetBot::clearTargetNames()
{
	m_targetNames.clear();
}

Sint32 TargetBot::getTargetDistance(Creature* creature, const Position& selfPosition) const
{
	Position& creaturePosition = creature->getCurrentPosition();
	return UTIL_max<Sint32>(
		Position::getDistanceX(creaturePosition, selfPosition),
		Position::getDistanceY(creaturePosition, selfPosition));
}

bool TargetBot::hasTargetName(const std::string& name) const
{
	if(m_targetNames.empty())
		return true;

	std::string lowerName = botLowerName(name);
	for(std::vector<std::string>::const_iterator it = m_targetNames.begin(), end = m_targetNames.end(); it != end; ++it)
	{
		if(botLowerName(*it) == lowerName)
			return true;
	}
	return false;
}

bool TargetBot::isValidTarget(Creature* creature, const Position& selfPosition) const
{
	if(!creature || creature->isLocalCreature() || !creature->isVisible())
		return false;

	if(!creature->isMonster() || creature->getHealth() == 0)
		return false;

	if(!hasTargetName(creature->getName()))
		return false;

	Position& creaturePosition = creature->getCurrentPosition();
	if(creaturePosition.z != selfPosition.z)
		return false;

	return getTargetDistance(creature, selfPosition) <= m_maxDistance;
}

Creature* TargetBot::findBestTarget(const Position& selfPosition) const
{
	Creature* bestTarget = NULL;
	Sint32 bestDistance = 0x7FFFFFFF;
	Uint8 bestHealth = 0xFF;
	knownCreatures& creatures = g_map.getKnownCreatures();

	for(knownCreatures::iterator it = creatures.begin(), end = creatures.end(); it != end; ++it)
	{
		Creature* creature = it->second;
		if(!isValidTarget(creature, selfPosition))
			continue;

		Sint32 distance = getTargetDistance(creature, selfPosition);
		Uint8 health = creature->getHealth();
		if(!bestTarget || distance < bestDistance || (distance == bestDistance && health < bestHealth))
		{
			bestTarget = creature;
			bestDistance = distance;
			bestHealth = health;
		}
	}

	return bestTarget;
}

void TargetBot::tick(bool botEnabled)
{
	if(!botEnabled)
	{
		m_statusText = "Target: bot off";
		m_currentTargetId = 0;
		return;
	}

	if(!m_enabled)
	{
		m_statusText = "Target: off";
		m_currentTargetId = 0;
		return;
	}

	Uint32 now = SDL_GetTicks();
	if(now < m_nextTargetTick)
		return;

	m_nextTargetTick = now + TARGETBOT_SCAN_INTERVAL;

	Creature* player = g_map.getLocalCreature();
	if(!player)
	{
		m_statusText = "Target: no local player";
		m_currentTargetId = 0;
		return;
	}

	Position selfPosition = player->getCurrentPosition();
	Creature* currentTarget = g_map.getCreatureById(g_game.getAttackID());
	if(isValidTarget(currentTarget, selfPosition))
	{
		m_currentTargetId = currentTarget->getId();
		if(m_followTarget && g_game.getFollowID() != currentTarget->getId())
			g_game.sendFollow(currentTarget);
		m_statusText = "Target: attacking " + currentTarget->getName();
		return;
	}

	if(g_game.getAttackID() != 0)
		g_game.sendAttack(NULL);

	Creature* bestTarget = findBestTarget(selfPosition);
	if(bestTarget)
	{
		m_currentTargetId = bestTarget->getId();
		g_game.sendAttack(bestTarget);
		if(m_followTarget)
			g_game.sendFollow(bestTarget);
		m_statusText = "Target: attacking " + bestTarget->getName();
		return;
	}

	m_currentTargetId = 0;
	m_statusText = "Target: no monster";
}
