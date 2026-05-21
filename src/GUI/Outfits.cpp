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
#include "../GUI_Elements/GUI_Label.h"
#include "../GUI_Elements/GUI_CheckBox.h"
#include "../GUI_Elements/GUI_Grouper.h"
#include "../GUI_Elements/GUI_Icon.h"
#include "../thingManager.h"
#include "../game.h"
#include "Outfits.h"

#include <algorithm>
#include <memory>
#include <utility>

#define OUTFITS_TITLE "Select Outfit"
#define OUTFITS_WIDTH 505
#define OUTFITS_HEIGHT 360
#define OUTFITS_CANCEL_EVENTID 1000
#define OUTFITS_OK_EVENTID 1001
#define OUTFITS_RANDOMIZE_EVENTID 1022
#define OUTFITS_LABEL_OUTFIT_X 90
#define OUTFITS_LABEL_OUTFIT_Y 180
#define OUTFITS_LABEL_OUTFIT_EVENTID 1002
#define OUTFITS_LABEL_MOUNT_X 415
#define OUTFITS_LABEL_MOUNT_Y 300
#define OUTFITS_LABEL_MOUNT_EVENTID 1003
#define OUTFITS_GROUPER_OUTFIT_NAME_X 44
#define OUTFITS_GROUPER_OUTFIT_NAME_Y 175
#define OUTFITS_GROUPER_OUTFIT_NAME_W 92
#define OUTFITS_GROUPER_OUTFIT_NAME_H 20
#define OUTFITS_GROUPER_MOUNT_NAME_X 369
#define OUTFITS_GROUPER_MOUNT_NAME_Y 295
#define OUTFITS_GROUPER_MOUNT_NAME_W 92
#define OUTFITS_GROUPER_MOUNT_NAME_H 20
#define OUTFITS_RANDOMIZE_X 44
#define OUTFITS_RANDOMIZE_Y 200
#define OUTFITS_RANDOMIZE_W GUI_UI_BUTTON_86PX_GRAY_UP_W
#define OUTFITS_RANDOMIZE_H GUI_UI_BUTTON_86PX_GRAY_UP_H
#define OUTFITS_CHECKBOX_ADDON1_TEXT "Addon 1"
#define OUTFITS_CHECKBOX_ADDON1_X 190
#define OUTFITS_CHECKBOX_ADDON1_Y 150
#define OUTFITS_CHECKBOX_ADDON1_W 145
#define OUTFITS_CHECKBOX_ADDON1_H 22
#define OUTFITS_CHECKBOX_ADDON1_EVENTID 1009
#define OUTFITS_CHECKBOX_ADDON2_TEXT "Addon 2"
#define OUTFITS_CHECKBOX_ADDON2_X 190
#define OUTFITS_CHECKBOX_ADDON2_Y 177
#define OUTFITS_CHECKBOX_ADDON2_W 145
#define OUTFITS_CHECKBOX_ADDON2_H 22
#define OUTFITS_CHECKBOX_ADDON2_EVENTID 1010
#define OUTFITS_CHECKBOX_ADDON3_TEXT "Addon 3"
#define OUTFITS_CHECKBOX_ADDON3_X 190
#define OUTFITS_CHECKBOX_ADDON3_Y 204
#define OUTFITS_CHECKBOX_ADDON3_W 145
#define OUTFITS_CHECKBOX_ADDON3_H 22
#define OUTFITS_CHECKBOX_ADDON3_EVENTID 1011
#define OUTFITS_ICON_PREV_OUTFIT_X 20
#define OUTFITS_ICON_PREV_OUTFIT_Y 175
#define OUTFITS_ICON_PREV_OUTFIT_W GUI_UI_ICON_ARROW_LEFT_UP_W
#define OUTFITS_ICON_PREV_OUTFIT_H GUI_UI_ICON_ARROW_LEFT_UP_H
#define OUTFITS_ICON_PREV_OUTFIT_EVENTID 1012
#define OUTFITS_ICON_PREV_MOUNT_X 345
#define OUTFITS_ICON_PREV_MOUNT_Y 295
#define OUTFITS_ICON_PREV_MOUNT_W GUI_UI_ICON_ARROW_LEFT_UP_W
#define OUTFITS_ICON_PREV_MOUNT_H GUI_UI_ICON_ARROW_LEFT_UP_H
#define OUTFITS_ICON_PREV_MOUNT_EVENTID 1013
#define OUTFITS_ICON_PREV_OUTFIT_DIRECTION_X 20
#define OUTFITS_ICON_PREV_OUTFIT_DIRECTION_Y 30
#define OUTFITS_ICON_PREV_OUTFIT_DIRECTION_W GUI_UI_ICON_ROTATE_LEFT_UP_W
#define OUTFITS_ICON_PREV_OUTFIT_DIRECTION_H GUI_UI_ICON_ROTATE_LEFT_UP_H
#define OUTFITS_ICON_PREV_OUTFIT_DIRECTION_EVENTID 1014
#define OUTFITS_ICON_NEXT_OUTFIT_X 140
#define OUTFITS_ICON_NEXT_OUTFIT_Y 175
#define OUTFITS_ICON_NEXT_OUTFIT_W GUI_UI_ICON_ARROW_RIGHT_UP_W
#define OUTFITS_ICON_NEXT_OUTFIT_H GUI_UI_ICON_ARROW_RIGHT_UP_H
#define OUTFITS_ICON_NEXT_OUTFIT_EVENTID 1015
#define OUTFITS_ICON_NEXT_MOUNT_X 465
#define OUTFITS_ICON_NEXT_MOUNT_Y 295
#define OUTFITS_ICON_NEXT_MOUNT_W GUI_UI_ICON_ARROW_RIGHT_UP_W
#define OUTFITS_ICON_NEXT_MOUNT_H GUI_UI_ICON_ARROW_RIGHT_UP_H
#define OUTFITS_ICON_NEXT_MOUNT_EVENTID 1016
#define OUTFITS_ICON_NEXT_OUTFIT_DIRECTION_X 140
#define OUTFITS_ICON_NEXT_OUTFIT_DIRECTION_Y 30
#define OUTFITS_ICON_NEXT_OUTFIT_DIRECTION_W GUI_UI_ICON_ROTATE_RIGHT_UP_W
#define OUTFITS_ICON_NEXT_OUTFIT_DIRECTION_H GUI_UI_ICON_ROTATE_RIGHT_UP_H
#define OUTFITS_ICON_NEXT_OUTFIT_DIRECTION_EVENTID 1017
#define OUTFITS_OUTFIT_VIEW_X 20
#define OUTFITS_OUTFIT_VIEW_Y 30
#define OUTFITS_OUTFIT_VIEW_W 140
#define OUTFITS_OUTFIT_VIEW_H 140
#define OUTFITS_OUTFIT_VIEW_EVENTID 1018
#define OUTFITS_MOUNT_VIEW_X 345
#define OUTFITS_MOUNT_VIEW_Y 150
#define OUTFITS_MOUNT_VIEW_W 140
#define OUTFITS_MOUNT_VIEW_H 140
#define OUTFITS_MOUNT_VIEW_EVENTID 1019
#define OUTFITS_ICON_PREV_MOUNT_DIRECTION_X 345
#define OUTFITS_ICON_PREV_MOUNT_DIRECTION_Y 150
#define OUTFITS_ICON_PREV_MOUNT_DIRECTION_W GUI_UI_ICON_ROTATE_LEFT_UP_W
#define OUTFITS_ICON_PREV_MOUNT_DIRECTION_H GUI_UI_ICON_ROTATE_LEFT_UP_H
#define OUTFITS_ICON_PREV_MOUNT_DIRECTION_EVENTID 1020
#define OUTFITS_ICON_NEXT_MOUNT_DIRECTION_X 465
#define OUTFITS_ICON_NEXT_MOUNT_DIRECTION_Y 150
#define OUTFITS_ICON_NEXT_MOUNT_DIRECTION_W GUI_UI_ICON_ROTATE_RIGHT_UP_W
#define OUTFITS_ICON_NEXT_MOUNT_DIRECTION_H GUI_UI_ICON_ROTATE_RIGHT_UP_H
#define OUTFITS_ICON_NEXT_MOUNT_DIRECTION_EVENTID 1021
#define OUTFITS_OUTFIT_COLORS_X 190
#define OUTFITS_OUTFIT_COLORS_Y 35
#define OUTFITS_OUTFIT_COLORS_W 312
#define OUTFITS_OUTFIT_COLORS_H 91
#define OUTFITS_HELP_TEXT_X 20
#define OUTFITS_HELP_TEXT_Y 230
#define OUTFITS_HELP_TEXT_W 310
#define OUTFITS_HELP_TEXT_H 78
#define OUTFITS_COLOR_MODE_W 60
#define OUTFITS_COLOR_MODE_H 20
#define OUTFITS_COLOR_MODE_GAP 5
#define OUTFITS_COLOR_GRID_X 65
#define OUTFITS_COLOR_GRID_CELL 13
#define OUTFITS_COLOR_GRID_W 247
#define OUTFITS_COLOR_GRID_H 91
#define OUTFITS_PREVIEW_RENDER_SIZE 128
#define OUTFITS_PREVIEW_SPRITE_SCALE 56

