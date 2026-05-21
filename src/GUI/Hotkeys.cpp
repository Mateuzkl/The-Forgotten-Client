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
#include "../GUI_Elements/GUI_ListBox.h"
#include "../GUI_Elements/GUI_Label.h"
#include "../GUI_Elements/GUI_TextBox.h"

// Window dimensions - noticeably narrower than Options (286px)
#define HOTKEYS_TITLE "Hotkey Options"
#define HOTKEYS_WIDTH 220
#define HOTKEYS_HEIGHT 285
#define HOTKEYS_CANCEL_EVENTID 1000
#define HOTKEYS_OK_EVENTID 1001

// Hotkey list
#define HOTKEYS_LIST_X 18
#define HOTKEYS_LIST_Y 30
#define HOTKEYS_LIST_W (HOTKEYS_WIDTH - 36)
#define HOTKEYS_LIST_H 130
#define HOTKEYS_LIST_EVENTID 1002

// "Edit hotkey text:" label
#define HOTKEYS_LABEL_TEXT_X 18
#define HOTKEYS_LABEL_TEXT_Y 168
#define HOTKEYS_LABEL_TEXT_TITLE "Edit hotkey text:"

// Text input box
#define HOTKEYS_TEXTBOX_X 18
#define HOTKEYS_TEXTBOX_Y 182
#define HOTKEYS_TEXTBOX_W (HOTKEYS_WIDTH - 36)
#define HOTKEYS_TEXTBOX_H 16
#define HOTKEYS_TEXTBOX_EVENTID 1003

// "Send automatically" checkbox
#define HOTKEYS_SEND_AUTO_TEXT "Send automatically"
#define HOTKEYS_SEND_AUTO_X 18
#define HOTKEYS_SEND_AUTO_Y 204
#define HOTKEYS_SEND_AUTO_W (HOTKEYS_WIDTH - 36)
#define HOTKEYS_SEND_AUTO_H 22
#define HOTKEYS_SEND_AUTO_EVENTID 1004

// "Available hotkeys:" label
#define HOTKEYS_LABEL_AVAIL_X 18
#define HOTKEYS_LABEL_AVAIL_Y 18
#define HOTKEYS_LABEL_AVAIL_TITLE "Available hotkeys:"

// Total number of configurable hotkeys: F1-F12, Shift+F1-F12, Ctrl+F1-F12
#define HOTKEYS_COUNT 36

extern Engine g_engine;

struct HotkeySlot
{
	const char* label;
	SDL_Keycode key;
	Uint16 mod;
};

