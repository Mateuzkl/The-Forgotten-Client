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

#ifndef __FILE_GAME_h_
#define __FILE_GAME_h_

#include "position.h"

enum Stats_Cached_State : Uint16
{
	CACHED_STAT_LEVEL = (1 << 0),
	CACHED_STAT_EXPERIENCE = (1 << 1),
	CACHED_STAT_CAPACITY = (1 << 2),
	CACHED_STAT_HEALTH = (1 << 3),
	CACHED_STAT_MANA = (1 << 4),
	CACHED_STAT_MAGICLEVEL = (1 << 5),
	CACHED_STAT_STAMINA = (1 << 6),
	CACHED_STAT_SOUL = (1 << 7),
	CACHED_STAT_SPEED = (1 << 8),
	CACHED_STAT_REGENERATION = (1 << 9),
	CACHED_STAT_TRAINING = (1 << 10),
	CACHED_STAT_XPGAIN = (1 << 11)
};

enum Skills_Cached_State : Uint16
{
	CACHED_SKILL_FIST = (1 << 0),
	CACHED_SKILL_CLUB = (1 << 1),
	CACHED_SKILL_SWORD = (1 << 2),
	CACHED_SKILL_AXE = (1 << 3),
	CACHED_SKILL_DISTANCE = (1 << 4),
	CACHED_SKILL_SHIELDING = (1 << 5),
	CACHED_SKILL_FISHING = (1 << 6),
	CACHED_SKILL_CRITICAL = (1 << 7),
	CACHED_SKILL_LIFELEECH = (1 << 8),
	CACHED_SKILL_MANALEECH = (1 << 9)
};

class Creature;
class ItemUI;
class Container;
class Game
{
	public:
		Game();
		~Game();

		// non-copyable
		Game(const Game&) = delete;
		Game& operator=(const Game&) = delete;

		// non-moveable
		Game(Game&&) = delete;
		Game& operator=(Game&&) = delete;

		void reset();

		void clientChangeVersion(Uint32 clientVersion, Uint32 fileVersion);
		void applyDefaultFeaturesForVersion(Uint32 clientVersion, Uint32 fileVersion);
		void resetGameFeatures() {m_gameFeatures.reset();}
		bool hasGameFeature(GameFeatures feature) const {return (feature >= GAME_FEATURE_FIRST && feature < GAME_FEATURE_LAST ? m_gameFeatures[feature] : false);}
		void setGameFeature(GameFeatures feature, bool enabled);
		void forceGameFeature(GameFeatures feature, bool enabled);
		void enableGameFeature(GameFeatures feature) {setGameFeature(feature, true);}
		void disableGameFeature(GameFeatures feature) {setGameFeature(feature, false);}
		bool enableGameFeatureByName(const std::string& featureName);
		bool disableGameFeatureByName(const std::string& featureName);
		bool hasGameFeatureByName(const std::string& featureName) const;
		bool loadFeatureProfile(const std::string& profileName);