extern Engine g_engine;
extern Game g_game;
extern ThingManager g_thingManager;
extern Uint32 g_frameTime;

std::vector<OutfitDetail> g_outfits;
std::vector<MountDetail> g_mounts;
Uint16 g_outfitLookType;
Uint16 g_outfitMount;
Uint8 g_outfitColors[4];
Uint8 g_outfitAddons;

namespace
{
	GUI_Window* getOutfitWindow()
	{
		GUI_Window* pWindow = g_engine.getCurrentWindow();
		if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			return pWindow;

		return g_engine.getWindow(GUI_WINDOW_OUTFITS);
	}

	template<typename T, typename... Args>
	T* addWindowChild(GUI_Window& window, Args&&... args)
	{
		std::unique_ptr<T> child = std::make_unique<T>(std::forward<Args>(args)...);
		T* rawChild = child.get();
		window.addChild(child.release());
		return rawChild;
	}

	void clearOutfitState()
	{
		g_outfits.clear();
		g_mounts.clear();
		g_outfits.shrink_to_fit();
		g_mounts.shrink_to_fit();
	}

	GUI_Outfit_View* getOutfitView(GUI_Window* pWindow, Uint32 internalID)
	{
		if(!pWindow)
			return nullptr;

		return SDL_static_cast(GUI_Outfit_View*, pWindow->getChild(internalID));
	}

	void refreshOutfitViews(GUI_Window* pWindow)
	{
		if(GUI_Outfit_View* outfitView = getOutfitView(pWindow, OUTFITS_OUTFIT_VIEW_EVENTID))
			outfitView->refresh();

		if(GUI_Outfit_View* mountView = getOutfitView(pWindow, OUTFITS_MOUNT_VIEW_EVENTID))
			mountView->refresh();
	}

	void setCenteredLabel(GUI_Window* pWindow, Uint32 internalID, const std::string& text)
	{
		if(!pWindow)
			return;

		if(GUI_Label* label = SDL_static_cast(GUI_Label*, pWindow->getChild(internalID)))
			label->setName(text);
	}

	void configureAddonCheckBox(GUI_CheckBox* checkBox, Uint8 bit, bool enabled)
	{
		if(!checkBox)
			return;

		if(enabled)
		{
			checkBox->setColor(180, 180, 180);
			checkBox->startEvents();
		}
		else
		{
			g_outfitAddons &= ~bit;
			checkBox->setColor(112, 112, 112);
			checkBox->stopEvents();
		}

		const bool shouldCheck = ((g_outfitAddons & bit) != 0);
		if(checkBox->isChecked() != shouldCheck)
			checkBox->setChecked(shouldCheck);
	}

