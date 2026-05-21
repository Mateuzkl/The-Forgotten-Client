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

#ifndef __FILE_GUI_TASKSWINDOW_h_
#define __FILE_GUI_TASKSWINDOW_h_

#include "../GUI_Elements/GUI_Element.h"

class GUI_TaskChecker : public GUI_Element
{
	public:
		GUI_TaskChecker(iRect boxRect, Uint32 internalID = 0);

		// non-copyable
		GUI_TaskChecker(const GUI_TaskChecker&) = delete;
		GUI_TaskChecker& operator=(const GUI_TaskChecker&) = delete;

		// non-moveable
		GUI_TaskChecker(GUI_TaskChecker&&) = delete;
		GUI_TaskChecker& operator=(GUI_TaskChecker&&) = delete;

		void render();
};

class GUI_TaskCreature : public GUI_Element
{
	public:
		GUI_TaskCreature(iRect boxRect, Uint16 taskId, Uint32 internalID = 0);

		// non-copyable
		GUI_TaskCreature(const GUI_TaskCreature&) = delete;
		GUI_TaskCreature& operator=(const GUI_TaskCreature&) = delete;

		// non-moveable
		GUI_TaskCreature(GUI_TaskCreature&&) = delete;
		GUI_TaskCreature& operator=(GUI_TaskCreature&&) = delete;

		void render();

	protected:
		Uint16 m_taskId;
};

#endif /* __FILE_GUI_TASKSWINDOW_h_ */
