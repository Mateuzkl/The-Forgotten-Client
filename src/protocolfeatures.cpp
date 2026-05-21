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

#include "protocolfeatures.h"
#include "game.h"
#include "util.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdarg>
#include <memory>
#include <utility>

extern Game g_game;
extern Uint32 g_clientVersion;

namespace
{
	struct SdlRwopsDeleter
	{
		void operator()(SDL_RWops* rwops) const
		{
			if(rwops)
				SDL_RWclose(rwops);
		}
	};

	struct FeatureAlias
	{
		const char* name;
		GameFeatures feature;
	};

	const std::vector<FeatureAlias>& featureAliases()
	{
		static const std::vector<FeatureAlias> aliases =
		{
			{"GameXtea", GAME_FEATURE_XTEA},
		{"GameLoginPacketEncryption", GAME_FEATURE_XTEA},
		{"GameRSA1024", GAME_FEATURE_RSA1024},
		{"GameMessageStatement", GAME_FEATURE_MESSAGE_STATEMENT},
		{"GameMessageStatements", GAME_FEATURE_MESSAGE_STATEMENT},
		{"GameLooktypeU16", GAME_FEATURE_LOOKTYPE_U16},
		{"GameNewFluids", GAME_FEATURE_NEWFLUIDS},
		{"GameMessageLevel", GAME_FEATURE_MESSAGE_LEVEL},
		{"GamePlayerStateU16", GAME_FEATURE_PLAYERICONS_U16},
		{"GamePlayerIconsU16", GAME_FEATURE_PLAYERICONS_U16},
		{"GamePlayerAddons", GAME_FEATURE_ADDONS},
		{"GamePlayerStamina", GAME_FEATURE_STAMINA},
		{"GameNewOutfitProtocol", GAME_FEATURE_NEWOUTFITS},
		{"GameWritableDate", GAME_FEATURE_WRITABLE_DATE},
		{"GameNpcInterface", GAME_FEATURE_NPC_INTERFACE},
		{"GameProtocolChecksum", GAME_FEATURE_CHECKSUM},
		{"GameAccountNames", GAME_FEATURE_ACCOUNT_NAME},
		{"GameChallengeOnLogin", GAME_FEATURE_SERVER_SENDFIRST},
		{"GameServerSendsFirst", GAME_FEATURE_SERVER_SENDFIRST},
		{"GameServerSendFirst", GAME_FEATURE_SERVER_SENDFIRST},
		{"GameDoubleCapacity", GAME_FEATURE_DOUBLE_CAPACITY},
		{"GameDoubleFreeCapacity", GAME_FEATURE_DOUBLE_CAPACITY},
		{"GameTileAddThingWithStackpos", GAME_FEATURE_TILE_ADDTHING_STACKPOS},
		{"GameCreatureEmblems", GAME_FEATURE_CREATURE_EMBLEM},
		{"GameAttackSeq", GAME_FEATURE_ATTACK_SEQUENCE},
		{"GameAttackSequence", GAME_FEATURE_ATTACK_SEQUENCE},
		{"GameDeathPenalty", GAME_FEATURE_DEATH_PENALTY},
		{"GameMounts", GAME_FEATURE_MOUNTS},
		{"GamePlayerMounts", GAME_FEATURE_MOUNTS},
		{"GameDoubleExperience", GAME_FEATURE_DOUBLE_EXPERIENCE},
		{"GameSpellList", GAME_FEATURE_SPELL_LIST},
		{"GameChatPlayerList", GAME_FEATURE_CHAT_PLAYERLIST},
		{"GameTotalCapacity", GAME_FEATURE_TOTAL_CAPACITY},
		{"GameBaseSkills", GAME_FEATURE_BASE_SKILLS},
		{"GameSkillsBase", GAME_FEATURE_BASE_SKILLS},
		{"GameRegenerationTime", GAME_FEATURE_REGENERATION_TIME},
		{"GameItemAnimationPhases", GAME_FEATURE_ITEM_ANIMATION_PHASES},
		{"GameEnvironmentEffects", GAME_FEATURE_ENVIRONMENT_EFFECTS},
		{"GameNpcNameOnTrade", GAME_FEATURE_NPC_NAME_ON_TRADE},
		{"GameMarket", GAME_FEATURE_MARKET},
		{"GamePlayerMarket", GAME_FEATURE_MARKET},
		{"GamePing", GAME_FEATURE_PING},
		{"GamePurseSlot", GAME_FEATURE_PURSE_SLOT},
		{"GameOfflineTraining", GAME_FEATURE_OFFLINE_TRAINING},
		{"GameOfflineTrainingTime", GAME_FEATURE_OFFLINE_TRAINING},
		{"GameExtendedSprites", GAME_FEATURE_EXTENDED_SPRITES},
		{"GameSpritesU32", GAME_FEATURE_EXTENDED_SPRITES},
		{"GameLookAtCreature", GAME_FEATURE_LOOKATCREATURE},
		{"GameAdditionalVipInfo", GAME_FEATURE_ADDITIONAL_VIPINFO},
		{"GamePreviewState", GAME_FEATURE_PREVIEW_STATE},
		{"GameClientVersion", GAME_FEATURE_CLIENT_VERSION},
		{"GameKeepConnectionAfterDeath", GAME_FEATURE_KEEP_CONNECTION_AFTER_DEATH},
		{"GameLoginPending", GAME_FEATURE_LOGIN_PENDING},
		{"GameVipStatus", GAME_FEATURE_VIP_STATUS},
		{"GameNewSpeedLaw", GAME_FEATURE_NEWSPEED_LAW},
		{"GameBrowseField", GAME_FEATURE_BROWSEFIELD},
		{"GameContainerPagination", GAME_FEATURE_CONTAINER_PAGINATION},
		{"GamePVPMode", GAME_FEATURE_PVP_MODE},
		{"GamePvpMode", GAME_FEATURE_PVP_MODE},
		{"GameItemMarks", GAME_FEATURE_ITEM_MARK},
		{"GameCreatureMarks", GAME_FEATURE_CREATURE_MARK},
		{"GameCreatureType", GAME_FEATURE_CREATURE_TYPE},
		{"GameDoubleSkills", GAME_FEATURE_DOUBLE_SKILLS},
		{"GameCreatureIcons", GAME_FEATURE_CREATURE_ICONS},
		{"GameHideNpcNames", GAME_FEATURE_HIDE_NPC_NAMES},
		{"GamePremiumExpiration", GAME_FEATURE_PREMIUM_EXPIRATION},
		{"GameEnhancedAnimations", GAME_FEATURE_ENHANCED_ANIMATIONS},
		{"GameUnjustifiedPoints", GAME_FEATURE_UNJUSTIFIED_POINTS},
		{"GameExperienceBonus", GAME_FEATURE_EXPERIENCE_BONUS},
		{"GameDeathType", GAME_FEATURE_DEATH_TYPE},
		{"GameFrameGroups", GAME_FEATURE_FRAMEGROUPS},
		{"GameIdleAnimations", GAME_FEATURE_FRAMEGROUPS},
		{"GameRenderInformation", GAME_FEATURE_RENDER_INFORMATION},
		{"GameOGLInformation", GAME_FEATURE_RENDER_INFORMATION},
		{"GameContentRevision", GAME_FEATURE_CONTENT_REVISION},
		{"GameAuthenticator", GAME_FEATURE_AUTHENTICATOR},
		{"GameSessionKey", GAME_FEATURE_SESSIONKEY},
		{"GameEquipHotkey", GAME_FEATURE_EQUIP_HOTKEY},
		{"GameStore", GAME_FEATURE_STORE},
		{"GameIngameStore", GAME_FEATURE_STORE},
		{"GameWrappable", GAME_FEATURE_WRAPPABLE},
		{"GameStoreInbox", GAME_FEATURE_STORE_INBOX},
		{"GameStoreServiceType", GAME_FEATURE_STORE_SERVICETYPE},
		{"GameIngameStoreServiceType", GAME_FEATURE_STORE_SERVICETYPE},
		{"GameStoreHighlights", GAME_FEATURE_STORE_HIGHLIGHTS},
		{"GameIngameStoreHighlights", GAME_FEATURE_STORE_HIGHLIGHTS},
		{"GameAdditionalSkills", GAME_FEATURE_ADDITIONAL_SKILLS},
		{"GameDetailedExperienceBonus", GAME_FEATURE_DETAILED_EXPERIENCE_BONUS},
		{"GameStoreHighlights2", GAME_FEATURE_STORE_HIGHLIGHTS2},
		{"GameNewFilesStructure", GAME_FEATURE_NEWFILES_STRUCTURE},
		{"GameClientSendServerName", GAME_FEATURE_CLIENT_SENDSERVERNAME},
		{"GamePrey", GAME_FEATURE_PREY},
		{"GameImbuing", GAME_FEATURE_IMBUING},
		{"GameVipGroups", GAME_FEATURE_VIP_GROUPS},
		{"GameInspection", GAME_FEATURE_INSPECTION},
		{"GameSequencedPackets", GAME_FEATURE_PROTOCOLSEQUENCE},
		{"GameProtocolSequence", GAME_FEATURE_PROTOCOLSEQUENCE},
		{"GameBlessDialog", GAME_FEATURE_BLESS_DIALOG},
		{"GameQuestTracker", GAME_FEATURE_QUEST_TRACKER},
		{"GameTime", GAME_FEATURE_GAMETIME},
		{"GameCompedium", GAME_FEATURE_COMPEDIUM},
		{"GamePlayerStateU32", GAME_FEATURE_PLAYERICONS_U32},
		{"GamePlayerIconsU32", GAME_FEATURE_PLAYERICONS_U32},
		{"GameRewardWall", GAME_FEATURE_REWARD_WALL},
		{"GameAnalytics", GAME_FEATURE_ANALYTICS},
		{"GameQuickLoot", GAME_FEATURE_QUICK_LOOT},
		{"GameExtendedCapacity", GAME_FEATURE_EXTENDED_CAPACITY},
		{"GameCyclopediaMonsters", GAME_FEATURE_CYCLOPEDIA_MONSTERS},
		{"GameStash", GAME_FEATURE_STASH},
		{"GameCyclopediaMapAndDetails", GAME_FEATURE_CYCLOPEDIA_MAPANDDETAILS},
		{"GameDoublePercentSkills", GAME_FEATURE_DOUBLE_PERCENT_SKILLS},
		{"GameTournaments", GAME_FEATURE_TOURNAMENTS},
		{"GameAccountEmail", GAME_FEATURE_ACCOUNT_EMAIL},
		{"GameUpdateTile", GAME_FEATURE_UPDATE_TILE},
		{"GameItemsU16", GAME_FEATURE_ITEMS_U16},
		{"GameMagicEffectU16", GAME_FEATURE_EFFECTS_U16},
		{"GameEffectU16", GAME_FEATURE_EFFECTS_U16},
		{"GameEffectsU16", GAME_FEATURE_EFFECTS_U16},
		{"GameDistanceEffectU16", GAME_FEATURE_DISTANCEEFFECTS_U16},
		{"GameDistanceEffectsU16", GAME_FEATURE_DISTANCEEFFECTS_U16},
		{"GameDoubleShopBuyAmount", GAME_FEATURE_DOUBLE_SHOPBUYAMOUNT},
		{"GameDoubleShopSellAmount", GAME_FEATURE_DOUBLE_SHOPSELLAMOUNT},
		{"GameDoubleHealth", GAME_FEATURE_DOUBLE_HEALTH},
		{"GameMinimapRemoveMark", GAME_FEATURE_MINIMAP_REMOVEMARK},
		{"GameBaseSkillU16", GAME_FEATURE_BASE_SKILL_U16},
		{"GameDoublePlayerGoodsMoney", GAME_FEATURE_DOUBLE_PLAYER_GOODS_MONEY},
		{"GameExtendedClientPing", GAME_FEATURE_EXTENDED_CLIENT_PING},
		{"GameThingUpgradeClassification", GAME_FEATURE_ITEM_TIER_BYTE},
			{"GameItemTierByte", GAME_FEATURE_ITEM_TIER_BYTE}
		};
		return aliases;
	}

