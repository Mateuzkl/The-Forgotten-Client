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

#ifndef __FILE_CHAT_h_
#define __FILE_CHAT_h_

#include "../defines.h"

#define CHANNEL_ID_DEFAULT 0xFFFFFFF0
#define CHANNEL_ID_SERVERLOG 0xFFFFFFF1
#define CHANNEL_ID_NPC 0xFFFFFFF2
#define CHANNEL_ID_RVR 0xFFFFFFF3

#define CHANNEL_NAME_DEFAULT_LEGACY "Default"
#define CHANNEL_NAME_DEFAULT "Local Chat"
#define CHANNEL_NAME_SERVERLOG "Server Log"
#define CHANNEL_NAME_NPC "NPCs"
#define CHANNEL_NAME_RVR "Report Rule Violation"

class GUI_Console;
struct Channel
{
	Channel(std::string channelName, Uint32 channelId, bool closable, bool privateChannel);
	~Channel();

	// non-copyable
	Channel(const Channel&) = delete;
	Channel& operator=(const Channel&) = delete;

	// moveable
	Channel(Channel&& rhs) noexcept;
	Channel& operator=(Channel&& rhs) noexcept;

	std::string channelName;
	std::unique_ptr<GUI_Console> channelConsole;
	Uint32 channelId;
	Uint32 highlightTime = 0;
	bool workAsServerLog = false;
	bool channelClosable;
	bool privateChannel;
	bool alreadyClosed = false;
	bool unreadMessage = false;
};

enum Volumes
{
	VOLUME_SAY = 0,
	VOLUME_YELL = 1,
	VOLUME_WHISPER = 2
};

enum MessageFlags
{
	MESSAGE_FLAG_REPORTNAME = (1 << 0),
	MESSAGE_FLAG_REPORTSTATEMENT = (1 << 1)
};

class GUI_TextBox;
class Chat
{
	public:
		Chat();
		~Chat();

		// non-copyable
		Chat(const Chat&) = delete;
		Chat& operator=(const Chat&) = delete;

		// non-moveable
		Chat(Chat&&) = delete;
		Chat& operator=(Chat&&) = delete;

		void clear();
		void gameStart();
		void navigateHistory(Sint32 direction);
		void ignoreListEvent();

		void setOwnPrivateChannel(Uint32 channelId) {m_ownPrivatechannel = channelId;}
		void openPrivateChannel(const std::string& receiver);
		void openChannel(Uint32 channelId, const std::string channelName, bool privateChannel = false, bool closable = true);
		void closeChannel(Uint32 channelId);
		void leaveChannel(Uint32 channelId);
		void addChannelMessage(Uint32 channelId, MessageMode mode, Uint32 statementId, const std::string& playerName, Uint16 playerLevel, const std::string& text, time_t timestamp);
		void sendMessage();
		Channel* getPrivateChannel(const std::string& receiver);
		void switchToNextChannel();
		void switchToPreviousChannel();
		void setCurrentChannel(Uint32 channelId);
		Channel* getChannel(Uint32 channelId);
		Channel* getCurrentChannel();
		GUI_TextBox* getTextBox() {return m_textbox;}

		Uint32 getOwnPrivateChannel() {return m_ownPrivatechannel;}
		Uint32 getHelpChannelId();

		std::pair<size_t, size_t> calculateChannelPages(iRect& rect);
		void onTextInput(const char* textInput);
		void onKeyDown(SDL_Event& event);
		void onKeyUp(SDL_Event& event);
		void onLMouseDown(iRect& rect, Sint32 x, Sint32 y);
		void onLMouseUp(iRect& rect, Sint32 x, Sint32 y);
		void onRMouseDown(iRect& rect, Sint32 x, Sint32 y);
		void onRMouseUp(iRect& rect, Sint32 x, Sint32 y);
		void onWheel(iRect& rect, Sint32 x, Sint32 y, bool wheelUP);
		void onMouseMove(iRect& rect, Sint32 x, Sint32 y);
		void render(iRect& rect);

	protected:
		std::list<std::string> m_savedMessages;
		std::vector<Channel> m_channels;
		GUI_TextBox* m_textbox;
		size_t m_selectedChannel = 0;
		size_t m_currentPage = 0;
		Sint32 m_resizingY = 0;
		Sint32 m_historyNavigator = SDL_MIN_SINT32;
		Uint32 m_ownPrivatechannel = SDL_static_cast(Uint32, -1);
		Uint32 m_ignoreListTime = 0;
		Uint8 m_ignoreListStatus = 0;
		Uint8 m_channelListStatus = 0;
		Uint8 m_serverLogStatus = 0;
		Uint8 m_closeChannelStatus = 0;
		Uint8 m_volumeStatus = 0;
		Uint8 m_volumeAdjustement = VOLUME_SAY;
		Uint8 m_buttonNext = 0;
		Uint8 m_buttonPrevious = 0;
		Uint8 m_chatToggleStatus = 0;
		bool m_haveRMouse = false;
		bool m_bMouseResizing = false;
};

#endif /* __FILE_CHAT_h_ */
