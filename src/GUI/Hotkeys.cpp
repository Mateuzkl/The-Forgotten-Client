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
#include "../GUI_Elements/GUI_Window.h"
#include "../GUI_Elements/GUI_Button.h"
#include "../GUI_Elements/GUI_Separator.h"
#include "../GUI_Elements/GUI_CheckBox.h"
#include "../GUI_Elements/GUI_Grouper.h"
#include "../GUI_Elements/GUI_Label.h"
#include "../GUI_Elements/GUI_ListBox.h"
#include "../GUI_Elements/GUI_TextBox.h"
#include "itemUI.h"

#include <memory>

#define HOTKEYS_TITLE "Hotkey Options"
#define HOTKEYS_WIDTH 285
#define HOTKEYS_HEIGHT 385

#define HOTKEYS_CANCEL_EVENTID 1000
#define HOTKEYS_OK_EVENTID 1001
#define HOTKEYS_LISTBOX_EVENTID 1002
#define HOTKEYS_TEXTBOX_EVENTID 1003
#define HOTKEYS_SEND_AUTO_EVENTID 1004
#define HOTKEYS_SELECT_OBJECT_EVENTID 1005
#define HOTKEYS_CLEAR_OBJECT_EVENTID 1006
#define HOTKEYS_USE_YOURSELF_EVENTID 1007
#define HOTKEYS_USE_TARGET_EVENTID 1008
#define HOTKEYS_CROSSHAIRS_EVENTID 1009
#define HOTKEYS_OBJECT_DISPLAY_EVENTID 1010

#define HOTKEYS_LABEL_X 18
#define HOTKEYS_LABEL_COLOR 223

#define HOTKEYS_AVAILABLE_LABEL_Y 30
#define HOTKEYS_LIST_X 18
#define HOTKEYS_LIST_Y 45
#define HOTKEYS_LIST_W 252
#define HOTKEYS_LIST_H 103

#define HOTKEYS_TEXT_LABEL_Y 162
#define HOTKEYS_TEXTBOX_X 18
#define HOTKEYS_TEXTBOX_Y 178
#define HOTKEYS_TEXTBOX_W 252
#define HOTKEYS_TEXTBOX_H 16

#define HOTKEYS_SEND_AUTO_X 18
#define HOTKEYS_SEND_AUTO_Y 208
#define HOTKEYS_SEND_AUTO_W 252
#define HOTKEYS_SEND_AUTO_H 22

#define HOTKEYS_OBJECT_LABEL_Y 242
#define HOTKEYS_OBJECT_BOX_X 18
#define HOTKEYS_OBJECT_BOX_Y 258
#define HOTKEYS_OBJECT_BOX_W 52
#define HOTKEYS_OBJECT_BOX_H 52

#define HOTKEYS_OBJECT_BUTTON_X 85
#define HOTKEYS_OBJECT_BUTTON_W GUI_UI_BUTTON_86PX_GRAY_UP_W
#define HOTKEYS_OBJECT_BUTTON_H GUI_UI_BUTTON_86PX_GRAY_UP_H
#define HOTKEYS_OBJECT_BUTTON_GAP 33

#define HOTKEYS_USE_BUTTON_X 184
#define HOTKEYS_USE_BUTTON_W GUI_UI_BUTTON_86PX_GRAY_UP_W
#define HOTKEYS_USE_BUTTON_H GUI_UI_BUTTON_86PX_GRAY_UP_H

extern Engine g_engine;

struct ClassicHotkeySlot
{
	const char* name;
	SDL_Keycode keycode;
	Uint16 modifiers;
};

struct ClassicHotkeyEditState
{
	std::string text;
	bool sendAutomatically = false;
	Uint16 itemId = 0;
	Uint8 itemSubtype = 0;
	Uint8 itemUsageType = CLIENT_HOTKEY_ACTION_WITHCROSSHAIRS;
};

