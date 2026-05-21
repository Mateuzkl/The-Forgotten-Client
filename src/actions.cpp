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

#include "otpch.h"

#include "actions.h"
#include "bed.h"
#include "configmanager.h"
#include "container.h"
#include "game.h"
#include "pugicast.h"
#include "spells.h"
#include "rewardchest.h"
#include <fmt/format.h>

extern Game g_game;
extern Spells* g_spells;
extern Actions* g_actions;
extern ConfigManager g_config;

Actions::Actions() :
	scriptInterface("Action Interface")
{
	scriptInterface.initState();
}

Actions::~Actions()
{
	clear(false);
}

void Actions::clearMap(ActionUseMap& map, bool fromLua)
{
	for (auto it = map.begin(); it != map.end(); ) {
		if (fromLua == it->second.fromLua) {
			it = map.erase(it);
		} else {
			++it;
		}
	}
}

void Actions::clear(bool fromLua)
{
	clearMap(useItemMap, fromLua);
	clearMap(uniqueItemMap, fromLua);
	clearMap(actionItemMap, fromLua);

	reInitState(fromLua);
}

LuaScriptInterface& Actions::getScriptInterface()
{
	return scriptInterface;
}

std::string Actions::getScriptBaseName() const
{
	return "actions";
}

Event_ptr Actions::getEvent(const std::string& nodeName)
{
	if (strcasecmp(nodeName.c_str(), "action") != 0) {
		return nullptr;
	}
	return Event_ptr(new Action(&scriptInterface));
}

bool Actions::registerEvent(Event_ptr event, const pugi::xml_node& node)
{
	Action_ptr action{static_cast<Action*>(event.release())}; //event is guaranteed to be an Action

	pugi::xml_attribute attr;
	if ((attr = node.attribute("itemid"))) {
		std::vector<int32_t> idList = vectorAtoi(explodeString(attr.as_string(), ";"));
		bool success = true;

		for (const auto& id : idList) {
			auto result = useItemMap.emplace(id, std::move(*action));
			if (!result.second) {
				std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with id: " << id << std::endl;
				success = false;
			}
		}

		return success;
	} else if ((attr = node.attribute("fromid"))) {
		pugi::xml_attribute toIdAttribute = node.attribute("toid");
		if (!toIdAttribute) {
			std::cout << "[Warning - Actions::registerEvent] Missing toid in fromid: " << attr.as_string() << std::endl;
			return false;
		}

		uint16_t fromId = pugi::cast<uint16_t>(attr.value());
		uint16_t iterId = fromId;
		uint16_t toId = pugi::cast<uint16_t>(toIdAttribute.value());

		auto result = useItemMap.emplace(iterId, *action);
		if (!result.second) {
			std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with id: " << iterId << " in fromid: " << fromId << ", toid: " << toId << std::endl;
		}

		bool success = result.second;
		while (++iterId <= toId) {
			result = useItemMap.emplace(iterId, *action);
			if (!result.second) {
				std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with id: " << iterId << " in fromid: " << fromId << ", toid: " << toId << std::endl;
				continue;
			}
			success = true;
		}
		return success;
	} else if ((attr = node.attribute("uniqueid"))) {
		std::vector<int32_t> uidList = vectorAtoi(explodeString(attr.as_string(), ";"));
		bool success = true;

		for (const auto& uid : uidList) {
			auto result = uniqueItemMap.emplace(uid, std::move(*action));
			if (!result.second) {
				std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with uniqueid: " << uid << std::endl;
				success = false;
			}
		}

		return success;
	} else if ((attr = node.attribute("fromuid"))) {
		pugi::xml_attribute toUidAttribute = node.attribute("touid");
		if (!toUidAttribute) {
			std::cout << "[Warning - Actions::registerEvent] Missing touid in fromuid: " << attr.as_string() << std::endl;
			return false;
		}

		uint16_t fromUid = pugi::cast<uint16_t>(attr.value());
		uint16_t iterUid = fromUid;
		uint16_t toUid = pugi::cast<uint16_t>(toUidAttribute.value());

		auto result = uniqueItemMap.emplace(iterUid, *action);
		if (!result.second) {
			std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with unique id: " << iterUid << " in fromuid: " << fromUid << ", touid: " << toUid << std::endl;
		}

		bool success = result.second;
		while (++iterUid <= toUid) {
			result = uniqueItemMap.emplace(iterUid, *action);
			if (!result.second) {
				std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with unique id: " << iterUid << " in fromuid: " << fromUid << ", touid: " << toUid << std::endl;
				continue;
			}
			success = true;
		}
		return success;
	} else if ((attr = node.attribute("actionid"))) {
		std::vector<int32_t> aidList = vectorAtoi(explodeString(attr.as_string(), ";"));
		bool success = true;

		for (const auto& aid : aidList) {
			auto result = actionItemMap.emplace(aid, std::move(*action));
			if (!result.second) {
				std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with actionid: " << aid << std::endl;
				success = false;
			}
		}

		return success;
	} else if ((attr = node.attribute("fromaid"))) {
		pugi::xml_attribute toAidAttribute = node.attribute("toaid");
		if (!toAidAttribute) {
			std::cout << "[Warning - Actions::registerEvent] Missing toaid in fromaid: " << attr.as_string() << std::endl;
			return false;
		}

		uint16_t fromAid = pugi::cast<uint16_t>(attr.value());
		uint16_t iterAid = fromAid;
		uint16_t toAid = pugi::cast<uint16_t>(toAidAttribute.value());

		auto result = actionItemMap.emplace(iterAid, *action);
		if (!result.second) {
			std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with action id: " << iterAid << " in fromaid: " << fromAid << ", toaid: " << toAid << std::endl;
		}

		bool success = result.second;
		while (++iterAid <= toAid) {
			result = actionItemMap.emplace(iterAid, *action);
			if (!result.second) {
				std::cout << "[Warning - Actions::registerEvent] Duplicate registered item with action id: " << iterAid << " in fromaid: " << fromAid << ", toaid: " << toAid << std::endl;
				continue;
			}
			success = true;
		}
		return success;
	}
	return false;
}

