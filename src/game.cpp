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

#include "game.h"
#include "engine.h"
#include "map.h"
#include "tile.h"
#include "creature.h"
#include "protocolgame.h"
#include "automap.h"
#include "container.h"

#include "GUI/itemUI.h"
#include "GUI/Chat.h"

Game g_game;
extern Map g_map;
extern Chat g_chat;
extern Engine g_engine;
extern Automap g_automap;
extern Connection* g_connection;
extern Uint32 g_frameTime;

Game::Game() : m_autoWalkDestination(0xFFFF, 0xFFFF, 0xFF), m_limitWalkDestination(0xFFFF, 0xFFFF, 0xFF), m_lastCancelWalkPos(0xFFFF, 0xFFFF, 0xFF)
{
	for(Uint8 skill = 0; skill < Skills_LastAdditionalSkill; ++skill)
	{
		m_playerSkillsLevel[skill] = 10;
		m_playerSkillsBaseLevel[skill] = 10;
		m_playerSkillsLevelPercent[skill] = 0;
	}
}

Game::~Game()
{
	reset();
}

void Game::reset()
{
	m_autoWalkDestination = Position(0xFFFF, 0xFFFF, 0xFF);
	m_limitWalkDestination = Position(0xFFFF, 0xFFFF, 0xFF);
	m_lastCancelWalkPos = Position(0xFFFF, 0xFFFF, 0xFF);
	m_cancelWalkCounter = 0;
	m_autoWalkDirections.clear();

	m_playerBaseXpGain = 100;
	m_playerTournamentFactor = 0;
	m_playerVoucherXpGain = 0;
	m_playerGrindingXpGain = 0;
	m_playerStoreXpGain = 0;
	m_playerHuntingXpGain = 100;

	m_playerMovement = DIRECTION_INVALID;
	m_playerCurrentDir = DIRECTION_INVALID;
	m_playerLastDir = DIRECTION_INVALID;
	for(size_t i = SLOT_HEAD; i < SLOT_LAST; ++i)
	{
		if(m_inventoryItem[i])
		{
			delete m_inventoryItem[i];
			m_inventoryItem[i] = NULL;
		}
	}
	for(size_t i = 0; i < GAME_MAX_CONTAINERS; ++i)
	{
		if(m_containers[i])
		{
			delete m_containers[i];
			m_containers[i] = NULL;
		}
	}
}

