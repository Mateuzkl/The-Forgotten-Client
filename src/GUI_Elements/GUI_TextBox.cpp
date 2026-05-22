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

#include "GUI_TextBox.h"
#include "../engine.h"

extern Engine g_engine;
extern Uint32 g_frameTime;

GUI_TextBox::GUI_TextBox(iRect boxRect, const std::string text, Uint32 internalID, Uint8 red, Uint8 green, Uint8 blue) :
	m_sText(std::move(text)), m_red(red), m_green(green), m_blue(blue)
{
	setRect(boxRect);
	m_sVisibleText.assign(m_sText);
	m_internalID = internalID;
	m_cursorTimer = g_frameTime;
}

void GUI_TextBox::setTextEventCallback(void (*eventHandlerFunction)(Uint32,Sint32), Uint32 mEvent)
{
	m_eventHandlerFunction = eventHandlerFunction;
	m_evtParam = mEvent;
}

void GUI_TextBox::setHideCharacter(char character)
{
	m_hideCharacter = character;
	if(m_hideCharacter == '\0')
		m_sVisibleText.assign(m_sText);
	else
		m_sVisibleText.assign(m_sText.length(), m_hideCharacter);
}

void GUI_TextBox::selectAll()
{
	setSelection(0, SDL_static_cast(Uint32, m_sText.length()));
}

void GUI_TextBox::setSelection(Uint32 start, Uint32 end)
{
	if(start == m_selectionStart && end == m_selectionEnd)
		return;

	if(start > end)
		std::swap(start, end);

	m_selectionStart = UTIL_max<Uint32>(0, UTIL_min<Uint32>(SDL_static_cast(Uint32, m_sText.length()), start));
	m_selectionEnd = UTIL_max<Uint32>(0, UTIL_min<Uint32>(SDL_static_cast(Uint32, m_sText.length()), end));
}

void GUI_TextBox::clearSelection()
{
	m_selectionStart = 0;
	m_selectionEnd = 0;
}

void GUI_TextBox::deleteSelection()
{
	Sint32 n = SDL_static_cast(Sint32, m_selectionEnd - m_selectionStart);
	if(n <= 0)
	{
		clearSelection();
		return;
	}

	removeTextToTextBox(n, m_selectionStart);
	if(m_cursorPosition != m_selectionStart)
		moveCursor(-n);
	clearSelection();
}

void GUI_TextBox::onTextInput(const char* textInput)
{
	if(hasSelection())
		deleteSelection();

	size_t len = SDL_strlen(textInput);
	Sint32 n = addTextToTextBox(std::string(textInput, len), m_cursorPosition);
	if(n != 0)
		moveCursor(n);
}

