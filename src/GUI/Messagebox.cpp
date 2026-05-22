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
#include "../GUI_Elements/GUI_Label.h"
#include "../GUI_Elements/GUI_Log.h"
#include "../protocolloginHttp.h"

#include <cctype>

#define MESSAGEBOX_OK_EVENTID 1000

extern Engine g_engine;
extern GUI_Log g_logger;
extern ProtocolLoginHttp g_protocolLoginHttp;

namespace
{
	std::string toLowerCopy(const std::string& text)
	{
		std::string result = text;
		std::transform(result.begin(), result.end(), result.begin(), [](unsigned char ch) -> char
		{
			return SDL_static_cast(char, std::tolower(ch));
		});
		return result;
	}

	bool containsText(const std::string& text, const char* needle)
	{
		return text.find(needle) != std::string::npos;
	}

	bool shouldSendToTerminal(const std::string& title, const std::string& message)
	{
		const std::string titleLower = toLowerCopy(title);
		const std::string messageLower = toLowerCopy(message);

		return containsText(titleLower, "error") ||
			containsText(titleLower, "failed") ||
			containsText(titleLower, "sorry") ||
			containsText(titleLower, "two-factor") ||
			containsText(messageLower, "error:") ||
			containsText(messageLower, "failed") ||
			containsText(messageLower, "cannot connect") ||
			containsText(messageLower, "cannot open") ||
			containsText(messageLower, "unsupported packet header") ||
			containsText(messageLower, "protocol mismatched") ||
			containsText(messageLower, "client needs update") ||
			containsText(messageLower, "invalid authentification");
	}

	std::string buildTerminalError(const std::string& title, const std::string& message)
	{
		std::ostringstream output;
		output << title << '\n';

		StringVector lines = UTIL_explodeString(message, "\n");
		if(lines.size() <= 1)
			output << "Message: " << message;
		else
		{
			output << "Message:";
			for(size_t i = 0, end = lines.size(); i < end; ++i)
			{
				if(!lines[i].empty())
					output << "\n  " << lines[i];
			}
		}

		const std::string fullLower = toLowerCopy(title + "\n" + message);
		if(containsText(fullLower, "unsupported packet header"))
		{
			output << "\nDetails: The server sent a packet opcode that this client did not decode for the current protocol/features.";
			output << "\nCheck: protocol version, features.lua, and custom server packets.";
		}
		else if(containsText(fullLower, "protocol mismatched"))
		{
			output << "\nDetails: The server and client protocol/features are not aligned.";
			output << "\nCheck: selected protocol, server version, RSA/XTEA/login mode, and enabled features.";
		}
		else if(containsText(fullLower, "cannot connect") ||
			containsText(fullLower, "connection failed") ||
			containsText(fullLower, "connection refused") ||
			containsText(fullLower, "connection timed out") ||
			containsText(fullLower, "failed to call recv") ||
			containsText(fullLower, "failed to call send") ||
			containsText(fullLower, "cannot resolve host"))
		{
			output << "\nDetails: The client could not complete the connection with the login/game server.";
			output << "\nCheck: host, port, firewall, server online status, and matching protocol.";
		}
		else if(containsText(fullLower, "cannot open file"))
		{
			output << "\nDetails: A required client data file could not be loaded.";
			output << "\nCheck: dat/spr/pic or catalog path for the selected client version.";
		}
		else if(containsText(fullLower, "client needs update"))
		{
			output << "\nDetails: The server requested a newer/different client data version.";
			output << "\nCheck: client version and asset files.";
		}

		output << "\nShortcut: press Ctrl+T to hide or reopen this terminal.";
		return output.str();
	}
}

void messagebox_Events(Uint32 event, Sint32)
{
	switch(event)
	{
		case MESSAGEBOX_OK_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_MESSAGEBOX)
			{
				g_engine.removeWindow(pWindow);
				if(!g_engine.isIngame())
				{
					g_engine.releaseConnection();
					g_protocolLoginHttp.closeConnection();
				}
			}
		}
		break;
		default: break;
	}
}

void UTIL_messageBox(const std::string& title, const std::string& message)
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_MESSAGEBOX);
	if(pWindow)
		g_engine.removeWindow(pWindow);

	if(shouldSendToTerminal(title, message))
	{
		g_logger.addLog(LOG_CATEGORY_ERROR, buildTerminalError(title, message));
		g_engine.showLogger(true);
		if(!g_engine.isIngame())
		{
			g_engine.releaseConnection();
			g_protocolLoginHttp.closeConnection();
		}
		return;
	}

	Uint32 cacheMSGsize = 274, cacheMSGsize2 = 91;
	StringVector messages = UTIL_explodeString(message, "\n");
	for(size_t i = 0, end = messages.size(); i < end; ++i)
	{
		Uint32 cachedMSGsize = g_engine.calculateFontWidth(CLIENT_FONT_NONOUTLINED, messages[i]);
		if(cachedMSGsize > cacheMSGsize)
			cacheMSGsize = cachedMSGsize;
		cacheMSGsize2 += 14;
	}

	GUI_Window* newWindow = new GUI_Window(iRect(0, 0, cacheMSGsize + 36, cacheMSGsize2), title, GUI_WINDOW_MESSAGEBOX);
	GUI_Label* newLabel;
	Sint32 labelY = 34;
	for(size_t i = 0, end = messages.size(); i < end; ++i)
	{
		newLabel = new GUI_Label(iRect(18, labelY, 43, 20), messages[i]);
		newWindow->addChild(newLabel);
		labelY += 14;
	}
	GUI_Button* newButton = new GUI_Button(iRect(cacheMSGsize - 20, cacheMSGsize2 - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Ok", CLIENT_GUI_ESCAPE_TRIGGER);
	newButton->setButtonEventCallback(&messagebox_Events, MESSAGEBOX_OK_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(cacheMSGsize - 20, cacheMSGsize2 - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Ok", CLIENT_GUI_ENTER_TRIGGER);
	newButton->setButtonEventCallback(&messagebox_Events, MESSAGEBOX_OK_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	GUI_Separator* newSeparator = new GUI_Separator(iRect(13, cacheMSGsize2 - 40, cacheMSGsize + 10, 2));
	newWindow->addChild(newSeparator);
	g_engine.addWindow(newWindow);
}
