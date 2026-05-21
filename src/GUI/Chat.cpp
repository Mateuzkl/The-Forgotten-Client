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

//TODO: channel buttons moving

#include "Chat.h"
#include "../GUI_Elements/GUI_Window.h"
#include "../GUI_Elements/GUI_Label.h"
#include "../GUI_Elements/GUI_Button.h"
#include "../GUI_Elements/GUI_Separator.h"
#include "../GUI_Elements/GUI_TextBox.h"
#include "../GUI_Elements/GUI_Console.h"
#include "../GUI_Elements/GUI_ContextMenu.h"
#include "../creature.h"
#include "../engine.h"
#include "../game.h"

Chat g_chat;
extern Engine g_engine;
extern Game g_game;
extern Uint32 g_frameTime;
extern Sint32 g_actualCursor;

ConsoleMessage g_lastMessage;

#define CHAT_COPY_EVENTID 1000
#define CHAT_COPYNAME_EVENTID 1001
#define CHAT_COPYMESSAGE_EVENTID 1002
#define CHAT_SELECTALL_EVENTID 1003
#define CHAT_MESSAGETO_EVENTID 1004
#define CHAT_INVITETO_EVENTID 1005
#define CHAT_EXCLUDEFROM_EVENTID 1006
#define CHAT_ADDTOVIP_EVENTID 1007
#define CHAT_IGNORE_EVENTID 1008
#define CHAT_RULEVIOLATION_EVENTID 1009

#define CHAT_TEXT_CUT_EVENTID 2000
#define CHAT_TEXT_COPY_EVENTID 2001
#define CHAT_TEXT_PASTE_EVENTID 2002
#define CHAT_TEXT_DELETE_EVENTID 2003
#define CHAT_TEXT_SELECTALL_EVENTID 2004

#define CHAT_CHANNEL_INVITE_EVENTID 3000
#define CHAT_CHANNEL_EXCLUDE_EVENTID 3001
#define CHAT_CHANNEL_CLOSE_EVENTID 3002
#define CHAT_CHANNEL_SERVERMESSAGES_EVENTID 3003
#define CHAT_CHANNEL_SAVEWINDOW_EVENTID 3004
#define CHAT_CHANNEL_CLEARWINDOW_EVENTID 3005

#define INVITE_PRIVATECHAT_TITLE "Invite player to private chat channel"
#define INVITE_PRIVATECHAT_WIDTH 262
#define INVITE_PRIVATECHAT_HEIGHT 124
#define INVITE_PRIVATECHAT_CANCEL_EVENTID 1500
#define INVITE_PRIVATECHAT_ADD_EVENTID 1501
#define INVITE_PRIVATECHAT_LABEL_TEXT "Please enter a character name:"
#define INVITE_PRIVATECHAT_LABEL_X 18
#define INVITE_PRIVATECHAT_LABEL_Y 34
#define INVITE_PRIVATECHAT_TEXTBOX_X 18
#define INVITE_PRIVATECHAT_TEXTBOX_Y 47
#define INVITE_PRIVATECHAT_TEXTBOX_W 225
#define INVITE_PRIVATECHAT_TEXTBOX_H 16
#define INVITE_PRIVATECHAT_TEXTBOX_EVENTID 3000

#define EXCLUDE_PRIVATECHAT_TITLE "Exclude player from private chat channel"
#define EXCLUDE_PRIVATECHAT_WIDTH 262
#define EXCLUDE_PRIVATECHAT_HEIGHT 124
#define EXCLUDE_PRIVATECHAT_CANCEL_EVENTID 2500
#define EXCLUDE_PRIVATECHAT_ADD_EVENTID 2501
#define EXCLUDE_PRIVATECHAT_LABEL_TEXT "Please enter a character name:"
#define EXCLUDE_PRIVATECHAT_LABEL_X 18
#define EXCLUDE_PRIVATECHAT_LABEL_Y 34
#define EXCLUDE_PRIVATECHAT_TEXTBOX_X 18
#define EXCLUDE_PRIVATECHAT_TEXTBOX_Y 47
#define EXCLUDE_PRIVATECHAT_TEXTBOX_W 225
#define EXCLUDE_PRIVATECHAT_TEXTBOX_H 16
#define EXCLUDE_PRIVATECHAT_TEXTBOX_EVENTID 3000

#define CHANNEL_ID_PRIVATE_START 0x10000
#define CHANNEL_ID_PRIVATE_END 0x12710

#define CHANNEL_MESSAGE_HELP_LEGACY "Welcome to the help channel! Feel free to ask questions concerning client controls, general game play, use of accounts and the official homepage. In-depth questions about the content of the game will not be answered. Experienced players will be glad to help you to the best of their knowledge. Keep in mind that this is not a chat channel for general conversations. Therefore please limit your statements to relevant questions and answers."
#define CHANNEL_MESSAGE_HELP "Welcome to the help channel.\nIn this channel you can ask questions about Tibia. Experienced players will gladly help you to the best of their knowledge.\nFor detailed information about quests and other game content please take a look at our supported fansites at http://www.tibia.com/community/?subtopic=fansites\nPlease remember to write in English here so everybody can understand you."
#define CHANNEL_MESSAGE_ADVERTISING "Here you can advertise all kinds of things. Among others, you can trade Tibia items, advertise ingame events, seek characters for a quest or a hunting group, find members for your guild or look for somebody to help you with something.\nIt goes without saying that all advertisements must be conform to the Tibia Rules.Keep in mind that it is illegal to advertise trades including premium time, real money or Tibia characters."
#define CHANNEL_MESSAGE_CLOSED "The channel has been closed. You need to re-join the channel if you get invited."

Channel::Channel(std::string channelName, Uint32 channelId, bool closable, bool privateChannel) :
	channelName(std::move(channelName)), channelId(channelId), channelClosable(closable), privateChannel(privateChannel)
{
	channelConsole = new GUI_Console(iRect(0, 0, 0, 0));
}

