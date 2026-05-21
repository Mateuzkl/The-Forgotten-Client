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
#include "../game.h"
#include "../creature.h"
#include "../json/json.h"
#include "../GUI_Elements/GUI_Panel.h"
#include "../GUI_Elements/GUI_PanelWindow.h"
#include "../GUI_Elements/GUI_StaticImage.h"
#include "../GUI_Elements/GUI_Label.h"
#include "../GUI_Elements/GUI_Icon.h"
#include "../GUI_Elements/GUI_Container.h"
#include "../GUI_Elements/GUI_Window.h"
#include "../GUI_Elements/GUI_Button.h"
#include "../GUI_Elements/GUI_ListBox.h"
#include "../GUI_Elements/GUI_Log.h"
#include "../GUI_Elements/GUI_MultiTextBox.h"
#include "../GUI_Elements/GUI_Separator.h"
#include "../GUI_Elements/GUI_ScrollBar.h"
#include "TasksWindow.h"

#include <algorithm>
#include <map>
#include <memory>
#include <vector>

#define TASKS_TITLE "Tasks"
#define TASKS_TRACKER_TITLE "Tasks Tracker"
#define TASKS_OPCODE 110

#define TASKS_TRACKER_MAXIMINI_EVENTID 3400
#define TASKS_TRACKER_CLOSE_EVENTID 3401
#define TASKS_TRACKER_REFRESH_EVENTID 3402
#define TASKS_TRACKER_RESIZE_WIDTH_EVENTID 3403
#define TASKS_TRACKER_RESIZE_HEIGHT_EVENTID 3404
#define TASKS_TRACKER_EXIT_WINDOW_EVENTID 3405
#define TASKS_TRACKER_CONTAINER_EVENTID 3406

#define TASKS_WINDOW_WIDTH 560
#define TASKS_WINDOW_HEIGHT 392
#define TASKS_WINDOW_CLOSE_EVENTID 3450
#define TASKS_WINDOW_REFRESH_EVENTID 3451
#define TASKS_WINDOW_START_EVENTID 3452
#define TASKS_WINDOW_CANCEL_EVENTID 3453
#define TASKS_WINDOW_LIST_EVENTID 3454
#define TASKS_WINDOW_KILLS_SCROLL_EVENTID 3455

#define TASKS_WINDOW_POINTS_EVENTID 3460
#define TASKS_WINDOW_STATUS_EVENTID 3461
#define TASKS_WINDOW_DETAILS_EVENTID 3462
#define TASKS_WINDOW_KILLS_VALUE_EVENTID 3463
#define TASKS_WINDOW_KILLS_MIN_EVENTID 3464
#define TASKS_WINDOW_KILLS_MAX_EVENTID 3465

enum TaskRewardType
{
	TASK_REWARD_POINTS = 1,
	TASK_REWARD_EXPERIENCE = 2,
	TASK_REWARD_GOLD = 3,
	TASK_REWARD_ITEM = 4,
	TASK_REWARD_STORAGE = 5,
	TASK_REWARD_TELEPORT = 6
};

struct TaskOutfit
{
	Uint16 type = 0;
	Uint16 auxType = 0;
	Uint16 mount = 0;
	Uint8 head = 114;
	Uint8 body = 114;
	Uint8 legs = 114;
	Uint8 feet = 114;
	Uint8 addons = 0;
};

struct TaskReward
{
	Uint8 type = 0;
	Uint32 value = 0;
	Uint32 amount = 0;
	Uint16 itemId = 0;
	std::string itemName;
	std::string desc;
};

struct TaskDefinition
{
	std::string name;
	Uint16 level = 0;
	Uint16 tokenCost = 1;
	std::vector<std::string> monsters;
	std::vector<TaskOutfit> outfits;
	std::vector<TaskReward> rewards;
};

struct TaskActiveState
{
	Uint16 kills = 0;
	Uint16 required = 0;
	Uint8 slot = 0;
	bool pending = false;
	Creature* creature = NULL;
};

extern Engine g_engine;
extern Game g_game;
extern GUI_Log g_logger;

bool g_haveTaskTrackerOpen = false;
bool g_recreateTaskTrackerWindow = false;

static std::vector<TaskDefinition> g_taskDefinitions;
static std::map<Uint16, TaskActiveState> g_taskActiveStates;
static std::vector<Uint16> g_taskActiveOrder;
static std::string g_taskChunkBuffer;
static Uint32 g_taskPoints = 0;
static Uint16 g_taskKillMin = 0;
static Uint16 g_taskKillMax = 0;
static Uint16 g_taskKillBonus = 0;
static Uint16 g_taskPointIncrease = 0;
static Uint16 g_taskExpIncrease = 0;
static Uint16 g_taskGoldIncrease = 0;
static Uint16 g_taskItemIncrease = 0;
static Uint16 g_taskSelectedKills = 0;
static Sint32 g_taskSelectedIndex = -1;
static Uint32 g_nextTaskPreviewId = 0x7E000000;

static void tasks_Events(Uint32 event, Sint32 status);

static std::string getJsonString(JSON_VALUE* object, const std::string& key)
{
	if(!object)
		return std::string();

	JSON_VALUE* value = object->find(key);
	if(!value || !value->IsString())
		return std::string();
	return value->AsString();
}

static Uint32 getJsonNumber(JSON_VALUE* object, const std::string& key, Uint32 fallback = 0)
{
	if(!object)
		return fallback;

	JSON_VALUE* value = object->find(key);
	if(!value || !value->IsNumber())
		return fallback;
	return SDL_static_cast(Uint32, value->AsNumber());
}

static bool getJsonBool(JSON_VALUE* object, const std::string& key, bool fallback = false)
{
	if(!object)
		return fallback;

	JSON_VALUE* value = object->find(key);
	if(!value || !value->IsBool())
		return fallback;
	return value->AsBool();
}

