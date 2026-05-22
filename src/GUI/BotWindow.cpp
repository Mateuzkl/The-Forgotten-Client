/*
  The Forgotten Client
  Native bot panel.
*/

#include "GUI_UTIL.h"
#include "../bot.h"
#include "../creature.h"
#include "../engine.h"
#include "../game.h"
#include "../map.h"
#include "../GUI_Elements/GUI_Button.h"
#include "../GUI_Elements/GUI_CheckBox.h"
#include "../GUI_Elements/GUI_Grouper.h"
#include "../GUI_Elements/GUI_Label.h"
#include "../GUI_Elements/GUI_ListBox.h"
#include "../GUI_Elements/GUI_Panel.h"
#include "../GUI_Elements/GUI_PanelWindow.h"
#include "../GUI_Elements/GUI_TextBox.h"
#include "../GUI_Elements/GUI_Window.h"
#include "itemUI.h"

#include <cctype>
#include <cstdlib>
#include <memory>

#define BOT_PANEL_EVENT_TOGGLE 4100
#define BOT_PANEL_EVENT_ADD_WAYPOINT 4101
#define BOT_PANEL_EVENT_CLEAR_WAYPOINTS 4102
#define BOT_PANEL_EVENT_TARGET 4103
#define BOT_PANEL_EVENT_CAVE 4104
#define BOT_PANEL_EVENT_PAUSE_CAVE 4105
#define BOT_PANEL_EVENT_OPEN_WINDOW 4106

#define BOT_PANEL_STATUS_LABEL 4200
#define BOT_PANEL_WAYPOINT_LABEL 4201

#define BOT_WINDOW_EVENT_CLOSE 5000
#define BOT_WINDOW_EVENT_TAB_CAVE 5010
#define BOT_WINDOW_EVENT_TAB_TARGET 5011
#define BOT_WINDOW_EVENT_TAB_HEALING 5012
#define BOT_WINDOW_EVENT_TAB_LOOT 5013
#define BOT_WINDOW_EVENT_TAB_HOTKEYS 5014
#define BOT_WINDOW_EVENT_TAB_SETTINGS 5015
#define BOT_WINDOW_EVENT_TAB_ICONS 5016
#define BOT_WINDOW_EVENT_START 5020
#define BOT_WINDOW_EVENT_STOP 5021
#define BOT_WINDOW_EVENT_SAVE 5022
#define BOT_WINDOW_EVENT_LOAD 5023
#define BOT_WINDOW_EVENT_DELETE_PROFILE 5024
#define BOT_WINDOW_EVENT_ADD_WALK 5100
#define BOT_WINDOW_EVENT_ADD_CENTER 5101
#define BOT_WINDOW_EVENT_ADD_STAND 5102
#define BOT_WINDOW_EVENT_ADD_NODE 5103
#define BOT_WINDOW_EVENT_ADD_ROPE 5104
#define BOT_WINDOW_EVENT_ADD_LADDER 5105
#define BOT_WINDOW_EVENT_ADD_SHOVEL 5106
#define BOT_WINDOW_EVENT_ADD_LURE 5107
#define BOT_WINDOW_EVENT_ADD_ACTION 5108
#define BOT_WINDOW_EVENT_DELETE_WP 5110
#define BOT_WINDOW_EVENT_CLEAR_WP 5111
#define BOT_WINDOW_EVENT_CAVE_ENABLE 5112
#define BOT_WINDOW_EVENT_CAVE_SMART_WALK 5113
#define BOT_WINDOW_EVENT_CAVE_AVOID_PLAYERS 5114
#define BOT_WINDOW_EVENT_TARGET_ENABLE 5200
#define BOT_WINDOW_EVENT_ADD_TARGET 5201
#define BOT_WINDOW_EVENT_DELETE_TARGET 5202
#define BOT_WINDOW_EVENT_CLEAR_TARGETS 5203
#define BOT_WINDOW_EVENT_HEAL_ENABLE 5300
#define BOT_WINDOW_EVENT_APPLY_HEAL 5301
#define BOT_WINDOW_EVENT_ADD_LOOT 5400
#define BOT_WINDOW_EVENT_DELETE_LOOT 5401
#define BOT_WINDOW_EVENT_CLEAR_LOOT 5402
#define BOT_WINDOW_EVENT_SETTINGS_PAUSE_CAVE 5500
#define BOT_WINDOW_EVENT_HOTKEYS_ENABLE 5520
#define BOT_WINDOW_EVENT_APPLY_HOTKEYS 5521
#define BOT_WINDOW_EVENT_APPLY_ICONS 5530
#define BOT_WINDOW_EVENT_TARGET_FOLLOW 5541

#define BOT_WINDOW_STATUS_LABEL 5600
#define BOT_WINDOW_STATS_LABEL 5601
#define BOT_WINDOW_STATUS_BOT_LABEL 5610
#define BOT_WINDOW_STATUS_TARGET_LABEL 5611
#define BOT_WINDOW_STATUS_CAVE_LABEL 5612
#define BOT_WINDOW_STATUS_PING_LABEL 5613
#define BOT_WINDOW_STATUS_POS_LABEL 5614
#define BOT_WINDOW_WAYPOINT_LIST 5700
#define BOT_WINDOW_TARGET_LIST 5701
#define BOT_WINDOW_LOOT_LIST 5702
#define BOT_WINDOW_TARGET_TEXT 5800
#define BOT_WINDOW_LOOT_TEXT 5801
#define BOT_WINDOW_HP_ITEM_TEXT 5802
#define BOT_WINDOW_HP_PERCENT_TEXT 5803
#define BOT_WINDOW_MP_ITEM_TEXT 5804
#define BOT_WINDOW_MP_PERCENT_TEXT 5805
#define BOT_WINDOW_UH_ITEM_TEXT 5806
#define BOT_WINDOW_FRIEND_PERCENT_TEXT 5807
#define BOT_WINDOW_FRIEND_NAME_TEXT 5808
#define BOT_WINDOW_PROFILE_TEXT 5809
#define BOT_WINDOW_HOTKEY_BASE 5900
#define BOT_WINDOW_ICON_BASE 6100

#define BOT_WINDOW_WIDTH 720
#define BOT_WINDOW_HEIGHT 360
#define BOT_PANEL_COMPACT_HEIGHT 124
#define BOT_HOTKEY_ROW_COUNT 5
#define BOT_ICON_ROW_COUNT 5
#define BOT_ICON_BASE_X 8
#define BOT_ICON_BASE_Y 126

extern Engine g_engine;
extern Game g_game;
extern Map g_map;
extern Bot g_bot;
extern Uint16 g_ping;

void bot_Events(Uint32 event, Sint32 status);
static void executeBotIconAction(const std::string& action, Uint16 itemId);

enum BotWindowTab
{
	BOT_TAB_CAVE,
	BOT_TAB_TARGET,
	BOT_TAB_HEALING,
	BOT_TAB_LOOT,
	BOT_TAB_HOTKEYS,
	BOT_TAB_ICONS,
	BOT_TAB_SETTINGS
};

static BotWindowTab s_botTab = BOT_TAB_CAVE;
static std::string s_botProfileName = "native_bot";

class GUI_BotItemIcon : public GUI_Element
{
	public:
		GUI_BotItemIcon(iRect boxRect, const BotIconConfig& config, Uint32 internalID = 0) :
			m_item(ItemUI::createItemUI(config.itemId, 1)), m_itemId(config.itemId), m_label(config.label),
			m_leftAction(config.leftAction), m_rightAction(config.rightAction),
			m_ctrlLeftAction(config.ctrlLeftAction), m_ctrlRightAction(config.ctrlRightAction)
		{
			setRect(boxRect);
			m_internalID = internalID;
		}

		void onMouseMove(Sint32 x, Sint32 y, bool isInsideParent)
		{
			if(!m_label.empty() && isInsideParent && m_tRect.isPointInside(x, y))
				g_engine.showDescription(x, y, m_label);
		}

