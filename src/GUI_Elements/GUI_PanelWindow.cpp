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

#include "GUI_PanelWindow.h"
#include "GUI_Panel.h"
#include "../engine.h"

extern Engine g_engine;
extern Sint32 g_actualCursor;

GUI_PanelWindow::GUI_PanelWindow(iRect boxRect, bool windowed, Uint32 internalID, bool parentChangeable) :
	m_internalID(internalID), m_windowed(windowed), m_parentChangeable(parentChangeable)
{
	setRect(boxRect, true);
	m_cachedHeight = m_nRect.y2;
}

GUI_PanelWindow::~GUI_PanelWindow()
{
	clearChilds();
	if(m_eventHandlerFunction)
		m_eventHandlerFunction(m_exitEvent, (m_tRect.y2 > 19 ? m_tRect.y2 : m_cachedHeight));

	if(g_engine.getTopPanel() == this)
		g_engine.setTopPanel(NULL);
}

void GUI_PanelWindow::setEventCallback(void (*eventHandlerFunction)(Uint32, Sint32), Uint32 widthEvent, Uint32 heightEvent, Uint32 exitEvent)
{
	m_resizeWidth = widthEvent;
	m_resizeHeight = heightEvent;
	m_exitEvent = exitEvent;
	m_eventHandlerFunction = eventHandlerFunction;
}

void GUI_PanelWindow::setRect(iRect& NewRect, bool NewPos)
{
	// Only move children if x1 or y1 changed (position change = drag)
	// If only y2 changed (height change = resize), children stay in place
	bool positionChanged = (NewRect.x1 != m_tRect.x1 || NewRect.y1 != m_tRect.y1);

	if(positionChanged)
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
	}
	if(NewPos)
		m_nRect = NewRect;
	m_tRect = NewRect;
}

void GUI_PanelWindow::setSize(Sint32 width, Sint32 height)
{
	m_nRect.x2 = width;
	m_nRect.y2 = height;
	m_tRect.x2 = width;
	m_tRect.y2 = height;
	if(m_eventHandlerFunction)
	{
		UTIL_SafeEventHandler(m_eventHandlerFunction, m_resizeWidth, width);
		UTIL_SafeEventHandler(m_eventHandlerFunction, m_resizeHeight, height);
	}
}

void GUI_PanelWindow::checkPanels()
{
	if(m_parent)
		m_parent->checkPanels();
}

void GUI_PanelWindow::clearChilds()
{
	setActiveElement(NULL);
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
		delete (*it);
	m_childs.clear();
}

void GUI_PanelWindow::addChild(GUI_Element* pChild)
{
	m_childs.push_back(pChild);

	iRect newRect;
	newRect.x1 = m_tRect.x1 + pChild->getRect().x1;
	newRect.y1 = m_tRect.y1 + pChild->getRect().y1;
	newRect.x2 = pChild->getRect().x2;
	newRect.y2 = pChild->getRect().y2;
	pChild->setRect(newRect);
}

void GUI_PanelWindow::removeChild(GUI_Element* pChild)
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

GUI_Element* GUI_PanelWindow::getChild(Uint32 internalID)
{
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
	{
		if((*it)->getInternalID() == internalID)
			return (*it);
	}
	return NULL;
}

void GUI_PanelWindow::setActiveElement(GUI_Element* actElement)
{
	if(m_actElement == actElement)
		return;

	if(m_actElement)
		m_actElement->deActivate();

	m_actElement = actElement;
	if(m_actElement)
	{
        m_actElement->activate();
		for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
		{
			if((*it) == m_actElement)
			{
				m_childs.erase(it);
				break;
			}
		}
		m_childs.push_back(m_actElement);
	}
}

void* GUI_PanelWindow::onAction(Sint32 x, Sint32 y)
{
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
	{
		if((*it)->isEventable() && (*it)->getRect().isPointInside(x, y))
			return (*it)->onAction(x, y);
	}
	return NULL;
}

void GUI_PanelWindow::onLMouseDown(Sint32 x, Sint32 y)
{
	std::vector<GUI_Element*> childsBackup = m_childs;
	for(std::vector<GUI_Element*>::reverse_iterator it = childsBackup.rbegin(), end = childsBackup.rend(); it != end; ++it)
	{
		if((*it)->isEventable() && (*it)->getRect().isPointInside(x, y))
		{
			setActiveElement((*it));
			(*it)->onLMouseDown(x, y);
			return;
		}
	}
	if(m_windowed && m_tRect.y2 > 19)
	{
		iRect dragWindow = iRect(m_tRect.x1, m_tRect.y1 + m_tRect.y2 - 4, m_tRect.x2, 4);
		if(dragWindow.isPointInside(x, y))
		{
			m_bMouseResizing = true;
			m_mouseEvent.x1 = x - m_tRect.x2;
			m_mouseEvent.y1 = y - m_tRect.y2;
			return;
		}
		m_bMouseDragging = true;
		m_mouseEvent.x1 = x - m_tRect.x1;
		m_mouseEvent.y1 = y - m_tRect.y1;
		g_engine.setTopPanel(this);
	}
	else
	{
		m_bMouseDragging = true;
		m_mouseEvent.x1 = x - m_tRect.x1;
		m_mouseEvent.y1 = y - m_tRect.y1;
		g_engine.setTopPanel(this);
	}
}