	bool isIdentifierStart(char ch)
	{
		return (std::isalpha(SDL_static_cast(unsigned char, ch)) != 0 || ch == '_');
	}

	bool isIdentifierChar(char ch)
	{
		return (std::isalnum(SDL_static_cast(unsigned char, ch)) != 0 || ch == '_');
	}

	size_t skipWhitespace(const std::string& data, size_t pos)
	{
		while(pos < data.size() && std::isspace(SDL_static_cast(unsigned char, data[pos])) != 0)
			++pos;
		return pos;
	}

	bool isWordBoundary(const std::string& data, size_t pos, size_t length)
	{
		if(pos > 0 && isIdentifierChar(data[pos - 1]))
			return false;
		const size_t end = pos + length;
		return (end >= data.size() || !isIdentifierChar(data[end]));
	}

	std::string stripLuaComments(const std::string& data)
	{
		std::string result;
		result.reserve(data.size());
		bool inString = false;
		char stringQuote = 0;
		for(size_t i = 0; i < data.size(); ++i)
		{
			const char ch = data[i];
			if(inString)
			{
				result.push_back(ch);
				if(ch == '\\' && i + 1 < data.size())
					result.push_back(data[++i]);
				else if(ch == stringQuote)
					inString = false;
				continue;
			}
			if(ch == '"' || ch == '\'')
			{
				inString = true;
				stringQuote = ch;
				result.push_back(ch);
				continue;
			}
			if(ch == '-' && i + 1 < data.size() && data[i + 1] == '-')
			{
				while(i < data.size() && data[i] != '\n')
					++i;
				if(i < data.size())
					result.push_back('\n');
				continue;
			}
			result.push_back(ch);
		}
		return result;
	}