		void onLMouseDown(Sint32, Sint32)
		{
			const bool ctrl = (SDL_GetModState() & KMOD_CTRL) != 0;
			executeBotIconAction(ctrl ? m_ctrlLeftAction : m_leftAction, m_itemId);
		}

		void onRMouseDown(Sint32, Sint32)
		{
			const bool ctrl = (SDL_GetModState() & KMOD_CTRL) != 0;
			executeBotIconAction(ctrl ? m_ctrlRightAction : m_rightAction, m_itemId);
		}

		void render()
		{
			if(m_item)
				m_item->render(m_tRect.x1, m_tRect.y1, 32);

			if(!m_label.empty())
				g_engine.drawFont(CLIENT_FONT_OUTLINED, m_tRect.x1 + 16, m_tRect.y1 + 33, m_label, 255, 255, 255, CLIENT_FONT_ALIGN_CENTER);
		}

	protected:
		std::unique_ptr<ItemUI> m_item;
		Uint16 m_itemId = 0;
		std::string m_label;
		std::string m_leftAction;
		std::string m_rightAction;
		std::string m_ctrlLeftAction;
		std::string m_ctrlRightAction;
};

static bool CheckBotEnabled()
{
	return g_bot.isEnabled();
}

static bool CheckBotTabCave()
{
	return s_botTab == BOT_TAB_CAVE;
}

static bool CheckBotTabTarget()
{
	return s_botTab == BOT_TAB_TARGET;
}

static bool CheckBotTabHealing()
{
	return s_botTab == BOT_TAB_HEALING;
}

static bool CheckBotTabLoot()
{
	return s_botTab == BOT_TAB_LOOT;
}

static bool CheckBotTabHotkeys()
{
	return s_botTab == BOT_TAB_HOTKEYS;
}

static bool CheckBotTabSettings()
{
	return s_botTab == BOT_TAB_SETTINGS;
}

static bool CheckBotTabIcons()
{
	return s_botTab == BOT_TAB_ICONS;
}

static std::string botNumberText(Uint32 value)
{
	SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", value);
	return std::string(g_buffer);
}

static std::string botTitleCaseText(const std::string& text)
{
	std::string result = text;
	bool upperNext = true;
	for(size_t i = 0, end = result.length(); i < end; ++i)
	{
		unsigned char ch = SDL_static_cast(unsigned char, result[i]);
		if(std::isspace(ch) || result[i] == '-' || result[i] == '_')
		{
			upperNext = true;
			if(result[i] == '_')
				result[i] = ' ';
			continue;
		}

		result[i] = SDL_static_cast(char, upperNext ? std::toupper(ch) : std::tolower(ch));
		upperNext = false;
	}
	return result;
}

static std::string botLootDisplayText(const BotLootItem& item)
{
	std::string name = item.name.empty() ? "Unknown Item" : botTitleCaseText(item.name);
	SDL_snprintf(g_buffer, sizeof(g_buffer), "%s [%u]", name.c_str(), SDL_static_cast(Uint32, item.itemId));
	return std::string(g_buffer);
}

static std::string botCompactStatusText()
{
	std::string status = g_bot.getStatusText();
	const size_t maxLength = 36;
	if(status.length() <= maxLength)
		return status;

	return status.substr(0, maxLength - 3) + "...";
}

static Uint32 botReadNumber(Uint32 childId)
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_BOT);
	if(!pWindow)
		return 0;

	GUI_TextBox* textBox = SDL_static_cast(GUI_TextBox*, pWindow->getChild(childId));
	if(!textBox)
		return 0;

	return SDL_static_cast(Uint32, std::strtoul(textBox->getActualText().c_str(), NULL, 10));
}

static Sint32 botReadSignedNumber(Uint32 childId)
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_BOT);
	if(!pWindow)
		return 0;

	GUI_TextBox* textBox = SDL_static_cast(GUI_TextBox*, pWindow->getChild(childId));
	if(!textBox)
		return 0;

	return SDL_static_cast(Sint32, std::strtol(textBox->getActualText().c_str(), NULL, 10));
}

static std::string botReadText(Uint32 childId)
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_BOT);
	if(!pWindow)
		return std::string();

	GUI_TextBox* textBox = SDL_static_cast(GUI_TextBox*, pWindow->getChild(childId));
	if(!textBox)
		return std::string();

	return textBox->getActualText();
}

static void executeBotIconAction(const std::string& action, Uint16 itemId)
{
	std::string lower = action;
	for(size_t i = 0, end = lower.length(); i < end; ++i)
		lower[i] = SDL_static_cast(char, std::tolower(SDL_static_cast(unsigned char, lower[i])));

	Position hotkeyPosition(0xFFFF, 0, 0);
	if(lower.empty())
		return;
	if(!g_engine.isIngame())
		return;

	if(lower == "toggletarget" || lower == "targeting")
	{
		g_bot.setTargetEnabled(!g_bot.isTargetEnabled());
		return;
	}
	if(lower == "togglecavebot" || lower == "cavebot")
	{
		g_bot.setCaveEnabled(!g_bot.isCaveEnabled());
		return;
	}
	if(lower == "togglebot")
	{
		g_bot.toggleEnabled();
		return;
	}
	if(lower == "usegsp" || lower == "usesmp" || lower == "useitem")
	{
		if(itemId != 0)
			g_game.sendUseOnCreature(hotkeyPosition, itemId, 0, g_game.getPlayerID());
		return;
	}
	if(lower == "buygsp100" || lower == "buysmp100" || lower == "buy100")
	{
		g_game.processTextMessage(MessageStatus, "Bot icon: buy action needs NPC trade open.");
		return;
	}
	if(lower == "toggleutani" || lower == "utani")
	{
		g_game.sendSay(MessageSpell, 0, std::string(), "utani hur");
		return;
	}
	if(lower == "toggleutito" || lower == "utito")
	{
		g_game.sendSay(MessageSpell, 0, std::string(), "utito tempo");
		return;
	}
	if(lower == "castexori" || lower == "exori")
	{
		g_game.sendSay(MessageSpell, 0, std::string(), "exori");
		return;
	}
	if(lower == "castexevo" || lower == "exevo")
	{
		g_game.sendSay(MessageSpell, 0, std::string(), "exevo");
		return;
	}
	if(lower == "equipssa")
	{
		if(itemId != 0)
			g_game.sendEquipItem(itemId, 1);
		return;
	}
	if(lower == "userope" || lower == "useshovel" || lower == "usemachete")
	{
		g_game.processTextMessage(MessageStatus, "Bot icon: use tool action is armed for mapping.");
		return;
	}
	if(lower == "castexanitera")
	{
		g_game.sendSay(MessageSpell, 0, std::string(), "exani tera");
		return;
	}

	g_game.processTextMessage(MessageStatus, "Bot icon action is ready for mapping.");
}

static std::string botKeyNameFromKeycode(SDL_Keycode key)
{
	if(key >= SDLK_F1 && key <= SDLK_F12)
	{
		SDL_snprintf(g_buffer, sizeof(g_buffer), "F%u", SDL_static_cast(Uint32, key - SDLK_F1 + 1));
		return std::string(g_buffer);
	}
	if(key >= SDLK_KP_0 && key <= SDLK_KP_9)
	{
		SDL_snprintf(g_buffer, sizeof(g_buffer), "NUM%u", SDL_static_cast(Uint32, key - SDLK_KP_0));
		return std::string(g_buffer);
	}
	if(key >= SDLK_a && key <= SDLK_z)
	{
		char text[2] = { SDL_static_cast(char, 'A' + (key - SDLK_a)), 0 };
		return std::string(text);
	}

	switch(key)
	{
		case SDLK_HOME: return "Home";
		case SDLK_END: return "End";
		case SDLK_INSERT: return "Insert";
		case SDLK_DELETE: return "Delete";
		case SDLK_PAGEUP: return "PageUp";
		case SDLK_PAGEDOWN: return "PageDown";
		default: break;
	}
	return std::string();
}

