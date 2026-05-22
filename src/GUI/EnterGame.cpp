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
#include "../GUI_Elements/GUI_CheckBox.h"
#include "../GUI_Elements/GUI_Separator.h"
#include "../GUI_Elements/GUI_TextBox.h"
#include "../GUI_Elements/GUI_Label.h"
#include "../protocolloginHttp.h"
#include "../game.h"
#include "../thingManager.h"
#include "../spriteManager.h"

#define ENTERGAME_TITLE "Enter Game"
#define ENTERGAME_WIDTH 236
#define ENTERGAME_HEIGHT 200
#define ENTERGAME_CANCEL_EVENTID 1000
#define ENTERGAME_OK_EVENTID 1001
#define ENTERGAME_REMEMBER_EVENTID 1003
#define ENTERGAME_ACCNAME_LABEL_TEXT1 "Account Name:"
#define ENTERGAME_ACCNAME_LABEL_TEXT2 "Account Number:"
#define ENTERGAME_ACCNAME_LABEL_TEXT3 "Email:"
#define ENTERGAME_ACCNAME_LABEL_X 18
#define ENTERGAME_ACCNAME_LABEL_Y 36
#define ENTERGAME_ACCNAME_TEXTBOX_X 132
#define ENTERGAME_ACCNAME_TEXTBOX_Y 32
#define ENTERGAME_ACCNAME_TEXTBOX_W 86
#define ENTERGAME_ACCNAME_TEXTBOX_H 16
#define ENTERGAME_ACCNAME_TEXTBOX_EVENTID 3000
#define ENTERGAME_PASSWORD_LABEL_TEXT "Password:"
#define ENTERGAME_PASSWORD_LABEL_X 18
#define ENTERGAME_PASSWORD_LABEL_Y 65
#define ENTERGAME_PASSWORD_TEXTBOX_X 132
#define ENTERGAME_PASSWORD_TEXTBOX_Y 61
#define ENTERGAME_PASSWORD_TEXTBOX_W 86
#define ENTERGAME_PASSWORD_TEXTBOX_H 16
#define ENTERGAME_PASSWORD_TEXTBOX_EVENTID 3001
#define ENTERGAME_ACCOUNT_LABEL_TEXT "If you don't have\nan account yet:"
#define ENTERGAME_ACCOUNT_LABEL_X 18
#define ENTERGAME_REMEMBER_TEXT "Save account and password"
#define ENTERGAME_REMEMBER_X 18
#define ENTERGAME_REMEMBER_Y 84
#define ENTERGAME_REMEMBER_W 200
#define ENTERGAME_REMEMBER_H 22
#define ENTERGAME_ACCOUNT_LABEL_Y 114
#define ENTERGAME_ACCOUNT_BUTTON_TEXT "Create Account"
#define ENTERGAME_ACCOUNT_BUTTON_LINK "https://secure.tibia.com/account/?subtopic=createaccount"
#define ENTERGAME_ACCOUNT_BUTTON_X 132
#define ENTERGAME_ACCOUNT_BUTTON_Y 118
#define ENTERGAME_ACCOUNT_BUTTON_W GUI_UI_BUTTON_86PX_GRAY_UP_W
#define ENTERGAME_ACCOUNT_BUTTON_H GUI_UI_BUTTON_86PX_GRAY_UP_H
#define ENTERGAME_ACCOUNT_BUTTON_EVENTID 1002

extern Engine g_engine;
extern Game g_game;
extern SpriteManager g_spriteManager;
extern ThingManager g_thingManager;
extern ProtocolLoginHttp g_protocolLoginHttp;

