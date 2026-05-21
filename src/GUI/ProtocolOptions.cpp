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
#include "../GUI_Elements/GUI_MultiTextBox.h"
#include "../GUI_Elements/GUI_TextBox.h"
#include "../GUI_Elements/GUI_ListBox.h"
#include "../GUI_Elements/GUI_Label.h"
#include "../game.h"
#include "../thingManager.h"
#include "../spriteManager.h"

#define PROTOCOL_TITLE "Protocol Options"
#define PROTOCOL_WIDTH 236
#define PROTOCOL_HEIGHT 448
#define PROTOCOL_CANCEL_EVENTID 1000
#define PROTOCOL_OK_EVENTID 1001
#define PROTOCOL_FIRST_LABEL_TEXT "Server Address:"
#define PROTOCOL_FIRST_LABEL_X 18
#define PROTOCOL_FIRST_LABEL_Y 32
#define PROTOCOL_SECOND_LABEL_TEXT "Server Port:"
#define PROTOCOL_SECOND_LABEL_X 18
#define PROTOCOL_SECOND_LABEL_Y 70
#define PROTOCOL_THIRD_LABEL_TEXT "Socks Proxy:"
#define PROTOCOL_THIRD_LABEL_X 18
#define PROTOCOL_THIRD_LABEL_Y 108
#define PROTOCOL_FOURTH_LABEL_TEXT "Proxy Authentication:"
#define PROTOCOL_FOURTH_LABEL_X 18
#define PROTOCOL_FOURTH_LABEL_Y 146
#define PROTOCOL_FIFTH_LABEL_TEXT "RSA Key:"
#define PROTOCOL_FIFTH_LABEL_X 18
#define PROTOCOL_FIFTH_LABEL_Y 184
#define PROTOCOL_SIXTH_LABEL_TEXT "Choose Client Version:"
#define PROTOCOL_SIXTH_LABEL_X 18
#define PROTOCOL_SIXTH_LABEL_Y 294
#define PROTOCOL_FIRST_TEXTBOX_EVENTID 3000
#define PROTOCOL_FIRST_TEXTBOX_X 18
#define PROTOCOL_FIRST_TEXTBOX_Y 48
#define PROTOCOL_FIRST_TEXTBOX_W 200
#define PROTOCOL_FIRST_TEXTBOX_H 16
#define PROTOCOL_SECOND_TEXTBOX_EVENTID 3001
#define PROTOCOL_SECOND_TEXTBOX_X 18
#define PROTOCOL_SECOND_TEXTBOX_Y 86
#define PROTOCOL_SECOND_TEXTBOX_W 200
#define PROTOCOL_SECOND_TEXTBOX_H 16
#define PROTOCOL_THIRD_TEXTBOX_EVENTID 3002
#define PROTOCOL_THIRD_TEXTBOX_X 18
#define PROTOCOL_THIRD_TEXTBOX_Y 124
#define PROTOCOL_THIRD_TEXTBOX_W 200
#define PROTOCOL_THIRD_TEXTBOX_H 16
#define PROTOCOL_FOURTH_TEXTBOX_EVENTID 3003
#define PROTOCOL_FOURTH_TEXTBOX_X 18
#define PROTOCOL_FOURTH_TEXTBOX_Y 162
#define PROTOCOL_FOURTH_TEXTBOX_W 200
#define PROTOCOL_FOURTH_TEXTBOX_H 16
#define PROTOCOL_FIFTH_TEXTBOX_EVENTID 3004
#define PROTOCOL_FIFTH_TEXTBOX_X 18
#define PROTOCOL_FIFTH_TEXTBOX_Y 200
#define PROTOCOL_FIFTH_TEXTBOX_W 200
#define PROTOCOL_FIFTH_TEXTBOX_H 78
#define PROTOCOL_CLIENT_LISTBOX_EVENTID 1002
#define PROTOCOL_CLIENT_LISTBOX_X 18
#define PROTOCOL_CLIENT_LISTBOX_Y 310
#define PROTOCOL_CLIENT_LISTBOX_W 200
#define PROTOCOL_CLIENT_LISTBOX_H 78
#define PROTOCOL_CLIENT_VERSIONS "713:720:721;723:724:726;727:730:740:741:750:755:760:770:771:772:780:781:782:790:792:800:810:811:820:821:822:823:830:831:840:841:842:850:851:852:853:854:855:856:857:860:861:862:870:871:872:873:874:900:910:920:931:940:941:942:943:944:945:946:950:951:952:953:954:960:961:962:963:970:971:972:973:980:981:982:983:984:985:986:1000:1001:1002:1010:1011:1012:1013:1020:1021:1022:1030:1031:1032:1033:1034:1035:1036:1037:1038:1039:1040:1041:1050:1051:1052:1053:1054:1055:1056:1057:1058:1059:1060:1061:1062:1063:1064:1070:1071:1072:1073:1074:1075:1076:1077:1078:1079:1080:1081:1082:1090:1091:1092:1093:1094:1095:1096:1097:1098:1099:1100;1101:1102:1103:1104:1110:1120;1121:1130:1132:1134:1135:1140:1141:1142:1143:1144:1145:1146:1147:1148:1149:1150:1151:1152:1153:1154:1155:1156:1157:1158:1159:1160:1165:1166:1170:1171:1172:1173:1174:1175:1180:1181:1182:1183:1185:1186:1187:1190:1191:1192:1193:1194:1195:1200:1201:1202:1203:1205:1206:1207:1208:1209:1210:1211:1212:1215:1220:1230:1231:1240:1250"

