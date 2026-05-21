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

#include "GUI_UTIL.h"
#include "../engine.h"
#include "../GUI_Elements/GUI_Panel.h"
#include "../GUI_Elements/GUI_PanelWindow.h"
#include "../GUI_Elements/GUI_Button.h"
#include "../GUI_Elements/GUI_Icon.h"
#include "../GUI_Elements/GUI_StaticImage.h"
#include "../game.h"

#define BUTTONS_SKILLS_TITLE "Skills"
#define BUTTONS_SKILLS_DESCRIPTION1 "Open skills window"
#define BUTTONS_SKILLS_DESCRIPTION2 "Close skills window"
#define BUTTONS_SKILLS_EVENTID 1000
#define BUTTONS_BATTLE_TITLE "Battle"
#define BUTTONS_BATTLE_DESCRIPTION1 "Open battle list"
#define BUTTONS_BATTLE_DESCRIPTION2 "Close battle list"
#define BUTTONS_BATTLE_EVENTID 1001
#define BUTTONS_VIP_TITLE "VIP"
#define BUTTONS_VIP_DESCRIPTION1 "Open VIP list"
#define BUTTONS_VIP_DESCRIPTION2 "Close VIP list"
#define BUTTONS_VIP_EVENTID 1002
#define BUTTONS_LOGOUT_TITLE "Logout"
#define BUTTONS_LOGOUT_DESCRIPTION "Leave the game"
#define BUTTONS_LOGOUT_EVENTID 1003
#define BUTTONS_QUESTLOG_DESCRIPTION "Open quest log"
#define BUTTONS_QUESTLOG_EVENTID 1004
#define BUTTONS_OPTIONS_DESCRIPTION "Open options"
#define BUTTONS_OPTIONS_EVENTID 1005
#define BUTTONS_HELP_DESCRIPTION "Open help menu"
#define BUTTONS_HELP_EVENTID 1006
#define BUTTONS_UNJUSTIFIED_DESCRIPTION1 "Open unjustifiedpoints window"
#define BUTTONS_UNJUSTIFIED_DESCRIPTION2 "Close unjustifiedpoints window"
#define BUTTONS_UNJUSTIFIED_EVENTID 1007
#define BUTTONS_PREY_DESCRIPTION1 "Open prey window"
#define BUTTONS_PREY_DESCRIPTION2 "Close prey window"
#define BUTTONS_PREY_EVENTID 1008
#define BUTTONS_SPELLS_DESCRIPTION1 "Open spell list"
#define BUTTONS_SPELLS_DESCRIPTION2 "Close spell list"
#define BUTTONS_SPELLS_EVENTID 1009
#define BUTTONS_COMPENDIUM_DESCRIPTION "Open compendium window"
#define BUTTONS_COMPENDIUM_EVENTID 1010
#define BUTTONS_REWARDWALL_DESCRIPTION "Open reward wall"
#define BUTTONS_REWARDWALL_EVENTID 1011
#define BUTTONS_CYCLOPEDIA_DESCRIPTION "Open Tibia Cyclopedia"
#define BUTTONS_CYCLOPEDIA_EVENTID 1012
#define BUTTONS_ANALYTICS_DESCRIPTION1 "Open analytics selector windows"
#define BUTTONS_ANALYTICS_DESCRIPTION2 "Close analytics selector windows"
#define BUTTONS_ANALYTICS_EVENTID 1013
#define BUTTONS_TOURNAMENT_DESCRIPTION "Open tournament window"
#define BUTTONS_TOURNAMENT_EVENTID 1014
#define BUTTONS_LEADERBOARD_DESCRIPTION1 "Open tournament leaderboard window"
#define BUTTONS_LEADERBOARD_DESCRIPTION2 "Close tournament leaderboard window"
#define BUTTONS_LEADERBOARD_DESCRIPTION3 "Opens tournament window. Only available on Tournament game worlds."
#define BUTTONS_LEADERBOARD_EVENTID 1015
#define BUTTONS_FRIENDS_DESCRIPTION "Open friends window"
#define BUTTONS_FRIENDS_EVENTID 1016
#define BUTTONS_PARTY_DESCRIPTION1 "Open party list"
#define BUTTONS_PARTY_DESCRIPTION2 "Close party list"
#define BUTTONS_PARTY_EVENTID 1017

extern Engine g_engine;
extern Game g_game;

extern bool g_haveSkillsOpen;
extern bool g_haveBattleOpen;
extern bool g_haveVIPOpen;
extern bool g_havePartyOpen;

bool CheckSkillsWindowOpen() {return g_haveSkillsOpen;}
bool CheckBattleWindowOpen() {return g_haveBattleOpen;}
bool CheckVIPWindowOpen() {return g_haveVIPOpen;}
bool CheckUnjustifiedWindowOpen() {return false;}
bool CheckPreyWindowOpen() {return false;}
bool CheckSpellsWindowOpen() {return false;}
bool CheckAnalyticsWindowOpen() {return false;}
bool CheckLeaderboardWindowOpen() {return false;}
bool CheckPartyWindowOpen() {return g_havePartyOpen;}

void buttons_Events(Uint32 event, Sint32)
{
	switch(event)
	{
		case BUTTONS_SKILLS_EVENTID: UTIL_toggleSkillsWindow(); break;
		case BUTTONS_BATTLE_EVENTID: UTIL_toggleBattleWindow(); break;
		case BUTTONS_VIP_EVENTID: UTIL_toggleVipWindow(); break;
		case BUTTONS_LOGOUT_EVENTID: UTIL_logout(); break;
		case BUTTONS_QUESTLOG_EVENTID: g_game.sendOpenQuestLog(); break;
		case BUTTONS_OPTIONS_EVENTID: UTIL_options(); break;
		case BUTTONS_HELP_EVENTID: UTIL_help(); break;
		case BUTTONS_UNJUSTIFIED_EVENTID: break;
		case BUTTONS_PREY_EVENTID: break;
		case BUTTONS_SPELLS_EVENTID: break;
		case BUTTONS_COMPENDIUM_EVENTID: break;
		case BUTTONS_REWARDWALL_EVENTID: break;
		case BUTTONS_CYCLOPEDIA_EVENTID: break;
		case BUTTONS_ANALYTICS_EVENTID: break;
		case BUTTONS_TOURNAMENT_EVENTID: break;
		case BUTTONS_LEADERBOARD_EVENTID: break;
		case BUTTONS_FRIENDS_EVENTID: break;
		case BUTTONS_PARTY_EVENTID: UTIL_togglePartyWindow(); break;
		default: break;
	}
}

