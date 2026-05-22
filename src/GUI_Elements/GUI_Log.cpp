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

#include "GUI_Log.h"
#include "GUI_Console.h"
#include "../engine.h"

GUI_Log g_logger;
extern Engine g_engine;

GUI_Log::GUI_Log() : m_console(std::make_unique<GUI_Console>(iRect(0, 0, 0, 0)))
{
}

GUI_Log::~GUI_Log() = default;

void GUI_Log::addLog(Sint32 category, const std::string& text)
{
	switch(category)
	{
		case LOG_CATEGORY_INFO: m_console->addMessage(0, time(NULL), "Information", 0, text, 255, 255, 255); break;
		case LOG_CATEGORY_WARNING: m_console->addMessage(0, time(NULL), "Warning", 0, text, 255, 255, 0); break;
		case LOG_CATEGORY_ERROR: m_console->addMessage(0, time(NULL), "Error", 0, text, 255, 0, 0); break;
		default: break;
	}
}

void GUI_Log::onTextInput(const char* textInput)
{
	m_console->onTextInput(textInput);
}

void GUI_Log::onKeyDown(SDL_Event& event)
{
	m_console->onKeyDown(event);
}

void GUI_Log::onKeyUp(SDL_Event& event)
{
	m_console->onKeyUp(event);
}

void GUI_Log::onLMouseDown(Sint32 x, Sint32 y)
{
	m_console->onLMouseDown(x, y);
}

void GUI_Log::onLMouseUp(Sint32 x, Sint32 y)
{
	m_console->onLMouseUp(x, y);
}

void GUI_Log::onRMouseDown(Sint32 x, Sint32 y)
{
	m_console->onRMouseDown(x, y);
}

void GUI_Log::onRMouseUp(Sint32 x, Sint32 y)
{
	m_console->onRMouseUp(x, y);
}

void GUI_Log::onWheel(Sint32 x, Sint32 y, bool wheelUP)
{
	m_console->onWheel(x, y, wheelUP);
}

void GUI_Log::onMouseMove(Sint32 x, Sint32 y, bool isInsideParent)
{
	m_console->onMouseMove(x, y, isInsideParent);
}

void GUI_Log::render(Sint32 x, Sint32 y, Sint32 w, Sint32 h)
{
	auto& renderer = g_engine.getRender();
	renderer->fillRectangle(x, y, w, h, 0, 0, 0, 178);
	renderer->fillRectangle(x, y, w, 22, 26, 26, 26, 230);
	g_engine.drawFont(CLIENT_FONT_NONOUTLINED, x + 6, y + 4, "Terminal - Client diagnostics (Ctrl+T)", 220, 220, 220, CLIENT_FONT_ALIGN_LEFT);

	iRect nRect = iRect(x + 4, y + 24, w - 8, h - 28);
	m_console->setRect(nRect);
	m_console->render();
}