	bool parseQuotedString(const std::string& data, size_t& pos, std::string& output)
	{
		pos = skipWhitespace(data, pos);
		if(pos >= data.size() || (data[pos] != '"' && data[pos] != '\''))
			return false;
		const char quote = data[pos++];
		output.clear();
		while(pos < data.size())
		{
			const char ch = data[pos++];
			if(ch == '\\' && pos < data.size())
			{
				output.push_back(data[pos++]);
				continue;
			}
			if(ch == quote)
				return true;
			output.push_back(ch);
		}
		return false;
	}

	bool findMatchingBrace(const std::string& data, size_t openPos, size_t& closePos)
	{
		if(openPos >= data.size() || data[openPos] != '{')
			return false;
		size_t depth = 0;
		bool inString = false;
		char stringQuote = 0;
		for(size_t pos = openPos; pos < data.size(); ++pos)
		{
			const char ch = data[pos];
			if(inString)
			{
				if(ch == '\\' && pos + 1 < data.size())
					++pos;
				else if(ch == stringQuote)
					inString = false;
				continue;
			}
			if(ch == '"' || ch == '\'')
			{
				inString = true;
				stringQuote = ch;
				continue;
			}
			if(ch == '{')
				++depth;
			else if(ch == '}')
			{
				if(--depth == 0)
				{
					closePos = pos;
					return true;
				}
			}
		}
		return false;
	}