extern Engine g_engine;
extern Game g_game;
extern SpriteManager g_spriteManager;
extern ThingManager g_thingManager;

std::map<Uint32, Uint32> g_protocols;

void protocoloptions_Events(Uint32 event, Sint32)
{
	switch(event)
	{
		case PROTOCOL_CANCEL_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_PROTOCOLOPTIONS)
			{
				g_protocols.clear();
				g_engine.removeWindow(pWindow);
			}
		}
		break;
		case PROTOCOL_OK_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_PROTOCOLOPTIONS)
			{
				if(!g_engine.isIngame())
				{
					GUI_TextBox* pTextBox = SDL_static_cast(GUI_TextBox*, pWindow->getChild(PROTOCOL_FIRST_TEXTBOX_EVENTID));
					if(pTextBox)
						g_engine.setClientHost(pTextBox->getActualText());

					pTextBox = SDL_static_cast(GUI_TextBox*, pWindow->getChild(PROTOCOL_SECOND_TEXTBOX_EVENTID));
					if(pTextBox)
						g_engine.setClientPort(pTextBox->getActualText());

					pTextBox = SDL_static_cast(GUI_TextBox*, pWindow->getChild(PROTOCOL_THIRD_TEXTBOX_EVENTID));
					if(pTextBox)
						g_engine.setClientProxy(pTextBox->getActualText());

					pTextBox = SDL_static_cast(GUI_TextBox*, pWindow->getChild(PROTOCOL_FOURTH_TEXTBOX_EVENTID));
					if(pTextBox)
						g_engine.setClientProxyAuth(pTextBox->getActualText());

					GUI_ListBox* pListBox = SDL_static_cast(GUI_ListBox*, pWindow->getChild(PROTOCOL_CLIENT_LISTBOX_EVENTID));
					if(pListBox)
					{
						std::map<Uint32, Uint32>::iterator it = g_protocols.find(pListBox->getSelect());
						if(it != g_protocols.end())
						{
							g_clientVersion = it->second;
							g_game.clientChangeVersion(g_clientVersion, g_clientVersion);
						}
					}

					if(g_spriteManager.isSprLoaded())
						g_spriteManager.unloadSprites();

					if(g_thingManager.isDatLoaded())
						g_thingManager.unloadDat();
				}
				g_protocols.clear();
				g_engine.removeWindow(pWindow);
			}
		}
		break;
		default: break;
	}
}