	void configureAddonBoxes(GUI_Window* pWindow, Uint8 allowedAddons)
	{
		if(!pWindow)
			return;

		g_outfitAddons &= allowedAddons;
		configureAddonCheckBox(SDL_static_cast(GUI_CheckBox*, pWindow->getChild(OUTFITS_CHECKBOX_ADDON1_EVENTID)), 1, ((allowedAddons & 1) != 0));
		configureAddonCheckBox(SDL_static_cast(GUI_CheckBox*, pWindow->getChild(OUTFITS_CHECKBOX_ADDON2_EVENTID)), 2, ((allowedAddons & 2) != 0));

		if(GUI_CheckBox* addon3 = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(OUTFITS_CHECKBOX_ADDON3_EVENTID)))
		{
			addon3->setColor(112, 112, 112);
			addon3->stopEvents();
		}
	}

	void cycleOutfit(GUI_Window* pWindow, Sint32 step)
	{
		if(!pWindow || g_outfits.empty())
			return;

		auto it = std::find_if(g_outfits.begin(), g_outfits.end(), [](const OutfitDetail& outfit) {
			return outfit.outfitID == g_outfitLookType;
		});
		if(it == g_outfits.end())
			it = g_outfits.begin();
		else if(step < 0)
		{
			if(it == g_outfits.begin())
				it = g_outfits.end();
			--it;
		}
		else
		{
			++it;
			if(it == g_outfits.end())
				it = g_outfits.begin();
		}

		g_outfitLookType = (*it).outfitID;
		g_outfitAddons = (*it).outfitAddons;
		setCenteredLabel(pWindow, OUTFITS_LABEL_OUTFIT_EVENTID, (*it).outfitName);
		configureAddonBoxes(pWindow, (*it).outfitAddons);
		refreshOutfitViews(pWindow);
	}

	void cycleMount(GUI_Window* pWindow, Sint32 step)
	{
		if(!pWindow || g_mounts.empty())
			return;

		Sint32 currentIndex = 0;
		for(size_t i = 0, end = g_mounts.size(); i < end; ++i)
		{
			if(g_mounts[i].mountID == g_outfitMount)
			{
				currentIndex = SDL_static_cast(Sint32, i + 1);
				break;
			}
		}

		const Sint32 mountCount = SDL_static_cast(Sint32, g_mounts.size());
		Sint32 nextIndex = currentIndex + step;
		if(nextIndex < 0)
			nextIndex = mountCount;
		else if(nextIndex > mountCount)
			nextIndex = 0;

		if(nextIndex == 0)
		{
			g_outfitMount = 0;
			setCenteredLabel(pWindow, OUTFITS_LABEL_MOUNT_EVENTID, "No Mount");
		}
		else
		{
			const MountDetail& mount = g_mounts[SDL_static_cast(size_t, nextIndex - 1)];
			g_outfitMount = mount.mountID;
			setCenteredLabel(pWindow, OUTFITS_LABEL_MOUNT_EVENTID, mount.mountName);
		}

		refreshOutfitViews(pWindow);
	}
}

void outfits_Events(Uint32 event, Sint32 status)
{
	switch(event)
	{
		case OUTFITS_CANCEL_EVENTID:
		{
			GUI_Window* pWindow = getOutfitWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				g_engine.removeWindow(pWindow);
				clearOutfitState();
			}
		}
		break;
		case OUTFITS_OK_EVENTID:
		{
			GUI_Window* pWindow = getOutfitWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				g_game.sendSetOutfit(g_outfitLookType, g_outfitColors[0], g_outfitColors[1], g_outfitColors[2], g_outfitColors[3], g_outfitAddons, g_outfitMount);
				g_engine.removeWindow(pWindow);
				clearOutfitState();
			}
		}
		break;
		case OUTFITS_RANDOMIZE_EVENTID:
		{
			GUI_Window* pWindow = getOutfitWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				for(Uint8& color : g_outfitColors)
					color = SDL_static_cast(Uint8, UTIL_random(0, 132));
				refreshOutfitViews(pWindow);
			}
		}
		break;
		case OUTFITS_CHECKBOX_ADDON1_EVENTID:
		{
			GUI_Window* pWindow = getOutfitWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				if(status == 1)
					g_outfitAddons |= 1;
				else
					g_outfitAddons &= ~1;
				refreshOutfitViews(pWindow);
			}
		}
		break;
		case OUTFITS_CHECKBOX_ADDON2_EVENTID:
		{
			GUI_Window* pWindow = getOutfitWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				if(status == 1)
					g_outfitAddons |= 2;
				else
					g_outfitAddons &= ~2;
				refreshOutfitViews(pWindow);
			}
		}
		break;
		case OUTFITS_ICON_PREV_OUTFIT_EVENTID:
		{
			cycleOutfit(getOutfitWindow(), -1);
		}
		break;
		case OUTFITS_ICON_NEXT_OUTFIT_EVENTID:
		{
			cycleOutfit(getOutfitWindow(), 1);
		}
		break;
		case OUTFITS_ICON_PREV_MOUNT_EVENTID:
		{
			cycleMount(getOutfitWindow(), -1);
		}
		break;
		case OUTFITS_ICON_NEXT_MOUNT_EVENTID:
		{
			cycleMount(getOutfitWindow(), 1);
		}
		break;
		case OUTFITS_ICON_PREV_OUTFIT_DIRECTION_EVENTID:
		{
			GUI_Window* pWindow = getOutfitWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				GUI_Outfit_View* pOutfitView = SDL_static_cast(GUI_Outfit_View*, pWindow->getChild(OUTFITS_OUTFIT_VIEW_EVENTID));
				if(pOutfitView)
					pOutfitView->previousDirection();
			}
		}
		break;
		case OUTFITS_ICON_NEXT_OUTFIT_DIRECTION_EVENTID:
		{
			GUI_Window* pWindow = getOutfitWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				GUI_Outfit_View* pOutfitView = SDL_static_cast(GUI_Outfit_View*, pWindow->getChild(OUTFITS_OUTFIT_VIEW_EVENTID));
				if(pOutfitView)
					pOutfitView->nextDirection();
			}
		}
		break;
		case OUTFITS_ICON_PREV_MOUNT_DIRECTION_EVENTID:
		{
			GUI_Window* pWindow = getOutfitWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				GUI_Outfit_View* pMountView = SDL_static_cast(GUI_Outfit_View*, pWindow->getChild(OUTFITS_MOUNT_VIEW_EVENTID));
				if(pMountView)
					pMountView->previousDirection();
			}
		}
		break;
		case OUTFITS_ICON_NEXT_MOUNT_DIRECTION_EVENTID:
		{
			GUI_Window* pWindow = getOutfitWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_OUTFITS)
			{
				GUI_Outfit_View* pMountView = SDL_static_cast(GUI_Outfit_View*, pWindow->getChild(OUTFITS_MOUNT_VIEW_EVENTID));
				if(pMountView)
					pMountView->nextDirection();
			}
		}
		break;
		default: break;
	}
}

