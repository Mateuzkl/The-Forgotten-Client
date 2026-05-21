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
#include "../GUI_Elements/GUI_StaticImage.h"
#include "../GUI_Elements/GUI_Label.h"
#include "../GUI_Elements/GUI_Icon.h"
#include "../GUI_Elements/GUI_ContextMenu.h"
#include "../GUI_Elements/GUI_ScrollBar.h"
#include "../game.h"
#include "SkillsWindow.h"

#define SKILLS_TITLE "Skills"
#define SKILLS_MAXIMINI_EVENTID 1000
#define SKILLS_CLOSE_EVENTID 1001
#define SKILLS_CONFIGURE_EVENTID 1002
#define SKILLS_RESIZE_WIDTH_EVENTID 1003
#define SKILLS_RESIZE_HEIGHT_EVENTID 1004
#define SKILLS_EXIT_WINDOW_EVENTID 1005
#define SKILLS_CONTAINER_EVENTID 1006

#define SKILLS_LEVEL_TITLE "Level"
#define SKILLS_LEVEL_EVENTID 1100
#define SKILLS_LEVEL_BAR_EVENTID 1200
#define SKILLS_EXPERIENCE_TITLE "Exp."
#define SKILLS_EXPERIENCE_EVENTID 1101
#define SKILLS_XPGAIN_TITLE "XP Gain Rate"
#define SKILLS_XPGAIN_EVENTID 1102
#define SKILLS_HITPOINTS_TITLE "Hit Points"
#define SKILLS_HITPOINTS_EVENTID 1103
#define SKILLS_MANA_TITLE "Mana"
#define SKILLS_MANA_EVENTID 1104
#define SKILLS_SOUL_TITLE "Soul Points"
#define SKILLS_SOUL_EVENTID 1105
#define SKILLS_CAPACITY_TITLE "Capacity"
#define SKILLS_CAPACITY_EVENTID 1106
#define SKILLS_SPEED_TITLE "Speed"
#define SKILLS_SPEED_EVENTID 1107
#define SKILLS_FOOD_TITLE "Food"
#define SKILLS_FOOD_EVENTID 1108
#define SKILLS_STAMINA_TITLE "Stamina"
#define SKILLS_STAMINA_EVENTID 1109
#define SKILLS_STAMINA_BAR_EVENTID 1201
#define SKILLS_TRAINING_TITLE "Offline Training"
#define SKILLS_TRAINING_EVENTID 1110
#define SKILLS_TRAINING_BAR_EVENTID 1202
#define SKILLS_MLEVEL_TITLE "Magic Level"
#define SKILLS_MLEVEL_EVENTID 1111
#define SKILLS_MLEVEL_BAR_EVENTID 1203
#define SKILLS_FIST_TITLE "Fist Fighting"
#define SKILLS_FIST_EVENTID 1112
#define SKILLS_FIST_BAR_EVENTID 1204
#define SKILLS_CLUB_TITLE "Club Fighting"
#define SKILLS_CLUB_EVENTID 1113
#define SKILLS_CLUB_BAR_EVENTID 1205
#define SKILLS_SWORD_TITLE "Sword Fighting"
#define SKILLS_SWORD_EVENTID 1114
#define SKILLS_SWORD_BAR_EVENTID 1206
#define SKILLS_AXE_TITLE "Axe Fighting"
#define SKILLS_AXE_EVENTID 1115
#define SKILLS_AXE_BAR_EVENTID 1207
#define SKILLS_DISTANCE_TITLE "Distance Fighting"
#define SKILLS_DISTANCE_EVENTID 1116
#define SKILLS_DISTANCE_BAR_EVENTID 1208
#define SKILLS_SHIELDING_TITLE "Shielding"
#define SKILLS_SHIELDING_EVENTID 1117
#define SKILLS_SHIELDING_BAR_EVENTID 1209
#define SKILLS_FISHING_TITLE "Fishing"
#define SKILLS_FISHING_EVENTID 1118
#define SKILLS_FISHING_BAR_EVENTID 1210
#define SKILLS_CRITICAL_HIT_TITLE "Critical Hit:"
#define SKILLS_CRITICAL_HIT_EVENTID 1119
#define SKILLS_CRITICAL_CHANCE_TITLE "Chance"
#define SKILLS_CRITICAL_CHANCE_EVENTID 1120
#define SKILLS_CRITICAL_DAMAGE_TITLE "Extra Damage"
#define SKILLS_CRITICAL_DAMAGE_EVENTID 1121
#define SKILLS_LIFE_LEECH_TITLE "Life Leech:"
#define SKILLS_LIFE_LEECH_EVENTID 1122
#define SKILLS_LIFE_CHANCE_TITLE "Chance"
#define SKILLS_LIFE_CHANCE_EVENTID 1123
#define SKILLS_LIFE_AMOUNT_TITLE "Amount"
#define SKILLS_LIFE_AMOUNT_EVENTID 1124
#define SKILLS_MANA_LEECH_TITLE "Mana Leech:"
#define SKILLS_MANA_LEECH_EVENTID 1125
#define SKILLS_MANA_CHANCE_TITLE "Chance"
#define SKILLS_MANA_CHANCE_EVENTID 1126
#define SKILLS_MANA_AMOUNT_TITLE "Amount"
#define SKILLS_MANA_AMOUNT_EVENTID 1127

#define SKILLS_POPUP_RESETEXP "Reset Experience Counter"
#define SKILLS_POPUP_RESETEXP_EVENTID 2000
#define SKILLS_POPUP_LEVEL "Level"
#define SKILLS_POPUP_LEVEL_EVENTID 2001
#define SKILLS_POPUP_STAMINA "Stamina"
#define SKILLS_POPUP_STAMINA_EVENTID 2002
#define SKILLS_POPUP_TRAINING "Offline Training"
#define SKILLS_POPUP_TRAINING_EVENTID 2003
#define SKILLS_POPUP_MLEVEL "Magic Level"
#define SKILLS_POPUP_MLEVEL_EVENTID 2004
#define SKILLS_POPUP_FIST "Fist Fighting"
#define SKILLS_POPUP_FIST_EVENTID 2005
#define SKILLS_POPUP_CLUB "Club Fighting"
#define SKILLS_POPUP_CLUB_EVENTID 2006
#define SKILLS_POPUP_SWORD "Sword Fighting"
#define SKILLS_POPUP_SWORD_EVENTID 2007
#define SKILLS_POPUP_AXE "Axe Fighting"
#define SKILLS_POPUP_AXE_EVENTID 2008
#define SKILLS_POPUP_DISTANCE "Distance Fighting"
#define SKILLS_POPUP_DISTANCE_EVENTID 2009
#define SKILLS_POPUP_SHIELDING "Shielding"
#define SKILLS_POPUP_SHIELDING_EVENTID 2010
#define SKILLS_POPUP_FISHING "Fishing"
#define SKILLS_POPUP_FISHING_EVENTID 2011
#define SKILLS_POPUP_SHOWALL "Show all Skill Bars"
#define SKILLS_POPUP_SHOWALL_EVENTID 2012
#define SKILLS_POPUP_HIDEALL "Hide all Skill Bars"
#define SKILLS_POPUP_HIDEALL_EVENTID 2013

extern Engine g_engine;
extern Game g_game;

bool g_haveSkillsOpen = false;