void Game::clientChangeVersion(Uint32 clientVersion, Uint32 fileVersion)
{
	m_gameFeatures.reset();
	enableGameFeature(GAME_FEATURE_UPDATE_TILE);
	if(clientVersion >= 770)
	{
		enableGameFeature(GAME_FEATURE_XTEA);
		enableGameFeature(GAME_FEATURE_RSA1024);
		enableGameFeature(GAME_FEATURE_MESSAGE_STATEMENT);
		enableGameFeature(GAME_FEATURE_LOOKTYPE_U16);
	}
	if(clientVersion >= 780)
	{
		enableGameFeature(GAME_FEATURE_NEWFLUIDS);
		enableGameFeature(GAME_FEATURE_MESSAGE_LEVEL);
		enableGameFeature(GAME_FEATURE_PLAYERICONS_U16);
		enableGameFeature(GAME_FEATURE_ADDONS);
		enableGameFeature(GAME_FEATURE_STAMINA);
		enableGameFeature(GAME_FEATURE_NEWOUTFITS);
	}
	if(clientVersion >= 790)
		enableGameFeature(GAME_FEATURE_WRITABLE_DATE);
	if(clientVersion >= 820)
		enableGameFeature(GAME_FEATURE_NPC_INTERFACE);
	if(clientVersion >= 830)
	{
		enableGameFeature(GAME_FEATURE_CHECKSUM);
		enableGameFeature(GAME_FEATURE_ACCOUNT_NAME);
	}
	if(clientVersion >= 840)
	{
		enableGameFeature(GAME_FEATURE_DOUBLE_CAPACITY);
		enableGameFeature(GAME_FEATURE_SERVER_SENDFIRST);
	}
	if(clientVersion >= 841)
	{
		disableGameFeature(GAME_FEATURE_UPDATE_TILE);
		enableGameFeature(GAME_FEATURE_TILE_ADDTHING_STACKPOS);
	}
	if(clientVersion >= 854)
		enableGameFeature(GAME_FEATURE_CREATURE_EMBLEM);
	if(clientVersion >= 860)
	{
		enableGameFeature(GAME_FEATURE_ATTACK_SEQUENCE);
		enableGameFeature(GAME_FEATURE_MOUNTS);
		enableGameFeature(GAME_FEATURE_EFFECTS_U16);
		enableGameFeature(GAME_FEATURE_DISTANCEEFFECTS_U16);
		enableGameFeature(GAME_FEATURE_DOUBLE_HEALTH);
		enableGameFeature(GAME_FEATURE_OFFLINE_TRAINING);
		enableGameFeature(GAME_FEATURE_BASE_SKILLS);
		enableGameFeature(GAME_FEATURE_BASE_SKILL_U16);
		enableGameFeature(GAME_FEATURE_ADDITIONAL_SKILLS);
		enableGameFeature(GAME_FEATURE_FRAMEGROUPS);
		enableGameFeature(GAME_FEATURE_ENHANCED_ANIMATIONS);
		enableGameFeature(GAME_FEATURE_EXTENDED_CLIENT_PING);
		enableGameFeature(GAME_FEATURE_EXTENDED_SPRITES);
		enableGameFeature(GAME_FEATURE_DOUBLE_PLAYER_GOODS_MONEY);
		enableGameFeature(GAME_FEATURE_CREATURE_ICONS);
		enableGameFeature(GAME_FEATURE_PURSE_SLOT);
		enableGameFeature(GAME_FEATURE_PREY);
	}
	if(clientVersion >= 862)
		enableGameFeature(GAME_FEATURE_DEATH_PENALTY);
	if(clientVersion >= 870)
	{
		enableGameFeature(GAME_FEATURE_MOUNTS);
		enableGameFeature(GAME_FEATURE_DOUBLE_EXPERIENCE);
		enableGameFeature(GAME_FEATURE_SPELL_LIST);
	}
	if(clientVersion >= 872)
		enableGameFeature(GAME_FEATURE_CHAT_PLAYERLIST);
	if(clientVersion >= 910)
	{
		enableGameFeature(GAME_FEATURE_TOTAL_CAPACITY);
		enableGameFeature(GAME_FEATURE_BASE_SKILLS);
		enableGameFeature(GAME_FEATURE_REGENERATION_TIME);
		enableGameFeature(GAME_FEATURE_ITEM_ANIMATION_PHASES);
		enableGameFeature(GAME_FEATURE_ENVIRONMENT_EFFECTS);
		enableGameFeature(GAME_FEATURE_NPC_NAME_ON_TRADE);
	}
	if(clientVersion >= 940)
		enableGameFeature(GAME_FEATURE_MARKET);
	if(clientVersion >= 953)
		enableGameFeature(GAME_FEATURE_PING);
	if(clientVersion >= 953 && clientVersion < 1080)
		enableGameFeature(GAME_FEATURE_PURSE_SLOT);
	if(clientVersion >= 960)
		enableGameFeature(GAME_FEATURE_OFFLINE_TRAINING);
	if(fileVersion >= 960)
		enableGameFeature(GAME_FEATURE_EXTENDED_SPRITES);
	if(clientVersion >= 961)
		enableGameFeature(GAME_FEATURE_LOOKATCREATURE);
	if(clientVersion >= 963)
		enableGameFeature(GAME_FEATURE_ADDITIONAL_VIPINFO);
	if(clientVersion >= 971)
	{
		enableGameFeature(GAME_FEATURE_PREVIEW_STATE);
		enableGameFeature(GAME_FEATURE_CLIENT_VERSION);
	}
	if(clientVersion >= 980)
	{
		enableGameFeature(GAME_FEATURE_KEEP_CONNECTION_AFTER_DEATH);
		enableGameFeature(GAME_FEATURE_LOGIN_PENDING);
		enableGameFeature(GAME_FEATURE_VIP_STATUS);
		enableGameFeature(GAME_FEATURE_NEWSPEED_LAW);
	}
	if(clientVersion >= 984)
	{
		enableGameFeature(GAME_FEATURE_BROWSEFIELD);
		enableGameFeature(GAME_FEATURE_CONTAINER_PAGINATION);
	}
	if(clientVersion >= 1000)
	{
		enableGameFeature(GAME_FEATURE_PVP_MODE);
		enableGameFeature(GAME_FEATURE_ITEM_MARK);
		enableGameFeature(GAME_FEATURE_CREATURE_MARK);
		enableGameFeature(GAME_FEATURE_CREATURE_TYPE);
	}
	if(clientVersion >= 1035)
		enableGameFeature(GAME_FEATURE_DOUBLE_SKILLS);
	if(clientVersion >= 1036)
	{
		enableGameFeature(GAME_FEATURE_CREATURE_ICONS);
		enableGameFeature(GAME_FEATURE_HIDE_NPC_NAMES);
	}
	if(clientVersion >= 1038)
		enableGameFeature(GAME_FEATURE_PREMIUM_EXPIRATION);
	if(fileVersion >= 1050)
		enableGameFeature(GAME_FEATURE_ENHANCED_ANIMATIONS);
	if(clientVersion >= 1053)
		enableGameFeature(GAME_FEATURE_UNJUSTIFIED_POINTS);
	if(clientVersion >= 1054)
		enableGameFeature(GAME_FEATURE_EXPERIENCE_BONUS);
	if(clientVersion >= 1055)
		enableGameFeature(GAME_FEATURE_DEATH_TYPE);
	if(fileVersion >= 1057)
		enableGameFeature(GAME_FEATURE_FRAMEGROUPS);
	if(clientVersion >= 1061)
		enableGameFeature(GAME_FEATURE_RENDER_INFORMATION);
	if(clientVersion >= 1071)
		enableGameFeature(GAME_FEATURE_CONTENT_REVISION);
	if(clientVersion >= 1072)
		enableGameFeature(GAME_FEATURE_AUTHENTICATOR);
	if(clientVersion >= 1074)
		enableGameFeature(GAME_FEATURE_SESSIONKEY);
	if(clientVersion >= 1076)
		enableGameFeature(GAME_FEATURE_EQUIP_HOTKEY);
	if(clientVersion >= 1080)
		enableGameFeature(GAME_FEATURE_STORE);
	if(clientVersion >= 1092)
	{
		enableGameFeature(GAME_FEATURE_WRAPPABLE);
		enableGameFeature(GAME_FEATURE_STORE_INBOX);
		enableGameFeature(GAME_FEATURE_STORE_SERVICETYPE);
	}
	if(clientVersion >= 1093)
		enableGameFeature(GAME_FEATURE_STORE_HIGHLIGHTS);
	if(clientVersion >= 1094)
		enableGameFeature(GAME_FEATURE_ADDITIONAL_SKILLS);
	if(clientVersion >= 1097)
	{
		enableGameFeature(GAME_FEATURE_DETAILED_EXPERIENCE_BONUS);
		enableGameFeature(GAME_FEATURE_STORE_HIGHLIGHTS2);
	}
	if(fileVersion >= 1100)
		enableGameFeature(GAME_FEATURE_NEWFILES_STRUCTURE);
	if(clientVersion >= 1102)
	{
		enableGameFeature(GAME_FEATURE_CLIENT_SENDSERVERNAME);
		enableGameFeature(GAME_FEATURE_PREY);
		enableGameFeature(GAME_FEATURE_IMBUING);
	}
	if(clientVersion >= 1110)
	{
		enableGameFeature(GAME_FEATURE_VIP_GROUPS);
		enableGameFeature(GAME_FEATURE_INSPECTION);
	}
	if(clientVersion >= 1111)
	{
		disableGameFeature(GAME_FEATURE_CHECKSUM);
		enableGameFeature(GAME_FEATURE_PROTOCOLSEQUENCE);
	}
	if(clientVersion >= 1120)
	{
		enableGameFeature(GAME_FEATURE_BLESS_DIALOG);
		enableGameFeature(GAME_FEATURE_QUEST_TRACKER);
	}
	if(clientVersion >= 1130)
		enableGameFeature(GAME_FEATURE_GAMETIME);
	if(clientVersion >= 1132)
		enableGameFeature(GAME_FEATURE_COMPEDIUM);
	if(clientVersion >= 1140)
	{
		disableGameFeature(GAME_FEATURE_PLAYERICONS_U16);
		enableGameFeature(GAME_FEATURE_PLAYERICONS_U32);
		enableGameFeature(GAME_FEATURE_REWARD_WALL);
		enableGameFeature(GAME_FEATURE_ANALYTICS);
	}
	if(clientVersion >= 1150)
	{
		enableGameFeature(GAME_FEATURE_QUICK_LOOT);
		enableGameFeature(GAME_FEATURE_EXTENDED_CAPACITY);
		enableGameFeature(GAME_FEATURE_CYCLOPEDIA_MONSTERS);
	}
	if(clientVersion >= 1180)
	{
		enableGameFeature(GAME_FEATURE_STASH);
		enableGameFeature(GAME_FEATURE_CYCLOPEDIA_MAPANDDETAILS);
	}
	if(clientVersion >= 1185)
	{
		disableGameFeature(GAME_FEATURE_ENVIRONMENT_EFFECTS);
		disableGameFeature(GAME_FEATURE_ITEM_MARK);
	}
	if(clientVersion >= 1200)
		enableGameFeature(GAME_FEATURE_DOUBLE_PERCENT_SKILLS);
	if(clientVersion >= 1215)
		enableGameFeature(GAME_FEATURE_TOURNAMENTS);
	if(clientVersion >= 1220)
		enableGameFeature(GAME_FEATURE_ACCOUNT_EMAIL);
}

void Game::processGMActions(std::vector<Uint8> privileges)
{
	(void)privileges;
}

void Game::processLoginWait(const std::string& message, Uint8 time)
{
	(void)message;
	(void)time;
}

void Game::processDeath(Uint8 deathType, Uint8 penalty)
{
	processTextMessage(MessageLogin, "You are dead.");
	UTIL_deathWindow(deathType, penalty);
}

void Game::processContainerOpen(Uint8 containerId, ItemUI* item, const std::string& name, Uint8 capacity, bool hasParent, bool canUseDepotSearch, bool isUnlocked, bool hasPages, Uint16 containerSize, Uint16 firstIndex, std::vector<ItemUI*>& itemVector)
{
	processContainerClose(containerId);
	m_containers[containerId] = new Container(containerId, item, name, capacity, hasParent, canUseDepotSearch, isUnlocked, hasPages, containerSize, firstIndex);
	m_containers[containerId]->setItems(itemVector);
	UTIL_createContainerWindow(containerId);
}

void Game::processContainerClose(Uint8 containerId)
{
	//Don't bother with ui - window will be automatically deleted or keeped(we need to maintain position if possible)
	if(m_containers[containerId])
	{
		delete m_containers[containerId];
		m_containers[containerId] = NULL;
	}
}

