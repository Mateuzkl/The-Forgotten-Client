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

#include "GUI_Window.h"
#include "../engine.h"

extern Engine g_engine;

GUI_Window::GUI_Window(iRect boxRect, const std::string title, Uint32 internalID) :
	m_Title(std::move(title)), m_internalID(internalID)
{
	setRect(boxRect);
}

GUI_Window::~GUI_Window()
{
	clearChilds();
}

void GUI_Window::setRect(iRect& NewRect)
{
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
	{
		GUI_Element* child = (*it);
		iRect childRect = child->getRect();
		iRect newChildRect;

		newChildRect.x1 = NewRect.x1 + (childRect.x1 - m_tRect.x1);
		newChildRect.y1 = NewRect.y1 + (childRect.y1 - m_tRect.y1);
		newChildRect.x2 = childRect.x2;
		newChildRect.y2 = childRect.y2;
		child->setRect(newChildRect);
	}
	m_tRect = NewRect;
}

void GUI_Window::clearChilds()
{
	setActiveElement(NULL);
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
		delete (*it);
	m_childs.clear();
}

void GUI_Window::addChild(GUI_Element* pChild)
{
	m_childs.push_back(pChild);
	if(pChild->getInternalID() == CLIENT_GUI_TAB_START)
		setActiveElement(pChild);

	iRect newRect;
	newRect.x1 = m_tRect.x1 + pChild->getRect().x1;
	newRect.y1 = m_tRect.y1 + pChild->getRect().y1;
	newRect.x2 = pChild->getRect().x2;
	newRect.y2 = pChild->getRect().y2;
	pChild->setRect(newRect);
}

void GUI_Window::removeChild(GUI_Element* pChild)
{
	if(pChild == m_actElement)
		setActiveElement(NULL);

	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
	{
		if((*it) == pChild)
		{
			m_childs.erase(it);
			break;
		}
	}
	delete pChild;
}

GUI_Element* GUI_Window::getChild(Uint32 internalID)
{
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
	{
		if((*it)->getInternalID() == internalID)
			return (*it);
	}
	return NULL;
}

void GUI_Window::setActiveElement(GUI_Element* actElement)
{
	if(m_lockActive && actElement != m_actElement && actElement != NULL)
		return; // Focus is locked - ignore attempts to change it

	if(m_actElement == actElement)
		return;

	if(m_actElement)
		m_actElement->deActivate();

	m_actElement = actElement;
	if(m_actElement)
        m_actElement->activate();
}

void GUI_Window::onReshape(Sint32 w, Sint32 h)
{
	iRect newRect = m_tRect;
	switch(m_internalID)
	{
		case GUI_WINDOW_MAIN:
		{
			newRect.x1 = 60;
			newRect.y1 = h - 60 - m_tRect.y2;
		}
		break;
		default:
		{
			newRect.x1 = w / 2 - m_tRect.x2 / 2;
			newRect.y1 = h / 2 - m_tRect.y2 / 2;
		}
		break;
	}
	setRect(newRect);
}

void GUI_Window::onTextInput(const char* textInput)
{
	if(m_actElement)
		m_actElement->onTextInput(textInput);
}

void GUI_Window::onKeyDown(SDL_Event& event)
{
	if(event.key.keysym.mod == KMOD_NONE)
	{
		switch(event.key.keysym.sym)
		{
			case SDLK_ESCAPE:
			{
				GUI_Element* elementEnt = getChild(CLIENT_GUI_ESCAPE_TRIGGER);
				if(elementEnt)
				{
					iRect newRect = elementEnt->getRect();
					elementEnt->onLMouseDown(newRect.x1, newRect.y1);
					elementEnt->onLMouseUp(newRect.x1, newRect.y1);
					return;
				}
			}
			break;

			case SDLK_RETURN:
			case SDLK_KP_ENTER:
			{
				GUI_Element* elementEnt = getChild(CLIENT_GUI_ENTER_TRIGGER);
				if(elementEnt)
				{
					iRect newRect = elementEnt->getRect();
					elementEnt->onLMouseDown(newRect.x1, newRect.y1);
					elementEnt->onLMouseUp(newRect.x1, newRect.y1);
					return;
				}
			}
			break;

			case SDLK_TAB:
			{
				GUI_Element* elementEnt = getChild(CLIENT_GUI_TAB_START);
				if(m_actElement && m_actElement->getInternalID() >= CLIENT_GUI_TAB_START && m_actElement->getInternalID() <= CLIENT_GUI_TAB_END)
				{
					GUI_Element* elementTest = getChild(m_actElement->getInternalID() + 1);
					if(elementTest)
						elementEnt = elementTest;
				}
				if(elementEnt)
					setActiveElement(elementEnt);
			}
			return;

			default:
				break;
		}
	}
	if(m_actElement)
		m_actElement->onKeyDown(event);
}