void UTIL_createOutfitWindow(Uint16 lookType, Uint8 lookHead, Uint8 lookBody, Uint8 lookLegs, Uint8 lookFeet, Uint8 lookAddons, Uint16 lookMount, const std::vector<OutfitDetail>& outfits, const std::vector<MountDetail>& mounts)
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_OUTFITS);
	if(pWindow)
		g_engine.removeWindow(pWindow);

	const bool mountsEnabled = g_game.hasGameFeature(GAME_FEATURE_MOUNTS);
	std::vector<OutfitDetail> windowOutfits = outfits;
	std::vector<MountDetail> windowMounts = (mountsEnabled ? mounts : std::vector<MountDetail>());
	if(!mountsEnabled)
		lookMount = 0;

	if(lookType == 0)
	{
		if(windowOutfits.empty())
			return;
		lookType = (*windowOutfits.begin()).outfitID;
	}

	std::string outfitName = "Your Character";
	std::string mountName = "No Mount";
	Uint8 outfitAddons = 0;
	bool foundOutfit = false;
	bool foundMount = false;
	for(const OutfitDetail& outfit : windowOutfits)
	{
		if(outfit.outfitID == lookType)
		{
			outfitName = outfit.outfitName;
			outfitAddons = outfit.outfitAddons;
			foundOutfit = true;
			break;
		}
	}

	if(lookMount != 0)
	{
		for(const MountDetail& mount : windowMounts)
		{
			if(mount.mountID == lookMount)
			{
				mountName = mount.mountName;
				foundMount = true;
				break;
			}
		}

		if(!foundMount)
		{
			MountDetail newMount;
			newMount.mountID = lookMount;
			newMount.mountName = "Your Mount";
			windowMounts.push_back(newMount);
			mountName = "Your Mount";
			foundMount = true;
		}
	}

	if(!foundOutfit)
	{
		OutfitDetail newOutfit;
		newOutfit.outfitID = lookType;
		newOutfit.outfitName = "Your Character";
		newOutfit.outfitAddons = 0;
		windowOutfits.push_back(newOutfit);
	}

	g_outfits = std::move(windowOutfits);
	g_mounts = std::move(windowMounts);
	g_outfitLookType = lookType;
	g_outfitMount = lookMount;
	g_outfitColors[0] = lookHead;
	g_outfitColors[1] = lookBody;
	g_outfitColors[2] = lookLegs;
	g_outfitColors[3] = lookFeet;
	g_outfitAddons = (lookAddons & outfitAddons);

	std::unique_ptr<GUI_Window> newWindow = std::make_unique<GUI_Window>(iRect(0, 0, OUTFITS_WIDTH, OUTFITS_HEIGHT), OUTFITS_TITLE, GUI_WINDOW_OUTFITS);

	addWindowChild<GUI_Outfit_View>(*newWindow, iRect(OUTFITS_OUTFIT_VIEW_X, OUTFITS_OUTFIT_VIEW_Y, OUTFITS_OUTFIT_VIEW_W, OUTFITS_OUTFIT_VIEW_H), OUTFITS_OUTFIT_VIEW_EVENTID, GUI_Outfit_View::PreviewMode_Outfit);
	if(mountsEnabled)
		addWindowChild<GUI_Outfit_View>(*newWindow, iRect(OUTFITS_MOUNT_VIEW_X, OUTFITS_MOUNT_VIEW_Y, OUTFITS_MOUNT_VIEW_W, OUTFITS_MOUNT_VIEW_H), OUTFITS_MOUNT_VIEW_EVENTID, GUI_Outfit_View::PreviewMode_Mount);

	GUI_Outfit_Colors* newColors = addWindowChild<GUI_Outfit_Colors>(*newWindow, iRect(OUTFITS_OUTFIT_COLORS_X, OUTFITS_OUTFIT_COLORS_Y, OUTFITS_OUTFIT_COLORS_W, OUTFITS_OUTFIT_COLORS_H));
	newColors->startEvents();

	GUI_Icon* newIcon = addWindowChild<GUI_Icon>(*newWindow, iRect(OUTFITS_ICON_PREV_OUTFIT_X, OUTFITS_ICON_PREV_OUTFIT_Y, OUTFITS_ICON_PREV_OUTFIT_W, OUTFITS_ICON_PREV_OUTFIT_H), GUI_UI_IMAGE, GUI_UI_ICON_ARROW_LEFT_UP_X, GUI_UI_ICON_ARROW_LEFT_UP_Y, GUI_UI_ICON_ARROW_LEFT_DOWN_X, GUI_UI_ICON_ARROW_LEFT_DOWN_Y);
	newIcon->setButtonEventCallback(&outfits_Events, OUTFITS_ICON_PREV_OUTFIT_EVENTID);
	newIcon->startEvents();

	newIcon = addWindowChild<GUI_Icon>(*newWindow, iRect(OUTFITS_ICON_NEXT_OUTFIT_X, OUTFITS_ICON_NEXT_OUTFIT_Y, OUTFITS_ICON_NEXT_OUTFIT_W, OUTFITS_ICON_NEXT_OUTFIT_H), GUI_UI_IMAGE, GUI_UI_ICON_ARROW_RIGHT_UP_X, GUI_UI_ICON_ARROW_RIGHT_UP_Y, GUI_UI_ICON_ARROW_RIGHT_DOWN_X, GUI_UI_ICON_ARROW_RIGHT_DOWN_Y);
	newIcon->setButtonEventCallback(&outfits_Events, OUTFITS_ICON_NEXT_OUTFIT_EVENTID);
	newIcon->startEvents();

	if(mountsEnabled)
	{
		newIcon = addWindowChild<GUI_Icon>(*newWindow, iRect(OUTFITS_ICON_PREV_MOUNT_X, OUTFITS_ICON_PREV_MOUNT_Y, OUTFITS_ICON_PREV_MOUNT_W, OUTFITS_ICON_PREV_MOUNT_H), GUI_UI_IMAGE, GUI_UI_ICON_ARROW_LEFT_UP_X, GUI_UI_ICON_ARROW_LEFT_UP_Y, GUI_UI_ICON_ARROW_LEFT_DOWN_X, GUI_UI_ICON_ARROW_LEFT_DOWN_Y);
		newIcon->setButtonEventCallback(&outfits_Events, OUTFITS_ICON_PREV_MOUNT_EVENTID);
		newIcon->startEvents();

		newIcon = addWindowChild<GUI_Icon>(*newWindow, iRect(OUTFITS_ICON_NEXT_MOUNT_X, OUTFITS_ICON_NEXT_MOUNT_Y, OUTFITS_ICON_NEXT_MOUNT_W, OUTFITS_ICON_NEXT_MOUNT_H), GUI_UI_IMAGE, GUI_UI_ICON_ARROW_RIGHT_UP_X, GUI_UI_ICON_ARROW_RIGHT_UP_Y, GUI_UI_ICON_ARROW_RIGHT_DOWN_X, GUI_UI_ICON_ARROW_RIGHT_DOWN_Y);
		newIcon->setButtonEventCallback(&outfits_Events, OUTFITS_ICON_NEXT_MOUNT_EVENTID);
		newIcon->startEvents();
	}

	newIcon = addWindowChild<GUI_Icon>(*newWindow, iRect(OUTFITS_ICON_PREV_OUTFIT_DIRECTION_X, OUTFITS_ICON_PREV_OUTFIT_DIRECTION_Y, OUTFITS_ICON_PREV_OUTFIT_DIRECTION_W, OUTFITS_ICON_PREV_OUTFIT_DIRECTION_H), GUI_UI_IMAGE, GUI_UI_ICON_ROTATE_LEFT_UP_X, GUI_UI_ICON_ROTATE_LEFT_UP_Y, GUI_UI_ICON_ROTATE_LEFT_DOWN_X, GUI_UI_ICON_ROTATE_LEFT_DOWN_Y);
	newIcon->setButtonEventCallback(&outfits_Events, OUTFITS_ICON_PREV_OUTFIT_DIRECTION_EVENTID);
	newIcon->startEvents();

	newIcon = addWindowChild<GUI_Icon>(*newWindow, iRect(OUTFITS_ICON_NEXT_OUTFIT_DIRECTION_X, OUTFITS_ICON_NEXT_OUTFIT_DIRECTION_Y, OUTFITS_ICON_NEXT_OUTFIT_DIRECTION_W, OUTFITS_ICON_NEXT_OUTFIT_DIRECTION_H), GUI_UI_IMAGE, GUI_UI_ICON_ROTATE_RIGHT_UP_X, GUI_UI_ICON_ROTATE_RIGHT_UP_Y, GUI_UI_ICON_ROTATE_RIGHT_DOWN_X, GUI_UI_ICON_ROTATE_RIGHT_DOWN_Y);
	newIcon->setButtonEventCallback(&outfits_Events, OUTFITS_ICON_NEXT_OUTFIT_DIRECTION_EVENTID);
	newIcon->startEvents();

	if(mountsEnabled)
	{
		newIcon = addWindowChild<GUI_Icon>(*newWindow, iRect(OUTFITS_ICON_PREV_MOUNT_DIRECTION_X, OUTFITS_ICON_PREV_MOUNT_DIRECTION_Y, OUTFITS_ICON_PREV_MOUNT_DIRECTION_W, OUTFITS_ICON_PREV_MOUNT_DIRECTION_H), GUI_UI_IMAGE, GUI_UI_ICON_ROTATE_LEFT_UP_X, GUI_UI_ICON_ROTATE_LEFT_UP_Y, GUI_UI_ICON_ROTATE_LEFT_DOWN_X, GUI_UI_ICON_ROTATE_LEFT_DOWN_Y);
		newIcon->setButtonEventCallback(&outfits_Events, OUTFITS_ICON_PREV_MOUNT_DIRECTION_EVENTID);
		newIcon->startEvents();

		newIcon = addWindowChild<GUI_Icon>(*newWindow, iRect(OUTFITS_ICON_NEXT_MOUNT_DIRECTION_X, OUTFITS_ICON_NEXT_MOUNT_DIRECTION_Y, OUTFITS_ICON_NEXT_MOUNT_DIRECTION_W, OUTFITS_ICON_NEXT_MOUNT_DIRECTION_H), GUI_UI_IMAGE, GUI_UI_ICON_ROTATE_RIGHT_UP_X, GUI_UI_ICON_ROTATE_RIGHT_UP_Y, GUI_UI_ICON_ROTATE_RIGHT_DOWN_X, GUI_UI_ICON_ROTATE_RIGHT_DOWN_Y);
		newIcon->setButtonEventCallback(&outfits_Events, OUTFITS_ICON_NEXT_MOUNT_DIRECTION_EVENTID);
		newIcon->startEvents();
	}

	addWindowChild<GUI_Grouper>(*newWindow, iRect(OUTFITS_GROUPER_OUTFIT_NAME_X, OUTFITS_GROUPER_OUTFIT_NAME_Y, OUTFITS_GROUPER_OUTFIT_NAME_W, OUTFITS_GROUPER_OUTFIT_NAME_H));
	if(mountsEnabled)
		addWindowChild<GUI_Grouper>(*newWindow, iRect(OUTFITS_GROUPER_MOUNT_NAME_X, OUTFITS_GROUPER_MOUNT_NAME_Y, OUTFITS_GROUPER_MOUNT_NAME_W, OUTFITS_GROUPER_MOUNT_NAME_H));

	GUI_CheckBox* newCheckBox = addWindowChild<GUI_CheckBox>(*newWindow, iRect(OUTFITS_CHECKBOX_ADDON1_X, OUTFITS_CHECKBOX_ADDON1_Y, OUTFITS_CHECKBOX_ADDON1_W, OUTFITS_CHECKBOX_ADDON1_H), OUTFITS_CHECKBOX_ADDON1_TEXT, ((g_outfitAddons & 1) != 0), OUTFITS_CHECKBOX_ADDON1_EVENTID);
	newCheckBox->setBoxEventCallback(&outfits_Events, OUTFITS_CHECKBOX_ADDON1_EVENTID);

	newCheckBox = addWindowChild<GUI_CheckBox>(*newWindow, iRect(OUTFITS_CHECKBOX_ADDON2_X, OUTFITS_CHECKBOX_ADDON2_Y, OUTFITS_CHECKBOX_ADDON2_W, OUTFITS_CHECKBOX_ADDON2_H), OUTFITS_CHECKBOX_ADDON2_TEXT, ((g_outfitAddons & 2) != 0), OUTFITS_CHECKBOX_ADDON2_EVENTID);
	newCheckBox->setBoxEventCallback(&outfits_Events, OUTFITS_CHECKBOX_ADDON2_EVENTID);

	newCheckBox = addWindowChild<GUI_CheckBox>(*newWindow, iRect(OUTFITS_CHECKBOX_ADDON3_X, OUTFITS_CHECKBOX_ADDON3_Y, OUTFITS_CHECKBOX_ADDON3_W, OUTFITS_CHECKBOX_ADDON3_H), OUTFITS_CHECKBOX_ADDON3_TEXT, false, OUTFITS_CHECKBOX_ADDON3_EVENTID, SDL_static_cast(Uint8, 112), SDL_static_cast(Uint8, 112), SDL_static_cast(Uint8, 112));
	newCheckBox->stopEvents();

	GUI_Label* newLabel = addWindowChild<GUI_Label>(*newWindow, iRect(OUTFITS_LABEL_OUTFIT_X, OUTFITS_LABEL_OUTFIT_Y, 0, 0), outfitName, OUTFITS_LABEL_OUTFIT_EVENTID);
	newLabel->setAlign(CLIENT_FONT_ALIGN_CENTER);
	if(mountsEnabled)
	{
		newLabel = addWindowChild<GUI_Label>(*newWindow, iRect(OUTFITS_LABEL_MOUNT_X, OUTFITS_LABEL_MOUNT_Y, 0, 0), mountName, OUTFITS_LABEL_MOUNT_EVENTID);
		newLabel->setAlign(CLIENT_FONT_ALIGN_CENTER);
	}

	GUI_Button* newButton = addWindowChild<GUI_Button>(*newWindow, iRect(OUTFITS_RANDOMIZE_X, OUTFITS_RANDOMIZE_Y, OUTFITS_RANDOMIZE_W, OUTFITS_RANDOMIZE_H), "Randomize Colors");
	newButton->setButtonEventCallback(&outfits_Events, OUTFITS_RANDOMIZE_EVENTID);
	newButton->startEvents();

	newButton = addWindowChild<GUI_Button>(*newWindow, iRect(OUTFITS_WIDTH - 56, OUTFITS_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Cancel", CLIENT_GUI_ESCAPE_TRIGGER);
	newButton->setButtonEventCallback(&outfits_Events, OUTFITS_CANCEL_EVENTID);
	newButton->startEvents();

	newButton = addWindowChild<GUI_Button>(*newWindow, iRect(OUTFITS_WIDTH - 109, OUTFITS_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Ok", CLIENT_GUI_ENTER_TRIGGER);
	newButton->setButtonEventCallback(&outfits_Events, OUTFITS_OK_EVENTID);
	newButton->startEvents();

	addWindowChild<GUI_Outfit_HelpText>(*newWindow, iRect(OUTFITS_HELP_TEXT_X, OUTFITS_HELP_TEXT_Y, OUTFITS_HELP_TEXT_W, OUTFITS_HELP_TEXT_H));
	addWindowChild<GUI_Separator>(*newWindow, iRect(13, OUTFITS_HEIGHT - 40, OUTFITS_WIDTH - 26, 2));

	configureAddonBoxes(newWindow.get(), outfitAddons);
	g_engine.addWindow(newWindow.release());
}

GUI_Outfit_View::GUI_Outfit_View(iRect boxRect, Uint32 internalID, PreviewMode mode) : m_currentFrame(ThingFrameGroup_Idle), m_mode(mode)
{
	setRect(boxRect);
	m_internalID = internalID;
	m_outfit = g_thingManager.getThingType(ThingCategory_Creature, g_outfitLookType);
	m_mount = (g_outfitMount == 0 ? nullptr : g_thingManager.getThingType(ThingCategory_Creature, g_outfitMount));
	resetAnimation();
}

void GUI_Outfit_View::previousDirection()
{
	if(m_direction == DIRECTION_WEST)
		m_direction = DIRECTION_NORTH;
	else
		++m_direction;
}

void GUI_Outfit_View::nextDirection()
{
	if(m_direction == DIRECTION_NORTH)
		m_direction = DIRECTION_WEST;
	else
		--m_direction;
}

void GUI_Outfit_View::refresh()
{
	m_outfit = g_thingManager.getThingType(ThingCategory_Creature, g_outfitLookType);
	m_mount = (g_outfitMount == 0 ? nullptr : g_thingManager.getThingType(ThingCategory_Creature, g_outfitMount));
	m_showOutfit = (m_mode == PreviewMode_Outfit);
	m_showMount = (m_mode == PreviewMode_Mount);

	resetAnimation();
}

void GUI_Outfit_View::startMovement()
{
	m_walking = true;
	m_outfitAnim = 0;
	m_mountAnim = 0;
	if(!g_game.hasGameFeature(GAME_FEATURE_FRAMEGROUPS) || (m_outfit && m_outfit->hasFlag(ThingAttribute_AnimateAlways)))
		m_currentFrame = ThingFrameGroup_Idle;
	else
		m_currentFrame = ThingFrameGroup_Moving;

	m_walkStartTime = g_frameTime;
	resetAnimation();
}

void GUI_Outfit_View::stopMovement()
{
	m_walking = false;
	m_outfitAnim = 0;
	m_mountAnim = 0;
	m_currentFrame = ThingFrameGroup_Idle;
	resetAnimation();
}

void GUI_Outfit_View::resetAnimation()
{
	if(m_outfit && m_outfit->m_frameGroup[m_currentFrame].m_animator)
		m_outfit->m_frameGroup[m_currentFrame].m_animator->resetAnimation(m_outfitAnimation[m_currentFrame]);
	
	if(m_mount && m_mount->m_frameGroup[m_currentFrame].m_animator)
		m_mount->m_frameGroup[m_currentFrame].m_animator->resetAnimation(m_mountAnimation[m_currentFrame]);
}

void GUI_Outfit_View::updateMovement()
{
	static const Uint32 walkTime = 500;//500ms per tile
	if(m_walking)
	{
		float walkTicks = (walkTime * 0.03125f);
		m_walkedPixels = SDL_static_cast(Sint32, (g_frameTime - m_walkStartTime) / walkTicks);
	}
	if(m_outfit)
	{
		if(m_outfit->m_frameGroup[m_currentFrame].m_animator)
		{
			//Calculate with new animation
			m_outfitAnim = SDL_static_cast(Uint8, m_outfit->m_frameGroup[m_currentFrame].m_animator->getPhase(m_outfitAnimation[m_currentFrame], (m_currentFrame == ThingFrameGroup_Idle ? 0 : walkTime)));
		}
		else
		{
			//Calculate with old animation
			if(m_outfit->hasFlag(ThingAttribute_AnimateAlways))
				m_outfitAnim = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / CREATURE_TICKS_PER_FRAME)), m_outfit->m_frameGroup[m_currentFrame].m_animCount);
			else if(m_walking)
			{
				Uint8 animCount = m_outfit->m_frameGroup[m_currentFrame].m_animCount;
				if(animCount > 1)
					m_outfitAnim = (SDL_static_cast(Uint8, (m_walkedPixels % 32) / 8) % (animCount - 1)) + 1;
				else
					m_outfitAnim = 0;
			}
			else
				m_outfitAnim = 0;
		}
	}
	if(m_mount)
	{
		if(m_mount->m_frameGroup[m_currentFrame].m_animator)
		{
			//Calculate with new animation
			m_mountAnim = SDL_static_cast(Uint8, m_mount->m_frameGroup[m_currentFrame].m_animator->getPhase(m_mountAnimation[m_currentFrame], (m_currentFrame == ThingFrameGroup_Idle ? 0 : walkTime)));
		}
		else
		{
			//Calculate with old animation
			if(m_mount->hasFlag(ThingAttribute_AnimateAlways))
				m_mountAnim = UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / CREATURE_TICKS_PER_FRAME)), m_mount->m_frameGroup[m_currentFrame].m_animCount);
			else if(m_walking)
			{
				Uint8 animCount = m_mount->m_frameGroup[m_currentFrame].m_animCount;
				if(animCount > 1)
					m_mountAnim = (SDL_static_cast(Uint8, (m_walkedPixels % 32) / 8) % (animCount - 1)) + 1;
				else
					m_mountAnim = 0;
			}
			else
				m_mountAnim = 0;
		}
	}
}