		void processGMActions(std::vector<Uint8> privileges);
		void processLoginWait(const std::string& message, Uint8 time);
		void processDeath(Uint8 deathType, Uint8 penalty);
		void processContainerOpen(Uint8 containerId, ItemUI* item, const std::string& name, Uint8 capacity, bool hasParent, bool canUseDepotSearch, bool isUnlocked, bool hasPages, Uint16 containerSize, Uint16 firstIndex, std::vector<ItemUI*>& itemVector);
		void processContainerClose(Uint8 containerId);
		void processContainerAddItem(Uint8 containerId, Uint16 slot, ItemUI* item);
		void processContainerTransformItem(Uint8 containerId, Uint16 slot, ItemUI* item);
		void processContainerRemoveItem(Uint8 containerId, Uint16 slot, ItemUI* lastItem);
		void processTransformInventoryItem(Uint8 slot, ItemUI* item);
		void processTradeOwn(const std::string& name, std::vector<ItemUI*>& itemVector);
		void processTradeCounter(const std::string& name, std::vector<ItemUI*>& itemVector);
		void processTradeClose();
		void processNPCTradeOpen(const std::string& name, Uint16 currencyId, std::vector<NPCTradeItems>& npcItems);
		void processNPCTradeGoods(Uint64 playerMoney, NPCTradeGoods& npcGoods);
		void processNPCTradeClose();
		void processEditTextWindow(Uint32 windowId, ItemUI* item, Uint16 maxLen, const std::string& text, const std::string& writer, const std::string& date);
		void processEditHouseWindow(Uint8 doorId, Uint32 windowId, const std::string& text);
		void processTextMessage(MessageMode mode, const std::string& text, Uint32 channelId = 0);
		void processWalkCancel(Direction direction);
		void processWalkWait(Uint16 walkTime);
		void processOutfits(Uint16 lookType, Uint8 lookHead, Uint8 lookBody, Uint8 lookLegs, Uint8 lookFeet, Uint8 lookAddons, Uint16 lookMount, std::vector<OutfitDetail>& outfits, std::vector<MountDetail>& mounts);
		void processTutorialHint(Uint8 tutorialId);
		void processCancelTarget(Uint32 sequence);
		void processSpellDelay(Uint8 spellId, Uint32 delay);
		void processSpellGroupDelay(Uint8 groupId, Uint32 delay);
		void processMultiUseDelay(Uint32 delay);
		void processPlayerModes(Uint8 fightMode, Uint8 chaseMode, Uint8 safeMode, Uint8 pvpMode);
		void processTalk(const std::string& name, Uint32 statementId, Uint16 playerLevel, MessageMode mode, std::string& text, Uint32 channelId, const Position& position);
		void processChannelList(std::vector<ChannelDetail>& channels);
		void processOpenChannel(Uint16 channelId, const std::string& channelName);
		void processOpenPrivateChannel(const std::string& receiver);
		void processOpenOwnPrivateChannel(Uint16 channelId, const std::string& channelName);
		void processCloseChannel(Uint16 channelId);
		void processQuestLog(Uint32 questId, std::vector<QuestLogDetail>& questLogElements);
		void processQuestLine(Uint16 questId, std::vector<QuestLineDetail>& questLineElements);
		void processVipGroups(std::vector<VipGroups>& groups, Uint8 createGroupsLeft);
		void processVipAdd(Uint32 playerGUID, const std::string& playerName, const std::string& description, Uint32 iconId, bool notifyLogin, Uint8 status, std::vector<Uint8>& groups);
		void processVipStatusChange(Uint32 playerGUID, Uint8 status);
		void processModalWindow(Uint32 windowId, bool priority, const std::string& title, const std::string& message, Uint8 enterButtonId, Uint8 escapeButtonId, std::vector<std::pair<std::string, Uint8>>& buttons, std::vector<std::pair<std::string, Uint8>>& choices);