void Game::processContainerAddItem(Uint8 containerId, Uint16 slot, ItemUI* item)
{
	if(m_containers[containerId])
		m_containers[containerId]->addItem(slot, item);
}

void Game::processContainerTransformItem(Uint8 containerId, Uint16 slot, ItemUI* item)
{
	if(m_containers[containerId])
		m_containers[containerId]->transformItem(slot, item);
}

void Game::processContainerRemoveItem(Uint8 containerId, Uint16 slot, ItemUI* lastItem)
{
	if(m_containers[containerId])
		m_containers[containerId]->removeItem(slot, lastItem);
}

void Game::processTransformInventoryItem(Uint8 slot, ItemUI* item)
{
	if(m_inventoryItem[slot])
		delete m_inventoryItem[slot];
	m_inventoryItem[slot] = item;
	if(item)
	{
		Position position = Position(0xFFFF, SDL_static_cast(Uint16, slot) + 1, 0);
		item->setCurrentPosition(position);
	}
	UTIL_updateShopWindow();
}

void Game::processTradeOwn(const std::string& name, std::vector<ItemUI*>& itemVector)
{
	UTIL_createTradeWindow(false, name, SDL_reinterpret_cast(void*, &itemVector));
}

void Game::processTradeCounter(const std::string& name, std::vector<ItemUI*>& itemVector)
{
	UTIL_createTradeWindow(true, name, SDL_reinterpret_cast(void*, &itemVector));
}

void Game::processTradeClose()
{
	UTIL_closeTradeWindow();
}

void Game::processNPCTradeOpen(const std::string& name, Uint16 currencyId, std::vector<NPCTradeItems>& npcItems)
{
	UTIL_createShopWindow(name, currencyId, SDL_reinterpret_cast(void*, &npcItems));
}

void Game::processNPCTradeGoods(Uint64 playerMoney, NPCTradeGoods& npcGoods)
{
	UTIL_goodsShopWindow(playerMoney, SDL_reinterpret_cast(void*, &npcGoods));
}

void Game::processNPCTradeClose()
{
	UTIL_closeShopWindow();
}

void Game::processEditTextWindow(Uint32 windowId, ItemUI* item, Uint16 maxLen, const std::string& text, const std::string& writer, const std::string& date)
{
	UTIL_createReadWriteWindow(windowId, SDL_reinterpret_cast(void*, item), maxLen, text, writer, date);
}

void Game::processEditHouseWindow(Uint8 doorId, Uint32 windowId, const std::string& text)
{
	UTIL_createReadWriteWindow(doorId, windowId, text);
}

void Game::processTextMessage(MessageMode mode, const std::string& text, Uint32 channelId)
{
	switch(mode)
	{
		case MessageChannelManagement:
		{
			size_t pos1, pos2;
			std::string speaker;
			if((pos1 = text.find(" invites you to ", 0)) != std::string::npos)
				speaker = text.substr(0, pos1);
			else if((pos1 = text.find("You have been excluded from the channel ", 0)) != std::string::npos && (pos2 = text.find("'s Channel.", 0)) != std::string::npos)
				speaker = text.substr(pos1 + 40, pos2 - pos1 - 40);

			//check white list and black list
			if(UTIL_onWhiteList(speaker) || !UTIL_onBlackList(speaker))
			{
				Channel* channel = g_chat.getChannel(channelId);
				if(channel)
					g_chat.addChannelMessage(channelId, mode, 0, "", 0, text, time(NULL));
				else
					processTextMessage(MessageLook, text);
			}
			else
				g_chat.ignoreListEvent();
		}
		break;
		case MessageGuild:
		case MessagePartyManagement:
		case MessageParty:
		{
			Channel* channel = g_chat.getChannel(channelId);
			if(channel)
				g_chat.addChannelMessage(channelId, mode, 0, "", 0, text, time(NULL));
			else
				processTextMessage(MessageLook, text);
		}
		break;
		case MessageWarning:
		{
			g_map.addOnscreenText(ONSCREEN_MESSAGE_CENTER_HIGH, mode, text);
			g_chat.addChannelMessage(CHANNEL_ID_SERVERLOG, mode, 0, "", 0, text, time(NULL));
		}
		break;
		case MessageFailure:
			g_map.addOnscreenText(ONSCREEN_MESSAGE_BOTTOM, mode, text);
			break;
		case MessageLogin:
		case MessageTutorialHint:
		case MessageBeyondLast:
		{
			g_map.addOnscreenText(ONSCREEN_MESSAGE_BOTTOM, mode, text);
			if(g_engine.hasShowEventMessages())
				g_chat.addChannelMessage(CHANNEL_ID_SERVERLOG, mode, 0, "", 0, text, time(NULL));
		}
		break;
		case MessageGame:
		case MessageReport:
		case MessageThankyou:
		{
			g_map.addOnscreenText(ONSCREEN_MESSAGE_CENTER_LOW, mode, text);
			if(g_engine.hasShowEventMessages())
				g_chat.addChannelMessage(CHANNEL_ID_SERVERLOG, mode, 0, "", 0, text, time(NULL));
		}
		break;
		case MessageStatus:
		{
			g_map.addOnscreenText(ONSCREEN_MESSAGE_BOTTOM, mode, text);
			if(g_engine.hasShowStatusMessages())
				g_chat.addChannelMessage(CHANNEL_ID_SERVERLOG, mode, 0, "", 0, text, time(NULL));
		}
		break;
		case MessageLook:
		case MessageLoot:
		case MessageTradeNpc:
		case MessageHotkeyUse:
		{
			g_map.addOnscreenText(ONSCREEN_MESSAGE_CENTER_HIGH, mode, text);
			if(g_engine.hasShowInfoMessages())
				g_chat.addChannelMessage(CHANNEL_ID_SERVERLOG, mode, 0, "", 0, text, time(NULL));
		}
		break;
		case MessageMarket://TODO: use real market message
			UTIL_messageBox("Market Message", text);
			break;
		case MessageDamageDealed:
		case MessageDamageReceived:
		case MessageHeal:
		case MessageMana:
		case MessageExp:
		{
			if(g_engine.hasShowStatusMessages())
				g_chat.addChannelMessage(CHANNEL_ID_SERVERLOG, mode, 0, "", 0, text, time(NULL));
		}
		break;
		case MessageDamageOthers:
		case MessageHealOthers:
		case MessageExpOthers:
		{
			if(g_engine.hasShowStatusOthersMessages())
				g_chat.addChannelMessage(CHANNEL_ID_SERVERLOG, mode, 0, "", 0, text, time(NULL));
		}
		break;
		case MessagePrivateFrom:
		case MessageMonsterSay:
		case MessageMonsterYell:
		case MessageBarkLow:
		case MessageBarkLoud:
		case MessageRed:
		case MessageBlue:
			g_chat.addChannelMessage(CHANNEL_ID_DEFAULT, mode, 0, "", 0, text, time(NULL));
			break;
		case MessageGamemasterBroadcast:
		case MessageGamemasterPrivateFrom:
		case MessageGameHighlight:
			g_chat.addChannelMessage(CHANNEL_ID_SERVERLOG, mode, 0, "", 0, text, time(NULL));
			break;
	}
}

