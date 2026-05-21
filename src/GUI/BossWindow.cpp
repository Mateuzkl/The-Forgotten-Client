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
#include "../game.h"
#include "../creature.h"
#include "../json/json.h"
#include "../GUI_Elements/GUI_Panel.h"
#include "../GUI_Elements/GUI_PanelWindow.h"
#include "../GUI_Elements/GUI_StaticImage.h"
#include "../GUI_Elements/GUI_Label.h"
#include "../GUI_Elements/GUI_Icon.h"
#include "../GUI_Elements/GUI_Container.h"
#include "BossWindow.h"

#include <algorithm>
#include <map>
#include <memory>
#include <vector>

#define BOSSES_TITLE "Bosses"
#define BOSSES_OPCODE 120
#define BOSSES_MAXIMINI_EVENTID 3000
#define BOSSES_CLOSE_EVENTID 3001
#define BOSSES_REFRESH_EVENTID 3002
#define BOSSES_RESIZE_WIDTH_EVENTID 3003
#define BOSSES_RESIZE_HEIGHT_EVENTID 3004
#define BOSSES_EXIT_WINDOW_EVENTID 3005
#define BOSSES_CONTAINER_EVENTID 3006

struct BossState
{
	bool alive = false;
	Creature* creature = NULL;
};

extern Engine g_engine;
extern Game g_game;

bool g_haveBossOpen = false;
bool g_recreateBossWindow = false;

static std::vector<std::string> g_bossOrder;
static std::map<std::string, BossState> g_bossStates;
static Uint32 g_nextBossPreviewId = 0x7F000000;

static BossState& getBossState(const std::string& bossName)
{
	BossState& state = g_bossStates[bossName];
	if(!state.creature)
	{
		state.creature = new Creature();
		state.creature->setId(g_nextBossPreviewId++);
		state.creature->setType(CREATURETYPE_MONSTER);
		state.creature->setName(bossName);
		state.creature->setVisible(false);
		state.creature->setShowStatus(true);
		state.creature->setHealth(0);
		state.creature->setOutfit(0, 0, 0, 0, 0, 0, 0, 0);
	}
	return state;
}

static BossState* findBossState(const std::string& bossName)
{
	std::map<std::string, BossState>::iterator it = g_bossStates.find(bossName);
	if(it == g_bossStates.end())
		return NULL;
	return &it->second;
}

static Uint16 getJsonNumber(JSON_VALUE* object, const std::string& key, Uint16 fallback = 0)
{
	if(!object)
		return fallback;

	JSON_VALUE* value = object->find(key);
	if(!value || !value->IsNumber())
		return fallback;
	return SDL_static_cast(Uint16, value->AsNumber());
}

static bool getJsonBool(JSON_VALUE* object, const std::string& key, bool fallback = false)
{
	if(!object)
		return fallback;

	JSON_VALUE* value = object->find(key);
	if(!value || !value->IsBool())
		return fallback;
	return value->AsBool();
}

static std::string getJsonString(JSON_VALUE* object, const std::string& key)
{
	if(!object)
		return std::string();

	JSON_VALUE* value = object->find(key);
	if(!value || !value->IsString())
		return std::string();
	return value->AsString();
}

static void updateBossState(const std::string& bossName, bool alive, JSON_VALUE* outfit)
{
	BossState& state = getBossState(bossName);
	state.alive = alive;
	state.creature->setName(bossName);
	state.creature->setVisible(alive);
	state.creature->setHealth(alive ? 100 : 0);
	if(outfit && outfit->IsObject())
	{
		state.creature->setOutfit(
			getJsonNumber(outfit, "type"),
			getJsonNumber(outfit, "auxType"),
			SDL_static_cast(Uint8, getJsonNumber(outfit, "head")),
			SDL_static_cast(Uint8, getJsonNumber(outfit, "body")),
			SDL_static_cast(Uint8, getJsonNumber(outfit, "legs")),
			SDL_static_cast(Uint8, getJsonNumber(outfit, "feet")),
			SDL_static_cast(Uint8, getJsonNumber(outfit, "addons")),
			getJsonNumber(outfit, "mount"));
	}
}

