/*
  The Forgotten Client
  Native bot coordinator.
*/

#include "bot.h"
#include "container.h"
#include "creature.h"
#include "engine.h"
#include "game.h"
#include "item.h"
#include "map.h"
#include "thingManager.h"
#include "tile.h"
#include "util.h"
#include "GUI/itemUI.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <map>
#include <sstream>

extern Engine g_engine;
extern Game g_game;
extern Map g_map;
extern ThingManager g_thingManager;
extern std::string g_prefPath;

Bot g_bot;

static const Uint32 BOT_HEAL_INTERVAL = 650;
static const size_t BOT_HOTKEY_COUNT = 5;
static const size_t BOT_ICON_COUNT = 12;
static const Uint32 BOT_LOOT_PENDING_TTL = 5000;
static const Uint32 BOT_LOOT_LOOTED_TTL = 30000;

static std::string botTrim(const std::string& text);

static std::string botTrimLeft(const std::string& text)
{
	size_t pos = text.find_first_not_of(" \t");
	if(pos == std::string::npos)
		return std::string();
	return text.substr(pos);
}

static std::string botLowerText(const std::string& text)
{
	std::string lower = text;
	for(size_t i = 0, end = lower.length(); i < end; ++i)
		lower[i] = SDL_static_cast(char, std::tolower(SDL_static_cast(unsigned char, lower[i])));
	return lower;
}

static std::string botTitleCase(const std::string& text)
{
	std::string result = botLowerText(botTrim(text));
	bool upperNext = true;
	for(size_t i = 0, end = result.length(); i < end; ++i)
	{
		unsigned char ch = SDL_static_cast(unsigned char, result[i]);
		if(std::isspace(ch) || result[i] == '-' || result[i] == '_')
		{
			upperNext = true;
			if(result[i] == '_')
				result[i] = ' ';
			continue;
		}

		if(upperNext)
		{
			result[i] = SDL_static_cast(char, std::toupper(ch));
			upperNext = false;
		}
	}
	return result;
}

static bool botKnownLootItemById(Uint16 itemId, std::string& itemName)
{
	switch(itemId)
	{
		case 3031: itemName = "Gold Coin"; return true;
		case 3035: itemName = "Platinum Coin"; return true;
		case 3043: itemName = "Crystal Coin"; return true;
		case 3387: itemName = "Demon Helmet"; return true;
		default: break;
	}
	return false;
}

static bool botReadXmlAttribute(const std::string& line, const std::string& attribute, std::string& value)
{
	std::string needle = attribute + "=\"";
	size_t begin = line.find(needle);
	if(begin == std::string::npos)
		return false;

	begin += needle.length();
	size_t end = line.find('"', begin);
	if(end == std::string::npos)
		return false;

	value = line.substr(begin, end - begin);
	return true;
}

static void botLoadItemNameDatabase(std::map<Uint16, std::string>& namesById, std::map<std::string, Uint16>& idsByName)
{
	static const char* candidates[] =
	{
		"data/items/items.xml",
		"../data/items/items.xml",
		"forgottenserver-downgrade-1.8-8.60/data/items/items.xml",
		"../forgottenserver-downgrade-1.8-8.60/data/items/items.xml",
		"../../forgottenserver-downgrade-1.8-8.60/data/items/items.xml",
		"../../../forgottenserver-downgrade-1.8-8.60/data/items/items.xml"
	};

	for(size_t pathIndex = 0; pathIndex < sizeof(candidates) / sizeof(candidates[0]); ++pathIndex)
	{
		std::ifstream file(candidates[pathIndex]);
		if(!file.is_open())
			continue;

		std::string line;
		while(std::getline(file, line))
		{
			if(line.find("<item ") == std::string::npos)
				continue;

			std::string idText;
			std::string nameText;
			if(!botReadXmlAttribute(line, "id", idText) || !botReadXmlAttribute(line, "name", nameText))
				continue;

			Uint32 id = SDL_static_cast(Uint32, std::strtoul(idText.c_str(), NULL, 10));
			if(id == 0 || id > 0xFFFF)
				continue;

			std::string niceName = botTitleCase(nameText);
			namesById[SDL_static_cast(Uint16, id)] = niceName;
			idsByName[botLowerText(nameText)] = SDL_static_cast(Uint16, id);
		}

		return;
	}
}

static bool botItemDatabaseById(Uint16 itemId, std::string& itemName)
{
	static bool loaded = false;
	static std::map<Uint16, std::string> namesById;
	static std::map<std::string, Uint16> idsByName;
	if(!loaded)
	{
		botLoadItemNameDatabase(namesById, idsByName);
		loaded = true;
	}

	std::map<Uint16, std::string>::iterator it = namesById.find(itemId);
	if(it == namesById.end())
		return false;

	itemName = it->second;
	return true;
}

static bool botItemDatabaseByName(const std::string& name, Uint16& itemId, std::string& itemName)
{
	static bool loaded = false;
	static std::map<Uint16, std::string> namesById;
	static std::map<std::string, Uint16> idsByName;
	if(!loaded)
	{
		botLoadItemNameDatabase(namesById, idsByName);
		loaded = true;
	}

	std::string lower = botLowerText(botTrim(name));
	std::map<std::string, Uint16>::iterator it = idsByName.find(lower);
	if(it == idsByName.end())
		return false;

	itemId = it->second;
	itemName = namesById[itemId];
	return true;
}

static bool botKnownLootItemByName(const std::string& name, Uint16& itemId, std::string& itemName)
{
	std::string lower = botLowerText(botTrim(name));
	if(lower == "gold coin" || lower == "gold")
	{
		itemId = 3031;
		itemName = "Gold Coin";
		return true;
	}
	if(lower == "platinum coin" || lower == "platinum")
	{
		itemId = 3035;
		itemName = "Platinum Coin";
		return true;
	}
	if(lower == "crystal coin" || lower == "crystal")
	{
		itemId = 3043;
		itemName = "Crystal Coin";
		return true;
	}
	if(lower == "demon helmet")
	{
		itemId = 3387;
		itemName = "Demon Helmet";
		return true;
	}
	return false;
}

