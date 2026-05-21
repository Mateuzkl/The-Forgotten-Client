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

#ifndef __FILE_GUI_BUTTON_h_
#define __FILE_GUI_BUTTON_h_

#include "GUI_Element.h"

class GUI_Button : public GUI_Element
{
	public:
		GUI_Button(iRect boxRect, const std::string labelName, Uint32 internalID = 0, const std::string description = "");

		// non-copyable
		GUI_Button(const GUI_Button&) = delete;
		GUI_Button& operator=(const GUI_Button&) = delete;

		// non-moveable
		GUI_Button(GUI_Button&&) = delete;
		GUI_Button& operator=(GUI_Button&&) = delete;

		void cacheUI();
		void setButtonEventCallback(void (*eventHandlerFunction)(Uint32,Sint32), Uint32 mEvent);
		void setTextColor(Uint8 red, Uint8 green, Uint8 blue) {m_red = red; m_green = green; m_blue = blue;}

		void onMouseMove(Sint32 x, Sint32 y, bool isInsideParent);
		void onLMouseDown(Sint32 x, Sint32 y);
		void onLMouseUp(Sint32 x, Sint32 y);

		void render();

	protected:
		void (*m_eventHandlerFunction)(Uint32,Sint32) = NULL;
		std::string m_description;
		std::string m_label;
		Uint32 m_evtParam = 0;
		Uint32 m_startX;
		Sint32 m_sx[2] = {};
		Sint32 m_sy[2] = {};
		Uint8 m_red = 255;
		Uint8 m_green = 255;
		Uint8 m_blue = 255;
		Uint8 m_pressed = 0;
};

class GUI_RadioButton : public GUI_Button
{
	public:
		GUI_RadioButton(iRect boxRect, const std::string labelName, Uint32 internalID = 0, const std::string description = "") : GUI_Button(boxRect, labelName, internalID, description) {}

		// non-copyable
		GUI_RadioButton(const GUI_RadioButton&) = delete;
		GUI_RadioButton& operator=(const GUI_RadioButton&) = delete;

		// non-moveable
		GUI_RadioButton(GUI_RadioButton&&) = delete;
		GUI_RadioButton& operator=(GUI_RadioButton&&) = delete;

		void setRadioEventCallback(bool (*eventRadioChecked)(void), const std::string description = "");

		void onMouseMove(Sint32 x, Sint32 y, bool isInsideParent);

		void render();

	protected:
		bool (*m_eventRadioChecked)(void) = NULL;
		std::string m_radioDescription;
};

#endif /* __FILE_GUI_BUTTON_h_ */