void UTIL_recreateSkillsWindow(GUI_SkillsContainer* container);
void skills_Events(Uint32 event, Sint32 status)
{
	switch(event)
	{
		case SKILLS_MAXIMINI_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
			{
				GUI_Panel* parent = pPanel->getParent();
				if(!parent)
					break;

				iRect& pRect = pPanel->getOriginalRect();
				if(pRect.y2 > 19)
				{
					pPanel->setCachedHeight(pRect.y2);
					pPanel->setSize(pRect.x2, 19);
					parent->checkPanels();

					GUI_SkillsContainer* pContainer = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
					if(pContainer)
						pContainer->makeInvisible();

					GUI_Icon* pIcon = SDL_static_cast(GUI_Icon*, pPanel->getChild(SKILLS_MAXIMINI_EVENTID));
					if(pIcon)
						pIcon->setData(GUI_UI_IMAGE, GUI_UI_ICON_MAXIMIZE_WINDOW_UP_X, GUI_UI_ICON_MAXIMIZE_WINDOW_UP_Y, GUI_UI_ICON_MAXIMIZE_WINDOW_DOWN_X, GUI_UI_ICON_MAXIMIZE_WINDOW_DOWN_Y);
				}
				else
				{
					Sint32 cachedHeight = pPanel->getCachedHeight();
					parent->tryFreeHeight(cachedHeight - pRect.y2);
					pPanel->setSize(pRect.x2, cachedHeight);
					parent->checkPanels();

					GUI_SkillsContainer* pContainer = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
					if(pContainer)
						pContainer->makeVisible();

					GUI_Icon* pIcon = SDL_static_cast(GUI_Icon*, pPanel->getChild(SKILLS_MAXIMINI_EVENTID));
					if(pIcon)
						pIcon->setData(GUI_UI_IMAGE, GUI_UI_ICON_MINIMIZE_WINDOW_UP_X, GUI_UI_ICON_MINIMIZE_WINDOW_UP_Y, GUI_UI_ICON_MINIMIZE_WINDOW_DOWN_X, GUI_UI_ICON_MINIMIZE_WINDOW_DOWN_Y);
				}
			}
		}
		break;
		case SKILLS_CLOSE_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
			{
				GUI_Panel* parent = pPanel->getParent();
				if(!parent)
					break;

				parent->removePanel(pPanel);
			}
		}
		break;
		case SKILLS_CONFIGURE_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
			{
				GUI_Icon* pIcon = SDL_static_cast(GUI_Icon*, pPanel->getChild(SKILLS_CONFIGURE_EVENTID));
				if(pIcon)
				{
					iRect& iconRect = pIcon->getRect();
					UTIL_createSkillsPopupMenu(iconRect.x1, iconRect.y1 + 12);
				}
			}
		}
		break;
		case SKILLS_RESIZE_HEIGHT_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
			{
				GUI_SkillsContainer* pContainer = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
				if(pContainer)
				{
					iRect cRect = pContainer->getRect();
					cRect.y2 = status - 19;
					pContainer->setRect(cRect);
				}
			}
		}
		break;
		case SKILLS_EXIT_WINDOW_EVENTID: {g_engine.setContentWindowHeight(GUI_PANEL_WINDOW_SKILLS, status - 19); g_haveSkillsOpen = false;} break;
		case SKILLS_POPUP_RESETEXP_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
				g_game.resetPlayerExperienceTable();
		}
		break;
		case SKILLS_POPUP_LEVEL_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
			{
				GUI_SkillsContainer* pContainer = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
				if(pContainer)
				{
					g_engine.setShowLevelBar(!g_engine.getShowLevelBar());
					UTIL_recreateSkillsWindow(pContainer);
				}
			}
		}
		break;
		case SKILLS_POPUP_STAMINA_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
			{
				GUI_SkillsContainer* pContainer = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
				if(pContainer)
				{
					g_engine.setShowStaminaBar(!g_engine.getShowStaminaBar());
					UTIL_recreateSkillsWindow(pContainer);
				}
			}
		}
		break;
		case SKILLS_POPUP_TRAINING_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
			{
				GUI_SkillsContainer* pContainer = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
				if(pContainer)
				{
					g_engine.setShowTrainingBar(!g_engine.getShowTrainingBar());
					UTIL_recreateSkillsWindow(pContainer);
				}
			}
		}
		break;
		case SKILLS_POPUP_MLEVEL_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
			{
				GUI_SkillsContainer* pContainer = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
				if(pContainer)
				{
					g_engine.setShowMagLevelBar(!g_engine.getShowMagLevelBar());
					UTIL_recreateSkillsWindow(pContainer);
				}
			}
		}
		break;
		case SKILLS_POPUP_FIST_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
			{
				GUI_SkillsContainer* pContainer = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
				if(pContainer)
				{
					g_engine.setShowSkillBar(Skills_Fist, !g_engine.getShowSkillBar(Skills_Fist));
					UTIL_recreateSkillsWindow(pContainer);
				}
			}
		}
		break;
		case SKILLS_POPUP_CLUB_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
			{
				GUI_SkillsContainer* pContainer = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
				if(pContainer)
				{
					g_engine.setShowSkillBar(Skills_Club, !g_engine.getShowSkillBar(Skills_Club));
					UTIL_recreateSkillsWindow(pContainer);
				}
			}
		}
		break;
		case SKILLS_POPUP_SWORD_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
			{
				GUI_SkillsContainer* pContainer = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
				if(pContainer)
				{
					g_engine.setShowSkillBar(Skills_Sword, !g_engine.getShowSkillBar(Skills_Sword));
					UTIL_recreateSkillsWindow(pContainer);
				}
			}
		}
		break;
		case SKILLS_POPUP_AXE_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
			{
				GUI_SkillsContainer* pContainer = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
				if(pContainer)
				{
					g_engine.setShowSkillBar(Skills_Axe, !g_engine.getShowSkillBar(Skills_Axe));
					UTIL_recreateSkillsWindow(pContainer);
				}
			}
		}
		break;
		case SKILLS_POPUP_DISTANCE_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
			{
				GUI_SkillsContainer* pContainer = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
				if(pContainer)
				{
					g_engine.setShowSkillBar(Skills_Distance, !g_engine.getShowSkillBar(Skills_Distance));
					UTIL_recreateSkillsWindow(pContainer);
				}
			}
		}
		break;
		case SKILLS_POPUP_SHIELDING_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
			{
				GUI_SkillsContainer* pContainer = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
				if(pContainer)
				{
					g_engine.setShowSkillBar(Skills_Shielding, !g_engine.getShowSkillBar(Skills_Shielding));
					UTIL_recreateSkillsWindow(pContainer);
				}
			}
		}
		break;
		case SKILLS_POPUP_FISHING_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
			{
				GUI_SkillsContainer* pContainer = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
				if(pContainer)
				{
					g_engine.setShowSkillBar(Skills_Fishing, !g_engine.getShowSkillBar(Skills_Fishing));
					UTIL_recreateSkillsWindow(pContainer);
				}
			}
		}
		break;
		case SKILLS_POPUP_SHOWALL_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
			{
				GUI_SkillsContainer* pContainer = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
				if(pContainer)
				{
					g_engine.setShowLevelBar(true);
					if(g_game.hasGameFeature(GAME_FEATURE_STAMINA))
						g_engine.setShowStaminaBar(true);
					if(g_game.hasGameFeature(GAME_FEATURE_OFFLINE_TRAINING))
						g_engine.setShowTrainingBar(true);
					g_engine.setShowMagLevelBar(true);
					g_engine.setShowSkillBar(Skills_Fist, true);
					g_engine.setShowSkillBar(Skills_Club, true);
					g_engine.setShowSkillBar(Skills_Sword, true);
					g_engine.setShowSkillBar(Skills_Axe, true);
					g_engine.setShowSkillBar(Skills_Distance, true);
					g_engine.setShowSkillBar(Skills_Shielding, true);
					g_engine.setShowSkillBar(Skills_Fishing, true);
					UTIL_recreateSkillsWindow(pContainer);
				}
			}
		}
		break;
		case SKILLS_POPUP_HIDEALL_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
			if(pPanel)
			{
				GUI_SkillsContainer* pContainer = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
				if(pContainer)
				{
					g_engine.setShowLevelBar(false);
					if(g_game.hasGameFeature(GAME_FEATURE_STAMINA))
						g_engine.setShowStaminaBar(false);
					if(g_game.hasGameFeature(GAME_FEATURE_OFFLINE_TRAINING))
						g_engine.setShowTrainingBar(false);
					g_engine.setShowMagLevelBar(false);
					g_engine.setShowSkillBar(Skills_Fist, false);
					g_engine.setShowSkillBar(Skills_Club, false);
					g_engine.setShowSkillBar(Skills_Sword, false);
					g_engine.setShowSkillBar(Skills_Axe, false);
					g_engine.setShowSkillBar(Skills_Distance, false);
					g_engine.setShowSkillBar(Skills_Shielding, false);
					g_engine.setShowSkillBar(Skills_Fishing, false);
					UTIL_recreateSkillsWindow(pContainer);
				}
			}
		}
		break;
		default: break;
	}
}

Uint32 getSkillXPGainRate()
{
	Uint32 percent = 100;
	if(g_game.hasGameFeature(GAME_FEATURE_EXPERIENCE_BONUS))
	{
		if(g_game.hasGameFeature(GAME_FEATURE_DETAILED_EXPERIENCE_BONUS))
		{
			percent = SDL_static_cast(Uint32, g_game.getPlayerBaseXpGain());
			percent += SDL_static_cast(Uint32, g_game.getPlayerTournamentFactor());
			percent += SDL_static_cast(Uint32, g_game.getPlayerVoucherXpGain());
			percent += SDL_static_cast(Uint32, g_game.getPlayerGrindingXpGain());
			percent += SDL_static_cast(Uint32, g_game.getPlayerStoreXpGain());
			percent = percent * SDL_static_cast(Uint32, g_game.getPlayerHuntingXpGain()) / 100;
		}
		else
		{
			percent += SDL_static_cast(Uint32, g_game.getPlayerTournamentFactor());
			percent += SDL_static_cast(Uint32, g_game.getPlayerExpBonus() * 100.0);
		}
	}
	return percent;
}

std::string getSkillData(Uint32 skillId)
{
	std::string result;
	switch(skillId)
	{
		case SKILLS_LEVEL_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, g_game.getPlayerLevel()));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_EXPERIENCE_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%" SDL_PRIu64, g_game.getPlayerExperience());
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_XPGAIN_EVENTID:
		{
			Uint32 percent = getSkillXPGainRate();
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u%%", percent);
			result = std::string(g_buffer, SDL_static_cast(size_t, len));
		}
		break;
		case SKILLS_HITPOINTS_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", g_game.getPlayerHealth());
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_MANA_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", g_game.getPlayerMana());
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_SOUL_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, g_game.getPlayerSoul()));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_CAPACITY_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, g_game.getPlayerCapacity()));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_SPEED_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, g_game.getPlayerBaseSpeed()));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_FOOD_EVENTID:
		{
			Uint32 regen = SDL_static_cast(Uint32, g_game.getPlayerRegeneration()) / 60;
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u:%02u", (regen / 60), (regen % 60));
			result = std::string(g_buffer, SDL_static_cast(size_t, len));
		}
		break;
		case SKILLS_STAMINA_EVENTID:
		{
			Uint32 stamina = SDL_static_cast(Uint32, g_game.getPlayerStamina());
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u:%02u", (stamina / 60), (stamina % 60));
			result = std::string(g_buffer, SDL_static_cast(size_t, len));
		}
		break;
		case SKILLS_TRAINING_EVENTID:
		{
			Uint32 training = SDL_static_cast(Uint32, g_game.getPlayerOfflineTraining());
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u:%02u", (training / 60), (training % 60));
			result = std::string(g_buffer, SDL_static_cast(size_t, len));
		}
		break;
		case SKILLS_MLEVEL_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, g_game.getPlayerMagicLevel()));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_FIST_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Fist)));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_CLUB_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Club)));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_SWORD_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Sword)));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_AXE_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Axe)));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_DISTANCE_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Distance)));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_SHIELDING_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Shielding)));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_FISHING_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Fishing)));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_CRITICAL_CHANCE_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u%%", SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_CriticalChance)));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_CRITICAL_DAMAGE_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u%%", SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_CriticalDamage)));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_LIFE_CHANCE_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u%%", SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_LifeLeechChance)));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_LIFE_AMOUNT_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u%%", SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_LifeLeechAmount)));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_MANA_CHANCE_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u%%", SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_ManaLeechChance)));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		case SKILLS_MANA_AMOUNT_EVENTID:
		{
			Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u%%", SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_ManaLeechAmount)));
			result = UTIL_formatStringCommas(std::string(g_buffer, SDL_static_cast(size_t, len)));
		}
		break;
		default: break;
	}
	return result;
}