static const HotkeySlot s_hotkeySlots[HOTKEYS_COUNT] =
{
	{"F1:",         SDLK_F1,  KMOD_NONE},
	{"F2:",         SDLK_F2,  KMOD_NONE},
	{"F3:",         SDLK_F3,  KMOD_NONE},
	{"F4:",         SDLK_F4,  KMOD_NONE},
	{"F5:",         SDLK_F5,  KMOD_NONE},
	{"F6:",         SDLK_F6,  KMOD_NONE},
	{"F7:",         SDLK_F7,  KMOD_NONE},
	{"F8:",         SDLK_F8,  KMOD_NONE},
	{"F9:",         SDLK_F9,  KMOD_NONE},
	{"F10:",        SDLK_F10, KMOD_NONE},
	{"F11:",        SDLK_F11, KMOD_NONE},
	{"F12:",        SDLK_F12, KMOD_NONE},
	{"Shift+F1:",   SDLK_F1,  KMOD_SHIFT},
	{"Shift+F2:",   SDLK_F2,  KMOD_SHIFT},
	{"Shift+F3:",   SDLK_F3,  KMOD_SHIFT},
	{"Shift+F4:",   SDLK_F4,  KMOD_SHIFT},
	{"Shift+F5:",   SDLK_F5,  KMOD_SHIFT},
	{"Shift+F6:",   SDLK_F6,  KMOD_SHIFT},
	{"Shift+F7:",   SDLK_F7,  KMOD_SHIFT},
	{"Shift+F8:",   SDLK_F8,  KMOD_SHIFT},
	{"Shift+F9:",   SDLK_F9,  KMOD_SHIFT},
	{"Shift+F10:",  SDLK_F10, KMOD_SHIFT},
	{"Shift+F11:",  SDLK_F11, KMOD_SHIFT},
	{"Shift+F12:",  SDLK_F12, KMOD_SHIFT},
	{"Ctrl+F1:",    SDLK_F1,  KMOD_CTRL},
	{"Ctrl+F2:",    SDLK_F2,  KMOD_CTRL},
	{"Ctrl+F3:",    SDLK_F3,  KMOD_CTRL},
	{"Ctrl+F4:",    SDLK_F4,  KMOD_CTRL},
	{"Ctrl+F5:",    SDLK_F5,  KMOD_CTRL},
	{"Ctrl+F6:",    SDLK_F6,  KMOD_CTRL},
	{"Ctrl+F7:",    SDLK_F7,  KMOD_CTRL},
	{"Ctrl+F8:",    SDLK_F8,  KMOD_CTRL},
	{"Ctrl+F9:",    SDLK_F9,  KMOD_CTRL},
	{"Ctrl+F10:",   SDLK_F10, KMOD_CTRL},
	{"Ctrl+F11:",   SDLK_F11, KMOD_CTRL},
	{"Ctrl+F12:",   SDLK_F12, KMOD_CTRL},
};

static std::string getHotkeyText(Sint32 slotIndex)
{
	if(slotIndex < 0 || slotIndex >= HOTKEYS_COUNT)
		return std::string();

	const HotkeySlot& slot = s_hotkeySlots[slotIndex];
	HotkeyUsage* usage = g_engine.getHotkey(slot.key, slot.mod);
	if(!usage || usage->action.type != CLIENT_HOTKEY_ACTION_TEXT)
		return std::string();

	if(usage->action.text.text)
		return *usage->action.text.text;

	return std::string();
}

static bool getHotkeySendAuto(Sint32 slotIndex)
{
	if(slotIndex < 0 || slotIndex >= HOTKEYS_COUNT)
		return false;

	const HotkeySlot& slot = s_hotkeySlots[slotIndex];
	HotkeyUsage* usage = g_engine.getHotkey(slot.key, slot.mod);
	if(!usage || usage->action.type != CLIENT_HOTKEY_ACTION_TEXT)
		return false;

	return usage->action.text.sendAutomatically;
}

// Builds the display string for a list entry.
// sendAuto=true  → white text (both selected and unselected)
// sendAuto=false → grey text (both selected and unselected)
// We use \x0E R G B to override the base color that drawFont uses.
static std::string buildListEntry(Sint32 slotIndex, const std::string& overrideText = std::string(), Sint32 overrideAuto = -1)
{
	std::string text = (overrideText.empty() && overrideAuto == -1) ? getHotkeyText(slotIndex) : overrideText;
	bool sendAuto = (overrideAuto == -1) ? getHotkeySendAuto(slotIndex) : (overrideAuto != 0);

	std::string entry;
	if(sendAuto)
	{
		// White for both selected and unselected rows
		entry += "\x0E\xFF\xFF\xFF";
	}
	else
	{
		// Grey for both selected and unselected rows
		// This overrides the white (255,255,255) that selected rows use by default
		entry += "\x0E\xAF\xAF\xAF";
	}
	entry += s_hotkeySlots[slotIndex].label;
	if(!text.empty())
	{
		entry += " ";
		entry += text;
	}
	entry += "\x0F";

	return entry;
}

static void saveCurrentSlot(Sint32 slotIndex, const std::string& text, bool sendAuto)
{
	if(slotIndex < 0 || slotIndex >= HOTKEYS_COUNT)
		return;

	const HotkeySlot& slot = s_hotkeySlots[slotIndex];
	g_engine.bindHotkeyAction(slot.key, slot.mod, text, sendAuto);
}