void GUI_TextBox::onKeyDown(SDL_Event& event)
{
	switch(event.key.keysym.sym)
	{
		case SDLK_BACKSPACE:
		{
			if(event.key.keysym.mod == KMOD_NONE)
			{
				if(hasSelection())
					deleteSelection();
				else if(m_cursorPosition != 0)
				{
					removeTextToTextBox(1, m_cursorPosition - 1);
					moveCursor(-1);
				}
			}
		}
		break;
		case SDLK_DELETE:
		{
			if(event.key.keysym.mod == KMOD_NONE)
			{
				if(hasSelection())
					deleteSelection();
				else if(m_cursorPosition != m_sText.length())
					removeTextToTextBox(1, m_cursorPosition);
			}
		}
		break;
		case SDLK_LEFT:
		{
			if(event.key.keysym.mod == KMOD_SHIFT)
			{
				if(!hasSelection())
					m_selectionReference = m_cursorPosition;

				moveCursor(-1);
				setSelection(m_selectionReference, m_cursorPosition);
			}
			else
			{
				clearSelection();
				moveCursor(-1);
			}
		}
		break;
		case SDLK_RIGHT:
		{
			if(event.key.keysym.mod == KMOD_SHIFT)
			{
				if(!hasSelection())
					m_selectionReference = m_cursorPosition;

				moveCursor(1);
				setSelection(m_selectionReference, m_cursorPosition);
			}
			else
			{
				clearSelection();
				moveCursor(1);
			}
		}
		break;
		case SDLK_HOME:
		{
			if(event.key.keysym.mod == KMOD_SHIFT)
			{
				if(m_cursorPosition != 0)
				{
					setSelection(m_cursorPosition, 0);
					moveCursor(-SDL_static_cast(Sint32, m_cursorPosition));
				}
			}
			else
			{
				if(m_cursorPosition != 0)
				{
					clearSelection();
					moveCursor(-SDL_static_cast(Sint32, m_cursorPosition));
				}
			}
		}
		break;
		case SDLK_END:
		{
			if(event.key.keysym.mod == KMOD_SHIFT)
			{
				if(m_cursorPosition != SDL_static_cast(Uint32, m_sText.length()))
				{
					setSelection(m_cursorPosition, SDL_static_cast(Uint32, m_sText.length()));
					moveCursor(SDL_static_cast(Sint32, SDL_static_cast(Uint32, m_sText.length()) - m_cursorPosition));
				}
			}
			else
			{
				if(m_cursorPosition != SDL_static_cast(Uint32, m_sText.length()))
				{
					clearSelection();
					moveCursor(SDL_static_cast(Sint32, SDL_static_cast(Uint32, m_sText.length()) - m_cursorPosition));
				}
			}
		}
		break;
		case SDLK_a:
		{
			if(event.key.keysym.mod == KMOD_CTRL)
			{
				m_selectionReference = 0;
				setSelection(m_selectionReference, SDL_static_cast(Uint32, m_sText.length()));
				moveCursor(SDL_static_cast(Sint32, SDL_static_cast(Uint32, m_sText.length()) - m_cursorPosition));
			}
		}
		break;
		case SDLK_x:
		{
			if(event.key.keysym.mod == KMOD_CTRL)
			{
				if(hasSelection())
				{
					UTIL_SetClipboardTextLatin1(UTIL_formatConsoleText(m_sText.substr(SDL_static_cast(size_t, m_selectionStart), SDL_static_cast(size_t, m_selectionEnd - m_selectionStart))).c_str());
					deleteSelection();
				}
			}
		}
		break;
		case SDLK_c:
		{
			if(event.key.keysym.mod == KMOD_CTRL)
			{
				if(hasSelection())
					UTIL_SetClipboardTextLatin1(UTIL_formatConsoleText(m_sText.substr(SDL_static_cast(size_t, m_selectionStart), SDL_static_cast(size_t, m_selectionEnd - m_selectionStart))).c_str());
			}
		}
		break;
		case SDLK_v:
		{
			if(event.key.keysym.mod == KMOD_CTRL)
			{
				if(hasSelection())
					deleteSelection();

				char* latin1 = UTIL_GetClipboardTextLatin1();
				if(latin1)
				{
					size_t len = SDL_strlen(latin1);
					Sint32 n = addTextToTextBox(std::string(latin1, len), m_cursorPosition);
					if(n != 0)
						moveCursor(n);
				}
			}
		}
		break;
		default: break;
	}
}

void GUI_TextBox::onLMouseDown(Sint32 x, Sint32)
{
	m_selecting = true;

	m_cursorRelativePosition = getNearestLetter(x);
	m_cursorPosition = m_positionOfFirstShownLetter + m_cursorRelativePosition;

	m_selectionReference = m_cursorPosition;
	setSelection(m_selectionReference, m_cursorPosition);
}

void GUI_TextBox::onLMouseUp(Sint32, Sint32)
{
	m_selecting = false;
}

void GUI_TextBox::onMouseMove(Sint32 x, Sint32, bool)
{
	if(m_selecting)
	{
		m_cursorRelativePosition = getNearestLetter(x);
		m_cursorPosition = m_positionOfFirstShownLetter + m_cursorRelativePosition;

		setSelection(m_selectionReference, m_cursorPosition);
	}
}

void GUI_TextBox::activate()
{
	m_bActive = true;
	if(SDL_HasScreenKeyboardSupport() && !SDL_IsTextInputActive())
		SDL_StartTextInput();

	m_cursorTimer = g_frameTime;
	m_bShowCursor = true;

	clearSelection();
	if(m_cursorPosition != SDL_static_cast(Uint32, m_sText.length()))
	{
		clearSelection();
		moveCursor(SDL_static_cast(Sint32, SDL_static_cast(Uint32, m_sText.length()) - m_cursorPosition));
	}
}

void GUI_TextBox::deActivate()
{
	m_bActive = false;
	if(SDL_HasScreenKeyboardSupport() && SDL_IsTextInputActive())
		SDL_StopTextInput();

	clearSelection();
}