void UTIL_protocolOptions()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_PROTOCOLOPTIONS);
	if(pWindow)
		return;

	Uint32 count = 0;
	std::string data = std::string(PROTOCOL_CLIENT_VERSIONS, sizeof(PROTOCOL_CLIENT_VERSIONS) - 1);
	StringVector versions = UTIL_explodeString(data, ":");
	for(StringVector::iterator it = versions.begin(), end = versions.end(); it != end; ++it)
		g_protocols[count++] = SDL_static_cast(Uint32, SDL_strtoul((*it).c_str(), NULL, 10));

	GUI_Window* newWindow = new GUI_Window(iRect(0, 0, PROTOCOL_WIDTH, PROTOCOL_HEIGHT), PROTOCOL_TITLE, GUI_WINDOW_PROTOCOLOPTIONS);
	GUI_Label* newLabel = new GUI_Label(iRect(PROTOCOL_FIRST_LABEL_X, PROTOCOL_FIRST_LABEL_Y, 0, 0), PROTOCOL_FIRST_LABEL_TEXT);
	newWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(PROTOCOL_SECOND_LABEL_X, PROTOCOL_SECOND_LABEL_Y, 0, 0), PROTOCOL_SECOND_LABEL_TEXT);
	newWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(PROTOCOL_THIRD_LABEL_X, PROTOCOL_THIRD_LABEL_Y, 0, 0), PROTOCOL_THIRD_LABEL_TEXT);
	newWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(PROTOCOL_FOURTH_LABEL_X, PROTOCOL_FOURTH_LABEL_Y, 0, 0), PROTOCOL_FOURTH_LABEL_TEXT);
	newWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(PROTOCOL_FIFTH_LABEL_X, PROTOCOL_FIFTH_LABEL_Y, 0, 0), PROTOCOL_FIFTH_LABEL_TEXT);
	newWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(PROTOCOL_SIXTH_LABEL_X, PROTOCOL_SIXTH_LABEL_Y, 0, 0), PROTOCOL_SIXTH_LABEL_TEXT);
	newWindow->addChild(newLabel);
	GUI_TextBox* newTextBox = new GUI_TextBox(iRect(PROTOCOL_FIRST_TEXTBOX_X, PROTOCOL_FIRST_TEXTBOX_Y, PROTOCOL_FIRST_TEXTBOX_W, PROTOCOL_FIRST_TEXTBOX_H), g_engine.getClientHost(), PROTOCOL_FIRST_TEXTBOX_EVENTID);
	newTextBox->setMaxLength(100);
	newTextBox->startEvents();
	newWindow->addChild(newTextBox);
	newTextBox = new GUI_TextBox(iRect(PROTOCOL_SECOND_TEXTBOX_X, PROTOCOL_SECOND_TEXTBOX_Y, PROTOCOL_SECOND_TEXTBOX_W, PROTOCOL_SECOND_TEXTBOX_H), g_engine.getClientPort(), PROTOCOL_SECOND_TEXTBOX_EVENTID);
	newTextBox->setMaxLength(5);
	newTextBox->setOnlyNumbers(true);
	newTextBox->startEvents();
	newWindow->addChild(newTextBox);
	newTextBox = new GUI_TextBox(iRect(PROTOCOL_THIRD_TEXTBOX_X, PROTOCOL_THIRD_TEXTBOX_Y, PROTOCOL_THIRD_TEXTBOX_W, PROTOCOL_THIRD_TEXTBOX_H), g_engine.getClientProxy(), PROTOCOL_THIRD_TEXTBOX_EVENTID);
	newTextBox->setMaxLength(110);
	newTextBox->startEvents();
	newWindow->addChild(newTextBox);
	newTextBox = new GUI_TextBox(iRect(PROTOCOL_FOURTH_TEXTBOX_X, PROTOCOL_FOURTH_TEXTBOX_Y, PROTOCOL_FOURTH_TEXTBOX_W, PROTOCOL_FOURTH_TEXTBOX_H), g_engine.getClientProxyAuth(), PROTOCOL_FOURTH_TEXTBOX_EVENTID);
	newTextBox->setMaxLength(100);
	newTextBox->startEvents();
	newWindow->addChild(newTextBox);
	GUI_MultiTextBox* newMultiTextBox = new GUI_MultiTextBox(iRect(PROTOCOL_FIFTH_TEXTBOX_X, PROTOCOL_FIFTH_TEXTBOX_Y, PROTOCOL_FIFTH_TEXTBOX_W, PROTOCOL_FIFTH_TEXTBOX_H), false, CLIENT_RSA_KEY, PROTOCOL_FIFTH_TEXTBOX_EVENTID);
	newMultiTextBox->setMaxLength(320);
	newMultiTextBox->startEvents();
	newWindow->addChild(newMultiTextBox);
	GUI_ListBox* newListBox = new GUI_ListBox(iRect(PROTOCOL_CLIENT_LISTBOX_X, PROTOCOL_CLIENT_LISTBOX_Y, PROTOCOL_CLIENT_LISTBOX_W, PROTOCOL_CLIENT_LISTBOX_H), PROTOCOL_CLIENT_LISTBOX_EVENTID);
	Sint32 currentVersion = -1, i = 0;
	for(std::map<Uint32, Uint32>::iterator it = g_protocols.begin(), end = g_protocols.end(); it != end; ++it)
	{
		Uint32 version = it->second;
		Uint32 versionUpper = version / 100;
		Uint32 versionLower = version % 100;
		Sint32 len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%u.%02u", versionUpper, versionLower);
		newListBox->add(std::string(g_buffer, SDL_static_cast(size_t, len)));
		if(g_clientVersion == version)
			currentVersion = i;
		++i;
	}
	if(currentVersion == -1)
		currentVersion = --i;
	newListBox->setSelect(currentVersion);
	newListBox->setEventCallback(&protocoloptions_Events, PROTOCOL_CLIENT_LISTBOX_EVENTID);
	newListBox->startEvents();
	newWindow->addChild(newListBox);
	GUI_Separator* newSeparator = new GUI_Separator(iRect(13, PROTOCOL_HEIGHT - 40, PROTOCOL_WIDTH - 26, 2));
	newWindow->addChild(newSeparator);
	GUI_Button* newButton = new GUI_Button(iRect(PROTOCOL_WIDTH - 109, PROTOCOL_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Ok", CLIENT_GUI_ENTER_TRIGGER);
	newButton->setButtonEventCallback(&protocoloptions_Events, PROTOCOL_OK_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	newButton = new GUI_Button(iRect(PROTOCOL_WIDTH - 56, PROTOCOL_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Cancel", CLIENT_GUI_ESCAPE_TRIGGER);
	newButton->setButtonEventCallback(&protocoloptions_Events, PROTOCOL_CANCEL_EVENTID);
	newButton->startEvents();
	newWindow->addChild(newButton);
	g_engine.addWindow(newWindow);
}