static void syncBossList(JSON_VALUE* data)
{
	if(!data || !data->IsArray())
		return;

	std::vector<std::string> nextOrder;
	nextOrder.reserve(data->size());
	for(size_t i = 0, end = data->size(); i < end; ++i)
	{
		JSON_VALUE* entry = data->find(i);
		if(!entry || !entry->IsString())
			continue;

		const std::string bossName = entry->AsString();
		nextOrder.push_back(bossName);
		getBossState(bossName);
	}

	for(std::map<std::string, BossState>::iterator it = g_bossStates.begin(); it != g_bossStates.end();)
	{
		if(std::find(nextOrder.begin(), nextOrder.end(), it->first) == nextOrder.end())
		{
			delete it->second.creature;
			it = g_bossStates.erase(it);
		}
		else
			++it;
	}

	g_bossOrder.swap(nextOrder);
	g_recreateBossWindow = true;
}

static void sendBossRefreshRequest()
{
	g_game.sendExtendedOpcode(BOSSES_OPCODE, "{\"action\":\"refresh\"}");
}

void boss_Events(Uint32 event, Sint32 status)
{
	switch(event)
	{
		case BOSSES_MAXIMINI_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_BOSSES);
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

					GUI_Container* pContainer = SDL_static_cast(GUI_Container*, pPanel->getChild(BOSSES_CONTAINER_EVENTID));
					if(pContainer)
						pContainer->makeInvisible();

					GUI_Icon* pIcon = SDL_static_cast(GUI_Icon*, pPanel->getChild(BOSSES_MAXIMINI_EVENTID));
					if(pIcon)
						pIcon->setData(GUI_UI_IMAGE, GUI_UI_ICON_MAXIMIZE_WINDOW_UP_X, GUI_UI_ICON_MAXIMIZE_WINDOW_UP_Y, GUI_UI_ICON_MAXIMIZE_WINDOW_DOWN_X, GUI_UI_ICON_MAXIMIZE_WINDOW_DOWN_Y);
				}
				else
				{
					Sint32 cachedHeight = pPanel->getCachedHeight();
					parent->tryFreeHeight(cachedHeight - pRect.y2);
					pPanel->setSize(pRect.x2, cachedHeight);
					parent->checkPanels();

					GUI_Container* pContainer = SDL_static_cast(GUI_Container*, pPanel->getChild(BOSSES_CONTAINER_EVENTID));
					if(pContainer)
						pContainer->makeVisible();

					GUI_Icon* pIcon = SDL_static_cast(GUI_Icon*, pPanel->getChild(BOSSES_MAXIMINI_EVENTID));
					if(pIcon)
						pIcon->setData(GUI_UI_IMAGE, GUI_UI_ICON_MINIMIZE_WINDOW_UP_X, GUI_UI_ICON_MINIMIZE_WINDOW_UP_Y, GUI_UI_ICON_MINIMIZE_WINDOW_DOWN_X, GUI_UI_ICON_MINIMIZE_WINDOW_DOWN_Y);
				}
			}
		}
		break;
		case BOSSES_CLOSE_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_BOSSES);
			if(pPanel)
			{
				GUI_Panel* parent = pPanel->getParent();
				if(parent)
					parent->removePanel(pPanel);
			}
		}
		break;
		case BOSSES_REFRESH_EVENTID:
			sendBossRefreshRequest();
		break;
		case BOSSES_RESIZE_HEIGHT_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_BOSSES);
			if(pPanel)
			{
				GUI_Container* pContainer = SDL_static_cast(GUI_Container*, pPanel->getChild(BOSSES_CONTAINER_EVENTID));
				if(pContainer)
				{
					iRect cRect = pContainer->getRect();
					cRect.y2 = status - 19;
					pContainer->setRect(cRect);
				}
			}
		}
		break;
		case BOSSES_EXIT_WINDOW_EVENTID:
			g_engine.setContentWindowHeight(GUI_PANEL_WINDOW_BOSSES, status - 19);
			g_haveBossOpen = false;
		break;
		default: break;
	}
}