Sint32 GUI_Outfit_View::getOffsetX()
{
	if(!m_walking) return 0;
	switch(m_direction)
	{
		case DIRECTION_NORTH:
			return 0;
		case DIRECTION_EAST:
			return (m_walkedPixels % 32);
		case DIRECTION_SOUTH:
			return 0;
		case DIRECTION_WEST:
			return -(m_walkedPixels % 32);
		default: return 0;
	}
}

Sint32 GUI_Outfit_View::getOffsetY()
{
	if(!m_walking) return 0;
	switch(m_direction)
	{
		case DIRECTION_NORTH:
			return -(m_walkedPixels % 32);
		case DIRECTION_EAST:
			return 0;
		case DIRECTION_SOUTH:
			return (m_walkedPixels % 32);
		case DIRECTION_WEST:
			return 0;
		default: return 0;
	}
}

void GUI_Outfit_View::renderPanel()
{
	auto& renderer = g_engine.getRender();
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_DARK_X, GUI_UI_ICON_HORIZONTAL_LINE_DARK_Y, GUI_UI_ICON_HORIZONTAL_LINE_DARK_W, GUI_UI_ICON_HORIZONTAL_LINE_DARK_H, m_tRect.x1, m_tRect.y1, m_tRect.x2, 1);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_DARK_X, GUI_UI_ICON_VERTICAL_LINE_DARK_Y, GUI_UI_ICON_VERTICAL_LINE_DARK_W, GUI_UI_ICON_VERTICAL_LINE_DARK_H, m_tRect.x1, m_tRect.y1 + 1, 1, m_tRect.y2 - 1);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_X, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_Y, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_W, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_H, m_tRect.x1 + 1, m_tRect.y1 + m_tRect.y2 - 1, m_tRect.x2 - 1, 1);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_X, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_Y, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_W, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_H, m_tRect.x1 + m_tRect.x2 - 1, m_tRect.y1 + 1, 1, m_tRect.y2 - 2);
}