static std::string formatNumber(Uint32 value)
{
	std::string digits = std::to_string(value);
	for(Sint32 i = SDL_static_cast(Sint32, digits.length()) - 3; i > 0; i -= 3)
		digits.insert(SDL_static_cast(size_t, i), 1, ',');
	return digits;
}

static TaskDefinition* getTaskDefinition(Uint16 taskId)
{
	if(taskId == 0 || SDL_static_cast(size_t, taskId) > g_taskDefinitions.size())
		return NULL;
	return &g_taskDefinitions[SDL_static_cast(size_t, taskId - 1)];
}

static Uint16 getSelectedTaskId()
{
	if(g_taskSelectedIndex < 0 || SDL_static_cast(size_t, g_taskSelectedIndex) >= g_taskDefinitions.size())
		return 0;
	return SDL_static_cast(Uint16, g_taskSelectedIndex + 1);
}

static TaskDefinition* getSelectedTaskDefinition()
{
	return getTaskDefinition(getSelectedTaskId());
}

static TaskActiveState* getTaskActiveState(Uint16 taskId)
{
	std::map<Uint16, TaskActiveState>::iterator it = g_taskActiveStates.find(taskId);
	if(it == g_taskActiveStates.end())
		return NULL;
	return &it->second;
}

static Uint8 calculateTaskProgress(Uint16 kills, Uint16 required)
{
	if(required == 0)
		return 0;
	return SDL_static_cast(Uint8, UTIL_min<Uint32>(100, SDL_static_cast(Uint32, (kills * 100) / required)));
}

static Uint32 calculateTaskBonusBlocks(Uint16 kills)
{
	if(g_taskKillBonus == 0 || kills <= g_taskKillBonus)
		return 0;
	return SDL_static_cast(Uint32, ((kills - g_taskKillBonus) + (g_taskKillBonus / 2)) / g_taskKillBonus);
}

static TaskActiveState& ensureTaskActiveState(Uint16 taskId)
{
	TaskActiveState& state = g_taskActiveStates[taskId];
	if(!state.creature)
	{
		state.creature = new Creature();
		state.creature->setId(g_nextTaskPreviewId++);
		state.creature->setType(CREATURETYPE_MONSTER);
		state.creature->setVisible(true);
		state.creature->setShowStatus(true);
		state.creature->setHealth(0);
		state.creature->setOutfit(0, 0, 114, 114, 114, 114, 0, 0);
	}
	return state;
}

static void refreshTaskPreview(Uint16 taskId)
{
	TaskActiveState* state = getTaskActiveState(taskId);
	if(!state || !state->creature)
		return;

	TaskDefinition* definition = getTaskDefinition(taskId);
	if(definition)
	{
		state->creature->setName(definition->name);
		if(!definition->outfits.empty())
		{
			const TaskOutfit& outfit = definition->outfits.front();
			state->creature->setOutfit(outfit.type, outfit.auxType, outfit.head, outfit.body, outfit.legs, outfit.feet, outfit.addons, outfit.mount);
		}
		else
			state->creature->setOutfit(0, 0, 114, 114, 114, 114, 0, 0);
	}
	else
	{
		state->creature->setName("Task " + std::to_string(taskId));
		state->creature->setOutfit(0, 0, 114, 114, 114, 114, 0, 0);
	}

	state->creature->setVisible(true);
	state->creature->setHealth(state->pending ? 100 : calculateTaskProgress(state->kills, state->required));
}

static void clampTaskSelections()
{
	if(g_taskDefinitions.empty())
	{
		g_taskSelectedIndex = -1;
		g_taskSelectedKills = g_taskKillMin;
		return;
	}

	if(g_taskSelectedIndex < 0 || SDL_static_cast(size_t, g_taskSelectedIndex) >= g_taskDefinitions.size())
		g_taskSelectedIndex = 0;

	if(g_taskKillMax < g_taskKillMin)
		g_taskKillMax = g_taskKillMin;

	if(g_taskSelectedKills == 0)
		g_taskSelectedKills = g_taskKillMin;

	g_taskSelectedKills = UTIL_max<Uint16>(g_taskKillMin, UTIL_min<Uint16>(g_taskKillMax, g_taskSelectedKills));
}

static void requestTasksRefresh()
{
	g_game.sendExtendedOpcode(TASKS_OPCODE, "{\"action\":\"refresh\"}");
}

static void sendTaskStart(Uint16 taskId)
{
	JSON_OBJECT dataObject;
	dataObject["taskId"] = new JSON_VALUE(SDL_static_cast(Uint32, taskId));
	dataObject["kills"] = new JSON_VALUE(SDL_static_cast(Uint32, g_taskSelectedKills));

	JSON_OBJECT payloadObject;
	payloadObject["action"] = new JSON_VALUE("start");
	payloadObject["data"] = new JSON_VALUE(dataObject);
	std::unique_ptr<JSON_VALUE> payload(new JSON_VALUE(payloadObject));
	g_game.sendExtendedOpcode(TASKS_OPCODE, JSON_VALUE::encode(payload.get()));
}

static void sendTaskCancel(Uint16 taskId)
{
	JSON_OBJECT payloadObject;
	payloadObject["action"] = new JSON_VALUE("cancel");
	payloadObject["data"] = new JSON_VALUE(SDL_static_cast(Uint32, taskId));
	std::unique_ptr<JSON_VALUE> payload(new JSON_VALUE(payloadObject));
	g_game.sendExtendedOpcode(TASKS_OPCODE, JSON_VALUE::encode(payload.get()));
}