void Game::processWalkCancel(Direction direction)
{
	Creature* player = g_map.getLocalCreature();
	if(player)
	{
		player->stopMove();
		player->turnDirection(direction);
	}

	if(m_autoWalkDestination.x != 0xFFFF)
	{
		Position& pos = g_map.getCentralPosition();
		if(pos == m_lastCancelWalkPos)
		{
			++m_cancelWalkCounter;
			if(m_cancelWalkCounter >= MAX_CANCEL_WALK_BEFORE_STOP_AUTOWALK)
			{
				stopAutoWalk();
				m_cancelWalkCounter = 0;
			}
			else
				startAutoWalk(m_autoWalkDestination);
		}
		else
		{
			m_lastCancelWalkPos = pos;
			m_cancelWalkCounter = 0;
			startAutoWalk(m_autoWalkDestination);
		}
	}
}

void Game::processWalkWait(Uint16 walkTime)
{
	Creature* player = g_map.getLocalCreature();
	if(player)
		player->addWalkTime(SDL_static_cast(Uint32, walkTime));
}

void Game::processOutfits(Uint16 lookType, Uint8 lookHead, Uint8 lookBody, Uint8 lookLegs, Uint8 lookFeet, Uint8 lookAddons, Uint16 lookMount, std::vector<OutfitDetail>& outfits, std::vector<MountDetail>& mounts)
{
	UTIL_createOutfitWindow(lookType, lookHead, lookBody, lookLegs, lookFeet, lookAddons, lookMount, outfits, mounts);
}

void Game::processTutorialHint(Uint8 tutorialId)
{
	UTIL_createTutorialHint(SDL_static_cast(Uint32, tutorialId));
}

void Game::processCancelTarget(Uint32 sequence)
{
	if(sequence == 0 || sequence == m_sequence)
	{
		m_attackId = 0;
		m_followId = 0;
	}
}

void Game::processSpellDelay(Uint8 spellId, Uint32 delay)
{
	(void)spellId;
	(void)delay;
}

void Game::processSpellGroupDelay(Uint8 groupId, Uint32 delay)
{
	(void)groupId;
	(void)delay;
}

void Game::processMultiUseDelay(Uint32 delay)
{
	(void)delay;
}

void Game::processPlayerModes(Uint8 fightMode, Uint8 chaseMode, Uint8 safeMode, Uint8 pvpMode)
{
	g_engine.setAttackMode(fightMode);
	g_engine.setChaseMode(chaseMode);
	g_engine.setSecureMode(safeMode);
	g_engine.setPvpMode(pvpMode);
}

void Game::processTalk(const std::string& name, Uint32 statementId, Uint16 playerLevel, MessageMode mode, std::string& text, Uint32 channelId, const Position& position)
{
	switch(mode)
	{
		case MessageSay:
		case MessageWhisper:
		case MessageSpell:
		{
			if(UTIL_onWhiteList(name) || !UTIL_onBlackList(name))
			{
				g_map.addStaticText(position, name, mode, text);
				g_chat.addChannelMessage(CHANNEL_ID_DEFAULT, mode, statementId, name, playerLevel, text, time(NULL));
			}
			else
				g_chat.ignoreListEvent();
		}
		break;
		case MessageYell:
		{
			if(UTIL_onWhiteList(name) || (!g_engine.getIgnoreYellingMessages() && !UTIL_onBlackList(name)))
			{
				g_map.addStaticText(position, name, mode, text);
				g_chat.addChannelMessage(CHANNEL_ID_DEFAULT, mode, statementId, name, playerLevel, text, time(NULL));
			}
			else
				g_chat.ignoreListEvent();
		}
		break;
		case MessageMonsterSay:
		case MessageMonsterYell:
		case MessageBarkLow:
		case MessageBarkLoud:
			g_map.addStaticText(position, name, mode, text);
			break;
		case MessageNpcFrom:
		case MessageNpcFromBlock:
		{
			UTIL_replaceString(text, "{", "\x0E\x20\xA0\xFF");
			UTIL_replaceString(text, "}", "\x0F");
			if(mode != MessageNpcFromBlock)//TODO: add MessageNpcFromBlock mode to static texts
				g_map.addStaticText(position, name, mode, text);

			g_chat.addChannelMessage(CHANNEL_ID_NPC, mode, statementId, name, playerLevel, text, time(NULL));
		}
		break;
		case MessageChannel:
		case MessageChannelManagement:
		case MessageChannelHighlight:
		case MessageGamemasterChannel:
			g_chat.addChannelMessage(channelId, mode, statementId, name, playerLevel, text, time(NULL));
			break;
		case MessageRVRAnswer:
		case MessageRVRContinue:
			g_chat.addChannelMessage(CHANNEL_ID_RVR, mode, statementId, name, playerLevel, text, time(NULL));
			break;
		case MessageRVRChannel:
			g_chat.addChannelMessage(CHANNEL_ID_RVR, mode, statementId, name, playerLevel, text, time(NULL) - SDL_static_cast(time_t, channelId));
			break;
		case MessagePrivateFrom:
		{
			if(UTIL_onWhiteList(name) || (!g_engine.getIgnorePrivateMessages() && !UTIL_onBlackList(name)))
			{
				if(g_engine.hasShowPrivateMessages())
					g_map.addOnscreenText(ONSCREEN_MESSAGE_TOP, mode, name + ":\n" + text);

				Channel* privateChannel = g_chat.getPrivateChannel(name);
				if(privateChannel)
					g_chat.addChannelMessage(privateChannel->channelId, mode, statementId, name, playerLevel, text, time(NULL));
				else
					g_chat.addChannelMessage(CHANNEL_ID_DEFAULT, mode, statementId, name, playerLevel, text, time(NULL));
			}
			else
				g_chat.ignoreListEvent();
		}
		break;
		case MessageGamemasterBroadcast:
		{
			g_map.addOnscreenText(ONSCREEN_MESSAGE_CENTER_LOW, mode, name + ":\n" + text);
			g_chat.addChannelMessage(CHANNEL_ID_SERVERLOG, mode, statementId, name, playerLevel, text, time(NULL));
		}
		break;
		case MessageGamemasterPrivateFrom:
		{
			g_map.addOnscreenText(ONSCREEN_MESSAGE_TOP, mode, name + ":\n" + text);
			Channel* privateChannel = g_chat.getPrivateChannel(name);
			if(privateChannel)
				g_chat.addChannelMessage(privateChannel->channelId, mode, statementId, name, playerLevel, text, time(NULL));
			else
				g_chat.addChannelMessage(CHANNEL_ID_SERVERLOG, mode, statementId, name, playerLevel, text, time(NULL));
		}
		break;
		default: break;
	}
}

void Game::processChannelList(std::vector<ChannelDetail>& channels)
{
	UTIL_createChannels(channels);
}

void Game::processOpenChannel(Uint16 channelId, const std::string& channelName)
{
	g_chat.openChannel(SDL_static_cast(Uint32, channelId), channelName);
	g_chat.setCurrentChannel(SDL_static_cast(Uint32, channelId));
}

void Game::processOpenPrivateChannel(const std::string& receiver)
{
	g_chat.openPrivateChannel(receiver);
}

void Game::processOpenOwnPrivateChannel(Uint16 channelId, const std::string& channelName)
{
	g_chat.setOwnPrivateChannel(SDL_static_cast(Uint32, channelId));
	g_chat.openChannel(SDL_static_cast(Uint32, channelId), channelName);
	g_chat.setCurrentChannel(SDL_static_cast(Uint32, channelId));
}

void Game::processCloseChannel(Uint16 channelId)
{
	g_chat.closeChannel(SDL_static_cast(Uint32, channelId));
}

void Game::processQuestLog(Uint32 questId, std::vector<QuestLogDetail>& questLogElements)
{
	UTIL_createQuestLog(questId, questLogElements);
}