void GUI_Outfit_View::renderCreature(ThingType* thingType, Uint8 animation, Uint8 addonPattern, Uint8 zPattern, Uint32 outfitColor)
{
	if(!thingType)
		return;

	ThingFrameGroup frameGroup = SDL_static_cast(ThingFrameGroup, m_currentFrame);
	if(addonPattern >= thingType->m_frameGroup[m_currentFrame].m_patternY)
		return;

	auto& renderer = g_engine.getRender();
	const Sint32 spriteScale = OUTFITS_PREVIEW_SPRITE_SCALE;
	const Sint32 scaleOffset = (spriteScale - 32) / 2;
	const Sint32 displacementX = thingType->m_displacement[0] * spriteScale / 32;
	const Sint32 displacementY = thingType->m_displacement[1] * spriteScale / 32;
	Sint32 posYc = 80 - displacementY - scaleOffset;
	for(Uint8 cy = 0; cy < thingType->m_frameGroup[m_currentFrame].m_height; ++cy)
	{
		Sint32 posXc = 64 - displacementX - scaleOffset;
		for(Uint8 cx = 0; cx < thingType->m_frameGroup[m_currentFrame].m_width; ++cx)
		{
			Uint32 sprite = thingType->getSprite(frameGroup, cx, cy, 0, m_direction, addonPattern, zPattern, animation);
			if(sprite != 0)
			{
				if(thingType->m_frameGroup[m_currentFrame].m_layers > 1)
				{
					Uint32 spriteMask = thingType->getSprite(frameGroup, cx, cy, 1, m_direction, addonPattern, zPattern, animation);
					if(spriteMask != 0)
						renderer->drawSpriteMask(sprite, spriteMask, posXc, posYc, spriteScale, spriteScale, 0, 0, 32, 32, outfitColor);
					else
						renderer->drawSprite(sprite, posXc, posYc, spriteScale, spriteScale, 0, 0, 32, 32);
				}
				else
					renderer->drawSprite(sprite, posXc, posYc, spriteScale, spriteScale, 0, 0, 32, 32);
			}

			posXc -= spriteScale;
		}
		posYc -= spriteScale;
	}
}

