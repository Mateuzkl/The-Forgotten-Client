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

#ifndef __FILE_GUI_BOSSWINDOW_h_
#define __FILE_GUI_BOSSWINDOW_h_

#include "../GUI_Elements/GUI_Element.h"

class GUI_BossChecker : public GUI_Element
{
	public:
		GUI_BossChecker(iRect boxRect, Uint32 internalID = 0);

		// non-copyable
		GUI_BossChecker(const GUI_BossChecker&) = delete;
		GUI_BossChecker& operator=(const GUI_BossChecker&) = delete;

		// non-moveable
		GUI_BossChecker(GUI_BossChecker&&) = delete;
		GUI_BossChecker& operator=(GUI_BossChecker&&) = delete;

		void render();
};

class GUI_BossCreature : public GUI_Element
{
	public:
		GUI_BossCreature(iRect boxRect, const std::string& bossName, Uint32 internalID = 0);

		// non-copyable
		GUI_BossCreature(const GUI_BossCreature&) = delete;
		GUI_BossCreature& operator=(const GUI_BossCreature&) = delete;

		// non-moveable
		GUI_BossCreature(GUI_BossCreature&&) = delete;
		GUI_BossCreature& operator=(GUI_BossCreature&&) = delete;

		void render();

	protected:
		std::string m_bossName;
};

#endif /* __FILE_GUI_BOSSWINDOW_h_ */
