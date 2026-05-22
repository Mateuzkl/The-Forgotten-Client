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

#ifndef __FILE_GUI_WINDOW_h_
#define __FILE_GUI_WINDOW_h_

#include "GUI_Element.h"

enum GUIWindowID
{
	GUI_WINDOW_MAIN,
	GUI_WINDOW_ENTERGAME,
	GUI_WINDOW_PROTOCOLOPTIONS,
	GUI_WINDOW_OPTIONS,
	GUI_WINDOW_HELP,
	GUI_WINDOW_INFO,
	GUI_WINDOW_GENERALOPTIONS,
	GUI_WINDOW_GRAPHICSOPTIONS,
	GUI_WINDOW_ADVANCEDGRAPHICSOPTIONS,
	GUI_WINDOW_CONSOLEOPTIONS,
	GUI_WINDOW_HOTKEYS,
	GUI_WINDOW_MESSAGEBOX,
	GUI_WINDOW_MOTD,
	GUI_WINDOW_CHARACTERLIST,
	GUI_WINDOW_EXITWARNING,
	GUI_WINDOW_LOGOUT,
	GUI_WINDOW_DEATHNOTIFICATION,
	GUI_WINDOW_READWRITE,
	GUI_WINDOW_CHANNELS,
	GUI_WINDOW_OUTFITS,
	GUI_WINDOW_TUTORIALHINT,
	GUI_WINDOW_QUESTLOG,
	GUI_WINDOW_QUESTLINE,
	GUI_WINDOW_ITEMMOVE,
	GUI_WINDOW_INVITE,
	GUI_WINDOW_EXCLUDE,
	GUI_WINDOW_EDITMARK,
	GUI_WINDOW_ADDVIP,
	GUI_WINDOW_EDITVIP,
	GUI_WINDOW_ADDGROUP,
	GUI_WINDOW_EDITGROUP,
	GUI_WINDOW_MODALDIALOG,
	GUI_WINDOW_IGNORELIST,
	GUI_WINDOW_GAMESTORE,
	GUI_WINDOW_GAMESTORE_CONFIRM,
	GUI_WINDOW_SERVERBROWSER,
	GUI_WINDOW_TASKS
};

class GUI_Window
{
	public:
		GUI_Window(iRect boxRect, const std::string title, Uint32 internalID = 0);
		~GUI_Window();

		// non-copyable
		GUI_Window(const GUI_Window&) = delete;
		GUI_Window& operator=(const GUI_Window&) = delete;

		// non-moveable
		GUI_Window(GUI_Window&&) = delete;
		GUI_Window& operator=(GUI_Window&&) = delete;

		void setRect(iRect& NewRect);
		iRect& getRect() {return m_tRect;}
		bool isInsideRect(Sint32 x, Sint32 y) {return m_tRect.isPointInside(x, y);}

		void setSize(Sint32 width, Sint32 height) {m_tRect.x2 = width; m_tRect.y2 = height;}
		void clearChilds();
		void addChild(GUI_Element* pChild);
		void removeChild(GUI_Element* pChild);
		GUI_Element* getChild(Uint32 internalID);

		Sint32 getPositionX() {return m_tRect.x1;}
		Sint32 getPositionY() {return m_tRect.y1;}

		GUI_Element* getActiveElement() {return m_actElement;}
		void setActiveElement(GUI_Element* actElement);

		void onReshape(Sint32 w, Sint32 h);

		bool isDragging() {return m_bMouseDragging;}
		Uint32 getInternalID() {return m_internalID;}

		bool isActive() {return m_bActive;}
		void activate() {m_bActive = true;}
		void deActivate() {m_bActive = false;}

		void onTextInput(const char* textInput);
		void onKeyDown(SDL_Event& event);
		void onKeyUp(SDL_Event& event);
		void onLMouseDown(Sint32 x, Sint32 y);
		void onLMouseUp(Sint32 x, Sint32 y);
		void onRMouseDown(Sint32 x, Sint32 y);
		void onRMouseUp(Sint32 x, Sint32 y);
		void onWheel(Sint32 x, Sint32 y, bool wheelUP);
		void onMouseMove(Sint32 x, Sint32 y, bool isInsideParent);
		void onMove(Sint32 x, Sint32 y);

		void render();

	protected:
		std::vector<GUI_Element*> m_childs;
		std::string m_Title;
		GUI_Element* m_actElement = NULL;
		Uint32 m_internalID;
		iRect m_mouseEvent;
		iRect m_tRect;
		bool m_bMouseDragging = false;
		bool m_bActive = true;
};

#endif /* __FILE_GUI_WINDOW_h_ */