static const ClassicHotkeySlot classicHotkeySlots[] =
{
	{"F1:", SDLK_F1, KMOD_NONE},
	{"F2:", SDLK_F2, KMOD_NONE},
	{"F3:", SDLK_F3, KMOD_NONE},
	{"F4:", SDLK_F4, KMOD_NONE},
	{"F5:", SDLK_F5, KMOD_NONE},
	{"F6:", SDLK_F6, KMOD_NONE},
	{"F7:", SDLK_F7, KMOD_NONE},
	{"F8:", SDLK_F8, KMOD_NONE},
	{"F9:", SDLK_F9, KMOD_NONE},
	{"F10:", SDLK_F10, KMOD_NONE},
	{"F11:", SDLK_F11, KMOD_NONE},
	{"F12:", SDLK_F12, KMOD_NONE},
	{"Shift+F1:", SDLK_F1, KMOD_SHIFT},
	{"Shift+F2:", SDLK_F2, KMOD_SHIFT},
	{"Shift+F3:", SDLK_F3, KMOD_SHIFT},
	{"Shift+F4:", SDLK_F4, KMOD_SHIFT},
	{"Shift+F5:", SDLK_F5, KMOD_SHIFT},
	{"Shift+F6:", SDLK_F6, KMOD_SHIFT},
	{"Shift+F7:", SDLK_F7, KMOD_SHIFT},
	{"Shift+F8:", SDLK_F8, KMOD_SHIFT},
	{"Shift+F9:", SDLK_F9, KMOD_SHIFT},
	{"Shift+F10:", SDLK_F10, KMOD_SHIFT},
	{"Shift+F11:", SDLK_F11, KMOD_SHIFT},
	{"Shift+F12:", SDLK_F12, KMOD_SHIFT},
	{"Ctrl+F1:", SDLK_F1, KMOD_CTRL},
	{"Ctrl+F2:", SDLK_F2, KMOD_CTRL},
	{"Ctrl+F3:", SDLK_F3, KMOD_CTRL},
	{"Ctrl+F4:", SDLK_F4, KMOD_CTRL},
	{"Ctrl+F5:", SDLK_F5, KMOD_CTRL},
	{"Ctrl+F6:", SDLK_F6, KMOD_CTRL},
	{"Ctrl+F7:", SDLK_F7, KMOD_CTRL},
	{"Ctrl+F8:", SDLK_F8, KMOD_CTRL},
	{"Ctrl+F9:", SDLK_F9, KMOD_CTRL},
	{"Ctrl+F10:", SDLK_F10, KMOD_CTRL},
	{"Ctrl+F11:", SDLK_F11, KMOD_CTRL},
	{"Ctrl+F12:", SDLK_F12, KMOD_CTRL}
};

static ClassicHotkeyEditState g_classicHotkeyEditStates[SDL_arraysize(classicHotkeySlots)];
static Sint32 g_selectedClassicHotkey = 0;

class GUI_HotkeyItemBox : public GUI_Element
{
	public:
		GUI_HotkeyItemBox(iRect boxRect, Uint32 internalID = 0)
		{
			setRect(boxRect);
			m_internalID = internalID;
		}

		~GUI_HotkeyItemBox() = default;

		void setItemData(Uint16 itemId, Uint8 itemSubtype)
		{
			m_item.reset();
			if(itemId != 0)
				m_item.reset(ItemUI::createItemUI(itemId, itemSubtype));
		}

		void render()
		{
			auto& renderer = g_engine.getRender();
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_DARK_X, GUI_UI_ICON_HORIZONTAL_LINE_DARK_Y, GUI_UI_ICON_HORIZONTAL_LINE_DARK_W, GUI_UI_ICON_HORIZONTAL_LINE_DARK_H, m_tRect.x1, m_tRect.y1, m_tRect.x2, 1);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_DARK_X, GUI_UI_ICON_VERTICAL_LINE_DARK_Y, GUI_UI_ICON_VERTICAL_LINE_DARK_W, GUI_UI_ICON_VERTICAL_LINE_DARK_H, m_tRect.x1, m_tRect.y1 + 1, 1, m_tRect.y2 - 1);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_X, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_Y, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_W, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_H, m_tRect.x1 + 1, m_tRect.y1 + m_tRect.y2 - 1, m_tRect.x2 - 1, 1);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_X, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_Y, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_W, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_H, m_tRect.x1 + m_tRect.x2 - 1, m_tRect.y1 + 1, 1, m_tRect.y2 - 2);
			renderer->fillRectangle(m_tRect.x1 + 1, m_tRect.y1 + 1, m_tRect.x2 - 2, m_tRect.y2 - 2, 64, 64, 64, 255);
			if(m_item)
				m_item->render(m_tRect.x1 + 10, m_tRect.y1 + 10, 32);
		}

	protected:
		std::unique_ptr<ItemUI> m_item;
};

static GUI_TextBox* getHotkeyTextBox(GUI_Window* window)
{
	return SDL_static_cast(GUI_TextBox*, window->getChild(HOTKEYS_TEXTBOX_EVENTID));
}