static bool botResolveLootItem(const std::string& input, Uint16& itemId, std::string& itemName)
{
	std::string text = botTrim(input);
	if(text.empty())
		return false;

	char* endPtr = NULL;
	unsigned long value = std::strtoul(text.c_str(), &endPtr, 10);
	if(endPtr && *endPtr == 0)
	{
		if(value == 0 || value > 0xFFFF)
			return false;

		itemId = SDL_static_cast(Uint16, value);
		ThingType* type = g_thingManager.getThingType(ThingCategory_Item, itemId);
		if(!type)
			return false;

		if(!type->m_marketData.m_name.empty())
			itemName = botTitleCase(type->m_marketData.m_name);
		else if(botItemDatabaseById(itemId, itemName))
			return true;
		else if(!botKnownLootItemById(itemId, itemName))
			itemName = "Unknown Item";
		return true;
	}

	if(botItemDatabaseByName(text, itemId, itemName))
		return g_thingManager.getThingType(ThingCategory_Item, itemId) != NULL;

	if(botKnownLootItemByName(text, itemId, itemName))
		return g_thingManager.getThingType(ThingCategory_Item, itemId) != NULL;

	return false;
}

static std::string botTrim(const std::string& text)
{
	size_t first = text.find_first_not_of(" \t\r\n");
	if(first == std::string::npos)
		return std::string();

	size_t last = text.find_last_not_of(" \t\r\n");
	return text.substr(first, last - first + 1);
}

static std::string botJsonEscape(const std::string& text)
{
	std::string escaped;
	for(size_t i = 0, end = text.length(); i < end; ++i)
	{
		char ch = text[i];
		if(ch == '\\' || ch == '"')
			escaped += '\\';
		escaped += ch;
	}
	return escaped;
}