void UTIL_recreateBossWindow(GUI_Container* container)
{
	if(!container)
	{
		GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_BOSSES);
		if(pPanel)
			container = SDL_static_cast(GUI_Container*, pPanel->getChild(BOSSES_CONTAINER_EVENTID));
		if(!container)
			return;
	}

	container->clearChilds(false);
	Sint32 posY = -9;
	for(std::vector<std::string>::iterator it = g_bossOrder.begin(), end = g_bossOrder.end(); it != end; ++it)
	{
		GUI_BossCreature* newBossCreature = new GUI_BossCreature(iRect(1, posY, 155, 23), (*it));
		newBossCreature->startEvents();
		container->addChild(newBossCreature, false);
		posY += 23;
	}
	container->validateScrollBar();
}

bool UTIL_handleExtendedOpcode(Uint8 opcode, const std::string& payload)
{
	if(opcode != BOSSES_OPCODE)
		return false;

	std::unique_ptr<JSON_VALUE> dataJson(JSON_VALUE::decode(payload.c_str()));
	JSON_VALUE* dataObject = dataJson.get();
	if(!dataObject || !dataObject->IsObject())
		return true;

	const std::string action = getJsonString(dataObject, "action");
	if(action == "list")
	{
		syncBossList(dataObject->find("data"));
	}
	else if(action == "update")
	{
		const std::string bossName = getJsonString(dataObject, "name");
		if(!bossName.empty())
		{
			if(std::find(g_bossOrder.begin(), g_bossOrder.end(), bossName) == g_bossOrder.end())
				g_bossOrder.push_back(bossName);

			updateBossState(bossName, getJsonBool(dataObject, "alive"), dataObject->find("outfit"));
			g_recreateBossWindow = true;
		}
	}
	return true;
}

void UTIL_resetBossEntries()
{
	g_bossOrder.clear();
	for(std::map<std::string, BossState>::iterator it = g_bossStates.begin(), end = g_bossStates.end(); it != end; ++it)
		delete it->second.creature;
	g_bossStates.clear();
	g_recreateBossWindow = true;
	g_haveBossOpen = false;
	g_nextBossPreviewId = 0x7F000000;
}