static GUI_CheckBox* getSendAutoCheckBox(GUI_Window* window)
{
	return SDL_static_cast(GUI_CheckBox*, window->getChild(HOTKEYS_SEND_AUTO_EVENTID));
}

static GUI_HotkeyItemBox* getHotkeyItemBox(GUI_Window* window)
{
	return SDL_static_cast(GUI_HotkeyItemBox*, window->getChild(HOTKEYS_OBJECT_DISPLAY_EVENTID));
}

static const char* getHotkeyItemActionText(Uint8 usageType)
{
	switch(usageType)
	{
		case CLIENT_HOTKEY_ACTION_USEONYOURSELF: return "(use object on yourself)";
		case CLIENT_HOTKEY_ACTION_USEONTARGET: return "(use object on target)";
		case CLIENT_HOTKEY_ACTION_WITHCROSSHAIRS: return "(use object with crosshairs)";
		default: return "(use object)";
	}
}

static const char* getHotkeyItemActionColor(Uint8 usageType)
{
	switch(usageType)
	{
		case CLIENT_HOTKEY_ACTION_USEONYOURSELF: return "\x0E\x99\xFF\x99";
		case CLIENT_HOTKEY_ACTION_USEONTARGET: return "\x0E\xFF\x99\x99";
		case CLIENT_HOTKEY_ACTION_WITHCROSSHAIRS: return "\x0E\xFF\x99\x99";
		default: return "\x0E\xFF\x99\x99";
	}
}

static std::string getHotkeyListText(size_t index)
{
	std::string text = classicHotkeySlots[index].name;
	if(!g_classicHotkeyEditStates[index].text.empty())
	{
		text += " ";
		text += g_classicHotkeyEditStates[index].text;
	}
	else if(g_classicHotkeyEditStates[index].itemId != 0)
	{
		text += " ";
		text += getHotkeyItemActionColor(g_classicHotkeyEditStates[index].itemUsageType);
		text += getHotkeyItemActionText(g_classicHotkeyEditStates[index].itemUsageType);
		text += "\x0F";
	}
	return text;
}

static void refreshHotkeyList(GUI_Window* window)
{
	GUI_ListBox* listBox = SDL_static_cast(GUI_ListBox*, window->getChild(HOTKEYS_LISTBOX_EVENTID));
	if(!listBox)
		return;

	for(size_t i = 0; i < SDL_arraysize(classicHotkeySlots); ++i)
		listBox->set(SDL_static_cast(Sint32, i), getHotkeyListText(i));
}

static void loadHotkeyEditorState()
{
	for(size_t i = 0; i < SDL_arraysize(classicHotkeySlots); ++i)
	{
		HotkeyUsage* hotkey = g_engine.getHotkey(classicHotkeySlots[i].keycode, classicHotkeySlots[i].modifiers);
		if(hotkey && hotkey->action.type == CLIENT_HOTKEY_ACTION_TEXT && hotkey->action.text.text)
		{
			g_classicHotkeyEditStates[i].text = *hotkey->action.text.text;
			g_classicHotkeyEditStates[i].sendAutomatically = hotkey->action.text.sendAutomatically;
			g_classicHotkeyEditStates[i].itemId = 0;
			g_classicHotkeyEditStates[i].itemSubtype = 0;
			g_classicHotkeyEditStates[i].itemUsageType = CLIENT_HOTKEY_ACTION_WITHCROSSHAIRS;
		}
		else if(hotkey && hotkey->action.type >= CLIENT_HOTKEY_ACTION_USEITEM && hotkey->action.type <= CLIENT_HOTKEY_ACTION_WITHCROSSHAIRS && hotkey->action.item.itemId != 0)
		{
			g_classicHotkeyEditStates[i].text.clear();
			g_classicHotkeyEditStates[i].sendAutomatically = false;
			g_classicHotkeyEditStates[i].itemId = hotkey->action.item.itemId;
			g_classicHotkeyEditStates[i].itemSubtype = hotkey->action.item.itemSubtype;
			g_classicHotkeyEditStates[i].itemUsageType = hotkey->action.item.usageType;
		}
		else
		{
			g_classicHotkeyEditStates[i].text.clear();
			g_classicHotkeyEditStates[i].sendAutomatically = false;
			g_classicHotkeyEditStates[i].itemId = 0;
			g_classicHotkeyEditStates[i].itemSubtype = 0;
			g_classicHotkeyEditStates[i].itemUsageType = CLIENT_HOTKEY_ACTION_WITHCROSSHAIRS;
		}
	}
	g_selectedClassicHotkey = 0;
}