void chat_Events(Uint32 event, Sint32)
{
	switch(event)
	{
		case CHAT_COPY_EVENTID:
		{
			Channel* selectedchannel = g_chat.getCurrentChannel();
			if(selectedchannel)
			{
				SDL_Event keyEvent;
				keyEvent.key.keysym.sym = SDLK_c;
				keyEvent.key.keysym.mod = KMOD_CTRL;
				selectedchannel->channelConsole->onKeyDown(keyEvent);
				//Simulating CTRL+C shortcut should do the job
			}
		}
		break;
		case CHAT_COPYNAME_EVENTID:
		{
			if(!g_lastMessage.name.empty())
				UTIL_SetClipboardTextLatin1(g_lastMessage.name.c_str());
		}
		break;
		case CHAT_COPYMESSAGE_EVENTID:
		{
			Sint32 len;
			if(g_engine.hasShowTimestamps())
			{
				if(!g_lastMessage.name.empty())
				{
					if(g_engine.hasShowLevels() && g_lastMessage.level > 0)
						len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s %s[%u]: %s", g_lastMessage.timestamp.c_str(), g_lastMessage.name.c_str(), SDL_static_cast(Uint32, g_lastMessage.level), g_lastMessage.message.c_str());
					else
						len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s %s: %s", g_lastMessage.timestamp.c_str(), g_lastMessage.name.c_str(), g_lastMessage.message.c_str());
				}
				else
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s %s", g_lastMessage.timestamp.c_str(), g_lastMessage.message.c_str());
			}
			else
			{
				if(!g_lastMessage.name.empty())
				{
					if(g_engine.hasShowLevels() && g_lastMessage.level > 0)
						len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s[%u]: %s", g_lastMessage.name.c_str(), SDL_static_cast(Uint32, g_lastMessage.level), g_lastMessage.message.c_str());
					else
						len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s: %s", g_lastMessage.name.c_str(), g_lastMessage.message.c_str());
				}
				else
					len = SDL_snprintf(g_buffer, sizeof(g_buffer), "%s", g_lastMessage.message.c_str());
			}
			UTIL_SetClipboardTextLatin1(UTIL_formatConsoleText(std::string(g_buffer, SDL_static_cast(size_t, len))).c_str());
		}
		break;
		case CHAT_SELECTALL_EVENTID:
		{
			Channel* selectedchannel = g_chat.getCurrentChannel();
			if(selectedchannel)
				selectedchannel->channelConsole->selectAll();
		}
		break;
		case CHAT_MESSAGETO_EVENTID:
		{
			if(!g_lastMessage.name.empty())
				g_game.sendOpenPrivateChannel(g_lastMessage.name);
		}
		break;
		case CHAT_INVITETO_EVENTID:
		{
			if(!g_lastMessage.name.empty())
				g_game.sendChannelInvite(g_lastMessage.name, SDL_static_cast(Uint16, g_chat.getOwnPrivateChannel()));
		}
		break;
		case CHAT_EXCLUDEFROM_EVENTID:
		{
			if(!g_lastMessage.name.empty())
				g_game.sendChannelExclude(g_lastMessage.name, SDL_static_cast(Uint16, g_chat.getOwnPrivateChannel()));
		}
		break;
		case CHAT_ADDTOVIP_EVENTID:
		{
			if(!g_lastMessage.name.empty())
				g_game.sendAddVip(g_lastMessage.name);
		}
		break;
		case CHAT_IGNORE_EVENTID:
		{
			if(!g_lastMessage.name.empty())
				UTIL_toggleIgnore(g_lastMessage.name);
		}
		break;
		case CHAT_RULEVIOLATION_EVENTID:
		{
			//Rule Violation
		}
		break;

		case CHAT_TEXT_CUT_EVENTID:
		{
			SDL_Event keyEvent;
			keyEvent.key.keysym.sym = SDLK_x;
			keyEvent.key.keysym.mod = KMOD_CTRL;
			g_chat.getTextBox()->onKeyDown(keyEvent);
			//Simulating CTRL+X shortcut should do the job
		}
		break;
		case CHAT_TEXT_COPY_EVENTID:
		{
			SDL_Event keyEvent;
			keyEvent.key.keysym.sym = SDLK_c;
			keyEvent.key.keysym.mod = KMOD_CTRL;
			g_chat.getTextBox()->onKeyDown(keyEvent);
			//Simulating CTRL+C shortcut should do the job
		}
		break;
		case CHAT_TEXT_PASTE_EVENTID:
		{
			SDL_Event keyEvent;
			keyEvent.key.keysym.sym = SDLK_v;
			keyEvent.key.keysym.mod = KMOD_CTRL;
			g_chat.getTextBox()->onKeyDown(keyEvent);
			//Simulating CTRL+V shortcut should do the job
		}
		break;
		case CHAT_TEXT_DELETE_EVENTID:
		{
			SDL_Event keyEvent;
			keyEvent.key.keysym.sym = SDLK_DELETE;
			keyEvent.key.keysym.mod = KMOD_NONE;
			g_chat.getTextBox()->onKeyDown(keyEvent);
			//Simulating DELETE shortcut should do the job
		}
		break;
		case CHAT_TEXT_SELECTALL_EVENTID:
		{
			if(!g_chat.getTextBox()->getActualText().empty())
				g_chat.getTextBox()->selectAll();
		}
		break;

		case INVITE_PRIVATECHAT_CANCEL_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_INVITE)
				g_engine.removeWindow(pWindow);
		}
		break;
		case INVITE_PRIVATECHAT_ADD_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_INVITE)
			{
				g_engine.removeWindow(pWindow);
				GUI_TextBox* pTextBox = SDL_static_cast(GUI_TextBox*, pWindow->getChild(INVITE_PRIVATECHAT_TEXTBOX_EVENTID));
				if(pTextBox)
					g_game.sendChannelInvite(pTextBox->getActualText(), SDL_static_cast(Uint16, g_chat.getOwnPrivateChannel()));
			}
		}
		break;

		case EXCLUDE_PRIVATECHAT_CANCEL_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_EXCLUDE)
				g_engine.removeWindow(pWindow);
		}
		break;
		case EXCLUDE_PRIVATECHAT_ADD_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_EXCLUDE)
			{
				g_engine.removeWindow(pWindow);
				GUI_TextBox* pTextBox = SDL_static_cast(GUI_TextBox*, pWindow->getChild(EXCLUDE_PRIVATECHAT_TEXTBOX_EVENTID));
				if(pTextBox)
					g_game.sendChannelExclude(pTextBox->getActualText(), SDL_static_cast(Uint16, g_chat.getOwnPrivateChannel()));
			}
		}
		break;
		
		case CHAT_CHANNEL_INVITE_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_INVITE);
			if(pWindow)
				return;

			GUI_Window* newWindow = new GUI_Window(iRect(0, 0, INVITE_PRIVATECHAT_WIDTH, INVITE_PRIVATECHAT_HEIGHT), INVITE_PRIVATECHAT_TITLE, GUI_WINDOW_INVITE);
			GUI_Label* newLabel = new GUI_Label(iRect(INVITE_PRIVATECHAT_LABEL_X, INVITE_PRIVATECHAT_LABEL_Y, 0, 0), INVITE_PRIVATECHAT_LABEL_TEXT);
			newWindow->addChild(newLabel);
			GUI_TextBox* newTextBox = new GUI_TextBox(iRect(INVITE_PRIVATECHAT_TEXTBOX_X, INVITE_PRIVATECHAT_TEXTBOX_Y, INVITE_PRIVATECHAT_TEXTBOX_W, INVITE_PRIVATECHAT_TEXTBOX_H), "", INVITE_PRIVATECHAT_TEXTBOX_EVENTID);
			newTextBox->setMaxLength(32);
			newTextBox->startEvents();
			newWindow->addChild(newTextBox);
			GUI_Button* newButton = new GUI_Button(iRect(INVITE_PRIVATECHAT_WIDTH - 56, INVITE_PRIVATECHAT_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Cancel", CLIENT_GUI_ESCAPE_TRIGGER);
			newButton->setButtonEventCallback(&chat_Events, INVITE_PRIVATECHAT_CANCEL_EVENTID);
			newButton->startEvents();
			newWindow->addChild(newButton);
			newButton = new GUI_Button(iRect(INVITE_PRIVATECHAT_WIDTH - 109, INVITE_PRIVATECHAT_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Add", CLIENT_GUI_ENTER_TRIGGER);
			newButton->setButtonEventCallback(&chat_Events, INVITE_PRIVATECHAT_ADD_EVENTID);
			newButton->startEvents();
			newWindow->addChild(newButton);
			GUI_Separator* newSeparator = new GUI_Separator(iRect(13, INVITE_PRIVATECHAT_HEIGHT - 40, INVITE_PRIVATECHAT_WIDTH - 26, 2));
			newWindow->addChild(newSeparator);
			g_engine.addWindow(newWindow);
		}
		break;
		case CHAT_CHANNEL_EXCLUDE_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_EXCLUDE);
			if(pWindow)
				return;

			GUI_Window* newWindow = new GUI_Window(iRect(0, 0, EXCLUDE_PRIVATECHAT_WIDTH, EXCLUDE_PRIVATECHAT_HEIGHT), EXCLUDE_PRIVATECHAT_TITLE, GUI_WINDOW_EXCLUDE);
			GUI_Label* newLabel = new GUI_Label(iRect(EXCLUDE_PRIVATECHAT_LABEL_X, EXCLUDE_PRIVATECHAT_LABEL_Y, 0, 0), EXCLUDE_PRIVATECHAT_LABEL_TEXT);
			newWindow->addChild(newLabel);
			GUI_TextBox* newTextBox = new GUI_TextBox(iRect(EXCLUDE_PRIVATECHAT_TEXTBOX_X, EXCLUDE_PRIVATECHAT_TEXTBOX_Y, EXCLUDE_PRIVATECHAT_TEXTBOX_W, EXCLUDE_PRIVATECHAT_TEXTBOX_H), "", EXCLUDE_PRIVATECHAT_TEXTBOX_EVENTID);
			newTextBox->setMaxLength(32);
			newTextBox->startEvents();
			newWindow->addChild(newTextBox);
			GUI_Button* newButton = new GUI_Button(iRect(EXCLUDE_PRIVATECHAT_WIDTH - 56, EXCLUDE_PRIVATECHAT_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Cancel", CLIENT_GUI_ESCAPE_TRIGGER);
			newButton->setButtonEventCallback(&chat_Events, EXCLUDE_PRIVATECHAT_CANCEL_EVENTID);
			newButton->startEvents();
			newWindow->addChild(newButton);
			newButton = new GUI_Button(iRect(EXCLUDE_PRIVATECHAT_WIDTH - 109, EXCLUDE_PRIVATECHAT_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Add", CLIENT_GUI_ENTER_TRIGGER);
			newButton->setButtonEventCallback(&chat_Events, EXCLUDE_PRIVATECHAT_ADD_EVENTID);
			newButton->startEvents();
			newWindow->addChild(newButton);
			GUI_Separator* newSeparator = new GUI_Separator(iRect(13, EXCLUDE_PRIVATECHAT_HEIGHT - 40, EXCLUDE_PRIVATECHAT_WIDTH - 26, 2));
			newWindow->addChild(newSeparator);
			g_engine.addWindow(newWindow);
		}
		break;
		case CHAT_CHANNEL_CLOSE_EVENTID:
		{
			Channel* selectedchannel = g_chat.getCurrentChannel();
			if(selectedchannel)
				g_chat.leaveChannel(selectedchannel->channelId);
		}
		break;
		case CHAT_CHANNEL_SERVERMESSAGES_EVENTID:
		{
			Channel* selectedchannel = g_chat.getCurrentChannel();
			if(selectedchannel)
				selectedchannel->workAsServerLog = !selectedchannel->workAsServerLog;
		}
		break;
		case CHAT_CHANNEL_SAVEWINDOW_EVENTID:
		{
			Channel* selectedchannel = g_chat.getCurrentChannel();
			if(selectedchannel)
			{
				std::string cachedText = selectedchannel->channelConsole->getCachedText();
				#if defined(SDL_VIDEO_DRIVER_WINDOWS)
				UTIL_replaceString(cachedText, "\n", "\r\n");
				#endif
				SDL_snprintf(g_buffer, sizeof(g_buffer), "%s%s.txt", g_basePath.c_str(), selectedchannel->channelName.c_str());
				SDL_RWops* fileHandle = SDL_RWFromFile(g_buffer, "ab+");
				if(!fileHandle)
				{
					SDL_snprintf(g_buffer, sizeof(g_buffer), "%s%s.txt", g_prefPath.c_str(), selectedchannel->channelName.c_str());
					fileHandle = SDL_RWFromFile(g_buffer, "ab+");
					if(!fileHandle)
					{
						std::string error; error.assign("Failed to save channel '").append(g_buffer).append("' access denied.");
						g_game.processTextMessage(MessageLogin, error);
						return;
					}
				}

				std::string info; info.assign("Channel ").append(selectedchannel->channelName).append(" saved at ").append(UTIL_formatDate("%a %b %e %T %Y", time(NULL)));
				#if defined(SDL_VIDEO_DRIVER_WINDOWS)
				SDL_WriteU8(fileHandle, '\r');
				#endif
				SDL_WriteU8(fileHandle, '\n');
				SDL_RWwrite(fileHandle, info.c_str(), 1, info.length());
				#if defined(SDL_VIDEO_DRIVER_WINDOWS)
				SDL_WriteU8(fileHandle, '\r');
				#endif
				SDL_WriteU8(fileHandle, '\n');
				SDL_RWwrite(fileHandle, cachedText.c_str(), 1, cachedText.length());
				#if defined(SDL_VIDEO_DRIVER_WINDOWS)
				SDL_WriteU8(fileHandle, '\r');
				#endif
				SDL_WriteU8(fileHandle, '\n');
				SDL_RWclose(fileHandle);

				info.assign("Channel ").append(selectedchannel->channelName).append(" appended to '").append(g_buffer).append("'.");
				g_game.processTextMessage(MessageLogin, info);
			}
		}
		break;
		case CHAT_CHANNEL_CLEARWINDOW_EVENTID:
		{
			Channel* selectedchannel = g_chat.getCurrentChannel();
			if(selectedchannel)
				selectedchannel->channelConsole->clearConsole();
		}
		break;
		default: break;
	}
}

Chat::Chat()
{
	m_textbox = new GUI_TextBox(iRect(0, 0, 0, 0), "", 0, 223, 223, 223);
	m_textbox->setMaxLength(255);
}

Chat::~Chat()
{
	delete m_textbox;
	clear();
}

void Chat::clear()
{
	for(std::vector<Channel>::iterator it = m_channels.begin(), end = m_channels.end(); it != end; ++it)
		delete (*it).channelConsole;

	m_channels.clear();
}

void Chat::gameStart()
{
	clear();
	if(g_clientVersion >= 870)
		openChannel(CHANNEL_ID_DEFAULT, CHANNEL_NAME_DEFAULT, false, false);
	else
		openChannel(CHANNEL_ID_DEFAULT, CHANNEL_NAME_DEFAULT_LEGACY, false, false);

	openChannel(CHANNEL_ID_SERVERLOG, CHANNEL_NAME_SERVERLOG, false, false);

	m_savedMessages.clear();
	m_textbox->setText("");
}

void Chat::navigateHistory(Sint32 direction)
{
	Sint32 messagesSize = SDL_static_cast(Sint32, m_savedMessages.size()) - 1;
	if(m_historyNavigator == SDL_MIN_SINT32)
	{
		if(m_savedMessages.empty() || direction > 0)
			return;

		m_historyNavigator = messagesSize + 1;
	}
	m_historyNavigator = UTIL_max<Sint32>(0, UTIL_min<Sint32>(m_historyNavigator + direction, messagesSize));

	std::list<std::string>::iterator it = m_savedMessages.begin();
	std::advance(it, m_historyNavigator);

	std::string& historyMessage = (*it);
	m_textbox->setText(historyMessage);
	m_textbox->clearSelection();
	m_textbox->moveCursor(SDL_static_cast(Sint32, historyMessage.length()));
}

void Chat::ignoreListEvent()
{
	m_ignoreListTime = g_frameTime + 1000;
}

void Chat::openPrivateChannel(const std::string& receiver)
{
	//Search for available channelId
	Channel* channel = getPrivateChannel(receiver);
	if(channel)
	{
		setCurrentChannel(channel->channelId);
		return;
	}

	for(Uint32 i = CHANNEL_ID_PRIVATE_START; i < CHANNEL_ID_PRIVATE_END; ++i)
	{
		channel = getChannel(i);
		if(!channel)
		{
			openChannel(i, receiver, true);
			setCurrentChannel(i);
			return;
		}
	}
}

void Chat::openChannel(Uint32 channelId, const std::string channelName, bool privateChannel, bool closable)
{
	for(std::vector<Channel>::iterator it = m_channels.begin(), end = m_channels.end(); it != end; ++it)
	{
		Channel& currentChannel = (*it);
		if(currentChannel.channelId == channelId)
		{
			delete currentChannel.channelConsole;
			m_channels.erase(it);
			break;
		}
	}

	m_channels.emplace_back(std::move(channelName), channelId, closable, privateChannel);
	if(g_clientVersion >= 871)
	{
		if(channelId == 7)//Help Channel
			addChannelMessage(channelId, MessageChannelManagement, 0, "", 0, CHANNEL_MESSAGE_HELP, time(NULL));
		else if(channelId == 5 || channelId == 6)//Advertising Channels
			addChannelMessage(channelId, MessageChannelManagement, 0, "", 0, CHANNEL_MESSAGE_ADVERTISING, time(NULL));
	}
	else if(g_clientVersion == 870)//Some weird special case with 8.70 version
	{
		if(channelId == 6)//Help Channel
			addChannelMessage(channelId, MessageChannelManagement, 0, "", 0, CHANNEL_MESSAGE_HELP, time(NULL));
		else if(channelId == 4 || channelId == 5)//Advertising Channels
			addChannelMessage(channelId, MessageChannelManagement, 0, "", 0, CHANNEL_MESSAGE_ADVERTISING, time(NULL));
	}
	else if(g_clientVersion >= 840)
	{
		if(channelId == 9)//Help Channel
		{
			if(g_clientVersion >= 854)
				addChannelMessage(channelId, MessageChannelManagement, 0, "", 0, CHANNEL_MESSAGE_HELP, time(NULL));
			else
				addChannelMessage(channelId, MessageGamemasterChannel, 0, "", 0, CHANNEL_MESSAGE_HELP_LEGACY, time(NULL));
		}
	}
	else if(g_clientVersion >= 790)
	{
		if(channelId == 8)//Help Channel
			addChannelMessage(channelId, MessageGamemasterChannel, 0, "", 0, CHANNEL_MESSAGE_HELP_LEGACY, time(NULL));
	}
	else
	{
		if(channelId == 7)//Help Channel
			addChannelMessage(channelId, MessageGamemasterChannel, 0, "", 0, CHANNEL_MESSAGE_HELP_LEGACY, time(NULL));
	}
}

void Chat::closeChannel(Uint32 channelId)
{
	addChannelMessage(channelId, MessageChannelManagement, 0, "", 0, CHANNEL_MESSAGE_CLOSED, time(NULL));
	for(std::vector<Channel>::iterator it = m_channels.begin(), end = m_channels.end(); it != end; ++it)
	{
		Channel& currentChannel = (*it);
		if(currentChannel.channelId == channelId)
		{
			currentChannel.alreadyClosed = true;
			break;
		}
	}
}

void Chat::leaveChannel(Uint32 channelId)
{
	bool alreadyClosed = false;
	for(std::vector<Channel>::iterator it = m_channels.begin(), end = m_channels.end(); it != end; ++it)
	{
		Channel& currentChannel = (*it);
		if(currentChannel.channelId == channelId)
		{
			alreadyClosed = currentChannel.alreadyClosed;
			delete currentChannel.channelConsole;
			m_channels.erase(it);
			break;
		}
	}

	if(channelId == CHANNEL_ID_NPC)
		g_game.sendCloseNPCChannel();
	else if(!alreadyClosed)
		g_game.sendCloseChannel(SDL_static_cast(Uint16, channelId));

	if(channelId == m_ownPrivatechannel)
		m_ownPrivatechannel = SDL_static_cast(Uint32, -1);

	if(m_selectedChannel > 0 && m_selectedChannel == m_channels.size())
		--m_selectedChannel;
}

void Chat::addChannelMessage(Uint32 channelId, MessageMode mode, Uint32 statementId, const std::string& playerName, Uint16 playerLevel, const std::string& text, time_t timestamp)
{
	Uint32 messageFlags = 0;
	Uint8 red, green, blue;
	switch(mode)
	{
		case MessageSay:
		case MessageWhisper:
		case MessageYell:
		case MessageChannel:
		{
			messageFlags = (MESSAGE_FLAG_REPORTNAME|MESSAGE_FLAG_REPORTSTATEMENT);
			red = 240;
			green = 240;
			blue = 0;
		}
		break;
		case MessageSpell:
		{
			messageFlags = MESSAGE_FLAG_REPORTNAME;
			red = 240;
			green = 240;
			blue = 0;
		}
		break;
		case MessageRVRContinue:
		{
			red = 240;
			green = 240;
			blue = 0;
		}
		break;
		case MessageMonsterSay:
		case MessageMonsterYell:
		case MessageBarkLow:
		case MessageBarkLoud:
		{
			red = 255;
			green = 100;
			blue = 0;
		}
		break;
		case MessageNpcFrom:
		case MessageNpcFromBlock:
		{
			red = 96;
			green = 250;
			blue = 250;
		}
		break;
		case MessageNpcTo:
		case MessageGamemasterPrivateTo:
		case MessageBlue:
		{
			red = 160;
			green = 160;
			blue = 255;
		}
		break;
		case MessagePrivateFrom:
		{
			messageFlags = (MESSAGE_FLAG_REPORTNAME|MESSAGE_FLAG_REPORTSTATEMENT);
			red = 96;
			green = 250;
			blue = 250;
		}
		break;
		case MessagePrivateTo:
		{
			messageFlags = (MESSAGE_FLAG_REPORTNAME|MESSAGE_FLAG_REPORTSTATEMENT);
			red = 160;
			green = 160;
			blue = 255;
		}
		break;
		case MessageChannelManagement:
		case MessageLogin:
		case MessageGame:
		case MessageDamageDealed:
		case MessageDamageReceived:
		case MessageHeal:
		case MessageExp:
		case MessageDamageOthers:
		case MessageHealOthers:
		case MessageExpOthers:
		case MessageStatus:
		case MessageReport:
		case MessageTutorialHint:
		case MessageThankyou:
		case MessageMana:
		case MessageBeyondLast:
		case MessageRVRChannel:
		{
			red = 255;
			green = 255;
			blue = 255;
		}
		break;
		case MessageChannelHighlight:
		{
			messageFlags = (MESSAGE_FLAG_REPORTNAME|MESSAGE_FLAG_REPORTSTATEMENT);
			red = 240;
			green = 180;
			blue = 0;
		}
		break;
		case MessageGuild:
		case MessagePartyManagement:
		case MessageParty:
		{
			red = 255;
			green = 255;
			blue = 255;
		}
		break;
		case MessageGamemasterBroadcast:
		case MessageGamemasterChannel:
		case MessageGamemasterPrivateFrom:
		case MessageWarning:
		case MessageRed:
		case MessageRVRAnswer:
		case MessageGameHighlight:
		{
			red = 250;
			green = 96;
			blue = 96;
		}
		break;
		case MessageLook:
		case MessageHotkeyUse:
		case MessageLoot:
		case MessageTradeNpc:
		{
			red = 0;
			green = 240;
			blue = 0;
		}
		break;
		default: return;
	}

	if(channelId == CHANNEL_ID_NPC)
	{
		Channel* NPCchannel = getChannel(CHANNEL_ID_NPC);
		if(!NPCchannel)
		{
			openChannel(CHANNEL_ID_NPC, CHANNEL_NAME_NPC);
			setCurrentChannel(CHANNEL_ID_NPC);
		}
	}

	for(std::vector<Channel>::iterator it = m_channels.begin(), end = m_channels.end(); it != end; ++it)
	{
		Channel& currentChannel = (*it);
		if(currentChannel.channelId == channelId || (channelId == CHANNEL_ID_SERVERLOG && currentChannel.workAsServerLog))
		{
			currentChannel.channelConsole->addMessage(statementId, timestamp, playerName, playerLevel, text, red, green, blue, messageFlags);
			currentChannel.highlightTime = g_frameTime + 1000;
			currentChannel.unreadMessage = true;
			return;
		}
	}
}

void Chat::sendMessage()
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	std::string message = m_textbox->getActualText();
	if(message.empty())
		return;

	Uint32 channelId = selectedchannel->channelId;
	std::string receiver;
	MessageMode mode = (m_volumeAdjustement == VOLUME_SAY ? MessageSay : m_volumeAdjustement == VOLUME_YELL ? MessageYell : m_volumeAdjustement == VOLUME_WHISPER ? MessageWhisper : MessageNone);
	if(selectedchannel->alreadyClosed)
		mode = MessageNone;
	else if(selectedchannel->privateChannel)
	{
		mode = MessagePrivateTo;
		receiver = selectedchannel->channelName;
	}
	else if(channelId == CHANNEL_ID_NPC)
		mode = MessageNpcTo;
	else if(channelId != CHANNEL_ID_DEFAULT && channelId != CHANNEL_ID_SERVERLOG)
		mode = MessageChannel;

	if(m_savedMessages.size() >= CHAT_MAXIMUM_MESSAGE_HISTORY)
		m_savedMessages.pop_front();

	if(m_savedMessages.empty() || m_savedMessages.back() != message)
		m_savedMessages.push_back(message);

	m_historyNavigator = SDL_MIN_SINT32;
	m_volumeAdjustement = VOLUME_SAY;

	m_textbox->setText("");
	m_textbox->clearSelection();
	if(message.size() > 3)
	{
		//#a, #d and $name$text are deprecated and not supported by any ots(?) so I don't see any point to implement them
		if(message[0] == '#')
		{
			char command = message[1];
			size_t found = message.find(' ', 2);
			if(found != std::string::npos)
				message = message.substr(found + 1);
			else
				return;

			if(command == 'b')
				mode = MessageGamemasterBroadcast;
			else if(command == 'c')
			{
				if(mode != MessageChannel)
				{
					mode = MessageNone;
					g_game.processTextMessage(MessageFailure, "This is not a chat channel.");
				}
				else
					mode = MessageGamemasterChannel;
			}
			else if(command == 'i')
			{
				mode = MessageNone;
				if(m_ownPrivatechannel != SDL_static_cast(Uint32, -1))
					g_game.sendChannelInvite(message, SDL_static_cast(Uint16, m_ownPrivatechannel));
			}
			else if(command == 'x')
			{
				mode = MessageNone;
				if(m_ownPrivatechannel != SDL_static_cast(Uint32, -1))
					g_game.sendChannelExclude(message, SDL_static_cast(Uint16, m_ownPrivatechannel));
			}
			else if(command == 's')
				mode = MessageSay;
			else if(command == 'w')
				mode = MessageWhisper;
			else if(command == 'y')
				mode = MessageYell;
		}
		else if(message[0] == '@')
		{
			size_t found = message.find('@', 2);
			if(found != std::string::npos)
			{
				mode = MessageGamemasterPrivateTo;
				receiver = message.substr(1, found - 1);
				if(selectedchannel->privateChannel && receiver != selectedchannel->channelName)
				{
					Channel* havePrivateChanel = getPrivateChannel(receiver);
					if(!havePrivateChanel)
						channelId = CHANNEL_ID_DEFAULT;
					else
						channelId = havePrivateChanel->channelId;
				}
				message = message.substr(found + 1);
			}
		}
		else if(message[0] == '*')
		{
			size_t found = message.find('*', 2);
			if(found != std::string::npos)
			{
				mode = MessagePrivateTo;
				receiver = message.substr(1, found - 1);
				if(selectedchannel->privateChannel && receiver != selectedchannel->channelName)
				{
					m_textbox->setText(message.substr(0, found + 1) + ' ');
					Channel* havePrivateChanel = getPrivateChannel(receiver);
					if(!havePrivateChanel)
						channelId = CHANNEL_ID_DEFAULT;
					else
						channelId = havePrivateChanel->channelId;
				}
				message = message.substr(found + 1);
			}
		}
	}

	switch(mode)
	{
		case MessageSay:
		case MessageWhisper:
		case MessageYell:
		case MessageGamemasterBroadcast:
		case MessageChannel:
		case MessageGamemasterChannel:
			g_game.sendSay(mode, SDL_static_cast(Uint16, channelId), receiver, message);
			break;
		case MessagePrivateTo:
		case MessageGamemasterPrivateTo:
		{
			addChannelMessage(channelId, mode, 0, g_game.getPlayerName(), g_game.getPlayerLevel(), message, time(NULL));
			g_game.sendSay(mode, 0, receiver, message);
		}
		break;
		case MessageNpcTo:
		{
			addChannelMessage(channelId, mode, 0, g_game.getPlayerName(), g_game.getPlayerLevel(), message, time(NULL));
			g_game.sendSay(mode, 0, receiver, message);
		}
		break;
		default: break;
	}
}

Channel* Chat::getPrivateChannel(const std::string& receiver)
{
	for(std::vector<Channel>::iterator it = m_channels.begin(), end = m_channels.end(); it != end; ++it)
	{
		Channel& currentChannel = (*it);
		if(currentChannel.privateChannel && currentChannel.channelName == receiver)
			return &currentChannel;
	}
	return NULL;
}

Channel* Chat::getChannel(Uint32 channelId)
{
	for(std::vector<Channel>::iterator it = m_channels.begin(), end = m_channels.end(); it != end; ++it)
	{
		Channel& currentChannel = (*it);
		if(currentChannel.channelId == channelId)
			return &currentChannel;
	}
	return NULL;
}

void Chat::switchToNextChannel()
{
	if(m_channels.empty())
		return;

	++m_selectedChannel;
	if(m_selectedChannel == m_channels.size())
		m_selectedChannel = 0;
}

void Chat::switchToPreviousChannel()
{
	if(m_channels.empty())
		return;

	if(m_selectedChannel == 0)
		m_selectedChannel = m_channels.size();
	--m_selectedChannel;
}

void Chat::setCurrentChannel(Uint32 channelId)
{
	for(size_t it = 0, end = m_channels.size(); it != end; ++it)
	{
		Channel& currentChannel = m_channels[it];
		if(currentChannel.channelId == channelId)
		{
			m_selectedChannel = it;
			return;
		}
	}
}

Channel* Chat::getCurrentChannel()
{
	if(m_channels.empty())
		return NULL;

	if(m_selectedChannel < m_channels.size())
		return &m_channels[m_selectedChannel];

	m_selectedChannel = 0;
	return &m_channels[m_selectedChannel];
}

Uint32 Chat::getHelpChannelId()
{
	if(g_clientVersion >= 871)
		return 7;
	else if(g_clientVersion == 870)//Some weird special case with 8.70 version
		return 6;
	else if(g_clientVersion >= 840)
		return 9;
	else if(g_clientVersion >= 790)
		return 8;

	return 7;
}

std::pair<size_t, size_t> Chat::calculateChannelPages(iRect& rect)
{
	size_t maxChannels = SDL_static_cast(size_t, (rect.x2 - 100) / 96);
	size_t channels = m_channels.size();
	if(channels > maxChannels)
		return std::make_pair(channels - maxChannels, maxChannels);

	return std::make_pair(0, maxChannels);
}

void Chat::onTextInput(const char* textInput)
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	m_textbox->onTextInput(textInput);
}

void Chat::onKeyDown(SDL_Event& event)
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	m_textbox->onKeyDown(event);
}