void UTIL_toggleBossWindow()
{
	GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_BOSSES);
	if(pPanel)
	{
		g_engine.removePanelWindow(pPanel);
		return;
	}

	Sint32 savedHeight = g_engine.getContentWindowHeight(GUI_PANEL_WINDOW_BOSSES);
	if(savedHeight < 38)
		savedHeight = 119;
	GUI_PanelWindow* newWindow = new GUI_PanelWindow(iRect(0, 0, GAME_PANEL_FIXED_WIDTH - 4, savedHeight + 19), true, GUI_PANEL_WINDOW_BOSSES, true);
	newWindow->setEventCallback(&boss_Events, BOSSES_RESIZE_WIDTH_EVENTID, BOSSES_RESIZE_HEIGHT_EVENTID, BOSSES_EXIT_WINDOW_EVENTID);

	GUI_BossChecker* newBossChecker = new GUI_BossChecker(iRect(19, 2, 0, 0));
	newWindow->addChild(newBossChecker);
	GUI_StaticImage* newImage = new GUI_StaticImage(iRect(2, 0, GUI_UI_ICON_BATTLELIST_W, GUI_UI_ICON_BATTLELIST_H), GUI_UI_IMAGE, GUI_UI_ICON_BATTLELIST_X, GUI_UI_ICON_BATTLELIST_Y);
	newWindow->addChild(newImage);
	GUI_Icon* newIcon = new GUI_Icon(iRect(147, 0, GUI_UI_ICON_MINIMIZE_WINDOW_UP_W, GUI_UI_ICON_MINIMIZE_WINDOW_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_MINIMIZE_WINDOW_UP_X, GUI_UI_ICON_MINIMIZE_WINDOW_UP_Y, GUI_UI_ICON_MINIMIZE_WINDOW_DOWN_X, GUI_UI_ICON_MINIMIZE_WINDOW_DOWN_Y, BOSSES_MAXIMINI_EVENTID, "Maximise or minimise window");
	newIcon->setButtonEventCallback(&boss_Events, BOSSES_MAXIMINI_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(159, 0, GUI_UI_ICON_CLOSE_WINDOW_UP_W, GUI_UI_ICON_CLOSE_WINDOW_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_CLOSE_WINDOW_UP_X, GUI_UI_ICON_CLOSE_WINDOW_UP_Y, GUI_UI_ICON_CLOSE_WINDOW_DOWN_X, GUI_UI_ICON_CLOSE_WINDOW_DOWN_Y, 0, "Close this window");
	newIcon->setButtonEventCallback(&boss_Events, BOSSES_CLOSE_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(131, 0, GUI_UI_ICON_CONFIGURE_WINDOW_UP_W, GUI_UI_ICON_CONFIGURE_WINDOW_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_CONFIGURE_WINDOW_UP_X, GUI_UI_ICON_CONFIGURE_WINDOW_UP_Y, GUI_UI_ICON_CONFIGURE_WINDOW_DOWN_X, GUI_UI_ICON_CONFIGURE_WINDOW_DOWN_Y, BOSSES_REFRESH_EVENTID, "Refresh boss states");
	newIcon->setButtonEventCallback(&boss_Events, BOSSES_REFRESH_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	GUI_DynamicLabel* newLabel = new GUI_DynamicLabel(iRect(19, 2, 100, 14), BOSSES_TITLE, 0, 144, 144, 144);
	newWindow->addChild(newLabel);
	GUI_Container* newContainer = new GUI_Container(iRect(2, 13, 168, savedHeight), newWindow, BOSSES_CONTAINER_EVENTID);
	UTIL_recreateBossWindow(newContainer);
	newContainer->startEvents();
	newWindow->addChild(newContainer);

	Sint32 preferredPanel = g_engine.getContentWindowParent(GUI_PANEL_WINDOW_BOSSES);
	bool added = g_engine.addToPanel(newWindow, preferredPanel);
	if(!added && preferredPanel != GUI_PANEL_RANDOM)
		added = g_engine.addToPanel(newWindow, GUI_PANEL_RANDOM);

	if(added)
	{
		g_recreateBossWindow = false;
		g_haveBossOpen = true;
		sendBossRefreshRequest();
	}
	else
	{
		g_recreateBossWindow = true;
		g_haveBossOpen = false;
		delete newWindow;
	}
}

GUI_BossChecker::GUI_BossChecker(iRect boxRect, Uint32 internalID)
{
	setRect(boxRect);
	m_internalID = internalID;
}

void GUI_BossChecker::render()
{
	if(g_recreateBossWindow)
	{
		UTIL_recreateBossWindow(NULL);
		g_recreateBossWindow = false;
	}
}

GUI_BossCreature::GUI_BossCreature(iRect boxRect, const std::string& bossName, Uint32 internalID) : m_bossName(bossName)
{
	setRect(boxRect);
	m_internalID = internalID;
}

void GUI_BossCreature::render()
{
	BossState* state = findBossState(m_bossName);
	if(!state || !state->creature)
		return;

	state->creature->renderOnBattle(m_tRect.x1, m_tRect.y1, false);
	g_engine.drawFont(CLIENT_FONT_NONOUTLINED, m_tRect.x1 + 152, m_tRect.y1 + 4, (state->alive ? "Alive" : "Dead"), (state->alive ? 96 : 224), (state->alive ? 224 : 96), 96, CLIENT_FONT_ALIGN_RIGHT);
}