		void sendEnterGame();
		void sendLogout();
		void sendAutoWalk(const std::vector<Direction>& path);
		void sendWalk(Direction dir);
		void sendTurn(Direction dir);
		void sendSay(MessageMode mode, Uint16 channelId, const std::string& receiver, const std::string& message);
		void sendRequestChannels();
		void sendCreatePrivateChannel();
		void sendOpenChannel(Uint16 channelId);
		void sendCloseChannel(Uint16 channelId);
		void sendOpenPrivateChannel(const std::string& receiver);
		void sendCloseNPCChannel();
		void sendChannelInvite(const std::string& target, Uint16 channelId);
		void sendChannelExclude(const std::string& target, Uint16 channelId);
		void sendOpenQuestLog();
		void sendOpenQuestLine(Uint16 questId);
		void sendPing();
		void sendPingBack();
		void sendAttackModes();
		void sendAttack(Creature* creature);
		void sendFollow(Creature* creature);
		void sendCancelAttackAndFollow();
		void sendJoinAggression(Uint32 creatureId);
		void sendInviteToParty(Uint32 creatureId);
		void sendJoinToParty(Uint32 creatureId);
		void sendRevokePartyInvitation(Uint32 creatureId);
		void sendPassPartyLeadership(Uint32 creatureId);
		void sendLeaveParty();
		void sendEnableSharedPartyExperience(bool active);
		void sendRequestOutfit();
		void sendSetOutfit(Uint16 lookType, Uint8 lookHead, Uint8 lookBody, Uint8 lookLegs, Uint8 lookFeet, Uint8 lookAddons, Uint16 lookMount);
		void sendMount(bool active);
		void sendEquipItem(Uint16 itemid, Uint16 count);
		void sendMove(const Position& fromPos, Uint16 itemid, Uint8 stackpos, const Position& toPos, Uint16 count);
		void sendLookAt(const Position& position, Uint16 itemId, Uint8 stackpos);
		void sendLookInBattle(Uint32 creatureId);
		void sendUseItem(const Position& position, Uint16 itemId, Uint8 stackpos, Uint8 index);
		void sendUseItemEx(const Position& fromPos, Uint16 itemId, Uint8 fromStackPos, const Position& toPos, Uint16 toItemId, Uint8 toStackPos);
		void sendUseOnCreature(const Position& position, Uint16 itemId, Uint8 stackpos, Uint32 creatureId);
		void sendRotateItem(const Position& position, Uint16 itemId, Uint8 stackpos);
		void sendWrapState(const Position& position, Uint16 itemId, Uint8 stackpos);
		void sendCloseContainer(Uint8 containerId);
		void sendUpContainer(Uint8 containerId);
		void sendUpdateContainer(Uint8 containerId);
		void sendSeekInContainer(Uint8 containerId, Uint16 index);
		void sendBrowseField(const Position& position);
		void sendOpenParentContainer(const Position& position);
		void sendTextWindow(Uint32 windowId, const std::string& text);
		void sendHouseWindow(Uint32 windowId, Uint8 doorId, const std::string& text);
		void sendRequestTrade(const Position& position, Uint16 itemId, Uint8 stackpos, Uint32 creatureId);
		void sendLookInTrade(bool counterOffer, Uint8 index);
		void sendAcceptTrade();
		void sendCloseTrade();
		void sendLookInShop(Uint16 itemid, Uint16 count);
		void sendNPCPurchase(Uint16 itemid, Uint8 subtype, Uint16 amount);
		void sendNPCSell(Uint16 itemid, Uint8 subtype, Uint16 amount);
		void sendNPCClose();
		void sendAddVip(const std::string& name);
		void sendRemoveVip(Uint32 playerGUID);
		void sendEditVip(Uint32 playerGUID, const std::string& description, Uint32 iconId, bool notifyLogin, std::vector<Uint8>& groupIds);
		void sendVipGroupAction(VipGroupActions groupAction, Uint8 groupId, const std::string& name);
		void sendAnswerModalDialog(Uint32 dialogId, Uint8 button, Uint8 choice);

		void stopActions();
		void startAutoWalk(const Position& toPosition);
		void stopAutoWalk();
		void checkServerMovement(Direction dir);
		void checkLocalCreatureMovement();
		void checkMovement(Direction dir);
		void releaseMovement();

		void switchToNextChannel();
		void switchToPreviousChannel();
		void closeCurrentChannel();
		void openHelpChannel();
		void openNPCChannel();
		void switchToDefault();

		void minimapCenter();
		void minimapFloorDown();
		void minimapFloorUp();
		void minimapScrollEast();
		void minimapScrollNorth();
		void minimapScrollSouth();
		void minimapScrollWest();
		void minimapScrollNorthWest();
		void minimapScrollNorthEast();
		void minimapScrollSouthWest();
		void minimapScrollSouthEast();
		void minimapZoomIn();
		void minimapZoomOut();
		void minimapSetZoom(Sint32 zoom);
		Sint32 minimapGetZoom();

		Container* findContainer(Uint8 cid) {return m_containers[cid];}
		Uint8 findEmptyContainerId();
		bool containerHasParent(Uint8 cid);