void Game::processQuestLine(Uint16 questId, std::vector<QuestLineDetail>& questLineElements)
{
	UTIL_createQuestine(questId, questLineElements);
}

void Game::processVipGroups(std::vector<VipGroups>& groups, Uint8 createGroupsLeft)
{
	UTIL_changeVipGroups(groups, createGroupsLeft);
}

void Game::processVipAdd(Uint32 playerGUID, const std::string& playerName, const std::string& description, Uint32 iconId, bool notifyLogin, Uint8 status, std::vector<Uint8>& groups)
{
	UTIL_addVipPlayer(playerGUID, playerName, description, iconId, notifyLogin, status, groups);
}

void Game::processVipStatusChange(Uint32 playerGUID, Uint8 status)
{
	UTIL_changeVipStatus(playerGUID, status);
}

void Game::processModalWindow(Uint32 windowId, bool priority, const std::string& title, const std::string& message, Uint8 enterButtonId, Uint8 escapeButtonId, std::vector<std::pair<std::string, Uint8>>& buttons, std::vector<std::pair<std::string, Uint8>>& choices)
{
	UTIL_createModalDialog(windowId, priority, title, message, enterButtonId, escapeButtonId, buttons, choices);
}

void Game::sendEnterGame()
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendEnterGame();
	}
}

void Game::sendLogout()
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendLogout();
	}
}

void Game::sendAutoWalk(const std::vector<Direction>& path)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendAutoWalk(path);
	}
}

void Game::sendWalk(Direction dir)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendWalk(dir);
	}
}

void Game::sendTurn(Direction dir)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
		{
			switch(dir)
			{
				case DIRECTION_NORTH: game->sendTurnNorth(); break;
				case DIRECTION_EAST: game->sendTurnEast(); break;
				case DIRECTION_SOUTH: game->sendTurnSouth(); break;
				case DIRECTION_WEST: game->sendTurnWest(); break;
				default: break;
			}
		}
	}
}

void Game::sendSay(MessageMode mode, Uint16 channelId, const std::string& receiver, const std::string& message)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendSay(mode, channelId, receiver, message);
	}
}

void Game::sendRequestChannels()
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendRequestChannels();
	}
}

void Game::sendCreatePrivateChannel()
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendCreatePrivateChannel();
	}
}

void Game::sendOpenChannel(Uint16 channelId)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendOpenChannel(channelId);
	}
}

void Game::sendCloseChannel(Uint16 channelId)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendCloseChannel(channelId);
	}
}

void Game::sendOpenPrivateChannel(const std::string& receiver)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendOpenPrivateChannel(receiver);
	}
}

void Game::sendCloseNPCChannel()
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendCloseNPCChannel();
	}
}

void Game::sendChannelInvite(const std::string& target, Uint16 channelId)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendChannelInvite(target, channelId);
	}
}

void Game::sendChannelExclude(const std::string& target, Uint16 channelId)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendChannelExclude(target, channelId);
	}
}

void Game::sendOpenQuestLog()
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendShowQuestLog();
	}
}

void Game::sendOpenQuestLine(Uint16 questId)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendShowQuestLine(questId);
	}
}

void Game::sendPing()
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendPing();
	}
}

void Game::sendPingBack()
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendPingBack();
	}
}

void Game::sendAttackModes()
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendAttackModes(g_engine.getAttackMode(), g_engine.getChaseMode(), g_engine.getSecureMode(), g_engine.getPvpMode());
	}
}

void Game::sendAttack(Creature* creature)
{
	Uint32 sequence = m_sequence;
	m_attackId = (creature ? creature->getId() : 0);
	if(g_clientVersion >= 963)
		m_sequence = (m_attackId ? m_attackId : sequence);
	else
		++m_sequence;

	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
		{
			if(m_followId)
				game->sendFollow(0, sequence);

			game->sendAttack(m_attackId, m_sequence);
		}
	}
	m_followId = 0;
}

void Game::sendFollow(Creature* creature)
{
	Uint32 sequence = m_sequence;
	m_followId = (creature ? creature->getId() : 0);
	if(g_clientVersion >= 963)
		m_sequence = (m_followId ? m_followId : sequence);
	else
		++m_sequence;

	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
		{
			if(m_attackId)
				game->sendAttack(0, sequence);

			game->sendFollow(m_followId, m_sequence);
		}
	}
	m_attackId = 0;
}

void Game::sendCancelAttackAndFollow()
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendCancelAttackAndFollow();
	}
}

void Game::sendJoinAggression(Uint32 creatureId)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendJoinAggression(creatureId);
	}
}

void Game::sendInviteToParty(Uint32 creatureId)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendInviteToParty(creatureId);
	}
}

void Game::sendJoinToParty(Uint32 creatureId)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendJoinToParty(creatureId);
	}
}

void Game::sendRevokePartyInvitation(Uint32 creatureId)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendRevokePartyInvitation(creatureId);
	}
}

void Game::sendPassPartyLeadership(Uint32 creatureId)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendPassPartyLeadership(creatureId);
	}
}

void Game::sendLeaveParty()
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendLeaveParty();
	}
}

void Game::sendEnableSharedPartyExperience(bool active)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendEnableSharedPartyExperience(active);
	}
}

void Game::sendRequestOutfit()
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendRequestOutfit();
	}
}

void Game::sendSetOutfit(Uint16 lookType, Uint8 lookHead, Uint8 lookBody, Uint8 lookLegs, Uint8 lookFeet, Uint8 lookAddons, Uint16 lookMount)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendSetOutfit(lookType, lookHead, lookBody, lookLegs, lookFeet, lookAddons, lookMount);
	}
}

void Game::sendMount(bool active)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendMount(active);
	}
}

void Game::sendEquipItem(Uint16 itemid, Uint16 count)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendEquipItem(itemid, count);
	}
}

void Game::sendMove(const Position& fromPos, Uint16 itemid, Uint8 stackpos, const Position& toPos, Uint16 count)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendMove(fromPos, itemid, stackpos, toPos, count);
	}
}

void Game::sendLookAt(const Position& position, Uint16 itemId, Uint8 stackpos)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendLookAt(position, itemId, stackpos);
	}
}

void Game::sendLookInBattle(Uint32 creatureId)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendLookInBattle(creatureId);
	}
}

void Game::sendUseItem(const Position& position, Uint16 itemId, Uint8 stackpos, Uint8 index)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendUseItem(position, itemId, stackpos, index);
	}
}

void Game::sendUseItemEx(const Position& fromPos, Uint16 itemId, Uint8 fromStackPos, const Position& toPos, Uint16 toItemId, Uint8 toStackPos)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendUseItemEx(fromPos, itemId, fromStackPos, toPos, toItemId, toStackPos);
	}
}

void Game::sendUseOnCreature(const Position& position, Uint16 itemId, Uint8 stackpos, Uint32 creatureId)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendUseOnCreature(position, itemId, stackpos, creatureId);
	}
}

void Game::sendRotateItem(const Position& position, Uint16 itemId, Uint8 stackpos)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendRotateItem(position, itemId, stackpos);
	}
}

void Game::sendWrapState(const Position& position, Uint16 itemId, Uint8 stackpos)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendWrapState(position, itemId, stackpos);
	}
}

void Game::sendCloseContainer(Uint8 containerId)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendCloseContainer(containerId);
	}
}

void Game::sendUpContainer(Uint8 containerId)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendUpContainer(containerId);
	}
}

void Game::sendUpdateContainer(Uint8 containerId)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendUpdateContainer(containerId);
	}
}