bool Actions::registerLuaEvent(Action* event)
{
	Action_ptr action{ event };
	if (!action->getItemIdRange().empty()) {
		const auto& range = action->getItemIdRange();
		for (auto id : range) {
			auto result = useItemMap.emplace(id, *action);
			if (!result.second) {
				std::cout << "[Warning - Actions::registerLuaEvent] Duplicate registered item with id: " << id << " in range from id: " << range.front() << ", to id: " << range.back() << std::endl;
			}
		}
		return true;
	} else if (!action->getUniqueIdRange().empty()) {
		const auto& range = action->getUniqueIdRange();
		for (auto id : range) {
			auto result = uniqueItemMap.emplace(id, *action);
			if (!result.second) {
				std::cout << "[Warning - Actions::registerLuaEvent] Duplicate registered item with uid: " << id << " in range from uid: " << range.front() << ", to uid: " << range.back() << std::endl;
			}
		}
		return true;
	} else if (!action->getActionIdRange().empty()) {
		const auto& range = action->getActionIdRange();
		for (auto id : range) {
			auto result = actionItemMap.emplace(id, *action);
			if (!result.second) {
				std::cout << "[Warning - Actions::registerLuaEvent] Duplicate registered item with aid: " << id << " in range from aid: " << range.front() << ", to aid: " << range.back() << std::endl;
			}
		}
		return true;
	}

	std::cout << "[Warning - Actions::registerLuaEvent] There is no id / aid / uid set for this event" << std::endl;
	return false;
}