		SDL_INLINE ItemUI* getInventoryItem(Uint8 slot) {return m_inventoryItem[slot];}

		SDL_INLINE void setPlayerID(Uint32 localPlayerId) {m_localPlayerId = localPlayerId;}
		SDL_INLINE Uint32 getPlayerID() {return m_localPlayerId;}
		SDL_INLINE void setAttackID(Uint32 attackId) {m_attackId = attackId;}
		SDL_INLINE Uint32 getAttackID() {return m_attackId;}
		SDL_INLINE void setFollowID(Uint32 followId) {m_followId = followId;}
		SDL_INLINE Uint32 getFollowID() {return m_followId;}
		SDL_INLINE void setSelectID(Uint32 selectId) {m_selectId = selectId;}
		SDL_INLINE Uint32 getSelectID() {return m_selectId;}

		void resetPlayerExperienceTable();
		void updatePlayerExperienceTable();
		SDL_INLINE double getPlayerExperienceSpeed() {return m_playerExpSpeed;}
		std::string getPlayerName();

		void setPlayerExperience(Uint64 experience);
		SDL_INLINE Uint64 getPlayerExperience() {return m_playerExperience;}
		SDL_INLINE Uint64 getExpForLevel(Sint32 lvl) {Uint64 lv = SDL_static_cast(Uint64, lvl - 1); return ((50ULL * lv * lv * lv) - (150ULL * lv * lv) + (400ULL * lv)) / 3;}

		void setPlayerCapacity(double capacity, double totalCapacity);
		SDL_INLINE double getPlayerCapacity() {return m_playerCapacity;}
		SDL_INLINE double getPlayerTotalCapacity() {return m_playerTotalCapacity;}

		void setPlayerHealth(Uint32 health, Uint32 maxHealth);
		SDL_INLINE Uint32 getPlayerHealth() {return m_playerHealth;}
		SDL_INLINE Uint32 getPlayerMaxHealth() {return m_playerMaxHealth;}
		SDL_INLINE Uint8 getPlayerHealthPercent() {return m_playerHealthPercent;}

		void setPlayerMana(Uint32 mana, Uint32 maxMana);
		SDL_INLINE Uint32 getPlayerMana() {return m_playerMana;}
		SDL_INLINE Uint32 getPlayerMaxMana() {return m_playerMaxMana;}
		SDL_INLINE Uint8 getPlayerManaPercent() {return m_playerManaPercent;}

		void setPlayerLevel(Uint16 level, Uint8 levelPercent);
		SDL_INLINE Uint16 getPlayerLevel() {return m_playerLevel;}
		SDL_INLINE Uint8 getPlayerLevelPercent() {return m_playerLevelPercent;}

		void setPlayerMagicLevel(Uint16 level, Uint16 baseLevel, Uint8 levelPercent);
		SDL_INLINE Uint16 getPlayerMagicLevel() {return m_playerMagicLevel;}
		SDL_INLINE Uint16 getPlayerBaseMagicLevel() {return m_playerBaseMagicLevel;}
		SDL_INLINE Uint8 getPlayerMagicLevelPercent() {return m_playerMagicLevelPercent;}

		void setPlayerStamina(Uint16 stamina);
		SDL_INLINE Uint16 getPlayerStamina() {return m_playerStamina;}
		void setPlayerSoul(Uint8 soul);
		SDL_INLINE Uint8 getPlayerSoul() {return m_playerSoul;}
		void setPlayerBaseSpeed(Uint16 baseSpeed);
		SDL_INLINE Uint16 getPlayerBaseSpeed() {return m_playerBaseSpeed;}
		void setPlayerRegeneration(Uint16 regeneration);
		SDL_INLINE Uint16 getPlayerRegeneration() {return m_playerRegeneration;}
		void setPlayerOfflineTraining(Uint16 offlineTime);
		SDL_INLINE Uint16 getPlayerOfflineTraining() {return m_playerOfflineTraining;}