void UTIL_createButtonsPanel()
{
	bool newWindow;
	GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_BUTTONS);
	if(pPanel)
	{
		newWindow = false;
		pPanel->clearChilds();
	}
	else
	{
		newWindow = true;
		if(g_clientVersion >= 1215)
			pPanel = new GUI_PanelWindow(iRect(0, 0, GAME_PANEL_FIXED_WIDTH - 4, 70), false, GUI_PANEL_WINDOW_BUTTONS);
		else if(g_clientVersion >= 1102)
			pPanel = new GUI_PanelWindow(iRect(0, 0, GAME_PANEL_FIXED_WIDTH - 4, 48), false, GUI_PANEL_WINDOW_BUTTONS);
		else
			pPanel = new GUI_PanelWindow(iRect(0, 0, GAME_PANEL_FIXED_WIDTH - 4, 26), false, GUI_PANEL_WINDOW_BUTTONS);
	}

	if(g_clientVersion >= 1220)
	{
		GUI_RadioIcon* newRadioIcon = new GUI_RadioIcon(iRect(8, 3, GUI_UI_ICON_SKILLS_UP_W, GUI_UI_ICON_SKILLS_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_SKILLS_UP_X, GUI_UI_ICON_SKILLS_UP_Y, GUI_UI_ICON_SKILLS_DOWN_X, GUI_UI_ICON_SKILLS_DOWN_Y, 0, BUTTONS_SKILLS_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckSkillsWindowOpen, BUTTONS_SKILLS_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_SKILLS_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(30, 3, GUI_UI_ICON_BATTLE_UP_W, GUI_UI_ICON_BATTLE_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_BATTLE_UP_X, GUI_UI_ICON_BATTLE_UP_Y, GUI_UI_ICON_BATTLE_DOWN_X, GUI_UI_ICON_BATTLE_DOWN_Y, 0, BUTTONS_BATTLE_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckBattleWindowOpen, BUTTONS_BATTLE_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_BATTLE_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(74, 3, GUI_UI_ICON_VIP_UP_W, GUI_UI_ICON_VIP_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_VIP_UP_X, GUI_UI_ICON_VIP_UP_Y, GUI_UI_ICON_VIP_DOWN_X, GUI_UI_ICON_VIP_DOWN_Y, 0, BUTTONS_VIP_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckVIPWindowOpen, BUTTONS_VIP_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_VIP_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(30, 25, GUI_UI_ICON_UNJUSTIFIED_UP_W, GUI_UI_ICON_UNJUSTIFIED_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_UNJUSTIFIED_UP_X, GUI_UI_ICON_UNJUSTIFIED_UP_Y, GUI_UI_ICON_UNJUSTIFIED_DOWN_X, GUI_UI_ICON_UNJUSTIFIED_DOWN_Y, 0, BUTTONS_UNJUSTIFIED_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckUnjustifiedWindowOpen, BUTTONS_UNJUSTIFIED_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_UNJUSTIFIED_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		if(g_game.hasGameFeature(GAME_FEATURE_PREY))
		{
			newRadioIcon = new GUI_RadioIcon(iRect(52, 25, GUI_UI_ICON_PREYWIDGET_UP_W, GUI_UI_ICON_PREYWIDGET_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_PREYWIDGET_UP_X, GUI_UI_ICON_PREYWIDGET_UP_Y, GUI_UI_ICON_PREYWIDGET_DOWN_X, GUI_UI_ICON_PREYWIDGET_DOWN_Y, 0, BUTTONS_PREY_DESCRIPTION1);
			newRadioIcon->setRadioEventCallback(&CheckPreyWindowOpen, BUTTONS_PREY_DESCRIPTION2);
			newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_PREY_EVENTID);
			newRadioIcon->startEvents();
			pPanel->addChild(newRadioIcon);
		}
		newRadioIcon = new GUI_RadioIcon(iRect(8, 25, GUI_UI_ICON_SPELLIST_UP_W, GUI_UI_ICON_SPELLIST_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_SPELLIST_UP_X, GUI_UI_ICON_SPELLIST_UP_Y, GUI_UI_ICON_SPELLIST_DOWN_X, GUI_UI_ICON_SPELLIST_DOWN_Y, 0, BUTTONS_SPELLS_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckSpellsWindowOpen, BUTTONS_SPELLS_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_SPELLS_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(74, 25, GUI_UI_ICON_ANALYTICS_UP_W, GUI_UI_ICON_ANALYTICS_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_ANALYTICS_UP_X, GUI_UI_ICON_ANALYTICS_UP_Y, GUI_UI_ICON_ANALYTICS_DOWN_X, GUI_UI_ICON_ANALYTICS_DOWN_Y, 0, BUTTONS_ANALYTICS_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckAnalyticsWindowOpen, BUTTONS_ANALYTICS_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_ANALYTICS_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		if(g_game.getTournamentEnabled())
		{
			newRadioIcon = new GUI_RadioIcon(iRect(96, 25, GUI_UI_ICON_LEADERBOARD_UP_W, GUI_UI_ICON_LEADERBOARD_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_LEADERBOARD_UP_X, GUI_UI_ICON_LEADERBOARD_UP_Y, GUI_UI_ICON_LEADERBOARD_DOWN_X, GUI_UI_ICON_LEADERBOARD_DOWN_Y, 0, BUTTONS_LEADERBOARD_DESCRIPTION1);
			newRadioIcon->setRadioEventCallback(&CheckLeaderboardWindowOpen, BUTTONS_LEADERBOARD_DESCRIPTION2);
			newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_LEADERBOARD_EVENTID);
			newRadioIcon->startEvents();
			pPanel->addChild(newRadioIcon);
		}
		else
		{
			GUI_StaticImage* newImage = new GUI_StaticImage(iRect(96, 25, GUI_UI_ICON_LEADERBOARD_DISABLED_W, GUI_UI_ICON_LEADERBOARD_DISABLED_H), GUI_UI_IMAGE, GUI_UI_ICON_LEADERBOARD_DISABLED_X, GUI_UI_ICON_LEADERBOARD_DISABLED_Y, 0, BUTTONS_LEADERBOARD_DESCRIPTION3);
			newImage->startEvents();
			pPanel->addChild(newImage);
		}
		newRadioIcon = new GUI_RadioIcon(iRect(52, 3, GUI_UI_ICON_PARTYLIST_UP_W, GUI_UI_ICON_PARTYLIST_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_PARTYLIST_UP_X, GUI_UI_ICON_PARTYLIST_UP_Y, GUI_UI_ICON_PARTYLIST_DOWN_X, GUI_UI_ICON_PARTYLIST_DOWN_Y, 0, BUTTONS_PARTY_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckPartyWindowOpen, BUTTONS_PARTY_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_PARTY_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		GUI_Icon* newIcon = new GUI_Icon(iRect(96, 3, GUI_UI_ICON_QUESTLOG_UP_W, GUI_UI_ICON_QUESTLOG_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_QUESTLOG_UP_X, GUI_UI_ICON_QUESTLOG_UP_Y, GUI_UI_ICON_QUESTLOG_DOWN_X, GUI_UI_ICON_QUESTLOG_DOWN_Y, 0, BUTTONS_QUESTLOG_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_QUESTLOG_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(8, 47, GUI_UI_ICON_REWARDWALL_UP_W, GUI_UI_ICON_REWARDWALL_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_REWARDWALL_UP_X, GUI_UI_ICON_REWARDWALL_UP_Y, GUI_UI_ICON_REWARDWALL_DOWN_X, GUI_UI_ICON_REWARDWALL_DOWN_Y, 0, BUTTONS_REWARDWALL_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_REWARDWALL_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(30, 47, GUI_UI_ICON_COMPENDIUM_UP_W, GUI_UI_ICON_COMPENDIUM_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_COMPENDIUM_UP_X, GUI_UI_ICON_COMPENDIUM_UP_Y, GUI_UI_ICON_COMPENDIUM_DOWN_X, GUI_UI_ICON_COMPENDIUM_DOWN_Y, 0, BUTTONS_COMPENDIUM_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_COMPENDIUM_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(52, 47, GUI_UI_ICON_CYCLOPEDIA_UP_W, GUI_UI_ICON_CYCLOPEDIA_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_CYCLOPEDIA_UP_X, GUI_UI_ICON_CYCLOPEDIA_UP_Y, GUI_UI_ICON_CYCLOPEDIA_DOWN_X, GUI_UI_ICON_CYCLOPEDIA_DOWN_Y, 0, BUTTONS_CYCLOPEDIA_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_CYCLOPEDIA_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(74, 47, GUI_UI_ICON_TOURNAMENT_UP_W, GUI_UI_ICON_TOURNAMENT_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_TOURNAMENT_UP_X, GUI_UI_ICON_TOURNAMENT_UP_Y, GUI_UI_ICON_TOURNAMENT_DOWN_X, GUI_UI_ICON_TOURNAMENT_DOWN_Y, 0, BUTTONS_TOURNAMENT_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_TOURNAMENT_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(96, 47, GUI_UI_ICON_FRIENDS_UP_W, GUI_UI_ICON_FRIENDS_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_FRIENDS_UP_X, GUI_UI_ICON_FRIENDS_UP_Y, GUI_UI_ICON_FRIENDS_DOWN_X, GUI_UI_ICON_FRIENDS_DOWN_Y, 0, BUTTONS_FRIENDS_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_FRIENDS_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(124, 3, GUI_UI_ICON_OPTIONS_UP_W, GUI_UI_ICON_OPTIONS_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_OPTIONS_UP_X, GUI_UI_ICON_OPTIONS_UP_Y, GUI_UI_ICON_OPTIONS_DOWN_X, GUI_UI_ICON_OPTIONS_DOWN_Y, 0, BUTTONS_OPTIONS_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_OPTIONS_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(146, 3, GUI_UI_ICON_LOGOUT_UP_W, GUI_UI_ICON_LOGOUT_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_LOGOUT_UP_X, GUI_UI_ICON_LOGOUT_UP_Y, GUI_UI_ICON_LOGOUT_DOWN_X, GUI_UI_ICON_LOGOUT_DOWN_Y, 0, BUTTONS_LOGOUT_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_LOGOUT_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
	}
	else if(g_clientVersion >= 1215)
	{
		GUI_RadioIcon* newRadioIcon = new GUI_RadioIcon(iRect(8, 3, GUI_UI_ICON_SKILLS_UP_W, GUI_UI_ICON_SKILLS_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_SKILLS_UP_X, GUI_UI_ICON_SKILLS_UP_Y, GUI_UI_ICON_SKILLS_DOWN_X, GUI_UI_ICON_SKILLS_DOWN_Y, 0, BUTTONS_SKILLS_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckSkillsWindowOpen, BUTTONS_SKILLS_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_SKILLS_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(30, 3, GUI_UI_ICON_BATTLE_UP_W, GUI_UI_ICON_BATTLE_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_BATTLE_UP_X, GUI_UI_ICON_BATTLE_UP_Y, GUI_UI_ICON_BATTLE_DOWN_X, GUI_UI_ICON_BATTLE_DOWN_Y, 0, BUTTONS_BATTLE_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckBattleWindowOpen, BUTTONS_BATTLE_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_BATTLE_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(52, 3, GUI_UI_ICON_VIP_UP_W, GUI_UI_ICON_VIP_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_VIP_UP_X, GUI_UI_ICON_VIP_UP_Y, GUI_UI_ICON_VIP_DOWN_X, GUI_UI_ICON_VIP_DOWN_Y, 0, BUTTONS_VIP_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckVIPWindowOpen, BUTTONS_VIP_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_VIP_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(30, 25, GUI_UI_ICON_UNJUSTIFIED_UP_W, GUI_UI_ICON_UNJUSTIFIED_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_UNJUSTIFIED_UP_X, GUI_UI_ICON_UNJUSTIFIED_UP_Y, GUI_UI_ICON_UNJUSTIFIED_DOWN_X, GUI_UI_ICON_UNJUSTIFIED_DOWN_Y, 0, BUTTONS_UNJUSTIFIED_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckUnjustifiedWindowOpen, BUTTONS_UNJUSTIFIED_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_UNJUSTIFIED_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		if(g_game.hasGameFeature(GAME_FEATURE_PREY))
		{
			newRadioIcon = new GUI_RadioIcon(iRect(52, 25, GUI_UI_ICON_PREYWIDGET_UP_W, GUI_UI_ICON_PREYWIDGET_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_PREYWIDGET_UP_X, GUI_UI_ICON_PREYWIDGET_UP_Y, GUI_UI_ICON_PREYWIDGET_DOWN_X, GUI_UI_ICON_PREYWIDGET_DOWN_Y, 0, BUTTONS_PREY_DESCRIPTION1);
			newRadioIcon->setRadioEventCallback(&CheckPreyWindowOpen, BUTTONS_PREY_DESCRIPTION2);
			newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_PREY_EVENTID);
			newRadioIcon->startEvents();
			pPanel->addChild(newRadioIcon);
		}
		newRadioIcon = new GUI_RadioIcon(iRect(8, 25, GUI_UI_ICON_SPELLIST_UP_W, GUI_UI_ICON_SPELLIST_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_SPELLIST_UP_X, GUI_UI_ICON_SPELLIST_UP_Y, GUI_UI_ICON_SPELLIST_DOWN_X, GUI_UI_ICON_SPELLIST_DOWN_Y, 0, BUTTONS_SPELLS_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckSpellsWindowOpen, BUTTONS_SPELLS_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_SPELLS_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(8, 47, GUI_UI_ICON_ANALYTICS_UP_W, GUI_UI_ICON_ANALYTICS_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_ANALYTICS_UP_X, GUI_UI_ICON_ANALYTICS_UP_Y, GUI_UI_ICON_ANALYTICS_DOWN_X, GUI_UI_ICON_ANALYTICS_DOWN_Y, 0, BUTTONS_ANALYTICS_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckAnalyticsWindowOpen, BUTTONS_ANALYTICS_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_ANALYTICS_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(52, 47, GUI_UI_ICON_LEADERBOARD_UP_W, GUI_UI_ICON_LEADERBOARD_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_LEADERBOARD_UP_X, GUI_UI_ICON_LEADERBOARD_UP_Y, GUI_UI_ICON_LEADERBOARD_DOWN_X, GUI_UI_ICON_LEADERBOARD_DOWN_Y, 0, BUTTONS_LEADERBOARD_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckLeaderboardWindowOpen, BUTTONS_LEADERBOARD_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_LEADERBOARD_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		GUI_Icon* newIcon = new GUI_Icon(iRect(74, 3, GUI_UI_ICON_QUESTLOG_UP_W, GUI_UI_ICON_QUESTLOG_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_QUESTLOG_UP_X, GUI_UI_ICON_QUESTLOG_UP_Y, GUI_UI_ICON_QUESTLOG_DOWN_X, GUI_UI_ICON_QUESTLOG_DOWN_Y, 0, BUTTONS_QUESTLOG_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_QUESTLOG_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(96, 3, GUI_UI_ICON_REWARDWALL_UP_W, GUI_UI_ICON_REWARDWALL_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_REWARDWALL_UP_X, GUI_UI_ICON_REWARDWALL_UP_Y, GUI_UI_ICON_REWARDWALL_DOWN_X, GUI_UI_ICON_REWARDWALL_DOWN_Y, 0, BUTTONS_REWARDWALL_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_REWARDWALL_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(74, 25, GUI_UI_ICON_COMPENDIUM_UP_W, GUI_UI_ICON_COMPENDIUM_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_COMPENDIUM_UP_X, GUI_UI_ICON_COMPENDIUM_UP_Y, GUI_UI_ICON_COMPENDIUM_DOWN_X, GUI_UI_ICON_COMPENDIUM_DOWN_Y, 0, BUTTONS_COMPENDIUM_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_COMPENDIUM_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(96, 25, GUI_UI_ICON_CYCLOPEDIA_UP_W, GUI_UI_ICON_CYCLOPEDIA_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_CYCLOPEDIA_UP_X, GUI_UI_ICON_CYCLOPEDIA_UP_Y, GUI_UI_ICON_CYCLOPEDIA_DOWN_X, GUI_UI_ICON_CYCLOPEDIA_DOWN_Y, 0, BUTTONS_CYCLOPEDIA_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_CYCLOPEDIA_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(30, 47, GUI_UI_ICON_TOURNAMENT_UP_W, GUI_UI_ICON_TOURNAMENT_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_TOURNAMENT_UP_X, GUI_UI_ICON_TOURNAMENT_UP_Y, GUI_UI_ICON_TOURNAMENT_DOWN_X, GUI_UI_ICON_TOURNAMENT_DOWN_Y, 0, BUTTONS_TOURNAMENT_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_TOURNAMENT_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(124, 3, GUI_UI_ICON_OPTIONS_UP_W, GUI_UI_ICON_OPTIONS_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_OPTIONS_UP_X, GUI_UI_ICON_OPTIONS_UP_Y, GUI_UI_ICON_OPTIONS_DOWN_X, GUI_UI_ICON_OPTIONS_DOWN_Y, 0, BUTTONS_OPTIONS_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_OPTIONS_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(146, 3, GUI_UI_ICON_LOGOUT_UP_W, GUI_UI_ICON_LOGOUT_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_LOGOUT_UP_X, GUI_UI_ICON_LOGOUT_UP_Y, GUI_UI_ICON_LOGOUT_DOWN_X, GUI_UI_ICON_LOGOUT_DOWN_Y, 0, BUTTONS_LOGOUT_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_LOGOUT_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
	}
	else if(g_clientVersion >= 1140)
	{
		GUI_RadioIcon* newRadioIcon = new GUI_RadioIcon(iRect(8, 3, GUI_UI_ICON_SKILLS_UP_W, GUI_UI_ICON_SKILLS_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_SKILLS_UP_X, GUI_UI_ICON_SKILLS_UP_Y, GUI_UI_ICON_SKILLS_DOWN_X, GUI_UI_ICON_SKILLS_DOWN_Y, 0, BUTTONS_SKILLS_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckSkillsWindowOpen, BUTTONS_SKILLS_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_SKILLS_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(30, 3, GUI_UI_ICON_BATTLE_UP_W, GUI_UI_ICON_BATTLE_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_BATTLE_UP_X, GUI_UI_ICON_BATTLE_UP_Y, GUI_UI_ICON_BATTLE_DOWN_X, GUI_UI_ICON_BATTLE_DOWN_Y, 0, BUTTONS_BATTLE_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckBattleWindowOpen, BUTTONS_BATTLE_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_BATTLE_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(52, 3, GUI_UI_ICON_VIP_UP_W, GUI_UI_ICON_VIP_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_VIP_UP_X, GUI_UI_ICON_VIP_UP_Y, GUI_UI_ICON_VIP_DOWN_X, GUI_UI_ICON_VIP_DOWN_Y, 0, BUTTONS_VIP_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckVIPWindowOpen, BUTTONS_VIP_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_VIP_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(30, 25, GUI_UI_ICON_UNJUSTIFIED_UP_W, GUI_UI_ICON_UNJUSTIFIED_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_UNJUSTIFIED_UP_X, GUI_UI_ICON_UNJUSTIFIED_UP_Y, GUI_UI_ICON_UNJUSTIFIED_DOWN_X, GUI_UI_ICON_UNJUSTIFIED_DOWN_Y, 0, BUTTONS_UNJUSTIFIED_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckUnjustifiedWindowOpen, BUTTONS_UNJUSTIFIED_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_UNJUSTIFIED_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		if(g_game.hasGameFeature(GAME_FEATURE_PREY))
		{
			newRadioIcon = new GUI_RadioIcon(iRect(52, 25, GUI_UI_ICON_PREYWIDGET_UP_W, GUI_UI_ICON_PREYWIDGET_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_PREYWIDGET_UP_X, GUI_UI_ICON_PREYWIDGET_UP_Y, GUI_UI_ICON_PREYWIDGET_DOWN_X, GUI_UI_ICON_PREYWIDGET_DOWN_Y, 0, BUTTONS_PREY_DESCRIPTION1);
			newRadioIcon->setRadioEventCallback(&CheckPreyWindowOpen, BUTTONS_PREY_DESCRIPTION2);
			newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_PREY_EVENTID);
			newRadioIcon->startEvents();
			pPanel->addChild(newRadioIcon);
		}
		newRadioIcon = new GUI_RadioIcon(iRect(8, 25, GUI_UI_ICON_SPELLIST_UP_W, GUI_UI_ICON_SPELLIST_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_SPELLIST_UP_X, GUI_UI_ICON_SPELLIST_UP_Y, GUI_UI_ICON_SPELLIST_DOWN_X, GUI_UI_ICON_SPELLIST_DOWN_Y, 0, BUTTONS_SPELLS_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckSpellsWindowOpen, BUTTONS_SPELLS_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_SPELLS_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(124, 25, GUI_UI_ICON_ANALYTICS_UP_W, GUI_UI_ICON_ANALYTICS_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_ANALYTICS_UP_X, GUI_UI_ICON_ANALYTICS_UP_Y, GUI_UI_ICON_ANALYTICS_DOWN_X, GUI_UI_ICON_ANALYTICS_DOWN_Y, 0, BUTTONS_ANALYTICS_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckAnalyticsWindowOpen, BUTTONS_ANALYTICS_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_ANALYTICS_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		GUI_Icon* newIcon = new GUI_Icon(iRect(74, 3, GUI_UI_ICON_QUESTLOG_UP_W, GUI_UI_ICON_QUESTLOG_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_QUESTLOG_UP_X, GUI_UI_ICON_QUESTLOG_UP_Y, GUI_UI_ICON_QUESTLOG_DOWN_X, GUI_UI_ICON_QUESTLOG_DOWN_Y, 0, BUTTONS_QUESTLOG_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_QUESTLOG_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(96, 3, GUI_UI_ICON_REWARDWALL_UP_W, GUI_UI_ICON_REWARDWALL_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_REWARDWALL_UP_X, GUI_UI_ICON_REWARDWALL_UP_Y, GUI_UI_ICON_REWARDWALL_DOWN_X, GUI_UI_ICON_REWARDWALL_DOWN_Y, 0, BUTTONS_REWARDWALL_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_REWARDWALL_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(74, 25, GUI_UI_ICON_COMPENDIUM_UP_W, GUI_UI_ICON_COMPENDIUM_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_COMPENDIUM_UP_X, GUI_UI_ICON_COMPENDIUM_UP_Y, GUI_UI_ICON_COMPENDIUM_DOWN_X, GUI_UI_ICON_COMPENDIUM_DOWN_Y, 0, BUTTONS_COMPENDIUM_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_COMPENDIUM_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(96, 25, GUI_UI_ICON_CYCLOPEDIA_UP_W, GUI_UI_ICON_CYCLOPEDIA_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_CYCLOPEDIA_UP_X, GUI_UI_ICON_CYCLOPEDIA_UP_Y, GUI_UI_ICON_CYCLOPEDIA_DOWN_X, GUI_UI_ICON_CYCLOPEDIA_DOWN_Y, 0, BUTTONS_CYCLOPEDIA_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_CYCLOPEDIA_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(124, 3, GUI_UI_ICON_OPTIONS_LEGACY_UP_W, GUI_UI_ICON_OPTIONS_LEGACY_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_OPTIONS_LEGACY_UP_X, GUI_UI_ICON_OPTIONS_LEGACY_UP_Y, GUI_UI_ICON_OPTIONS_LEGACY_DOWN_X, GUI_UI_ICON_OPTIONS_LEGACY_DOWN_Y, 0, BUTTONS_OPTIONS_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_OPTIONS_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(146, 3, GUI_UI_ICON_LOGOUT_UP_W, GUI_UI_ICON_LOGOUT_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_LOGOUT_UP_X, GUI_UI_ICON_LOGOUT_UP_Y, GUI_UI_ICON_LOGOUT_DOWN_X, GUI_UI_ICON_LOGOUT_DOWN_Y, 0, BUTTONS_LOGOUT_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_LOGOUT_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
	}
	else if(g_clientVersion >= 1132)
	{
		GUI_RadioIcon* newRadioIcon = new GUI_RadioIcon(iRect(19, 3, GUI_UI_ICON_SKILLS_UP_W, GUI_UI_ICON_SKILLS_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_SKILLS_UP_X, GUI_UI_ICON_SKILLS_UP_Y, GUI_UI_ICON_SKILLS_DOWN_X, GUI_UI_ICON_SKILLS_DOWN_Y, 0, BUTTONS_SKILLS_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckSkillsWindowOpen, BUTTONS_SKILLS_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_SKILLS_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(41, 3, GUI_UI_ICON_BATTLE_UP_W, GUI_UI_ICON_BATTLE_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_BATTLE_UP_X, GUI_UI_ICON_BATTLE_UP_Y, GUI_UI_ICON_BATTLE_DOWN_X, GUI_UI_ICON_BATTLE_DOWN_Y, 0, BUTTONS_BATTLE_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckBattleWindowOpen, BUTTONS_BATTLE_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_BATTLE_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(63, 3, GUI_UI_ICON_VIP_UP_W, GUI_UI_ICON_VIP_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_VIP_UP_X, GUI_UI_ICON_VIP_UP_Y, GUI_UI_ICON_VIP_DOWN_X, GUI_UI_ICON_VIP_DOWN_Y, 0, BUTTONS_VIP_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckVIPWindowOpen, BUTTONS_VIP_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_VIP_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(63, 25, GUI_UI_ICON_UNJUSTIFIED_UP_W, GUI_UI_ICON_UNJUSTIFIED_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_UNJUSTIFIED_UP_X, GUI_UI_ICON_UNJUSTIFIED_UP_Y, GUI_UI_ICON_UNJUSTIFIED_DOWN_X, GUI_UI_ICON_UNJUSTIFIED_DOWN_Y, 0, BUTTONS_UNJUSTIFIED_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckUnjustifiedWindowOpen, BUTTONS_UNJUSTIFIED_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_UNJUSTIFIED_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		if(g_game.hasGameFeature(GAME_FEATURE_PREY))
		{
			newRadioIcon = new GUI_RadioIcon(iRect(41, 25, GUI_UI_ICON_PREYWIDGET_UP_W, GUI_UI_ICON_PREYWIDGET_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_PREYWIDGET_UP_X, GUI_UI_ICON_PREYWIDGET_UP_Y, GUI_UI_ICON_PREYWIDGET_DOWN_X, GUI_UI_ICON_PREYWIDGET_DOWN_Y, 0, BUTTONS_PREY_DESCRIPTION1);
			newRadioIcon->setRadioEventCallback(&CheckPreyWindowOpen, BUTTONS_PREY_DESCRIPTION2);
			newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_PREY_EVENTID);
			newRadioIcon->startEvents();
			pPanel->addChild(newRadioIcon);
		}
		newRadioIcon = new GUI_RadioIcon(iRect(85, 3, GUI_UI_ICON_SPELLIST_UP_W, GUI_UI_ICON_SPELLIST_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_SPELLIST_UP_X, GUI_UI_ICON_SPELLIST_UP_Y, GUI_UI_ICON_SPELLIST_DOWN_X, GUI_UI_ICON_SPELLIST_DOWN_Y, 0, BUTTONS_SPELLS_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckSpellsWindowOpen, BUTTONS_SPELLS_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_SPELLS_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		GUI_Icon* newIcon = new GUI_Icon(iRect(19, 25, GUI_UI_ICON_QUESTLOG_UP_W, GUI_UI_ICON_QUESTLOG_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_QUESTLOG_UP_X, GUI_UI_ICON_QUESTLOG_UP_Y, GUI_UI_ICON_QUESTLOG_DOWN_X, GUI_UI_ICON_QUESTLOG_DOWN_Y, 0, BUTTONS_QUESTLOG_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_QUESTLOG_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(85, 25, GUI_UI_ICON_COMPENDIUM_UP_W, GUI_UI_ICON_COMPENDIUM_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_COMPENDIUM_UP_X, GUI_UI_ICON_COMPENDIUM_UP_Y, GUI_UI_ICON_COMPENDIUM_DOWN_X, GUI_UI_ICON_COMPENDIUM_DOWN_Y, 0, BUTTONS_COMPENDIUM_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_COMPENDIUM_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(124, 3, GUI_UI_ICON_OPTIONS_LEGACY_UP_W, GUI_UI_ICON_OPTIONS_LEGACY_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_OPTIONS_LEGACY_UP_X, GUI_UI_ICON_OPTIONS_LEGACY_UP_Y, GUI_UI_ICON_OPTIONS_LEGACY_DOWN_X, GUI_UI_ICON_OPTIONS_LEGACY_DOWN_Y, 0, BUTTONS_OPTIONS_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_OPTIONS_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(146, 3, GUI_UI_ICON_LOGOUT_UP_W, GUI_UI_ICON_LOGOUT_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_LOGOUT_UP_X, GUI_UI_ICON_LOGOUT_UP_Y, GUI_UI_ICON_LOGOUT_DOWN_X, GUI_UI_ICON_LOGOUT_DOWN_Y, 0, BUTTONS_LOGOUT_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_LOGOUT_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
	}
	else if(g_clientVersion >= 1130)
	{
		GUI_RadioIcon* newRadioIcon = new GUI_RadioIcon(iRect(19, 3, GUI_UI_ICON_SKILLS_UP_W, GUI_UI_ICON_SKILLS_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_SKILLS_UP_X, GUI_UI_ICON_SKILLS_UP_Y, GUI_UI_ICON_SKILLS_DOWN_X, GUI_UI_ICON_SKILLS_DOWN_Y, 0, BUTTONS_SKILLS_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckSkillsWindowOpen, BUTTONS_SKILLS_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_SKILLS_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(41, 3, GUI_UI_ICON_BATTLE_UP_W, GUI_UI_ICON_BATTLE_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_BATTLE_UP_X, GUI_UI_ICON_BATTLE_UP_Y, GUI_UI_ICON_BATTLE_DOWN_X, GUI_UI_ICON_BATTLE_DOWN_Y, 0, BUTTONS_BATTLE_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckBattleWindowOpen, BUTTONS_BATTLE_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_BATTLE_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(63, 3, GUI_UI_ICON_VIP_UP_W, GUI_UI_ICON_VIP_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_VIP_UP_X, GUI_UI_ICON_VIP_UP_Y, GUI_UI_ICON_VIP_DOWN_X, GUI_UI_ICON_VIP_DOWN_Y, 0, BUTTONS_VIP_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckVIPWindowOpen, BUTTONS_VIP_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_VIP_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(63, 25, GUI_UI_ICON_UNJUSTIFIED_UP_W, GUI_UI_ICON_UNJUSTIFIED_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_UNJUSTIFIED_UP_X, GUI_UI_ICON_UNJUSTIFIED_UP_Y, GUI_UI_ICON_UNJUSTIFIED_DOWN_X, GUI_UI_ICON_UNJUSTIFIED_DOWN_Y, 0, BUTTONS_UNJUSTIFIED_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckUnjustifiedWindowOpen, BUTTONS_UNJUSTIFIED_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_UNJUSTIFIED_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		if(g_game.hasGameFeature(GAME_FEATURE_PREY))
		{
			newRadioIcon = new GUI_RadioIcon(iRect(41, 25, GUI_UI_ICON_PREYWIDGET_UP_W, GUI_UI_ICON_PREYWIDGET_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_PREYWIDGET_UP_X, GUI_UI_ICON_PREYWIDGET_UP_Y, GUI_UI_ICON_PREYWIDGET_DOWN_X, GUI_UI_ICON_PREYWIDGET_DOWN_Y, 0, BUTTONS_PREY_DESCRIPTION1);
			newRadioIcon->setRadioEventCallback(&CheckPreyWindowOpen, BUTTONS_PREY_DESCRIPTION2);
			newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_PREY_EVENTID);
			newRadioIcon->startEvents();
			pPanel->addChild(newRadioIcon);
		}
		newRadioIcon = new GUI_RadioIcon(iRect(85, 3, GUI_UI_ICON_SPELLIST_UP_W, GUI_UI_ICON_SPELLIST_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_SPELLIST_UP_X, GUI_UI_ICON_SPELLIST_UP_Y, GUI_UI_ICON_SPELLIST_DOWN_X, GUI_UI_ICON_SPELLIST_DOWN_Y, 0, BUTTONS_SPELLS_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckSpellsWindowOpen, BUTTONS_SPELLS_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_SPELLS_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		GUI_Icon* newIcon = new GUI_Icon(iRect(19, 25, GUI_UI_ICON_QUESTLOG_UP_W, GUI_UI_ICON_QUESTLOG_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_QUESTLOG_UP_X, GUI_UI_ICON_QUESTLOG_UP_Y, GUI_UI_ICON_QUESTLOG_DOWN_X, GUI_UI_ICON_QUESTLOG_DOWN_Y, 0, BUTTONS_QUESTLOG_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_QUESTLOG_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(124, 3, GUI_UI_ICON_OPTIONS_LEGACY_UP_W, GUI_UI_ICON_OPTIONS_LEGACY_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_OPTIONS_LEGACY_UP_X, GUI_UI_ICON_OPTIONS_LEGACY_UP_Y, GUI_UI_ICON_OPTIONS_LEGACY_DOWN_X, GUI_UI_ICON_OPTIONS_LEGACY_DOWN_Y, 0, BUTTONS_OPTIONS_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_OPTIONS_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(146, 3, GUI_UI_ICON_LOGOUT_UP_W, GUI_UI_ICON_LOGOUT_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_LOGOUT_UP_X, GUI_UI_ICON_LOGOUT_UP_Y, GUI_UI_ICON_LOGOUT_DOWN_X, GUI_UI_ICON_LOGOUT_DOWN_Y, 0, BUTTONS_LOGOUT_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_LOGOUT_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);

	}
	else if(g_clientVersion >= 1102)
	{
		//I think this should be actually 11.01, every website says the prey system was introduced in 11.02
		//but I reversed the 11.01 client and there's prey system already implemented on it
		GUI_RadioIcon* newRadioIcon = new GUI_RadioIcon(iRect(30, 3, GUI_UI_ICON_SKILLS_UP_W, GUI_UI_ICON_SKILLS_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_SKILLS_UP_X, GUI_UI_ICON_SKILLS_UP_Y, GUI_UI_ICON_SKILLS_DOWN_X, GUI_UI_ICON_SKILLS_DOWN_Y, 0, BUTTONS_SKILLS_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckSkillsWindowOpen, BUTTONS_SKILLS_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_SKILLS_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(52, 3, GUI_UI_ICON_BATTLE_UP_W, GUI_UI_ICON_BATTLE_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_BATTLE_UP_X, GUI_UI_ICON_BATTLE_UP_Y, GUI_UI_ICON_BATTLE_DOWN_X, GUI_UI_ICON_BATTLE_DOWN_Y, 0, BUTTONS_BATTLE_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckBattleWindowOpen, BUTTONS_BATTLE_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_BATTLE_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(74, 3, GUI_UI_ICON_VIP_UP_W, GUI_UI_ICON_VIP_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_VIP_UP_X, GUI_UI_ICON_VIP_UP_Y, GUI_UI_ICON_VIP_DOWN_X, GUI_UI_ICON_VIP_DOWN_Y, 0, BUTTONS_VIP_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckVIPWindowOpen, BUTTONS_VIP_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_VIP_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(52, 25, GUI_UI_ICON_UNJUSTIFIED_UP_W, GUI_UI_ICON_UNJUSTIFIED_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_UNJUSTIFIED_UP_X, GUI_UI_ICON_UNJUSTIFIED_UP_Y, GUI_UI_ICON_UNJUSTIFIED_DOWN_X, GUI_UI_ICON_UNJUSTIFIED_DOWN_Y, 0, BUTTONS_UNJUSTIFIED_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckUnjustifiedWindowOpen, BUTTONS_UNJUSTIFIED_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_UNJUSTIFIED_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		if(g_game.hasGameFeature(GAME_FEATURE_PREY))
		{
			newRadioIcon = new GUI_RadioIcon(iRect(74, 25, GUI_UI_ICON_PREYWIDGET_UP_W, GUI_UI_ICON_PREYWIDGET_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_PREYWIDGET_UP_X, GUI_UI_ICON_PREYWIDGET_UP_Y, GUI_UI_ICON_PREYWIDGET_DOWN_X, GUI_UI_ICON_PREYWIDGET_DOWN_Y, 0, BUTTONS_PREY_DESCRIPTION1);
			newRadioIcon->setRadioEventCallback(&CheckPreyWindowOpen, BUTTONS_PREY_DESCRIPTION2);
			newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_PREY_EVENTID);
			newRadioIcon->startEvents();
			pPanel->addChild(newRadioIcon);
		}
		GUI_Icon* newIcon = new GUI_Icon(iRect(30, 25, GUI_UI_ICON_QUESTLOG_UP_W, GUI_UI_ICON_QUESTLOG_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_QUESTLOG_UP_X, GUI_UI_ICON_QUESTLOG_UP_Y, GUI_UI_ICON_QUESTLOG_DOWN_X, GUI_UI_ICON_QUESTLOG_DOWN_Y, 0, BUTTONS_QUESTLOG_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_QUESTLOG_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(124, 3, GUI_UI_ICON_OPTIONS_LEGACY_UP_W, GUI_UI_ICON_OPTIONS_LEGACY_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_OPTIONS_LEGACY_UP_X, GUI_UI_ICON_OPTIONS_LEGACY_UP_Y, GUI_UI_ICON_OPTIONS_LEGACY_DOWN_X, GUI_UI_ICON_OPTIONS_LEGACY_DOWN_Y, 0, BUTTONS_OPTIONS_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_OPTIONS_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(146, 3, GUI_UI_ICON_LOGOUT_UP_W, GUI_UI_ICON_LOGOUT_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_LOGOUT_UP_X, GUI_UI_ICON_LOGOUT_UP_Y, GUI_UI_ICON_LOGOUT_DOWN_X, GUI_UI_ICON_LOGOUT_DOWN_Y, 0, BUTTONS_LOGOUT_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_LOGOUT_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
	}
	else if(g_clientVersion >= 1053)
	{
		GUI_RadioIcon* newRadioIcon = new GUI_RadioIcon(iRect(8, 3, GUI_UI_ICON_SKILLS_UP_W, GUI_UI_ICON_SKILLS_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_SKILLS_UP_X, GUI_UI_ICON_SKILLS_UP_Y, GUI_UI_ICON_SKILLS_DOWN_X, GUI_UI_ICON_SKILLS_DOWN_Y, 0, BUTTONS_SKILLS_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckSkillsWindowOpen, BUTTONS_SKILLS_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_SKILLS_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(30, 3, GUI_UI_ICON_BATTLE_UP_W, GUI_UI_ICON_BATTLE_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_BATTLE_UP_X, GUI_UI_ICON_BATTLE_UP_Y, GUI_UI_ICON_BATTLE_DOWN_X, GUI_UI_ICON_BATTLE_DOWN_Y, 0, BUTTONS_BATTLE_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckBattleWindowOpen, BUTTONS_BATTLE_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_BATTLE_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(52, 3, GUI_UI_ICON_VIP_UP_W, GUI_UI_ICON_VIP_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_VIP_UP_X, GUI_UI_ICON_VIP_UP_Y, GUI_UI_ICON_VIP_DOWN_X, GUI_UI_ICON_VIP_DOWN_Y, 0, BUTTONS_VIP_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckVIPWindowOpen, BUTTONS_VIP_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_VIP_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(96, 3, GUI_UI_ICON_UNJUSTIFIED_UP_W, GUI_UI_ICON_UNJUSTIFIED_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_UNJUSTIFIED_UP_X, GUI_UI_ICON_UNJUSTIFIED_UP_Y, GUI_UI_ICON_UNJUSTIFIED_DOWN_X, GUI_UI_ICON_UNJUSTIFIED_DOWN_Y, 0, BUTTONS_UNJUSTIFIED_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckUnjustifiedWindowOpen, BUTTONS_UNJUSTIFIED_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_UNJUSTIFIED_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		GUI_Icon* newIcon = new GUI_Icon(iRect(74, 3, GUI_UI_ICON_QUESTLOG_UP_W, GUI_UI_ICON_QUESTLOG_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_QUESTLOG_UP_X, GUI_UI_ICON_QUESTLOG_UP_Y, GUI_UI_ICON_QUESTLOG_DOWN_X, GUI_UI_ICON_QUESTLOG_DOWN_Y, 0, BUTTONS_QUESTLOG_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_QUESTLOG_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(124, 3, GUI_UI_ICON_OPTIONS_LEGACY_UP_W, GUI_UI_ICON_OPTIONS_LEGACY_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_OPTIONS_LEGACY_UP_X, GUI_UI_ICON_OPTIONS_LEGACY_UP_Y, GUI_UI_ICON_OPTIONS_LEGACY_DOWN_X, GUI_UI_ICON_OPTIONS_LEGACY_DOWN_Y, 0, BUTTONS_OPTIONS_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_OPTIONS_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(146, 3, GUI_UI_ICON_LOGOUT_UP_W, GUI_UI_ICON_LOGOUT_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_LOGOUT_UP_X, GUI_UI_ICON_LOGOUT_UP_Y, GUI_UI_ICON_LOGOUT_DOWN_X, GUI_UI_ICON_LOGOUT_DOWN_Y, 0, BUTTONS_LOGOUT_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_LOGOUT_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
	}
	else if(g_clientVersion >= 1000)
	{
		GUI_RadioIcon* newRadioIcon = new GUI_RadioIcon(iRect(8, 3, GUI_UI_ICON_SKILLS_UP_W, GUI_UI_ICON_SKILLS_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_SKILLS_UP_X, GUI_UI_ICON_SKILLS_UP_Y, GUI_UI_ICON_SKILLS_DOWN_X, GUI_UI_ICON_SKILLS_DOWN_Y, 0, BUTTONS_SKILLS_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckSkillsWindowOpen, BUTTONS_SKILLS_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_SKILLS_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(30, 3, GUI_UI_ICON_BATTLE_UP_W, GUI_UI_ICON_BATTLE_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_BATTLE_UP_X, GUI_UI_ICON_BATTLE_UP_Y, GUI_UI_ICON_BATTLE_DOWN_X, GUI_UI_ICON_BATTLE_DOWN_Y, 0, BUTTONS_BATTLE_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckBattleWindowOpen, BUTTONS_BATTLE_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_BATTLE_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		newRadioIcon = new GUI_RadioIcon(iRect(52, 3, GUI_UI_ICON_VIP_UP_W, GUI_UI_ICON_VIP_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_VIP_UP_X, GUI_UI_ICON_VIP_UP_Y, GUI_UI_ICON_VIP_DOWN_X, GUI_UI_ICON_VIP_DOWN_Y, 0, BUTTONS_VIP_DESCRIPTION1);
		newRadioIcon->setRadioEventCallback(&CheckVIPWindowOpen, BUTTONS_VIP_DESCRIPTION2);
		newRadioIcon->setButtonEventCallback(&buttons_Events, BUTTONS_VIP_EVENTID);
		newRadioIcon->startEvents();
		pPanel->addChild(newRadioIcon);
		GUI_Icon* newIcon = new GUI_Icon(iRect(74, 3, GUI_UI_ICON_QUESTLOG_UP_W, GUI_UI_ICON_QUESTLOG_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_QUESTLOG_UP_X, GUI_UI_ICON_QUESTLOG_UP_Y, GUI_UI_ICON_QUESTLOG_DOWN_X, GUI_UI_ICON_QUESTLOG_DOWN_Y, 0, BUTTONS_QUESTLOG_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_QUESTLOG_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(102, 3, GUI_UI_ICON_OPTIONS_LEGACY_UP_W, GUI_UI_ICON_OPTIONS_LEGACY_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_OPTIONS_LEGACY_UP_X, GUI_UI_ICON_OPTIONS_LEGACY_UP_Y, GUI_UI_ICON_OPTIONS_LEGACY_DOWN_X, GUI_UI_ICON_OPTIONS_LEGACY_DOWN_Y, 0, BUTTONS_OPTIONS_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_OPTIONS_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(124, 3, GUI_UI_ICON_HELP_UP_W, GUI_UI_ICON_HELP_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_HELP_UP_X, GUI_UI_ICON_HELP_UP_Y, GUI_UI_ICON_HELP_DOWN_X, GUI_UI_ICON_HELP_DOWN_Y, 0, BUTTONS_HELP_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_HELP_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
		newIcon = new GUI_Icon(iRect(146, 3, GUI_UI_ICON_LOGOUT_UP_W, GUI_UI_ICON_LOGOUT_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_LOGOUT_UP_X, GUI_UI_ICON_LOGOUT_UP_Y, GUI_UI_ICON_LOGOUT_DOWN_X, GUI_UI_ICON_LOGOUT_DOWN_Y, 0, BUTTONS_LOGOUT_DESCRIPTION);
		newIcon->setButtonEventCallback(&buttons_Events, BUTTONS_LOGOUT_EVENTID);
		newIcon->startEvents();
		pPanel->addChild(newIcon);
	}
	else
	{
		GUI_RadioButton* newRadioButton = new GUI_RadioButton(iRect(8, 3, GUI_UI_BUTTON_34PX_GRAY_UP_W, GUI_UI_BUTTON_34PX_GRAY_UP_H), BUTTONS_SKILLS_TITLE, 0, BUTTONS_SKILLS_DESCRIPTION1);
		newRadioButton->setRadioEventCallback(&CheckSkillsWindowOpen, BUTTONS_SKILLS_DESCRIPTION2);
		newRadioButton->setButtonEventCallback(&buttons_Events, BUTTONS_SKILLS_EVENTID);
		newRadioButton->startEvents();
		pPanel->addChild(newRadioButton);
		newRadioButton = new GUI_RadioButton(iRect(45, 3, GUI_UI_BUTTON_34PX_GRAY_UP_W, GUI_UI_BUTTON_34PX_GRAY_UP_H), BUTTONS_BATTLE_TITLE, 0, BUTTONS_BATTLE_DESCRIPTION1);
		newRadioButton->setRadioEventCallback(&CheckBattleWindowOpen, BUTTONS_BATTLE_DESCRIPTION2);
		newRadioButton->setButtonEventCallback(&buttons_Events, BUTTONS_BATTLE_EVENTID);
		newRadioButton->startEvents();
		pPanel->addChild(newRadioButton);
		newRadioButton = new GUI_RadioButton(iRect(82, 3, GUI_UI_BUTTON_34PX_GRAY_UP_W, GUI_UI_BUTTON_34PX_GRAY_UP_H), BUTTONS_VIP_TITLE, 0, BUTTONS_VIP_DESCRIPTION1);
		newRadioButton->setRadioEventCallback(&CheckVIPWindowOpen, BUTTONS_VIP_DESCRIPTION2);
		newRadioButton->setButtonEventCallback(&buttons_Events, BUTTONS_VIP_EVENTID);
		newRadioButton->startEvents();
		pPanel->addChild(newRadioButton);
		GUI_Button* newButton = new GUI_Button(iRect(124, 3, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), BUTTONS_LOGOUT_TITLE, 0, BUTTONS_LOGOUT_DESCRIPTION);
		newButton->setButtonEventCallback(&buttons_Events, BUTTONS_LOGOUT_EVENTID);
		newButton->startEvents();
		pPanel->addChild(newButton);
	}
	
	if(newWindow)
		g_engine.addToPanel(pPanel, GUI_PANEL_MAIN);
	else
		pPanel->checkPanels();
}