static TaskOutfit parseTaskOutfit(JSON_VALUE* outfitObject)
{
	TaskOutfit outfit;
	if(!outfitObject || !outfitObject->IsObject())
		return outfit;

	outfit.type = SDL_static_cast(Uint16, getJsonNumber(outfitObject, "type"));
	outfit.auxType = SDL_static_cast(Uint16, getJsonNumber(outfitObject, "auxType"));
	outfit.mount = SDL_static_cast(Uint16, getJsonNumber(outfitObject, "mount"));
	outfit.head = SDL_static_cast(Uint8, getJsonNumber(outfitObject, "head", 114));
	outfit.body = SDL_static_cast(Uint8, getJsonNumber(outfitObject, "body", 114));
	outfit.legs = SDL_static_cast(Uint8, getJsonNumber(outfitObject, "legs", 114));
	outfit.feet = SDL_static_cast(Uint8, getJsonNumber(outfitObject, "feet", 114));
	outfit.addons = SDL_static_cast(Uint8, getJsonNumber(outfitObject, "addons"));
	return outfit;
}

static void syncTaskConfig(JSON_VALUE* data)
{
	if(!data || !data->IsObject())
		return;

	JSON_VALUE* kills = data->find("kills");
	g_taskKillMin = SDL_static_cast(Uint16, getJsonNumber(kills, "Min", g_taskKillMin));
	g_taskKillMax = SDL_static_cast(Uint16, getJsonNumber(kills, "Max", g_taskKillMax));
	g_taskKillBonus = SDL_static_cast(Uint16, getJsonNumber(data, "bonus", g_taskKillBonus));
	g_taskPointIncrease = SDL_static_cast(Uint16, getJsonNumber(data, "points", g_taskPointIncrease));
	g_taskExpIncrease = SDL_static_cast(Uint16, getJsonNumber(data, "exp", g_taskExpIncrease));
	g_taskGoldIncrease = SDL_static_cast(Uint16, getJsonNumber(data, "gold", g_taskGoldIncrease));
	g_taskItemIncrease = SDL_static_cast(Uint16, getJsonNumber(data, "item", g_taskItemIncrease));
	clampTaskSelections();
}

static void syncTaskDefinitions(JSON_VALUE* data)
{
	g_taskDefinitions.clear();
	if(data && data->IsArray())
	{
		g_taskDefinitions.reserve(data->size());
		for(size_t i = 0, end = data->size(); i < end; ++i)
		{
			JSON_VALUE* entry = data->find(i);
			if(!entry || !entry->IsObject())
				continue;

			TaskDefinition definition;
			definition.name = getJsonString(entry, "name");
			definition.level = SDL_static_cast(Uint16, getJsonNumber(entry, "lvl"));
			definition.tokenCost = SDL_static_cast(Uint16, getJsonNumber(entry, "tokenCost", 1));

			JSON_VALUE* monsters = entry->find("mobs");
			if(monsters && monsters->IsArray())
			{
				for(size_t m = 0, mend = monsters->size(); m < mend; ++m)
				{
					JSON_VALUE* monster = monsters->find(m);
					if(monster && monster->IsString())
						definition.monsters.push_back(monster->AsString());
				}
			}

			JSON_VALUE* outfits = entry->find("outfits");
			if(outfits && outfits->IsArray())
			{
				for(size_t o = 0, oend = outfits->size(); o < oend; ++o)
				{
					JSON_VALUE* outfit = outfits->find(o);
					definition.outfits.push_back(parseTaskOutfit(outfit));
				}
			}

			JSON_VALUE* rewards = entry->find("rewards");
			if(rewards && rewards->IsArray())
			{
				for(size_t r = 0, rend = rewards->size(); r < rend; ++r)
				{
					JSON_VALUE* rewardObject = rewards->find(r);
					if(!rewardObject || !rewardObject->IsObject())
						continue;

					TaskReward reward;
					reward.type = SDL_static_cast(Uint8, getJsonNumber(rewardObject, "type"));
					reward.value = getJsonNumber(rewardObject, "value");
					reward.amount = getJsonNumber(rewardObject, "amount");
					reward.itemId = SDL_static_cast(Uint16, getJsonNumber(rewardObject, "itemId"));
					reward.itemName = getJsonString(rewardObject, "itemName");
					reward.desc = getJsonString(rewardObject, "desc");
					definition.rewards.push_back(reward);
				}
			}

			g_taskDefinitions.push_back(definition);
		}
	}

	for(std::vector<Uint16>::iterator it = g_taskActiveOrder.begin(), end = g_taskActiveOrder.end(); it != end; ++it)
		refreshTaskPreview((*it));

	clampTaskSelections();
}

static void syncActiveTasks(JSON_VALUE* data)
{
	std::vector<Uint16> nextOrder;
	if(data && data->IsArray())
	{
		nextOrder.reserve(data->size());
		for(size_t i = 0, end = data->size(); i < end; ++i)
		{
			JSON_VALUE* entry = data->find(i);
			if(!entry || !entry->IsObject())
				continue;

			Uint16 taskId = SDL_static_cast(Uint16, getJsonNumber(entry, "taskId"));
			if(taskId == 0)
				continue;

			TaskActiveState& state = ensureTaskActiveState(taskId);
			state.kills = SDL_static_cast(Uint16, getJsonNumber(entry, "kills"));
			state.required = SDL_static_cast(Uint16, getJsonNumber(entry, "required"));
			state.slot = SDL_static_cast(Uint8, getJsonNumber(entry, "slot"));
			state.pending = getJsonBool(entry, "pending", false);
			refreshTaskPreview(taskId);
			nextOrder.push_back(taskId);
		}
	}

	for(std::map<Uint16, TaskActiveState>::iterator it = g_taskActiveStates.begin(); it != g_taskActiveStates.end();)
	{
		if(std::find(nextOrder.begin(), nextOrder.end(), it->first) == nextOrder.end())
		{
			delete it->second.creature;
			it = g_taskActiveStates.erase(it);
		}
		else
			++it;
	}

	g_taskActiveOrder.swap(nextOrder);
	g_recreateTaskTrackerWindow = true;
}