void Chat::onKeyUp(SDL_Event& event)
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	m_textbox->onKeyUp(event);
}

void Chat::onLMouseDown(iRect& rect, Sint32 x, Sint32 y)
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	iRect irect = iRect(rect.x1, rect.y1, rect.x2, 4);
	if(irect.isPointInside(x, y))
	{
		y *= -1;
		m_bMouseResizing = true;
		m_resizingY = y - g_engine.getConsoleHeight();
		return;
	}

	irect = iRect(rect.x1 + rect.x2 - 16, rect.y1 + 5, 16, 16);
	if(irect.isPointInside(x, y))
	{
		m_ignoreListStatus = 1;
		return;
	}

	irect = iRect(rect.x1 + rect.x2 - 32, rect.y1 + 5, 16, 16);
	if(irect.isPointInside(x, y))
	{
		m_channelListStatus = 1;
		return;
	}

	if(selectedchannel->channelClosable)
	{
		irect = iRect(rect.x1 + rect.x2 - 48, rect.y1 + 5, 16, 16);
		if(irect.isPointInside(x, y))
		{
			m_serverLogStatus = 1;
			return;
		}
		
		irect = iRect(rect.x1 + rect.x2 - 64, rect.y1 + 5, 16, 16);
		if(irect.isPointInside(x, y))
		{
			m_closeChannelStatus = 1;
			return;
		}
	}
	else
	{
		irect = iRect(rect.x1 + 5, rect.y1 + rect.y2 - 20, 16, 16);
		if(irect.isPointInside(x, y))
		{
			m_volumeStatus = 1;
			return;
		}
	}

	std::pair<size_t, size_t> calculatedPages = calculateChannelPages(rect);
	if(calculatedPages.first > 0)
	{
		if(m_currentPage > calculatedPages.first)
			m_currentPage = calculatedPages.first;

		if(m_currentPage != calculatedPages.first)
		{
			irect = iRect(rect.x1 + rect.x2 - 82, rect.y1 + 5, 18, 18);
			if(irect.isPointInside(x, y))
			{
				m_buttonNext = 1;
				return;
			}
		}
		if(m_currentPage != 0)
		{
			irect = iRect(rect.x1, rect.y1 + 5, 18, 18);
			if(irect.isPointInside(x, y))
			{
				m_buttonPrevious = 1;
				return;
			}
		}
	}
	else
		m_currentPage = 0;

	Sint32 posX = rect.x1 + 18;
	irect = iRect(posX, rect.y1 + 5, rect.x2 - 100, 18);
	if(irect.isPointInside(x, y))
	{
		for(size_t it = m_currentPage, end = UTIL_min<size_t>(m_channels.size(), it + calculatedPages.second + 1); it != end; ++it)
		{
			irect = iRect(posX, rect.y1 + 5, 96, 18);
			if(irect.isPointInside(x, y))
			{
				m_selectedChannel = it;
				return;
			}
			posX += 96;
		}
	}

	if(m_textbox->getRect().isPointInside(x, y))
	{
		m_textbox->onLMouseDown(x, y);
		selectedchannel->channelConsole->clearSelection();
	}
	else if(selectedchannel->channelConsole->getRect().isPointInside(x, y))
	{
		m_textbox->clearSelection();
		selectedchannel->channelConsole->onLMouseDown(x, y);
	}
}