void Game::sendSeekInContainer(Uint8 containerId, Uint16 index)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendSeekInContainer(containerId, index);
	}
}

void Game::sendBrowseField(const Position& position)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendBrowseField(position);
	}
}

void Game::sendOpenParentContainer(const Position& position)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendOpenParentContainer(position);
	}
}

void Game::sendTextWindow(Uint32 windowId, const std::string& text)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendTextWindow(windowId, text);
	}
}

void Game::sendHouseWindow(Uint32 windowId, Uint8 doorId, const std::string& text)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendHouseWindow(windowId, doorId, text);
	}
}

void Game::sendRequestTrade(const Position& position, Uint16 itemId, Uint8 stackpos, Uint32 creatureId)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendRequestTrade(position, itemId, stackpos, creatureId);
	}
}

void Game::sendLookInTrade(bool counterOffer, Uint8 index)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendLookInTrade(counterOffer, index);
	}
}

void Game::sendAcceptTrade()
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendAcceptTrade();
	}
}

void Game::sendCloseTrade()
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendCloseTrade();
	}
}

void Game::sendLookInShop(Uint16 itemid, Uint16 count)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendLookInShop(itemid, count);
	}
}

void Game::sendNPCPurchase(Uint16 itemid, Uint8 subtype, Uint16 amount)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendNPCPurchase(itemid, subtype, amount, g_engine.getIgnoreCapacity(), g_engine.getBuyWithBackpacks());
	}
}

void Game::sendNPCSell(Uint16 itemid, Uint8 subtype, Uint16 amount)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendNPCSell(itemid, subtype, amount, g_engine.getIgnoreEquiped());
	}
}

void Game::sendNPCClose()
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendNPCClose();
	}
}

void Game::sendAddVip(const std::string& name)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendAddVip(name);
	}
}

void Game::sendRemoveVip(Uint32 playerGUID)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendRemoveVip(playerGUID);
	}
}

void Game::sendEditVip(Uint32 playerGUID, const std::string& description, Uint32 iconId, bool notifyLogin, std::vector<Uint8>& groupIds)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendEditVip(playerGUID, description, iconId, notifyLogin, groupIds);
	}
}

void Game::sendVipGroupAction(VipGroupActions groupAction, Uint8 groupId, const std::string& name)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendVipGroupAction(groupAction, groupId, name);
	}
}

void Game::sendAnswerModalDialog(Uint32 dialogId, Uint8 button, Uint8 choice)
{
	if(g_engine.isIngame())
	{
		ProtocolGame* game = GET_SAFE_PROTOCOLGAME;
		if(game && game->canPerformAction())
			game->sendAnswerModalDialog(dialogId, button, choice);
	}
}

void Game::stopActions()
{
	bool stopAttackAndFollow = (getAttackID() != 0 || getFollowID() != 0);
	stopAutoWalk();
	if(stopAttackAndFollow)
	{
		setAttackID(0);
		setFollowID(0);
		sendCancelAttackAndFollow();
	}
}

void Game::startAutoWalk(const Position& toPosition)
{
	Position& pos = g_map.getCentralPosition();
	m_autoWalkDestination = toPosition;
	m_limitWalkDestination.x = 0xFFFF;

	Creature* player = g_map.getLocalCreature();
	if(!player || player->isPreWalking() || player->isWalking())
		return;

	if(player->getSpeed() == 0)
	{
		g_game.processTextMessage(MessageFailure, "Sorry, not possible.");
		m_autoWalkDestination.x = 0xFFFF;
		return;
	}

	PathFind result = g_map.findPath(m_autoWalkDirections, pos, toPosition);
	if(result != PathFind_ReturnSuccessfull)
	{
		switch(result)
		{
			case PathFind_ReturnImpossible: g_game.processTextMessage(MessageFailure, "Sorry, not possible."); break;
			case PathFind_ReturnTooFar: g_game.processTextMessage(MessageFailure, "Destination is out of range."); break;
			case PathFind_ReturnFirstGoDownStairs: g_game.processTextMessage(MessageFailure, "First go downstairs."); break;
			case PathFind_ReturnFirstGoUpStairs: g_game.processTextMessage(MessageFailure, "First go upstairs."); break;
			case PathFind_ReturnNoWay: g_game.processTextMessage(MessageFailure, "There is no way."); break;
			default: break;
		}
		m_autoWalkDestination.x = 0xFFFF;
		return;
	}

	if(getAttackID() != 0 && g_engine.getChaseMode() == CHASEMODE_FOLLOW && g_engine.hasAutoChaseOff())
	{
		g_engine.setChaseMode(CHASEMODE_STAND);
		sendAttackModes();
	}

	if(m_autoWalkDirections.size() == 1)
	{
		m_autoWalkDestination.x = 0xFFFF;
		m_playerCurrentDir = m_autoWalkDirections.back();
		m_playerMovement = m_playerCurrentDir;
		m_autoWalkDirections.clear();
		checkLocalCreatureMovement();
		return;
	}

	m_limitWalkDestination = pos.translatedToDirections(m_autoWalkDirections);
	sendAutoWalk(m_autoWalkDirections);
	checkLocalCreatureMovement();
}

void Game::stopAutoWalk()
{
	if(m_autoWalkDestination.x == 0xFFFF)
		return;

	Position& pos = g_map.getCentralPosition();
	if(pos != m_autoWalkDestination)
	{
		m_autoWalkDestination.x = 0xFFFF;
		m_limitWalkDestination.x = 0xFFFF;
		m_autoWalkDirections.clear();
		sendWalk(DIRECTION_INVALID);
	}
}

void Game::checkServerMovement(Direction dir)
{
	if(!m_autoWalkDirections.empty())
	{
		Direction walkDir = m_autoWalkDirections.back();
		if(walkDir != dir)
		{
			//Stop autowalk because we got server interrupt our walking
			m_autoWalkDestination.x = 0xFFFF;
			m_limitWalkDestination.x = 0xFFFF;
			m_autoWalkDirections.clear();
		}
		else
			m_autoWalkDirections.pop_back();
	}
}

