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

#include "GUI_Panel.h"
#include "GUI_PanelWindow.h"
#include "../engine.h"

extern Engine g_engine;

GUI_Panel::GUI_Panel(iRect boxRect, Sint32 internalId)
{
	setRect(boxRect);
	m_lastPosY = m_tRect.y1 + 2;
	m_freeHeight = m_tRect.y2 - 4;
	m_internalID = internalId;
}

GUI_Panel::~GUI_Panel()
{
	clearPanels();
}

void GUI_Panel::setRect(iRect& NewRect)
{
	Sint32 posX = NewRect.x1 + 2;
	m_lastPosY = NewRect.y1 + 2;
	m_freeHeight = NewRect.y2 - 4;
	for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
	{
		GUI_PanelWindow* panel = (*it);
		iRect& panelRect = panel->getOriginalRect();
		iRect newPanelRect;
		newPanelRect.x1 = posX;
		newPanelRect.y1 = m_lastPosY;
		newPanelRect.x2 = panelRect.x2;
		newPanelRect.y2 = panelRect.y2;
		panel->setRect(newPanelRect, true);
		m_lastPosY += panelRect.y2;
		m_freeHeight -= panelRect.y2;
	}
	m_tRect = NewRect;
}

Sint32 GUI_Panel::getFreeHeight()
{
	return m_freeHeight;
}

void GUI_Panel::clearPanels()
{
	for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
		delete (*it);
	m_panels.clear();
}

void GUI_Panel::resizePanel(GUI_PanelWindow* pPanel, Sint32 x, Sint32 y)
{
	iRect& panelRect = pPanel->getOriginalRect();
	Sint32 height = panelRect.y2;
	if(y == height)
		return;

	std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin();
	for(std::vector<GUI_PanelWindow*>::iterator end = m_panels.end(); it != end; ++it)
	{
		if((*it) == pPanel)
		{
			++it;
			break;
		}
	}

	Sint32 minHeight = pPanel->getMinHeight();
	Sint32 maxHeight = pPanel->getMaxHeight();
	if(maxHeight)
		maxHeight = UTIL_min<Sint32>(height + m_freeHeight, maxHeight);
	else
		maxHeight = height + m_freeHeight;
	if(y < height)
	{
		if(height > minHeight)
		{
			Sint32 newHeight = UTIL_max<Sint32>(y, minHeight);
			pPanel->setSize(x, newHeight);
			if(it == m_panels.end())
			{
				Sint32 diff = height - newHeight;
				m_lastPosY -= diff;
				m_freeHeight += diff;
			}
			else
			{
				Sint32 diff = height - newHeight;
				it = --m_panels.end();

				iRect& pRect = (*it)->getOriginalRect();
				Sint32 pHeight = pRect.y2;
				if(pHeight > 19)
				{
					if((*it)->getMaxHeight())
						(*it)->setSize(pRect.x2, UTIL_min<Sint32>(pHeight + diff, (*it)->getMaxHeight()));
					else
						(*it)->setSize(pRect.x2, pHeight + diff);
				}

				checkPanels();
			}
		}
	}
	else
	{
		if(it == m_panels.end())
		{
			if(height < maxHeight)
			{
				Sint32 newHeight = UTIL_min<Sint32>(y, maxHeight);
				pPanel->setSize(x, newHeight);

				Sint32 diff = newHeight - height;
				m_lastPosY += diff;
				m_freeHeight -= diff;
			}
		}
		else
		{
			Sint32 expectDiff = (pPanel->getMaxHeight() ? UTIL_min<Sint32>(y, pPanel->getMaxHeight()) : y) - height;
			if(m_freeHeight > 0)
			{
				Sint32 diff = UTIL_min<Sint32>(expectDiff, m_freeHeight);
				expectDiff -= diff;
				height += diff;
			}
			if(expectDiff > 0)
			{
				for(std::vector<GUI_PanelWindow*>::reverse_iterator rit = m_panels.rbegin(), end = m_panels.rend(); rit != end; ++rit)
				{
					if(expectDiff <= 0 || (*rit) == pPanel)
						break;
					else
					{
						iRect& pRect = (*rit)->getOriginalRect();
						Sint32 pHeight = pRect.y2;
						minHeight = (*it)->getMinHeight();
						if(pHeight > minHeight)
						{
							Sint32 diff = UTIL_min<Sint32>(expectDiff, pHeight - minHeight);
							(*rit)->setSize(pRect.x2, pHeight - diff);
							expectDiff -= diff;
							height += diff;
						}
					}
				}
			}
			pPanel->setSize(x, height);
			checkPanels();
		}
	}
}