static bool botReadCheck(Uint32 childId)
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_BOT);
	if(!pWindow)
		return false;

	GUI_CheckBox* checkBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(childId));
	if(!checkBox)
		return false;

	return checkBox->isChecked();
}

static Sint32 botReadSelected(Uint32 childId)
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_BOT);
	if(!pWindow)
		return -1;

	GUI_ListBox* listBox = SDL_static_cast(GUI_ListBox*, pWindow->getChild(childId));
	if(!listBox)
		return -1;

	return listBox->getSelect();
}

static void addSmallLabel(GUI_Window* pWindow, Sint32 x, Sint32 y, const std::string& text)
{
	std::unique_ptr<GUI_Label> label(new GUI_Label(iRect(x, y, 0, 0), text, 0, 225, 225, 225));
	label->setFont(CLIENT_FONT_SMALL);
	pWindow->addChild(label.release());
}

static GUI_DynamicLabel* addDynamicSmallLabel(GUI_Window* pWindow, Sint32 x, Sint32 y, Sint32 w, const std::string& text, Uint32 id, Uint8 red, Uint8 green, Uint8 blue)
{
	std::unique_ptr<GUI_DynamicLabel> label(new GUI_DynamicLabel(iRect(x, y, w, 12), text, id, red, green, blue));
	label->setFont(CLIENT_FONT_SMALL);
	GUI_DynamicLabel* rawLabel = label.get();
	pWindow->addChild(label.release());
	return rawLabel;
}

static GUI_Button* addButton(GUI_Window* pWindow, Sint32 x, Sint32 y, Sint32 w, const std::string& text, Uint32 event, const std::string& description = std::string(), Uint32 internalId = 0)
{
	std::unique_ptr<GUI_Button> button(new GUI_Button(iRect(x, y, w, GUI_UI_BUTTON_34PX_GRAY_UP_H), text, internalId, description));
	button->setButtonEventCallback(&bot_Events, event);
	button->startEvents();
	GUI_Button* rawButton = button.get();
	pWindow->addChild(button.release());
	return rawButton;
}

static GUI_CheckBox* addCheckBox(GUI_Window* pWindow, Sint32 x, Sint32 y, Sint32 w, const std::string& text, bool checked, Uint32 event, Uint32 internalId = 0)
{
	std::unique_ptr<GUI_CheckBox> checkBox(new GUI_CheckBox(iRect(x, y, w, 22), text, checked, internalId));
	checkBox->setBoxEventCallback(&bot_Events, event);
	checkBox->startEvents();
	GUI_CheckBox* rawCheckBox = checkBox.get();
	pWindow->addChild(checkBox.release());
	return rawCheckBox;
}

static GUI_TextBox* addTextBox(GUI_Window* pWindow, Sint32 x, Sint32 y, Sint32 w, const std::string& text, Uint32 id, bool onlyNumbers = false, Uint32 maxLength = 32)
{
	std::unique_ptr<GUI_TextBox> textBox(new GUI_TextBox(iRect(x, y, w, 16), text, id));
	textBox->setMaxLength(maxLength);
	textBox->setOnlyNumbers(onlyNumbers);
	textBox->startEvents();
	GUI_TextBox* rawTextBox = textBox.get();
	pWindow->addChild(textBox.release());
	return rawTextBox;
}

static char getWaypointTypeLetter(CaveBotWaypointType type)
{
	switch(type)
	{
		case CAVEBOT_WAYPOINT_CENTER: return 'C';
		case CAVEBOT_WAYPOINT_STAND: return 'S';
		case CAVEBOT_WAYPOINT_NODE: return 'N';
		case CAVEBOT_WAYPOINT_ROPE: return 'R';
		case CAVEBOT_WAYPOINT_LADDER: return 'L';
		case CAVEBOT_WAYPOINT_SHOVEL: return 'H';
		case CAVEBOT_WAYPOINT_LURE: return 'U';
		case CAVEBOT_WAYPOINT_ACTION: return 'A';
		case CAVEBOT_WAYPOINT_WALK:
		default: break;
	}
	return 'W';
}

static void addBotTabs(GUI_Window* pWindow)
{
	struct BotTabButton
	{
		const char* label;
		Uint32 event;
		bool (*check)();
	};

	static const BotTabButton tabs[] =
	{
		{"Cavebot", BOT_WINDOW_EVENT_TAB_CAVE, &CheckBotTabCave},
		{"Target", BOT_WINDOW_EVENT_TAB_TARGET, &CheckBotTabTarget},
		{"Healing", BOT_WINDOW_EVENT_TAB_HEALING, &CheckBotTabHealing},
		{"Looting", BOT_WINDOW_EVENT_TAB_LOOT, &CheckBotTabLoot},
		{"Hotkeys", BOT_WINDOW_EVENT_TAB_HOTKEYS, &CheckBotTabHotkeys},
		{"Icons", BOT_WINDOW_EVENT_TAB_ICONS, &CheckBotTabIcons},
		{"Settings", BOT_WINDOW_EVENT_TAB_SETTINGS, &CheckBotTabSettings}
	};

	Sint32 x = 12;
	for(size_t i = 0; i < SDL_arraysize(tabs); ++i)
	{
	GUI_RadioButton* button = new GUI_RadioButton(iRect(x, 24, GUI_UI_BUTTON_86PX_GRAY_UP_W, GUI_UI_BUTTON_86PX_GRAY_UP_H), tabs[i].label);
		button->setRadioEventCallback(tabs[i].check);
		button->setButtonEventCallback(&bot_Events, tabs[i].event);
		button->startEvents();
		pWindow->addChild(button);
		x += 94;
	}
}

static void addWindowShell(GUI_Window* pWindow)
{
	addBotTabs(pWindow);
	GUI_Grouper* grouper = new GUI_Grouper(iRect(12, 51, BOT_WINDOW_WIDTH - 24, 198));
	pWindow->addChild(grouper);

	Creature* player = g_map.getLocalCreature();
	Position pos = player ? player->getCurrentPosition() : Position(0, 0, 0);

	addDynamicSmallLabel(pWindow, 18, 258, 58, g_bot.isEnabled() ? "Bot: ON" : "Bot: OFF", BOT_WINDOW_STATUS_BOT_LABEL,
		g_bot.isEnabled() ? 110 : 230, g_bot.isEnabled() ? 230 : 90, g_bot.isEnabled() ? 110 : 90);
	addDynamicSmallLabel(pWindow, 88, 258, 124, g_bot.hasActiveTarget() ? "Target: found" : "Target: no monster", BOT_WINDOW_STATUS_TARGET_LABEL,
		g_bot.hasActiveTarget() ? 110 : 205, g_bot.hasActiveTarget() ? 230 : 190, g_bot.hasActiveTarget() ? 110 : 120);
	addDynamicSmallLabel(pWindow, 225, 258, 80, (g_bot.isEnabled() && g_bot.isCaveEnabled() && g_bot.getWaypointCount() > 0) ? "Cave: active" : "Cave: idle", BOT_WINDOW_STATUS_CAVE_LABEL,
		(g_bot.isEnabled() && g_bot.isCaveEnabled() && g_bot.getWaypointCount() > 0) ? 110 : 205,
		(g_bot.isEnabled() && g_bot.isCaveEnabled() && g_bot.getWaypointCount() > 0) ? 230 : 190,
		(g_bot.isEnabled() && g_bot.isCaveEnabled() && g_bot.getWaypointCount() > 0) ? 110 : 120);
	SDL_snprintf(g_buffer, sizeof(g_buffer), "Ping: %u ms", SDL_static_cast(Uint32, g_ping));
	addDynamicSmallLabel(pWindow, 325, 258, 84, g_buffer, BOT_WINDOW_STATUS_PING_LABEL,
		g_ping <= 80 ? 110 : (g_ping <= 180 ? 230 : 230),
		g_ping <= 80 ? 230 : (g_ping <= 180 ? 205 : 90),
		g_ping <= 80 ? 110 : 90);
	SDL_snprintf(g_buffer, sizeof(g_buffer), "Pos: %u %u %u", SDL_static_cast(Uint32, pos.x), SDL_static_cast(Uint32, pos.y), SDL_static_cast(Uint32, pos.z));
	addDynamicSmallLabel(pWindow, 430, 258, 170, g_buffer, BOT_WINDOW_STATUS_POS_LABEL, 120, 230, 255);

	addSmallLabel(pWindow, 18, 282, "Profile");
	addTextBox(pWindow, 70, 280, 118, s_botProfileName, BOT_WINDOW_PROFILE_TEXT, false, 32);
	addButton(pWindow, 42, 312, GUI_UI_BUTTON_86PX_GRAY_UP_W, "Start", BOT_WINDOW_EVENT_START);
	addButton(pWindow, 142, 312, GUI_UI_BUTTON_86PX_GRAY_UP_W, "Stop", BOT_WINDOW_EVENT_STOP);
	addButton(pWindow, 242, 312, GUI_UI_BUTTON_86PX_GRAY_UP_W, "Save", BOT_WINDOW_EVENT_SAVE);
	addButton(pWindow, 342, 312, GUI_UI_BUTTON_86PX_GRAY_UP_W, "Load", BOT_WINDOW_EVENT_LOAD);
	addButton(pWindow, 442, 312, GUI_UI_BUTTON_86PX_GRAY_UP_W, "Delete", BOT_WINDOW_EVENT_DELETE_PROFILE);
	addButton(pWindow, 542, 312, GUI_UI_BUTTON_86PX_GRAY_UP_W, "Close", BOT_WINDOW_EVENT_CLOSE, std::string(), CLIENT_GUI_ESCAPE_TRIGGER);
}