void Chat::onLMouseUp(iRect&, Sint32 x, Sint32 y)
{
	if(m_bMouseResizing)
		m_bMouseResizing = false;

	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
	{
		m_ignoreListStatus = 0;
		m_channelListStatus = 0;
		m_serverLogStatus = 0;
		m_closeChannelStatus = 0;
		m_volumeStatus = 0;
		m_buttonNext = 0;
		m_buttonPrevious = 0;
		return;
	}
	
	if(m_ignoreListStatus > 0)
	{
		if(m_ignoreListStatus == 1)
			UTIL_createIgnoreList();

		m_ignoreListStatus = 0;
	}
	else if(m_channelListStatus > 0)
	{
		if(m_channelListStatus == 1)
			g_game.sendRequestChannels();

		m_channelListStatus = 0;
	}
	else if(m_serverLogStatus > 0)
	{
		if(m_serverLogStatus == 1)
			selectedchannel->workAsServerLog = !selectedchannel->workAsServerLog;

		m_serverLogStatus = 0;
	}
	else if(m_closeChannelStatus > 0)
	{
		if(m_closeChannelStatus == 1)
			leaveChannel(selectedchannel->channelId);

		m_closeChannelStatus = 0;
	}
	else if(m_volumeStatus > 0)
	{
		if(m_volumeStatus == 1)
		{
			if(m_volumeAdjustement == VOLUME_SAY)
				m_volumeAdjustement = VOLUME_YELL;
			else if(m_volumeAdjustement == VOLUME_YELL)
				m_volumeAdjustement = VOLUME_WHISPER;
			else if(m_volumeAdjustement == VOLUME_WHISPER)
				m_volumeAdjustement = VOLUME_SAY;
		}

		m_volumeStatus = 0;
	}
	else if(m_buttonNext > 0)
	{
		if(m_buttonNext == 1)
			++m_currentPage;

		m_buttonNext = 0;
	}
	else if(m_buttonPrevious > 0)
	{
		if(m_buttonPrevious == 1)
			--m_currentPage;

		m_buttonPrevious = 0;
	}

	m_textbox->onLMouseUp(x, y);
	selectedchannel->channelConsole->onLMouseUp(x, y);
}