void enterGame_Events(Uint32 event, Sint32)
{
	switch(event)
	{
		case ENTERGAME_ACCOUNT_BUTTON_EVENTID: UTIL_OpenURL(ENTERGAME_ACCOUNT_BUTTON_LINK); break;
		case ENTERGAME_CANCEL_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_ENTERGAME)
				g_engine.removeWindow(pWindow);
		}
		break;
		case ENTERGAME_OK_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_ENTERGAME)
			{
				std::string accountName;
				std::string accountPassword;
				bool rememberCredentials = false;
				GUI_TextBox* pTextBox = SDL_static_cast(GUI_TextBox*, pWindow->getChild(ENTERGAME_ACCNAME_TEXTBOX_EVENTID));
				if(pTextBox)
					accountName = pTextBox->getActualText();
				pTextBox = SDL_static_cast(GUI_TextBox*, pWindow->getChild(ENTERGAME_PASSWORD_TEXTBOX_EVENTID));
				if(pTextBox)
					accountPassword = pTextBox->getActualText();
				GUI_CheckBox* pCheckBox = SDL_static_cast(GUI_CheckBox*, pWindow->getChild(ENTERGAME_REMEMBER_EVENTID));
				if(pCheckBox)
					rememberCredentials = pCheckBox->isChecked();

				g_engine.removeWindow(pWindow);
				if(!g_spriteManager.isSprLoaded())
				{
					if(g_game.hasGameFeature(GAME_FEATURE_NEWFILES_STRUCTURE))
					{
						#if CLIENT_OVVERIDE_VERSION > 0
						SDL_snprintf(g_buffer, sizeof(g_buffer), "%s%s%c%s", g_basePath.c_str(), ASSETS_CATALOG, PATH_PLATFORM_SLASH, CLIENT_ASSET_CATALOG);
						#else
						SDL_snprintf(g_buffer, sizeof(g_buffer), "%s%s%c%u%c%s", g_basePath.c_str(), ASSETS_CATALOG, PATH_PLATFORM_SLASH, g_clientVersion, PATH_PLATFORM_SLASH, CLIENT_ASSET_CATALOG);
						#endif
						g_sprPath = std::string(g_buffer);
						if(!g_spriteManager.loadCatalog(g_sprPath.c_str()))
						{
							SDL_snprintf(g_buffer, sizeof(g_buffer), "Cannot open file '%s'.\n\nPlease re-install the program.", g_sprPath.c_str());
							UTIL_messageBox("The Forgotten Client - Error", g_buffer);
							return;
						}
					}
					else
					{
						#if CLIENT_OVVERIDE_VERSION > 0
						SDL_snprintf(g_buffer, sizeof(g_buffer), "%s%s%c%s", g_basePath.c_str(), ASSETS_CATALOG, PATH_PLATFORM_SLASH, CLIENT_ASSET_SPR);
						#else
						SDL_snprintf(g_buffer, sizeof(g_buffer), "%s%s%c%u%c%s", g_basePath.c_str(), ASSETS_CATALOG, PATH_PLATFORM_SLASH, g_clientVersion, PATH_PLATFORM_SLASH, CLIENT_ASSET_SPR);
						#endif
						g_sprPath = std::string(g_buffer);
						if(!g_spriteManager.loadSprites(g_sprPath.c_str()))
						{
							SDL_snprintf(g_buffer, sizeof(g_buffer), "Cannot open file '%s'.\n\nPlease re-install the program.", g_sprPath.c_str());
							UTIL_messageBox("The Forgotten Client - Error", g_buffer);
							return;
						}
					}
					g_engine.getRender()->spriteManagerReset();
				}
				if(!g_thingManager.isDatLoaded())
				{
					#if CLIENT_OVVERIDE_VERSION > 0
					Uint32 oldClient = g_clientVersion;
					g_clientVersion = CLIENT_OVERRIDE_FILE_VERSION;
					#endif
					if(g_game.hasGameFeature(GAME_FEATURE_NEWFILES_STRUCTURE))
					{
						//We should have appearance file name inside g_datPath from parsing catalog in spritemanager
						#if CLIENT_OVVERIDE_VERSION > 0
						SDL_snprintf(g_buffer, sizeof(g_buffer), "%s%s%c%s", g_basePath.c_str(), ASSETS_CATALOG, PATH_PLATFORM_SLASH, g_datPath.c_str());
						#else
						SDL_snprintf(g_buffer, sizeof(g_buffer), "%s%s%c%u%c%s", g_basePath.c_str(), ASSETS_CATALOG, PATH_PLATFORM_SLASH, g_clientVersion, PATH_PLATFORM_SLASH, g_datPath.c_str());
						#endif
						g_datPath = std::string(g_buffer);
						if(!g_thingManager.loadAppearances(g_datPath.c_str()))
						{
							SDL_snprintf(g_buffer, sizeof(g_buffer), "Cannot open file '%s'.\n\nPlease re-install the program.", g_datPath.c_str());
							UTIL_messageBox("The Forgotten Client - Error", g_buffer);
							return;
						}
					}
					else
					{
						#if CLIENT_OVVERIDE_VERSION > 0
						SDL_snprintf(g_buffer, sizeof(g_buffer), "%s%s%c%s", g_basePath.c_str(), ASSETS_CATALOG, PATH_PLATFORM_SLASH, CLIENT_ASSET_DAT);
						#else
						SDL_snprintf(g_buffer, sizeof(g_buffer), "%s%s%c%u%c%s", g_basePath.c_str(), ASSETS_CATALOG, PATH_PLATFORM_SLASH, g_clientVersion, PATH_PLATFORM_SLASH, CLIENT_ASSET_DAT);
						#endif
						g_datPath = std::string(g_buffer);
						if(!g_thingManager.loadDat(g_datPath.c_str()))
						{
							SDL_snprintf(g_buffer, sizeof(g_buffer), "Cannot open file '%s'.\n\nPlease re-install the program.", g_datPath.c_str());
							UTIL_messageBox("The Forgotten Client - Error", g_buffer);
							return;
						}
					}
					#if CLIENT_OVVERIDE_VERSION > 0
					g_clientVersion = oldClient;
					#endif
				}

				g_engine.setAccountName(accountName);
				g_engine.setAccountPassword(accountPassword);
				g_engine.setRememberAccountCredentials(rememberCredentials);

				UTIL_messageBox("Connecting", "Your character list is being loaded. Please wait.");
				#if CLIENT_OVVERIDE_VERSION == 0
				if(g_engine.getClientHost().find("http") != std::string::npos)
				#else
				if(std::string(CLIENT_OVERRIDE_LOGIN_HOST).find("http") != std::string::npos)
				#endif
					g_protocolLoginHttp.initializeConnection();
				else
					g_engine.issueNewConnection(false);
			}
		}
		break;
		default: break;
	}
}

