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

#ifndef __FILE_GUI_OUTFITS_h_
#define __FILE_GUI_OUTFITS_h_

#include "../GUI_Elements/GUI_Element.h"
#include "../animator.h"

class ThingType;
class GUI_Outfit_View : public GUI_Element
{
	public:
		enum PreviewMode
		{
			PreviewMode_Outfit,
			PreviewMode_Mount
		};

		GUI_Outfit_View(iRect boxRect, Uint32 internalID = 0, PreviewMode mode = PreviewMode_Outfit);

		// non-copyable
		GUI_Outfit_View(const GUI_Outfit_View&) = delete;
		GUI_Outfit_View& operator=(const GUI_Outfit_View&) = delete;

		// non-moveable
		GUI_Outfit_View(GUI_Outfit_View&&) = delete;
		GUI_Outfit_View& operator=(GUI_Outfit_View&&) = delete;

		void previousDirection();
		void nextDirection();
		void refresh();

		void startMovement();
		void stopMovement();
		void resetAnimation();
		void updateMovement();

		Sint32 getOffsetX();
		Sint32 getOffsetY();

		void render();

	protected:
		void renderPanel();
		void renderCreature(ThingType* thingType, Uint8 animation, Uint8 addonPattern, Uint8 zPattern, Uint32 outfitColor);

		ThingType* m_outfit = nullptr;
		ThingType* m_mount = nullptr;
		Animation m_outfitAnimation[ThingFrameGroup_Last];
		Animation m_mountAnimation[ThingFrameGroup_Last];
		Uint32 m_walkStartTime = 0;
		Sint32 m_walkedPixels = 0;
		Uint8 m_direction = DIRECTION_SOUTH;
		Uint8 m_outfitAnim = 0;
		Uint8 m_mountAnim = 0;
		Uint8 m_currentFrame;
		bool m_showOutfit = true;
		bool m_showMount = true;
		bool m_walking = false;
		PreviewMode m_mode = PreviewMode_Outfit;
};

class GUI_Outfit_Colors : public GUI_Element
{
	public:
		GUI_Outfit_Colors(iRect boxRect, Uint32 internalID = 0);

		// non-copyable
		GUI_Outfit_Colors(const GUI_Outfit_Colors&) = delete;
		GUI_Outfit_Colors& operator=(const GUI_Outfit_Colors&) = delete;

		// non-moveable
		GUI_Outfit_Colors(GUI_Outfit_Colors&&) = delete;
		GUI_Outfit_Colors& operator=(GUI_Outfit_Colors&&) = delete;

		void onMouseMove(Sint32 x, Sint32 y, bool isInsideParent);
		void onLMouseDown(Sint32 x, Sint32 y);
		void onLMouseUp(Sint32 x, Sint32 y);

		void render();

	protected:
		Sint32 m_selected = 0;
		Sint32 m_hoverColor[2] = {-1, -1};
		bool m_Pressed = false;
};

class GUI_Outfit_HelpText : public GUI_Element
{
	public:
		GUI_Outfit_HelpText(iRect boxRect, Uint32 internalID = 0);

		// non-copyable
		GUI_Outfit_HelpText(const GUI_Outfit_HelpText&) = delete;
		GUI_Outfit_HelpText& operator=(const GUI_Outfit_HelpText&) = delete;

		// non-moveable
		GUI_Outfit_HelpText(GUI_Outfit_HelpText&&) = delete;
		GUI_Outfit_HelpText& operator=(GUI_Outfit_HelpText&&) = delete;

		void render();
};

#endif /* __FILE_GUI_OUTFITS_h_ */