static void addCaveTab(GUI_Window* pWindow)
{
	addSmallLabel(pWindow, 24, 63, "Cavebot Options");
	addCheckBox(pWindow, 24, 82, 150, "Enable Cavebot", g_bot.isCaveEnabled(), BOT_WINDOW_EVENT_CAVE_ENABLE);
	addCheckBox(pWindow, 24, 104, 170, "Pause cave in fight", g_bot.isPauseCaveInFight(), BOT_WINDOW_EVENT_SETTINGS_PAUSE_CAVE);
	addCheckBox(pWindow, 24, 126, 150, "Smart Walk", g_bot.isSmartWalk(), BOT_WINDOW_EVENT_CAVE_SMART_WALK);
	addCheckBox(pWindow, 24, 148, 150, "Avoid Players", g_bot.isAvoidPlayers(), BOT_WINDOW_EVENT_CAVE_AVOID_PLAYERS);

	addSmallLabel(pWindow, 214, 63, "Waypoints");
	GUI_ListBox* listBox = new GUI_ListBox(iRect(214, 82, 214, 112), BOT_WINDOW_WAYPOINT_LIST);
	const std::vector<CaveBotWaypoint>& waypoints = g_bot.getWaypoints();
	for(size_t i = 0, end = waypoints.size(); i < end; ++i)
	{
		const CaveBotWaypoint& waypoint = waypoints[i];
		SDL_snprintf(g_buffer, sizeof(g_buffer), "%c %03u: %u %u %u",
			getWaypointTypeLetter(waypoint.type),
			SDL_static_cast(Uint32, i),
			SDL_static_cast(Uint32, waypoint.position.x),
			SDL_static_cast(Uint32, waypoint.position.y),
			SDL_static_cast(Uint32, waypoint.position.z));
		listBox->add(std::string(g_buffer));
	}
	if(!waypoints.empty())
		listBox->setSelect(SDL_static_cast(Sint32, UTIL_min<size_t>(g_bot.getCurrentWaypointIndex(), waypoints.size() - 1)));
	listBox->startEvents();
	pWindow->addChild(listBox);

	addButton(pWindow, 214, 202, GUI_UI_BUTTON_58PX_GRAY_UP_W, "Del", BOT_WINDOW_EVENT_DELETE_WP);
	addButton(pWindow, 276, 202, GUI_UI_BUTTON_58PX_GRAY_UP_W, "Clear", BOT_WINDOW_EVENT_CLEAR_WP);

	addSmallLabel(pWindow, 462, 63, "Add Current Position");
	addButton(pWindow, 462, 82, GUI_UI_BUTTON_58PX_GRAY_UP_W, "Walk", BOT_WINDOW_EVENT_ADD_WALK);
	addButton(pWindow, 524, 82, GUI_UI_BUTTON_75PX_GRAY_UP_W, "Center", BOT_WINDOW_EVENT_ADD_CENTER);
	addButton(pWindow, 462, 106, GUI_UI_BUTTON_58PX_GRAY_UP_W, "Stand", BOT_WINDOW_EVENT_ADD_STAND);
	addButton(pWindow, 524, 106, GUI_UI_BUTTON_75PX_GRAY_UP_W, "Node", BOT_WINDOW_EVENT_ADD_NODE);
	addButton(pWindow, 462, 130, GUI_UI_BUTTON_58PX_GRAY_UP_W, "Rope", BOT_WINDOW_EVENT_ADD_ROPE);
	addButton(pWindow, 524, 130, GUI_UI_BUTTON_75PX_GRAY_UP_W, "Ladder", BOT_WINDOW_EVENT_ADD_LADDER);
	addButton(pWindow, 462, 154, GUI_UI_BUTTON_58PX_GRAY_UP_W, "Shovel", BOT_WINDOW_EVENT_ADD_SHOVEL);
	addButton(pWindow, 524, 154, GUI_UI_BUTTON_75PX_GRAY_UP_W, "Lure", BOT_WINDOW_EVENT_ADD_LURE);
	addButton(pWindow, 462, 178, GUI_UI_BUTTON_58PX_GRAY_UP_W, "Action", BOT_WINDOW_EVENT_ADD_ACTION);
}

static void addTargetTab(GUI_Window* pWindow)
{
	addSmallLabel(pWindow, 24, 63, "Targeting");
	addCheckBox(pWindow, 24, 82, 150, "Enable Targeting", g_bot.isTargetEnabled(), BOT_WINDOW_EVENT_TARGET_ENABLE);
	addCheckBox(pWindow, 24, 104, 150, "Follow Target", g_bot.isFollowTarget(), BOT_WINDOW_EVENT_TARGET_FOLLOW);

	addSmallLabel(pWindow, 24, 132, "Monster Name");
	addTextBox(pWindow, 24, 152, 220, "", BOT_WINDOW_TARGET_TEXT, false, 40);
	addButton(pWindow, 252, 151, GUI_UI_BUTTON_58PX_GRAY_UP_W, "Add", BOT_WINDOW_EVENT_ADD_TARGET);

	GUI_ListBox* listBox = new GUI_ListBox(iRect(24, 182, 286, 46), BOT_WINDOW_TARGET_LIST);
	const std::vector<std::string>& targets = g_bot.getTargetNames();
	for(std::vector<std::string>::const_iterator it = targets.begin(), end = targets.end(); it != end; ++it)
		listBox->add(*it);
	if(!targets.empty())
		listBox->setSelect(0);
	listBox->startEvents();
	pWindow->addChild(listBox);

	addButton(pWindow, 330, 181, GUI_UI_BUTTON_58PX_GRAY_UP_W, "Del", BOT_WINDOW_EVENT_DELETE_TARGET);
	addButton(pWindow, 392, 181, GUI_UI_BUTTON_58PX_GRAY_UP_W, "Clear", BOT_WINDOW_EVENT_CLEAR_TARGETS);

	addSmallLabel(pWindow, 350, 63, "Status");
	GUI_DynamicLabel* label = new GUI_DynamicLabel(iRect(350, 85, 240, 12), g_bot.getStatusText());
	label->setFont(CLIENT_FONT_SMALL);
	pWindow->addChild(label);
}