static void storeCurrentEditorWidgets(GUI_Window* window)
{
	if(!window)
		return;

	GUI_TextBox* textBox = getHotkeyTextBox(window);
	GUI_CheckBox* sendAutoBox = getSendAutoCheckBox(window);
	if(!textBox || !sendAutoBox)
		return;

	const Sint32 slotCount = SDL_static_cast(Sint32, SDL_arraysize(classicHotkeySlots));
	Sint32 select = UTIL_max<Sint32>(0, UTIL_min<Sint32>(slotCount - 1, g_selectedClassicHotkey));
	g_classicHotkeyEditStates[select].text = textBox->getActualText();
	g_classicHotkeyEditStates[select].sendAutomatically = sendAutoBox->isChecked();
	if(!g_classicHotkeyEditStates[select].text.empty())
		g_classicHotkeyEditStates[select].itemId = 0;
}

static void refreshHotkeyEditor(GUI_Window* window, Sint32 select)
{
	if(!window)
		return;

	const Sint32 slotCount = SDL_static_cast(Sint32, SDL_arraysize(classicHotkeySlots));
	select = UTIL_max<Sint32>(0, UTIL_min<Sint32>(slotCount - 1, select));
	GUI_TextBox* textBox = getHotkeyTextBox(window);
	GUI_CheckBox* sendAutoBox = getSendAutoCheckBox(window);
	GUI_HotkeyItemBox* itemBox = getHotkeyItemBox(window);

	if(textBox)
		textBox->setText(g_classicHotkeyEditStates[select].text);

	if(sendAutoBox)
		sendAutoBox->setChecked(g_classicHotkeyEditStates[select].sendAutomatically);

	if(itemBox)
		itemBox->setItemData(g_classicHotkeyEditStates[select].itemId, g_classicHotkeyEditStates[select].itemSubtype);

	g_selectedClassicHotkey = select;
}

static void saveEditedHotkeys(GUI_Window* window)
{
	if(!window)
		return;

	storeCurrentEditorWidgets(window);

	for(size_t i = 0; i < SDL_arraysize(classicHotkeySlots); ++i)
	{
		if(!g_classicHotkeyEditStates[i].text.empty())
		{
			g_engine.bindHotkeyAction(classicHotkeySlots[i].keycode, classicHotkeySlots[i].modifiers,
				g_classicHotkeyEditStates[i].text, g_classicHotkeyEditStates[i].sendAutomatically);
		}
		else if(g_classicHotkeyEditStates[i].itemId != 0)
		{
			g_engine.bindHotkeyItem(classicHotkeySlots[i].keycode, classicHotkeySlots[i].modifiers,
				g_classicHotkeyEditStates[i].itemId, g_classicHotkeyEditStates[i].itemSubtype, g_classicHotkeyEditStates[i].itemUsageType);
		}
		else
			g_engine.bindHotkeyAction(classicHotkeySlots[i].keycode, classicHotkeySlots[i].modifiers, "", false);
	}
}

