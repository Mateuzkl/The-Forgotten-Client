/*
  The Forgotten Client
  Native cavebot module.
*/

#include "cavebot.h"
#include "creature.h"
#include "game.h"
#include "item.h"
#include "map.h"
#include "thing.h"
#include "tile.h"

extern Game g_game;
extern Map g_map;

static const Uint32 CAVEBOT_WALK_INTERVAL = 650;
static const Uint32 CAVEBOT_REPATH_INTERVAL = 2500;
static const Uint32 CAVEBOT_MANUAL_ADD_DEBOUNCE = 300;
static const Uint32 CAVEBOT_STUCK_TIMEOUT = 8000;

CaveBot::CaveBot()
{
	reset();
}

const char* CaveBot::getWaypointTypeName(CaveBotWaypointType type)
{
	switch(type)
	{
		case CAVEBOT_WAYPOINT_WALK: return "Walk";
		case CAVEBOT_WAYPOINT_CENTER: return "Center";
		case CAVEBOT_WAYPOINT_STAND: return "Stand";
		case CAVEBOT_WAYPOINT_NODE: return "Node";
		case CAVEBOT_WAYPOINT_ROPE: return "Rope";
		case CAVEBOT_WAYPOINT_LADDER: return "Ladder";
		case CAVEBOT_WAYPOINT_SHOVEL: return "Shovel";
		case CAVEBOT_WAYPOINT_LURE: return "Lure";
		case CAVEBOT_WAYPOINT_ACTION: return "Action";
		default: break;
	}
	return "Walk";
}

void CaveBot::reset()
{
	m_waypoints.clear();
	m_statusText = "Cave: no waypoints";
	m_lastPosition = Position(0xFFFF, 0xFFFF, 0xFF);
	m_activeDestination = Position(0xFFFF, 0xFFFF, 0xFF);
	m_currentWaypoint = 0;
	m_nextWalkTick = 0;
	m_nextManualAddTick = 0;
	m_lastProgressTick = 0;
	m_enabled = true;
}

bool CaveBot::addWaypoint(const Position& position, CaveBotWaypointType type)
{
	if(position.x == 0 || position.y == 0 || position.z > GAME_MAP_FLOORS)
		return false;

	CaveBotWaypoint waypoint;
	waypoint.position = position;
	waypoint.type = type;
	m_waypoints.push_back(waypoint);
	if(m_waypoints.size() == 1)
		m_currentWaypoint = 0;

	m_statusText = "Cave: waypoint added";
	return true;
}

bool CaveBot::addCurrentWaypoint(CaveBotWaypointType type)
{
	Uint32 now = SDL_GetTicks();
	if(now < m_nextManualAddTick)
		return false;

	Creature* player = g_map.getLocalCreature();
	if(!player)
	{
		m_statusText = "Cave: no local player";
		return false;
	}

	Position currentPosition = player->getCurrentPosition();
	if(currentPosition.x == 0 || currentPosition.y == 0 || currentPosition.z > GAME_MAP_FLOORS)
	{
		m_statusText = "Cave: invalid position";
		return false;
	}

	if(!m_waypoints.empty() && m_waypoints.back().position == currentPosition && m_waypoints.back().type == type)
	{
		m_statusText = "Cave: duplicate ignored";
		return false;
	}

	m_nextManualAddTick = now + CAVEBOT_MANUAL_ADD_DEBOUNCE;
	return addWaypoint(currentPosition, type);
}

bool CaveBot::eraseWaypoint(size_t index)
{
	if(index >= m_waypoints.size())
		return false;

	m_waypoints.erase(m_waypoints.begin() + index);
	if(m_waypoints.empty())
		m_currentWaypoint = 0;
	else if(m_currentWaypoint >= m_waypoints.size())
		m_currentWaypoint = 0;

	m_activeDestination = Position(0xFFFF, 0xFFFF, 0xFF);
	m_statusText = m_waypoints.empty() ? "Cave: no waypoints" : "Cave: waypoint removed";
	return true;
}

void CaveBot::clearWaypoints()
{
	m_waypoints.clear();
	m_currentWaypoint = 0;
	m_activeDestination = Position(0xFFFF, 0xFFFF, 0xFF);
	m_statusText = "Cave: no waypoints";
}

bool CaveBot::isAtWaypoint(const Position& position, const Position& waypoint) const
{
	return position == waypoint;
}

bool CaveBot::isMovementWaypoint(CaveBotWaypointType type) const
{
	return type == CAVEBOT_WAYPOINT_WALK ||
		type == CAVEBOT_WAYPOINT_CENTER ||
		type == CAVEBOT_WAYPOINT_STAND ||
		type == CAVEBOT_WAYPOINT_NODE ||
		type == CAVEBOT_WAYPOINT_LURE;
}