static void addHealingTab(GUI_Window* pWindow)
{
	const BotHealingConfig& config = g_bot.getHealingConfig();
	addSmallLabel(pWindow, 24, 63, "Auto-Heal");
	addCheckBox(pWindow, 24, 83, 150, "Enable Healing", config.enabled, BOT_WINDOW_EVENT_HEAL_ENABLE);

	addSmallLabel(pWindow, 24, 120, "HP Potion ID");
	addSmallLabel(pWindow, 175, 120, "Health %");
	addTextBox(pWindow, 24, 140, 120, botNumberText(config.hpPotionId), BOT_WINDOW_HP_ITEM_TEXT, true, 5);
	addTextBox(pWindow, 175, 140, 55, botNumberText(config.hpPotionPercent), BOT_WINDOW_HP_PERCENT_TEXT, true, 3);

	addSmallLabel(pWindow, 24, 170, "MP Potion ID");
	addSmallLabel(pWindow, 175, 170, "Mana %");
	addTextBox(pWindow, 24, 190, 120, botNumberText(config.mpPotionId), BOT_WINDOW_MP_ITEM_TEXT, true, 5);
	addTextBox(pWindow, 175, 190, 55, botNumberText(config.mpPotionPercent), BOT_WINDOW_MP_PERCENT_TEXT, true, 3);

	addSmallLabel(pWindow, 300, 120, "UH Rune ID");
	addSmallLabel(pWindow, 450, 120, "Friend HP %");
	addTextBox(pWindow, 300, 140, 120, botNumberText(config.uhRuneId), BOT_WINDOW_UH_ITEM_TEXT, true, 5);
	addTextBox(pWindow, 450, 140, 55, botNumberText(config.friendPercent), BOT_WINDOW_FRIEND_PERCENT_TEXT, true, 3);

	addSmallLabel(pWindow, 300, 170, "Friend Name");
	addTextBox(pWindow, 300, 190, 205, config.friendName, BOT_WINDOW_FRIEND_NAME_TEXT, false, 40);

	addButton(pWindow, 24, 218, GUI_UI_BUTTON_86PX_GRAY_UP_W, "Apply", BOT_WINDOW_EVENT_APPLY_HEAL);
}

static void addLootTab(GUI_Window* pWindow)
{
	addSmallLabel(pWindow, 24, 63, "Loot Item ID or Name");
	addTextBox(pWindow, 24, 82, 180, "", BOT_WINDOW_LOOT_TEXT, false, 40);
	addButton(pWindow, 212, 81, GUI_UI_BUTTON_58PX_GRAY_UP_W, "Add", BOT_WINDOW_EVENT_ADD_LOOT);

	GUI_ListBox* listBox = new GUI_ListBox(iRect(24, 112, 300, 94), BOT_WINDOW_LOOT_LIST);
	const std::vector<BotLootItem>& lootItems = g_bot.getLootItems();
	for(std::vector<BotLootItem>::const_iterator it = lootItems.begin(), end = lootItems.end(); it != end; ++it)
		listBox->add(botLootDisplayText(*it));
	if(!lootItems.empty())
		listBox->setSelect(0);
	listBox->startEvents();
	pWindow->addChild(listBox);

	addButton(pWindow, 24, 214, GUI_UI_BUTTON_58PX_GRAY_UP_W, "Del", BOT_WINDOW_EVENT_DELETE_LOOT);
	addButton(pWindow, 86, 214, GUI_UI_BUTTON_58PX_GRAY_UP_W, "Clear", BOT_WINDOW_EVENT_CLEAR_LOOT);
}

static void addHotkeysTab(GUI_Window* pWindow)
{
	addSmallLabel(pWindow, 24, 63, "Bot Hotkeys");
	addCheckBox(pWindow, 24, 83, 140, "Hotkeys enabled", g_bot.isHotkeysEnabled(), BOT_WINDOW_EVENT_HOTKEYS_ENABLE);
	addSmallLabel(pWindow, 24, 112, "On");
	addSmallLabel(pWindow, 62, 112, "Loop");
	addSmallLabel(pWindow, 112, 112, "Key");
	addSmallLabel(pWindow, 190, 112, "Spell words");
	addSmallLabel(pWindow, 430, 112, "Cooldown ms");

	const std::vector<BotHotkeyConfig>& hotkeys = g_bot.getHotkeys();
	for(size_t i = 0; i < BOT_HOTKEY_ROW_COUNT && i < hotkeys.size(); ++i)
	{
		const BotHotkeyConfig& hotkey = hotkeys[i];
		Sint32 y = 132 + SDL_static_cast(Sint32, i) * 18;
		Uint32 base = BOT_WINDOW_HOTKEY_BASE + SDL_static_cast(Uint32, i) * 10;
		addCheckBox(pWindow, 24, y - 2, 28, "", hotkey.enabled, 0, base + 3);
		addCheckBox(pWindow, 62, y - 2, 28, "", hotkey.persistent, 0, base + 4);
		addTextBox(pWindow, 112, y, 58, hotkey.keyName, base + 0, false, 8);
		addTextBox(pWindow, 190, y, 220, hotkey.spellWords, base + 1, false, 40);
		addTextBox(pWindow, 430, y, 86, botNumberText(hotkey.cooldownMs), base + 2, true, 7);
	}

	addButton(pWindow, 24, 224, GUI_UI_BUTTON_86PX_GRAY_UP_W, "Apply", BOT_WINDOW_EVENT_APPLY_HOTKEYS);
	addSmallLabel(pWindow, 128, 228, "Click Key, then press F1-F12, Home, NUM1-NUM9 or A-Z.");
}

static void addIconsTab(GUI_Window* pWindow)
{
	addSmallLabel(pWindow, 24, 63, "Icons");
	addSmallLabel(pWindow, 24, 84, "On");
	addSmallLabel(pWindow, 58, 84, "ClientID");
	addSmallLabel(pWindow, 126, 84, "Label");
	addSmallLabel(pWindow, 242, 84, "X");
	addSmallLabel(pWindow, 288, 84, "Y");
	addSmallLabel(pWindow, 314, 84, "Left Click Action");
	addSmallLabel(pWindow, 430, 84, "Right Click Action");
	addSmallLabel(pWindow, 546, 84, "Ctrl L");
	addSmallLabel(pWindow, 622, 84, "Ctrl R");

	const std::vector<BotIconConfig>& icons = g_bot.getIcons();
	for(size_t i = 0; i < BOT_ICON_ROW_COUNT && i < icons.size(); ++i)
	{
		const BotIconConfig& icon = icons[i];
		Sint32 y = 102 + SDL_static_cast(Sint32, i) * 24;
		Uint32 base = BOT_WINDOW_ICON_BASE + SDL_static_cast(Uint32, i) * 10;
		addCheckBox(pWindow, 24, y - 2, 28, "", icon.enabled, 0, base + 4);
		addTextBox(pWindow, 58, y, 56, botNumberText(icon.itemId), base + 0, true, 5);
		addTextBox(pWindow, 126, y, 82, icon.label, base + 1, false, 14);
		addTextBox(pWindow, 242, y, 38, std::to_string(icon.x), base + 2, false, 5);
		addTextBox(pWindow, 288, y, 38, std::to_string(icon.y), base + 3, false, 5);
		addTextBox(pWindow, 314, y, 100, icon.leftAction, base + 5, false, 24);
		addTextBox(pWindow, 430, y, 100, icon.rightAction, base + 6, false, 24);
		addTextBox(pWindow, 546, y, 60, icon.ctrlLeftAction, base + 7, false, 24);
		addTextBox(pWindow, 622, y, 60, icon.ctrlRightAction, base + 8, false, 24);
	}

	addButton(pWindow, 24, 238, GUI_UI_BUTTON_86PX_GRAY_UP_W, "Apply", BOT_WINDOW_EVENT_APPLY_ICONS);
	addSmallLabel(pWindow, 128, 242, "X/Y can be negative. Actions: toggleTarget, toggleCavebot, useItem, buy100, exori, utani.");
}