ReturnValue Actions::canUse(const Player* player, const Position& pos)
{
	if (pos.x != 0xFFFF) {
		const Position& playerPos = player->getPosition();
		if (playerPos.z != pos.z) {
			return playerPos.z > pos.z ? RETURNVALUE_FIRSTGOUPSTAIRS : RETURNVALUE_FIRSTGODOWNSTAIRS;
		}

		if (!Position::areInRange<1, 1>(playerPos, pos)) {
			return RETURNVALUE_TOOFARAWAY;
		}
	}
	return RETURNVALUE_NOERROR;
}

ReturnValue Actions::canUse(const Player* player, const Position& pos, const Item* item)
{
	Action* action = getAction(item);
	if (action) {
		return action->canExecuteAction(player, pos);
	}
	return RETURNVALUE_NOERROR;
}

ReturnValue Actions::canUseFar(const Creature* creature, const Position& toPos, bool checkLineOfSight, bool checkFloor)
{
	if (toPos.x == 0xFFFF) {
		return RETURNVALUE_NOERROR;
	}

	const Position& creaturePos = creature->getPosition();
	if (checkFloor && creaturePos.z != toPos.z) {
		return creaturePos.z > toPos.z ? RETURNVALUE_FIRSTGOUPSTAIRS : RETURNVALUE_FIRSTGODOWNSTAIRS;
	}

	if (!Position::areInRange<Map::maxClientViewportX - 1, Map::maxClientViewportY - 1>(toPos, creaturePos)) {
		return RETURNVALUE_TOOFARAWAY;
	}

	if (checkLineOfSight && !g_game.canThrowObjectTo(creaturePos, toPos, checkLineOfSight, checkFloor)) {
		return RETURNVALUE_CANNOTTHROW;
	}

	return RETURNVALUE_NOERROR;
}

Action* Actions::getAction(const Item* item)
{
	if (item->hasAttribute(ITEM_ATTRIBUTE_UNIQUEID)) {
		auto it = uniqueItemMap.find(item->getUniqueId());
		if (it != uniqueItemMap.end()) {
			return &it->second;
		}
	}

	if (item->hasAttribute(ITEM_ATTRIBUTE_ACTIONID)) {
		auto it = actionItemMap.find(item->getActionId());
		if (it != actionItemMap.end()) {
			return &it->second;
		}
	}

	auto it = useItemMap.find(item->getID());
	if (it != useItemMap.end()) {
		return &it->second;
	}

	//rune items
	return g_spells->getRuneSpell(item->getID());
}