	size_t findAssignment(const std::string& data, const std::string& key)
	{
		size_t pos = 0;
		while((pos = data.find(key, pos)) != std::string::npos)
		{
			if(!isWordBoundary(data, pos, key.size()))
			{
				++pos;
				continue;
			}
			size_t valuePos = skipWhitespace(data, pos + key.size());
			if(valuePos < data.size() && data[valuePos] == '=')
				return valuePos + 1;
			++pos;
		}
		return std::string::npos;
	}

	bool extractTable(const std::string& data, const std::string& key, std::string& table)
	{
		size_t valuePos = findAssignment(data, key);
		if(valuePos == std::string::npos)
			return false;
		valuePos = skipWhitespace(data, valuePos);
		if(valuePos >= data.size() || data[valuePos] != '{')
			return false;
		size_t closePos = 0;
		if(!findMatchingBrace(data, valuePos, closePos))
			return false;
		table.assign(data, valuePos + 1, closePos - valuePos - 1);
		return true;
	}

	bool parseStringAssignment(const std::string& data, const std::string& key, std::string& output)
	{
		size_t valuePos = findAssignment(data, key);
		if(valuePos == std::string::npos)
			return false;
		return parseQuotedString(data, valuePos, output);
	}

	bool parseNumberAssignment(const std::string& data, const std::string& key, Uint32& output)
	{
		size_t valuePos = findAssignment(data, key);
		if(valuePos == std::string::npos)
			return false;
		valuePos = skipWhitespace(data, valuePos);
		if(valuePos >= data.size() || std::isdigit(SDL_static_cast(unsigned char, data[valuePos])) == 0)
			return false;
		output = SDL_static_cast(Uint32, SDL_strtoul(data.c_str() + valuePos, NULL, 10));
		return true;
	}

	bool parseBoolAssignment(const std::string& data, const std::string& key, bool& output)
	{
		size_t valuePos = findAssignment(data, key);
		if(valuePos == std::string::npos)
			return false;
		valuePos = skipWhitespace(data, valuePos);
		if(data.compare(valuePos, 4, "true") == 0 && isWordBoundary(data, valuePos, 4))
		{
			output = true;
			return true;
		}
		if(data.compare(valuePos, 5, "false") == 0 && isWordBoundary(data, valuePos, 5))
		{
			output = false;
			return true;
		}
		return false;
	}

	std::string valueTypeAt(const std::string& data, size_t pos)
	{
		pos = skipWhitespace(data, pos);
		if(pos >= data.size())
			return "empty";
		if(data[pos] == '"' || data[pos] == '\'')
			return "string";
		if(data[pos] == '{')
			return "table";
		if(std::isdigit(SDL_static_cast(unsigned char, data[pos])) != 0 || data[pos] == '-' || data[pos] == '+')
			return "number";
		if(isIdentifierStart(data[pos]))
		{
			const size_t begin = pos;
			while(pos < data.size() && isIdentifierChar(data[pos]))
				++pos;
			return data.substr(begin, pos - begin);
		}
		return "unknown";
	}