void GUI_TextBox::render()
{
	if(g_frameTime - m_cursorTimer >= 333)
	{
		m_cursorTimer = g_frameTime;
		m_bShowCursor = !m_bShowCursor;
	}

	auto& renderer = g_engine.getRender();
	Uint32 numberOfLetters = getNumberOfLettersToShow();
	renderer->fillRectangle(m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2, 54, 54, 54, 255);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_DARK_X, GUI_UI_ICON_HORIZONTAL_LINE_DARK_Y, GUI_UI_ICON_HORIZONTAL_LINE_DARK_W, GUI_UI_ICON_HORIZONTAL_LINE_DARK_H, m_tRect.x1, m_tRect.y1, m_tRect.x2, 1);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_DARK_X, GUI_UI_ICON_VERTICAL_LINE_DARK_Y, GUI_UI_ICON_VERTICAL_LINE_DARK_W, GUI_UI_ICON_VERTICAL_LINE_DARK_H, m_tRect.x1, m_tRect.y1 + 1, 1, m_tRect.y2 - 1);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_X, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_Y, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_W, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_H, m_tRect.x1 + 1, m_tRect.y1 + m_tRect.y2 - 1, m_tRect.x2 - 1, 1);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_X, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_Y, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_W, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_H, m_tRect.x1 + m_tRect.x2 - 1, m_tRect.y1 + 1, 1, m_tRect.y2 - 2);
	if(hasSelection())
	{
		Uint32 selectionStart = UTIL_max<Uint32>(m_positionOfFirstShownLetter, m_selectionStart);
		Uint32 selectionLen = UTIL_min<Uint32>(m_positionOfFirstShownLetter + numberOfLetters, m_selectionEnd) - selectionStart;
		Uint32 selectionWidth = g_engine.calculateFontWidth(m_font, m_sVisibleText, selectionStart, selectionLen);
		Uint32 positionOfSelection = m_textStartPosition + g_engine.calculateFontWidth(m_font, m_sVisibleText, m_positionOfFirstShownLetter, selectionStart - m_positionOfFirstShownLetter);
		renderer->fillRectangle(m_tRect.x1 + positionOfSelection, m_tRect.y1 + 2, selectionWidth, m_tRect.y2 - 4, 128, 128, 128, 255);
	}
	g_engine.drawFont(m_font, m_tRect.x1 + m_textStartPosition, m_tRect.y1 + 3, m_sVisibleText, m_red, m_green, m_blue, CLIENT_FONT_ALIGN_LEFT, m_positionOfFirstShownLetter, numberOfLetters);
	if(m_bActive && m_bShowCursor)
	{
		Uint32 positionOfCursor = m_textStartPosition + g_engine.calculateFontWidth(m_font, m_sVisibleText, m_positionOfFirstShownLetter, m_cursorRelativePosition);
		renderer->fillRectangle(m_tRect.x1 + positionOfCursor, m_tRect.y1 + 2, 1, m_tRect.y2 - 4, 255, 255, 255, 255);
	}
}

Sint32 GUI_TextBox::addTextToTextBox(std::string text, Uint32 position)
{
	Uint32 remainLen = m_maxLength - SDL_static_cast(Uint32, m_sText.length());
	if(remainLen == 0)
		return 0;
	else
	{
		if(SDL_static_cast(Uint32, text.length()) > remainLen)
			text.resize(SDL_static_cast(size_t, remainLen));
	}

	Sint32 addedCharacters = 0;
	if(m_onlyNumbers)
	{
		for(size_t i = 0, len = text.length(); i < len; ++i)
		{
			char character = text[i];
			if(character >= 48 && character <= 57)
			{
				m_sText.insert(position, 1, character);
				++position;
				++addedCharacters;
			}
		}
	}
	else
	{
		for(size_t i = 0, len = text.length(); i < len; ++i)
		{
			char character = text[i];
			if(character >= 32)
			{
				m_sText.insert(position, 1, character);
				++position;
				++addedCharacters;
			}
		}
	}

	if(m_hideCharacter == '\0')
		m_sVisibleText.assign(m_sText);
	else
		m_sVisibleText.assign(m_sText.length(), m_hideCharacter);

	if(m_eventHandlerFunction)
		UTIL_SafeEventHandler(m_eventHandlerFunction, m_evtParam, 1);
	return addedCharacters;
}

void GUI_TextBox::removeTextToTextBox(Sint32 nbr, Uint32 position)
{
	m_sText.erase(position, nbr);
	m_sVisibleText.erase(position, nbr);
	if(m_eventHandlerFunction)
		UTIL_SafeEventHandler(m_eventHandlerFunction, m_evtParam, 2);
}