void UTIL_recreateSkillsWindow(GUI_SkillsContainer* container)
{
	if(!container)
	{
		GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
		if(pPanel)
			container = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
		if(!container)
			return;
	}
	container->clearChilds(false);

	Sint32 PosY = 0;
	GUI_SkillDescription* newDescription;
	if(g_engine.getShowLevelBar())
	{
		GUI_SkillBar* newBar = new GUI_SkillBar(iRect(10, PosY + 12, 135, 5), SKILLS_LEVEL_BAR_EVENTID);
		newBar->setPercent(g_game.getPlayerLevelPercent());
		newBar->setColor(192, 0, 0);
		container->addChild(newBar, false);
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 20), SKILLS_LEVEL_EVENTID);
		container->addChild(newDescription, false);
		PosY += 22;
	}
	else
	{
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_LEVEL_EVENTID);
		container->addChild(newDescription, false);
		PosY += 14;
	}
	newDescription->setName(false, SKILLS_LEVEL_TITLE);
	newDescription->setName(true, std::move(getSkillData(SKILLS_LEVEL_EVENTID)));
	newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_EXPERIENCE_EVENTID);
	newDescription->setName(false, SKILLS_EXPERIENCE_TITLE);
	newDescription->setName(true, std::move(getSkillData(SKILLS_EXPERIENCE_EVENTID)));
	container->addChild(newDescription, false);
	PosY += 14;
	newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_XPGAIN_EVENTID);
	newDescription->setName(false, SKILLS_XPGAIN_TITLE);
	newDescription->setName(true, std::move(getSkillData(SKILLS_XPGAIN_EVENTID)));
	Uint32 percent = getSkillXPGainRate();
	Uint32 basePercent = (g_game.hasGameFeature(GAME_FEATURE_EXPERIENCE_BONUS) ? SDL_static_cast(Uint32, g_game.getPlayerBaseXpGain()) : 100);
	if(percent > basePercent)
		newDescription->setColor(true, 68, 172, 36);
	else if(percent < basePercent)
		newDescription->setColor(true, 255, 152, 84);

	container->addChild(newDescription, false);
	PosY += 14;
	GUI_SkillSeparator* newSeparator = new GUI_SkillSeparator(iRect(10, PosY, 135, 1));
	container->addChild(newSeparator, false);
	PosY += 11;
	newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_HITPOINTS_EVENTID);
	newDescription->setName(false, SKILLS_HITPOINTS_TITLE);
	newDescription->setName(true, std::move(getSkillData(SKILLS_HITPOINTS_EVENTID)));
	container->addChild(newDescription, false);
	PosY += 14;
	newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_MANA_EVENTID);
	newDescription->setName(false, SKILLS_MANA_TITLE);
	newDescription->setName(true, std::move(getSkillData(SKILLS_MANA_EVENTID)));
	container->addChild(newDescription, false);
	PosY += 14;
	newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_CAPACITY_EVENTID);
	newDescription->setName(false, SKILLS_CAPACITY_TITLE);
	newDescription->setName(true, std::move(getSkillData(SKILLS_CAPACITY_EVENTID)));
	container->addChild(newDescription, false);
	PosY += 14;
	if(g_game.hasGameFeature(GAME_FEATURE_BASE_SKILLS))
	{
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_SPEED_EVENTID);
		newDescription->setName(false, SKILLS_SPEED_TITLE);
		newDescription->setName(true, std::move(getSkillData(SKILLS_SPEED_EVENTID)));
		container->addChild(newDescription, false);
		PosY += 14;
	}
	if(g_game.hasGameFeature(GAME_FEATURE_REGENERATION_TIME))
	{
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_FOOD_EVENTID);
		newDescription->setName(false, SKILLS_FOOD_TITLE);
		newDescription->setName(true, std::move(getSkillData(SKILLS_FOOD_EVENTID)));
		container->addChild(newDescription, false);
		PosY += 14;
	}
	if(g_game.hasGameFeature(GAME_FEATURE_STAMINA))
	{
		if(g_engine.getShowStaminaBar())
		{
			Uint32 staminaMinutes = SDL_static_cast(Uint32, g_game.getPlayerStamina());
			Uint8 staminaPercent;
			if(g_clientVersion <= 840)
				staminaPercent = SDL_static_cast(Uint8, (staminaMinutes * 100) / 3360);
			else
				staminaPercent = SDL_static_cast(Uint8, (staminaMinutes * 100) / 2520);
			GUI_SkillBar* newBar = new GUI_SkillBar(iRect(10, PosY + 12, 135, 5), SKILLS_STAMINA_BAR_EVENTID);
			newBar->setPercent(staminaPercent);
			if(g_clientVersion <= 840)
				newBar->setColor(192, 96, 0);
			else
			{
				if((g_clientVersion < 855 && staminaMinutes >= 2400) || (g_clientVersion >= 855 && staminaMinutes >= 2460))
					newBar->setColor(0, 255, 0);
				else if(staminaMinutes >= 840)
					newBar->setColor(192, 96, 0);
				else
					newBar->setColor(255, 0, 0);
			}
			container->addChild(newBar, false);
			newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 20), SKILLS_STAMINA_EVENTID);
			container->addChild(newDescription, false);
			PosY += 22;
		}
		else
		{
			newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_STAMINA_EVENTID);
			container->addChild(newDescription, false);
			PosY += 14;
		}
		newDescription->setName(false, SKILLS_STAMINA_TITLE);
		newDescription->setName(true, std::move(getSkillData(SKILLS_STAMINA_EVENTID)));
	}
	if(g_game.hasGameFeature(GAME_FEATURE_OFFLINE_TRAINING))
	{
		if(g_engine.getShowTrainingBar())
		{
			GUI_SkillBar* newBar = new GUI_SkillBar(iRect(10, PosY + 12, 135, 5), SKILLS_TRAINING_BAR_EVENTID);
			newBar->setPercent(SDL_static_cast(Uint8, (SDL_static_cast(Uint32, g_game.getPlayerOfflineTraining()) * 100) / 720));
			newBar->setColor(192, 0, 0);
			container->addChild(newBar, false);
			newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 20), SKILLS_TRAINING_EVENTID);
			container->addChild(newDescription, false);
			PosY += 22;
		}
		else
		{
			newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_TRAINING_EVENTID);
			container->addChild(newDescription, false);
			PosY += 14;
		}
		newDescription->setName(false, SKILLS_TRAINING_TITLE);
		newDescription->setName(true, std::move(getSkillData(SKILLS_TRAINING_EVENTID)));
	}
	newSeparator = new GUI_SkillSeparator(iRect(10, PosY, 135, 1));
	container->addChild(newSeparator, false);
	PosY += 11;
	if(g_engine.getShowMagLevelBar())
	{
		GUI_SkillBar* newBar = new GUI_SkillBar(iRect(10, PosY + 12, 135, 5), SKILLS_MLEVEL_BAR_EVENTID);
		newBar->setPercent(g_game.getPlayerMagicLevelPercent());
		container->addChild(newBar, false);
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 20), SKILLS_MLEVEL_EVENTID);
		container->addChild(newDescription, false);
		PosY += 22;
	}
	else
	{
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_MLEVEL_EVENTID);
		container->addChild(newDescription, false);
		PosY += 14;
	}
	newDescription->setName(false, SKILLS_MLEVEL_TITLE);
	newDescription->setName(true, std::move(getSkillData(SKILLS_MLEVEL_EVENTID)));
	Uint32 level = SDL_static_cast(Uint32, g_game.getPlayerMagicLevel());
	Uint32 baseLevel = SDL_static_cast(Uint32, g_game.getPlayerBaseMagicLevel());
	if(level > baseLevel)
		newDescription->setColor(true, 68, 172, 36);
	else if(level < baseLevel)
		newDescription->setColor(true, 255, 152, 84);

	if(g_engine.getShowSkillBar(Skills_Fist))
	{
		GUI_SkillBar* newBar = new GUI_SkillBar(iRect(10, PosY + 12, 135, 5), SKILLS_FIST_BAR_EVENTID);
		newBar->setPercent(g_game.getPlayerSkillLevelPercent(Skills_Fist));
		container->addChild(newBar, false);
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 20), SKILLS_FIST_EVENTID);
		container->addChild(newDescription, false);
		PosY += 22;
	}
	else
	{
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_FIST_EVENTID);
		container->addChild(newDescription, false);
		PosY += 14;
	}
	newDescription->setName(false, SKILLS_FIST_TITLE);
	newDescription->setName(true, std::move(getSkillData(SKILLS_FIST_EVENTID)));
	level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Fist));
	baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Fist));
	if(level > baseLevel)
		newDescription->setColor(true, 68, 172, 36);
	else if(level < baseLevel)
		newDescription->setColor(true, 255, 152, 84);

	if(g_engine.getShowSkillBar(Skills_Club))
	{
		GUI_SkillBar* newBar = new GUI_SkillBar(iRect(10, PosY + 12, 135, 5), SKILLS_CLUB_BAR_EVENTID);
		newBar->setPercent(g_game.getPlayerSkillLevelPercent(Skills_Club));
		container->addChild(newBar, false);
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 20), SKILLS_CLUB_EVENTID);
		container->addChild(newDescription, false);
		PosY += 22;
	}
	else
	{
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_CLUB_EVENTID);
		container->addChild(newDescription, false);
		PosY += 14;
	}
	newDescription->setName(false, SKILLS_CLUB_TITLE);
	newDescription->setName(true, std::move(getSkillData(SKILLS_CLUB_EVENTID)));
	level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Club));
	baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Club));
	if(level > baseLevel)
		newDescription->setColor(true, 68, 172, 36);
	else if(level < baseLevel)
		newDescription->setColor(true, 255, 152, 84);

	if(g_engine.getShowSkillBar(Skills_Sword))
	{
		GUI_SkillBar* newBar = new GUI_SkillBar(iRect(10, PosY + 12, 135, 5), SKILLS_SWORD_BAR_EVENTID);
		newBar->setPercent(g_game.getPlayerSkillLevelPercent(Skills_Sword));
		container->addChild(newBar, false);
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 20), SKILLS_SWORD_EVENTID);
		container->addChild(newDescription, false);
		PosY += 22;
	}
	else
	{
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_SWORD_EVENTID);
		container->addChild(newDescription, false);
		PosY += 14;
	}
	newDescription->setName(false, SKILLS_SWORD_TITLE);
	newDescription->setName(true, std::move(getSkillData(SKILLS_SWORD_EVENTID)));
	level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Sword));
	baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Sword));
	if(level > baseLevel)
		newDescription->setColor(true, 68, 172, 36);
	else if(level < baseLevel)
		newDescription->setColor(true, 255, 152, 84);

	if(g_engine.getShowSkillBar(Skills_Axe))
	{
		GUI_SkillBar* newBar = new GUI_SkillBar(iRect(10, PosY + 12, 135, 5), SKILLS_AXE_BAR_EVENTID);
		newBar->setPercent(g_game.getPlayerSkillLevelPercent(Skills_Axe));
		container->addChild(newBar, false);
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 20), SKILLS_AXE_EVENTID);
		container->addChild(newDescription, false);
		PosY += 22;
	}
	else
	{
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_AXE_EVENTID);
		container->addChild(newDescription, false);
		PosY += 14;
	}
	newDescription->setName(false, SKILLS_AXE_TITLE);
	newDescription->setName(true, std::move(getSkillData(SKILLS_AXE_EVENTID)));
	level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Axe));
	baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Axe));
	if(level > baseLevel)
		newDescription->setColor(true, 68, 172, 36);
	else if(level < baseLevel)
		newDescription->setColor(true, 255, 152, 84);

	if(g_engine.getShowSkillBar(Skills_Distance))
	{
		GUI_SkillBar* newBar = new GUI_SkillBar(iRect(10, PosY + 12, 135, 5), SKILLS_DISTANCE_BAR_EVENTID);
		newBar->setPercent(g_game.getPlayerSkillLevelPercent(Skills_Distance));
		container->addChild(newBar, false);
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 20), SKILLS_DISTANCE_EVENTID);
		container->addChild(newDescription, false);
		PosY += 22;
	}
	else
	{
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_DISTANCE_EVENTID);
		container->addChild(newDescription, false);
		PosY += 14;
	}
	newDescription->setName(false, SKILLS_DISTANCE_TITLE);
	newDescription->setName(true, std::move(getSkillData(SKILLS_DISTANCE_EVENTID)));
	level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Distance));
	baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Distance));
	if(level > baseLevel)
		newDescription->setColor(true, 68, 172, 36);
	else if(level < baseLevel)
		newDescription->setColor(true, 255, 152, 84);

	if(g_engine.getShowSkillBar(Skills_Shielding))
	{
		GUI_SkillBar* newBar = new GUI_SkillBar(iRect(10, PosY + 12, 135, 5), SKILLS_SHIELDING_BAR_EVENTID);
		newBar->setPercent(g_game.getPlayerSkillLevelPercent(Skills_Shielding));
		container->addChild(newBar, false);
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 20), SKILLS_SHIELDING_EVENTID);
		container->addChild(newDescription, false);
		PosY += 22;
	}
	else
	{
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_SHIELDING_EVENTID);
		container->addChild(newDescription, false);
		PosY += 14;
	}
	newDescription->setName(false, SKILLS_SHIELDING_TITLE);
	newDescription->setName(true, std::move(getSkillData(SKILLS_SHIELDING_EVENTID)));
	level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Shielding));
	baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Shielding));
	if(level > baseLevel)
		newDescription->setColor(true, 68, 172, 36);
	else if(level < baseLevel)
		newDescription->setColor(true, 255, 152, 84);

	if(g_engine.getShowSkillBar(Skills_Fishing))
	{
		GUI_SkillBar* newBar = new GUI_SkillBar(iRect(10, PosY + 12, 135, 5), SKILLS_FISHING_BAR_EVENTID);
		newBar->setPercent(g_game.getPlayerSkillLevelPercent(Skills_Fishing));
		container->addChild(newBar, false);
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 20), SKILLS_FISHING_EVENTID);
		container->addChild(newDescription, false);
		PosY += 22;
	}
	else
	{
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_FISHING_EVENTID);
		container->addChild(newDescription, false);
		PosY += 14;
	}
	newDescription->setName(false, SKILLS_FISHING_TITLE);
	newDescription->setName(true, std::move(getSkillData(SKILLS_FISHING_EVENTID)));
	level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Fishing));
	baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Fishing));
	if(level > baseLevel)
		newDescription->setColor(true, 68, 172, 36);
	else if(level < baseLevel)
		newDescription->setColor(true, 255, 152, 84);

	if(g_game.hasGameFeature(GAME_FEATURE_ADDITIONAL_SKILLS))
	{
		newSeparator = new GUI_SkillSeparator(iRect(10, PosY, 135, 1));
		container->addChild(newSeparator, false);
		PosY += 11;
		GUI_Label* newLabel = new GUI_Label(iRect(10, PosY, 0, 12), SKILLS_CRITICAL_HIT_TITLE, 0, 175, 175, 175);
		container->addChild(newLabel, false);
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 40), SKILLS_CRITICAL_HIT_EVENTID);
		container->addChild(newDescription, false);
		PosY += 14;
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_CRITICAL_CHANCE_EVENTID);
		newDescription->setName(false, SKILLS_CRITICAL_CHANCE_TITLE);
		newDescription->setName(true, std::move(getSkillData(SKILLS_CRITICAL_CHANCE_EVENTID)));
		container->addChild(newDescription, false);
		PosY += 14;
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_CRITICAL_DAMAGE_EVENTID);
		newDescription->setName(false, SKILLS_CRITICAL_DAMAGE_TITLE);
		newDescription->setName(true, std::move(getSkillData(SKILLS_CRITICAL_DAMAGE_EVENTID)));
		container->addChild(newDescription, false);
		PosY += 14;
		newLabel = new GUI_Label(iRect(10, PosY, 0, 12), SKILLS_LIFE_LEECH_TITLE, 0, 175, 175, 175);
		container->addChild(newLabel, false);
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 40), SKILLS_LIFE_LEECH_EVENTID);
		container->addChild(newDescription, false);
		PosY += 14;
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_LIFE_CHANCE_EVENTID);
		newDescription->setName(false, SKILLS_LIFE_CHANCE_TITLE);
		newDescription->setName(true, std::move(getSkillData(SKILLS_LIFE_CHANCE_EVENTID)));
		container->addChild(newDescription, false);
		PosY += 14;
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_LIFE_AMOUNT_EVENTID);
		newDescription->setName(false, SKILLS_LIFE_AMOUNT_TITLE);
		newDescription->setName(true, std::move(getSkillData(SKILLS_LIFE_AMOUNT_EVENTID)));
		container->addChild(newDescription, false);
		PosY += 14;
		newLabel = new GUI_Label(iRect(10, PosY, 0, 12), SKILLS_MANA_LEECH_TITLE, 0, 175, 175, 175);
		container->addChild(newLabel, false);
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 40), SKILLS_MANA_LEECH_EVENTID);
		container->addChild(newDescription, false);
		PosY += 14;
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_MANA_CHANCE_EVENTID);
		newDescription->setName(false, SKILLS_MANA_CHANCE_TITLE);
		newDescription->setName(true, std::move(getSkillData(SKILLS_MANA_CHANCE_EVENTID)));
		container->addChild(newDescription, false);
		PosY += 14;
		newDescription = new GUI_SkillDescription(iRect(10, PosY, 135, 12), SKILLS_MANA_AMOUNT_EVENTID);
		newDescription->setName(false, SKILLS_MANA_AMOUNT_TITLE);
		newDescription->setName(true, std::move(getSkillData(SKILLS_MANA_AMOUNT_EVENTID)));
		container->addChild(newDescription, false);
		PosY += 14;
	}
	container->setAsMaxHeight();
	container->validateScrollBar();
}

