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

#ifndef __FILE_GUI_PANELWINDOW_h_
#define __FILE_GUI_PANELWINDOW_h_

#include "GUI_Element.h"

enum GUIPanelWindowID
{
	GUI_PANEL_WINDOW_UNKNOWN,
	GUI_PANEL_WINDOW_INVENTORY, // Done
	GUI_PANEL_WINDOW_INVENTORY_MINIMIZED, // Done
	GUI_PANEL_WINDOW_MINIMAP, // Done
	GUI_PANEL_WINDOW_HEALTH, // Done
	GUI_PANEL_WINDOW_BUTTONS, // Partially done - need to add options settings
	GUI_PANEL_WINDOW_SKILLS, // Done
	GUI_PANEL_WINDOW_BATTLE, // Done - need to add configuration
	GUI_PANEL_WINDOW_TRADE, // Done
	GUI_PANEL_WINDOW_VIP, // Done
	GUI_PANEL_WINDOW_SHOP, // Done
	GUI_PANEL_WINDOW_SPELL_LIST, // TODO
	GUI_PANEL_WINDOW_PREMIUM_FEATURES, // TODO
	GUI_PANEL_WINDOW_UNJUSTIFIED_POINTS, // TODO
	GUI_PANEL_WINDOW_PREY_WIDGET, // TODO
	GUI_PANEL_WINDOW_PARTY, // Done - need to add configuration
	GUI_PANEL_WINDOW_TOURNAMENT_WIDGET, // TODO
	GUI_PANEL_WINDOW_ANALYTICS_SELECTOR, // TODO
	GUI_PANEL_WINDOW_ANALYTICS_HUNTING, // TODO
	GUI_PANEL_WINDOW_ANALYTICS_LOOT, // TODO
	GUI_PANEL_WINDOW_ANALYTICS_SUPPLY, // TODO
	GUI_PANEL_WINDOW_ANALYTICS_IMPACT, // TODO
	GUI_PANEL_WINDOW_ANALYTICS_XP, // TODO
	GUI_PANEL_WINDOW_ANALYTICS_DROP, // TODO
	GUI_PANEL_WINDOW_ANALYTICS_QUEST, // TODO
	GUI_PANEL_WINDOW_DEPOT_SEARCH, // TODO
	GUI_PANEL_WINDOW_BOT, // Native bot

	//Reserved ids for containers
	GUI_PANEL_WINDOW_CONTAINERS_START = 64, // Done
	GUI_PANEL_WINDOW_CONTAINERS_END = 100 // Done
};

class GUI_Panel;
class GUI_PanelWindow
{
	public:
		GUI_PanelWindow(iRect boxRect, bool windowed, Uint32 internalID = 0, bool parentChangeable = false);
		~GUI_PanelWindow();

		// non-copyable
		GUI_PanelWindow(const GUI_PanelWindow&) = delete;
		GUI_PanelWindow& operator=(const GUI_PanelWindow&) = delete;

		// non-moveable
		GUI_PanelWindow(GUI_PanelWindow&&) = delete;
		GUI_PanelWindow& operator=(GUI_PanelWindow&&) = delete;

		void setEventCallback(void (*eventHandlerFunction)(Uint32, Sint32), Uint32 widthEvent, Uint32 heightEvent, Uint32 exitEvent);

		void setRect(iRect& NewRect, bool NewPos = false);
		void setSize(Sint32 width, Sint32 height);
		iRect& getOriginalRect() {return m_nRect;}
		iRect& getRect() {return m_tRect;}
		bool isInsideRect(Sint32 x, Sint32 y) {return m_tRect.isPointInside(x, y);}

		void checkPanels();
		void clearChilds();
		void addChild(GUI_Element* pChild);
		void removeChild(GUI_Element* pChild);
		GUI_Element* getChild(Uint32 internalID);

		GUI_Element* getActiveElement() {return m_actElement;}
		void setActiveElement(GUI_Element* actElement);

		GUI_Panel* getParent() {return m_parent;}
		void setParent(GUI_Panel* parent) {m_parent = parent;}

		void setInternalID(Uint32 internalId) {m_internalID = internalId;}
		Uint32 getInternalID() {return m_internalID;}
		bool isParentChangeable() {return m_parentChangeable;}

		void setCachedHeight(Sint32 height) {m_cachedHeight = height;}
		Sint32 getCachedHeight() {return m_cachedHeight;}
		void setMinHeight(Sint32 height) {m_minHeight = height;}
		Sint32 getMinHeight() {return m_minHeight;}
		void setMaxHeight(Sint32 height) {m_maxHeight = height;}
		Sint32 getMaxHeight() {return m_maxHeight;}

		void* onAction(Sint32 x, Sint32 y);
		void onLMouseDown(Sint32 x, Sint32 y);
		void onLMouseUp(Sint32 x, Sint32 y);
		void onRMouseDown(Sint32 x, Sint32 y);
		void onRMouseUp(Sint32 x, Sint32 y);
		void onWheel(Sint32 x, Sint32 y, bool wheelUP);
		void onMouseMove(Sint32 x, Sint32 y, bool isInsideParent);

		void onMoveWindowed(Sint32 x, Sint32 y);
		void onMoveNormal(Sint32 x, Sint32 y);

		void render();

	protected:
		void (*m_eventHandlerFunction)(Uint32, Sint32) = NULL;
		std::vector<GUI_Element*> m_childs;
		GUI_Element* m_actElement = NULL;
		GUI_Panel* m_parent = NULL;
		iRect m_mouseEvent;
		iRect m_nRect;
		iRect m_tRect;
		Uint32 m_resizeWidth = 0;
		Uint32 m_resizeHeight = 0;
		Uint32 m_exitEvent = 0;
		Uint32 m_internalID;
		Sint32 m_cachedHeight;
		Sint32 m_minHeight = GAME_PANEL_MINIMUM_HEIGHT;
		Sint32 m_maxHeight = 0;
		bool m_bMouseDragging = false;
		bool m_bMouseResizing = false;
		bool m_windowed;
		bool m_parentChangeable;
};

#endif /* __FILE_GUI_PANELWINDOW_h_ */