static void applyTaskUpdate(JSON_VALUE* data)
{
	if(!data || !data->IsObject())
		return;

	Uint16 taskId = SDL_static_cast(Uint16, getJsonNumber(data, "taskId"));
	if(taskId == 0)
		return;

	Uint8 status = SDL_static_cast(Uint8, getJsonNumber(data, "status"));
	if(status == 1)
	{
		TaskActiveState& state = ensureTaskActiveState(taskId);
		state.kills = SDL_static_cast(Uint16, getJsonNumber(data, "kills"));
		state.required = SDL_static_cast(Uint16, getJsonNumber(data, "required"));
		state.pending = false;
		refreshTaskPreview(taskId);
		if(std::find(g_taskActiveOrder.begin(), g_taskActiveOrder.end(), taskId) == g_taskActiveOrder.end())
			g_taskActiveOrder.push_back(taskId);
	}
	else
	{
		std::vector<Uint16>::iterator order = std::find(g_taskActiveOrder.begin(), g_taskActiveOrder.end(), taskId);
		if(order != g_taskActiveOrder.end())
			g_taskActiveOrder.erase(order);

		std::map<Uint16, TaskActiveState>::iterator it = g_taskActiveStates.find(taskId);
		if(it != g_taskActiveStates.end())
		{
			delete it->second.creature;
			g_taskActiveStates.erase(it);
		}
	}

	g_recreateTaskTrackerWindow = true;
}

static std::string getTaskStateLine(Uint16 taskId)
{
	TaskActiveState* active = getTaskActiveState(taskId);
	if(!active)
		return "Status: Not active";
	if(active->pending)
		return "Status: Reward pending";
	return "Status: Active (" + std::to_string(active->kills) + "/" + std::to_string(active->required) + ")";
}

static std::string buildTaskDetailText(const TaskDefinition& task, Uint16 taskId)
{
	std::string text;
	text.reserve(512);
	text.append("Recommended Level: ").append(std::to_string(task.level)).append("\n");
	text.append("Task Tokens Required: ").append(std::to_string(task.tokenCost)).append("\n");
	text.append(getTaskStateLine(taskId)).append("\n");
	text.append("Selected Kills: ").append(std::to_string(g_taskSelectedKills)).append("\n\n");
	text.append("Monsters:\n");
	if(task.monsters.empty())
		text.append("- No monsters received.\n");
	else
	{
		for(std::vector<std::string>::const_iterator it = task.monsters.begin(), end = task.monsters.end(); it != end; ++it)
			text.append("- ").append(*it).append("\n");
	}

	text.append("\nRewards:\n");
	if(task.rewards.empty())
		text.append("- No rewards received.\n");
	else
	{
		for(std::vector<TaskReward>::const_iterator it = task.rewards.begin(), end = task.rewards.end(); it != end; ++it)
		{
			switch(it->type)
			{
				case TASK_REWARD_POINTS: text.append("- Task Points: ").append(std::to_string(it->value)).append("\n"); break;
				case TASK_REWARD_EXPERIENCE: text.append("- Experience: ").append(formatNumber(it->value)).append("\n"); break;
				case TASK_REWARD_GOLD: text.append("- Gold: ").append(formatNumber(it->value)).append("\n"); break;
				case TASK_REWARD_ITEM: text.append("- ").append(std::to_string(it->amount)).append("x ").append(it->itemName.empty() ? "Item" : it->itemName).append("\n"); break;
				case TASK_REWARD_STORAGE: text.append("- ").append(it->desc.empty() ? "Storage reward" : it->desc).append("\n"); break;
				case TASK_REWARD_TELEPORT: text.append("- Teleport: ").append(it->desc.empty() ? "Destination" : it->desc).append("\n"); break;
				default: text.append("- Unknown reward\n"); break;
			}
		}
	}

	Uint32 bonusBlocks = calculateTaskBonusBlocks(g_taskSelectedKills);
	if(bonusBlocks > 0)
	{
		bool wroteBonus = false;
		text.append("\nBonus At Current Kills:\n");
		for(std::vector<TaskReward>::const_iterator it = task.rewards.begin(), end = task.rewards.end(); it != end; ++it)
		{
			switch(it->type)
			{
				case TASK_REWARD_POINTS:
					text.append("- +").append(std::to_string(bonusBlocks * g_taskPointIncrease)).append("% Task Points\n");
					wroteBonus = true;
				break;
				case TASK_REWARD_EXPERIENCE:
					text.append("- +").append(std::to_string(bonusBlocks * g_taskExpIncrease)).append("% Experience\n");
					wroteBonus = true;
				break;
				case TASK_REWARD_GOLD:
					text.append("- +").append(std::to_string(bonusBlocks * g_taskGoldIncrease)).append("% Gold\n");
					wroteBonus = true;
				break;
				case TASK_REWARD_ITEM:
					text.append("- +").append(std::to_string(bonusBlocks * g_taskItemIncrease)).append("% Item reward\n");
					wroteBonus = true;
				break;
				default: break;
			}
		}
		if(!wroteBonus)
			text.append("- No scalable bonuses for this task.\n");
	}

	return text;
}

static void closeTasksWindow()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_TASKS);
	if(pWindow)
		g_engine.removeWindow(pWindow);
}

