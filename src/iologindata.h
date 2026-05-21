/**
 * The Forgotten Server - a free and open-source MMORPG server emulator
 * Copyright (C) 2019  Mark Samman <mark.samman@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef FS_IOLOGINDATA_H_28B0440BEC594654AC0F4E1A5E42B2EF
#define FS_IOLOGINDATA_H_28B0440BEC594654AC0F4E1A5E42B2EF

#include "account.h"
#include "player.h"
#include "database.h"

using ItemBlockList = std::list<std::pair<int32_t, Item*>>;
struct Proxy
{
	Proxy(std::string host, uint16_t port, uint16_t priority)
		: host(std::move(host)), port(port), priority(priority) {}

	std::string host;
	uint16_t port;
	uint16_t priority;
};

using ProxyList = std::vector<Proxy>;


class IOLoginData
{
	public:
		static Account loadAccount(uint32_t accno);

		static bool loginserverAuthentication(const std::string& name, const std::string& password, Account& account);
		static uint32_t gameworldAuthentication(const std::string& accountName, const std::string& password, std::string& characterName);
		static uint32_t getAccountIdByPlayerName(const std::string& playerName);
		static uint32_t getAccountIdByPlayerId(uint32_t playerId);

		static AccountType_t getAccountType(uint32_t accountId);
		static void setAccountType(uint32_t accountId, AccountType_t accountType);
		static void updateOnlineStatus(uint32_t guid, bool login);
		static bool preloadPlayer(Player* player, const std::string& name);

		static bool loadPlayerById(Player* player, uint32_t id);
		static bool loadPlayerByName(Player* player, const std::string& name);
		static bool loadPlayer(Player* player, DBResult_ptr result);
		static bool savePlayer(Player* player);
		static uint32_t getGuidByName(const std::string& name);
		static bool getGuidByNameEx(uint32_t& guid, bool& specialVip, std::string& name);
		static std::string getNameByGuid(uint32_t guid);
		static bool formatPlayerName(std::string& name);
		static void increaseBankBalance(uint32_t guid, uint64_t bankBalance);
		static bool hasBiddedOnHouse(uint32_t guid);

		static std::forward_list<VIPEntry> getVIPEntries(uint32_t accountId);
		static void addVIPEntry(uint32_t accountId, uint32_t guid);
		static void removeVIPEntry(uint32_t accountId, uint32_t guid);

		static void updatePremiumTime(uint32_t accountId, time_t endTime);
		static bool createAccount(const std::string& name, const std::string& password, uint32_t& accountId);
		static bool setPassword(uint32_t accountId, const std::string& newPassword);
		static bool setRecoveryKey(uint32_t accountId, const std::string& recoveryKey);
		static bool createPlayer(uint32_t accountId, const std::string& name, uint16_t vocationId, PlayerSex_t sex);
		static bool deletePlayer(uint32_t playerId);
		static std::vector<std::string> getPlayersByAccountId(uint32_t accountId);
		static bool playerNameExists(const std::string& name);
		static bool accountNameExists(const std::string& name);
		static const ProxyList getProxies();
		static bool addRewardItems(uint32_t playerId, const ItemBlockList& itemList, DBInsert& query_insert, PropWriteStream& propWriteStream);

	private:
		using ItemMap = std::map<uint32_t, std::pair<Item*, uint32_t>>;

		static void loadItems(ItemMap& itemMap, DBResult_ptr result);
		static bool saveItems(const Player* player, const ItemBlockList& itemList, DBInsert& query_insert, PropWriteStream& propWriteStream);
};

#endif
