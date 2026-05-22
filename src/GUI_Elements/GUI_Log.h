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

#ifndef __FILE_GUI_LOG_h_
#define __FILE_GUI_LOG_h_

#include "../defines.h"

enum
{
	LOG_CATEGORY_INFO,
	LOG_CATEGORY_WARNING,
	LOG_CATEGORY_ERROR
};

class GUI_Console;
class GUI_Log
{
	public:
		GUI_Log();
		~GUI_Log();

		// non-copyable
		GUI_Log(const GUI_Log&) = delete;
		GUI_Log& operator=(const GUI_Log&) = delete;

		// non-moveable
		GUI_Log(GUI_Log&&) = delete;
		GUI_Log& operator=(GUI_Log&&) = delete;

		void addLog(Sint32 category, const std::string& text);

		void onTextInput(const char* textInput);
		void onKeyDown(SDL_Event& event);
		void onKeyUp(SDL_Event& event);
		void onLMouseDown(Sint32 x, Sint32 y);
		void onLMouseUp(Sint32 x, Sint32 y);
		void onRMouseDown(Sint32 x, Sint32 y);
		void onRMouseUp(Sint32 x, Sint32 y);
		void onWheel(Sint32 x, Sint32 y, bool wheelUP);
		void onMouseMove(Sint32 x, Sint32 y, bool isInsideParent);
		void render(Sint32 x, Sint32 y, Sint32 w, Sint32 h);

	protected:
		std::unique_ptr<GUI_Console> m_console;
};

#endif /* __FILE_GUI_LOG_h_ */