	size_t skipLuaValue(const std::string& data, size_t pos)
	{
		pos = skipWhitespace(data, pos);
		if(pos >= data.size())
			return pos;
		if(data[pos] == '"' || data[pos] == '\'')
		{
			std::string ignored;
			parseQuotedString(data, pos, ignored);
			return pos;
		}
		if(data[pos] == '{')
		{
			size_t closePos = pos;
			if(findMatchingBrace(data, pos, closePos))
				return closePos + 1;
		}
		while(pos < data.size() && data[pos] != ',' && data[pos] != '\n' && data[pos] != '\r')
			++pos;
		return pos;
	}

	std::string readIdentifierOrKey(const std::string& data, size_t& pos)
	{
		pos = skipWhitespace(data, pos);
		if(pos >= data.size())
			return std::string();
		if(data[pos] == '[')
		{
			++pos;
			std::string key;
			if(parseQuotedString(data, pos, key))
			{
				pos = skipWhitespace(data, pos);
				if(pos < data.size() && data[pos] == ']')
					++pos;
				return key;
			}
			return std::string();
		}
		if(!isIdentifierStart(data[pos]))
			return std::string();
		const size_t begin = pos;
		while(pos < data.size() && isIdentifierChar(data[pos]))
			++pos;
		return data.substr(begin, pos - begin);
	}

	void parseServerProfiles(const std::string& data, std::map<std::string, std::string>& serverProfiles)
	{
		size_t pos = 0;
		while(pos < data.size())
		{
			std::string key = readIdentifierOrKey(data, pos);
			if(key.empty())
			{
				++pos;
				continue;
			}
			pos = skipWhitespace(data, pos);
			if(pos >= data.size() || data[pos] != '=')
				continue;
			++pos;
			std::string profile;
			if(parseQuotedString(data, pos, profile) && !key.empty() && !profile.empty())
				serverProfiles[key] = profile;
		}
	}

	void parseFeatureSettings(const std::string& data, ProtocolFeatureManager::Profile& profile)
	{
		size_t pos = 0;
		while(pos < data.size())
		{
			std::string key = readIdentifierOrKey(data, pos);
			if(key.empty())
			{
				++pos;
				continue;
			}
			pos = skipWhitespace(data, pos);
			if(pos >= data.size() || data[pos] != '=')
				continue;
			++pos;
			pos = skipWhitespace(data, pos);
			if(key == "inherit")
			{
				std::string inheritProfile;
				if(parseQuotedString(data, pos, inheritProfile))
					profile.inheritProfile = inheritProfile;
				continue;
			}

			ProtocolFeatureManager::FeatureSetting setting;
			setting.name = key;
			if(data.compare(pos, 4, "true") == 0 && isWordBoundary(data, pos, 4))
			{
				setting.enabled = true;
				setting.valid = true;
				pos += 4;
			}
			else if(data.compare(pos, 5, "false") == 0 && isWordBoundary(data, pos, 5))
			{
				setting.enabled = false;
				setting.valid = true;
				pos += 5;
			}
			else
			{
				setting.valid = false;
				setting.valueType = valueTypeAt(data, pos);
				pos = skipLuaValue(data, pos);
			}
			profile.features.emplace_back(std::move(setting));
		}
	}

	void parseProfiles(const std::string& data, std::map<std::string, ProtocolFeatureManager::Profile>& profiles)
	{
		size_t pos = 0;
		while(pos < data.size())
		{
			std::string name = readIdentifierOrKey(data, pos);
			if(name.empty())
			{
				++pos;
				continue;
			}
			pos = skipWhitespace(data, pos);
			if(pos >= data.size() || data[pos] != '=')
				continue;
			++pos;
			pos = skipWhitespace(data, pos);
			if(pos >= data.size() || data[pos] != '{')
				continue;
			size_t closePos = 0;
			if(!findMatchingBrace(data, pos, closePos))
				break;
			const std::string block = data.substr(pos + 1, closePos - pos - 1);

			ProtocolFeatureManager::Profile profile;
			profile.name = name;
			parseStringAssignment(block, "description", profile.description);
			parseNumberAssignment(block, "protocol", profile.protocol);
			parseNumberAssignment(block, "clientVersion", profile.clientVersion);
			parseNumberAssignment(block, "fileVersion", profile.fileVersion);
			parseNumberAssignment(block, "contentRevision", profile.contentRevision);
			parseNumberAssignment(block, "previewState", profile.previewState);
			parseBoolAssignment(block, "inheritDefaults", profile.inheritDefaults);

			std::string featuresBlock;
			if(extractTable(block, "features", featuresBlock))
				parseFeatureSettings(featuresBlock, profile);
			profiles[profile.name] = std::move(profile);
			pos = closePos + 1;
		}
	}