static void addSettingsTab(GUI_Window* pWindow)
{
	addSmallLabel(pWindow, 24, 63, "Settings");
	addCheckBox(pWindow, 24, 86, 180, "Pause cave in fight", g_bot.isPauseCaveInFight(), BOT_WINDOW_EVENT_SETTINGS_PAUSE_CAVE);
	addCheckBox(pWindow, 24, 110, 180, "Follow Target", g_bot.isFollowTarget(), BOT_WINDOW_EVENT_TARGET_FOLLOW);
	addSmallLabel(pWindow, 24, 150, "Save names use .scruopt when no extension is typed.");
}

void UTIL_updateBotPanel()
{
	GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_BOT);
	if(pPanel)
	{
		GUI_DynamicLabel* statusLabel = SDL_static_cast(GUI_DynamicLabel*, pPanel->getChild(BOT_PANEL_STATUS_LABEL));
		if(statusLabel)
			statusLabel->setName(botCompactStatusText());

		GUI_DynamicLabel* waypointLabel = SDL_static_cast(GUI_DynamicLabel*, pPanel->getChild(BOT_PANEL_WAYPOINT_LABEL));
		if(waypointLabel)
		{
			size_t count = g_bot.getWaypointCount();
			size_t current = count == 0 ? 0 : g_bot.getCurrentWaypointIndex() + 1;
			SDL_snprintf(g_buffer, sizeof(g_buffer), "Waypoints: %u / next: %u",
				SDL_static_cast(Uint32, count), SDL_static_cast(Uint32, current));
			waypointLabel->setName(g_buffer);
		}
	}

	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_BOT);
	if(!pWindow)
		return;

	GUI_DynamicLabel* botLabel = SDL_static_cast(GUI_DynamicLabel*, pWindow->getChild(BOT_WINDOW_STATUS_BOT_LABEL));
	if(botLabel)
	{
		botLabel->setName(g_bot.isEnabled() ? "Bot: ON" : "Bot: OFF");
		botLabel->setColor(g_bot.isEnabled() ? 110 : 230, g_bot.isEnabled() ? 230 : 90, g_bot.isEnabled() ? 110 : 90);
	}

	GUI_DynamicLabel* targetLabel = SDL_static_cast(GUI_DynamicLabel*, pWindow->getChild(BOT_WINDOW_STATUS_TARGET_LABEL));
	if(targetLabel)
	{
		targetLabel->setName(g_bot.hasActiveTarget() ? "Target: found" : "Target: no monster");
		targetLabel->setColor(g_bot.hasActiveTarget() ? 110 : 205, g_bot.hasActiveTarget() ? 230 : 190, g_bot.hasActiveTarget() ? 110 : 120);
	}

	bool caveActive = g_bot.isEnabled() && g_bot.isCaveEnabled() && g_bot.getWaypointCount() > 0;
	GUI_DynamicLabel* caveLabel = SDL_static_cast(GUI_DynamicLabel*, pWindow->getChild(BOT_WINDOW_STATUS_CAVE_LABEL));
	if(caveLabel)
	{
		caveLabel->setName(caveActive ? "Cave: active" : "Cave: idle");
		caveLabel->setColor(caveActive ? 110 : 205, caveActive ? 230 : 190, caveActive ? 110 : 120);
	}

	GUI_DynamicLabel* pingLabel = SDL_static_cast(GUI_DynamicLabel*, pWindow->getChild(BOT_WINDOW_STATUS_PING_LABEL));
	if(pingLabel)
	{
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Ping: %u ms", SDL_static_cast(Uint32, g_ping));
		pingLabel->setName(g_buffer);
		pingLabel->setColor(g_ping <= 80 ? 110 : (g_ping <= 180 ? 230 : 230),
			g_ping <= 80 ? 230 : (g_ping <= 180 ? 205 : 90),
			g_ping <= 80 ? 110 : 90);
	}

	GUI_DynamicLabel* posLabel = SDL_static_cast(GUI_DynamicLabel*, pWindow->getChild(BOT_WINDOW_STATUS_POS_LABEL));
	if(posLabel)
	{
		Creature* player = g_map.getLocalCreature();
		Position pos = player ? player->getCurrentPosition() : Position(0, 0, 0);
		SDL_snprintf(g_buffer, sizeof(g_buffer), "Pos: %u %u %u",
			SDL_static_cast(Uint32, pos.x), SDL_static_cast(Uint32, pos.y), SDL_static_cast(Uint32, pos.z));
		posLabel->setName(g_buffer);
	}
}

static void refreshBotWindow()
{
	UTIL_createBotWindow();
	UTIL_updateBotPanel();
}