static Sint32 s_currentSlot = 0;

// Helper: get the window's listbox, textbox and checkbox
static void getHotkeyControls(GUI_Window* pWindow, GUI_ListBox*& pList, GUI_TextBox*& pText, GUI_CheckBox*& pCheck)
{
	pList  = SDL_static_cast(GUI_ListBox*, pWindow->getChild(HOTKEYS_LIST_EVENTID));
	pText  = SDL_static_cast(GUI_TextBox*, pWindow->getChild(HOTKEYS_TEXTBOX_EVENTID));
	pCheck = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(HOTKEYS_SEND_AUTO_EVENTID));
}

// Live-update the list entry for the current slot from the current UI state
static void liveUpdateCurrentEntry(GUI_Window* pWindow)
{
	GUI_ListBox* pList; GUI_TextBox* pText; GUI_CheckBox* pCheck;
	getHotkeyControls(pWindow, pList, pText, pCheck);
	if(!pList || !pText || !pCheck)
		return;

	std::string currentText = pText->getActualText();
	bool currentAuto = pCheck->isChecked();
	pList->set(s_currentSlot, buildListEntry(s_currentSlot, currentText, currentAuto ? 1 : 0));
}

// State: whether a slot has been selected (textbox unlocked)
static bool s_slotSelected = false;

// Restore focus to the text box - call after any event that might steal focus
static void restoreFocus(GUI_Window* pWindow)
{
	GUI_TextBox* pText = SDL_static_cast(GUI_TextBox*, pWindow->getChild(HOTKEYS_TEXTBOX_EVENTID));
	if(pText)
		pWindow->setActiveElement(pText);
}