static void refreshTasksWindowDetails(GUI_Window* pWindow)
{
	if(!pWindow || pWindow->getInternalID() != GUI_WINDOW_TASKS)
		return;

	clampTaskSelections();
	TaskDefinition* task = getSelectedTaskDefinition();
	Uint16 taskId = getSelectedTaskId();

	GUI_Label* pointsLabel = SDL_static_cast(GUI_Label*, pWindow->getChild(TASKS_WINDOW_POINTS_EVENTID));
	if(pointsLabel)
		pointsLabel->setName("Current Task Points: " + formatNumber(g_taskPoints));

	GUI_Label* statusLabel = SDL_static_cast(GUI_Label*, pWindow->getChild(TASKS_WINDOW_STATUS_EVENTID));
	if(statusLabel)
		statusLabel->setName(task ? getTaskStateLine(taskId) : "Status: Select a task");

	GUI_MultiTextBox* detailsBox = SDL_static_cast(GUI_MultiTextBox*, pWindow->getChild(TASKS_WINDOW_DETAILS_EVENTID));
	if(detailsBox)
	{
		if(task)
			detailsBox->setText(buildTaskDetailText(*task, taskId));
		else
			detailsBox->setText("No task data received yet.\n\nSpeak with the NPC or click Refresh to request the task list from the server.");
	}

	GUI_Label* killsValue = SDL_static_cast(GUI_Label*, pWindow->getChild(TASKS_WINDOW_KILLS_VALUE_EVENTID));
	if(killsValue)
		killsValue->setName(std::to_string(g_taskSelectedKills));

	GUI_Label* killsMin = SDL_static_cast(GUI_Label*, pWindow->getChild(TASKS_WINDOW_KILLS_MIN_EVENTID));
	if(killsMin)
		killsMin->setName(std::to_string(g_taskKillMin));

	GUI_Label* killsMax = SDL_static_cast(GUI_Label*, pWindow->getChild(TASKS_WINDOW_KILLS_MAX_EVENTID));
	if(killsMax)
		killsMax->setName(std::to_string(g_taskKillMax));
}