		void setPlayerExpBonus(double expBonus);
		void setPlayerTournamentFactor(Sint32 factor) {m_playerTournamentFactor = factor;}
		void setPlayerExpBonus(Uint16 baseXp, Uint16 voucherXp, Uint16 grindingXp, Uint16 storeXp, Uint16 huntingXp);
		SDL_INLINE double getPlayerExpBonus() {return m_playerExpBonus;}
		SDL_INLINE Uint16 getPlayerBaseXpGain() {return m_playerBaseXpGain;}
		SDL_INLINE Sint32 getPlayerTournamentFactor() {return m_playerTournamentFactor;}
		SDL_INLINE Uint16 getPlayerVoucherXpGain() {return m_playerVoucherXpGain;}
		SDL_INLINE Uint16 getPlayerGrindingXpGain() {return m_playerGrindingXpGain;}
		SDL_INLINE Uint16 getPlayerStoreXpGain() {return m_playerStoreXpGain;}
		SDL_INLINE Uint16 getPlayerHuntingXpGain() {return m_playerHuntingXpGain;}
		
		void setPlayerSkill(Skills skill, Uint16 level, Uint16 baseLevel, Uint8 levelPercent);
		SDL_INLINE Uint16 getPlayerSkillLevel(Skills skill) {return m_playerSkillsLevel[skill];}
		SDL_INLINE Uint16 getPlayerSkillBaseLevel(Skills skill) {return m_playerSkillsBaseLevel[skill];}
		SDL_INLINE Uint8 getPlayerSkillLevelPercent(Skills skill) {return m_playerSkillsLevelPercent[skill];}

		SDL_INLINE void setServerBeat(Uint16 serverBeat) {m_serverBeat = serverBeat;}
		SDL_INLINE Uint16 getServerBeat() {return m_serverBeat;}
		SDL_INLINE void setGameTime(Uint16 gameTime) {m_gameTime = gameTime;}
		SDL_INLINE Uint16 getGameTime() {return m_gameTime;}
		
		SDL_INLINE void setIcons(Uint32 icons) {m_icons = icons;}
		SDL_INLINE Uint32 getIcons() {return m_icons;}

		SDL_INLINE void resetCachedStats() {m_cached_stats = 0;}
		SDL_INLINE void resetCachedSkills() {m_cached_skills = 0;}
		SDL_INLINE bool hasCachedStat(Stats_Cached_State flag) {return (m_cached_stats & flag) != 0;}
		SDL_INLINE bool hasCachedSkill(Skills_Cached_State flag) {return (m_cached_skills & flag) != 0;}

		SDL_INLINE void setCanReportBugs(bool canReportBugs) {m_canReportBugs = canReportBugs;}
		SDL_INLINE bool getCanReportBugs() {return m_canReportBugs;}

		SDL_INLINE void setExpertPvpMode(bool expertPvpMode) {m_expertPvpMode = expertPvpMode;}
		SDL_INLINE bool getExpertPvpMode() {return m_expertPvpMode;}
		SDL_INLINE void setCanChangePvpFrames(bool canChangePvpFrames) {m_canChangePvpFrames = canChangePvpFrames;}
		SDL_INLINE bool getCanChangePvpFrames() {return m_canChangePvpFrames;}
		SDL_INLINE void setHaveExivaRestrictions(bool haveExivaRestrictions) {m_haveExivaRestrictions = haveExivaRestrictions;}
		SDL_INLINE bool getHaveExivaRestrictions() {return m_haveExivaRestrictions;}
		SDL_INLINE void setTournamentEnabled(bool tournamentEnabled) {m_tournamentEnabled = tournamentEnabled;}
		SDL_INLINE bool getTournamentEnabled() {return m_tournamentEnabled;}
		
		SDL_INLINE void setStoreUrl(std::string storeUrl) {m_storeUrl = std::move(storeUrl);}
		SDL_INLINE std::string& getStoreUrl() {return m_storeUrl;}
		SDL_INLINE void setStorePackages(Uint16 storePackages) {m_storePackages = storePackages;}
		SDL_INLINE Uint16 getStorePackages() {return m_storePackages;}
		