void bot_Events(Uint32 event, Sint32 status)
{
	switch(event)
	{
		case BOT_PANEL_EVENT_TOGGLE: g_bot.toggleEnabled(); break;
		case BOT_PANEL_EVENT_ADD_WAYPOINT: g_bot.addCurrentWaypoint(CAVEBOT_WAYPOINT_WALK); break;
		case BOT_PANEL_EVENT_CLEAR_WAYPOINTS: g_bot.clearWaypoints(); break;
		case BOT_PANEL_EVENT_TARGET: g_bot.setTargetEnabled(status != 0); break;
		case BOT_PANEL_EVENT_CAVE: g_bot.setCaveEnabled(status != 0); break;
		case BOT_PANEL_EVENT_PAUSE_CAVE: g_bot.setPauseCaveInFight(status != 0); break;
		case BOT_PANEL_EVENT_OPEN_WINDOW: UTIL_toggleBotWindow(); return;

		case BOT_WINDOW_EVENT_CLOSE:
		{
			GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_BOT);
			if(pWindow)
				g_engine.removeWindow(pWindow);
			return;
		}
		case BOT_WINDOW_EVENT_TAB_CAVE: s_botTab = BOT_TAB_CAVE; refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_TAB_TARGET: s_botTab = BOT_TAB_TARGET; refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_TAB_HEALING: s_botTab = BOT_TAB_HEALING; refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_TAB_LOOT: s_botTab = BOT_TAB_LOOT; refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_TAB_HOTKEYS: s_botTab = BOT_TAB_HOTKEYS; refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_TAB_ICONS: s_botTab = BOT_TAB_ICONS; refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_TAB_SETTINGS: s_botTab = BOT_TAB_SETTINGS; refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_START: g_bot.setEnabled(true); break;
		case BOT_WINDOW_EVENT_STOP: g_bot.setEnabled(false); break;
		case BOT_WINDOW_EVENT_SAVE:
			s_botProfileName = botReadText(BOT_WINDOW_PROFILE_TEXT);
			g_bot.saveProfile(s_botProfileName);
			break;
		case BOT_WINDOW_EVENT_LOAD:
			s_botProfileName = botReadText(BOT_WINDOW_PROFILE_TEXT);
			if(g_bot.loadProfile(s_botProfileName))
				refreshBotWindow();
			return;
		case BOT_WINDOW_EVENT_DELETE_PROFILE:
			s_botProfileName = botReadText(BOT_WINDOW_PROFILE_TEXT);
			g_bot.deleteProfile(s_botProfileName);
			return;
		case BOT_WINDOW_EVENT_ADD_WALK: g_bot.addCurrentWaypoint(CAVEBOT_WAYPOINT_WALK); refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_ADD_CENTER: g_bot.addCurrentWaypoint(CAVEBOT_WAYPOINT_CENTER); refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_ADD_STAND: g_bot.addCurrentWaypoint(CAVEBOT_WAYPOINT_STAND); refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_ADD_NODE: g_bot.addCurrentWaypoint(CAVEBOT_WAYPOINT_NODE); refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_ADD_ROPE: g_bot.addCurrentWaypoint(CAVEBOT_WAYPOINT_ROPE); refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_ADD_LADDER: g_bot.addCurrentWaypoint(CAVEBOT_WAYPOINT_LADDER); refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_ADD_SHOVEL: g_bot.addCurrentWaypoint(CAVEBOT_WAYPOINT_SHOVEL); refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_ADD_LURE: g_bot.addCurrentWaypoint(CAVEBOT_WAYPOINT_LURE); refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_ADD_ACTION: g_bot.addCurrentWaypoint(CAVEBOT_WAYPOINT_ACTION); refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_DELETE_WP:
		{
			Sint32 selected = botReadSelected(BOT_WINDOW_WAYPOINT_LIST);
			if(selected >= 0)
				g_bot.eraseWaypoint(SDL_static_cast(size_t, selected));
			refreshBotWindow();
			return;
		}
		case BOT_WINDOW_EVENT_CLEAR_WP: g_bot.clearWaypoints(); refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_CAVE_ENABLE: g_bot.setCaveEnabled(status != 0); break;
		case BOT_WINDOW_EVENT_CAVE_SMART_WALK: g_bot.setSmartWalk(status != 0); break;
		case BOT_WINDOW_EVENT_CAVE_AVOID_PLAYERS: g_bot.setAvoidPlayers(status != 0); break;
		case BOT_WINDOW_EVENT_TARGET_ENABLE: g_bot.setTargetEnabled(status != 0); break;
		case BOT_WINDOW_EVENT_TARGET_FOLLOW: g_bot.setFollowTarget(status != 0); break;
		case BOT_WINDOW_EVENT_ADD_TARGET:
		{
			g_bot.addTargetName(botReadText(BOT_WINDOW_TARGET_TEXT));
			refreshBotWindow();
			return;
		}
		case BOT_WINDOW_EVENT_DELETE_TARGET:
		{
			Sint32 selected = botReadSelected(BOT_WINDOW_TARGET_LIST);
			if(selected >= 0)
				g_bot.eraseTargetName(SDL_static_cast(size_t, selected));
			refreshBotWindow();
			return;
		}
		case BOT_WINDOW_EVENT_CLEAR_TARGETS: g_bot.clearTargetNames(); refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_HEAL_ENABLE: g_bot.setHealingEnabled(status != 0); break;
		case BOT_WINDOW_EVENT_APPLY_HEAL:
		{
			BotHealingConfig config = g_bot.getHealingConfig();
			config.hpPotionId = SDL_static_cast(Uint16, botReadNumber(BOT_WINDOW_HP_ITEM_TEXT));
			config.hpPotionPercent = SDL_static_cast(Uint8, UTIL_min<Uint32>(100, botReadNumber(BOT_WINDOW_HP_PERCENT_TEXT)));
			config.mpPotionId = SDL_static_cast(Uint16, botReadNumber(BOT_WINDOW_MP_ITEM_TEXT));
			config.mpPotionPercent = SDL_static_cast(Uint8, UTIL_min<Uint32>(100, botReadNumber(BOT_WINDOW_MP_PERCENT_TEXT)));
			config.uhRuneId = SDL_static_cast(Uint16, botReadNumber(BOT_WINDOW_UH_ITEM_TEXT));
			config.friendPercent = SDL_static_cast(Uint8, UTIL_min<Uint32>(100, botReadNumber(BOT_WINDOW_FRIEND_PERCENT_TEXT)));
			config.friendName = botReadText(BOT_WINDOW_FRIEND_NAME_TEXT);
			g_bot.setHealingConfig(config);
			refreshBotWindow();
			return;
		}
		case BOT_WINDOW_EVENT_ADD_LOOT:
		{
			g_bot.addLootItem(botReadText(BOT_WINDOW_LOOT_TEXT));
			refreshBotWindow();
			return;
		}
		case BOT_WINDOW_EVENT_DELETE_LOOT:
		{
			Sint32 selected = botReadSelected(BOT_WINDOW_LOOT_LIST);
			if(selected >= 0)
				g_bot.eraseLootItem(SDL_static_cast(size_t, selected));
			refreshBotWindow();
			return;
		}
		case BOT_WINDOW_EVENT_CLEAR_LOOT: g_bot.clearLootItems(); refreshBotWindow(); return;
		case BOT_WINDOW_EVENT_HOTKEYS_ENABLE: g_bot.setHotkeysEnabled(status != 0); break;
		case BOT_WINDOW_EVENT_APPLY_HOTKEYS:
		{
			for(size_t i = 0; i < BOT_HOTKEY_ROW_COUNT && i < g_bot.getHotkeys().size(); ++i)
			{
				Uint32 base = BOT_WINDOW_HOTKEY_BASE + SDL_static_cast(Uint32, i) * 10;
				BotHotkeyConfig hotkey = g_bot.getHotkeys()[i];
				hotkey.keyName = botReadText(base + 0);
				hotkey.spellWords = botReadText(base + 1);
				hotkey.cooldownMs = botReadNumber(base + 2);
				hotkey.enabled = botReadCheck(base + 3);
				hotkey.persistent = botReadCheck(base + 4);
				g_bot.setHotkey(i, hotkey);
			}
			refreshBotWindow();
			return;
		}
		case BOT_WINDOW_EVENT_APPLY_ICONS:
		{
			for(size_t i = 0; i < BOT_ICON_ROW_COUNT && i < g_bot.getIcons().size(); ++i)
			{
				Uint32 base = BOT_WINDOW_ICON_BASE + SDL_static_cast(Uint32, i) * 10;
				BotIconConfig icon = g_bot.getIcons()[i];
				icon.itemId = SDL_static_cast(Uint16, botReadNumber(base + 0));
				icon.label = botReadText(base + 1);
				icon.x = botReadSignedNumber(base + 2);
				icon.y = botReadSignedNumber(base + 3);
				icon.enabled = botReadCheck(base + 4);
				icon.leftAction = botReadText(base + 5);
				icon.rightAction = botReadText(base + 6);
				icon.ctrlLeftAction = botReadText(base + 7);
				icon.ctrlRightAction = botReadText(base + 8);
				g_bot.setIcon(i, icon);
			}
			UTIL_createBotPanel();
			refreshBotWindow();
			return;
		}
		case BOT_WINDOW_EVENT_SETTINGS_PAUSE_CAVE: g_bot.setPauseCaveInFight(status != 0); break;
		default: break;
	}
	UTIL_updateBotPanel();
}