void GUI_Outfit_View::render()
{
	Uint32 outfitColor = (g_outfitColors[3] << 24) | (g_outfitColors[2] << 16) | (g_outfitColors[1] << 8) | (g_outfitColors[0]);
	updateMovement();

	auto& renderer = g_engine.getRender();
	renderer->beginGameScene();
	renderer->fillRectangle(0, 0, OUTFITS_PREVIEW_RENDER_SIZE, OUTFITS_PREVIEW_RENDER_SIZE, 64, 64, 64, 255);

	if(m_mode == PreviewMode_Mount)
	{
		if(m_mount && m_showMount)
			renderCreature(m_mount, m_mountAnim, 0, 0, outfitColor);
	}
	else if(m_outfit && m_showOutfit)
	{
		renderCreature(m_outfit, m_outfitAnim, 0, 0, outfitColor);
		if(g_outfitAddons & 1)
			renderCreature(m_outfit, m_outfitAnim, 1, 0, outfitColor);
		if(g_outfitAddons & 2)
			renderCreature(m_outfit, m_outfitAnim, 2, 0, outfitColor);
	}

	renderer->endGameScene();
	renderer->drawGameScene(0, 0, OUTFITS_PREVIEW_RENDER_SIZE, OUTFITS_PREVIEW_RENDER_SIZE, m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2);
	renderPanel();
}

GUI_Outfit_Colors::GUI_Outfit_Colors(iRect boxRect, Uint32 internalID)
{
	setRect(boxRect);
	m_internalID = internalID;
}

void GUI_Outfit_Colors::onMouseMove(Sint32 x, Sint32 y, bool)
{
	iRect rect = iRect(m_tRect.x1 + OUTFITS_COLOR_GRID_X, m_tRect.y1, OUTFITS_COLOR_GRID_W, OUTFITS_COLOR_GRID_H);
	if(rect.isPointInside(x, y))
	{
		Sint32 xFactor = (x - m_tRect.x1 - OUTFITS_COLOR_GRID_X) / OUTFITS_COLOR_GRID_CELL;
		Sint32 yFactor = (y - m_tRect.y1) / OUTFITS_COLOR_GRID_CELL;
		Sint32 cFactor = yFactor * 19 + xFactor;
		if(!m_Pressed || m_hoverColor[1] == cFactor)
			m_hoverColor[0] = cFactor;
		else
			m_hoverColor[0] = -1;
		return;
	}

	if(!m_Pressed)
		m_hoverColor[0] = -1;
}

