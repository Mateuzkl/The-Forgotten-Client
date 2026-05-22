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

#ifndef __FILE_GUI_UTIL_h_
#define __FILE_GUI_UTIL_h_

#include "../defines.h"

//Main window
void UTIL_createMainWindow();

//Enter Game window
void UTIL_createEnterGame();

//Protocol Options window
void UTIL_protocolOptions();

//Options window
void UTIL_options();
void UTIL_generalOptions();
void UTIL_graphicsOptions();
void UTIL_advancedGraphicsOptions();
void UTIL_consoleOptions();
void UTIL_hotkeyOptions();

//Help window
void UTIL_help();

//Info window
void UTIL_info();

//Exit window
void UTIL_exitWarning();

//Logout window
void UTIL_logout();

//Message Box window
void UTIL_messageBox(const std::string& title, const std::string& message);

//Motd window
void UTIL_createMotd();

//Character List window
void UTIL_createCharacterList();

//Quest Log window
void UTIL_createQuestLog(Uint32 questId, std::vector<QuestLogDetail>& questLogElements);
void UTIL_createQuestine(Uint16 questId, std::vector<QuestLineDetail>& questLineElements);

//Death window
void UTIL_deathWindow(Uint8 deathType, Uint8 penalty);

//Read Write window
void UTIL_createReadWriteWindow(Uint32 windowId, void* item, Uint16 maxLen, const std::string& text, const std::string& writer, const std::string& date);
void UTIL_createReadWriteWindow(Uint8 doorId, Uint32 windowId, const std::string& text);

//Chat Channels window
void UTIL_createChannels(std::vector<ChannelDetail>& channels);

//Outfit window
void UTIL_createOutfitWindow(Uint16 lookType, Uint8 lookHead, Uint8 lookBody, Uint8 lookLegs, Uint8 lookFeet, Uint8 lookAddons, Uint16 lookMount, std::vector<OutfitDetail>& outfits, std::vector<MountDetail>& mounts);

//Item Move window
void UTIL_createItemMove();

//Modal Dialog Window
void UTIL_createModalDialog(Uint32 windowId, bool priority, const std::string& title, const std::string& message, Uint8 enterButtonId, Uint8 escapeButtonId, std::vector<std::pair<std::string, Uint8>>& buttons, std::vector<std::pair<std::string, Uint8>>& choices);

//Ignore List Window
void UTIL_createIgnoreList();
void UTIL_toggleIgnore(const std::string& playerName);
bool UTIL_onBlackList(const std::string& playerName);
bool UTIL_onWhiteList(const std::string& playerName);

//Tutorial Hint window
void UTIL_createTutorialHint(Uint32 hintId);

//Server Browser window
void UTIL_createServerBrowser();

//Minimap widget
void UTIL_createMinimapPanel();

//Health widget
void UTIL_createHealthPanel();
void UTIL_updateHealthPanel();
void UTIL_flashHealthPanel();

//Inventory widget
void UTIL_createInventoryPanel(bool minimized = false);
void UTIL_updateInventoryPanel();
void UTIL_flashQuestsButton();
void UTIL_flashFollowButton();

//Buttons widget
void UTIL_createButtonsPanel();

//Skills widget
void UTIL_toggleSkillsWindow();
void UTIL_createSkillsPopupMenu(Sint32 x, Sint32 y);
void UTIL_updateSkillsWindowStats();
void UTIL_updateSkillsWindowSkills();

//Battle + Party widget
void UTIL_toggleBattleWindow();
void UTIL_createBattlePopupMenu(Sint32 x, Sint32 y);
void UTIL_sortBattleWindow();
void UTIL_addBattleCreature(void* creature);
void UTIL_removeBattleCreature(void* creature);
void UTIL_resetBattleCreatures();
void UTIL_refreshBattleWindow();
void UTIL_flashBattleWindow();
void UTIL_togglePartyWindow();
void UTIL_refreshPartyWindow();

//Boss widget
bool UTIL_handleExtendedOpcode(Uint8 opcode, const std::string& payload);
void UTIL_resetBossEntries();
void UTIL_recreateBossWindow(class GUI_Container* container);
void UTIL_toggleBossWindow();

//Tasks widget
bool UTIL_handleTaskExtendedOpcode(Uint8 opcode, const std::string& payload);
void UTIL_resetTaskEntries();
void UTIL_recreateTaskTrackerWindow(class GUI_Container* container);
void UTIL_toggleTaskTrackerWindow();

//Game store window
bool UTIL_handleGameStoreExtendedOpcode(Uint8 opcode, const std::string& payload);
void UTIL_createGameStoreWindow();
void UTIL_toggleGameStore();
void UTIL_resetGameStore();
void gameStore_Events(Uint32 event, Sint32 status);

//VIP widget
void UTIL_toggleVipWindow();
void UTIL_createVipPopupMenu(Sint32 x, Sint32 y, VipAction& action);
void UTIL_resetVipPlayers();
void UTIL_addVipPlayer(Uint32 playerGUID, const std::string& playerName, const std::string& description, Uint32 iconId, bool notifyLogin, Uint8 status, std::vector<Uint8>& groups);
void UTIL_changeVipStatus(Uint32 playerGUID, Uint8 status);
void UTIL_changeVipGroups(std::vector<VipGroups>& groups, Uint8 createGroupsLeft);
bool UTIL_haveVipPlayer(const std::string& name);

//Container widget
void UTIL_createContainerWindow(Uint8 index);

//Trade widget
void UTIL_createTradeWindow(bool counter, const std::string& name, void* itemsData);
void UTIL_closeTradeWindow();

//Shop widget
void UTIL_createShopWindow(const std::string& name, Uint16 currencyId, void* itemsData);
void UTIL_createShopPopupMenu(Sint32 x, Sint32 y);
void UTIL_goodsShopWindow(Uint64 playerMoney, void* itemsData);
void UTIL_closeShopWindow();
void UTIL_updateShopWindow();

//Spells widget
void UTIL_toggleSpellsWindow();

#endif /* __FILE_GUI_UTIL_h_ */