ReturnValue Actions::internalUseItem(Player* player, const Position& pos, uint8_t index, Item* item, bool isHotkey)
{
	if (Door* door = item->getDoor()) {
		if (!door->canUse(player)) {
			return RETURNVALUE_NOTPOSSIBLE;
		}
	}

	Action* action = getAction(item);
	if (action) {
		if (action->isScripted()) {
			if (action->executeUse(player, item, pos, nullptr, pos, isHotkey)) {
				return RETURNVALUE_NOERROR;
			}

			if (item->isRemoved()) {
				return RETURNVALUE_CANNOTUSETHISOBJECT;
			}
		} else if (action->function && action->function(player, item, pos, nullptr, pos, isHotkey)) {
			return RETURNVALUE_NOERROR;
		}
	}

	if (BedItem* bed = item->getBed()) {
		if (!bed->canUse(player)) {
			if (!bed->getHouse()) {
				return RETURNVALUE_YOUCANNOTUSETHISBED;
			}
		}

		if (bed->trySleep(player)) {
			player->setBedItem(bed);
			bed->sleep(player);
		}

		return RETURNVALUE_NOERROR;
	}

	if (Container* container = item->getContainer()) {
		Container* openContainer;

		//depot container
		if (DepotLocker* depot = container->getDepotLocker()) {
			DepotLocker* myDepotLocker = player->getDepotLocker(depot->getDepotId());
			myDepotLocker->setParent(depot->getParent()->getTile());
			openContainer = myDepotLocker;
			player->setLastDepotId(depot->getDepotId());
		} else {
			openContainer = container;
		}

		uint32_t corpseOwner = container->getCorpseOwner();
		if (container->isRewardCorpse()) {
			RewardChest& myRewardChest = player->getRewardChest();

			for (Item* subItem : container->getItemList()) {
				if (subItem->getID() == ITEM_REWARD_CONTAINER) {
					int64_t rewardDate = subItem->getIntAttr(ITEM_ATTRIBUTE_DATE);

					bool foundMatch = false;
					for (Item* rewardItem : myRewardChest.getItemList()) {
						if (rewardItem->getID() == ITEM_REWARD_CONTAINER && rewardItem->getIntAttr(ITEM_ATTRIBUTE_DATE) == rewardDate) {
							foundMatch = true;
							break;
						}
					}

					if (!foundMatch) {
						return RETURNVALUE_NOTPOSSIBLE;
					}
				}
			}
		}
		else if (corpseOwner != 0 && !player->canOpenCorpse(corpseOwner)) {
			return RETURNVALUE_YOUARENOTTHEOWNER;
		}
		else {
			// ✅ Only run autoloot if this is really a corpse the player can open
			if (player->canOpenCorpse(corpseOwner) && !player->autoLootList.empty()) {

				// Premium-only check
				if (!player->isPremium()) {
					// Not premium → skip autoloot; allow manual loot
				}
				else {
					// Premium → run autoloot
					if (player->getCapacity() > 100 * 100) { // Minimum 100 cap required

						// Require a backpack (avoid items "disappearing")
						Container* backpack = nullptr;
						if (Item* bpItem = player->getInventoryItem(CONST_SLOT_BACKPACK)) {
							backpack = bpItem->getContainer();
						}
						if (!backpack) {
							player->sendTextMessage(MESSAGE_EVENT_ADVANCE,
								"You need a container in the backpack slot in order to autoloot!");
							return RETURNVALUE_NOERROR;
						}

						// ----- Helpers -----
						// Is this item a coin? (and how much gp to deposit)
						auto coinValueOf = [](Item* it) -> uint32_t {
							if (!it) return 0;
							const uint16_t id = it->getID();
							const uint32_t cnt = it->getItemCount();
							switch (id) {
							case ITEM_GOLD_COIN:     return cnt;          // 1 gp each
							case ITEM_PLATINUM_COIN: return cnt * 100u;   // 100 gp each
							case ITEM_CRYSTAL_COIN:  return cnt * 10000u; // 10,000 gp each
							default: return 0;
							}
							};

						// Find a destination container anywhere inside player's backpack tree
						auto findDestinationFor = [&](Item* moving) -> Container* {
							std::function<Container* (Container*)> dfs = [&](Container* c) -> Container* {
								if (!c) return nullptr;

								// 1) Prefer stacking
								if (moving->isStackable()) {
									for (Item* it : c->getItemList()) {
										if (!it || !it->isStackable()) continue;
										if (it->getID() != moving->getID()) continue;
										if (it->getItemCount() < 100) {
											return c;
										}
									}
								}

								// 2) Free slot
								if (c->size() < c->capacity()) {
									return c;
								}

								// 3) Recurse inner containers
								for (Item* it : c->getItemList()) {
									if (!it) continue;
									if (Container* inner = it->getContainer()) {
										if (Container* res = dfs(inner)) {
											return res;
										}
									}
								}
								return nullptr;
								};
							return dfs(backpack);
							};

						// ===== New: accumulate looted stacks for a single final message =====
						// itemId -> total count
						std::unordered_map<uint16_t, uint64_t> totalById;
						// itemId -> display name (singular; we’ll print like "24x gold coin")
						std::unordered_map<uint16_t, std::string> nameById;

						auto addLootCount = [&](Item* item, uint32_t count) {
							if (!item || count == 0) return;
							const uint16_t id = item->getID();
							totalById[id] += count;
							// cache a readable name (avoid repeated getName() calls)
							if (nameById.find(id) == nameById.end()) {
								nameById[id] = item->getName();
							}
							};

						// Recursive autoloot (no per-item messages; only accumulate)
						std::function<void(Container*)> autoLootContainer = [&](Container* cont) {
							// Copy items first to avoid iterator invalidation
							std::vector<Item*> items;
							items.reserve(cont->getItemList().size());
							for (Item* it : cont->getItemList()) {
								items.push_back(it);
							}

							for (Item* item : items) {
								if (!item) continue;

								if (Container* inner = item->getContainer()) {
									if (player->getItemFromAutoLoot(item->getID())) {
										if (Container* dest = findDestinationFor(item)) {
											// Try to move the whole inner bag as a single item
											if (g_game.internalMoveItem(cont, dest, INDEX_WHEREEVER,
												item, item->getItemCount(), nullptr) == RETURNVALUE_NOERROR) {
												// Count the container as 1 item moved
												addLootCount(item, 1);
												continue; // moved entire bag
											}
										}
										// couldn’t move container → loot its contents
										autoLootContainer(inner);
										continue;
									}
									else {
										autoLootContainer(inner);
										continue;
									}
								}

								// Regular item OR coins
								if (player->getItemFromAutoLoot(item->getID())) {
									// Special case: coins → deposit to bank, but show as "24x gold coin"
									if (uint32_t coinGp = coinValueOf(item)) {
										// Update bank
										uint64_t bank = player->getBankBalance();
										bank += static_cast<uint64_t>(coinGp);
										player->setBankBalance(bank);

										// Accumulate visible stack text (no gp)
										addLootCount(item, item->getItemCount());

										// Remove the coins from the corpse
										g_game.internalRemoveItem(item, item->getItemCount());
										continue;
									}

									// Otherwise, handle as normal item into backpack tree
									if (Container* dest = findDestinationFor(item)) {
										if (g_game.internalMoveItem(cont, dest, INDEX_WHEREEVER,
											item, item->getItemCount(), nullptr) == RETURNVALUE_NOERROR) {
											addLootCount(item, item->getItemCount());
										}
										// If it didn't move, we simply don't add it (and we also don't spam a message)
									}
								}
							}
							};

						// ✅ Run autoloot and then send one summary message
						autoLootContainer(container);

						if (!totalById.empty()) {
							std::ostringstream sum;
							sum << "Auto-looted: ";

							bool first = true;
							for (const auto& kv : totalById) {
								const uint16_t id = kv.first;
								const uint64_t cnt = kv.second;
								const std::string& nm = nameById[id];

								if (!first) sum << ", ";
								first = false;
								// Print like "24x gold coin"
								sum << cnt << "x " << nm;
							}
							sum << ".";

							player->sendTextMessage(MESSAGE_EVENT_ADVANCE, sum.str());
						}

					}
					else {
						player->sendTextMessage(MESSAGE_EVENT_ADVANCE,
							"Sorry, you don't have enough capacity to use auto loot.");
					}
				}
			}
		}









		// Reward chest
		if (RewardChest* rewardchest = container->getRewardChest()) {
			RewardChest& myRewardChest = player->getRewardChest();
			myRewardChest.setParent(rewardchest->getParent()->getTile());

			if (myRewardChest.getItemList().empty()) {
				return RETURNVALUE_REWARDCHESTEMPTY;
			}

			for (Item* rewardItem : myRewardChest.getItemList()) {
				if (rewardItem->getID() == ITEM_REWARD_CONTAINER) {
					Container* rewardContainer = rewardItem->getContainer();
					if (rewardContainer) {
						rewardContainer->setParent(&myRewardChest);
					}
				}
			}
			openContainer = &myRewardChest;
		}
		else if (item->getID() == ITEM_REWARD_CONTAINER) {
			RewardChest& myRewardChest = player->getRewardChest();
			int64_t rewardDate = item->getIntAttr(ITEM_ATTRIBUTE_DATE);

			for (Item* rewardItem : myRewardChest.getItemList()) {
				if (rewardItem->getID() == ITEM_REWARD_CONTAINER && rewardItem->getIntAttr(ITEM_ATTRIBUTE_DATE) == rewardDate && rewardItem->getIntAttr(ITEM_ATTRIBUTE_REWARDID) == item->getIntAttr(ITEM_ATTRIBUTE_REWARDID)) {
					Container* rewardContainer = rewardItem->getContainer();
					if (rewardContainer) {
						rewardContainer->setParent(container->getRealParent());
						openContainer = rewardContainer;
					}
					break;
				}
			}
		}

		//open/close container
		int32_t oldContainerId = player->getContainerID(openContainer);
		if (oldContainerId == -1) {
			player->addContainer(index, openContainer);
			player->onSendContainer(openContainer);
		} else {
			player->onCloseContainer(openContainer);
			player->closeContainer(oldContainerId);
		}

		return RETURNVALUE_NOERROR;
	}

	const ItemType& it = Item::items[item->getID()];
	if (it.canReadText) {
		if (it.canWriteText) {
			player->setWriteItem(item, it.maxTextLen);
			player->sendTextWindow(item, it.maxTextLen, true);
		} else {
			player->setWriteItem(nullptr);
			player->sendTextWindow(item, 0, false);
		}

		return RETURNVALUE_NOERROR;
	}

	return RETURNVALUE_CANNOTUSETHISOBJECT;
}