	std::string buildPath(const std::string& first, const std::string& second)
	{
		if(first.empty())
			return second;
		if(first[first.size() - 1] == PATH_PLATFORM_SLASH)
			return first + second;
		return first + PATH_PLATFORM_SLASH + second;
	}
}

ProtocolFeatureManager& getProtocolFeatureManager()
{
	static ProtocolFeatureManager manager;
	return manager;
}

void ProtocolFeatureManager::clear()
{
	m_loaded = false;
	m_loadedPath.clear();
	m_selectedProfile.clear();
	m_currentProfile.clear();
	m_defaultProtocol = 0;
	m_currentProtocolVersion = 0;
	m_currentClientVersion = 0;
	m_currentFileVersion = 0;
	m_defaultHost.clear();
	m_defaultPort.clear();
	m_lockedFeatures.reset();
	m_serverProfiles.clear();
	m_profiles.clear();
}

void ProtocolFeatureManager::reload()
{
	clear();
	m_loadAttempted = false;
	load();
}

bool ProtocolFeatureManager::load()
{
	if(m_loadAttempted)
		return m_loaded;

	m_loadAttempted = true;
	log("[Features] Loading features.lua...");

	std::vector<std::string> paths;
	if(!g_prefPath.empty())
	{
		paths.emplace_back(buildPath(g_prefPath, "features.lua"));
		paths.emplace_back(buildPath(buildPath(g_prefPath, "data"), "features.lua"));
	}
	if(!g_basePath.empty())
	{
		paths.emplace_back(buildPath(buildPath(g_basePath, "data"), "features.lua"));
		paths.emplace_back(buildPath(g_basePath, "features.lua"));
	}
	paths.emplace_back(buildPath("data", "features.lua"));
	paths.emplace_back("features.lua");

	for(const std::string& path : paths)
	{
		if(loadFromFile(path))
			return true;
	}

	log("[Features] Failed to load features.lua: file not found or invalid.");
	log("[Features] Using built-in fallback profile.");
	return false;
}

bool ProtocolFeatureManager::loadFromFile(const std::string& path)
{
	std::unique_ptr<SDL_RWops, SdlRwopsDeleter> rwops(SDL_RWFromFile(path.c_str(), "rb"));
	if(!rwops)
		return false;

	Sint64 size = SDL_RWsize(rwops.get());
	if(size <= 0)
	{
		log("[Features] Failed to load features.lua: '%s' is empty.", path.c_str());
		return false;
	}

	std::string content(SDL_static_cast(size_t, size), '\0');
	if(SDL_RWread(rwops.get(), &content[0], 1, SDL_static_cast(size_t, size)) != SDL_static_cast(size_t, size))
	{
		log("[Features] Failed to load features.lua: could not read '%s'.", path.c_str());
		return false;
	}

	clear();
	const std::string data = stripLuaComments(content);
	parseStringAssignment(data, "selectedProfile", m_selectedProfile);

	std::string defaultBlock;
	if(extractTable(data, "default", defaultBlock))
	{
		parseNumberAssignment(defaultBlock, "protocol", m_defaultProtocol);
		parseStringAssignment(defaultBlock, "host", m_defaultHost);

		std::string portString;
		if(parseStringAssignment(defaultBlock, "port", portString))
			m_defaultPort = portString;
		else
		{
			Uint32 defaultPort = 0;
			if(parseNumberAssignment(defaultBlock, "port", defaultPort))
			{
				std::array<char, 32> buffer;
				SDL_snprintf(buffer.data(), buffer.size(), "%u", defaultPort);
				m_defaultPort = buffer.data();
			}
		}
	}

	std::string serverProfilesBlock;
	if(extractTable(data, "serverProfiles", serverProfilesBlock))
		parseServerProfiles(serverProfilesBlock, m_serverProfiles);

	std::string profilesBlock;
	if(extractTable(data, "profiles", profilesBlock))
		parseProfiles(profilesBlock, m_profiles);

	if(m_profiles.empty())
	{
		log("[Features] Failed to load features.lua: no profiles found in '%s'.", path.c_str());
		clear();
		return false;
	}

	m_loaded = true;
	m_loadedPath = path;
	log("[Features] Loaded features.lua successfully.");
	log("[Features] Loaded from: %s", m_loadedPath.c_str());
	return true;
}

Uint32 ProtocolFeatureManager::getDefaultProtocol(Uint32 fallback) const
{
	return (m_loaded && m_defaultProtocol != 0 ? m_defaultProtocol : fallback);
}

std::string ProtocolFeatureManager::getDefaultHost(const std::string& fallback) const
{
	if(m_loaded && !m_defaultHost.empty())
		return m_defaultHost;
	return fallback;
}