void Game::checkLocalCreatureMovement()
{
	Creature* player = g_map.getLocalCreature();
	if(!player || player->isPreWalking() || player->isWalking() || player->getSpeed() == 0)
		return;

	Position& pos = player->getCurrentPosition();
	if((m_limitWalkDestination.x == 0xFFFF && m_autoWalkDestination.x != 0xFFFF) || (m_limitWalkDestination.x != 0xFFFF && pos == m_limitWalkDestination))
	{
		if(pos == m_autoWalkDestination)
		{
			m_autoWalkDestination.x = 0xFFFF;
			m_limitWalkDestination.x = 0xFFFF;
			m_autoWalkDirections.clear();
		}
		else
			startAutoWalk(m_autoWalkDestination);
		return;
	}

	Position topos;
	bool prewalk = false;
	bool sendwalk = false;
	Direction walkDir = DIRECTION_INVALID;
	if(!m_autoWalkDirections.empty())
	{
		walkDir = m_autoWalkDirections.back();
		m_autoWalkDirections.pop_back();
		topos = pos.translatedToDirection(walkDir);
		prewalk = true;
	}
	else if(m_playerCurrentDir != DIRECTION_INVALID)
	{
		if(m_playerCurrentDir != m_playerLastDir)
		{
			walkDir = SDL_static_cast(Direction, m_playerCurrentDir);
			m_playerMovement = DIRECTION_INVALID;
		}
		else
		{
			switch(m_playerCurrentDir)
			{
				case DIRECTION_NORTH:
				{
					if(m_playerMovement == DIRECTION_NORTH)
					{
						walkDir = DIRECTION_NORTH;
						m_playerMovement = DIRECTION_INVALID;
					}
				}
				break;
				case DIRECTION_EAST:
				{
					if(m_playerMovement == DIRECTION_EAST)
					{
						walkDir = DIRECTION_EAST;
						m_playerMovement = DIRECTION_INVALID;
					}
				}
				break;
				case DIRECTION_SOUTH:
				{
					if(m_playerMovement == DIRECTION_SOUTH)
					{
						walkDir = DIRECTION_SOUTH;
						m_playerMovement = DIRECTION_INVALID;
					}
				}
				break;
				case DIRECTION_WEST:
				{
					if(m_playerMovement == DIRECTION_WEST)
					{
						walkDir = DIRECTION_WEST;
						m_playerMovement = DIRECTION_INVALID;
					}
				}
				break;
				case DIRECTION_SOUTHWEST:
				{
					if(m_playerMovement == DIRECTION_SOUTHWEST)
					{
						walkDir = DIRECTION_SOUTHWEST;
						m_playerMovement = DIRECTION_INVALID;
					}
				}
				break;
				case DIRECTION_SOUTHEAST:
				{
					if(m_playerMovement == DIRECTION_SOUTHEAST)
					{
						walkDir = DIRECTION_SOUTHEAST;
						m_playerMovement = DIRECTION_INVALID;
					}
				}
				break;
				case DIRECTION_NORTHWEST:
				{
					if(m_playerMovement == DIRECTION_NORTHWEST)
					{
						walkDir = DIRECTION_NORTHWEST;
						m_playerMovement = DIRECTION_INVALID;
					}
				}
				break;
				case DIRECTION_NORTHEAST:
				{
					if(m_playerMovement == DIRECTION_NORTHEAST)
					{
						walkDir = DIRECTION_NORTHEAST;
						m_playerMovement = DIRECTION_INVALID;
					}
				}
				break;
				default: break;
			}
		}

		if(walkDir != DIRECTION_INVALID)
		{
			topos = pos.translatedToDirection(walkDir);
			m_playerCurrentDir = DIRECTION_INVALID;

			Tile* toTile = g_map.getTile(topos);
			if(toTile && toTile->isWalkable())
			{
				prewalk = true;
				sendwalk = true;
			}
			else
			{
				//Check if we can go up
				Tile* tile = g_map.getTile(pos);
				if(tile && tile->getTileElevation() >= ITEM_MAX_ELEVATION)
				{
					Position uppos = topos;
					if(uppos.up())
					{
						toTile = g_map.getTile(uppos);
						if(toTile && toTile->isWalkable())
						{
							//Check what server thinks about our movement :)
							sendwalk = true;
						}
					}
				}

				if(!sendwalk)
				{
					//Check if we can go down
					Position downpos = topos;
					if(downpos.down())
					{
						toTile = g_map.getTile(downpos);
						if(toTile && toTile->isWalkable() && toTile->getTileElevation() >= ITEM_MAX_ELEVATION)
						{
							//Check what server thinks about our movement :)
							sendwalk = true;
						}
					}
				}

				if(!sendwalk)
					g_game.processTextMessage(MessageFailure, "Sorry, not possible.");
			}
		}
	}

	if(walkDir != DIRECTION_INVALID)
	{
		m_playerLastDir = SDL_static_cast(Uint8, walkDir);
		if(prewalk)
			player->preMove(pos, topos);
		if(sendwalk)
			sendWalk(walkDir);
	}
}

void Game::checkMovement(Direction dir)
{
	if(m_autoWalkDestination.x != 0xFFFF)
		stopAutoWalk();

	if(getAttackID() != 0 && g_engine.getChaseMode() == CHASEMODE_FOLLOW && g_engine.hasAutoChaseOff())
	{
		g_engine.setChaseMode(CHASEMODE_STAND);
		sendAttackModes();
	}

	m_playerCurrentDir = dir;
	m_playerMovement = dir;
	checkLocalCreatureMovement();
}

void Game::releaseMovement()
{
	m_playerMovement = DIRECTION_INVALID;
}

void Game::switchToNextChannel()
{
	g_chat.switchToNextChannel();
}

void Game::switchToPreviousChannel()
{
	g_chat.switchToPreviousChannel();
}

void Game::closeCurrentChannel()
{
	Channel* channel = g_chat.getCurrentChannel();
	if(channel && channel->channelClosable)
		g_chat.leaveChannel(channel->channelId);
}

void Game::openHelpChannel()
{
	sendOpenChannel(SDL_static_cast(Uint16, g_chat.getHelpChannelId()));
}

void Game::openNPCChannel()
{
	if(g_game.hasGameFeature(GAME_FEATURE_NPC_INTERFACE))
	{
		g_chat.openChannel(CHANNEL_ID_NPC, CHANNEL_NAME_NPC);
		g_chat.setCurrentChannel(CHANNEL_ID_NPC);
	}
}

void Game::switchToDefault()
{
	g_chat.setCurrentChannel(CHANNEL_ID_DEFAULT);
}

void Game::minimapCenter()
{
	g_automap.setPosition(g_automap.getCentralPosition());
}

void Game::minimapFloorDown()
{
	Position& pos = g_automap.getPosition();
	g_automap.setPosition(Position(pos.x, pos.y, pos.z + 1));
}

void Game::minimapFloorUp()
{
	Position& pos = g_automap.getPosition();
	g_automap.setPosition(Position(pos.x, pos.y, pos.z - 1));
}

void Game::minimapScrollEast()
{
	Position& pos = g_automap.getPosition();
	g_automap.setPosition(Position(pos.x + 5, pos.y, pos.z));
}

void Game::minimapScrollNorth()
{
	Position& pos = g_automap.getPosition();
	g_automap.setPosition(Position(pos.x, pos.y - 5, pos.z));
}

void Game::minimapScrollSouth()
{
	Position& pos = g_automap.getPosition();
	g_automap.setPosition(Position(pos.x, pos.y + 5, pos.z));
}

void Game::minimapScrollWest()
{
	Position& pos = g_automap.getPosition();
	g_automap.setPosition(Position(pos.x - 5, pos.y, pos.z));
}

void Game::minimapScrollNorthWest()
{
	Position& pos = g_automap.getPosition();
	g_automap.setPosition(Position(pos.x - 5, pos.y - 5, pos.z));
}

void Game::minimapScrollNorthEast()
{
	Position& pos = g_automap.getPosition();
	g_automap.setPosition(Position(pos.x + 5, pos.y - 5, pos.z));
}

void Game::minimapScrollSouthWest()
{
	Position& pos = g_automap.getPosition();
	g_automap.setPosition(Position(pos.x - 5, pos.y + 5, pos.z));
}

void Game::minimapScrollSouthEast()
{
	Position& pos = g_automap.getPosition();
	g_automap.setPosition(Position(pos.x + 5, pos.y + 5, pos.z));
}

void Game::minimapZoomIn()
{
	g_automap.zoomIn();
}

void Game::minimapZoomOut()
{
	g_automap.zoomOut();
}

void Game::minimapSetZoom(Sint32 zoom)
{
	g_automap.setZoom(zoom);
}

Sint32 Game::minimapGetZoom()
{
	return g_automap.getZoom();
}

Uint8 Game::findEmptyContainerId()
{
	for(Uint8 i = 0; i < GAME_MAX_CONTAINERS; ++i)
	{
		if(!m_containers[i])
			return i;
	}
	return 0x0F;
}

bool Game::containerHasParent(Uint8 cid)
{
	if(m_containers[cid] && m_containers[cid]->hasParent())
		return true;

	return false;
}

