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
#include "../GUI_Elements/GUI_ScrollBar.h"
#include "../GUI_Elements/GUI_CheckBox.h"
#include "../GUI_Elements/GUI_ListBox.h"
#include "../GUI_Elements/GUI_Label.h"

#define GRAPHICS_OPTIONS_TITLE "Advanced Graphics Options"
#define GRAPHICS_OPTIONS_WIDTH 266
#define GRAPHICS_OPTIONS_HEIGHT 486
#define GRAPHICS_OPTIONS_CANCEL_EVENTID 1000
#define GRAPHICS_OPTIONS_OK_EVENTID 1001
#define GRAPHICS_OPTIONS_HELP_LINK "http://www.tibia.com/gameguides/?subtopic=manual&section=options"
#define GRAPHICS_OPTIONS_HELP_EVENTID 1002
#define GRAPHICS_OPTIONS_LISTBOX_ENGINES_X 18
#define GRAPHICS_OPTIONS_LISTBOX_ENGINES_Y 47
#define GRAPHICS_OPTIONS_LISTBOX_ENGINES_W 230
#define GRAPHICS_OPTIONS_LISTBOX_ENGINES_H 55
#define GRAPHICS_OPTIONS_LISTBOX_ENGINES_EVENTID 1003
#define GRAPHICS_OPTIONS_LISTBOX_ANTIALIAS_X 18
#define GRAPHICS_OPTIONS_LISTBOX_ANTIALIAS_Y 284
#define GRAPHICS_OPTIONS_LISTBOX_ANTIALIAS_W 230
#define GRAPHICS_OPTIONS_LISTBOX_ANTIALIAS_H 43
#define GRAPHICS_OPTIONS_LISTBOX_ANTIALIAS_EVENTID 1004
#define GRAPHICS_OPTIONS_LISTBOX_LIGHTS_X 18
#define GRAPHICS_OPTIONS_LISTBOX_LIGHTS_Y 213
#define GRAPHICS_OPTIONS_LISTBOX_LIGHTS_W 230
#define GRAPHICS_OPTIONS_LISTBOX_LIGHTS_H 43
#define GRAPHICS_OPTIONS_LISTBOX_LIGHTS_EVENTID 1005
#define GRAPHICS_OPTIONS_SCROLLBAR_LIGHTS_X 18
#define GRAPHICS_OPTIONS_SCROLLBAR_LIGHTS_Y 133
#define GRAPHICS_OPTIONS_SCROLLBAR_LIGHTS_W 230
#define GRAPHICS_OPTIONS_SCROLLBAR_LIGHTS_H 12
#define GRAPHICS_OPTIONS_SCROLLBAR_LIGHTS_EVENTID 1006
#define GRAPHICS_OPTIONS_SCROLLBAR_LEVELSEPARATOR_X 18
#define GRAPHICS_OPTIONS_SCROLLBAR_LEVELSEPARATOR_Y 173
#define GRAPHICS_OPTIONS_SCROLLBAR_LEVELSEPARATOR_W 230
#define GRAPHICS_OPTIONS_SCROLLBAR_LEVELSEPARATOR_H 12
#define GRAPHICS_OPTIONS_SCROLLBAR_LEVELSEPARATOR_EVENTID 1007
#define GRAPHICS_OPTIONS_SCROLLBAR_FPS_X 18
#define GRAPHICS_OPTIONS_SCROLLBAR_FPS_Y 413
#define GRAPHICS_OPTIONS_SCROLLBAR_FPS_W 230
#define GRAPHICS_OPTIONS_SCROLLBAR_FPS_H 12
#define GRAPHICS_OPTIONS_SCROLLBAR_FPS_EVENTID 1008
#define GRAPHICS_OPTIONS_CHECKBOX_SHARPEN_TITLE "Game-Screen Sharpening"
#define GRAPHICS_OPTIONS_CHECKBOX_SHARPEN_X 18
#define GRAPHICS_OPTIONS_CHECKBOX_SHARPEN_Y 335
#define GRAPHICS_OPTIONS_CHECKBOX_SHARPEN_W 230
#define GRAPHICS_OPTIONS_CHECKBOX_SHARPEN_H 22
#define GRAPHICS_OPTIONS_CHECKBOX_SHARPEN_EVENTID 1009
#define GRAPHICS_OPTIONS_CHECKBOX_VSYNC_TITLE "Vertical Synchronization"
#define GRAPHICS_OPTIONS_CHECKBOX_VSYNC_X 18
#define GRAPHICS_OPTIONS_CHECKBOX_VSYNC_Y 365
#define GRAPHICS_OPTIONS_CHECKBOX_VSYNC_W 230
#define GRAPHICS_OPTIONS_CHECKBOX_VSYNC_H 22
#define GRAPHICS_OPTIONS_CHECKBOX_VSYNC_EVENTID 1010
#define GRAPHICS_OPTIONS_LABEL_ENGINES_TITLE "Select Graphics Engine:"
#define GRAPHICS_OPTIONS_LABEL_ENGINES_X 18
#define GRAPHICS_OPTIONS_LABEL_ENGINES_Y 34
#define GRAPHICS_OPTIONS_LABEL_ANTIALIAS_TITLE "Select Antialiasing Mode:"
#define GRAPHICS_OPTIONS_LABEL_ANTIALIAS_X 18
#define GRAPHICS_OPTIONS_LABEL_ANTIALIAS_Y 269
#define GRAPHICS_OPTIONS_LABEL_LIGHTS_TITLE "Select Light Mode:"
#define GRAPHICS_OPTIONS_LABEL_LIGHTS_X 18
#define GRAPHICS_OPTIONS_LABEL_LIGHTS_Y 198
#define GRAPHICS_OPTIONS_LABEL_CTRL_LIGHTS_X 18
#define GRAPHICS_OPTIONS_LABEL_CTRL_LIGHTS_Y 118
#define GRAPHICS_OPTIONS_LABEL_CTRL_LIGHTS_EVENTID 1011
#define GRAPHICS_OPTIONS_LABEL_CTRL_LEVELSEPARATOR_X 18
#define GRAPHICS_OPTIONS_LABEL_CTRL_LEVELSEPARATOR_Y 158
#define GRAPHICS_OPTIONS_LABEL_CTRL_LEVELSEPARATOR_EVENTID 1012
#define GRAPHICS_OPTIONS_LABEL_CTRL_FPS_X 18
#define GRAPHICS_OPTIONS_LABEL_CTRL_FPS_Y 398
#define GRAPHICS_OPTIONS_LABEL_CTRL_FPS_EVENTID 1013
#define GRAPHICS_OPTIONS_LIGHTS_TEXT "Set Ambient Light: %d%%"
#define GRAPHICS_OPTIONS_LEVELSEPARATOR_TEXT "Set Level Separator: %d%%"
#define GRAPHICS_OPTIONS_FPS_TEXT "Adjust Framerate Limit: %d"
#define GRAPHICS_OPTIONS_FPS_MAX_TEXT "Adjust Framerate Limit: max"