void Chat::onRMouseDown(iRect& rect, Sint32 x, Sint32 y)
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	if(rect.isPointInside(x, y))
		m_haveRMouse = true;

	if(m_textbox->getRect().isPointInside(x, y))
		m_textbox->onRMouseDown(x, y);
	else if(selectedchannel->channelConsole->getRect().isPointInside(x, y))
		selectedchannel->channelConsole->onRMouseDown(x, y);
}

void Chat::onRMouseUp(iRect& rect, Sint32 x, Sint32 y)
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	m_textbox->onRMouseUp(x, y);
	selectedchannel->channelConsole->onRMouseUp(x, y);
	if(m_haveRMouse)
	{
		Sint32 posX = rect.x1 + 18;
		iRect irect = iRect(posX, rect.y1 + 5, rect.x2 - 100, 18);
		if(irect.isPointInside(x, y))
		{
			std::pair<size_t, size_t> calculatedPages = calculateChannelPages(rect);
			if(calculatedPages.first > 0)
			{
				if(m_currentPage > calculatedPages.first)
					m_currentPage = calculatedPages.first;
			}
			else
				m_currentPage = 0;

			for(size_t it = m_currentPage, end = UTIL_min<size_t>(m_channels.size(), it + calculatedPages.second + 1); it != end; ++it)
			{
				irect = iRect(posX, rect.y1 + 5, 96, 18);
				if(irect.isPointInside(x, y))
				{
					selectedchannel = &m_channels[it];
					GUI_ContextMenu* newMenu = new GUI_ContextMenu();
					if(getOwnPrivateChannel() == selectedchannel->channelId)
					{
						newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_CHANNEL_INVITE_EVENTID, "Invite player", "");
						newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_CHANNEL_EXCLUDE_EVENTID, "Exclude player", "");
						newMenu->addSeparator();
					}
					if(selectedchannel->channelClosable)
					{
						newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_CHANNEL_CLOSE_EVENTID, "Close", "");
						newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_CHANNEL_SERVERMESSAGES_EVENTID, (selectedchannel->workAsServerLog ? "Hide server messages" : "Show server messages"), "");
						newMenu->addSeparator();
					}
					newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_CHANNEL_SAVEWINDOW_EVENTID, "Save Window", "");
					newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_CHANNEL_CLEARWINDOW_EVENTID, "Clear Window", "");
					newMenu->setEventCallback(&chat_Events);
					g_engine.showContextMenu(newMenu, x, y);
					m_selectedChannel = it;
					return;
				}
				posX += 96;
			}
			return;
		}

		if(m_textbox->getRect().isPointInside(x, y))
		{
			GUI_ContextMenu* newMenu = new GUI_ContextMenu();
			if(m_textbox->hasSelection())
			{
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_TEXT_CUT_EVENTID, "Cut", "");
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_TEXT_COPY_EVENTID, "Copy", "");
			}
			newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_TEXT_PASTE_EVENTID, "Paste", "");
			newMenu->addSeparator();
			if(m_textbox->hasSelection())
			{
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_TEXT_DELETE_EVENTID, "Delete", "");
				newMenu->addSeparator();
			}
			if(!m_textbox->getActualText().empty())
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_TEXT_SELECTALL_EVENTID, "Select All", "");

			newMenu->setEventCallback(&chat_Events);
			g_engine.showContextMenu(newMenu, x, y);
		}
		else if(selectedchannel->channelConsole->getRect().isPointInside(x, y))
		{
			ConsoleMessage* message = selectedchannel->channelConsole->getConsoleMessage(x, y);
			if(message)
			{
				g_lastMessage = *message;

				const std::string& creatureName = message->name;
				GUI_ContextMenu* newMenu = new GUI_ContextMenu();
				if(!creatureName.empty() && g_game.getPlayerName() != creatureName)
				{
					if(selectedchannel->channelId != CHANNEL_ID_NPC && !selectedchannel->privateChannel)
					{
						SDL_snprintf(g_buffer, sizeof(g_buffer), "Message to %s", creatureName.c_str());
						newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_MESSAGETO_EVENTID, g_buffer, "");
					}
					if(getOwnPrivateChannel() != SDL_static_cast(Uint32, -1))
					{
						newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_INVITETO_EVENTID, "Invite to private chat", "");
						newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_EXCLUDEFROM_EVENTID, "Exclude from private chat", "");
					}
					if(!UTIL_haveVipPlayer(creatureName))
						newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_ADDTOVIP_EVENTID, "Add to VIP list", "");

					if(!UTIL_onWhiteList(creatureName))
					{
						SDL_snprintf(g_buffer, sizeof(g_buffer), (UTIL_onBlackList(creatureName) ? "Unignore %s" : "Ignore %s"), creatureName.c_str());
						newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_IGNORE_EVENTID, g_buffer, "");
					}
					newMenu->addSeparator();
				}
				if(selectedchannel->channelConsole->hasSelection())
					newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_COPY_EVENTID, "Copy", "");

				if(!creatureName.empty())
					newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_COPYNAME_EVENTID, "Copy Name", "");

				newMenu->addContextMenu((CONTEXTMENU_STYLE_STANDARD|CONTEXTMENU_STYLE_SEPARATED), CHAT_COPYMESSAGE_EVENTID, "Copy Message", "");
				newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_SELECTALL_EVENTID, "Select all", "");
				//newMenu->addSeparator();
				//newMenu->addContextMenu(CONTEXTMENU_STYLE_STANDARD, CHAT_RULEVIOLATION_EVENTID, "Rule Violation", "");
				newMenu->setEventCallback(&chat_Events);
				g_engine.showContextMenu(newMenu, x, y);
			}
		}
		m_haveRMouse = false;
	}
}