void GUI_Window::onKeyUp(SDL_Event& event)
{
	if(m_actElement)
		m_actElement->onKeyUp(event);
}

void GUI_Window::onLMouseDown(Sint32 x, Sint32 y)
{
	for(std::vector<GUI_Element*>::reverse_iterator it = m_childs.rbegin(), end = m_childs.rend(); it != end; ++it)
	{
		if((*it)->isEventable() && (*it)->getRect().isPointInside(x, y))
		{
			setActiveElement((*it));
			(*it)->onLMouseDown(x, y);
			return;
		}
	}

	if(m_internalID != GUI_WINDOW_MAIN)
	{
		iRect dragWindow = iRect(m_tRect.x1, m_tRect.y1, m_tRect.x2, 17);
		if(dragWindow.isPointInside(x, y))
		{
			m_bMouseDragging = true;
			m_mouseEvent.x1 = x - m_tRect.x1;
			m_mouseEvent.y1 = y - m_tRect.y1;
		}
	}
}

void GUI_Window::onLMouseUp(Sint32 x, Sint32 y)
{
	if(m_bMouseDragging)
		m_bMouseDragging = false;

	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
	{
		if((*it)->isEventable())
			(*it)->onLMouseUp(x, y);
	}
}

void GUI_Window::onRMouseDown(Sint32 x, Sint32 y)
{
	for(std::vector<GUI_Element*>::reverse_iterator it = m_childs.rbegin(), end = m_childs.rend(); it != end; ++it)
	{
		if((*it)->isEventable() && (*it)->getRect().isPointInside(x, y))
		{
			setActiveElement((*it));
			(*it)->onRMouseDown(x, y);
			return;
		}
	}
}

void GUI_Window::onRMouseUp(Sint32 x, Sint32 y)
{
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
	{
		if((*it)->isEventable())
			(*it)->onRMouseUp(x, y);
	}
}

void GUI_Window::onWheel(Sint32 x, Sint32 y, bool wheelUP)
{
	for(std::vector<GUI_Element*>::reverse_iterator it = m_childs.rbegin(), end = m_childs.rend(); it != end; ++it)
	{
		if((*it)->isEventable() && (*it)->getRect().isPointInside(x, y))
		{
			setActiveElement((*it));
			(*it)->onWheel(x, y, wheelUP);
			return;
		}
	}
}

void GUI_Window::onMouseMove(Sint32 x, Sint32 y, bool isInsideParent)
{
	if(m_bMouseDragging)
	{
		onMove(x - m_mouseEvent.x1, y - m_mouseEvent.y1);
		return;
	}
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
		(*it)->onMouseMove(x, y, isInsideParent);
}

void GUI_Window::onMove(Sint32 x, Sint32 y)
{
	iRect newRect = m_tRect;
	newRect.x1 = x;
	newRect.y1 = y;

	#if !defined(__ALLOW_WINDOWS_OUTBOUNDS__)
	Sint32 maxX = g_engine.getWindowWidth();
	Sint32 maxY = g_engine.getWindowHeight();
	if(newRect.x1 + newRect.x2 > maxX)
		newRect.x1 = maxX - newRect.x2;
    if(newRect.y1 + newRect.y2 > maxY)
		newRect.y1 = maxY - newRect.y2;
	if(newRect.x1 < 0)
		newRect.x1 = 0;
	if(newRect.y1 < 0)
		newRect.y1 = 0;
	#endif

	setRect(newRect);
}