extern Engine g_engine;
extern bool g_inited;

void advanced_graphics_options_Events(Uint32 event, Sint32 status)
{
	switch(event)
	{
		case GRAPHICS_OPTIONS_HELP_EVENTID: UTIL_OpenURL(GRAPHICS_OPTIONS_HELP_LINK); break;
		case GRAPHICS_OPTIONS_CANCEL_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_ADVANCEDGRAPHICSOPTIONS)
				g_engine.removeWindow(pWindow);
		}
		break;
		case GRAPHICS_OPTIONS_OK_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_ADVANCEDGRAPHICSOPTIONS)
			{
				GUI_HScrollBar* pScrollBar = SDL_static_cast(GUI_HScrollBar*, pWindow->getChild(GRAPHICS_OPTIONS_SCROLLBAR_LIGHTS_EVENTID));
				if(pScrollBar)
					g_engine.setAmbientLight(UTIL_min<Uint8>(100, SDL_static_cast(Uint8, pScrollBar->getScrollPos())));

				pScrollBar = SDL_static_cast(GUI_HScrollBar*, pWindow->getChild(GRAPHICS_OPTIONS_SCROLLBAR_FPS_EVENTID));
				if(pScrollBar)
				{
					status = pScrollBar->getScrollPos();
					if(status == 171)
						g_engine.setUnlimitedFPS(true);
					else
					{
						g_engine.setUnlimitedFPS(false);
						SDL_setFramerate(&g_fpsmanager, status + 30);
					}
				}

				pScrollBar = SDL_static_cast(GUI_HScrollBar*, pWindow->getChild(GRAPHICS_OPTIONS_SCROLLBAR_LEVELSEPARATOR_EVENTID));
				if(pScrollBar)
					g_engine.setLevelSeparator(UTIL_min<Uint8>(100, SDL_static_cast(Uint8, pScrollBar->getScrollPos())));
				
				GUI_CheckBox* pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(GRAPHICS_OPTIONS_CHECKBOX_SHARPEN_EVENTID));
				if(pCheckBox)
					g_engine.setSharpening(pCheckBox->isChecked());

				pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(GRAPHICS_OPTIONS_CHECKBOX_VSYNC_EVENTID));
				if(pCheckBox)
				{
					if(pCheckBox->isChecked() != g_engine.isVsync())
					{
						g_engine.setVsync(pCheckBox->isChecked());
						g_inited = false;//ReInit engine so renderer will reset Vsync
					}
				}

				GUI_ListBox* pListBox = SDL_static_cast(GUI_ListBox*, pWindow->getChild(GRAPHICS_OPTIONS_LISTBOX_ENGINES_EVENTID));
				if(pListBox)
				{
					Sint32 engineCounter = 0;
					Uint8 engineId = g_engine.getEngineId();
					Sint32 selectedAPI = pListBox->getSelect();
					if(selectedAPI == engineCounter++ && engineId != CLIENT_ENGINE_SOFTWARE)
					{
						g_engine.setEngineId(CLIENT_ENGINE_SOFTWARE);
						g_inited = false;
					}
					#if defined(SDL_VIDEO_RENDER_OGL)
					else if(selectedAPI == engineCounter++ && engineId != CLIENT_ENGINE_OPENGL)
					{
						g_engine.setEngineId(CLIENT_ENGINE_OPENGL);
						g_inited = false;
					}
					else if(selectedAPI == engineCounter++ && engineId != CLIENT_ENGINE_OPENGLCORE)
					{
						g_engine.setEngineId(CLIENT_ENGINE_OPENGLCORE);
						g_inited = false;
					}
					#else
					#if defined(SDL_VIDEO_RENDER_OGL_ES)
					else if(selectedAPI == engineCounter++ && engineId != CLIENT_ENGINE_OPENGLES)
					{
						g_engine.setEngineId(CLIENT_ENGINE_OPENGLES);
						g_inited = false;
					}
					#endif
					#if defined(SDL_VIDEO_RENDER_OGL_ES2)
					else if(selectedAPI == engineCounter++ && engineId != CLIENT_ENGINE_OPENGLES2)
					{
						g_engine.setEngineId(CLIENT_ENGINE_OPENGLES2);
						g_inited = false;
					}
					#endif
					#endif
					#if defined(SDL_VIDEO_RENDER_DDRAW)
					else if(selectedAPI == engineCounter++ && engineId != CLIENT_ENGINE_DIRECT3D7)
					{
						g_engine.setEngineId(CLIENT_ENGINE_DIRECT3D7);
						g_inited = false;
					}
					#endif
					#if defined(SDL_VIDEO_RENDER_D3D)
					else if(selectedAPI == engineCounter++ && engineId != CLIENT_ENGINE_DIRECT3D)
					{
						g_engine.setEngineId(CLIENT_ENGINE_DIRECT3D);
						g_inited = false;
					}
					#endif
					#if defined(SDL_VIDEO_RENDER_D3D11)
					else if(selectedAPI == engineCounter++ && engineId != CLIENT_ENGINE_DIRECT3D11)
					{
						g_engine.setEngineId(CLIENT_ENGINE_DIRECT3D11);
						g_inited = false;
					}
					#endif
					#if defined(SDL_VIDEO_RENDER_METAL)
					else if(selectedAPI == engineCounter++ && engineId != CLIENT_ENGINE_METAL)
					{
						g_engine.setEngineId(CLIENT_ENGINE_METAL);
						g_inited = false;
					}
					#endif
					#if defined(SDL_VIDEO_VULKAN)
					else if(selectedAPI == engineCounter++ && engineId != CLIENT_ENGINE_VULKAN)
					{
						g_engine.setEngineId(CLIENT_ENGINE_VULKAN);
						g_inited = false;
					}
					#endif
				}

				pListBox = SDL_static_cast(GUI_ListBox*, pWindow->getChild(GRAPHICS_OPTIONS_LISTBOX_ANTIALIAS_EVENTID));
				if(pListBox)
				{
					Sint32 selectedAntialiasing = pListBox->getSelect();
					Uint8 currentAntialiasing = g_engine.getAntialiasing();
					if(selectedAntialiasing == 1)
						g_engine.setAntialiasing(CLIENT_ANTIALIASING_NORMAL);
					else if(selectedAntialiasing == 2)
						g_engine.setAntialiasing(CLIENT_ANTIALIASING_INTEGER);
					else
						g_engine.setAntialiasing(CLIENT_ANTIALIASING_NONE);

					if(currentAntialiasing != g_engine.getAntialiasing())
						g_inited = false;//ReInit engine so renderer will reset
				}

				pListBox = SDL_static_cast(GUI_ListBox*, pWindow->getChild(GRAPHICS_OPTIONS_LISTBOX_LIGHTS_EVENTID));
				if(pListBox)
				{
					Sint32 selectedLightMode = pListBox->getSelect();
					if(selectedLightMode == 2)
						g_engine.setLightMode(CLIENT_LIGHT_MODE_NEW);
					else if(selectedLightMode == 1)
						g_engine.setLightMode(CLIENT_LIGHT_MODE_OLD);
					else
						g_engine.setLightMode(CLIENT_LIGHT_MODE_NONE);
				}
				g_engine.removeWindow(pWindow);
			}
		}
		break;
		case GRAPHICS_OPTIONS_SCROLLBAR_LIGHTS_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_ADVANCEDGRAPHICSOPTIONS)
			{
				GUI_Label* pLabel = SDL_static_cast(GUI_Label*, pWindow->getChild(GRAPHICS_OPTIONS_LABEL_CTRL_LIGHTS_EVENTID));
				if(pLabel)
				{
					SDL_snprintf(g_buffer, sizeof(g_buffer), GRAPHICS_OPTIONS_LIGHTS_TEXT, status);
					pLabel->setName(g_buffer);
				}
			}
		}
		break;
		case GRAPHICS_OPTIONS_SCROLLBAR_LEVELSEPARATOR_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_ADVANCEDGRAPHICSOPTIONS)
			{
				GUI_Label* pLabel = SDL_static_cast(GUI_Label*, pWindow->getChild(GRAPHICS_OPTIONS_LABEL_CTRL_LEVELSEPARATOR_EVENTID));
				if(pLabel)
				{
					SDL_snprintf(g_buffer, sizeof(g_buffer), GRAPHICS_OPTIONS_LEVELSEPARATOR_TEXT, status);
					pLabel->setName(g_buffer);
				}
			}
		}
		break;
		case GRAPHICS_OPTIONS_SCROLLBAR_FPS_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_ADVANCEDGRAPHICSOPTIONS)
			{
				GUI_Label* pLabel = SDL_static_cast(GUI_Label*, pWindow->getChild(GRAPHICS_OPTIONS_LABEL_CTRL_FPS_EVENTID));
				if(pLabel)
				{
					if(status == 171)
						SDL_snprintf(g_buffer, sizeof(g_buffer), GRAPHICS_OPTIONS_FPS_MAX_TEXT);
					else
						SDL_snprintf(g_buffer, sizeof(g_buffer), GRAPHICS_OPTIONS_FPS_TEXT, status + 30);
					
					pLabel->setName(g_buffer);
				}
			}
		}
		break;
		default: break;
	}
}