static std::string botProfilePath(const std::string& profileName, const char* extension)
{
	std::string name = botTrim(profileName);
	if(name.empty())
		name = "native_bot";

	std::string clean;
	for(size_t i = 0, end = name.length(); i < end; ++i)
	{
		char ch = name[i];
		if((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0' && ch <= '9') ||
			ch == '_' || ch == '-' || ch == '.')
			clean += ch;
		else if(ch == ' ')
			clean += '_';
	}
	if(clean.empty())
		clean = "native_bot";

	if(clean.find('.') == std::string::npos)
		clean += extension;

	return g_prefPath + clean;
}

static bool botParseKeyName(const std::string& keyName, SDL_Keycode& keycode)
{
	std::string key = botLowerText(botTrim(keyName));
	if(key.empty())
		return false;

	if(key == "num1" || key == "kp1") {keycode = SDLK_KP_1; return true;}
	if(key == "num2" || key == "kp2") {keycode = SDLK_KP_2; return true;}
	if(key == "num3" || key == "kp3") {keycode = SDLK_KP_3; return true;}
	if(key == "num4" || key == "kp4") {keycode = SDLK_KP_4; return true;}
	if(key == "num5" || key == "kp5") {keycode = SDLK_KP_5; return true;}
	if(key == "num6" || key == "kp6") {keycode = SDLK_KP_6; return true;}
	if(key == "num7" || key == "kp7") {keycode = SDLK_KP_7; return true;}
	if(key == "num8" || key == "kp8") {keycode = SDLK_KP_8; return true;}
	if(key == "num9" || key == "kp9") {keycode = SDLK_KP_9; return true;}
	if(key == "num0" || key == "kp0") {keycode = SDLK_KP_0; return true;}
	if(key == "f1") {keycode = SDLK_F1; return true;}
	if(key == "f2") {keycode = SDLK_F2; return true;}
	if(key == "f3") {keycode = SDLK_F3; return true;}
	if(key == "f4") {keycode = SDLK_F4; return true;}
	if(key == "f5") {keycode = SDLK_F5; return true;}
	if(key == "f6") {keycode = SDLK_F6; return true;}
	if(key == "f7") {keycode = SDLK_F7; return true;}
	if(key == "f8") {keycode = SDLK_F8; return true;}
	if(key == "f9") {keycode = SDLK_F9; return true;}
	if(key == "f10") {keycode = SDLK_F10; return true;}
	if(key == "f11") {keycode = SDLK_F11; return true;}
	if(key == "f12") {keycode = SDLK_F12; return true;}
	if(key == "home") {keycode = SDLK_HOME; return true;}
	if(key == "end") {keycode = SDLK_END; return true;}
	if(key == "insert" || key == "ins") {keycode = SDLK_INSERT; return true;}
	if(key == "delete" || key == "del") {keycode = SDLK_DELETE; return true;}
	if(key == "pageup" || key == "pgup") {keycode = SDLK_PAGEUP; return true;}
	if(key == "pagedown" || key == "pgdn") {keycode = SDLK_PAGEDOWN; return true;}
	if(key.length() == 1)
	{
		keycode = SDL_static_cast(SDL_Keycode, key[0]);
		return true;
	}
	return false;
}

Bot::Bot()
{
	m_hotkeys.resize(BOT_HOTKEY_COUNT);
	m_icons.resize(BOT_ICON_COUNT);
	reset();
}

void Bot::reset()
{
	m_enabled = false;
	m_pauseCaveInFight = true;
	m_smartWalk = true;
	m_avoidPlayers = false;
	m_healingConfig = BotHealingConfig();
	m_lootItems.clear();
	m_lootCorpses.clear();
	m_hotkeys.assign(BOT_HOTKEY_COUNT, BotHotkeyConfig());
	m_icons.assign(BOT_ICON_COUNT, BotIconConfig());
	m_nextHealTick = 0;
	m_nextLootTick = 0;
	m_nextCorpseOpenTick = 0;
	m_lastCorpseOpenTick = 0;
	m_lastCorpseOpenPosition = Position();
	m_lastTrackedTargetId = 0;
	m_lastTrackedTargetName.clear();
	m_lastTrackedTargetPosition = Position();
	m_lastTrackedTargetTick = 0;
	m_lootInProgress = false;
	m_hotkeysEnabled = false;
	m_cavebot.reset();
	m_targetbot.reset();
}

void Bot::setEnabled(bool enabled)
{
	if(m_enabled == enabled)
		return;

	m_enabled = enabled;
	if(!m_enabled)
	{
		g_game.stopAutoWalk();
		if(g_game.getAttackID() != 0)
			g_game.sendAttack(NULL);
	}
}

void Bot::setSmartWalk(bool enabled)
{
	m_smartWalk = enabled;
	g_engine.setSmartWalking(enabled);
}

void Bot::tick()
{
	if(!g_engine.isIngame())
		return;

	tickPersistentHotkeys();

	if(!m_enabled)
		return;

	tickLootCorpseTracking();
	m_targetbot.tick(m_enabled);
	tickHealing();
	tickAutoLootOpenCorpse();
	m_cavebot.tick(m_enabled, m_lootInProgress || (m_pauseCaveInFight && m_targetbot.hasActiveTarget()));
}

void Bot::setHotkey(size_t index, const BotHotkeyConfig& hotkey)
{
	if(index >= m_hotkeys.size())
		return;

	m_hotkeys[index] = hotkey;
	m_hotkeys[index].spellWords = botTrim(m_hotkeys[index].spellWords);
	m_hotkeys[index].keyName = botTrim(m_hotkeys[index].keyName);
	if(m_hotkeys[index].cooldownMs < 250)
		m_hotkeys[index].cooldownMs = 250;
}

void Bot::setIcon(size_t index, const BotIconConfig& icon)
{
	if(index >= m_icons.size())
		return;

	m_icons[index] = icon;
	m_icons[index].label = botTrim(m_icons[index].label);
	m_icons[index].leftAction = botTrim(m_icons[index].leftAction);
	m_icons[index].rightAction = botTrim(m_icons[index].rightAction);
	m_icons[index].ctrlLeftAction = botTrim(m_icons[index].ctrlLeftAction);
	m_icons[index].ctrlRightAction = botTrim(m_icons[index].ctrlRightAction);
	if(m_icons[index].itemId == 0)
		m_icons[index].enabled = false;
}

bool Bot::castHotkey(BotHotkeyConfig& hotkey, Uint32 now)
{
	if(!m_hotkeysEnabled || !hotkey.enabled || hotkey.spellWords.empty())
		return false;

	if(now < hotkey.nextCastTick)
		return true;

	g_game.sendSay(MessageSpell, 0, std::string(), hotkey.spellWords);
	hotkey.nextCastTick = now + hotkey.cooldownMs;
	return true;
}

bool Bot::handleKeyDown(SDL_Event& event)
{
	if(event.key.repeat != 0)
		return false;

	SDL_Keycode keycode = SDLK_UNKNOWN;
	for(std::vector<BotHotkeyConfig>::iterator it = m_hotkeys.begin(), end = m_hotkeys.end(); it != end; ++it)
	{
		if(!it->enabled || !botParseKeyName(it->keyName, keycode))
			continue;

		if(event.key.keysym.sym == keycode)
			return castHotkey(*it, SDL_GetTicks());
	}
	return false;
}

void Bot::tickPersistentHotkeys()
{
	if(!m_hotkeysEnabled)
		return;

	Uint32 now = SDL_GetTicks();
	for(std::vector<BotHotkeyConfig>::iterator it = m_hotkeys.begin(), end = m_hotkeys.end(); it != end; ++it)
	{
		if(it->persistent)
			castHotkey(*it, now);
	}
}

bool Bot::addLootItem(Uint16 itemId)
{
	Uint16 resolvedId = 0;
	std::string resolvedName;
	SDL_snprintf(g_buffer, sizeof(g_buffer), "%u", SDL_static_cast(Uint32, itemId));
	if(!botResolveLootItem(g_buffer, resolvedId, resolvedName))
	{
		g_game.processTextMessage(MessageFailure, "Item not found");
		return false;
	}

	for(std::vector<BotLootItem>::iterator it = m_lootItems.begin(), end = m_lootItems.end(); it != end; ++it)
	{
		if(it->itemId == resolvedId)
		{
			g_game.processTextMessage(MessageStatus, "Item already added");
			return false;
		}
	}

	BotLootItem item;
	item.itemId = resolvedId;
	item.name = resolvedName;
	m_lootItems.push_back(item);
	return true;
}

bool Bot::addLootItem(const std::string& input)
{
	Uint16 itemId = 0;
	std::string itemName;
	if(!botResolveLootItem(input, itemId, itemName))
	{
		g_game.processTextMessage(MessageFailure, "Item not found");
		return false;
	}

	for(std::vector<BotLootItem>::iterator it = m_lootItems.begin(), end = m_lootItems.end(); it != end; ++it)
	{
		if(it->itemId == itemId)
		{
			g_game.processTextMessage(MessageStatus, "Item already added");
			return false;
		}
	}

	BotLootItem item;
	item.itemId = itemId;
	item.name = itemName;
	m_lootItems.push_back(item);
	return true;
}

bool Bot::eraseLootItem(size_t index)
{
	if(index >= m_lootItems.size())
		return false;

	m_lootItems.erase(m_lootItems.begin() + index);
	return true;
}

bool Bot::isLootItem(Uint16 itemId) const
{
	for(std::vector<BotLootItem>::const_iterator it = m_lootItems.begin(), end = m_lootItems.end(); it != end; ++it)
	{
		if(it->itemId == itemId)
			return true;
	}
	return false;
}

void Bot::onContainerOpen(Container* container)
{
	if(!m_enabled || !container || m_lootItems.empty())
		return;

	Uint32 now = SDL_GetTicks();
	cleanupLootCorpses(now);
	if(now < m_nextLootTick)
		return;

	ItemUI* containerItem = container->getItem();
	if(!containerItem)
		return;

	Position corpsePos = containerItem->getCurrentPosition();
	BotLootCorpse* corpse = findPendingLootCorpse(corpsePos, now);
	if(!corpse && now < m_lastCorpseOpenTick + 2500)
		corpse = findPendingLootCorpse(m_lastCorpseOpenPosition, now);
	if(!corpse)
	{
		for(std::vector<BotLootCorpse>::iterator it = m_lootCorpses.begin(), end = m_lootCorpses.end(); it != end; ++it)
		{
			if(it->status == BOT_LOOT_CORPSE_PENDING && it->corpseId == containerItem->getID() && now < it->createdAt + BOT_LOOT_PENDING_TTL)
			{
				corpse = &(*it);
				break;
			}
		}
	}
	if(!corpse || corpse->status == BOT_LOOT_CORPSE_LOOTED || corpse->status == BOT_LOOT_CORPSE_IGNORED)
		return;

	if(m_lootInProgress)
		return;

	m_lootInProgress = true;
	corpse->status = BOT_LOOT_CORPSE_OPENED;
	corpse->containerId = container->getCid();
	corpse->corpseId = containerItem->getID();
	UTIL_protocolDebugLog("bot", "[AutoLootDebug] corpse opened containerId=%u itemId=%u status=opened",
		SDL_static_cast(Uint32, container->getCid()), SDL_static_cast(Uint32, containerItem->getID()));

	Container* targetContainer = NULL;
	for(Uint8 cid = 0; cid < GAME_MAX_CONTAINERS; ++cid)
	{
		Container* candidate = g_game.findContainer(cid);
		if(!candidate || candidate == container)
			continue;

		if(candidate->getItems().size() < SDL_static_cast(size_t, candidate->getCapacity()))
		{
			targetContainer = candidate;
			break;
		}
	}

	if(!targetContainer)
	{
		g_game.processTextMessage(MessageFailure, "Auto Loot: open a backpack first.");
		m_nextLootTick = now + 1000;
		m_lootInProgress = false;
		return;
	}

	bool moved = false;
	Uint8 targetSlot = SDL_static_cast(Uint8, targetContainer->getItems().size());
	std::vector<ItemUI*>& items = container->getItems();
	for(size_t slot = 0, end = items.size(); slot < end; ++slot)
	{
		if(targetSlot >= targetContainer->getCapacity())
			break;

		ItemUI* item = items[slot];
		if(!item || !isLootItem(item->getID()))
			continue;

		Position fromPos = item->getCurrentPosition();
		if(fromPos.x != 0xFFFF || (fromPos.y & 0x40) == 0 || (fromPos.y & 0x0F) != container->getCid())
		{
			UTIL_protocolDebugLog("bot", "[AutoLootDebug] BLOCKED invalid move sourceContainer=%u expectedCorpseContainer=%u itemId=%u",
				SDL_static_cast(Uint32, fromPos.y & 0x0F), SDL_static_cast(Uint32, container->getCid()),
				SDL_static_cast(Uint32, item->getID()));
			g_game.processTextMessage(MessageFailure, "Auto Loot: blocked invalid source container.");
			continue;
		}

		Position toPos(0xFFFF, SDL_static_cast(Uint16, targetContainer->getCid()) | 0x40,
			targetSlot++);
		Uint16 count = item->getItemCount() == 0 ? 1 : item->getItemCount();
		UTIL_protocolDebugLog("bot", "[AutoLootDebug] move item clientId=%u from corpseContainer=%u to backpackContainer=%u count=%u",
			SDL_static_cast(Uint32, item->getID()), SDL_static_cast(Uint32, container->getCid()),
			SDL_static_cast(Uint32, targetContainer->getCid()), SDL_static_cast(Uint32, count));
		g_game.sendMove(fromPos, item->getID(), fromPos.z, toPos, count);
		moved = true;
	}

	if(moved)
		m_nextLootTick = now + 300;
	corpse->status = BOT_LOOT_CORPSE_LOOTED;
	corpse->lootedAt = now;
	UTIL_protocolDebugLog("bot", "[AutoLootDebug] corpse marked looted itemId=%u pos=%u,%u,%u",
		SDL_static_cast(Uint32, corpse->corpseId), SDL_static_cast(Uint32, corpse->position.x),
		SDL_static_cast(Uint32, corpse->position.y), SDL_static_cast(Uint32, corpse->position.z));
	m_lootInProgress = false;
}

void Bot::tickAutoLootOpenCorpse()
{
	if(m_lootItems.empty() || m_lootInProgress)
		return;

	Uint32 now = SDL_GetTicks();
	cleanupLootCorpses(now);
	if(now < m_nextCorpseOpenTick)
		return;

	Creature* player = g_map.getLocalCreature();
	if(!player)
		return;

	Position playerPos = player->getCurrentPosition();
	for(Sint32 dy = -1; dy <= 1; ++dy)
	{
		for(Sint32 dx = -1; dx <= 1; ++dx)
		{
			Sint32 corpseX = SDL_static_cast(Sint32, playerPos.x) + dx;
			Sint32 corpseY = SDL_static_cast(Sint32, playerPos.y) + dy;
			if(corpseX <= 0 || corpseY <= 0 || corpseX > 0xFFFF || corpseY > 0xFFFF)
				continue;

			Position corpsePos(SDL_static_cast(Uint16, corpseX), SDL_static_cast(Uint16, corpseY), playerPos.z);
			BotLootCorpse* corpse = findPendingLootCorpse(corpsePos, now);
			if(!corpse || corpse->status != BOT_LOOT_CORPSE_PENDING)
				continue;

			if(corpsePos == m_lastCorpseOpenPosition && now < m_lastCorpseOpenTick + 3000)
				continue;

			Tile* tile = g_map.getTile(corpsePos);
			if(!tile || !tile->isCreatureLying())
				continue;

			Thing* useThing = tile->getTopUseThing();
			if(!useThing || !useThing->isItem())
				continue;

			Item* item = useThing->getItem();
			if(!item || !item->getThingType() || !item->getThingType()->hasFlag(ThingAttribute_LyingCorpse))
				continue;

			g_game.sendUseItem(corpsePos, item->getID(), SDL_static_cast(Uint8, tile->getUseStackPos(useThing)), g_game.findEmptyContainerId());
			UTIL_protocolDebugLog("bot", "[AutoLootDebug] open corpse itemId=%u pos=%u,%u,%u",
				SDL_static_cast(Uint32, item->getID()), SDL_static_cast(Uint32, corpsePos.x),
				SDL_static_cast(Uint32, corpsePos.y), SDL_static_cast(Uint32, corpsePos.z));
			corpse->corpseId = item->getID();
			m_lastCorpseOpenPosition = corpsePos;
			m_lastCorpseOpenTick = now;
			m_nextCorpseOpenTick = now + 1200;
			return;
		}
	}

	m_nextCorpseOpenTick = now + 250;
}

void Bot::tickLootCorpseTracking()
{
	Uint32 now = SDL_GetTicks();
	cleanupLootCorpses(now);

	Uint32 attackId = g_game.getAttackID();
	Creature* target = attackId != 0 ? g_map.getCreatureById(attackId) : NULL;
	if(target && target->isMonster() && target->isVisible())
	{
		m_lastTrackedTargetId = target->getId();
		m_lastTrackedTargetName = target->getName();
		m_lastTrackedTargetPosition = target->getCurrentPosition();
		m_lastTrackedTargetTick = now;
		if(target->getHealth() == 0)
		{
			registerLootCorpse(target->getId(), target->getName(), target->getCurrentPosition());
			g_game.sendAttack(NULL);
		}
		return;
	}

	if(m_lastTrackedTargetId != 0 && now < m_lastTrackedTargetTick + 1500)
	{
		registerLootCorpse(m_lastTrackedTargetId, m_lastTrackedTargetName, m_lastTrackedTargetPosition);
		m_lastTrackedTargetId = 0;
	}
}

void Bot::registerLootCorpse(Uint32 creatureId, const std::string& creatureName, const Position& position)
{
	if(m_lootItems.empty())
		return;

	Uint32 now = SDL_GetTicks();
	cleanupLootCorpses(now);
	for(std::vector<BotLootCorpse>::iterator it = m_lootCorpses.begin(), end = m_lootCorpses.end(); it != end; ++it)
	{
		if(it->creatureId == creatureId || (it->position == position && it->status != BOT_LOOT_CORPSE_LOOTED))
		{
			if(it->status == BOT_LOOT_CORPSE_LOOTED)
				return;

			it->createdAt = now;
			it->status = BOT_LOOT_CORPSE_PENDING;
			return;
		}
	}

	BotLootCorpse corpse;
	corpse.creatureId = creatureId;
	corpse.creatureName = creatureName;
	corpse.position = position;
	corpse.createdAt = now;
	corpse.status = BOT_LOOT_CORPSE_PENDING;
	m_lootCorpses.push_back(corpse);
	UTIL_protocolDebugLog("bot", "[AutoLootDebug] monster corpse pending name=%s id=%u pos=%u,%u,%u",
		creatureName.c_str(), creatureId, SDL_static_cast(Uint32, position.x),
		SDL_static_cast(Uint32, position.y), SDL_static_cast(Uint32, position.z));
}

void Bot::registerLootCorpseItem(const Position& position, Uint16 corpseId)
{
	if(m_lootItems.empty() || corpseId == 0)
		return;

	Uint32 now = SDL_GetTicks();
	cleanupLootCorpses(now);
	for(std::vector<BotLootCorpse>::iterator it = m_lootCorpses.begin(), end = m_lootCorpses.end(); it != end; ++it)
	{
		if(it->position == position && it->status != BOT_LOOT_CORPSE_LOOTED)
		{
			it->corpseId = corpseId;
			it->createdAt = now;
			it->status = BOT_LOOT_CORPSE_PENDING;
			return;
		}
	}

	BotLootCorpse corpse;
	corpse.creatureName = "corpse";
	corpse.position = position;
	corpse.corpseId = corpseId;
	corpse.createdAt = now;
	corpse.status = BOT_LOOT_CORPSE_PENDING;
	m_lootCorpses.push_back(corpse);
	UTIL_protocolDebugLog("bot", "[AutoLootDebug] corpse registered pos=%u,%u,%u itemId=%u",
		SDL_static_cast(Uint32, position.x), SDL_static_cast(Uint32, position.y),
		SDL_static_cast(Uint32, position.z), SDL_static_cast(Uint32, corpseId));
}

void Bot::cleanupLootCorpses(Uint32 now)
{
	for(std::vector<BotLootCorpse>::iterator it = m_lootCorpses.begin(); it != m_lootCorpses.end();)
	{
		Uint32 ttl = (it->status == BOT_LOOT_CORPSE_LOOTED || it->status == BOT_LOOT_CORPSE_IGNORED) ? BOT_LOOT_LOOTED_TTL : BOT_LOOT_PENDING_TTL;
		Uint32 baseTick = it->lootedAt != 0 ? it->lootedAt : it->createdAt;
		if(now > baseTick + ttl)
			it = m_lootCorpses.erase(it);
		else
			++it;
	}
}

BotLootCorpse* Bot::findPendingLootCorpse(const Position& position, Uint32 now)
{
	for(std::vector<BotLootCorpse>::iterator it = m_lootCorpses.begin(), end = m_lootCorpses.end(); it != end; ++it)
	{
		if(it->position != position)
			continue;

		if(it->status == BOT_LOOT_CORPSE_LOOTED || it->status == BOT_LOOT_CORPSE_IGNORED)
			return &(*it);

		if(now > it->createdAt + BOT_LOOT_PENDING_TTL)
		{
			it->status = BOT_LOOT_CORPSE_IGNORED;
			it->lootedAt = now;
			return &(*it);
		}
		return &(*it);
	}
	return NULL;
}

Creature* Bot::findFriendByName(const std::string& name) const
{
	if(name.empty())
		return NULL;

	std::string wanted = botLowerText(name);
	knownCreatures& creatures = g_map.getKnownCreatures();
	for(knownCreatures::iterator it = creatures.begin(), end = creatures.end(); it != end; ++it)
	{
		Creature* creature = it->second;
		if(!creature || !creature->isPlayer() || !creature->isVisible() || creature->getHealth() == 0)
			continue;

		if(botLowerText(creature->getName()) == wanted)
			return creature;
	}
	return NULL;
}

void Bot::tickHealing()
{
	if(!m_healingConfig.enabled)
		return;

	Uint32 now = SDL_GetTicks();
	if(now < m_nextHealTick)
		return;

	m_nextHealTick = now + BOT_HEAL_INTERVAL;
	Position hotkeyPosition(0xFFFF, 0, 0);

	if(m_healingConfig.hpPotionId != 0 &&
		g_game.getPlayerHealthPercent() <= m_healingConfig.hpPotionPercent)
	{
		g_game.sendUseOnCreature(hotkeyPosition, m_healingConfig.hpPotionId, 0, g_game.getPlayerID());
		return;
	}

	if(m_healingConfig.mpPotionId != 0 &&
		g_game.getPlayerManaPercent() <= m_healingConfig.mpPotionPercent)
	{
		g_game.sendUseOnCreature(hotkeyPosition, m_healingConfig.mpPotionId, 0, g_game.getPlayerID());
		return;
	}

	if(m_healingConfig.uhRuneId != 0 && !m_healingConfig.friendName.empty())
	{
		Creature* friendCreature = findFriendByName(m_healingConfig.friendName);
		if(friendCreature && friendCreature->getHealth() <= m_healingConfig.friendPercent)
			g_game.sendUseOnCreature(hotkeyPosition, m_healingConfig.uhRuneId, 0, friendCreature->getId());
	}
}

bool Bot::saveProfile()
{
	return saveProfile("native_bot.cfg");
}

bool Bot::saveProfile(const std::string& profileName)
{
	std::string profilePath = botProfilePath(profileName, ".scruopt");
	std::ofstream file(profilePath.c_str(), std::ios::out | std::ios::trunc);
	if(!file.is_open())
		return false;

	file << "enabled " << (m_enabled ? 1 : 0) << "\n";
	file << "cave " << (m_cavebot.isEnabled() ? 1 : 0) << "\n";
	file << "target " << (m_targetbot.isEnabled() ? 1 : 0) << "\n";
	file << "pausecave " << (m_pauseCaveInFight ? 1 : 0) << "\n";
	file << "smartwalk " << (m_smartWalk ? 1 : 0) << "\n";
	file << "avoidplayers " << (m_avoidPlayers ? 1 : 0) << "\n";
	file << "followtarget " << (m_targetbot.isFollowTarget() ? 1 : 0) << "\n";
	file << "hotkeysenabled " << (m_hotkeysEnabled ? 1 : 0) << "\n";

	file << "healing " << (m_healingConfig.enabled ? 1 : 0) << " "
		<< SDL_static_cast(Uint32, m_healingConfig.hpPotionId) << " "
		<< SDL_static_cast(Uint32, m_healingConfig.hpPotionPercent) << " "
		<< SDL_static_cast(Uint32, m_healingConfig.mpPotionId) << " "
		<< SDL_static_cast(Uint32, m_healingConfig.mpPotionPercent) << " "
		<< SDL_static_cast(Uint32, m_healingConfig.uhRuneId) << " "
		<< SDL_static_cast(Uint32, m_healingConfig.friendPercent) << " "
		<< m_healingConfig.friendName << "\n";

	const std::vector<CaveBotWaypoint>& waypoints = m_cavebot.getWaypoints();
	for(std::vector<CaveBotWaypoint>::const_iterator it = waypoints.begin(), end = waypoints.end(); it != end; ++it)
	{
		file << "waypoint " << SDL_static_cast(Uint32, it->type) << " "
			<< SDL_static_cast(Uint32, it->position.x) << " "
			<< SDL_static_cast(Uint32, it->position.y) << " "
			<< SDL_static_cast(Uint32, it->position.z) << "\n";
	}

	const std::vector<std::string>& targets = m_targetbot.getTargetNames();
	for(std::vector<std::string>::const_iterator it = targets.begin(), end = targets.end(); it != end; ++it)
		file << "targetname " << *it << "\n";

	for(std::vector<BotLootItem>::const_iterator it = m_lootItems.begin(), end = m_lootItems.end(); it != end; ++it)
		file << "loot " << SDL_static_cast(Uint32, it->itemId) << " " << it->name << "\n";

	for(size_t i = 0, end = m_hotkeys.size(); i < end; ++i)
	{
		const BotHotkeyConfig& hotkey = m_hotkeys[i];
		file << "hotkey " << SDL_static_cast(Uint32, i) << " "
			<< (hotkey.enabled ? 1 : 0) << " "
			<< (hotkey.persistent ? 1 : 0) << " "
			<< hotkey.cooldownMs << " "
			<< hotkey.keyName << "|"
			<< hotkey.spellWords << "\n";
	}

	for(size_t i = 0, end = m_icons.size(); i < end; ++i)
	{
		const BotIconConfig& icon = m_icons[i];
		file << "icon " << SDL_static_cast(Uint32, i) << " "
			<< (icon.enabled ? 1 : 0) << " "
			<< SDL_static_cast(Uint32, icon.itemId) << " "
			<< icon.x << " "
			<< icon.y << " "
			<< icon.label << "|"
			<< icon.leftAction << "|"
			<< icon.rightAction << "|"
			<< icon.ctrlLeftAction << "|"
			<< icon.ctrlRightAction << "\n";
	}

	std::string jsonPath = profilePath;
	size_t dot = jsonPath.find_last_of('.');
	if(dot != std::string::npos)
		jsonPath.erase(dot);
	jsonPath += ".json";
	std::ofstream jsonFile(jsonPath.c_str(), std::ios::out | std::ios::trunc);
	if(jsonFile.is_open())
	{
		jsonFile << "{\n";
		jsonFile << "  \"enabled\": " << (m_enabled ? "true" : "false") << ",\n";
		jsonFile << "  \"caveEnabled\": " << (m_cavebot.isEnabled() ? "true" : "false") << ",\n";
		jsonFile << "  \"targetEnabled\": " << (m_targetbot.isEnabled() ? "true" : "false") << ",\n";
		jsonFile << "  \"followTarget\": " << (m_targetbot.isFollowTarget() ? "true" : "false") << ",\n";
		jsonFile << "  \"smartWalk\": " << (m_smartWalk ? "true" : "false") << ",\n";
		jsonFile << "  \"avoidPlayers\": " << (m_avoidPlayers ? "true" : "false") << ",\n";
		jsonFile << "  \"waypoints\": [\n";
		for(size_t i = 0, end = waypoints.size(); i < end; ++i)
		{
			const CaveBotWaypoint& waypoint = waypoints[i];
			jsonFile << "    {\"type\": \"" << CaveBot::getWaypointTypeName(waypoint.type) << "\", \"x\": "
				<< waypoint.position.x << ", \"y\": " << waypoint.position.y << ", \"z\": "
				<< SDL_static_cast(Uint32, waypoint.position.z) << "}" << (i + 1 == end ? "\n" : ",\n");
		}
		jsonFile << "  ],\n";
		jsonFile << "  \"targets\": [";
		for(size_t i = 0, end = targets.size(); i < end; ++i)
			jsonFile << (i == 0 ? "" : ", ") << "\"" << botJsonEscape(targets[i]) << "\"";
		jsonFile << "],\n";
		jsonFile << "  \"loot\": [";
		for(size_t i = 0, end = m_lootItems.size(); i < end; ++i)
		{
			jsonFile << (i == 0 ? "" : ", ")
				<< "{\"id\": " << SDL_static_cast(Uint32, m_lootItems[i].itemId)
				<< ", \"name\": \"" << botJsonEscape(m_lootItems[i].name) << "\"}";
		}
		jsonFile << "],\n";
		jsonFile << "  \"hotkeysEnabled\": " << (m_hotkeysEnabled ? "true" : "false") << ",\n";
		jsonFile << "  \"hotkeys\": [\n";
		for(size_t i = 0, end = m_hotkeys.size(); i < end; ++i)
		{
			const BotHotkeyConfig& hotkey = m_hotkeys[i];
			jsonFile << "    {\"enabled\": " << (hotkey.enabled ? "true" : "false")
				<< ", \"persistent\": " << (hotkey.persistent ? "true" : "false")
				<< ", \"key\": \"" << botJsonEscape(hotkey.keyName)
				<< "\", \"spell\": \"" << botJsonEscape(hotkey.spellWords)
				<< "\", \"cooldownMs\": " << hotkey.cooldownMs << "}" << (i + 1 == end ? "\n" : ",\n");
		}
		jsonFile << "  ],\n";
		jsonFile << "  \"icons\": [\n";
		for(size_t i = 0, end = m_icons.size(); i < end; ++i)
		{
			const BotIconConfig& icon = m_icons[i];
			jsonFile << "    {\"enabled\": " << (icon.enabled ? "true" : "false")
				<< ", \"itemId\": " << SDL_static_cast(Uint32, icon.itemId)
				<< ", \"clientId\": " << SDL_static_cast(Uint32, icon.itemId)
				<< ", \"label\": \"" << botJsonEscape(icon.label)
				<< "\", \"x\": " << icon.x << ", \"y\": " << icon.y
				<< ", \"leftAction\": \"" << botJsonEscape(icon.leftAction)
				<< "\", \"rightAction\": \"" << botJsonEscape(icon.rightAction)
				<< "\", \"ctrlLeftAction\": \"" << botJsonEscape(icon.ctrlLeftAction)
				<< "\", \"ctrlRightAction\": \"" << botJsonEscape(icon.ctrlRightAction)
				<< "\"}" << (i + 1 == end ? "\n" : ",\n");
		}
		jsonFile << "  ]\n";
		jsonFile << "}\n";
	}

	return true;
}

bool Bot::loadProfile()
{
	return loadProfile("native_bot.cfg");
}

bool Bot::loadProfile(const std::string& profileName)
{
	std::ifstream file(botProfilePath(profileName, ".scruopt").c_str());
	if(!file.is_open())
		return false;

	m_cavebot.clearWaypoints();
	m_targetbot.clearTargetNames();
	m_lootItems.clear();
	m_hotkeys.assign(BOT_HOTKEY_COUNT, BotHotkeyConfig());
	m_icons.assign(BOT_ICON_COUNT, BotIconConfig());

	std::string line;
	while(std::getline(file, line))
	{
		std::istringstream stream(line);
		std::string key;
		stream >> key;
		if(key.empty())
			continue;

		if(key == "enabled")
		{
			Sint32 value = 0;
			stream >> value;
			m_enabled = (value != 0);
		}
		else if(key == "cave")
		{
			Sint32 value = 0;
			stream >> value;
			m_cavebot.setEnabled(value != 0);
		}
		else if(key == "target")
		{
			Sint32 value = 0;
			stream >> value;
			m_targetbot.setEnabled(value != 0);
		}
		else if(key == "pausecave")
		{
			Sint32 value = 0;
			stream >> value;
			m_pauseCaveInFight = (value != 0);
		}
		else if(key == "smartwalk")
		{
			Sint32 value = 0;
			stream >> value;
			setSmartWalk(value != 0);
		}
		else if(key == "avoidplayers")
		{
			Sint32 value = 0;
			stream >> value;
			m_avoidPlayers = (value != 0);
		}
		else if(key == "followtarget")
		{
			Sint32 value = 0;
			stream >> value;
			m_targetbot.setFollowTarget(value != 0);
		}
		else if(key == "hotkeysenabled")
		{
			Sint32 value = 0;
			stream >> value;
			m_hotkeysEnabled = (value != 0);
		}
		else if(key == "healing")
		{
			Uint32 enabled = 0, hpId = 0, hpPercent = 0, mpId = 0, mpPercent = 0, uhId = 0, friendPercent = 0;
			stream >> enabled >> hpId >> hpPercent >> mpId >> mpPercent >> uhId >> friendPercent;
			m_healingConfig.enabled = (enabled != 0);
			m_healingConfig.hpPotionId = SDL_static_cast(Uint16, hpId);
			m_healingConfig.hpPotionPercent = SDL_static_cast(Uint8, UTIL_min<Uint32>(100, hpPercent));
			m_healingConfig.mpPotionId = SDL_static_cast(Uint16, mpId);
			m_healingConfig.mpPotionPercent = SDL_static_cast(Uint8, UTIL_min<Uint32>(100, mpPercent));
			m_healingConfig.uhRuneId = SDL_static_cast(Uint16, uhId);
			m_healingConfig.friendPercent = SDL_static_cast(Uint8, UTIL_min<Uint32>(100, friendPercent));
			std::string friendName;
			std::getline(stream, friendName);
			m_healingConfig.friendName = botTrimLeft(friendName);
		}
		else if(key == "waypoint")
		{
			Uint32 type = 0, x = 0, y = 0, z = 0;
			stream >> type >> x >> y >> z;
			m_cavebot.addWaypoint(Position(SDL_static_cast(Uint16, x), SDL_static_cast(Uint16, y), SDL_static_cast(Uint8, z)),
				SDL_static_cast(CaveBotWaypointType, type));
		}
		else if(key == "targetname")
		{
			std::string name;
			std::getline(stream, name);
			m_targetbot.addTargetName(botTrimLeft(name));
		}
		else if(key == "loot")
		{
			Uint32 itemId = 0;
			stream >> itemId;
			std::string itemName;
			std::getline(stream, itemName);
			itemName = botTrimLeft(itemName);
			if(addLootItem(SDL_static_cast(Uint16, itemId)) && !itemName.empty())
				m_lootItems.back().name = itemName;
		}
		else if(key == "hotkey")
		{
			Uint32 index = 0, enabled = 0, persistent = 0, cooldown = 0;
			stream >> index >> enabled >> persistent >> cooldown;

			std::string rest;
			std::getline(stream, rest);
			rest = botTrimLeft(rest);
			size_t divider = rest.find('|');

			BotHotkeyConfig hotkey;
			hotkey.enabled = (enabled != 0);
			hotkey.persistent = (persistent != 0);
			hotkey.cooldownMs = cooldown;
			if(divider != std::string::npos)
			{
				hotkey.keyName = botTrim(rest.substr(0, divider));
				hotkey.spellWords = botTrim(rest.substr(divider + 1));
			}
			setHotkey(index, hotkey);
		}
		else if(key == "icon")
		{
			Uint32 index = 0, enabled = 0, itemId = 0;
			Sint32 x = 0, y = 0;
			stream >> index >> enabled >> itemId >> x >> y;

			std::string label;
			std::getline(stream, label);
			label = botTrimLeft(label);
			std::string leftAction;
			std::string rightAction;
			std::string ctrlLeftAction;
			std::string ctrlRightAction;
			size_t divider = label.find('|');
			if(divider != std::string::npos)
			{
				std::vector<std::string> parts;
				size_t begin = 0;
				while(true)
				{
					size_t end = label.find('|', begin);
					parts.push_back(label.substr(begin, end == std::string::npos ? std::string::npos : end - begin));
					if(end == std::string::npos)
						break;
					begin = end + 1;
				}

				label = parts.size() > 0 ? botTrim(parts[0]) : std::string();
				leftAction = parts.size() > 1 ? botTrim(parts[1]) : std::string();
				rightAction = parts.size() > 2 ? botTrim(parts[2]) : std::string();
				ctrlLeftAction = parts.size() > 3 ? botTrim(parts[3]) : std::string();
				ctrlRightAction = parts.size() > 4 ? botTrim(parts[4]) : std::string();
			}

			BotIconConfig icon;
			icon.enabled = (enabled != 0);
			icon.itemId = SDL_static_cast(Uint16, itemId);
			icon.x = x;
			icon.y = y;
			icon.label = label;
			icon.leftAction = leftAction;
			icon.rightAction = rightAction;
			icon.ctrlLeftAction = ctrlLeftAction;
			icon.ctrlRightAction = ctrlRightAction;
			setIcon(index, icon);
		}
	}

	m_nextHealTick = 0;
	return true;
}

bool Bot::deleteProfile(const std::string& profileName)
{
	std::string profilePath = botProfilePath(profileName, ".scruopt");
	bool removed = (std::remove(profilePath.c_str()) == 0);
	size_t dot = profilePath.find_last_of('.');
	if(dot != std::string::npos)
		profilePath.erase(dot);
	profilePath += ".json";
	std::remove(profilePath.c_str());
	return removed;
}

std::string Bot::getStatusText() const
{
	if(!m_enabled)
		return "Bot: OFF";

	std::string status = "Bot: ON | ";
	status += m_targetbot.getStatusText();
	status += " | ";
	status += m_cavebot.getStatusText();
	return status;
}