void GUI_Outfit_Colors::onLMouseDown(Sint32 x, Sint32 y)
{
	m_Pressed = true;

	iRect rect = iRect(m_tRect.x1 + OUTFITS_COLOR_GRID_X, m_tRect.y1, OUTFITS_COLOR_GRID_W, OUTFITS_COLOR_GRID_H);
	if(rect.isPointInside(x, y))
	{
		Sint32 xFactor = (x - m_tRect.x1 - OUTFITS_COLOR_GRID_X) / OUTFITS_COLOR_GRID_CELL;
		Sint32 yFactor = (y - m_tRect.y1) / OUTFITS_COLOR_GRID_CELL;
		Sint32 cFactor = yFactor * 19 + xFactor;
		m_hoverColor[0] = m_hoverColor[1] = cFactor;
		return;
	}

	for(Sint32 i = 0; i < 4; ++i)
	{
		rect = iRect(m_tRect.x1, m_tRect.y1 + (OUTFITS_COLOR_MODE_H + OUTFITS_COLOR_MODE_GAP) * i, OUTFITS_COLOR_MODE_W, OUTFITS_COLOR_MODE_H);
		if(rect.isPointInside(x, y))
		{
			m_selected = i;
			return;
		}
	}
}

void GUI_Outfit_Colors::onLMouseUp(Sint32 x, Sint32 y)
{
	if(!m_Pressed)
		return;
	m_Pressed = false;

	iRect rect = iRect(m_tRect.x1 + OUTFITS_COLOR_GRID_X, m_tRect.y1, OUTFITS_COLOR_GRID_W, OUTFITS_COLOR_GRID_H);
	if(rect.isPointInside(x, y))
	{
		Sint32 xFactor = (x - m_tRect.x1 - OUTFITS_COLOR_GRID_X) / OUTFITS_COLOR_GRID_CELL;
		Sint32 yFactor = (y - m_tRect.y1) / OUTFITS_COLOR_GRID_CELL;
		Sint32 cFactor = yFactor * 19 + xFactor;
		if(m_hoverColor[0] == cFactor)
			g_outfitColors[m_selected] = SDL_static_cast(Uint8, cFactor);
	}
	m_hoverColor[0] = m_hoverColor[1] = -1;
}

void GUI_Outfit_Colors::render()
{
	auto& renderer = g_engine.getRender();
	const char* colorModeNames[4] = {"Head", "Primary", "Secondary", "Detail"};
	for(Sint32 i = 0; i < 4; ++i)
	{
		const Sint32 posY = m_tRect.y1 + (OUTFITS_COLOR_MODE_H + OUTFITS_COLOR_MODE_GAP) * i;
		const bool selected = (m_selected == i);
		renderer->fillRectangle(m_tRect.x1 + 1, posY + 1, OUTFITS_COLOR_MODE_W - 2, OUTFITS_COLOR_MODE_H - 2, (selected ? 86 : 64), (selected ? 86 : 64), (selected ? 86 : 64), 255);
		renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_DARK_X, GUI_UI_ICON_HORIZONTAL_LINE_DARK_Y, GUI_UI_ICON_HORIZONTAL_LINE_DARK_W, GUI_UI_ICON_HORIZONTAL_LINE_DARK_H, m_tRect.x1, posY, OUTFITS_COLOR_MODE_W, 1);
		renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_DARK_X, GUI_UI_ICON_VERTICAL_LINE_DARK_Y, GUI_UI_ICON_VERTICAL_LINE_DARK_W, GUI_UI_ICON_VERTICAL_LINE_DARK_H, m_tRect.x1, posY + 1, 1, OUTFITS_COLOR_MODE_H - 1);
		renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_X, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_Y, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_W, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_H, m_tRect.x1 + 1, posY + OUTFITS_COLOR_MODE_H - 1, OUTFITS_COLOR_MODE_W - 1, 1);
		renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_X, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_Y, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_W, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_H, m_tRect.x1 + OUTFITS_COLOR_MODE_W - 1, posY + 1, 1, OUTFITS_COLOR_MODE_H - 2);
		g_engine.drawFont(CLIENT_FONT_SMALL, m_tRect.x1 + (OUTFITS_COLOR_MODE_W / 2), posY + 6, colorModeNames[i], 255, 255, 255, CLIENT_FONT_ALIGN_CENTER);
	}

	Uint8 hoverC = SDL_static_cast(Uint8, m_hoverColor[0]);
	Uint8 c = 0;
	Sint32 posY = m_tRect.y1;
	for(Sint32 i = 0; i < 7; ++i)
	{
		Sint32 posX = m_tRect.x1 + OUTFITS_COLOR_GRID_X;
		for(Sint32 j = 0; j < 19; ++j)
		{
			Uint8 red, green, blue;
			getOutfitColorRGB(c, red, green, blue);
			renderer->fillRectangle(posX + 2, posY + 2, 8, 8, red, green, blue, 255);
			if(g_outfitColors[m_selected] == c)
			{
				renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_COLOR_BOX_DOWN_X, GUI_UI_ICON_COLOR_BOX_DOWN_Y, posX, posY, GUI_UI_ICON_COLOR_BOX_DOWN_W, GUI_UI_ICON_COLOR_BOX_DOWN_H);
				renderer->drawRectangle(posX - 1, posY - 1, 14, 14, 1, 255, 255, 255, 255);
			}
			else
				renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_COLOR_BOX_UP_X, (hoverC == c ? GUI_UI_ICON_COLOR_BOX_DOWN_Y : GUI_UI_ICON_COLOR_BOX_UP_Y), posX, posY, GUI_UI_ICON_COLOR_BOX_UP_W, GUI_UI_ICON_COLOR_BOX_UP_H);
			
			++c;
			posX += 13;
		}
		posY += 13;
	}
}

GUI_Outfit_HelpText::GUI_Outfit_HelpText(iRect boxRect, Uint32 internalID)
{
	setRect(boxRect);
	m_internalID = internalID;
}

void GUI_Outfit_HelpText::render()
{
	static const char* lines[] =
	{
		"Select an outfit by clicking on the arrows below the",
		"character box. Individualise the single parts by using",
		"the colour palette. If you are premium and have",
		"earned an outfit addon, you can activate it by",
		"checking the corresponding box. Mounts earned in",
		"the game can be selected from the right-hand box."
	};

	for(size_t i = 0; i < SDL_arraysize(lines); ++i)
		g_engine.drawFont(CLIENT_FONT_NONOUTLINED, m_tRect.x1, m_tRect.y1 + SDL_static_cast(Sint32, i * 13), lines[i], 210, 210, 210, CLIENT_FONT_ALIGN_LEFT);
}