Sint32 getSelectedAPI()
{
	Sint32 engineCounter = 1;
	Uint8 engineId = g_engine.getEngineId();
	#if defined(SDL_VIDEO_RENDER_OGL)
	if(engineId == CLIENT_ENGINE_OPENGL)
		return engineCounter;
	else
		++engineCounter;
	if(engineId == CLIENT_ENGINE_OPENGLCORE)
		return engineCounter;
	else
		++engineCounter;
	#else
	#if defined(SDL_VIDEO_RENDER_OGL_ES)
	if(engineId == CLIENT_ENGINE_OPENGLES)
		return engineCounter;
	else
		++engineCounter;
	#endif
	#if defined(SDL_VIDEO_RENDER_OGL_ES2)
	if(engineId == CLIENT_ENGINE_OPENGLES2)
		return engineCounter;
	else
		++engineCounter;
	#endif
	#endif
	#if defined(SDL_VIDEO_RENDER_DDRAW)
	if(engineId == CLIENT_ENGINE_DIRECT3D7)
		return engineCounter;
	else
		++engineCounter;
	#endif
	#if defined(SDL_VIDEO_RENDER_D3D)
	if(engineId == CLIENT_ENGINE_DIRECT3D)
		return engineCounter;
	else
		++engineCounter;
	#endif
	#if defined(SDL_VIDEO_RENDER_D3D11)
	if(engineId == CLIENT_ENGINE_DIRECT3D11)
		return engineCounter;
	else
		++engineCounter;
	#endif
	#if defined(SDL_VIDEO_RENDER_METAL)
	if(engineId == CLIENT_ENGINE_METAL)
		return engineCounter;
	else
		++engineCounter;
	#endif
	#if defined(SDL_VIDEO_VULKAN)
	if(engineId == CLIENT_ENGINE_VULKAN)
		return engineCounter;
	else
		++engineCounter;
	#endif
	return 0;//Software
}