void Chat::onWheel(iRect&, Sint32 x, Sint32 y, bool wheelUP)
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	if(m_textbox->getRect().isPointInside(x, y))
		m_textbox->onWheel(x, y, wheelUP);
	else if(selectedchannel->channelConsole->getRect().isPointInside(x, y))
		selectedchannel->channelConsole->onWheel(x, y, wheelUP);
}

void Chat::onMouseMove(iRect& rect, Sint32 x, Sint32 y)
{
	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	if(m_bMouseResizing)
	{
		y *= -1;
		g_engine.setConsoleHeight(y - m_resizingY);
		g_actualCursor = CLIENT_CURSOR_RESIZENS;
		return;
	}
	else
	{
		iRect dragWindow = iRect(rect.x1, rect.y1, rect.x2, 4);
		if(dragWindow.isPointInside(x, y))
			g_actualCursor = CLIENT_CURSOR_RESIZENS;
	}

	iRect irect = iRect(rect.x1 + rect.x2 - 16, rect.y1 + 5, 16, 16);
	bool inside = irect.isPointInside(x, y);
	if(m_ignoreListStatus > 0)
	{
		if(m_ignoreListStatus == 1 && !inside)
			m_ignoreListStatus = 2;
		else if(m_ignoreListStatus == 2 && inside)
			m_ignoreListStatus = 1;
	}
	if(inside)
		g_engine.showDescription(x, y, "Ignore/unignore other players");

	irect = iRect(rect.x1 + rect.x2 - 32, rect.y1 + 5, 16, 16);
	inside = irect.isPointInside(x, y);
	if(m_channelListStatus > 0)
	{
		if(m_channelListStatus == 1 && !inside)
			m_channelListStatus = 2;
		else if(m_channelListStatus == 2 && inside)
			m_channelListStatus = 1;
	}
	if(inside)
		g_engine.showDescription(x, y, "Open new channel");

	if(selectedchannel->channelClosable)
	{
		irect = iRect(rect.x1 + rect.x2 - 48, rect.y1 + 5, 16, 16);
		inside = irect.isPointInside(x, y);
		if(m_serverLogStatus > 0)
		{
			if(m_serverLogStatus == 1 && !inside)
				m_serverLogStatus = 2;
			else if(m_serverLogStatus == 2 && inside)
				m_serverLogStatus = 1;
		}
		if(inside)
			g_engine.showDescription(x, y, "Show server messages in this channel");

		irect = iRect(rect.x1 + rect.x2 - 64, rect.y1 + 5, 16, 16);
		inside = irect.isPointInside(x, y);
		if(m_closeChannelStatus > 0)
		{
			if(m_closeChannelStatus == 1 && !inside)
				m_closeChannelStatus = 2;
			else if(m_closeChannelStatus == 2 && inside)
				m_closeChannelStatus = 1;
		}
		if(inside)
			g_engine.showDescription(x, y, "Close this channel");
	}
	else
	{
		irect = iRect(rect.x1 + 5, rect.y1 + rect.y2 - 20, 16, 16);
		inside = irect.isPointInside(x, y);
		if(m_volumeStatus > 0)
		{
			if(m_volumeStatus == 1 && !inside)
				m_volumeStatus = 2;
			else if(m_volumeStatus == 2 && inside)
				m_volumeStatus = 1;
		}
		if(inside)
			g_engine.showDescription(x, y, "Adjust volume");
	}

	std::pair<size_t, size_t> calculatedPages = calculateChannelPages(rect);
	if(calculatedPages.first > 0)
	{
		if(m_currentPage > calculatedPages.first)
			m_currentPage = calculatedPages.first;

		if(m_currentPage != calculatedPages.first)
		{
			irect = iRect(rect.x1 + rect.x2 - 82, rect.y1 + 5, 18, 18);
			inside = irect.isPointInside(x, y);
			if(m_buttonNext > 0)
			{
				if(m_buttonNext == 1 && !inside)
					m_buttonNext = 2;
				else if(m_buttonNext == 2 && inside)
					m_buttonNext = 1;
			}
			if(inside)
				g_engine.showDescription(x, y, "Move channels to the left");
		}
		if(m_currentPage != 0)
		{
			irect = iRect(rect.x1, rect.y1 + 5, 18, 18);
			inside = irect.isPointInside(x, y);
			if(m_buttonPrevious > 0)
			{
				if(m_buttonPrevious == 1 && !inside)
					m_buttonPrevious = 2;
				else if(m_buttonPrevious == 2 && inside)
					m_buttonPrevious = 1;
			}
			if(inside)
				g_engine.showDescription(x, y, "Move channels to the right");
		}
	}
	else
		m_currentPage = 0;

	bool isInsideParent = rect.isPointInside(x, y);
	m_textbox->onMouseMove(x, y, isInsideParent);
	selectedchannel->channelConsole->onMouseMove(x, y, isInsideParent);
}