void GUI_Panel::checkPanels()
{
	Sint32 posX = m_tRect.x1 + 2;
	m_lastPosY = m_tRect.y1 + 2;
	m_freeHeight = m_tRect.y2 - 4;
	for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
	{
		GUI_PanelWindow* panel = (*it);
		iRect& panelRect = panel->getOriginalRect();
		iRect newPanelRect;
		newPanelRect.x1 = posX;
		newPanelRect.y1 = m_lastPosY;
		newPanelRect.x2 = panelRect.x2;
		newPanelRect.y2 = panelRect.y2;
		// Only call setRect (which moves children) if position actually changed
		if(newPanelRect.x1 != panelRect.x1 || newPanelRect.y1 != panelRect.y1 ||
		   newPanelRect.x2 != panelRect.x2 || newPanelRect.y2 != panelRect.y2)
			panel->setRect(newPanelRect, true);
		m_lastPosY += panelRect.y2;
		m_freeHeight -= panelRect.y2;
	}
}

void GUI_Panel::checkPanel(GUI_PanelWindow* pPanel, Sint32, Sint32 y)
{
	for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
	{
		if((*it) == pPanel)
		{
			m_panels.erase(it);
			break;
		}
	}
	std::vector<GUI_PanelWindow*>::iterator nit = m_panels.end();
	for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
	{
		GUI_PanelWindow* panel = (*it);
		if(panel != pPanel)
		{
			iRect& panelRect = panel->getOriginalRect();
			Sint32 py = panelRect.y1 + (panelRect.y2 >> 1);
			if(y < py)
			{
				nit = it;
				break;
			}
		}
	}
	m_panels.insert(nit, pPanel);
	checkPanels();
}

bool GUI_Panel::tryFreeHeight(GUI_PanelWindow* pPanel)
{
	if(m_internalID == GUI_PANEL_MAIN)
		return false;

	iRect& panelRect = pPanel->getOriginalRect();
	if(m_freeHeight >= pPanel->getMinHeight())
	{
		//We assume that the code already check if it can add "default" height so we check if the empty space can be assigned as height
		pPanel->setSize(panelRect.x2, m_freeHeight);
		return true;
	}

	Sint32 height = panelRect.y2;
	Sint32 needFreeHeight = pPanel->getMinHeight();
	if(height > needFreeHeight)
		pPanel->setSize(panelRect.x2, needFreeHeight);
	else
		needFreeHeight = height;//Probably minimized

	return tryFreeHeight(needFreeHeight);
}

bool GUI_Panel::tryFreeHeight(Sint32 needFreeHeight)
{
	if(m_tRect.y2 - 4 < needFreeHeight)//In this case we can't get enough room
		return false;

	if(m_freeHeight >= needFreeHeight)
		return true;

	//Cipbia minimizes inventory if necessary - so let's keep it as TODO
	//Do things in order instead of checking if we can resize windows?

	Sint32 resizableHeight = m_freeHeight;
	for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
	{
		iRect& pRect = (*it)->getOriginalRect();
		Sint32 minHeight = (*it)->getMinHeight();
		Sint32 height = pRect.y2;
		if(height > minHeight)
			resizableHeight += (height - minHeight);
	}
	if(resizableHeight >= needFreeHeight)
	{
		needFreeHeight -= m_freeHeight;
		for(std::vector<GUI_PanelWindow*>::reverse_iterator it = m_panels.rbegin(), end = m_panels.rend(); it != end; ++it)
		{
			iRect& pRect = (*it)->getOriginalRect();
			Sint32 minHeight = (*it)->getMinHeight();
			Sint32 height = pRect.y2;
			if(height > minHeight)
			{
				Sint32 gainHeight = UTIL_min<Sint32>(needFreeHeight, height - minHeight);
				(*it)->setSize(pRect.x2, height - gainHeight);
				needFreeHeight -= gainHeight;
				if(needFreeHeight <= 0)
					break;
			}
		}
		checkPanels();
	}
	else
	{
		needFreeHeight -= m_freeHeight;
		for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.end(); --it >= m_panels.begin();)
		{
			iRect& pRect = (*it)->getOriginalRect();
			Sint32 height = pRect.y2;
			needFreeHeight -= height;

			delete (*it);
			it = m_panels.erase(it);
			if(needFreeHeight <= 0)
				break;
		}
		checkPanels();
	}
	return true;
}

void GUI_Panel::addPanel(GUI_PanelWindow* pPanel)
{
	m_panels.push_back(pPanel);
	checkPanels();
}