void UTIL_updateSkillsWindowStats()
{
	GUI_SkillsContainer* container = NULL;
	GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
	if(pPanel)
		container = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
	if(!container)
		return;

	GUI_SkillDescription* pDescription;
	GUI_SkillBar* pBar;
	if(g_game.hasCachedStat(CACHED_STAT_LEVEL))
	{
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_LEVEL_EVENTID));
		if(pDescription)
			pDescription->setName(true, std::move(getSkillData(SKILLS_LEVEL_EVENTID)));

		pBar = SDL_static_cast(GUI_SkillBar*, container->getChild(SKILLS_LEVEL_BAR_EVENTID));
		if(pBar)
			pBar->setPercent(g_game.getPlayerLevelPercent());
	}

	if(g_game.hasCachedStat(CACHED_STAT_EXPERIENCE))
	{
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_EXPERIENCE_EVENTID));
		if(pDescription)
			pDescription->setName(true, std::move(getSkillData(SKILLS_EXPERIENCE_EVENTID)));
	}

	if(g_game.hasCachedStat(CACHED_STAT_XPGAIN))
	{
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_XPGAIN_EVENTID));
		if(pDescription)
		{
			pDescription->setName(true, std::move(getSkillData(SKILLS_XPGAIN_EVENTID)));

			Uint32 percent = getSkillXPGainRate();
			Uint32 basePercent = (g_game.hasGameFeature(GAME_FEATURE_EXPERIENCE_BONUS) ? SDL_static_cast(Uint32, g_game.getPlayerBaseXpGain()) : 100);
			if(percent > basePercent)
				pDescription->setColor(true, 68, 172, 36);
			else if(percent < basePercent)
				pDescription->setColor(true, 255, 152, 84);
			else
				pDescription->setColor(true, 175, 175, 175);
		}
	}

	if(g_game.hasCachedStat(CACHED_STAT_HEALTH))
	{
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_HITPOINTS_EVENTID));
		if(pDescription)
			pDescription->setName(true, std::move(getSkillData(SKILLS_HITPOINTS_EVENTID)));
	}

	if(g_game.hasCachedStat(CACHED_STAT_MANA))
	{
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_MANA_EVENTID));
		if(pDescription)
			pDescription->setName(true, std::move(getSkillData(SKILLS_MANA_EVENTID)));
	}

	if(g_game.hasCachedStat(CACHED_STAT_CAPACITY))
	{
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_CAPACITY_EVENTID));
		if(pDescription)
			pDescription->setName(true, std::move(getSkillData(SKILLS_CAPACITY_EVENTID)));
	}

	if(g_game.hasCachedStat(CACHED_STAT_SPEED))
	{
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_SPEED_EVENTID));
		if(pDescription)
			pDescription->setName(true, std::move(getSkillData(SKILLS_SPEED_EVENTID)));
	}

	if(g_game.hasCachedStat(CACHED_STAT_REGENERATION))
	{
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_FOOD_EVENTID));
		if(pDescription)
			pDescription->setName(true, std::move(getSkillData(SKILLS_FOOD_EVENTID)));
	}

	if(g_game.hasCachedStat(CACHED_STAT_STAMINA))
	{
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_STAMINA_EVENTID));
		if(pDescription)
			pDescription->setName(true, std::move(getSkillData(SKILLS_STAMINA_EVENTID)));

		pBar = SDL_static_cast(GUI_SkillBar*, container->getChild(SKILLS_STAMINA_BAR_EVENTID));
		if(pBar)
		{
			Uint32 staminaMinutes = SDL_static_cast(Uint32, g_game.getPlayerStamina());
			Uint8 percent;
			if(g_clientVersion <= 840)
				percent = SDL_static_cast(Uint8, (staminaMinutes * 100) / 3360);
			else
				percent = SDL_static_cast(Uint8, (staminaMinutes * 100) / 2520);
			pBar->setPercent(percent);
			if(g_clientVersion <= 840)
				pBar->setColor(192, 96, 0);
			else
			{
				if((g_clientVersion < 855 && staminaMinutes >= 2400) || (g_clientVersion >= 855 && staminaMinutes >= 2460))
					pBar->setColor(0, 255, 0);
				else if(staminaMinutes >= 840)
					pBar->setColor(192, 96, 0);
				else
					pBar->setColor(255, 0, 0);
			}
		}
	}

	if(g_game.hasCachedStat(CACHED_STAT_TRAINING))
	{
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_TRAINING_EVENTID));
		if(pDescription)
			pDescription->setName(true, std::move(getSkillData(SKILLS_TRAINING_EVENTID)));

		pBar = SDL_static_cast(GUI_SkillBar*, container->getChild(SKILLS_TRAINING_BAR_EVENTID));
		if(pBar)
			pBar->setPercent(SDL_static_cast(Uint8, (SDL_static_cast(Uint32, g_game.getPlayerOfflineTraining()) * 100) / 720));
	}

	if(g_game.hasCachedStat(CACHED_STAT_MAGICLEVEL))
	{
		Uint32 level = SDL_static_cast(Uint32, g_game.getPlayerMagicLevel());
		Uint32 baseLevel = SDL_static_cast(Uint32, g_game.getPlayerBaseMagicLevel());
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_MLEVEL_EVENTID));
		if(pDescription)
		{
			pDescription->setName(true, std::move(getSkillData(SKILLS_MLEVEL_EVENTID)));
			if(level > baseLevel)
				pDescription->setColor(true, 68, 172, 36);
			else if(level < baseLevel)
				pDescription->setColor(true, 255, 152, 84);
			else
				pDescription->setColor(true, 175, 175, 175);
		}
		
		pBar = SDL_static_cast(GUI_SkillBar*, container->getChild(SKILLS_MLEVEL_BAR_EVENTID));
		if(pBar)
			pBar->setPercent(g_game.getPlayerMagicLevelPercent());
	}
}