		SDL_INLINE void setResourceBalance(ResourceBalanceTypes type, Uint64 balance) {m_resourceBalances[type] = balance;}
		SDL_INLINE Uint64 getResourceBalance(ResourceBalanceTypes type) {std::map<ResourceBalanceTypes, Uint64>::iterator it = m_resourceBalances.find(type); if(it != m_resourceBalances.end()) return it->second; return 0;}
		SDL_INLINE void setCharmPoints(Sint32 charmPoints) {m_playerCharmPoints = charmPoints;}
		SDL_INLINE Sint32 getCharmPoints() {return m_playerCharmPoints;}
		
	protected:
		std::bitset<GAME_FEATURE_LAST> m_gameFeatures;
		std::vector<Direction> m_autoWalkDirections;
		std::vector<std::pair<Uint32, Uint64>> m_expTable;
		std::map<ResourceBalanceTypes, Uint64> m_resourceBalances;
		std::string m_storeUrl;

		ItemUI* m_inventoryItem[SLOT_LAST] = {};
		Container* m_containers[GAME_MAX_CONTAINERS] = {};

		Uint64 m_playerExperience = 0;
		double m_playerExpBonus = 0.0;
		double m_playerExpSpeed = 0.0;
		double m_playerCapacity = 0.0;
		double m_playerTotalCapacity = 0.0;

		Uint32 m_localPlayerId = 0;
		Uint32 m_attackId = 0;
		Uint32 m_followId = 0;
		Uint32 m_selectId = 0;
		Uint32 m_sequence = 0;
		Uint32 m_playerHealth = 100;
		Uint32 m_playerMaxHealth = 100;
		Uint32 m_playerMana = 0;
		Uint32 m_playerMaxMana = 0;
		Uint32 m_icons = 0;
		Uint32 m_cancelWalkCounter = 0;

		Sint32 m_playerTournamentFactor = 0;
		Sint32 m_playerCharmPoints = 0;

		Position m_autoWalkDestination;
		Position m_limitWalkDestination;
		Position m_lastCancelWalkPos;

		Uint16 m_playerLevel = 1;
		Uint16 m_playerMagicLevel = 0;
		Uint16 m_playerBaseMagicLevel = 0;
		Uint16 m_playerStamina = 2520;
		Uint16 m_playerBaseSpeed = 200;
		Uint16 m_playerRegeneration = 0;
		Uint16 m_playerOfflineTraining = 0;
		Uint16 m_playerBaseXpGain = 100;
		Uint16 m_playerVoucherXpGain = 0;
		Uint16 m_playerGrindingXpGain = 0;
		Uint16 m_playerStoreXpGain = 0;
		Uint16 m_playerHuntingXpGain = 100;
		Uint16 m_playerSkillsLevel[Skills_LastAdditionalSkill];
		Uint16 m_playerSkillsBaseLevel[Skills_LastAdditionalSkill];
		Uint16 m_serverBeat = 50;
		Uint16 m_storePackages = 25;
		Uint16 m_gameTime = 58;
		Uint16 m_cached_stats = 0;
		Uint16 m_cached_skills = 0;

		Uint8 m_playerHealthPercent = 100;
		Uint8 m_playerManaPercent = 100;
		Uint8 m_playerLevelPercent = 0;
		Uint8 m_playerMagicLevelPercent = 0;
		Uint8 m_playerSkillsLevelPercent[Skills_LastAdditionalSkill];
		Uint8 m_playerSoul = 0;

		Uint8 m_playerMovement = DIRECTION_INVALID;
		Uint8 m_playerCurrentDir = DIRECTION_INVALID;
		Uint8 m_playerLastDir = DIRECTION_INVALID;

		bool m_canReportBugs = false;
		bool m_expertPvpMode = false;
		bool m_canChangePvpFrames = true;
		bool m_haveExivaRestrictions = false;
		bool m_tournamentEnabled = false;
};

#endif /* __FILE_GAME_h_ */