void hotkey_Events(Uint32 event, Sint32 status)
{
	switch(event)
	{
		case HOTKEYS_LISTBOX_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_HOTKEYS)
			{
				storeCurrentEditorWidgets(pWindow);
				refreshHotkeyList(pWindow);
				refreshHotkeyEditor(pWindow, status);
			}
		}
		break;
		case HOTKEYS_TEXTBOX_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_HOTKEYS)
			{
				storeCurrentEditorWidgets(pWindow);
				refreshHotkeyList(pWindow);
			}
		}
		break;
		case HOTKEYS_SELECT_OBJECT_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_HOTKEYS)
			{
				storeCurrentEditorWidgets(pWindow);
				const Sint32 slotCount = SDL_static_cast(Sint32, SDL_arraysize(classicHotkeySlots));
				Sint32 select = UTIL_max<Sint32>(0, UTIL_min<Sint32>(slotCount - 1, g_selectedClassicHotkey));
				g_engine.removeWindow(pWindow);
				GUI_Window* optionsWindow = g_engine.getWindow(GUI_WINDOW_OPTIONS);
				if(optionsWindow)
					g_engine.removeWindow(optionsWindow);
				g_engine.beginHotkeyItemSelection(classicHotkeySlots[select].keycode, classicHotkeySlots[select].modifiers);
			}
		}
		break;
		case HOTKEYS_CLEAR_OBJECT_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_HOTKEYS)
			{
				g_classicHotkeyEditStates[g_selectedClassicHotkey].itemId = 0;
				g_classicHotkeyEditStates[g_selectedClassicHotkey].itemSubtype = 0;
				g_classicHotkeyEditStates[g_selectedClassicHotkey].itemUsageType = CLIENT_HOTKEY_ACTION_WITHCROSSHAIRS;
				refreshHotkeyList(pWindow);
				refreshHotkeyEditor(pWindow, g_selectedClassicHotkey);
			}
		}
		break;
		case HOTKEYS_USE_YOURSELF_EVENTID:
		case HOTKEYS_USE_TARGET_EVENTID:
		case HOTKEYS_CROSSHAIRS_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_HOTKEYS && g_classicHotkeyEditStates[g_selectedClassicHotkey].itemId != 0)
			{
				if(event == HOTKEYS_USE_YOURSELF_EVENTID)
					g_classicHotkeyEditStates[g_selectedClassicHotkey].itemUsageType = CLIENT_HOTKEY_ACTION_USEONYOURSELF;
				else if(event == HOTKEYS_USE_TARGET_EVENTID)
					g_classicHotkeyEditStates[g_selectedClassicHotkey].itemUsageType = CLIENT_HOTKEY_ACTION_USEONTARGET;
				else
					g_classicHotkeyEditStates[g_selectedClassicHotkey].itemUsageType = CLIENT_HOTKEY_ACTION_WITHCROSSHAIRS;
				g_classicHotkeyEditStates[g_selectedClassicHotkey].text.clear();
				GUI_TextBox* textBox = getHotkeyTextBox(pWindow);
				if(textBox)
					textBox->setText("");
				refreshHotkeyList(pWindow);
			}
		}
		break;
		case HOTKEYS_CANCEL_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_HOTKEYS)
				g_engine.removeWindow(pWindow);
		}
		break;
		case HOTKEYS_OK_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_HOTKEYS)
			{
				saveEditedHotkeys(pWindow);
				g_engine.removeWindow(pWindow);
			}
		}
		break;
		default: break;
	}
}