void UTIL_createEnterGame()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_ENTERGAME);
	if(pWindow)
		return;

	GUI_Window* newWindow = new GUI_Window(iRect(0, 0, ENTERGAME_WIDTH, ENTERGAME_HEIGHT), ENTERGAME_TITLE, GUI_WINDOW_ENTERGAME);
	GUI_Label* newLabel = new GUI_Label(iRect(ENTERGAME_ACCNAME_LABEL_X, ENTERGAME_ACCNAME_LABEL_Y, 0, 0), (g_game.hasGameFeature(GAME_FEATURE_ACCOUNT_EMAIL) ? ENTERGAME_ACCNAME_LABEL_TEXT3 : g_game.hasGameFeature(GAME_FEATURE_ACCOUNT_NAME) ? ENTERGAME_ACCNAME_LABEL_TEXT1 : ENTERGAME_ACCNAME_LABEL_TEXT2));
	newWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(ENTERGAME_PASSWORD_LABEL_X, ENTERGAME_PASSWORD_LABEL_Y, 0, 0), ENTERGAME_PASSWORD_LABEL_TEXT);
	newWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(ENTERGAME_ACCOUNT_LABEL_X, ENTERGAME_ACCOUNT_LABEL_Y, 0, 0), ENTERGAME_ACCOUNT_LABEL_TEXT);
	newWindow->addChild(newLabel);
	GUI_TextBox* newTextBox = new GUI_TextBox(iRect(ENTERGAME_ACCNAME_TEXTBOX_X, ENTERGAME_ACCNAME_TEXTBOX_Y, ENTERGAME_ACCNAME_TEXTBOX_W, ENTERGAME_ACCNAME_TEXTBOX_H), (g_engine.getRememberAccountCredentials() ? g_engine.getAccountName() : ""), ENTERGAME_ACCNAME_TEXTBOX_EVENTID);
	if(g_game.hasGameFeature(GAME_FEATURE_ACCOUNT_EMAIL))
		newTextBox->setMaxLength(80);
	else if(g_game.hasGameFeature(GAME_FEATURE_ACCOUNT_NAME))
		newTextBox->setMaxLength(30);
	else
	{
		newTextBox->setMaxLength(10);
		newTextBox->setOnlyNumbers(true);
	}
	newTextBox->startEvents();
	newWindow->addChild(newTextBox);
	newTextBox = new GUI_TextBox(iRect(ENTERGAME_PASSWORD_TEXTBOX_X, ENTERGAME_PASSWORD_TEXTBOX_Y, ENTERGAME_PASSWORD_TEXTBOX_W, ENTERGAME_PASSWORD_TEXTBOX_H), (g_engine.getRememberAccountCredentials() ? g_engine.getAccountPassword() : ""), ENTERGAME_PASSWORD_TEXTBOX_EVENTID);
	newTextBox->setMaxLength(30);
	newTextBox->setHideCharacter('*');
	newTextBox->startEvents();
	newWindow->addChild(newTextBox);
	GUI_CheckBox* newCheckBox = new GUI_CheckBox(iRect(ENTERGAME_REMEMBER_X, ENTERGAME_REMEMBER_Y, ENTERGAME_REMEMBER_W, ENTERGAME_REMEMBER_H), ENTERGAME_REMEMBER_TEXT, g_engine.getRememberAccountCredentials(), ENTERGAME_REMEMBER_EVENTID);
	newCheckBox->startEvents();
	newWindow->addChild(newCheckBox);
	GUI_Button* newButton = new GUI_Button(iRect(ENTERGAME_ACCOUNT_BUTTON_X, ENTERGAME_ACCOUNT_BUTTON_Y, ENTERGAME_ACCOUNT_BUTTON_W, ENTERGAME_ACCOUNT_BUTTON_H), ENTERGAME_ACCOUNT_BUTTON_TEXT);
	newButton->setButtonEventCallback(&enterGame_Events, ENTERGAME_ACCOUNT_BUTTON_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(ENTERGAME_WIDTH - 56, ENTERGAME_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Cancel", CLIENT_GUI_ESCAPE_TRIGGER);
	newButton->setButtonEventCallback(&enterGame_Events, ENTERGAME_CANCEL_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(ENTERGAME_WIDTH - 109, ENTERGAME_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Ok", CLIENT_GUI_ENTER_TRIGGER);
	newButton->setButtonEventCallback(&enterGame_Events, ENTERGAME_OK_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	GUI_Separator* newSeparator = new GUI_Separator(iRect(13, ENTERGAME_HEIGHT - 40, ENTERGAME_WIDTH - 26, 2));
	newWindow->addChild(newSeparator);
	g_engine.addWindow(newWindow);
}