std::string ProtocolFeatureManager::getDefaultPort(const std::string& fallback) const
{
	if(m_loaded && !m_defaultPort.empty())
		return m_defaultPort;
	return fallback;
}

const ProtocolFeatureManager::Profile* ProtocolFeatureManager::findProfile(const std::string& profileName) const
{
	const auto it = m_profiles.find(profileName);
	return (it == m_profiles.end() ? NULL : &it->second);
}

std::string ProtocolFeatureManager::makeServerKey(const std::string& host, const std::string& port) const
{
	if(host.empty() || port.empty())
		return std::string();
	return host + ":" + port;
}

std::string ProtocolFeatureManager::resolveProfileName(const std::string& host, const std::string& port) const
{
	const std::string serverKey = makeServerKey(host, port);
	if(!serverKey.empty())
	{
		const auto it = m_serverProfiles.find(serverKey);
		if(it != m_serverProfiles.end())
		{
			log("[Features] Server override: %s -> %s", serverKey.c_str(), it->second.c_str());
			return it->second;
		}
	}
	if(!m_selectedProfile.empty())
		return m_selectedProfile;
	return std::string("tfs860_custom");
}

bool ProtocolFeatureManager::applyForServer(Uint32 clientVersion, Uint32 fileVersion, const std::string& host, const std::string& port)
{
	if(!load())
	{
		m_lockedFeatures.reset();
		g_game.applyDefaultFeaturesForVersion(clientVersion, fileVersion);
		m_currentProfile = "built_in_fallback";
		m_currentProtocolVersion = clientVersion;
		m_currentClientVersion = clientVersion;
		m_currentFileVersion = fileVersion;
		return false;
	}

	std::string profileName = resolveProfileName(host, port);
	if(findProfile(profileName) == NULL)
	{
		log("[Features] Profile '%s' not found. Using selectedProfile/fallback.", profileName.c_str());
		if(!m_selectedProfile.empty() && m_selectedProfile != profileName && findProfile(m_selectedProfile) != NULL)
			profileName = m_selectedProfile;
		else if(findProfile("tfs860_custom") != NULL)
			profileName = "tfs860_custom";
		else
		{
			log("[Features] Using built-in fallback profile.");
			m_lockedFeatures.reset();
			g_game.applyDefaultFeaturesForVersion(clientVersion, fileVersion);
			m_currentProfile = "built_in_fallback";
			m_currentProtocolVersion = clientVersion;
			m_currentClientVersion = clientVersion;
			m_currentFileVersion = fileVersion;
			return false;
		}
	}
	return applyProfile(profileName, clientVersion, fileVersion);
}

bool ProtocolFeatureManager::applyProfile(const std::string& profileName, Uint32 clientVersion, Uint32 fileVersion)
{
	if(!load())
	{
		m_lockedFeatures.reset();
		g_game.applyDefaultFeaturesForVersion(clientVersion, fileVersion);
		m_currentProfile = "built_in_fallback";
		m_currentProtocolVersion = clientVersion;
		m_currentClientVersion = clientVersion;
		m_currentFileVersion = fileVersion;
		return false;
	}

	const Profile* profile = findProfile(profileName);
	if(!profile)
	{
		log("[Features] Profile '%s' not found. Using built-in fallback profile.", profileName.c_str());
		m_lockedFeatures.reset();
		g_game.applyDefaultFeaturesForVersion(clientVersion, fileVersion);
		m_currentProfile = "built_in_fallback";
		m_currentProtocolVersion = clientVersion;
		m_currentClientVersion = clientVersion;
		m_currentFileVersion = fileVersion;
		return false;
	}

	const Uint32 protocol = (profile->protocol != 0 ? profile->protocol : getDefaultProtocol(clientVersion));
	const Uint32 profileClientVersion = (profile->clientVersion != 0 ? profile->clientVersion : protocol);
	const Uint32 profileFileVersion = (profile->fileVersion != 0 ? profile->fileVersion : protocol);

	m_currentProfile = profile->name;
	m_currentProtocolVersion = protocol;
	m_currentClientVersion = profileClientVersion;
	m_currentFileVersion = profileFileVersion;
	g_clientVersion = profileClientVersion;

	log("[Features] Selected profile: %s", m_currentProfile.c_str());
	log("[Features] Protocol version: %u", protocol);
	log("[Features] Resetting features...");
	m_lockedFeatures.reset();
	if(profile->inheritDefaults)
		g_game.applyDefaultFeaturesForVersion(protocol, profileFileVersion);
	else
		g_game.resetGameFeatures();

	Uint32 enabledCount = 0;
	Uint32 disabledCount = 0;
	std::vector<std::string> profileStack;
	const bool applied = applyResolvedProfile(*profile, protocol, profileFileVersion, profileStack, enabledCount, disabledCount);
	log("[Features] Applied %u enabled features, %u disabled features.", enabledCount, disabledCount);
	log("[Features] Finished applying profile: %s", m_currentProfile.c_str());
	return applied;
}