void Chat::render(iRect& rect)
{
	auto& renderer = g_engine.getRender();
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_X, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_Y, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_W, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_H, rect.x1, rect.y1, rect.x2, 1);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_BACKGROUND_GREY_X, GUI_UI_BACKGROUND_GREY_Y, GUI_UI_BACKGROUND_GREY_W, GUI_UI_BACKGROUND_GREY_H, rect.x1, rect.y1 + 1, rect.x2, 3);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_DARK_X, GUI_UI_ICON_HORIZONTAL_LINE_DARK_Y, GUI_UI_ICON_HORIZONTAL_LINE_DARK_W, GUI_UI_ICON_HORIZONTAL_LINE_DARK_H, rect.x1, rect.y1 + 4, rect.x2, 1);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_CONSOLE_BACKGROUND_X, GUI_UI_ICON_CONSOLE_BACKGROUND_Y, GUI_UI_ICON_CONSOLE_BACKGROUND_W, GUI_UI_ICON_CONSOLE_BACKGROUND_H, rect.x1, rect.y1 + 5, rect.x2, 16);
	// Draw background anchored to absolute screen position so it doesn't shift during resize.
	// Use clip rect to constrain to the chat area, then draw from y=0 so tiles are always at same absolute position.
	renderer->setClipRect(rect.x1 + 2, rect.y1 + 23, rect.x2 - 4, rect.y2 - 25);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_BACKGROUND_GREY_X, GUI_UI_BACKGROUND_GREY_Y, GUI_UI_BACKGROUND_GREY_W, GUI_UI_BACKGROUND_GREY_H, rect.x1 + 2, 0, rect.x2 - 4, g_engine.getWindowHeight());
	renderer->disableClipRect();
	
	renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_TOPLEFT_BORDER_X, GUI_UI_ICON_TOPLEFT_BORDER_Y, rect.x1 + 4, rect.y1 + 26, GUI_UI_ICON_TOPLEFT_BORDER_W, GUI_UI_ICON_TOPLEFT_BORDER_H);
	renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BOTLEFT_BORDER_X, GUI_UI_ICON_BOTLEFT_BORDER_Y, rect.x1 + 4, rect.y1 + rect.y2 - 25, GUI_UI_ICON_BOTLEFT_BORDER_W, GUI_UI_ICON_BOTLEFT_BORDER_H);
	renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_TOPRIGHT_BORDER_X, GUI_UI_ICON_TOPRIGHT_BORDER_Y, rect.x1 + rect.x2 - 7, rect.y1 + 26, GUI_UI_ICON_TOPRIGHT_BORDER_W, GUI_UI_ICON_TOPRIGHT_BORDER_H);
	renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BOTRIGHT_BORDER_X, GUI_UI_ICON_BOTRIGHT_BORDER_Y, rect.x1 + rect.x2 - 7, rect.y1 + rect.y2 - 25, GUI_UI_ICON_BOTRIGHT_BORDER_W, GUI_UI_ICON_BOTRIGHT_BORDER_H);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_DIVIDER_X, GUI_UI_ICON_HORIZONTAL_DIVIDER_Y, GUI_UI_ICON_HORIZONTAL_DIVIDER_W, GUI_UI_ICON_HORIZONTAL_DIVIDER_H, rect.x1 + 7, rect.y1 + 26, rect.x2 - 14, 3);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_DIVIDER_X, GUI_UI_ICON_HORIZONTAL_DIVIDER_Y, GUI_UI_ICON_HORIZONTAL_DIVIDER_W, GUI_UI_ICON_HORIZONTAL_DIVIDER_H, rect.x1 + 7, rect.y1 + rect.y2 - 25, rect.x2 - 14, 3);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_DIVIDER_X, GUI_UI_ICON_VERTICAL_DIVIDER_Y, GUI_UI_ICON_VERTICAL_DIVIDER_W, GUI_UI_ICON_VERTICAL_DIVIDER_H, rect.x1 + 4, rect.y1 + 29, 3, rect.y2 - 54);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_DIVIDER_X, GUI_UI_ICON_VERTICAL_DIVIDER_Y, GUI_UI_ICON_VERTICAL_DIVIDER_W, GUI_UI_ICON_VERTICAL_DIVIDER_H, rect.x1 + rect.x2 - 7, rect.y1 + 29, 3, rect.y2 - 54);

	renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_EXTRA_BORDER_X, GUI_UI_ICON_EXTRA_BORDER_Y, rect.x1 + rect.x2 - 2, rect.y1 + 21, GUI_UI_ICON_EXTRA_BORDER_W, GUI_UI_ICON_EXTRA_BORDER_H);
	renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_EXTRA_BORDER_X, GUI_UI_ICON_EXTRA_BORDER_Y, rect.x1, rect.y1 + rect.y2 - 2, GUI_UI_ICON_EXTRA_BORDER_W, GUI_UI_ICON_EXTRA_BORDER_H);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_X, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_Y, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_W, GUI_UI_ICON_HORIZONTAL_LINE_BRIGHT_H, rect.x1, rect.y1 + 21, rect.x2 - 2, 2);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_X, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_Y, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_W, GUI_UI_ICON_VERTICAL_LINE_BRIGHT_H, rect.x1, rect.y1 + 23, 2, rect.y2 - 25);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_HORIZONTAL_LINE_DARK_X, GUI_UI_ICON_HORIZONTAL_LINE_DARK_Y, GUI_UI_ICON_HORIZONTAL_LINE_DARK_W, GUI_UI_ICON_HORIZONTAL_LINE_DARK_H, rect.x1 + 2, rect.y1 + rect.y2 - 2, rect.x2 - 2, 2);
	renderer->drawPictureRepeat(GUI_UI_IMAGE, GUI_UI_ICON_VERTICAL_LINE_DARK_X, GUI_UI_ICON_VERTICAL_LINE_DARK_Y, GUI_UI_ICON_VERTICAL_LINE_DARK_W, GUI_UI_ICON_VERTICAL_LINE_DARK_H, rect.x1 + rect.x2 - 2, rect.y1 + 23, 2, rect.y2 - 25);

	Channel* selectedchannel = getCurrentChannel();
	if(!selectedchannel)
		return;

	selectedchannel->highlightTime = 0;
	selectedchannel->unreadMessage = false;

	std::pair<size_t, size_t> calculatedPages = calculateChannelPages(rect);
	if(calculatedPages.first > 0)
	{
		if(m_currentPage > calculatedPages.first)
			m_currentPage = calculatedPages.first;

		if(m_currentPage != calculatedPages.first)
		{
			Uint8 buttonType = 0;
			for(size_t it = m_currentPage + calculatedPages.second, end = m_channels.size(); it != end; ++it)
			{
				Channel& currentChannel = m_channels[it];
				if(currentChannel.highlightTime >= g_frameTime)
				{
					buttonType = 2;
					break;
				}
				else if(currentChannel.unreadMessage)
					buttonType = 1;
			}
			if(buttonType == 2)
			{
				if(m_buttonNext == 1)
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_RIGHT_UPDATE_DOWN_X, GUI_UI_ICON_BROWSE_RIGHT_UPDATE_DOWN_Y, rect.x1 + rect.x2 - 82, rect.y1 + 5, GUI_UI_ICON_BROWSE_RIGHT_UPDATE_DOWN_W, GUI_UI_ICON_BROWSE_RIGHT_UPDATE_DOWN_H);
				else
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_RIGHT_UPDATE_UP_X, GUI_UI_ICON_BROWSE_RIGHT_UPDATE_UP_Y, rect.x1 + rect.x2 - 82, rect.y1 + 5, GUI_UI_ICON_BROWSE_RIGHT_UPDATE_UP_W, GUI_UI_ICON_BROWSE_RIGHT_UPDATE_UP_H);
			}
			else if(buttonType == 1)
			{
				if(m_buttonNext == 1)
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_RIGHT_FLASH_DOWN_X, GUI_UI_ICON_BROWSE_RIGHT_FLASH_DOWN_Y, rect.x1 + rect.x2 - 82, rect.y1 + 5, GUI_UI_ICON_BROWSE_RIGHT_FLASH_DOWN_W, GUI_UI_ICON_BROWSE_RIGHT_FLASH_DOWN_H);
				else
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_RIGHT_FLASH_UP_X, GUI_UI_ICON_BROWSE_RIGHT_FLASH_UP_Y, rect.x1 + rect.x2 - 82, rect.y1 + 5, GUI_UI_ICON_BROWSE_RIGHT_FLASH_UP_W, GUI_UI_ICON_BROWSE_RIGHT_FLASH_UP_H);
			}
			else
			{
				if(m_buttonNext == 1)
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_RIGHT_DOWN_X, GUI_UI_ICON_BROWSE_RIGHT_DOWN_Y, rect.x1 + rect.x2 - 82, rect.y1 + 5, GUI_UI_ICON_BROWSE_RIGHT_DOWN_W, GUI_UI_ICON_BROWSE_RIGHT_DOWN_H);
				else
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_RIGHT_UP_X, GUI_UI_ICON_BROWSE_RIGHT_UP_Y, rect.x1 + rect.x2 - 82, rect.y1 + 5, GUI_UI_ICON_BROWSE_RIGHT_UP_W, GUI_UI_ICON_BROWSE_RIGHT_UP_H);
			}
		}
		if(m_currentPage != 0)
		{
			Uint8 buttonType = 0;
			for(size_t it = 0, end = m_currentPage; it != end; ++it)
			{
				Channel& currentChannel = m_channels[it];
				if(currentChannel.highlightTime >= g_frameTime)
				{
					buttonType = 2;
					break;
				}
				else if(currentChannel.unreadMessage)
					buttonType = 1;
			}
			if(buttonType == 2)
			{
				if(m_buttonPrevious == 1)
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_LEFT_UPDATE_DOWN_X, GUI_UI_ICON_BROWSE_LEFT_UPDATE_DOWN_Y, rect.x1, rect.y1 + 5, GUI_UI_ICON_BROWSE_LEFT_UPDATE_DOWN_W, GUI_UI_ICON_BROWSE_LEFT_UPDATE_DOWN_H);
				else
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_LEFT_UPDATE_UP_X, GUI_UI_ICON_BROWSE_LEFT_UPDATE_UP_Y, rect.x1, rect.y1 + 5, GUI_UI_ICON_BROWSE_LEFT_UPDATE_UP_W, GUI_UI_ICON_BROWSE_LEFT_UPDATE_UP_H);
			}
			else if(buttonType == 1)
			{
				if(m_buttonPrevious == 1)
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_LEFT_FLASH_DOWN_X, GUI_UI_ICON_BROWSE_LEFT_FLASH_DOWN_Y, rect.x1, rect.y1 + 5, GUI_UI_ICON_BROWSE_LEFT_FLASH_DOWN_W, GUI_UI_ICON_BROWSE_LEFT_FLASH_DOWN_H);
				else
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_LEFT_FLASH_UP_X, GUI_UI_ICON_BROWSE_LEFT_FLASH_UP_Y, rect.x1, rect.y1 + 5, GUI_UI_ICON_BROWSE_LEFT_FLASH_UP_W, GUI_UI_ICON_BROWSE_LEFT_FLASH_UP_H);
			}
			else
			{
				if(m_buttonPrevious == 1)
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_LEFT_DOWN_X, GUI_UI_ICON_BROWSE_LEFT_DOWN_Y, rect.x1, rect.y1 + 5, GUI_UI_ICON_BROWSE_LEFT_DOWN_W, GUI_UI_ICON_BROWSE_LEFT_DOWN_H);
				else
					renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_BROWSE_LEFT_UP_X, GUI_UI_ICON_BROWSE_LEFT_UP_Y, rect.x1, rect.y1 + 5, GUI_UI_ICON_BROWSE_LEFT_UP_W, GUI_UI_ICON_BROWSE_LEFT_UP_H);
			}
		}
	}
	else
		m_currentPage = 0;

	Sint32 posX = rect.x1 + 18;
	renderer->setClipRect(posX, rect.y1 + 5, rect.x2 - 100, 18);
	for(size_t it = m_currentPage, end = UTIL_min<size_t>(m_channels.size(), it + calculatedPages.second + 1); it != end; ++it)
	{
		Channel& currentChannel = m_channels[it];
		if(m_selectedChannel == it)
		{
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_ACTIVE_CHANNEL_X, GUI_UI_ICON_ACTIVE_CHANNEL_Y, posX, rect.y1 + 5, GUI_UI_ICON_ACTIVE_CHANNEL_W, GUI_UI_ICON_ACTIVE_CHANNEL_H);
			g_engine.drawFont(CLIENT_FONT_OUTLINED, posX + 48, rect.y1 + 9, currentChannel.channelName, 223, 223, 223, CLIENT_FONT_ALIGN_CENTER);
		}
		else
		{
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_UNACTIVE_CHANNEL_X, GUI_UI_ICON_UNACTIVE_CHANNEL_Y, posX, rect.y1 + 5, GUI_UI_ICON_UNACTIVE_CHANNEL_W, GUI_UI_ICON_UNACTIVE_CHANNEL_H);
			if(currentChannel.highlightTime >= g_frameTime)
				g_engine.drawFont(CLIENT_FONT_OUTLINED, posX + 48, rect.y1 + 9, currentChannel.channelName, 250, 250, 250, CLIENT_FONT_ALIGN_CENTER);
			else if(currentChannel.unreadMessage)
				g_engine.drawFont(CLIENT_FONT_OUTLINED, posX + 48, rect.y1 + 9, currentChannel.channelName, 250, 96, 96, CLIENT_FONT_ALIGN_CENTER);
			else
				g_engine.drawFont(CLIENT_FONT_OUTLINED, posX + 48, rect.y1 + 9, currentChannel.channelName, 127, 127, 127, CLIENT_FONT_ALIGN_CENTER);
		}
		posX += 96;
	}
	renderer->disableClipRect();

	if(m_ignoreListTime >= g_frameTime)
		renderer->drawPicture(GUI_UI_IMAGE, (m_ignoreListStatus == 1 ? GUI_UI_ICON_IGNORE_WINDOW_DOWN_X : GUI_UI_ICON_IGNORE_WINDOW_FLASH_X), (m_ignoreListStatus == 1 ? GUI_UI_ICON_IGNORE_WINDOW_DOWN_Y : GUI_UI_ICON_IGNORE_WINDOW_FLASH_Y), rect.x1 + rect.x2 - 16, rect.y1 + 5, GUI_UI_ICON_IGNORE_WINDOW_FLASH_W, GUI_UI_ICON_IGNORE_WINDOW_FLASH_H);
	else
		renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_IGNORE_WINDOW_UP_X, (m_ignoreListStatus == 1 ? GUI_UI_ICON_IGNORE_WINDOW_DOWN_Y : GUI_UI_ICON_IGNORE_WINDOW_UP_Y), rect.x1 + rect.x2 - 16, rect.y1 + 5, GUI_UI_ICON_IGNORE_WINDOW_UP_W, GUI_UI_ICON_IGNORE_WINDOW_UP_H);

	renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_REQUEST_CHANNELS_UP_X, (m_channelListStatus == 1 ? GUI_UI_ICON_REQUEST_CHANNELS_DOWN_Y : GUI_UI_ICON_REQUEST_CHANNELS_UP_Y), rect.x1 + rect.x2 - 32, rect.y1 + 5, GUI_UI_ICON_REQUEST_CHANNELS_UP_W, GUI_UI_ICON_REQUEST_CHANNELS_UP_H);
	if(selectedchannel->channelClosable)
	{
		renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_CONSOLE_MESSAGES_UP_X, ((selectedchannel->workAsServerLog || m_serverLogStatus == 1) ? GUI_UI_ICON_CONSOLE_MESSAGES_DOWN_Y : GUI_UI_ICON_CONSOLE_MESSAGES_UP_Y), rect.x1 + rect.x2 - 48, rect.y1 + 5, GUI_UI_ICON_CONSOLE_MESSAGES_UP_W, GUI_UI_ICON_CONSOLE_MESSAGES_UP_H);
		renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_CLOSE_CHANNEL_UP_X, (m_closeChannelStatus == 1 ? GUI_UI_ICON_CLOSE_CHANNEL_DOWN_Y : GUI_UI_ICON_CLOSE_CHANNEL_UP_Y), rect.x1 + rect.x2 - 64, rect.y1 + 5, GUI_UI_ICON_CLOSE_CHANNEL_UP_W, GUI_UI_ICON_CLOSE_CHANNEL_UP_H);
	}
	else
	{
		if(m_volumeAdjustement == VOLUME_SAY)
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_VOLUME1_WINDOW_UP_X, (m_volumeStatus == 1 ? GUI_UI_ICON_VOLUME1_WINDOW_DOWN_Y : GUI_UI_ICON_VOLUME1_WINDOW_UP_Y), rect.x1 + 5, rect.y1 + rect.y2 - 20, GUI_UI_ICON_VOLUME1_WINDOW_UP_W, GUI_UI_ICON_VOLUME1_WINDOW_UP_H);
		else if(m_volumeAdjustement == VOLUME_YELL)
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_VOLUME2_WINDOW_UP_X, (m_volumeStatus == 1 ? GUI_UI_ICON_VOLUME2_WINDOW_DOWN_Y : GUI_UI_ICON_VOLUME2_WINDOW_UP_Y), rect.x1 + 5, rect.y1 + rect.y2 - 20, GUI_UI_ICON_VOLUME2_WINDOW_UP_W, GUI_UI_ICON_VOLUME2_WINDOW_UP_H);
		else if(m_volumeAdjustement == VOLUME_WHISPER)
			renderer->drawPicture(GUI_UI_IMAGE, GUI_UI_ICON_VOLUME0_WINDOW_UP_X, (m_volumeStatus == 1 ? GUI_UI_ICON_VOLUME0_WINDOW_DOWN_Y : GUI_UI_ICON_VOLUME0_WINDOW_UP_Y), rect.x1 + 5, rect.y1 + rect.y2 - 20, GUI_UI_ICON_VOLUME0_WINDOW_UP_W, GUI_UI_ICON_VOLUME0_WINDOW_UP_H);
	}

	if(!m_textbox->isActive())
		m_textbox->activate();

	iRect tRect = iRect(rect.x1 + 23, rect.y1 + rect.y2 - 20, rect.x2 - 27, 16);
	m_textbox->setRect(tRect);
	m_textbox->render();

	iRect cRect = iRect(rect.x1 + 6, rect.y1 + 28, rect.x2 - 12, rect.y2 - 52);
	selectedchannel->channelConsole->setRect(cRect);
	selectedchannel->channelConsole->render();
}