void GUI_PanelWindow::onLMouseUp(Sint32 x, Sint32 y)
{
	if(m_bMouseDragging)
	{
		g_engine.setTopPanel(NULL);
		m_bMouseDragging = false;
		setRect(m_nRect);
	}
	if(m_bMouseResizing)
		m_bMouseResizing = false;
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
		(*it)->onLMouseUp(x, y);
}

void GUI_PanelWindow::onRMouseDown(Sint32 x, Sint32 y)
{
	std::vector<GUI_Element*> childsBackup = m_childs;
	for(std::vector<GUI_Element*>::reverse_iterator it = childsBackup.rbegin(), end = childsBackup.rend(); it != end; ++it)
	{
		if((*it)->isEventable() && (*it)->getRect().isPointInside(x, y))
		{
			setActiveElement((*it));
			(*it)->onRMouseDown(x, y);
			return;
		}
	}
}

void GUI_PanelWindow::onRMouseUp(Sint32 x, Sint32 y)
{
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
		(*it)->onRMouseUp(x, y);
}

void GUI_PanelWindow::onWheel(Sint32 x, Sint32 y, bool wheelUP)
{
	std::vector<GUI_Element*> childsBackup = m_childs;
	for(std::vector<GUI_Element*>::reverse_iterator it = childsBackup.rbegin(), end = childsBackup.rend(); it != end; ++it)
	{
		if((*it)->isEventable() && (*it)->getRect().isPointInside(x, y))
		{
			setActiveElement((*it));
			(*it)->onWheel(x, y, wheelUP);
			return;
		}
	}
}

void GUI_PanelWindow::onMouseMove(Sint32 x, Sint32 y, bool isInsideParent)
{
	if(m_bMouseDragging)
	{
		Sint32 beforeY = m_tRect.y1;
		if(m_parentChangeable)
			onMoveWindowed(x - m_mouseEvent.x1, y - m_mouseEvent.y1);
		else
			onMoveNormal(m_tRect.x1, y - m_mouseEvent.y1);

		Sint32 _x = m_tRect.x1 + (m_tRect.x2 >> 1);
		Sint32 _y;
		if(m_tRect.y1 > beforeY)
			_y = m_tRect.y1 + m_tRect.y2;
		else
			_y = m_tRect.y1;
		UTIL_UpdatePanels(SDL_reinterpret_cast(void*, this), _x, _y);
		return;
	}
	else if(m_bMouseResizing)
	{
		UTIL_ResizePanel(SDL_reinterpret_cast(void*, this), m_tRect.x2, y - m_mouseEvent.y1);
		g_actualCursor = CLIENT_CURSOR_RESIZENS;
		return;
	}
	if(isInsideParent)
		isInsideParent = isInsideRect(x, y);
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
		(*it)->onMouseMove(x, y, isInsideParent);

	if(m_windowed && m_tRect.y2 > 19)
	{
		iRect dragWindow = iRect(m_tRect.x1, m_tRect.y1 + m_tRect.y2 - 4, m_tRect.x2, 4);
		if(dragWindow.isPointInside(x, y))
			g_actualCursor = CLIENT_CURSOR_RESIZENS;
	}
}

void GUI_PanelWindow::onMoveWindowed(Sint32 x, Sint32 y)
{
	iRect newRect = m_tRect;
	newRect.x1 = x;
	newRect.y1 = y;

	Sint32 maxX = g_engine.getWindowWidth() - 2;
	Sint32 maxY = g_engine.getWindowHeight() + 2;
	if(newRect.x1 + newRect.x2 > maxX)
		newRect.x1 = maxX - newRect.x2;
	if(newRect.y1 + newRect.y2 > maxY)
		newRect.y1 = maxY - newRect.y2;
	if(newRect.x1 < 2)
		newRect.x1 = 2;
	if(newRect.y1 < 2)
		newRect.y1 = 2;

	setRect(newRect);
}

