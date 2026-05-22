/*
  The Forgotten Client
  Native cavebot module.
*/

#ifndef __FILE_CAVEBOT_h_
#define __FILE_CAVEBOT_h_

#include "position.h"

enum CaveBotWaypointType
{
	CAVEBOT_WAYPOINT_WALK,
	CAVEBOT_WAYPOINT_CENTER,
	CAVEBOT_WAYPOINT_STAND,
	CAVEBOT_WAYPOINT_NODE,
	CAVEBOT_WAYPOINT_ROPE,
	CAVEBOT_WAYPOINT_LADDER,
	CAVEBOT_WAYPOINT_SHOVEL,
	CAVEBOT_WAYPOINT_LURE,
	CAVEBOT_WAYPOINT_ACTION
};

struct CaveBotWaypoint
{
	Position position;
	CaveBotWaypointType type = CAVEBOT_WAYPOINT_WALK;
};

class CaveBot
{
	public:
		CaveBot();

		void reset();
		void tick(bool botEnabled, bool pauseWalking);

		void setEnabled(bool enabled) {m_enabled = enabled;}
		void toggleEnabled() {m_enabled = !m_enabled;}
		bool isEnabled() const {return m_enabled;}

		bool addWaypoint(const Position& position, CaveBotWaypointType type = CAVEBOT_WAYPOINT_WALK);
		bool addCurrentWaypoint(CaveBotWaypointType type = CAVEBOT_WAYPOINT_WALK);
		bool eraseWaypoint(size_t index);
		void clearWaypoints();

		size_t getWaypointCount() const {return m_waypoints.size();}
		size_t getCurrentWaypointIndex() const {return m_currentWaypoint;}
		const std::vector<CaveBotWaypoint>& getWaypoints() const {return m_waypoints;}
		const std::string& getStatusText() const {return m_statusText;}
		static const char* getWaypointTypeName(CaveBotWaypointType type);

	private:
		bool isMovementWaypoint(CaveBotWaypointType type) const;
		bool useWaypointAction(const CaveBotWaypoint& waypoint);
		bool isAtWaypoint(const Position& position, const Position& waypoint) const;
		void advanceWaypoint();

		std::vector<CaveBotWaypoint> m_waypoints;
		std::string m_statusText;
		Position m_lastPosition;
		Position m_activeDestination;
		size_t m_currentWaypoint = 0;
		Uint32 m_nextWalkTick = 0;
		Uint32 m_nextManualAddTick = 0;
		Uint32 m_lastProgressTick = 0;
		bool m_enabled = true;
};

#endif /* __FILE_CAVEBOT_h_ */