void GUI_Window::render()
{
	auto& renderer = g_engine.getRender();
	switch(m_internalID)
	{
		case GUI_WINDOW_MAIN:
		{
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_DITHER_BACKGROUND_X, GUI_UI_ICON_DITHER_BACKGROUND_Y, GUI_UI_ICON_DITHER_BACKGROUND_W, GUI_UI_ICON_DITHER_BACKGROUND_H, m_tRect.x1 + 6, m_tRect.y1 + 6, m_tRect.x2 - 12, m_tRect.y2 - 12);
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_DITHER_TOPLEFT_BORDER_X, GUI_UI_ICON_DITHER_TOPLEFT_BORDER_Y, m_tRect.x1, m_tRect.y1, GUI_UI_ICON_DITHER_TOPLEFT_BORDER_W, GUI_UI_ICON_DITHER_TOPLEFT_BORDER_H);
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_DITHER_TOPRIGHT_BORDER_X, GUI_UI_ICON_DITHER_TOPRIGHT_BORDER_Y, m_tRect.x1 + m_tRect.x2 - 5, m_tRect.y1, GUI_UI_ICON_DITHER_TOPRIGHT_BORDER_W, GUI_UI_ICON_DITHER_TOPRIGHT_BORDER_H);
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_DITHER_BOTLEFT_BORDER_X, GUI_UI_ICON_DITHER_BOTLEFT_BORDER_Y, m_tRect.x1, m_tRect.y1 + m_tRect.y2 - 5, GUI_UI_ICON_DITHER_BOTLEFT_BORDER_W, GUI_UI_ICON_DITHER_BOTLEFT_BORDER_H);
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_DITHER_BOTRIGHT_BORDER_X, GUI_UI_ICON_DITHER_BOTRIGHT_BORDER_Y, m_tRect.x1 + m_tRect.x2 - 5, m_tRect.y1 + m_tRect.y2 - 5, GUI_UI_ICON_DITHER_BOTRIGHT_BORDER_W, GUI_UI_ICON_DITHER_BOTRIGHT_BORDER_H);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_DITHER_TOP_SLIDER_X, GUI_UI_ICON_DITHER_TOP_SLIDER_Y, GUI_UI_ICON_DITHER_TOP_SLIDER_W, GUI_UI_ICON_DITHER_TOP_SLIDER_H, m_tRect.x1 + 5, m_tRect.y1, m_tRect.x2 - 10, 6);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_DITHER_LEFT_SLIDER_X, GUI_UI_ICON_DITHER_LEFT_SLIDER_Y, GUI_UI_ICON_DITHER_LEFT_SLIDER_W, GUI_UI_ICON_DITHER_LEFT_SLIDER_H, m_tRect.x1, m_tRect.y1 + 5, 6, m_tRect.y2 - 10);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_DITHER_RIGHT_SLIDER_X, GUI_UI_ICON_DITHER_RIGHT_SLIDER_Y, GUI_UI_ICON_DITHER_RIGHT_SLIDER_W, GUI_UI_ICON_DITHER_RIGHT_SLIDER_H, m_tRect.x1 + m_tRect.x2 - 6, m_tRect.y1 + 5, 6, m_tRect.y2 - 10);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_DITHER_BOTTOM_SLIDER_X, GUI_UI_ICON_DITHER_BOTTOM_SLIDER_Y, GUI_UI_ICON_DITHER_BOTTOM_SLIDER_W, GUI_UI_ICON_DITHER_BOTTOM_SLIDER_H, m_tRect.x1 + 5, m_tRect.y1 + m_tRect.y2 - 6, m_tRect.x2 - 10, 6);
		}
		break;
		default:
		{
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_BACKGROUND_GREY_X, GUI_UI_BACKGROUND_GREY_Y, GUI_UI_BACKGROUND_GREY_W, GUI_UI_BACKGROUND_GREY_H, m_tRect.x1 + 4, m_tRect.y1 + 17, m_tRect.x2 - 8, m_tRect.y2 - 21);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_WINDOW_TOP_CENTER_X, GUI_UI_ICON_WINDOW_TOP_CENTER_Y, GUI_UI_ICON_WINDOW_TOP_CENTER_W, GUI_UI_ICON_WINDOW_TOP_CENTER_H, m_tRect.x1 + 4, m_tRect.y1, m_tRect.x2 - 8, 17);
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_WINDOW_LEFT_CENTER_X, GUI_UI_ICON_WINDOW_LEFT_CENTER_Y, m_tRect.x1, m_tRect.y1, GUI_UI_ICON_WINDOW_LEFT_CENTER_W, GUI_UI_ICON_WINDOW_LEFT_CENTER_H);
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_WINDOW_RIGHT_CENTER_X, GUI_UI_ICON_WINDOW_RIGHT_CENTER_Y, m_tRect.x1 + m_tRect.x2 - 4, m_tRect.y1, GUI_UI_ICON_WINDOW_RIGHT_CENTER_W, GUI_UI_ICON_WINDOW_RIGHT_CENTER_H);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_LEFT_SLIDER_X, GUI_UI_ICON_LEFT_SLIDER_Y, GUI_UI_ICON_LEFT_SLIDER_W, GUI_UI_ICON_LEFT_SLIDER_H, m_tRect.x1, m_tRect.y1 + 17, 4, m_tRect.y2 - 17);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_BOTTOM_SLIDER_X, GUI_UI_ICON_BOTTOM_SLIDER_Y, GUI_UI_ICON_BOTTOM_SLIDER_W, GUI_UI_ICON_BOTTOM_SLIDER_H, m_tRect.x1 + 4, m_tRect.y1 + m_tRect.y2 - 4, m_tRect.x2 - 4, 4);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_RIGHT_SLIDER_X, GUI_UI_ICON_RIGHT_SLIDER_Y, GUI_UI_ICON_RIGHT_SLIDER_W, GUI_UI_ICON_RIGHT_SLIDER_H, m_tRect.x1 + m_tRect.x2 - 4, m_tRect.y1 + 17, 4, m_tRect.y2 - 20);
			g_engine.drawFont(CLIENT_FONT_NONOUTLINED, m_tRect.x1 + (m_tRect.x2 / 2), m_tRect.y1 + 4, m_Title, 143, 143, 143, CLIENT_FONT_ALIGN_CENTER);
		}
		break;
	}
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
		(*it)->render();
}