static void buildTasksWindow(GUI_Window* pWindow)
{
	pWindow->clearChilds();
	clampTaskSelections();

	GUI_Label* newLabel = new GUI_Label(iRect(18, 32, 0, 0), "Current Task Points: " + formatNumber(g_taskPoints), TASKS_WINDOW_POINTS_EVENTID, 96, 224, 96);
	pWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(18, 48, 0, 0), "Selected task information", TASKS_WINDOW_STATUS_EVENTID, 175, 175, 175);
	pWindow->addChild(newLabel);

	GUI_ListBox* newListBox = new GUI_ListBox(iRect(18, 76, 180, 250), TASKS_WINDOW_LIST_EVENTID);
	for(std::vector<TaskDefinition>::iterator it = g_taskDefinitions.begin(), end = g_taskDefinitions.end(); it != end; ++it)
		newListBox->add(it->name + " (Lv " + std::to_string(it->level) + ")");
	if(g_taskSelectedIndex >= 0)
		newListBox->setSelect(g_taskSelectedIndex);
	newListBox->setEventCallback(&tasks_Events, TASKS_WINDOW_LIST_EVENTID);
	newListBox->startEvents();
	pWindow->addChild(newListBox);

	GUI_MultiTextBox* newTextBox = new GUI_MultiTextBox(iRect(214, 76, 328, 250), false, "", TASKS_WINDOW_DETAILS_EVENTID, 175, 175, 175);
	newTextBox->startEvents();
	pWindow->addChild(newTextBox);

	newLabel = new GUI_Label(iRect(18, 334, 0, 0), "Kills:", 0, 175, 175, 175);
	pWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(52, 334, 0, 0), std::to_string(g_taskKillMin), TASKS_WINDOW_KILLS_MIN_EVENTID, 143, 143, 143);
	pWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(200, 334, 0, 0), std::to_string(g_taskSelectedKills), TASKS_WINDOW_KILLS_VALUE_EVENTID, 214, 214, 214);
	pWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(353, 334, 0, 0), std::to_string(g_taskKillMax), TASKS_WINDOW_KILLS_MAX_EVENTID, 143, 143, 143);
	pWindow->addChild(newLabel);

	Sint32 killRange = SDL_static_cast(Sint32, (g_taskKillMax >= g_taskKillMin ? g_taskKillMax - g_taskKillMin : 0));
	GUI_HScrollBar* newScrollBar = new GUI_HScrollBar(iRect(82, 332, 264, 12), killRange, SDL_static_cast(Sint32, g_taskSelectedKills - g_taskKillMin), TASKS_WINDOW_KILLS_SCROLL_EVENTID);
	newScrollBar->setBarEventCallback(&tasks_Events, TASKS_WINDOW_KILLS_SCROLL_EVENTID);
	newScrollBar->startEvents();
	pWindow->addChild(newScrollBar);

	GUI_Separator* newSeparator = new GUI_Separator(iRect(13, TASKS_WINDOW_HEIGHT - 40, TASKS_WINDOW_WIDTH - 26, 2));
	pWindow->addChild(newSeparator);

	GUI_Button* newButton = new GUI_Button(iRect(18, TASKS_WINDOW_HEIGHT - 30, GUI_UI_BUTTON_58PX_GRAY_UP_W, GUI_UI_BUTTON_58PX_GRAY_UP_H), "Refresh", 0, "Fetch the latest task data");
	newButton->setButtonEventCallback(&tasks_Events, TASKS_WINDOW_REFRESH_EVENTID);
	newButton->startEvents();
	pWindow->addChild(newButton);

	newButton = new GUI_Button(iRect(TASKS_WINDOW_WIDTH - 202, TASKS_WINDOW_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Start", CLIENT_GUI_ENTER_TRIGGER, "Start the selected task");
	newButton->setTextColor(96, 224, 96);
	newButton->setButtonEventCallback(&tasks_Events, TASKS_WINDOW_START_EVENTID);
	newButton->startEvents();
	pWindow->addChild(newButton);

	newButton = new GUI_Button(iRect(TASKS_WINDOW_WIDTH - 149, TASKS_WINDOW_HEIGHT - 30, GUI_UI_BUTTON_58PX_GRAY_UP_W, GUI_UI_BUTTON_58PX_GRAY_UP_H), "Abandon", 0, "Abandon the selected task");
	newButton->setButtonEventCallback(&tasks_Events, TASKS_WINDOW_CANCEL_EVENTID);
	newButton->startEvents();
	pWindow->addChild(newButton);

	newButton = new GUI_Button(iRect(TASKS_WINDOW_WIDTH - 84, TASKS_WINDOW_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Close", CLIENT_GUI_ESCAPE_TRIGGER, "Close the tasks window");
	newButton->setButtonEventCallback(&tasks_Events, TASKS_WINDOW_CLOSE_EVENTID);
	newButton->startEvents();
	pWindow->addChild(newButton);

	refreshTasksWindowDetails(pWindow);
}

static void showTasksWindow()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_TASKS);
	if(!pWindow)
	{
		pWindow = new GUI_Window(iRect(0, 0, TASKS_WINDOW_WIDTH, TASKS_WINDOW_HEIGHT), TASKS_TITLE, GUI_WINDOW_TASKS);
		buildTasksWindow(pWindow);
		g_engine.addWindow(pWindow, true);
	}
	else
		buildTasksWindow(pWindow);
}

static void tasks_Events(Uint32 event, Sint32 status)
{
	switch(event)
	{
		case TASKS_TRACKER_MAXIMINI_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_TASK_TRACKER);
			if(pPanel)
			{
				GUI_Panel* parent = pPanel->getParent();
				if(!parent)
					break;

				iRect& pRect = pPanel->getOriginalRect();
				if(pRect.y2 > 19)
				{
					pPanel->setCachedHeight(pRect.y2);
					pPanel->setSize(pRect.x2, 19);
					parent->checkPanels();

					GUI_Container* pContainer = SDL_static_cast(GUI_Container*, pPanel->getChild(TASKS_TRACKER_CONTAINER_EVENTID));
					if(pContainer)
						pContainer->makeInvisible();

					GUI_Icon* pIcon = SDL_static_cast(GUI_Icon*, pPanel->getChild(TASKS_TRACKER_MAXIMINI_EVENTID));
					if(pIcon)
						pIcon->setData(GUI_UI_IMAGE, GUI_UI_ICON_MAXIMIZE_WINDOW_UP_X, GUI_UI_ICON_MAXIMIZE_WINDOW_UP_Y, GUI_UI_ICON_MAXIMIZE_WINDOW_DOWN_X, GUI_UI_ICON_MAXIMIZE_WINDOW_DOWN_Y);
				}
				else
				{
					Sint32 cachedHeight = pPanel->getCachedHeight();
					parent->tryFreeHeight(cachedHeight - pRect.y2);
					pPanel->setSize(pRect.x2, cachedHeight);
					parent->checkPanels();

					GUI_Container* pContainer = SDL_static_cast(GUI_Container*, pPanel->getChild(TASKS_TRACKER_CONTAINER_EVENTID));
					if(pContainer)
						pContainer->makeVisible();

					GUI_Icon* pIcon = SDL_static_cast(GUI_Icon*, pPanel->getChild(TASKS_TRACKER_MAXIMINI_EVENTID));
					if(pIcon)
						pIcon->setData(GUI_UI_IMAGE, GUI_UI_ICON_MINIMIZE_WINDOW_UP_X, GUI_UI_ICON_MINIMIZE_WINDOW_UP_Y, GUI_UI_ICON_MINIMIZE_WINDOW_DOWN_X, GUI_UI_ICON_MINIMIZE_WINDOW_DOWN_Y);
				}
			}
		}
		break;
		case TASKS_TRACKER_CLOSE_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_TASK_TRACKER);
			if(pPanel)
			{
				GUI_Panel* parent = pPanel->getParent();
				if(parent)
					parent->removePanel(pPanel);
			}
		}
		break;
		case TASKS_TRACKER_REFRESH_EVENTID:
			requestTasksRefresh();
		break;
		case TASKS_TRACKER_RESIZE_HEIGHT_EVENTID:
		{
			GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_TASK_TRACKER);
			if(pPanel)
			{
				GUI_Container* pContainer = SDL_static_cast(GUI_Container*, pPanel->getChild(TASKS_TRACKER_CONTAINER_EVENTID));
				if(pContainer)
				{
					iRect cRect = pContainer->getRect();
					cRect.y2 = status - 19;
					pContainer->setRect(cRect);
				}
			}
		}
		break;
		case TASKS_TRACKER_EXIT_WINDOW_EVENTID:
			g_engine.setContentWindowHeight(GUI_PANEL_WINDOW_TASK_TRACKER, status - 19);
			g_haveTaskTrackerOpen = false;
		break;
		case TASKS_WINDOW_CLOSE_EVENTID:
			closeTasksWindow();
		break;
		case TASKS_WINDOW_REFRESH_EVENTID:
			requestTasksRefresh();
		break;
		case TASKS_WINDOW_START_EVENTID:
		{
			Uint16 taskId = getSelectedTaskId();
			if(taskId == 0)
			{
				UTIL_messageBox(TASKS_TITLE, "Select a task first.");
				break;
			}

			TaskActiveState* active = getTaskActiveState(taskId);
			if(active)
			{
				if(active->pending)
					UTIL_messageBox(TASKS_TITLE, "This task already has a reward pending.");
				else
					UTIL_messageBox(TASKS_TITLE, "This task is already active.");
				break;
			}

			sendTaskStart(taskId);
		}
		break;
		case TASKS_WINDOW_CANCEL_EVENTID:
		{
			Uint16 taskId = getSelectedTaskId();
			if(taskId == 0)
			{
				UTIL_messageBox(TASKS_TITLE, "Select a task first.");
				break;
			}

			TaskActiveState* active = getTaskActiveState(taskId);
			if(!active)
			{
				UTIL_messageBox(TASKS_TITLE, "This task is not active.");
				break;
			}
			if(active->pending)
			{
				UTIL_messageBox(TASKS_TITLE, "You cannot abandon a task with a pending reward.");
				break;
			}

			sendTaskCancel(taskId);
		}
		break;
		case TASKS_WINDOW_LIST_EVENTID:
		{
			if(status < 0)
				status = ~(status);

			if(status >= 0 && SDL_static_cast(size_t, status) < g_taskDefinitions.size())
				g_taskSelectedIndex = status;
			else
				g_taskSelectedIndex = -1;

			GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_TASKS);
			if(pWindow)
				refreshTasksWindowDetails(pWindow);
		}
		break;
		case TASKS_WINDOW_KILLS_SCROLL_EVENTID:
		{
			g_taskSelectedKills = SDL_static_cast(Uint16, UTIL_min<Sint32>(g_taskKillMax, UTIL_max<Sint32>(g_taskKillMin, g_taskKillMin + status)));
			GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_TASKS);
			if(pWindow)
				refreshTasksWindowDetails(pWindow);
		}
		break;
		default: break;
	}
}