static void showUseHotkeyMessage(Player* player, const Item* item, uint32_t count)
{
	const ItemType& it = Item::items[item->getID()];
	if (!it.showCount) {
		player->sendTextMessage(MESSAGE_INFO_DESCR, fmt::format("Using one of {:s}...",  item->getName()));
	} else if (count == 1) {
		player->sendTextMessage(MESSAGE_INFO_DESCR, fmt::format("Using the last {:s}...",  item->getName()));
	} else {
		player->sendTextMessage(MESSAGE_INFO_DESCR, fmt::format("Using one of {:d} {:s}...", count, item->getPluralName()));
	}
}

bool Actions::useItem(Player* player, const Position& pos, uint8_t index, Item* item, bool isHotkey)
{
	int32_t cooldown = g_config.getNumber(ConfigManager::ACTIONS_DELAY_INTERVAL);
	player->setNextAction(OTSYS_TIME() + cooldown);
	player->sendUseItemCooldown(cooldown);

	if (isHotkey) {
		uint16_t subType = item->getSubType();
		showUseHotkeyMessage(player, item, player->getItemTypeCount(item->getID(), subType != item->getItemCount() ? subType : -1));
	}

	if (g_config.getBoolean(ConfigManager::ONLY_INVITED_CAN_MOVE_HOUSE_ITEMS)) {
		if (HouseTile* houseTile = dynamic_cast<HouseTile*>(item->getTile())) {
			House* house = houseTile->getHouse();
			if (house && !house->isInvited(player)) {
				player->sendCancelMessage(RETURNVALUE_PLAYERISNOTINVITED);
				return false;
			}
		}
	}

	ReturnValue ret = internalUseItem(player, pos, index, item, isHotkey);
	if (ret == RETURNVALUE_YOUCANNOTUSETHISBED) {
		g_game.internalCreatureSay(player, TALKTYPE_MONSTER_SAY, getReturnMessage(ret), false);
		return false;
	}

	if (ret != RETURNVALUE_NOERROR) {
		player->sendCancelMessage(ret);
		return false;
	}

	return true;
}