void hotkey_Events(Uint32 event, Sint32)
{
	GUI_Window* pWindow = g_engine.getCurrentWindow();
	if(!pWindow || pWindow->getInternalID() != GUI_WINDOW_HOTKEYS)
		return;

	switch(event)
	{
		case HOTKEYS_CANCEL_EVENTID:
			g_engine.removeWindow(pWindow);
			break;

		case HOTKEYS_OK_EVENTID:
		{
			if(s_slotSelected)
			{
				GUI_ListBox* pList; GUI_TextBox* pText; GUI_CheckBox* pCheck;
				getHotkeyControls(pWindow, pList, pText, pCheck);
				if(pText && pCheck)
					saveCurrentSlot(s_currentSlot, pText->getActualText(), pCheck->isChecked());
			}
			g_engine.removeWindow(pWindow);
		}
		break;

		case HOTKEYS_LIST_EVENTID:
		{
			GUI_ListBox* pList; GUI_TextBox* pText; GUI_CheckBox* pCheck;
			getHotkeyControls(pWindow, pList, pText, pCheck);
			if(!pList || !pText || !pCheck)
				break;

			if(s_slotSelected)
			{
				// Save previous slot before switching
				saveCurrentSlot(s_currentSlot, pText->getActualText(), pCheck->isChecked());
				pList->set(s_currentSlot, buildListEntry(s_currentSlot));
			}

			// Switch to new slot
			s_currentSlot = pList->getSelect();
			pText->setText(getHotkeyText(s_currentSlot));
			pCheck->setChecked(getHotkeySendAuto(s_currentSlot));

			if(!s_slotSelected)
			{
				// First selection - unlock textbox and lock focus permanently
				s_slotSelected = true;
				pWindow->setActiveElement(pText);
				pWindow->lockActiveElement(true);
			}
			else
			{
				// Already unlocked - restore focus (listbox click stole it)
				restoreFocus(pWindow);
			}
		}
		break;

		case HOTKEYS_TEXTBOX_EVENTID:
		{
			// Live update list entry as player types
			if(s_slotSelected)
				liveUpdateCurrentEntry(pWindow);
		}
		break;

		case HOTKEYS_SEND_AUTO_EVENTID:
		{
			// Live update + restore focus (checkbox click stole it via lock bypass)
			if(s_slotSelected)
			{
				liveUpdateCurrentEntry(pWindow);
				restoreFocus(pWindow);
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

	s_currentSlot = 0;
	s_slotSelected = false;

	GUI_Window* newWindow = new GUI_Window(iRect(0, 0, HOTKEYS_WIDTH, HOTKEYS_HEIGHT), HOTKEYS_TITLE, GUI_WINDOW_HOTKEYS);

	// "Available hotkeys:" label
	GUI_Label* newLabel = new GUI_Label(iRect(HOTKEYS_LABEL_AVAIL_X, HOTKEYS_LABEL_AVAIL_Y, 0, 0), HOTKEYS_LABEL_AVAIL_TITLE);
	newWindow->addChild(newLabel);

	// Hotkey list
	GUI_ListBox* newListBox = new GUI_ListBox(iRect(HOTKEYS_LIST_X, HOTKEYS_LIST_Y, HOTKEYS_LIST_W, HOTKEYS_LIST_H), HOTKEYS_LIST_EVENTID);
	for(Sint32 i = 0; i < HOTKEYS_COUNT; ++i)
		newListBox->add(buildListEntry(i));
	// No initial selection - player must click a row first
	newListBox->setEventCallback(&hotkey_Events, HOTKEYS_LIST_EVENTID);
	newListBox->startEvents();
	newWindow->addChild(newListBox);

	// "Edit hotkey text:" label
	newLabel = new GUI_Label(iRect(HOTKEYS_LABEL_TEXT_X, HOTKEYS_LABEL_TEXT_Y, 0, 0), HOTKEYS_LABEL_TEXT_TITLE);
	newWindow->addChild(newLabel);

	// Text input box - initially no text, no focus (unlocked after first row selection)
	GUI_TextBox* newTextBox = new GUI_TextBox(iRect(HOTKEYS_TEXTBOX_X, HOTKEYS_TEXTBOX_Y, HOTKEYS_TEXTBOX_W, HOTKEYS_TEXTBOX_H), std::string(), HOTKEYS_TEXTBOX_EVENTID);
	newTextBox->setTextEventCallback(&hotkey_Events, HOTKEYS_TEXTBOX_EVENTID);
	newTextBox->startEvents();
	newWindow->addChild(newTextBox);

	// "Send automatically" checkbox - initially unchecked, no callback until slot selected
	GUI_CheckBox* newCheckBox = new GUI_CheckBox(iRect(HOTKEYS_SEND_AUTO_X, HOTKEYS_SEND_AUTO_Y, HOTKEYS_SEND_AUTO_W, HOTKEYS_SEND_AUTO_H), HOTKEYS_SEND_AUTO_TEXT, false, HOTKEYS_SEND_AUTO_EVENTID);
	newCheckBox->setBoxEventCallback(&hotkey_Events, HOTKEYS_SEND_AUTO_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);

	// Separator
	GUI_Separator* newSeparator = new GUI_Separator(iRect(13, HOTKEYS_HEIGHT - 40, HOTKEYS_WIDTH - 26, 2));
	newWindow->addChild(newSeparator);

	// Ok / Cancel buttons
	GUI_Button* newButton = new GUI_Button(iRect(HOTKEYS_WIDTH - 56, HOTKEYS_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Cancel", CLIENT_GUI_ESCAPE_TRIGGER);
	newButton->setButtonEventCallback(&hotkey_Events, HOTKEYS_CANCEL_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);

	newButton = new GUI_Button(iRect(HOTKEYS_WIDTH - 109, HOTKEYS_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Ok", CLIENT_GUI_ENTER_TRIGGER);
	newButton->setButtonEventCallback(&hotkey_Events, HOTKEYS_OK_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);

	g_engine.addWindow(newWindow, true);
}