void GUI_TextBox::setText(const std::string text, bool updateCursor)
{
	if(m_onlyNumbers)
	{
		m_sText.clear();
		for(size_t i = 0, len = text.length(); i < len; ++i)
		{
			char character = text[i];
			if(character >= 48 && character <= 57)
				m_sText.append(1, character);
		}
	}
	else
	{
		m_sText.clear();
		for(size_t i = 0, len = text.length(); i < len; ++i)
		{
			char character = text[i];
			if(character >= 32)
				m_sText.append(1, character);
		}
	}

	if(SDL_static_cast(Uint32, text.length()) > m_maxLength)
		m_sText.resize(SDL_static_cast(size_t, m_maxLength));

	if(m_hideCharacter == '\0')
		m_sVisibleText.assign(m_sText);
	else
		m_sVisibleText.assign(m_sText.length(), m_hideCharacter);

    if(updateCursor)
    {
        m_cursorPosition = 0;
        m_cursorRelativePosition = 0;
    }
	if(m_eventHandlerFunction)
		UTIL_SafeEventHandler(m_eventHandlerFunction, m_evtParam, 0);
}

void GUI_TextBox::moveCursor(Sint32 position)
{
	if((m_cursorPosition + position >= 0) && (m_cursorPosition + position <= m_sText.length()))
	{
		m_cursorPosition += position;
		if(m_cursorRelativePosition + position < 0)
		{
			if(m_sText.length() <= m_cursorPosition)
			{
				m_positionOfFirstShownLetter = getBestPositionOfFirstShownLetter(m_cursorPosition);
				m_cursorRelativePosition = m_cursorPosition - m_positionOfFirstShownLetter;
			}
			else
			{
				m_positionOfFirstShownLetter += position;
				m_cursorRelativePosition = 0;
			}
		}
		else if(m_cursorRelativePosition + position > getNumberOfLettersToShow())
		{
			m_positionOfFirstShownLetter = getBestPositionOfFirstShownLetter(m_cursorPosition);
			m_cursorRelativePosition = m_cursorPosition - m_positionOfFirstShownLetter;
		}
		else
			m_cursorRelativePosition += position;
	}
	m_cursorTimer = g_frameTime;
	m_bShowCursor = true;
}

Uint32 GUI_TextBox::getNumberOfLettersToShow()
{
	Uint32 textLen = SDL_static_cast(Uint32, m_sText.length());
	Uint32 allowedCheckLen = (textLen < m_positionOfFirstShownLetter ? 0 : textLen - m_positionOfFirstShownLetter);
	Uint32 allowedTextWidth = m_tRect.x2 - m_textStartPosition;
	for(Uint32 i = 1; i <= allowedCheckLen; ++i)
	{
		Uint32 loopTextWidth = g_engine.calculateFontWidth(m_font, m_sVisibleText, m_positionOfFirstShownLetter, i);
		if(loopTextWidth >= allowedTextWidth)
			return i - 1;
	}
	return allowedCheckLen;
}

Uint32 GUI_TextBox::getBestPositionOfFirstShownLetter(Uint32 positionOfLetterToSee)
{
	if(positionOfLetterToSee > SDL_static_cast(Uint32, m_sText.length()))
		positionOfLetterToSee = SDL_static_cast(Uint32, m_sText.length());

	Uint32 allowedTextWidth = m_tRect.x2 - m_textStartPosition;
	for(Uint32 i = 0; i <= positionOfLetterToSee; ++i)
	{
		Uint32 len = positionOfLetterToSee - i;
		Uint32 loopTextWidth = g_engine.calculateFontWidth(m_font, m_sVisibleText, i, len);
		if(loopTextWidth < allowedTextWidth)
			return i;
	}
	return 0;
}

Uint32 GUI_TextBox::getNearestLetter(Sint32 x)
{
	Uint32 letters = getNumberOfLettersToShow();
	Sint32 lastPosition = 0;
	for(Uint32 i = 0; i <= letters; ++i)
	{
		Uint32 loopTextWidth = g_engine.calculateFontWidth(m_font, m_sVisibleText, m_positionOfFirstShownLetter, i);
		Sint32 position = m_tRect.x1 + m_textStartPosition + loopTextWidth;
		if(x - position < 0)
		{
			if(SDL_abs(x - position) < SDL_abs(x - lastPosition))
				return i;
			else
				return i - 1;
		}
		lastPosition = position;
	}
	return letters;
}