bool Actions::useItemEx(Player* player, const Position& fromPos, const Position& toPos,
                        uint8_t toStackPos, Item* item, bool isHotkey, Creature* creature/* = nullptr*/)
{
	int32_t cooldown = g_config.getNumber(ConfigManager::EX_ACTIONS_DELAY_INTERVAL);
	player->setNextAction(OTSYS_TIME() + cooldown);
	player->sendUseItemCooldown(cooldown);

	Action* action = getAction(item);
	if (!action) {
		player->sendCancelMessage(RETURNVALUE_CANNOTUSETHISOBJECT);
		return false;
	}

	ReturnValue ret = action->canExecuteAction(player, toPos);
	if (ret != RETURNVALUE_NOERROR) {
		player->sendCancelMessage(ret);
		return false;
	}

	if (isHotkey) {
		uint16_t subType = item->getSubType();
		showUseHotkeyMessage(player, item, player->getItemTypeCount(item->getID(), subType != item->getItemCount() ? subType : -1));
	}

	if (action->executeUse(player, item, fromPos, action->getTarget(player, creature, toPos, toStackPos), toPos, isHotkey)) {
		return true;
	}

	if (!action->hasOwnErrorHandler()) {
		player->sendCancelMessage(RETURNVALUE_CANNOTUSETHISOBJECT);
	}

	return false;
}