static void populateBotPanel(GUI_PanelWindow* pPanel)
{
	pPanel->clearChilds();
	Sint32 panelHeight = BOT_PANEL_COMPACT_HEIGHT;
	const std::vector<BotIconConfig>& icons = g_bot.getIcons();
	for(std::vector<BotIconConfig>::const_iterator it = icons.begin(), end = icons.end(); it != end; ++it)
	{
		if(it->enabled && it->itemId != 0)
			panelHeight = UTIL_max<Sint32>(panelHeight, BOT_ICON_BASE_Y + it->y + 48);
	}
	panelHeight = UTIL_max<Sint32>(BOT_PANEL_COMPACT_HEIGHT, panelHeight);
	pPanel->setMinHeight(panelHeight);
	pPanel->setMaxHeight(panelHeight);
	pPanel->setCachedHeight(panelHeight);
	pPanel->setSize(GAME_PANEL_FIXED_WIDTH - 4, panelHeight);

	GUI_Label* label = new GUI_Label(iRect(8, 4, 160, 12), "Native Bot", 0, 255, 220, 120);
	label->setFont(CLIENT_FONT_SMALL);
	pPanel->addChild(label);

	GUI_DynamicLabel* dynamicLabel = new GUI_DynamicLabel(iRect(8, 17, 160, 12), botCompactStatusText(), BOT_PANEL_STATUS_LABEL);
	dynamicLabel->setFont(CLIENT_FONT_SMALL);
	pPanel->addChild(dynamicLabel);

	GUI_RadioButton* radioButton = new GUI_RadioButton(iRect(8, 33, GUI_UI_BUTTON_34PX_GRAY_UP_W, GUI_UI_BUTTON_34PX_GRAY_UP_H), "Bot", 0, "Turn native bot on");
	radioButton->setRadioEventCallback(&CheckBotEnabled, "Turn native bot off");
	radioButton->setButtonEventCallback(&bot_Events, BOT_PANEL_EVENT_TOGGLE);
	radioButton->startEvents();
	pPanel->addChild(radioButton);

	GUI_Button* button = new GUI_Button(iRect(45, 33, GUI_UI_BUTTON_58PX_GRAY_UP_W, GUI_UI_BUTTON_58PX_GRAY_UP_H), "Panel", 0, "Open native bot control window");
	button->setButtonEventCallback(&bot_Events, BOT_PANEL_EVENT_OPEN_WINDOW);
	button->startEvents();
	pPanel->addChild(button);

	button = new GUI_Button(iRect(106, 33, GUI_UI_BUTTON_58PX_GRAY_UP_W, GUI_UI_BUTTON_58PX_GRAY_UP_H), "Add WP", 0, "Add your current tile as a cavebot waypoint");
	button->setButtonEventCallback(&bot_Events, BOT_PANEL_EVENT_ADD_WAYPOINT);
	button->startEvents();
	pPanel->addChild(button);

	GUI_CheckBox* checkbox = new GUI_CheckBox(iRect(8, 55, 76, 20), "Target", g_bot.isTargetEnabled());
	checkbox->setBoxEventCallback(&bot_Events, BOT_PANEL_EVENT_TARGET);
	checkbox->startEvents();
	pPanel->addChild(checkbox);

	checkbox = new GUI_CheckBox(iRect(88, 55, 76, 20), "Cave", g_bot.isCaveEnabled());
	checkbox->setBoxEventCallback(&bot_Events, BOT_PANEL_EVENT_CAVE);
	checkbox->startEvents();
	pPanel->addChild(checkbox);

	checkbox = new GUI_CheckBox(iRect(8, 76, 156, 20), "Pause cave in fight", g_bot.isPauseCaveInFight());
	checkbox->setBoxEventCallback(&bot_Events, BOT_PANEL_EVENT_PAUSE_CAVE);
	checkbox->startEvents();
	pPanel->addChild(checkbox);

	dynamicLabel = new GUI_DynamicLabel(iRect(8, 100, 160, 12), "", BOT_PANEL_WAYPOINT_LABEL);
	dynamicLabel->setFont(CLIENT_FONT_SMALL);
	pPanel->addChild(dynamicLabel);

	for(std::vector<BotIconConfig>::const_iterator it = icons.begin(), end = icons.end(); it != end; ++it)
	{
		if(!it->enabled || it->itemId == 0)
			continue;

		GUI_BotItemIcon* itemIcon = new GUI_BotItemIcon(iRect(BOT_ICON_BASE_X + it->x, BOT_ICON_BASE_Y + it->y, 40, 46), *it);
		itemIcon->startEvents();
		pPanel->addChild(itemIcon);
	}
}

void UTIL_createBotPanel()
{
	bool newWindow;
	GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_BOT);
	if(pPanel)
	{
		newWindow = false;
	}
	else
	{
		newWindow = true;
		pPanel = new GUI_PanelWindow(iRect(0, 0, GAME_PANEL_FIXED_WIDTH - 4, BOT_PANEL_COMPACT_HEIGHT), false, GUI_PANEL_WINDOW_BOT);
	}

	populateBotPanel(pPanel);
	if(newWindow)
		g_engine.addToPanel(pPanel, GUI_PANEL_MAIN);
	else
		pPanel->checkPanels();
	UTIL_updateBotPanel();
}

void UTIL_toggleBotWindow()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_BOT);
	if(pWindow)
	{
		g_engine.removeWindow(pWindow);
		return;
	}
	UTIL_createBotWindow();
}

void UTIL_createBotWindow()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_BOT);
	if(pWindow)
		g_engine.removeWindow(pWindow);

	GUI_Window* newWindow = new GUI_Window(iRect(0, 0, BOT_WINDOW_WIDTH, BOT_WINDOW_HEIGHT), "Bot Control Panel", GUI_WINDOW_BOT);
	addWindowShell(newWindow);

	switch(s_botTab)
	{
		case BOT_TAB_CAVE: addCaveTab(newWindow); break;
		case BOT_TAB_TARGET: addTargetTab(newWindow); break;
		case BOT_TAB_HEALING: addHealingTab(newWindow); break;
		case BOT_TAB_LOOT: addLootTab(newWindow); break;
		case BOT_TAB_HOTKEYS: addHotkeysTab(newWindow); break;
		case BOT_TAB_ICONS: addIconsTab(newWindow); break;
		case BOT_TAB_SETTINGS: addSettingsTab(newWindow); break;
		default: break;
	}

	g_engine.addWindow(newWindow, true);
	UTIL_updateBotPanel();
}

bool UTIL_botWindowShouldPassKey(SDL_Event& event)
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_BOT);
	if(!pWindow)
		return false;

	GUI_Element* active = pWindow->getActiveElement();
	if(active)
	{
		Uint32 id = active->getInternalID();
		if(id == BOT_WINDOW_TARGET_TEXT || id == BOT_WINDOW_LOOT_TEXT ||
			id == BOT_WINDOW_PROFILE_TEXT ||
			(id >= BOT_WINDOW_HP_ITEM_TEXT && id <= BOT_WINDOW_FRIEND_NAME_TEXT) ||
			(id >= BOT_WINDOW_HOTKEY_BASE && id < BOT_WINDOW_HOTKEY_BASE + 100) ||
			(id >= BOT_WINDOW_ICON_BASE && id < BOT_WINDOW_ICON_BASE + 100))
			return false;
	}

	SDL_Keycode key = event.key.keysym.sym;
	switch(key)
	{
		case SDLK_UP:
		case SDLK_DOWN:
		case SDLK_LEFT:
		case SDLK_RIGHT:
		case SDLK_w:
		case SDLK_a:
		case SDLK_s:
		case SDLK_d:
		case SDLK_KP_0:
		case SDLK_KP_1:
		case SDLK_KP_2:
		case SDLK_KP_3:
		case SDLK_KP_4:
		case SDLK_KP_5:
		case SDLK_KP_6:
		case SDLK_KP_7:
		case SDLK_KP_8:
		case SDLK_KP_9:
		case SDLK_F1:
		case SDLK_F2:
		case SDLK_F3:
		case SDLK_F4:
		case SDLK_F5:
		case SDLK_F6:
		case SDLK_F7:
		case SDLK_F8:
		case SDLK_F9:
		case SDLK_F10:
		case SDLK_F11:
		case SDLK_F12:
			return true;
		default:
			break;
	}
	return false;
}

bool UTIL_botWindowCaptureKey(SDL_Event& event)
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_BOT);
	if(!pWindow)
		return false;

	GUI_Element* active = pWindow->getActiveElement();
	if(!active)
		return false;

	Uint32 id = active->getInternalID();
	if(id < BOT_WINDOW_HOTKEY_BASE || id >= BOT_WINDOW_HOTKEY_BASE + 100 || ((id - BOT_WINDOW_HOTKEY_BASE) % 10) != 0)
		return false;

	std::string keyName = botKeyNameFromKeycode(event.key.keysym.sym);
	if(keyName.empty())
		return false;

	GUI_TextBox* textBox = SDL_static_cast(GUI_TextBox*, active);
	textBox->setText(keyName);
	return true;
}