void UTIL_recreateTaskTrackerWindow(GUI_Container* container)
{
	if(!container)
	{
		GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_TASK_TRACKER);
		if(pPanel)
			container = SDL_static_cast(GUI_Container*, pPanel->getChild(TASKS_TRACKER_CONTAINER_EVENTID));
		if(!container)
			return;
	}

	container->clearChilds(false);
	Sint32 posY = -9;
	for(std::vector<Uint16>::iterator it = g_taskActiveOrder.begin(), end = g_taskActiveOrder.end(); it != end; ++it)
	{
		GUI_TaskCreature* newTaskCreature = new GUI_TaskCreature(iRect(1, posY, 155, 23), (*it));
		newTaskCreature->startEvents();
		container->addChild(newTaskCreature, false);
		posY += 23;
	}
	container->validateScrollBar();
}

void UTIL_toggleTaskTrackerWindow()
{
	GUI_PanelWindow* pPanel = g_engine.getPanel(GUI_PANEL_WINDOW_TASK_TRACKER);
	if(pPanel)
	{
		g_engine.removePanelWindow(pPanel);
		return;
	}

	Sint32 savedHeight = g_engine.getContentWindowHeight(GUI_PANEL_WINDOW_TASK_TRACKER);
	if(savedHeight < 38)
		savedHeight = 119;
	GUI_PanelWindow* newWindow = new GUI_PanelWindow(iRect(0, 0, GAME_PANEL_FIXED_WIDTH - 4, savedHeight + 19), true, GUI_PANEL_WINDOW_TASK_TRACKER, true);
	newWindow->setEventCallback(&tasks_Events, TASKS_TRACKER_RESIZE_WIDTH_EVENTID, TASKS_TRACKER_RESIZE_HEIGHT_EVENTID, TASKS_TRACKER_EXIT_WINDOW_EVENTID);

	GUI_TaskChecker* newTaskChecker = new GUI_TaskChecker(iRect(19, 2, 0, 0));
	newWindow->addChild(newTaskChecker);
	GUI_StaticImage* newImage = new GUI_StaticImage(iRect(2, 0, GUI_UI_ICON_BATTLELIST_W, GUI_UI_ICON_BATTLELIST_H), GUI_UI_IMAGE, GUI_UI_ICON_BATTLELIST_X, GUI_UI_ICON_BATTLELIST_Y);
	newWindow->addChild(newImage);
	GUI_Icon* newIcon = new GUI_Icon(iRect(147, 0, GUI_UI_ICON_MINIMIZE_WINDOW_UP_W, GUI_UI_ICON_MINIMIZE_WINDOW_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_MINIMIZE_WINDOW_UP_X, GUI_UI_ICON_MINIMIZE_WINDOW_UP_Y, GUI_UI_ICON_MINIMIZE_WINDOW_DOWN_X, GUI_UI_ICON_MINIMIZE_WINDOW_DOWN_Y, TASKS_TRACKER_MAXIMINI_EVENTID, "Maximise or minimise window");
	newIcon->setButtonEventCallback(&tasks_Events, TASKS_TRACKER_MAXIMINI_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(159, 0, GUI_UI_ICON_CLOSE_WINDOW_UP_W, GUI_UI_ICON_CLOSE_WINDOW_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_CLOSE_WINDOW_UP_X, GUI_UI_ICON_CLOSE_WINDOW_UP_Y, GUI_UI_ICON_CLOSE_WINDOW_DOWN_X, GUI_UI_ICON_CLOSE_WINDOW_DOWN_Y, 0, "Close this window");
	newIcon->setButtonEventCallback(&tasks_Events, TASKS_TRACKER_CLOSE_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	newIcon = new GUI_Icon(iRect(131, 0, GUI_UI_ICON_CONFIGURE_WINDOW_UP_W, GUI_UI_ICON_CONFIGURE_WINDOW_UP_H), GUI_UI_IMAGE, GUI_UI_ICON_CONFIGURE_WINDOW_UP_X, GUI_UI_ICON_CONFIGURE_WINDOW_UP_Y, GUI_UI_ICON_CONFIGURE_WINDOW_DOWN_X, GUI_UI_ICON_CONFIGURE_WINDOW_DOWN_Y, TASKS_TRACKER_REFRESH_EVENTID, "Refresh active tasks");
	newIcon->setButtonEventCallback(&tasks_Events, TASKS_TRACKER_REFRESH_EVENTID);
	newIcon->startEvents();
	newWindow->addChild(newIcon);
	GUI_DynamicLabel* newDynamicLabel = new GUI_DynamicLabel(iRect(19, 2, 100, 14), TASKS_TRACKER_TITLE, 0, 144, 144, 144);
	newWindow->addChild(newDynamicLabel);
	GUI_Container* newContainer = new GUI_Container(iRect(2, 13, 168, savedHeight), newWindow, TASKS_TRACKER_CONTAINER_EVENTID);
	UTIL_recreateTaskTrackerWindow(newContainer);
	newContainer->startEvents();
	newWindow->addChild(newContainer);

	Sint32 preferredPanel = g_engine.getContentWindowParent(GUI_PANEL_WINDOW_TASK_TRACKER);
	bool added = g_engine.addToPanel(newWindow, preferredPanel);
	if(!added && preferredPanel != GUI_PANEL_RANDOM)
		added = g_engine.addToPanel(newWindow, GUI_PANEL_RANDOM);

	if(added)
	{
		g_recreateTaskTrackerWindow = false;
		g_haveTaskTrackerOpen = true;
		requestTasksRefresh();
	}
	else
	{
		g_recreateTaskTrackerWindow = true;
		g_haveTaskTrackerOpen = false;
		delete newWindow;
	}
}

void UTIL_resetTaskEntries()
{
	closeTasksWindow();
	g_taskDefinitions.clear();
	g_taskActiveOrder.clear();
	g_taskChunkBuffer.clear();
	g_taskPoints = 0;
	g_taskKillMin = 0;
	g_taskKillMax = 0;
	g_taskKillBonus = 0;
	g_taskPointIncrease = 0;
	g_taskExpIncrease = 0;
	g_taskGoldIncrease = 0;
	g_taskItemIncrease = 0;
	g_taskSelectedKills = 0;
	g_taskSelectedIndex = -1;
	for(std::map<Uint16, TaskActiveState>::iterator it = g_taskActiveStates.begin(), end = g_taskActiveStates.end(); it != end; ++it)
		delete it->second.creature;
	g_taskActiveStates.clear();
	g_haveTaskTrackerOpen = false;
	g_recreateTaskTrackerWindow = true;
	g_nextTaskPreviewId = 0x7E000000;
}

static bool decodeTaskPayload(const std::string& payload, std::unique_ptr<JSON_VALUE>& decodedObject)
{
	std::string jsonPayload(payload);
	if(!payload.empty())
	{
		char packetType = payload[0];
		if(packetType == 'S' || packetType == 'P' || packetType == 'E')
		{
			if(packetType == 'S')
				g_taskChunkBuffer.clear();
			g_taskChunkBuffer.append(payload.substr(1));
			if(packetType != 'E')
				return false;
			jsonPayload = g_taskChunkBuffer;
		}
	}

	decodedObject.reset(JSON_VALUE::decode(jsonPayload.c_str()));
	if(!decodedObject || !decodedObject->IsObject())
	{
		g_logger.addLog(LOG_CATEGORY_WARNING, "[Tasks] Failed to decode extended opcode payload.");
		return false;
	}
	return true;
}

bool UTIL_handleTaskExtendedOpcode(Uint8 opcode, const std::string& payload)
{
	if(opcode != TASKS_OPCODE)
		return false;

	std::unique_ptr<JSON_VALUE> dataJson;
	if(!decodeTaskPayload(payload, dataJson))
		return true;

	JSON_VALUE* dataObject = dataJson.get();
	const std::string action = getJsonString(dataObject, "action");
	JSON_VALUE* data = dataObject->find("data");

	if(action == "config")
		syncTaskConfig(data);
	else if(action == "tasks")
		syncTaskDefinitions(data);
	else if(action == "active")
		syncActiveTasks(data);
	else if(action == "update")
		applyTaskUpdate(data);
	else if(action == "points")
	{
		if(data && data->IsNumber())
			g_taskPoints = SDL_static_cast(Uint32, data->AsNumber());
	}
	else if(action == "open")
	{
		showTasksWindow();
		requestTasksRefresh();
		return true;
	}
	else if(action == "close")
	{
		closeTasksWindow();
		return true;
	}

	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_TASKS);
	if(pWindow)
		buildTasksWindow(pWindow);
	return true;
}

GUI_TaskChecker::GUI_TaskChecker(iRect boxRect, Uint32 internalID)
{
	setRect(boxRect);
	m_internalID = internalID;
}

void GUI_TaskChecker::render()
{
	if(g_recreateTaskTrackerWindow)
	{
		UTIL_recreateTaskTrackerWindow(NULL);
		g_recreateTaskTrackerWindow = false;
	}
}

GUI_TaskCreature::GUI_TaskCreature(iRect boxRect, Uint16 taskId, Uint32 internalID) : m_taskId(taskId)
{
	setRect(boxRect);
	m_internalID = internalID;
}

void GUI_TaskCreature::render()
{
	TaskActiveState* state = getTaskActiveState(m_taskId);
	if(!state || !state->creature)
		return;

	state->creature->renderOnBattle(m_tRect.x1, m_tRect.y1, false);
	if(state->pending)
		g_engine.drawFont(CLIENT_FONT_NONOUTLINED, m_tRect.x1 + 152, m_tRect.y1 + 4, "Reward", 240, 208, 96, CLIENT_FONT_ALIGN_RIGHT);
	else
		g_engine.drawFont(CLIENT_FONT_NONOUTLINED, m_tRect.x1 + 152, m_tRect.y1 + 4, std::to_string(state->kills) + "/" + std::to_string(state->required), 96, 224, 96, CLIENT_FONT_ALIGN_RIGHT);
}