Action::Action(LuaScriptInterface* interface) :
	Event(interface), function(nullptr), allowFarUse(false), checkFloor(true), checkLineOfSight(true) {}

bool Action::configureEvent(const pugi::xml_node& node)
{
	pugi::xml_attribute allowFarUseAttr = node.attribute("allowfaruse");
	if (allowFarUseAttr) {
		allowFarUse = allowFarUseAttr.as_bool();
	}

	pugi::xml_attribute blockWallsAttr = node.attribute("blockwalls");
	if (blockWallsAttr) {
		checkLineOfSight = blockWallsAttr.as_bool();
	}

	pugi::xml_attribute checkFloorAttr = node.attribute("checkfloor");
	if (checkFloorAttr) {
		checkFloor = checkFloorAttr.as_bool();
	}

	return true;
}

bool Action::loadFunction(const pugi::xml_attribute& attr, bool)
{
	std::cout << "[Warning - Action::loadFunction] Function \"" << attr.as_string() << "\" does not exist." << std::endl;
	return false;
}

std::string Action::getScriptEventName() const
{
	return "onUse";
}

ReturnValue Action::canExecuteAction(const Player* player, const Position& toPos)
{
	if (allowFarUse) {
		return g_actions->canUseFar(player, toPos, checkLineOfSight, checkFloor);
	}
	return g_actions->canUse(player, toPos);
}

Thing* Action::getTarget(Player* player, Creature* targetCreature, const Position& toPosition, uint8_t toStackPos) const
{
	if (targetCreature) {
		return targetCreature;
	}
	return g_game.internalGetThing(player, toPosition, toStackPos, 0, STACKPOS_USETARGET);
}

bool Action::executeUse(Player* player, Item* item, const Position& fromPosition, Thing* target, const Position& toPosition, bool isHotkey)
{
	//onUse(player, item, fromPosition, target, toPosition, isHotkey)
	if (!scriptInterface->reserveScriptEnv()) {
		std::cout << "[Error - Action::executeUse] Call stack overflow" << std::endl;
		return false;
	}

	ScriptEnvironment* env = scriptInterface->getScriptEnv();
	env->setScriptId(scriptId, scriptInterface);

	lua_State* L = scriptInterface->getLuaState();

	scriptInterface->pushFunction(scriptId);

	LuaScriptInterface::pushUserdata<Player>(L, player);
	LuaScriptInterface::setMetatable(L, -1, "Player");

	LuaScriptInterface::pushThing(L, item);
	LuaScriptInterface::pushPosition(L, fromPosition);

	LuaScriptInterface::pushThing(L, target);
	LuaScriptInterface::pushPosition(L, toPosition);

	LuaScriptInterface::pushBoolean(L, isHotkey);
	return scriptInterface->callFunction(6);
}