bool CaveBot::useWaypointAction(const CaveBotWaypoint& waypoint)
{
	Tile* tile = g_map.getTile(waypoint.position);
	if(!tile)
		return false;

	Thing* useThing = tile->getTopUseThing();
	if(!useThing || !useThing->isItem())
		return false;

	Item* item = useThing->getItem();
	Uint8 stackpos = SDL_static_cast(Uint8, tile->getUseStackPos(useThing));
	if(waypoint.type == CAVEBOT_WAYPOINT_ROPE)
	{
		g_game.sendUseItemEx(Position(0xFFFF, 0, 0), 3003, 0, waypoint.position, item->getID(), stackpos);
		m_statusText = "Cave: rope action";
		return true;
	}
	else if(waypoint.type == CAVEBOT_WAYPOINT_SHOVEL)
	{
		g_game.sendUseItemEx(Position(0xFFFF, 0, 0), 3457, 0, waypoint.position, item->getID(), stackpos);
		m_statusText = "Cave: shovel action";
		return true;
	}
	else if(waypoint.type == CAVEBOT_WAYPOINT_LADDER || waypoint.type == CAVEBOT_WAYPOINT_ACTION)
	{
		g_game.sendUseItem(waypoint.position, item->getID(), stackpos, g_game.findEmptyContainerId());
		m_statusText = "Cave: use action";
		return true;
	}
	return false;
}

void CaveBot::advanceWaypoint()
{
	if(m_waypoints.empty())
	{
		m_currentWaypoint = 0;
		return;
	}

	++m_currentWaypoint;
	if(m_currentWaypoint >= m_waypoints.size())
		m_currentWaypoint = 0;
}

void CaveBot::tick(bool botEnabled, bool pauseWalking)
{
	if(!botEnabled)
	{
		m_statusText = "Cave: bot off";
		return;
	}

	if(!m_enabled)
	{
		m_statusText = "Cave: off";
		return;
	}

	if(pauseWalking)
	{
		m_statusText = "Cave: paused in fight";
		return;
	}

	Creature* player = g_map.getLocalCreature();
	if(!player)
	{
		m_statusText = "Cave: no local player";
		return;
	}

	if(m_waypoints.empty())
	{
		m_statusText = "Cave: add waypoints";
		return;
	}

	Uint32 now = SDL_GetTicks();
	Position currentPosition = player->getCurrentPosition();
	if(m_lastProgressTick == 0)
		m_lastProgressTick = now;

	CaveBotWaypoint* waypoint = &m_waypoints[m_currentWaypoint];
	if(isAtWaypoint(currentPosition, waypoint->position))
	{
		if(!isMovementWaypoint(waypoint->type))
		{
			useWaypointAction(*waypoint);
			advanceWaypoint();
			m_activeDestination = Position(0xFFFF, 0xFFFF, 0xFF);
			m_lastPosition = currentPosition;
			m_lastProgressTick = now;
			m_nextWalkTick = now + CAVEBOT_WALK_INTERVAL;
			return;
		}

		advanceWaypoint();
		m_activeDestination = Position(0xFFFF, 0xFFFF, 0xFF);
		m_lastPosition = currentPosition;
		m_lastProgressTick = now;
	}

	waypoint = &m_waypoints[m_currentWaypoint];
	if(!isMovementWaypoint(waypoint->type))
	{
		advanceWaypoint();
		m_activeDestination = Position(0xFFFF, 0xFFFF, 0xFF);
		m_lastProgressTick = now;
		m_nextWalkTick = 0;
		m_statusText = "Cave: skipped action wp";
		return;
	}

	if(currentPosition.z != waypoint->position.z)
	{
		m_statusText = "Cave: different floor";
		return;
	}

	if(currentPosition != m_lastPosition)
	{
		m_lastPosition = currentPosition;
		m_lastProgressTick = now;
	}
	else if(now - m_lastProgressTick > CAVEBOT_STUCK_TIMEOUT)
	{
		advanceWaypoint();
		m_activeDestination = Position(0xFFFF, 0xFFFF, 0xFF);
		m_lastProgressTick = now;
		m_nextWalkTick = 0;
		m_statusText = "Cave: skipped stuck wp";
		return;
	}

	if(player->isWalking() || player->isPreWalking())
	{
		m_statusText = "Cave: walking";
		return;
	}

	if(m_activeDestination == waypoint->position && now < m_nextWalkTick)
	{
		m_statusText = "Cave: walking";
		return;
	}

	g_game.startAutoWalk(waypoint->position);
	m_activeDestination = waypoint->position;
	m_nextWalkTick = now + CAVEBOT_REPATH_INTERVAL;
	m_statusText = "Cave: walking";
}
