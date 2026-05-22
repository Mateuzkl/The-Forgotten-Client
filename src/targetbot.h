/*
  The Forgotten Client
  Native targetbot module.
*/

#ifndef __FILE_TARGETBOT_h_
#define __FILE_TARGETBOT_h_

#include "position.h"

class Creature;

class TargetBot
{
	public:
		TargetBot();

		void reset();
		void tick(bool botEnabled);

		void setEnabled(bool enabled) {m_enabled = enabled;}
		void toggleEnabled() {m_enabled = !m_enabled;}
		bool isEnabled() const {return m_enabled;}
		void setFollowTarget(bool enabled) {m_followTarget = enabled;}
		bool isFollowTarget() const {return m_followTarget;}

		bool hasActiveTarget() const {return m_currentTargetId != 0;}
		Uint32 getCurrentTargetId() const {return m_currentTargetId;}
		const std::string& getStatusText() const {return m_statusText;}
		const std::vector<std::string>& getTargetNames() const {return m_targetNames;}

		void addTargetName(const std::string& name);
		bool eraseTargetName(size_t index);
		void clearTargetNames();

	private:
		bool hasTargetName(const std::string& name) const;
		bool isValidTarget(Creature* creature, const Position& selfPosition) const;
		Sint32 getTargetDistance(Creature* creature, const Position& selfPosition) const;
		Creature* findBestTarget(const Position& selfPosition) const;

		std::vector<std::string> m_targetNames;
		std::string m_statusText;
		Uint32 m_nextTargetTick = 0;
		Uint32 m_currentTargetId = 0;
		Sint32 m_maxDistance = 7;
		bool m_enabled = true;
		bool m_followTarget = true;
};

#endif /* __FILE_TARGETBOT_h_ */