void GUI_Panel::removePanel(GUI_PanelWindow* pPanel, bool deletePanel)
{
	for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
	{
		if((*it) == pPanel)
		{
			m_panels.erase(it);
			break;
		}
	}
	if(deletePanel)
		delete pPanel;
	checkPanels();
}

GUI_PanelWindow* GUI_Panel::getPanel(Uint32 internalID)
{
	for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
	{
		if((*it)->getInternalID() == internalID)
			return (*it);
	}
	return NULL;
}

void* GUI_Panel::onAction(Sint32 x, Sint32 y)
{
	for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
	{
		if((*it)->isInsideRect(x, y))
			return (*it)->onAction(x, y);
	}
	return NULL;
}

void GUI_Panel::onLMouseDown(Sint32 x, Sint32 y)
{
	for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
	{
		if((*it)->isInsideRect(x, y))
		{
			(*it)->onLMouseDown(x, y);
			return;
		}
	}
}

void GUI_Panel::onLMouseUp(Sint32 x, Sint32 y)
{
	for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
		(*it)->onLMouseUp(x, y);
}

void GUI_Panel::onRMouseDown(Sint32 x, Sint32 y)
{
	for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
	{
		if((*it)->isInsideRect(x, y))
		{
			(*it)->onRMouseDown(x, y);
			return;
		}
	}
}

void GUI_Panel::onRMouseUp(Sint32 x, Sint32 y)
{
	for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
		(*it)->onRMouseUp(x, y);
}

void GUI_Panel::onWheel(Sint32 x, Sint32 y, bool wheelUP)
{
	for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
	{
		if((*it)->isInsideRect(x, y))
		{
			(*it)->onWheel(x, y, wheelUP);
			return;
		}
	}
}

void GUI_Panel::onMouseMove(Sint32 x, Sint32 y, bool isInsideParent)
{
	for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
		(*it)->onMouseMove(x, y, isInsideParent);
}

void GUI_Panel::render()
{
	auto& renderer = g_engine.getRender();
	renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_EXTRA_BORDER_X, GUI_UI_ICON_EXTRA_BORDER_Y, m_tRect.x1 + m_tRect.x2 - 2, m_tRect.y1, GUI_UI_ICON_EXTRA_BORDER_W, GUI_UI_ICON_EXTRA_BORDER_H);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_DARK_X, GUI_UI_ICON_VERTICAL_LINE_DARK_Y, GUI_UI_ICON_VERTICAL_LINE_DARK_W, GUI_UI_ICON_VERTICAL_LINE_DARK_H, m_tRect.x1 + m_tRect.x2 - 2, m_tRect.y1 + 2, 2, m_tRect.y2 - 4);
	renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_EXTRA_BORDER_X, GUI_UI_ICON_EXTRA_BORDER_Y, m_tRect.x1, m_tRect.y1 + m_tRect.y2 - 2, GUI_UI_ICON_EXTRA_BORDER_W, GUI_UI_ICON_EXTRA_BORDER_H);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_X, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_Y, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_W, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_H, m_tRect.x1, m_tRect.y1 + 2, 2, m_tRect.y2 - 4);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_X, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_Y, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_W, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_H, m_tRect.x1, m_tRect.y1, m_tRect.x2 - 2, 2);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_DARK_X, GUI_UI_ICON_HORIZONTAL_LINE_DARK_Y, GUI_UI_ICON_HORIZONTAL_LINE_DARK_W, GUI_UI_ICON_HORIZONTAL_LINE_DARK_H, m_tRect.x1 + 2, m_tRect.y1 + m_tRect.y2 - 2, m_tRect.x2 - 2, 2);
	if(m_freeHeight > 0)
	{
		renderer->setClipRect(m_tRect.x1 + 2, m_lastPosY, m_tRect.x2 - 4, m_freeHeight);
		renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_BACKGROUND_GREY_X, GUI_UI_BACKGROUND_GREY_Y, GUI_UI_BACKGROUND_GREY_W, GUI_UI_BACKGROUND_GREY_H, m_tRect.x1 + 2, 0, m_tRect.x2 - 4, g_engine.getWindowHeight());
		renderer->disableClipRect();
	}

	if(g_engine.getTopPanel())
	{
		for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
		{
			if(g_engine.getTopPanel() != (*it))
				(*it)->render();
		}
	}
	else
	{
		for(std::vector<GUI_PanelWindow*>::iterator it = m_panels.begin(), end = m_panels.end(); it != end; ++it)
			(*it)->render();
	}
}