void UTIL_advancedGraphicsOptions()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_ADVANCEDGRAPHICSOPTIONS);
	if(pWindow)
		g_engine.removeWindow(pWindow);

	GUI_Window* newWindow = new GUI_Window(iRect(0, 0, GRAPHICS_OPTIONS_WIDTH, GRAPHICS_OPTIONS_HEIGHT), GRAPHICS_OPTIONS_TITLE, GUI_WINDOW_ADVANCEDGRAPHICSOPTIONS);
	GUI_Button* newButton = new GUI_Button(iRect(GRAPHICS_OPTIONS_WIDTH - 56, GRAPHICS_OPTIONS_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Cancel", CLIENT_GUI_ESCAPE_TRIGGER);
	newButton->setButtonEventCallback(&advanced_graphics_options_Events, GRAPHICS_OPTIONS_CANCEL_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(GRAPHICS_OPTIONS_WIDTH - 109, GRAPHICS_OPTIONS_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Ok", CLIENT_GUI_ENTER_TRIGGER);
	newButton->setButtonEventCallback(&advanced_graphics_options_Events, GRAPHICS_OPTIONS_OK_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(GRAPHICS_OPTIONS_WIDTH - 162, GRAPHICS_OPTIONS_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Help");
	newButton->setButtonEventCallback(&advanced_graphics_options_Events, GRAPHICS_OPTIONS_HELP_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	GUI_ListBox* newListBox = new GUI_ListBox(iRect(GRAPHICS_OPTIONS_LISTBOX_ENGINES_X, GRAPHICS_OPTIONS_LISTBOX_ENGINES_Y, GRAPHICS_OPTIONS_LISTBOX_ENGINES_W, GRAPHICS_OPTIONS_LISTBOX_ENGINES_H), GRAPHICS_OPTIONS_LISTBOX_ENGINES_EVENTID);
	newListBox->add("Software");
	#if defined(SDL_VIDEO_RENDER_OGL)
	newListBox->add("OpenGL Legacy");
	newListBox->add("OpenGL Core");
	#else
	#if defined(SDL_VIDEO_RENDER_OGL_ES)
	newListBox->add("OpenGL ES");
	#endif
	#if defined(SDL_VIDEO_RENDER_OGL_ES2)
	newListBox->add("OpenGL ES2");
	#endif
	#endif
	#if defined(SDL_VIDEO_RENDER_DDRAW)
	newListBox->add("DirectDraw");
	#endif
	#if defined(SDL_VIDEO_RENDER_D3D)
	newListBox->add("Direct3D9");
	#endif
	#if defined(SDL_VIDEO_RENDER_D3D11)
	newListBox->add("Direct3D11");
	#endif
	#if defined(SDL_VIDEO_VULKAN)
	newListBox->add("Vulkan");
	#endif
	newListBox->setSelect(getSelectedAPI());
	newListBox->startEvents(); 
	newWindow->addChild(newListBox);
	GUI_Separator* newSeparator = new GUI_Separator(iRect(13, GRAPHICS_OPTIONS_HEIGHT - 40, GRAPHICS_OPTIONS_WIDTH - 26, 2));
	newWindow->addChild(newSeparator);
	GUI_Label* newLabel = new GUI_Label(iRect(GRAPHICS_OPTIONS_LABEL_ENGINES_X, GRAPHICS_OPTIONS_LABEL_ENGINES_Y, 0, 0), GRAPHICS_OPTIONS_LABEL_ENGINES_TITLE);
	newWindow->addChild(newLabel);
	SDL_snprintf(g_buffer, sizeof(g_buffer), GRAPHICS_OPTIONS_LIGHTS_TEXT, g_engine.getAmbientLight());
	newLabel = new GUI_Label(iRect(GRAPHICS_OPTIONS_LABEL_CTRL_LIGHTS_X, GRAPHICS_OPTIONS_LABEL_CTRL_LIGHTS_Y, 0, 0), g_buffer, GRAPHICS_OPTIONS_LABEL_CTRL_LIGHTS_EVENTID);
	newWindow->addChild(newLabel);
	if(g_engine.isUnlimitedFPS())
		SDL_snprintf(g_buffer, sizeof(g_buffer), GRAPHICS_OPTIONS_FPS_MAX_TEXT);
	else
		SDL_snprintf(g_buffer, sizeof(g_buffer), GRAPHICS_OPTIONS_FPS_TEXT, g_fpsmanager.rate);

	newLabel = new GUI_Label(iRect(GRAPHICS_OPTIONS_LABEL_CTRL_FPS_X, GRAPHICS_OPTIONS_LABEL_CTRL_FPS_Y, 0, 0), g_buffer, GRAPHICS_OPTIONS_LABEL_CTRL_FPS_EVENTID);
	newWindow->addChild(newLabel);
	SDL_snprintf(g_buffer, sizeof(g_buffer), GRAPHICS_OPTIONS_LEVELSEPARATOR_TEXT, g_engine.getLevelSeparator());
	newLabel = new GUI_Label(iRect(GRAPHICS_OPTIONS_LABEL_CTRL_LEVELSEPARATOR_X, GRAPHICS_OPTIONS_LABEL_CTRL_LEVELSEPARATOR_Y, 0, 0), g_buffer, GRAPHICS_OPTIONS_LABEL_CTRL_LEVELSEPARATOR_EVENTID);
	newWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(GRAPHICS_OPTIONS_LABEL_ANTIALIAS_X, GRAPHICS_OPTIONS_LABEL_ANTIALIAS_Y, 0, 0), GRAPHICS_OPTIONS_LABEL_ANTIALIAS_TITLE);
	newWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(GRAPHICS_OPTIONS_LABEL_LIGHTS_X, GRAPHICS_OPTIONS_LABEL_LIGHTS_Y, 0, 0), GRAPHICS_OPTIONS_LABEL_LIGHTS_TITLE);
	newWindow->addChild(newLabel);
	newListBox = new GUI_ListBox(iRect(GRAPHICS_OPTIONS_LISTBOX_ANTIALIAS_X, GRAPHICS_OPTIONS_LISTBOX_ANTIALIAS_Y, GRAPHICS_OPTIONS_LISTBOX_ANTIALIAS_W, GRAPHICS_OPTIONS_LISTBOX_ANTIALIAS_H), GRAPHICS_OPTIONS_LISTBOX_ANTIALIAS_EVENTID);
	newListBox->add("None");
	newListBox->add("Antialiasing");
	newListBox->add("Smooth Retro");
	newListBox->startEvents();
	if(g_engine.getAntialiasing() == CLIENT_ANTIALIASING_INTEGER)
		newListBox->setSelect(2);
	else if(g_engine.getAntialiasing() == CLIENT_ANTIALIASING_NORMAL)
		newListBox->setSelect(1);
	else
		newListBox->setSelect(0);

	newWindow->addChild(newListBox);
	newListBox = new GUI_ListBox(iRect(GRAPHICS_OPTIONS_LISTBOX_LIGHTS_X, GRAPHICS_OPTIONS_LISTBOX_LIGHTS_Y, GRAPHICS_OPTIONS_LISTBOX_LIGHTS_W, GRAPHICS_OPTIONS_LISTBOX_LIGHTS_H), GRAPHICS_OPTIONS_LISTBOX_LIGHTS_EVENTID);
	newListBox->add("No Light");
	newListBox->add("Old Tibia Light");
	newListBox->add("New Tibia Light (Experimental)");
	newListBox->startEvents();
	if(g_engine.getLightMode() == CLIENT_LIGHT_MODE_NEW)
		newListBox->setSelect(2);
	else if(g_engine.getLightMode() == CLIENT_LIGHT_MODE_OLD)
		newListBox->setSelect(1);
	else
		newListBox->setSelect(0);
	
	newWindow->addChild(newListBox);
	GUI_CheckBox* newCheckBox = new GUI_CheckBox(iRect(GRAPHICS_OPTIONS_CHECKBOX_SHARPEN_X, GRAPHICS_OPTIONS_CHECKBOX_SHARPEN_Y, GRAPHICS_OPTIONS_CHECKBOX_SHARPEN_W, GRAPHICS_OPTIONS_CHECKBOX_SHARPEN_H), GRAPHICS_OPTIONS_CHECKBOX_SHARPEN_TITLE, g_engine.isSharpening(), GRAPHICS_OPTIONS_CHECKBOX_SHARPEN_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	newCheckBox = new GUI_CheckBox(iRect(GRAPHICS_OPTIONS_CHECKBOX_VSYNC_X, GRAPHICS_OPTIONS_CHECKBOX_VSYNC_Y, GRAPHICS_OPTIONS_CHECKBOX_VSYNC_W, GRAPHICS_OPTIONS_CHECKBOX_VSYNC_H), GRAPHICS_OPTIONS_CHECKBOX_VSYNC_TITLE, g_engine.isVsync(), GRAPHICS_OPTIONS_CHECKBOX_VSYNC_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	GUI_HScrollBar* newHScrollBar = new GUI_HScrollBar(iRect(GRAPHICS_OPTIONS_SCROLLBAR_LIGHTS_X, GRAPHICS_OPTIONS_SCROLLBAR_LIGHTS_Y, GRAPHICS_OPTIONS_SCROLLBAR_LIGHTS_W, GRAPHICS_OPTIONS_SCROLLBAR_LIGHTS_H), 100, g_engine.getAmbientLight(), GRAPHICS_OPTIONS_SCROLLBAR_LIGHTS_EVENTID);
	newHScrollBar->setBarEventCallback(&advanced_graphics_options_Events, GRAPHICS_OPTIONS_SCROLLBAR_LIGHTS_EVENTID);
	newHScrollBar->startEvents();
	newWindow->addChild(newHScrollBar);
	newHScrollBar = new GUI_HScrollBar(iRect(GRAPHICS_OPTIONS_SCROLLBAR_FPS_X, GRAPHICS_OPTIONS_SCROLLBAR_FPS_Y, GRAPHICS_OPTIONS_SCROLLBAR_FPS_W, GRAPHICS_OPTIONS_SCROLLBAR_FPS_H), 171, (g_engine.isUnlimitedFPS() ? 171 : g_fpsmanager.rate - 30), GRAPHICS_OPTIONS_SCROLLBAR_FPS_EVENTID);
	newHScrollBar->setBarEventCallback(&advanced_graphics_options_Events, GRAPHICS_OPTIONS_SCROLLBAR_FPS_EVENTID);
	newHScrollBar->startEvents();
	newWindow->addChild(newHScrollBar);
	newHScrollBar = new GUI_HScrollBar(iRect(GRAPHICS_OPTIONS_SCROLLBAR_LEVELSEPARATOR_X, GRAPHICS_OPTIONS_SCROLLBAR_LEVELSEPARATOR_Y, GRAPHICS_OPTIONS_SCROLLBAR_LEVELSEPARATOR_W, GRAPHICS_OPTIONS_SCROLLBAR_LEVELSEPARATOR_H), 100, g_engine.getLevelSeparator(), GRAPHICS_OPTIONS_SCROLLBAR_LEVELSEPARATOR_EVENTID);
	newHScrollBar->setBarEventCallback(&advanced_graphics_options_Events, GRAPHICS_OPTIONS_SCROLLBAR_LEVELSEPARATOR_EVENTID);
	newHScrollBar->startEvents();
	newWindow->addChild(newHScrollBar);
	g_engine.addWindow(newWindow, true);
}