void Game::resetPlayerExperienceTable()
{
	m_expTable.clear();
	m_playerExpSpeed = 0.0;
}

void Game::updatePlayerExperienceTable()
{
	Uint32 timeStamp = g_frameTime / 1000;
	if(!m_expTable.empty())
	{
		std::pair<Uint32, Uint64>& pair = m_expTable.front();
		double expDiff = SDL_static_cast(double, (m_playerExperience - pair.second));
		double timeDiff = SDL_static_cast(double, (timeStamp - pair.first));
		m_playerExpSpeed = expDiff / UTIL_max<double>(timeDiff, 1.0);
	}

	m_expTable.push_back(std::make_pair(timeStamp, m_playerExperience));
	if(m_expTable.size() > 60)
		m_expTable.erase(m_expTable.begin());
}

std::string Game::getPlayerName()
{
	Creature* player = g_map.getLocalCreature();
	if(player)
		return player->getName();

	return std::string();
}

void Game::setPlayerExperience(Uint64 experience)
{
	if(m_playerExperience != experience)
	{
		m_playerExperience = experience;
		m_cached_stats |= CACHED_STAT_EXPERIENCE;
	}
}

void Game::setPlayerCapacity(double capacity, double totalCapacity)
{
	if(m_playerCapacity != capacity || m_playerTotalCapacity != totalCapacity)
	{
		m_playerCapacity = capacity;
		m_playerTotalCapacity = totalCapacity;
		m_cached_stats |= CACHED_STAT_CAPACITY;
	}
}

void Game::setPlayerHealth(Uint32 health, Uint32 maxHealth)
{
	if(m_playerHealth != health || m_playerMaxHealth != maxHealth)
	{
		m_playerHealth = health;
		m_playerMaxHealth = maxHealth;
		if(maxHealth == 0)
			m_playerHealthPercent = 100;
		else
			m_playerHealthPercent = SDL_static_cast(Uint8, SDL_static_cast(Uint64, health) * SDL_static_cast(Uint64, 100) / SDL_static_cast(Uint64, maxHealth));

		m_cached_stats |= CACHED_STAT_HEALTH;
	}
}

void Game::setPlayerMana(Uint32 mana, Uint32 maxMana)
{
	if(m_playerMana != mana || m_playerMaxMana != maxMana)
	{
		m_playerMana = mana;
		m_playerMaxMana = maxMana;
		if(maxMana == 0)
			m_playerManaPercent = 100;
		else
			m_playerManaPercent = SDL_static_cast(Uint8, SDL_static_cast(Uint64, mana) * SDL_static_cast(Uint64, 100) / SDL_static_cast(Uint64, maxMana));

		m_cached_stats |= CACHED_STAT_MANA;
	}
}

void Game::setPlayerLevel(Uint16 level, Uint8 levelPercent)
{
	if(m_playerLevel != level || m_playerLevelPercent != levelPercent)
	{
		m_playerLevel = level;
		m_playerLevelPercent = levelPercent;
		m_cached_stats |= CACHED_STAT_LEVEL;
	}
}

void Game::setPlayerMagicLevel(Uint16 level, Uint16 baseLevel, Uint8 levelPercent)
{
	if(m_playerMagicLevel != level || m_playerBaseMagicLevel != baseLevel || m_playerMagicLevelPercent != levelPercent)
	{
		m_playerMagicLevel = level;
		m_playerBaseMagicLevel = baseLevel;
		m_playerMagicLevelPercent = levelPercent;
		m_cached_stats |= CACHED_STAT_MAGICLEVEL;
	}
}

void Game::setPlayerStamina(Uint16 stamina)
{
	if(m_playerStamina != stamina)
	{
		m_playerStamina = stamina;
		m_cached_stats |= CACHED_STAT_STAMINA;
	}
}

void Game::setPlayerSoul(Uint8 soul)
{
	if(m_playerSoul != soul)
	{
		m_playerSoul = soul;
		m_cached_stats |= CACHED_STAT_SOUL;
	}
}

void Game::setPlayerBaseSpeed(Uint16 baseSpeed)
{
	if(m_playerBaseSpeed != baseSpeed)
	{
		m_playerBaseSpeed = baseSpeed;
		m_cached_stats |= CACHED_STAT_SPEED;
	}
}

void Game::setPlayerRegeneration(Uint16 regeneration)
{
	if(m_playerRegeneration != regeneration)
	{
		m_playerRegeneration = regeneration;
		m_cached_stats |= CACHED_STAT_REGENERATION;
	}
}

void Game::setPlayerOfflineTraining(Uint16 offlineTime)
{
	if(m_playerOfflineTraining != offlineTime)
	{
		m_playerOfflineTraining = offlineTime;
		m_cached_stats |= CACHED_STAT_TRAINING;
	}
}

void Game::setPlayerExpBonus(double expBonus)
{
	if(m_playerExpBonus != expBonus)
	{
		m_playerExpBonus = expBonus;
		m_cached_stats |= CACHED_STAT_XPGAIN;
	}
}

void Game::setPlayerExpBonus(Uint16 baseXp, Uint16 voucherXp, Uint16 grindingXp, Uint16 storeXp, Uint16 huntingXp)
{
	if(m_playerBaseXpGain != baseXp || m_playerVoucherXpGain != voucherXp || m_playerGrindingXpGain != grindingXp || m_playerStoreXpGain != storeXp || m_playerHuntingXpGain != huntingXp)
	{
		m_playerBaseXpGain = baseXp;
		m_playerVoucherXpGain = voucherXp;
		m_playerGrindingXpGain = grindingXp;
		m_playerStoreXpGain = storeXp;
		m_playerHuntingXpGain = huntingXp;
		m_cached_stats |= CACHED_STAT_XPGAIN;
	}
}

void Game::setPlayerSkill(Skills skill, Uint16 level, Uint16 baseLevel, Uint8 levelPercent)
{
	if(m_playerSkillsLevel[skill] != level || m_playerSkillsBaseLevel[skill] != baseLevel || m_playerSkillsLevelPercent[skill] != levelPercent)
	{
		m_playerSkillsLevel[skill] = level;
		m_playerSkillsBaseLevel[skill] = baseLevel;
		m_playerSkillsLevelPercent[skill] = levelPercent;
		switch(skill)
		{
			case Skills_Fist: m_cached_skills |= CACHED_SKILL_FIST;
			case Skills_Club: m_cached_skills |= CACHED_SKILL_CLUB;
			case Skills_Sword: m_cached_skills |= CACHED_SKILL_SWORD;
			case Skills_Axe: m_cached_skills |= CACHED_SKILL_AXE;
			case Skills_Distance: m_cached_skills |= CACHED_SKILL_DISTANCE;
			case Skills_Shielding: m_cached_skills |= CACHED_SKILL_SHIELDING;
			case Skills_Fishing: m_cached_skills |= CACHED_SKILL_FISHING;
			case Skills_CriticalChance: m_cached_skills |= CACHED_SKILL_CRITICAL;
			case Skills_CriticalDamage: m_cached_skills |= CACHED_SKILL_CRITICAL;
			case Skills_LifeLeechChance: m_cached_skills |= CACHED_SKILL_LIFELEECH;
			case Skills_LifeLeechAmount: m_cached_skills |= CACHED_SKILL_LIFELEECH;
			case Skills_ManaLeechChance: m_cached_skills |= CACHED_SKILL_MANALEECH;
			case Skills_ManaLeechAmount: m_cached_skills |= CACHED_SKILL_MANALEECH;
			default: break;
		}
	}
}