void GUI_PanelWindow::onMoveNormal(Sint32 x, Sint32 y)
{
	iRect& parentSize = m_parent->getRect();
	iRect newRect = m_tRect;
	newRect.x1 = x;
	newRect.y1 = y;

	Sint32 minY = parentSize.y1 + 2;
	Sint32 maxY = minY + parentSize.y2 - 4;
	if(newRect.y1 + newRect.y2 > maxY)
		newRect.y1 = maxY - newRect.y2;
	if(newRect.y1 < minY)
		newRect.y1 = minY;

	setRect(newRect);
}

void GUI_PanelWindow::render()
{
	auto& renderer = g_engine.getRender();
	if(m_bMouseDragging)
		renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_BACKGROUND_DARKGREY_X, GUI_UI_BACKGROUND_DARKGREY_Y, GUI_UI_BACKGROUND_DARKGREY_W, GUI_UI_BACKGROUND_DARKGREY_H, m_nRect.x1, m_nRect.y1, m_nRect.x2, m_nRect.y2);
	if(m_windowed)
	{
		renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_PWINDOW_LEFT_CENTER_X, GUI_UI_ICON_PWINDOW_LEFT_CENTER_Y, m_tRect.x1 - 2, m_tRect.y1 - 2, GUI_UI_ICON_PWINDOW_LEFT_CENTER_W, GUI_UI_ICON_PWINDOW_LEFT_CENTER_H);
		renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_PWINDOW_TOP_CENTER_X, GUI_UI_ICON_PWINDOW_TOP_CENTER_Y, GUI_UI_ICON_PWINDOW_TOP_CENTER_W, GUI_UI_ICON_PWINDOW_TOP_CENTER_H, m_tRect.x1 + 2, m_tRect.y1 - 2, m_tRect.x2 - 4, 15);
		renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_PWINDOW_RIGHT_CENTER_X, GUI_UI_ICON_PWINDOW_RIGHT_CENTER_Y, m_tRect.x1 + m_tRect.x2 - 2, m_tRect.y1 - 2, GUI_UI_ICON_PWINDOW_RIGHT_CENTER_W, GUI_UI_ICON_PWINDOW_RIGHT_CENTER_H);
		renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_BOTTOM_SLIDER_X, GUI_UI_ICON_BOTTOM_SLIDER_Y, GUI_UI_ICON_BOTTOM_SLIDER_W, GUI_UI_ICON_BOTTOM_SLIDER_H, m_tRect.x1 + 2, m_tRect.y1 + m_tRect.y2 - 6, m_tRect.x2 - 4, 4);
		if(!m_bMouseDragging)
		{
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_EXTRA_BORDER_X, GUI_UI_ICON_EXTRA_BORDER_Y, m_tRect.x1 + m_tRect.x2, m_tRect.y1 + m_tRect.y2 - 2, GUI_UI_ICON_EXTRA_BORDER_W, GUI_UI_ICON_EXTRA_BORDER_H);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_X, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_Y, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_W, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_H, m_tRect.x1 - 2, m_tRect.y1 + m_tRect.y2 - 2, m_tRect.x2 + 2, 2);
		}
		if(m_tRect.y2 > 19)
		{
			// Draw background anchored to absolute position to prevent shifting during resize
			renderer->setClipRect(m_tRect.x1 + 2, m_tRect.y1 + 13, m_tRect.x2 - 4, m_tRect.y2 - 19);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_BACKGROUND_GREY_X, GUI_UI_BACKGROUND_GREY_Y, GUI_UI_BACKGROUND_GREY_W, GUI_UI_BACKGROUND_GREY_H, m_tRect.x1 + 2, 0, m_tRect.x2 - 4, g_engine.getWindowHeight());
			renderer->disableClipRect();
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_LEFT_SLIDER_X, GUI_UI_ICON_LEFT_SLIDER_Y, GUI_UI_ICON_LEFT_SLIDER_W, GUI_UI_ICON_LEFT_SLIDER_H, m_tRect.x1 - 2, m_tRect.y1 + 13, 4, m_tRect.y2 - 19);
			renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_RIGHT_SLIDER_X, GUI_UI_ICON_RIGHT_SLIDER_Y, GUI_UI_ICON_RIGHT_SLIDER_W, GUI_UI_ICON_RIGHT_SLIDER_H, m_tRect.x1 + m_tRect.x2 - 2, m_tRect.y1 + 13, 4, m_tRect.y2 - 19);
		}
	}
	else
		renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_BACKGROUND_GREY_X, GUI_UI_BACKGROUND_GREY_Y, GUI_UI_BACKGROUND_GREY_W, GUI_UI_BACKGROUND_GREY_H, m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2);
	for(std::vector<GUI_Element*>::iterator it = m_childs.begin(), end = m_childs.end(); it != end; ++it)
		(*it)->render();

}