void UTIL_hotkeyOptions()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_HOTKEYS);
	if(pWindow)
		g_engine.removeWindow(pWindow);

	loadHotkeyEditorState();

	GUI_Window* newWindow = new GUI_Window(iRect(0, 0, HOTKEYS_WIDTH, HOTKEYS_HEIGHT), HOTKEYS_TITLE, GUI_WINDOW_HOTKEYS);

	GUI_Label* newLabel = new GUI_Label(iRect(HOTKEYS_LABEL_X, HOTKEYS_AVAILABLE_LABEL_Y, 0, 0), "Available hotkeys:", 0, HOTKEYS_LABEL_COLOR, HOTKEYS_LABEL_COLOR, HOTKEYS_LABEL_COLOR);
	newWindow->addChild(newLabel);

	GUI_ListBox* newListBox = new GUI_ListBox(iRect(HOTKEYS_LIST_X, HOTKEYS_LIST_Y, HOTKEYS_LIST_W, HOTKEYS_LIST_H), HOTKEYS_LISTBOX_EVENTID);
	for(size_t i = 0; i < SDL_arraysize(classicHotkeySlots); ++i)
		newListBox->add(getHotkeyListText(i));
	newListBox->setEventCallback(&hotkey_Events, HOTKEYS_LISTBOX_EVENTID);
	newListBox->setSelect(0);
	newListBox->startEvents();
	newWindow->addChild(newListBox);

	newLabel = new GUI_Label(iRect(HOTKEYS_LABEL_X, HOTKEYS_TEXT_LABEL_Y, 0, 0), "Edit hotkey text:", 0, HOTKEYS_LABEL_COLOR, HOTKEYS_LABEL_COLOR, HOTKEYS_LABEL_COLOR);
	newWindow->addChild(newLabel);

	GUI_TextBox* newTextBox = new GUI_TextBox(iRect(HOTKEYS_TEXTBOX_X, HOTKEYS_TEXTBOX_Y, HOTKEYS_TEXTBOX_W, HOTKEYS_TEXTBOX_H), "", HOTKEYS_TEXTBOX_EVENTID, 223, 223, 223);
	newTextBox->setMaxLength(255);
	newTextBox->setTextEventCallback(&hotkey_Events, HOTKEYS_TEXTBOX_EVENTID);
	newTextBox->startEvents();
	newWindow->addChild(newTextBox);

	GUI_CheckBox* newCheckBox = new GUI_CheckBox(iRect(HOTKEYS_SEND_AUTO_X, HOTKEYS_SEND_AUTO_Y, HOTKEYS_SEND_AUTO_W, HOTKEYS_SEND_AUTO_H), "Send automatically", false, HOTKEYS_SEND_AUTO_EVENTID);
	newCheckBox->setBoxEventCallback(&hotkey_Events, HOTKEYS_SEND_AUTO_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);

	newLabel = new GUI_Label(iRect(HOTKEYS_LABEL_X, HOTKEYS_OBJECT_LABEL_Y, 0, 0), "Edit hotkey object:", 0, HOTKEYS_LABEL_COLOR, HOTKEYS_LABEL_COLOR, HOTKEYS_LABEL_COLOR);
	newWindow->addChild(newLabel);

	GUI_HotkeyItemBox* newItemBox = new GUI_HotkeyItemBox(iRect(HOTKEYS_OBJECT_BOX_X, HOTKEYS_OBJECT_BOX_Y, HOTKEYS_OBJECT_BOX_W, HOTKEYS_OBJECT_BOX_H), HOTKEYS_OBJECT_DISPLAY_EVENTID);
	newWindow->addChild(newItemBox);

	GUI_Button* newButton = new GUI_Button(iRect(HOTKEYS_OBJECT_BUTTON_X, HOTKEYS_OBJECT_BOX_Y, HOTKEYS_OBJECT_BUTTON_W, HOTKEYS_OBJECT_BUTTON_H), "Select Object", HOTKEYS_SELECT_OBJECT_EVENTID);
	newButton->setButtonEventCallback(&hotkey_Events, HOTKEYS_SELECT_OBJECT_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);

	newButton = new GUI_Button(iRect(HOTKEYS_OBJECT_BUTTON_X, HOTKEYS_OBJECT_BOX_Y + HOTKEYS_OBJECT_BUTTON_GAP, HOTKEYS_OBJECT_BUTTON_W, HOTKEYS_OBJECT_BUTTON_H), "Clear Object", HOTKEYS_CLEAR_OBJECT_EVENTID);
	newButton->setButtonEventCallback(&hotkey_Events, HOTKEYS_CLEAR_OBJECT_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);

	newButton = new GUI_Button(iRect(HOTKEYS_USE_BUTTON_X, HOTKEYS_OBJECT_BOX_Y, HOTKEYS_USE_BUTTON_W, HOTKEYS_USE_BUTTON_H), "Use on yourself", HOTKEYS_USE_YOURSELF_EVENTID);
	newButton->setButtonEventCallback(&hotkey_Events, HOTKEYS_USE_YOURSELF_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);

	newButton = new GUI_Button(iRect(HOTKEYS_USE_BUTTON_X, HOTKEYS_OBJECT_BOX_Y + HOTKEYS_OBJECT_BUTTON_GAP, HOTKEYS_USE_BUTTON_W, HOTKEYS_USE_BUTTON_H), "Use on target", HOTKEYS_USE_TARGET_EVENTID);
	newButton->setButtonEventCallback(&hotkey_Events, HOTKEYS_USE_TARGET_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);

	newButton = new GUI_Button(iRect(HOTKEYS_USE_BUTTON_X, HOTKEYS_OBJECT_BOX_Y + HOTKEYS_OBJECT_BUTTON_GAP * 2, HOTKEYS_USE_BUTTON_W, HOTKEYS_USE_BUTTON_H), "With crosshairs", HOTKEYS_CROSSHAIRS_EVENTID);
	newButton->setButtonEventCallback(&hotkey_Events, HOTKEYS_CROSSHAIRS_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);

	newButton = new GUI_Button(iRect(HOTKEYS_WIDTH - 56, HOTKEYS_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Cancel", CLIENT_GUI_ESCAPE_TRIGGER);
	newButton->setButtonEventCallback(&hotkey_Events, HOTKEYS_CANCEL_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);

	newButton = new GUI_Button(iRect(HOTKEYS_WIDTH - 109, HOTKEYS_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Ok", CLIENT_GUI_ENTER_TRIGGER);
	newButton->setButtonEventCallback(&hotkey_Events, HOTKEYS_OK_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);

	GUI_Separator* newSeparator = new GUI_Separator(iRect(13, HOTKEYS_HEIGHT - 40, HOTKEYS_WIDTH - 26, 2));
	newWindow->addChild(newSeparator);

	g_engine.addWindow(newWindow, true);
	refreshHotkeyEditor(newWindow, 0);
}