void UTIL_updateSkillsWindowSkills()
{
	GUI_SkillsContainer* container = NULL;
	GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
	if(pPanel)
		container = SDL_static_cast(GUI_SkillsContainer*, pPanel->getChild(SKILLS_CONTAINER_EVENTID));
	if(!container)
		return;

	GUI_SkillDescription* pDescription;
	GUI_SkillBar* pBar;
	if(g_game.hasCachedSkill(CACHED_SKILL_FIST))
	{
		Uint32 level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Fist));
		Uint32 baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Fist));
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_FIST_EVENTID));
		if(pDescription)
		{
			pDescription->setName(true, std::move(getSkillData(SKILLS_FIST_EVENTID)));
			if(level > baseLevel)
				pDescription->setColor(true, 68, 172, 36);
			else if(level < baseLevel)
				pDescription->setColor(true, 255, 152, 84);
			else
				pDescription->setColor(true, 175, 175, 175);
		}

		pBar = SDL_static_cast(GUI_SkillBar*, container->getChild(SKILLS_FIST_BAR_EVENTID));
		if(pBar)
			pBar->setPercent(g_game.getPlayerSkillLevelPercent(Skills_Fist));
	}

	if(g_game.hasCachedSkill(CACHED_SKILL_CLUB))
	{
		Uint32 level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Club));
		Uint32 baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Club));
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_CLUB_EVENTID));
		if(pDescription)
		{
			pDescription->setName(true, std::move(getSkillData(SKILLS_CLUB_EVENTID)));
			if(level > baseLevel)
				pDescription->setColor(true, 68, 172, 36);
			else if(level < baseLevel)
				pDescription->setColor(true, 255, 152, 84);
			else
				pDescription->setColor(true, 175, 175, 175);
		}

		pBar = SDL_static_cast(GUI_SkillBar*, container->getChild(SKILLS_CLUB_BAR_EVENTID));
		if(pBar)
			pBar->setPercent(g_game.getPlayerSkillLevelPercent(Skills_Club));
	}

	if(g_game.hasCachedSkill(CACHED_SKILL_SWORD))
	{
		Uint32 level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Sword));
		Uint32 baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Sword));
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_SWORD_EVENTID));
		if(pDescription)
		{
			pDescription->setName(true, std::move(getSkillData(SKILLS_SWORD_EVENTID)));
			if(level > baseLevel)
				pDescription->setColor(true, 68, 172, 36);
			else if(level < baseLevel)
				pDescription->setColor(true, 255, 152, 84);
			else
				pDescription->setColor(true, 175, 175, 175);
		}

		pBar = SDL_static_cast(GUI_SkillBar*, container->getChild(SKILLS_SWORD_BAR_EVENTID));
		if(pBar)
			pBar->setPercent(g_game.getPlayerSkillLevelPercent(Skills_Sword));
	}

	if(g_game.hasCachedSkill(CACHED_SKILL_AXE))
	{
		Uint32 level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Axe));
		Uint32 baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Axe));
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_AXE_EVENTID));
		if(pDescription)
		{
			pDescription->setName(true, std::move(getSkillData(SKILLS_AXE_EVENTID)));
			if(level > baseLevel)
				pDescription->setColor(true, 68, 172, 36);
			else if(level < baseLevel)
				pDescription->setColor(true, 255, 152, 84);
			else
				pDescription->setColor(true, 175, 175, 175);
		}

		pBar = SDL_static_cast(GUI_SkillBar*, container->getChild(SKILLS_AXE_BAR_EVENTID));
		if(pBar)
			pBar->setPercent(g_game.getPlayerSkillLevelPercent(Skills_Axe));
	}

	if(g_game.hasCachedSkill(CACHED_SKILL_DISTANCE))
	{
		Uint32 level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Distance));
		Uint32 baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Distance));
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_DISTANCE_EVENTID));
		if(pDescription)
		{
			pDescription->setName(true, std::move(getSkillData(SKILLS_DISTANCE_EVENTID)));
			if(level > baseLevel)
				pDescription->setColor(true, 68, 172, 36);
			else if(level < baseLevel)
				pDescription->setColor(true, 255, 152, 84);
			else
				pDescription->setColor(true, 175, 175, 175);
		}

		pBar = SDL_static_cast(GUI_SkillBar*, container->getChild(SKILLS_DISTANCE_BAR_EVENTID));
		if(pBar)
			pBar->setPercent(g_game.getPlayerSkillLevelPercent(Skills_Distance));
	}

	if(g_game.hasCachedSkill(CACHED_SKILL_SHIELDING))
	{
		Uint32 level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Shielding));
		Uint32 baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Shielding));
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_SHIELDING_EVENTID));
		if(pDescription)
		{
			pDescription->setName(true, std::move(getSkillData(SKILLS_SHIELDING_EVENTID)));
			if(level > baseLevel)
				pDescription->setColor(true, 68, 172, 36);
			else if(level < baseLevel)
				pDescription->setColor(true, 255, 152, 84);
			else
				pDescription->setColor(true, 175, 175, 175);
		}

		pBar = SDL_static_cast(GUI_SkillBar*, container->getChild(SKILLS_SHIELDING_BAR_EVENTID));
		if(pBar)
			pBar->setPercent(g_game.getPlayerSkillLevelPercent(Skills_Shielding));
	}

	if(g_game.hasCachedSkill(CACHED_SKILL_FISHING))
	{
		Uint32 level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Fishing));
		Uint32 baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Fishing));
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_FISHING_EVENTID));
		if(pDescription)
		{
			pDescription->setName(true, std::move(getSkillData(SKILLS_FISHING_EVENTID)));
			if(level > baseLevel)
				pDescription->setColor(true, 68, 172, 36);
			else if(level < baseLevel)
				pDescription->setColor(true, 255, 152, 84);
			else
				pDescription->setColor(true, 175, 175, 175);
		}

		pBar = SDL_static_cast(GUI_SkillBar*, container->getChild(SKILLS_FISHING_BAR_EVENTID));
		if(pBar)
			pBar->setPercent(g_game.getPlayerSkillLevelPercent(Skills_Fishing));
	}

	if(g_game.hasCachedSkill(CACHED_SKILL_CRITICAL))
	{
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_CRITICAL_CHANCE_EVENTID));
		if(pDescription)
			pDescription->setName(true, std::move(getSkillData(SKILLS_CRITICAL_CHANCE_EVENTID)));

		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_CRITICAL_DAMAGE_EVENTID));
		if(pDescription)
			pDescription->setName(true, std::move(getSkillData(SKILLS_CRITICAL_DAMAGE_EVENTID)));
	}

	if(g_game.hasCachedSkill(CACHED_SKILL_LIFELEECH))
	{
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_LIFE_CHANCE_EVENTID));
		if(pDescription)
			pDescription->setName(true, std::move(getSkillData(SKILLS_LIFE_CHANCE_EVENTID)));

		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_LIFE_AMOUNT_EVENTID));
		if(pDescription)
			pDescription->setName(true, std::move(getSkillData(SKILLS_LIFE_AMOUNT_EVENTID)));
	}

	if(g_game.hasCachedSkill(CACHED_SKILL_MANALEECH))
	{
		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_MANA_CHANCE_EVENTID));
		if(pDescription)
			pDescription->setName(true, std::move(getSkillData(SKILLS_MANA_CHANCE_EVENTID)));

		pDescription = SDL_static_cast(GUI_SkillDescription*, container->getChild(SKILLS_MANA_AMOUNT_EVENTID));
		if(pDescription)
			pDescription->setName(true, std::move(getSkillData(SKILLS_MANA_AMOUNT_EVENTID)));
	}
}

