/*
  The Forgotten Client
  Native bot coordinator.
*/

#ifndef __FILE_BOT_h_
#define __FILE_BOT_h_

#include "cavebot.h"
#include "targetbot.h"

struct BotHealingConfig
{
	bool enabled = true;
	Uint16 hpPotionId = 0;
	Uint8 hpPotionPercent = 60;
	Uint16 mpPotionId = 0;
	Uint8 mpPotionPercent = 60;
	Uint16 uhRuneId = 0;
	Uint8 friendPercent = 50;
	std::string friendName;
};

struct BotHotkeyConfig
{
	bool enabled = false;
	bool persistent = false;
	std::string keyName;
	std::string spellWords;
	Uint32 cooldownMs = 2000;
	Uint32 nextCastTick = 0;
};

struct BotIconConfig
{
	bool enabled = false;
	Uint16 itemId = 0;
	std::string label;
	Sint32 x = 8;
	Sint32 y = 8;
	std::string leftAction;
	std::string rightAction;
	std::string ctrlLeftAction;
	std::string ctrlRightAction;
};

struct BotLootItem
{
	Uint16 itemId = 0;
	std::string name;
};

class Container;
class Creature;

enum BotLootCorpseStatus
{
	BOT_LOOT_CORPSE_PENDING,
	BOT_LOOT_CORPSE_OPENED,
	BOT_LOOT_CORPSE_LOOTED,
	BOT_LOOT_CORPSE_IGNORED
};

struct BotLootCorpse
{
	Uint32 creatureId = 0;
	std::string creatureName;
	Position position;
	Uint16 corpseId = 0;
	Uint8 containerId = 0xFF;
	Uint32 createdAt = 0;
	Uint32 lootedAt = 0;
	BotLootCorpseStatus status = BOT_LOOT_CORPSE_PENDING;
};

class Bot
{
	public:
		Bot();

		void reset();
		void tick();

		void setEnabled(bool enabled);
		void toggleEnabled() {setEnabled(!m_enabled);}
		bool isEnabled() const {return m_enabled;}

		void setCaveEnabled(bool enabled) {m_cavebot.setEnabled(enabled);}
		void setTargetEnabled(bool enabled) {m_targetbot.setEnabled(enabled);}
		void setPauseCaveInFight(bool enabled) {m_pauseCaveInFight = enabled;}
		void setSmartWalk(bool enabled);
		void setAvoidPlayers(bool enabled) {m_avoidPlayers = enabled;}

		bool isCaveEnabled() const {return m_cavebot.isEnabled();}
		bool isTargetEnabled() const {return m_targetbot.isEnabled();}
		bool isPauseCaveInFight() const {return m_pauseCaveInFight;}
		bool isSmartWalk() const {return m_smartWalk;}
		bool isAvoidPlayers() const {return m_avoidPlayers;}
		void setFollowTarget(bool enabled) {m_targetbot.setFollowTarget(enabled);}
		bool isFollowTarget() const {return m_targetbot.isFollowTarget();}
		bool hasActiveTarget() const {return m_targetbot.hasActiveTarget();}
		const std::string& getTargetStatusText() const {return m_targetbot.getStatusText();}
		const std::string& getCaveStatusText() const {return m_cavebot.getStatusText();}

		bool addCurrentWaypoint(CaveBotWaypointType type = CAVEBOT_WAYPOINT_WALK) {return m_cavebot.addCurrentWaypoint(type);}
		bool eraseWaypoint(size_t index) {return m_cavebot.eraseWaypoint(index);}
		void clearWaypoints() {m_cavebot.clearWaypoints();}

		size_t getWaypointCount() const {return m_cavebot.getWaypointCount();}
		size_t getCurrentWaypointIndex() const {return m_cavebot.getCurrentWaypointIndex();}
		const std::vector<CaveBotWaypoint>& getWaypoints() const {return m_cavebot.getWaypoints();}

		void addTargetName(const std::string& name) {m_targetbot.addTargetName(name);}
		bool eraseTargetName(size_t index) {return m_targetbot.eraseTargetName(index);}
		void clearTargetNames() {m_targetbot.clearTargetNames();}
		const std::vector<std::string>& getTargetNames() const {return m_targetbot.getTargetNames();}

		void setHealingConfig(const BotHealingConfig& config) {m_healingConfig = config;}
		const BotHealingConfig& getHealingConfig() const {return m_healingConfig;}
		void setHealingEnabled(bool enabled) {m_healingConfig.enabled = enabled;}
		bool isHealingEnabled() const {return m_healingConfig.enabled;}

		bool addLootItem(Uint16 itemId);
		bool addLootItem(const std::string& input);
		bool eraseLootItem(size_t index);
		void clearLootItems() {m_lootItems.clear();}
		const std::vector<BotLootItem>& getLootItems() const {return m_lootItems;}
		bool isLootItem(Uint16 itemId) const;
		void onContainerOpen(Container* container);
		void registerLootCorpse(Uint32 creatureId, const std::string& creatureName, const Position& position);
		void registerLootCorpseItem(const Position& position, Uint16 corpseId);

		void setHotkeysEnabled(bool enabled) {m_hotkeysEnabled = enabled;}
		bool isHotkeysEnabled() const {return m_hotkeysEnabled;}
		std::vector<BotHotkeyConfig>& getHotkeys() {return m_hotkeys;}
		const std::vector<BotHotkeyConfig>& getHotkeys() const {return m_hotkeys;}
		void setHotkey(size_t index, const BotHotkeyConfig& hotkey);
		bool handleKeyDown(SDL_Event& event);

		std::vector<BotIconConfig>& getIcons() {return m_icons;}
		const std::vector<BotIconConfig>& getIcons() const {return m_icons;}
		void setIcon(size_t index, const BotIconConfig& icon);

		bool saveProfile();
		bool loadProfile();
		bool saveProfile(const std::string& profileName);
		bool loadProfile(const std::string& profileName);
		bool deleteProfile(const std::string& profileName);

		bool exportWaypoints(const std::string& profileName);
		bool importWaypoints(const std::string& profileName);

		std::string getStatusText() const;

	private:
		Creature* findFriendByName(const std::string& name) const;
		void tickHealing();
		void tickLootCorpseTracking();
		void tickAutoLootOpenCorpse();
		void cleanupLootCorpses(Uint32 now);
		BotLootCorpse* findPendingLootCorpse(const Position& position, Uint32 now);
		void tickPersistentHotkeys();
		bool castHotkey(BotHotkeyConfig& hotkey, Uint32 now);

		CaveBot m_cavebot;
		TargetBot m_targetbot;
		BotHealingConfig m_healingConfig;
		std::vector<BotLootItem> m_lootItems;
		std::vector<BotLootCorpse> m_lootCorpses;
		std::vector<BotHotkeyConfig> m_hotkeys;
		std::vector<BotIconConfig> m_icons;
		Uint32 m_nextHealTick = 0;
		Uint32 m_nextLootTick = 0;
		Uint32 m_nextCorpseOpenTick = 0;
		Uint32 m_lastCorpseOpenTick = 0;
		Position m_lastCorpseOpenPosition;
		Uint32 m_lastTrackedTargetId = 0;
		std::string m_lastTrackedTargetName;
		Position m_lastTrackedTargetPosition;
		Uint32 m_lastTrackedTargetTick = 0;
		bool m_lootInProgress = false;
		bool m_enabled = false;
		bool m_pauseCaveInFight = true;
		bool m_hotkeysEnabled = false;
		bool m_smartWalk = true;
		bool m_avoidPlayers = false;
};

extern Bot g_bot;

#endif /* __FILE_BOT_h_ */