bool ProtocolFeatureManager::applyResolvedProfile(const Profile& profile, Uint32 clientVersion, Uint32 fileVersion, std::vector<std::string>& profileStack, Uint32& enabledCount, Uint32& disabledCount)
{
	(void)clientVersion;
	(void)fileVersion;
	if(std::find(profileStack.begin(), profileStack.end(), profile.name) != profileStack.end())
	{
		log("[Features] Profile inheritance loop detected at '%s'. Ignored.", profile.name.c_str());
		return false;
	}

	profileStack.emplace_back(profile.name);
	if(!profile.inheritProfile.empty())
	{
		const Profile* inheritedProfile = findProfile(profile.inheritProfile);
		if(inheritedProfile)
			applyResolvedProfile(*inheritedProfile, clientVersion, fileVersion, profileStack, enabledCount, disabledCount);
		else
			log("[Features] Profile '%s' inherits missing profile '%s'.", profile.name.c_str(), profile.inheritProfile.c_str());
	}

	bool result = true;
	for(const FeatureSetting& setting : profile.features)
	{
		if(!applyFeatureSetting(setting, enabledCount, disabledCount))
			result = false;
	}
	profileStack.pop_back();
	return result;
}

bool ProtocolFeatureManager::applyFeatureSetting(const FeatureSetting& setting, Uint32& enabledCount, Uint32& disabledCount)
{
	if(!setting.valid)
	{
		log("[Features] Invalid value for %s. Expected boolean, got %s. Ignored.", setting.name.c_str(), setting.valueType.c_str());
		return false;
	}

	std::vector<GameFeatures> features;
	if(!resolveFeatureName(setting.name, features))
	{
		log("[Features] Unknown feature '%s', ignored.", setting.name.c_str());
		return false;
	}

	for(GameFeatures feature : features)
	{
		g_game.forceGameFeature(feature, setting.enabled);
		m_lockedFeatures.set(feature, true);
	}

	if(setting.enabled)
	{
		enabledCount += SDL_static_cast(Uint32, features.size());
		log("[Features] Enabled: %s", setting.name.c_str());
	}
	else
	{
		disabledCount += SDL_static_cast(Uint32, features.size());
		log("[Features] Disabled: %s", setting.name.c_str());
	}
	return true;
}

bool ProtocolFeatureManager::isFeatureLocked(GameFeatures feature) const
{
	return (feature >= GAME_FEATURE_FIRST && feature < GAME_FEATURE_LAST ? m_lockedFeatures.test(feature) : false);
}

bool ProtocolFeatureManager::resolveFeatureName(const std::string& featureName, std::vector<GameFeatures>& features) const
{
	features.clear();
	if(featureName == "GameThingMarks")
	{
		features.emplace_back(GAME_FEATURE_ITEM_MARK);
		features.emplace_back(GAME_FEATURE_CREATURE_MARK);
		return true;
	}

	for(const FeatureAlias& alias : featureAliases())
	{
		if(featureName == alias.name)
		{
			features.emplace_back(alias.feature);
			return true;
		}
	}
	return false;
}

bool ProtocolFeatureManager::enableFeatureByName(const std::string& featureName)
{
	FeatureSetting setting;
	setting.name = featureName;
	setting.enabled = true;
	Uint32 enabledCount = 0;
	Uint32 disabledCount = 0;
	return applyFeatureSetting(setting, enabledCount, disabledCount);
}

bool ProtocolFeatureManager::disableFeatureByName(const std::string& featureName)
{
	FeatureSetting setting;
	setting.name = featureName;
	setting.enabled = false;
	Uint32 enabledCount = 0;
	Uint32 disabledCount = 0;
	return applyFeatureSetting(setting, enabledCount, disabledCount);
}

bool ProtocolFeatureManager::hasFeatureByName(const std::string& featureName) const
{
	std::vector<GameFeatures> features;
	if(!resolveFeatureName(featureName, features))
		return false;

	for(GameFeatures feature : features)
	{
		if(!g_game.hasGameFeature(feature))
			return false;
	}
	return true;
}

void ProtocolFeatureManager::log(const char* format, ...) const
{
	std::array<char, 2048> message;
	va_list args;
	va_start(args, format);
	SDL_vsnprintf(message.data(), message.size(), format, args);
	va_end(args);
	UTIL_protocolDebugLog("features", "%s", message.data());
}