void UTIL_toggleSkillsWindow()
{
	GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_SKILLS);
	if(pPanel)
	{
		g_engine.removePanelWindow(pPanel);
		return;
	}

	Sint32 savedHeight = g_engine.getContentWindowHeight(GUI_PANEL_WINDOW_SKILLS);
	if(savedHeight < 38)
		savedHeight = 119;
	GUI_PanelWindow* newWindow = new GUI_PanelWindow(iRect(0, 0, GAME_PANEL_FIXED_WIDTH - 4, savedHeight + 19), true, GUI_PANEL_WINDOW_SKILLS, true);
	newWindow->setEventCallback(&skills_Events, SKILLS_RESIZE_WIDTH_EVENTID, SKILLS_RESIZE_HEIGHT_EVENTID, SKILLS_EXIT_WINDOW_EVENTID);
	GUI_StaticImage* newImage = new GUI_StaticImage(iRect(2, 0, GUI_UI_ICON_SKILLSLIST_W, GUI_UI_ICON_SKILLSLIST_H), GUI_UI_IMAGE, GUI_UI_ICON_SKILLSLIST_X, GUI_UI_ICON_SKILLSLIST_Y);
	newWindow->addChild(newImage);
	GUI_Icon* newIcon = new GUI_Icon(iRect(147, 0, GUI_UI_ICON_MINIMIZE_WINDOW_UP_W, GUI_UI_ICON_MINIMIZE_WINDOW_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_MINIMIZE_WINDOW_UP_X, GUI_UI_ICON_MINIMIZE_WINDOW_UP_Y, GUI_UI_ICON_MINIMIZE_WINDOW_DOWN_X, GUI_UI_ICON_MINIMIZE_WINDOW_DOWN_Y, SKILLS_MAXIMINI_EVENTID, "Maximise or minimise window");
	newIcon->setButtonEventCallback(&skills_Events, SKILLS_MAXIMINI_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(159, 0, GUI_UI_ICON_CLOSE_WINDOW_UP_W, GUI_UI_ICON_CLOSE_WINDOW_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_CLOSE_WINDOW_UP_X, GUI_UI_ICON_CLOSE_WINDOW_UP_Y, GUI_UI_ICON_CLOSE_WINDOW_DOWN_X, GUI_UI_ICON_CLOSE_WINDOW_DOWN_Y, 0, "Close this window");
	newIcon->setButtonEventCallback(&skills_Events, SKILLS_CLOSE_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(131, 0, GUI_UI_ICON_CONFIGURE_WINDOW_UP_W, GUI_UI_ICON_CONFIGURE_WINDOW_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_CONFIGURE_WINDOW_UP_X, GUI_UI_ICON_CONFIGURE_WINDOW_UP_Y, GUI_UI_ICON_CONFIGURE_WINDOW_DOWN_X, GUI_UI_ICON_CONFIGURE_WINDOW_DOWN_Y, SKILLS_CONFIGURE_EVENTID, "Click here to configure the skills window");
	newIcon->setButtonEventCallback(&skills_Events, SKILLS_CONFIGURE_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	GUI_DynamicLabel* newLabel = new GUI_DynamicLabel(iRect(19, 2, 100, 14), SKILLS_TITLE, 0, 144, 144, 144);
	newWindow->addChild(newLabel);
	GUI_SkillsContainer* newContainer = new GUI_SkillsContainer(iRect(2, 13, 168, savedHeight), newWindow, SKILLS_CONTAINER_EVENTID);
	UTIL_recreateSkillsWindow(newContainer);
	newContainer->startEvents();
	newWindow->addChild(newContainer);

	Sint32 preferredPanel = g_engine.getContentWindowParent(GUI_PANEL_WINDOW_SKILLS);
	bool added = g_engine.addToPanel(newWindow, preferredPanel);
	if(!added && preferredPanel != GUI_PANEL_RANDOM)
		added = g_engine.addToPanel(newWindow, GUI_PANEL_RANDOM);

	g_haveSkillsOpen = added;
	if(!added)
		delete newWindow;
}

void UTIL_createSkillsPopupMenu(Sint32 x, Sint32 y)
{
	GUI_ContextMenu* newMenu = new GUI_ContextMenu();
	newMenu->addContextMenu((CONTEXTMENU_STYLE_STANDARD|CONTEXTMENU_STYLE_SEPARATED), SKILLS_POPUP_RESETEXP_EVENTID, SKILLS_POPUP_RESETEXP, "");
	newMenu->addContextMenu((g_engine.getShowLevelBar() ? CONTEXTMENU_STYLE_CHECKED : CONTEXTMENU_STYLE_UNCHECKED), SKILLS_POPUP_LEVEL_EVENTID, SKILLS_POPUP_LEVEL, "");
	if(g_game.hasGameFeature(GAME_FEATURE_STAMINA))
		newMenu->addContextMenu((g_engine.getShowStaminaBar() ? CONTEXTMENU_STYLE_CHECKED : CONTEXTMENU_STYLE_UNCHECKED), SKILLS_POPUP_STAMINA_EVENTID, SKILLS_POPUP_STAMINA, "");
	if(g_game.hasGameFeature(GAME_FEATURE_OFFLINE_TRAINING))
		newMenu->addContextMenu((g_engine.getShowTrainingBar() ? CONTEXTMENU_STYLE_CHECKED : CONTEXTMENU_STYLE_UNCHECKED), SKILLS_POPUP_TRAINING_EVENTID, SKILLS_POPUP_TRAINING, "");
	newMenu->addContextMenu((g_engine.getShowMagLevelBar() ? CONTEXTMENU_STYLE_CHECKED : CONTEXTMENU_STYLE_UNCHECKED), SKILLS_POPUP_MLEVEL_EVENTID, SKILLS_POPUP_MLEVEL, "");
	newMenu->addContextMenu((g_engine.getShowSkillBar(Skills_Fist) ? CONTEXTMENU_STYLE_CHECKED : CONTEXTMENU_STYLE_UNCHECKED), SKILLS_POPUP_FIST_EVENTID, SKILLS_POPUP_FIST, "");
	newMenu->addContextMenu((g_engine.getShowSkillBar(Skills_Club) ? CONTEXTMENU_STYLE_CHECKED : CONTEXTMENU_STYLE_UNCHECKED), SKILLS_POPUP_CLUB_EVENTID, SKILLS_POPUP_CLUB, "");
	newMenu->addContextMenu((g_engine.getShowSkillBar(Skills_Sword) ? CONTEXTMENU_STYLE_CHECKED : CONTEXTMENU_STYLE_UNCHECKED), SKILLS_POPUP_SWORD_EVENTID, SKILLS_POPUP_SWORD, "");
	newMenu->addContextMenu((g_engine.getShowSkillBar(Skills_Axe) ? CONTEXTMENU_STYLE_CHECKED : CONTEXTMENU_STYLE_UNCHECKED), SKILLS_POPUP_AXE_EVENTID, SKILLS_POPUP_AXE, "");
	newMenu->addContextMenu((g_engine.getShowSkillBar(Skills_Distance) ? CONTEXTMENU_STYLE_CHECKED : CONTEXTMENU_STYLE_UNCHECKED), SKILLS_POPUP_DISTANCE_EVENTID, SKILLS_POPUP_DISTANCE, "");
	newMenu->addContextMenu((g_engine.getShowSkillBar(Skills_Shielding) ? CONTEXTMENU_STYLE_CHECKED : CONTEXTMENU_STYLE_UNCHECKED), SKILLS_POPUP_SHIELDING_EVENTID, SKILLS_POPUP_SHIELDING, "");
	newMenu->addContextMenu(((g_engine.getShowSkillBar(Skills_Fishing) ? CONTEXTMENU_STYLE_CHECKED : CONTEXTMENU_STYLE_UNCHECKED)|CONTEXTMENU_STYLE_SEPARATED), SKILLS_POPUP_FISHING_EVENTID, SKILLS_POPUP_FISHING, "");
	newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, SKILLS_POPUP_SHOWALL_EVENTID, SKILLS_POPUP_SHOWALL, "");
	newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, SKILLS_POPUP_HIDEALL_EVENTID, SKILLS_POPUP_HIDEALL, "");
	newMenu->setEventCallback(&skills_Events);
	g_engine.showContextMenu(newMenu, x, y);
}

void GUI_SkillsContainer::onRMouseDown(Sint32 x, Sint32 y)
{
	if(!m_visible)
		return;

	if(m_scrollBar.getRect().isPointInside(x, y))
	{
		m_scrollBar.onRMouseDown(x, y);
		return;
	}

	m_rmouse = true;
	for(std::vector<GUI_Element*>::reverse_iterator it = m_childs.rbegin(), end = m_childs.rend(); it != end; ++it)
	{
		if((*it)->isEventable() && (*it)->getRect().isPointInside(x, y))
		{
			setActiveElement((*it));
			(*it)->onRMouseDown(x, y);
			return;
		}
	}
}

void GUI_SkillsContainer::onRMouseUp(Sint32 x, Sint32 y)
{
	if(!m_visible)
		return;

	m_scrollBar.onRMouseUp(x, y);
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
	{
		if((*it)->isEventable())
			(*it)->onRMouseUp(x, y);
	}

	if(m_rmouse && isInsideRect(x, y))
		UTIL_createSkillsPopupMenu(x, y);

	m_rmouse = false;
}

GUI_SkillSeparator::GUI_SkillSeparator(iRect boxRect, Uint32 internalID)
{
	setRect(boxRect);
	m_internalID = internalID;
}

void GUI_SkillSeparator::render()
{
	g_engine.getRender()->fillRectangle(m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2, 176, 176, 176, 255);
}

GUI_SkillBar::GUI_SkillBar(iRect boxRect, Uint32 internalID)
{
	setRect(boxRect);
	m_internalID = internalID;
}

void GUI_SkillBar::setPercent(Uint8 percent)
{
	m_percent = (SDL_static_cast(Sint32, percent) * (m_tRect.x2 - 2)) / 100;
	if(m_percent > (m_tRect.x2 - 2))
		m_percent = (m_tRect.x2 - 2);
}

void GUI_SkillBar::render()
{
	auto& renderer = g_engine.getRender();
	renderer->drawRectangle(m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2, 1, 0, 0, 0, 255);
	if(m_percent > 0)
		renderer->fillRectangle(m_tRect.x1 + 1, m_tRect.y1 + 1, m_percent, m_tRect.y2 - 2, m_red, m_green, m_blue, 255);
}

GUI_SkillDescription::GUI_SkillDescription(iRect boxRect, Uint32 internalID)
{
	setRect(boxRect);
	m_internalID = internalID;
}

void GUI_SkillDescription::setName(bool value, const std::string labelName)
{
	if(value)
	{
		skillValue.m_Label = std::move(labelName);
		skillValue.m_nameLen = g_engine.calculateFontWidth(skillValue.m_font, labelName) - 1;
	}
	else
		skillName.m_Label = std::move(labelName);
}

void GUI_SkillDescription::setColor(bool value, Uint8 red, Uint8 green, Uint8 blue)
{
	if(value)
	{
		skillValue.m_red = red;
		skillValue.m_green = green;
		skillValue.m_blue = blue;
	}
	else
	{
		skillName.m_red = red;
		skillName.m_green = green;
		skillName.m_blue = blue;
	}
}

void GUI_SkillDescription::setAlign(bool value, Uint8 align)
{
	if(value)
		skillValue.m_align = align;
	else
		skillName.m_align = align;
}

void GUI_SkillDescription::setFont(bool value, Uint8 font)
{
	if(value)
		skillValue.m_font = font;
	else
		skillName.m_font = font;
}

void GUI_SkillDescription::onMouseMove(Sint32 x, Sint32 y, bool isInsideParent)
{
	if(isInsideParent && m_tRect.isPointInside(x, y))
	{
		switch(getInternalID())
		{
			case SKILLS_LEVEL_EVENTID:
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have %u percent to go", 100 - SDL_static_cast(Uint32, g_game.getPlayerLevelPercent()));
				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			case SKILLS_EXPERIENCE_EVENTID:
			{
				Sint32 level = SDL_static_cast(Sint32, g_game.getPlayerLevel());
				Uint64 expToCurrentLvl = g_game.getExpForLevel(level);
				Uint64 expToNextLvl = g_game.getExpForLevel(level + 1);
				double expPerHour = (g_game.getPlayerExperienceSpeed() * 3600.0);
				Sint32 len;
				if(expPerHour > 0.0)
				{
					double hrl = (expToNextLvl - g_game.getPlayerExperience()) / expPerHour;
					Uint32 hoursLeft = SDL_static_cast(Uint32, hrl);
					Uint32 minutesLeft = SDL_static_cast(Uint32, (hrl - SDL_floor(hrl)) * 60.0);
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u XP for next level, currently %u XP per hour, next level in %u hours and %u minutes", expToNextLvl - expToCurrentLvl, SDL_static_cast(Uint32, expPerHour), hoursLeft, minutesLeft);
				}
				else
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u XP for next level", expToNextLvl - expToCurrentLvl);

				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			case SKILLS_XPGAIN_EVENTID:
			{
				Uint32 percent = 100;
				if(g_game.hasGameFeature(GAME_FEATURE_EXPERIENCE_BONUS))
				{
					if(g_game.hasGameFeature(GAME_FEATURE_DETAILED_EXPERIENCE_BONUS))
					{
						percent = SDL_static_cast(Uint32, g_game.getPlayerBaseXpGain());
						percent += SDL_static_cast(Uint32, g_game.getPlayerTournamentFactor());
						percent += SDL_static_cast(Uint32, g_game.getPlayerVoucherXpGain());
						percent += SDL_static_cast(Uint32, g_game.getPlayerGrindingXpGain());
						percent += SDL_static_cast(Uint32, g_game.getPlayerStoreXpGain());
						percent = percent * SDL_static_cast(Uint32, g_game.getPlayerHuntingXpGain()) / 100;
					}
					else
					{
						percent += SDL_static_cast(Uint32, g_game.getPlayerTournamentFactor());
						percent += SDL_static_cast(Uint32, g_game.getPlayerExpBonus() * 100.0);
					}
				}

				std::string description;
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "Your current XP gain rate amounts to %u%%\nYour XP gain rate is calculated as fellows:", percent);
				description.append(std::string(g_buffer, SDL_static_cast(size_t, len)));
				len = SDL_snprintf(g_buffer, sizeof(g_buffer), "\n- Base XP gain rate: %u%%", SDL_static_cast(Uint32, g_game.getPlayerBaseXpGain()));
				description.append(std::string(g_buffer, SDL_static_cast(size_t, len)));
				if(g_game.hasGameFeature(GAME_FEATURE_EXPERIENCE_BONUS))
				{
					Sint32 factor = g_game.getPlayerTournamentFactor();
					if(factor != 0)
					{
						len = SDL_snprintf(g_buffer, sizeof(g_buffer), "\n- Tournament Factor: %s%d%%", (factor > 0 ? "+" : ""), factor);
						description.append(std::string(g_buffer, SDL_static_cast(size_t, len)));
					}
					if(g_game.hasGameFeature(GAME_FEATURE_DETAILED_EXPERIENCE_BONUS))
					{
						Uint16 xpGain = g_game.getPlayerGrindingXpGain();
						if(xpGain != 0)
						{
							len = SDL_snprintf(g_buffer, sizeof(g_buffer), "\n- Low level bonus: +%u%% (until level 50)", SDL_static_cast(Uint32, xpGain));
							description.append(std::string(g_buffer, SDL_static_cast(size_t, len)));
						}
						xpGain = g_game.getPlayerVoucherXpGain();
						if(xpGain != 0)
						{
							len = SDL_snprintf(g_buffer, sizeof(g_buffer), "\n- XP voucher: +%u%%", SDL_static_cast(Uint32, xpGain));
							description.append(std::string(g_buffer, SDL_static_cast(size_t, len)));
						}
						xpGain = g_game.getPlayerStoreXpGain();
						if(xpGain != 0)
						{
							len = SDL_snprintf(g_buffer, sizeof(g_buffer), "\n- XP store: +%u%%", SDL_static_cast(Uint32, xpGain));
							description.append(std::string(g_buffer, SDL_static_cast(size_t, len)));
						}
						xpGain = g_game.getPlayerHuntingXpGain();
						if(xpGain != 100)
						{
							len = SDL_snprintf(g_buffer, sizeof(g_buffer), "\n- Stamina bonus: x%f", xpGain / 100.0);
							description.append(std::string(g_buffer, SDL_static_cast(size_t, len)));
						}
					}
					else
					{
						double xpGain = g_game.getPlayerExpBonus();
						if(xpGain != 0.0)
						{
							len = SDL_snprintf(g_buffer, sizeof(g_buffer), "\n- XP bonus: +%u%%", SDL_static_cast(Uint32, xpGain * 100.0));
							description.append(std::string(g_buffer, SDL_static_cast(size_t, len)));
						}
					}
				}
				g_engine.showDescription(x, y, description);
			}
			break;
			case SKILLS_HITPOINTS_EVENTID:
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have %u of %u Hit Points left", g_game.getPlayerHealth(), g_game.getPlayerMaxHealth());
				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			case SKILLS_MANA_EVENTID:
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have %u of %u Mana left", g_game.getPlayerMana(), g_game.getPlayerMaxMana());
				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			case SKILLS_CAPACITY_EVENTID:
			{
				if(g_game.hasGameFeature(GAME_FEATURE_TOTAL_CAPACITY))
				{
					Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have %f of %f Capacity left", g_game.getPlayerCapacity(), g_game.getPlayerTotalCapacity());
					g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
				}
			}
			break;
			case SKILLS_FOOD_EVENTID:
			{
				Uint32 regen = g_game.getPlayerRegeneration();
				Sint32 len;
				if(regen == 0)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You are hungry\nIf you eat something you will regenerate Hit Points and Mana over time");
				else
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You are satiated for %u minutes and %u seconds", (regen / 60), (regen % 60));

				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			case SKILLS_STAMINA_EVENTID:
			{
				Uint32 staminaMinutes = SDL_static_cast(Uint32, g_game.getPlayerStamina());
				Sint32 len;
				if(g_clientVersion <= 840)
				{
					if(staminaMinutes == 0)
						len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have no stamina left and receive no more experience and loot");
					else
						len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have %u hours and %u minutes left", (staminaMinutes / 60), (staminaMinutes % 60));
				}
				else
				{
					if((g_clientVersion < 855 && staminaMinutes >= 2400) || (g_clientVersion >= 855 && staminaMinutes >= 2460))
						len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have %u hours and %u minutes left and receive 50%% more experience (premium only)", (staminaMinutes / 60), (staminaMinutes % 60));
					else if(staminaMinutes >= 840)
						len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have %u hours and %u minutes left", (staminaMinutes / 60), (staminaMinutes % 60));
					else if(staminaMinutes == 0)
						len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have no stamina left and receive no more experience and loot");
					else
						len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have %u hours and %u minutes left and receive 50%% less experience and no loot", (staminaMinutes / 60), (staminaMinutes % 60));
				}
				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			case SKILLS_TRAINING_EVENTID:
			{
				Uint32 training = SDL_static_cast(Uint32, g_game.getPlayerOfflineTraining());
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have %u hours and %u minutes of offline training time left", (training / 60), (training % 60));
				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			case SKILLS_MLEVEL_EVENTID:
			{
				Uint32 level = SDL_static_cast(Uint32, g_game.getPlayerMagicLevel());
				Uint32 baseLevel = SDL_static_cast(Uint32, g_game.getPlayerBaseMagicLevel());
				Sint32 len;
				if(level > baseLevel)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u = %u+%u\nYou have %u percent to go", level, baseLevel, (level - baseLevel), 100 - SDL_static_cast(Uint32, g_game.getPlayerMagicLevelPercent()));
				else if(level < baseLevel)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u = %u-%u\nYou have %u percent to go", level, baseLevel, (baseLevel - level), 100 - SDL_static_cast(Uint32, g_game.getPlayerMagicLevelPercent()));
				else
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have %u percent to go", 100 - SDL_static_cast(Uint32, g_game.getPlayerMagicLevelPercent()));

				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			case SKILLS_FIST_EVENTID:
			{
				Uint32 level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Fist));
				Uint32 baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Fist));
				Sint32 len;
				if(level > baseLevel)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u = %u+%u\nYou have %u percent to go", level, baseLevel, (level - baseLevel), 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Fist)));
				else if(level < baseLevel)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u = %u-%u\nYou have %u percent to go", level, baseLevel, (baseLevel - level), 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Fist)));
				else
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have %u percent to go", 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Fist)));

				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			case SKILLS_CLUB_EVENTID:
			{
				Uint32 level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Club));
				Uint32 baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Club));
				Sint32 len;
				if(level > baseLevel)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u = %u+%u\nYou have %u percent to go", level, baseLevel, (level - baseLevel), 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Club)));
				else if(level < baseLevel)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u = %u-%u\nYou have %u percent to go", level, baseLevel, (baseLevel - level), 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Club)));
				else
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have %u percent to go", 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Club)));

				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			case SKILLS_SWORD_EVENTID:
			{
				Uint32 level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Sword));
				Uint32 baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Sword));
				Sint32 len;
				if(level > baseLevel)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u = %u+%u\nYou have %u percent to go", level, baseLevel, (level - baseLevel), 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Sword)));
				else if(level < baseLevel)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u = %u-%u\nYou have %u percent to go", level, baseLevel, (baseLevel - level), 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Sword)));
				else
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have %u percent to go", 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Sword)));

				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			case SKILLS_AXE_EVENTID:
			{
				Uint32 level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Axe));
				Uint32 baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Axe));
				Sint32 len;
				if(level > baseLevel)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u = %u+%u\nYou have %u percent to go", level, baseLevel, (level - baseLevel), 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Axe)));
				else if(level < baseLevel)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u = %u-%u\nYou have %u percent to go", level, baseLevel, (baseLevel - level), 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Axe)));
				else
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have %u percent to go", 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Axe)));

				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			case SKILLS_DISTANCE_EVENTID:
			{
				Uint32 level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Distance));
				Uint32 baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Distance));
				Sint32 len;
				if(level > baseLevel)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u = %u+%u\nYou have %u percent to go", level, baseLevel, (level - baseLevel), 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Distance)));
				else if(level < baseLevel)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u = %u-%u\nYou have %u percent to go", level, baseLevel, (baseLevel - level), 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Distance)));
				else
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have %u percent to go", 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Distance)));

				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			case SKILLS_SHIELDING_EVENTID:
			{
				Uint32 level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Shielding));
				Uint32 baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Shielding));
				Sint32 len;
				if(level > baseLevel)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u = %u+%u\nYou have %u percent to go", level, baseLevel, (level - baseLevel), 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Shielding)));
				else if(level < baseLevel)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u = %u-%u\nYou have %u percent to go", level, baseLevel, (baseLevel - level), 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Shielding)));
				else
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have %u percent to go", 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Shielding)));

				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			case SKILLS_FISHING_EVENTID:
			{
				Uint32 level = SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_Fishing));
				Uint32 baseLevel = SDL_static_cast(Uint32, g_game.getPlayerSkillBaseLevel(Skills_Fishing));
				Sint32 len;
				if(level > baseLevel)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u = %u+%u\nYou have %u percent to go", level, baseLevel, (level - baseLevel), 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Fishing)));
				else if(level < baseLevel)
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u = %u-%u\nYou have %u percent to go", level, baseLevel, (baseLevel - level), 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Fishing)));
				else
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have %u percent to go", 100 - SDL_static_cast(Uint32, g_game.getPlayerSkillLevelPercent(Skills_Fishing)));

				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			case SKILLS_CRITICAL_HIT_EVENTID:
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have a %u%% chance to cause %u%% extra damage", SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_CriticalChance)), SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_CriticalDamage)));
				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			case SKILLS_LIFE_LEECH_EVENTID:
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have a %u%% chance to get %u%% of the damage as health gain", SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_LifeLeechChance)), SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_LifeLeechAmount)));
				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			case SKILLS_MANA_LEECH_EVENTID:
			{
				Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "You have a %u%% chance to get %u%% of the damage as mana gain", SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_ManaLeechChance)), SDL_static_cast(Uint32, g_game.getPlayerSkillLevel(Skills_ManaLeechAmount)));
				g_engine.showDescription(x, y, std::string(g_buffer, SDL_static_cast(size_t, len)));
			}
			break;
			default: break;
		}
	}
}

void GUI_SkillDescription::render()
{
	g_engine.drawFont(skillName.m_font, m_tRect.x1, m_tRect.y1, skillName.m_Label, skillName.m_red, skillName.m_green, skillName.m_blue, skillName.m_align);
	g_engine.drawFont(skillValue.m_font, m_tRect.x1 + m_tRect.x2 - skillValue.m_nameLen, m_tRect.y1, skillValue.m_Label, skillValue.m_red, skillValue.m_green, skillValue.m_blue, skillValue.m_align);
}
