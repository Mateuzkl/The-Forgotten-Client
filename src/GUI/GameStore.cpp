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
#include "../map.h"
#include "../thingManager.h"
#include "../json/json.h"
#include "../GUI_Elements/GUI_Window.h"
#include "../GUI_Elements/GUI_Button.h"
#include "../GUI_Elements/GUI_ListBox.h"
#include "../GUI_Elements/GUI_MultiTextBox.h"
#include "../GUI_Elements/GUI_Label.h"
#include "../GUI_Elements/GUI_Separator.h"
#include "itemUI.h"
#include "GameStore.h"

#include <map>
#include <memory>
#include <vector>

#define GAME_STORE_TITLE "Game Store"
#define GAME_STORE_OPCODE 102
#define GAME_STORE_WIDTH 640
#define GAME_STORE_HEIGHT 420
#define GAME_STORE_CONFIRM_WIDTH 290
#define GAME_STORE_CONFIRM_HEIGHT 145

#define GAME_STORE_CLOSE_EVENTID 4100
#define GAME_STORE_REFRESH_EVENTID 4101
#define GAME_STORE_PURCHASE_EVENTID 4102
#define GAME_STORE_CATEGORIES_EVENTID 4103
#define GAME_STORE_OFFERS_EVENTID 4104
#define GAME_STORE_CONFIRM_YES_EVENTID 4105
#define GAME_STORE_CONFIRM_NO_EVENTID 4106

#define GAME_STORE_CATEGORY_INFO_EVENTID 4110
#define GAME_STORE_POINTS_EVENTID 4111
#define GAME_STORE_GUILD_POINTS_EVENTID 4112
#define GAME_STORE_OFFER_TITLE_EVENTID 4113
#define GAME_STORE_OFFER_PRICE_EVENTID 4114
#define GAME_STORE_OFFER_TYPE_EVENTID 4115
#define GAME_STORE_DESCRIPTION_EVENTID 4116
#define GAME_STORE_CONFIRM_TEXT_EVENTID 4117

struct GameStoreCategory
{
	std::string title;
	std::string description;
};

struct GameStoreOutfitPreview
{
	Uint16 type = 0;
	Uint16 auxType = 0;
	Uint16 mount = 0;
	Uint8 head = 0;
	Uint8 body = 0;
	Uint8 legs = 0;
	Uint8 feet = 0;
	Uint8 addons = 0;
	bool valid = false;
};

struct GameStoreOffer
{
	std::string category;
	std::string type;
	std::string title;
	std::string description;
	std::string currency;
	Uint32 price = 0;
	Uint16 count = 0;
	Uint16 clientId = 0;
	GameStoreOutfitPreview previewOutfit;
};

extern Engine g_engine;
extern Game g_game;
extern ThingManager g_thingManager;
extern Uint32 g_frameTime;

bool g_haveGameStoreOpen = false;

static std::vector<GameStoreCategory> g_gameStoreCategories;
static std::map<std::string, std::vector<GameStoreOffer> > g_gameStoreOffers;
static Uint32 g_gameStorePoints = 0;
static Uint32 g_gameStoreGuildPoints = 0;
static Sint32 g_gameStoreSelectedCategory = -1;
static Sint32 g_gameStoreSelectedOffer = -1;
static bool g_gameStoreHasPendingPurchase = false;
static GameStoreOffer g_gameStorePendingPurchase;

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

static std::string getOfferPriceLabel(const GameStoreOffer& offer)
{
	return formatNumber(offer.price) + (offer.currency == "guildpoints" ? " GP" : " points");
}

static GameStoreCategory* getSelectedCategory()
{
	if(g_gameStoreSelectedCategory < 0 || SDL_static_cast(size_t, g_gameStoreSelectedCategory) >= g_gameStoreCategories.size())
		return NULL;
	return &g_gameStoreCategories[SDL_static_cast(size_t, g_gameStoreSelectedCategory)];
}

static std::vector<GameStoreOffer>* getSelectedOfferList()
{
	GameStoreCategory* category = getSelectedCategory();
	if(!category)
		return NULL;

	std::map<std::string, std::vector<GameStoreOffer> >::iterator it = g_gameStoreOffers.find(category->title);
	if(it == g_gameStoreOffers.end())
		return NULL;
	return &it->second;
}

static GameStoreOffer* getSelectedOffer()
{
	std::vector<GameStoreOffer>* offers = getSelectedOfferList();
	if(!offers || g_gameStoreSelectedOffer < 0 || SDL_static_cast(size_t, g_gameStoreSelectedOffer) >= offers->size())
		return NULL;
	return &(*offers)[SDL_static_cast(size_t, g_gameStoreSelectedOffer)];
}

static void ensureSelections()
{
	if(g_gameStoreCategories.empty())
	{
		g_gameStoreSelectedCategory = -1;
		g_gameStoreSelectedOffer = -1;
		return;
	}

	if(g_gameStoreSelectedCategory < 0 || SDL_static_cast(size_t, g_gameStoreSelectedCategory) >= g_gameStoreCategories.size())
		g_gameStoreSelectedCategory = 0;

	std::vector<GameStoreOffer>* offers = getSelectedOfferList();
	if(!offers || offers->empty())
	{
		g_gameStoreSelectedOffer = -1;
		return;
	}

	if(g_gameStoreSelectedOffer < 0 || SDL_static_cast(size_t, g_gameStoreSelectedOffer) >= offers->size())
		g_gameStoreSelectedOffer = 0;
}

static GameStoreOutfitPreview parseOutfitPreview(JSON_VALUE* outfitObject)
{
	GameStoreOutfitPreview preview;
	if(!outfitObject || !outfitObject->IsObject())
		return preview;

	preview.type = SDL_static_cast(Uint16, getJsonNumber(outfitObject, "type"));
	preview.auxType = SDL_static_cast(Uint16, getJsonNumber(outfitObject, "auxType"));
	preview.mount = SDL_static_cast(Uint16, getJsonNumber(outfitObject, "mount"));
	preview.head = SDL_static_cast(Uint8, getJsonNumber(outfitObject, "head"));
	preview.body = SDL_static_cast(Uint8, getJsonNumber(outfitObject, "body"));
	preview.legs = SDL_static_cast(Uint8, getJsonNumber(outfitObject, "legs"));
	preview.feet = SDL_static_cast(Uint8, getJsonNumber(outfitObject, "feet"));
	preview.addons = SDL_static_cast(Uint8, getJsonNumber(outfitObject, "addons"));
	preview.valid = (preview.type != 0 || preview.auxType != 0);
	return preview;
}

static GameStoreOutfitPreview getFallbackOutfitPreview(const GameStoreOffer& offer)
{
	GameStoreOutfitPreview preview = offer.previewOutfit;
	if(offer.type == "mount")
	{
		preview.type = 0;
		preview.auxType = 0;
		preview.head = 0;
		preview.body = 0;
		preview.legs = 0;
		preview.feet = 0;
		preview.addons = 0;
		preview.mount = (preview.mount != 0 ? preview.mount : offer.clientId);
		preview.valid = (preview.mount != 0);
		return preview;
	}

	if(preview.valid)
		return preview;

	if(offer.type == "outfit" && offer.clientId != 0)
	{
		preview.type = offer.clientId;
		preview.head = 114;
		preview.body = 114;
		preview.legs = 114;
		preview.feet = 114;
		preview.valid = true;
	}

	return preview;
}

static Uint8 getStorePreviewAnimation(ThingType* thingType)
{
	if(!thingType)
		return 0;

	if(thingType->hasFlag(ThingAttribute_AnimateAlways) && thingType->m_frameGroup[ThingFrameGroup_Idle].m_animCount > 0)
	{
		Sint32 ticks = (1000 / thingType->m_frameGroup[ThingFrameGroup_Idle].m_animCount);
		return UTIL_safeMod<Uint8>(SDL_static_cast(Uint8, (g_frameTime / ticks)), thingType->m_frameGroup[ThingFrameGroup_Idle].m_animCount);
	}

	return 0;
}

static void renderStorePreviewFloor()
{
	ThingType* ground = g_thingManager.getThingType(ThingCategory_Item, 429);
	if(!ground)
		return;

	auto& renderer = g_engine.getRender();
	Sint32 posY = -32;
	for(Uint8 y = 0; y < 5; ++y)
	{
		Sint32 posX = -32;
		for(Uint8 x = 0; x < 7; ++x)
		{
			Uint8 xPattern = UTIL_safeMod<Uint8>(x, ground->m_frameGroup[ThingFrameGroup_Default].m_patternX);
			Uint8 yPattern = UTIL_safeMod<Uint8>(y, ground->m_frameGroup[ThingFrameGroup_Default].m_patternY);
			Uint32 sprite = ground->getSprite(ThingFrameGroup_Default, 0, 0, 0, xPattern, yPattern, 0, 0);
			if(sprite != 0)
				renderer->drawSprite(sprite, posX, posY);
			posX += 32;
		}
		posY += 32;
	}
}

static void renderStorePreviewCreature(ThingType* thingType, Uint32 outfitColors, Uint8 outfitAddons, Uint8 zPattern, Uint8 animation, bool colorize)
{
	if(!thingType)
		return;

	auto& renderer = g_engine.getRender();
	Sint32 startX = 64 - thingType->m_displacement[0];
	Sint32 startY = 32 - thingType->m_displacement[1];
	Sint32 posY = startY;
	for(Uint8 cy = 0; cy < thingType->m_frameGroup[ThingFrameGroup_Idle].m_height; ++cy)
	{
		Sint32 posX = startX;
		for(Uint8 cx = 0; cx < thingType->m_frameGroup[ThingFrameGroup_Idle].m_width; ++cx)
		{
			if(colorize && thingType->m_frameGroup[ThingFrameGroup_Idle].m_layers > 1)
			{
				Uint32 sprite = thingType->getSprite(ThingFrameGroup_Idle, cx, cy, 0, DIRECTION_SOUTH, 0, zPattern, animation);
				Uint32 spriteMask = thingType->getSprite(ThingFrameGroup_Idle, cx, cy, 1, DIRECTION_SOUTH, 0, zPattern, animation);
				if(sprite != 0)
				{
					if(spriteMask != 0)
						renderer->drawSpriteMask(sprite, spriteMask, posX, posY, outfitColors);
						else
							renderer->drawSprite(sprite, posX, posY);
					}

				if(thingType->m_frameGroup[ThingFrameGroup_Idle].m_patternY > 1)
				{
					if(outfitAddons & 1)
					{
						sprite = thingType->getSprite(ThingFrameGroup_Idle, cx, cy, 0, DIRECTION_SOUTH, 1, zPattern, animation);
						spriteMask = thingType->getSprite(ThingFrameGroup_Idle, cx, cy, 1, DIRECTION_SOUTH, 1, zPattern, animation);
						if(sprite != 0)
						{
							if(spriteMask != 0)
								renderer->drawSpriteMask(sprite, spriteMask, posX, posY, outfitColors);
							else
								renderer->drawSprite(sprite, posX, posY);
						}
					}
					if(outfitAddons & 2)
					{
						sprite = thingType->getSprite(ThingFrameGroup_Idle, cx, cy, 0, DIRECTION_SOUTH, 2, zPattern, animation);
						spriteMask = thingType->getSprite(ThingFrameGroup_Idle, cx, cy, 1, DIRECTION_SOUTH, 2, zPattern, animation);
						if(sprite != 0)
						{
							if(spriteMask != 0)
								renderer->drawSpriteMask(sprite, spriteMask, posX, posY, outfitColors);
							else
								renderer->drawSprite(sprite, posX, posY);
						}
					}
				}
			}
			else
			{
				Uint32 sprite = thingType->getSprite(ThingFrameGroup_Idle, cx, cy, 0, DIRECTION_SOUTH, 0, zPattern, animation);
				if(sprite != 0)
					renderer->drawSprite(sprite, posX, posY);

				if(colorize && thingType->m_frameGroup[ThingFrameGroup_Idle].m_patternY > 1)
				{
					if(outfitAddons & 1)
					{
						sprite = thingType->getSprite(ThingFrameGroup_Idle, cx, cy, 0, DIRECTION_SOUTH, 1, zPattern, animation);
						if(sprite != 0)
							renderer->drawSprite(sprite, posX, posY);
					}
					if(outfitAddons & 2)
					{
						sprite = thingType->getSprite(ThingFrameGroup_Idle, cx, cy, 0, DIRECTION_SOUTH, 2, zPattern, animation);
						if(sprite != 0)
							renderer->drawSprite(sprite, posX, posY);
					}
				}
			}

			posX -= 32;
		}
		posY -= 32;
	}
}

static void renderStorePreviewOutfit(const GameStoreOutfitPreview& preview, const iRect& rect)
{
	ThingType* outfit = (preview.type == 0 ? NULL : g_thingManager.getThingType(ThingCategory_Creature, preview.type));
	ThingType* mount = (preview.mount == 0 ? NULL : g_thingManager.getThingType(ThingCategory_Creature, preview.mount));
	if(!outfit && !mount)
		return;

	Uint32 outfitColors = (preview.feet << 24) | (preview.legs << 16) | (preview.body << 8) | (preview.head);
	Uint8 outfitAnimation = getStorePreviewAnimation(outfit);
	Uint8 mountAnimation = getStorePreviewAnimation(mount);
	Uint8 zPattern = 0;
	if(mount && outfit)
		zPattern = UTIL_min<Uint8>(1, outfit->m_frameGroup[ThingFrameGroup_Idle].m_patternZ - 1);

	auto& renderer = g_engine.getRender();
	renderer->beginGameScene();
	renderStorePreviewFloor();
	if(mount)
		renderStorePreviewCreature(mount, 0, 0, 0, mountAnimation, false);
	if(outfit)
		renderStorePreviewCreature(outfit, outfitColors, preview.addons, zPattern, outfitAnimation, true);
	renderer->endGameScene();
	renderer->drawGameScene(16, 0, 128, 72, rect.x1 + 4, rect.y1 + 4, rect.x2 - 8, rect.y2 - 8);
}

static void closeGameStoreConfirmWindow()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_GAMESTORE_CONFIRM);
	if(pWindow)
		g_engine.removeWindow(pWindow);
}

static void createGameStoreConfirmWindow(const GameStoreOffer& offer)
{
	closeGameStoreConfirmWindow();

	g_gameStorePendingPurchase = offer;
	g_gameStoreHasPendingPurchase = true;

	GUI_Window* pWindow = new GUI_Window(iRect(0, 0, GAME_STORE_CONFIRM_WIDTH, GAME_STORE_CONFIRM_HEIGHT), "Confirm Purchase", GUI_WINDOW_GAMESTORE_CONFIRM);
	std::string message = "Are you sure you want to buy this item?\n\n" + offer.title + "\nPrice: " + getOfferPriceLabel(offer);
	GUI_MultiTextBox* textBox = new GUI_MultiTextBox(iRect(16, 34, GAME_STORE_CONFIRM_WIDTH - 32, 58), false, message, GAME_STORE_CONFIRM_TEXT_EVENTID, 180, 180, 180);
	pWindow->addChild(textBox);

	GUI_Separator* separator = new GUI_Separator(iRect(13, GAME_STORE_CONFIRM_HEIGHT - 40, GAME_STORE_CONFIRM_WIDTH - 26, 2));
	pWindow->addChild(separator);

	GUI_Button* button = new GUI_Button(iRect(GAME_STORE_CONFIRM_WIDTH - 102, GAME_STORE_CONFIRM_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "No", CLIENT_GUI_ESCAPE_TRIGGER, "Cancel this purchase");
	button->setButtonEventCallback(&gameStore_Events, GAME_STORE_CONFIRM_NO_EVENTID);
	button->startEvents();
	pWindow->addChild(button);

	button = new GUI_Button(iRect(GAME_STORE_CONFIRM_WIDTH - 155, GAME_STORE_CONFIRM_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Yes", CLIENT_GUI_ENTER_TRIGGER, "Confirm this purchase");
	button->setTextColor(96, 224, 96);
	button->setButtonEventCallback(&gameStore_Events, GAME_STORE_CONFIRM_YES_EVENTID);
	button->startEvents();
	pWindow->addChild(button);

	g_engine.addWindow(pWindow, true);
}

static void requestGameStoreFetch()
{
	JSON_OBJECT requestData;
	JSON_OBJECT rootObject;
	rootObject["action"] = new JSON_VALUE("fetch");
	rootObject["data"] = new JSON_VALUE(requestData);
	std::unique_ptr<JSON_VALUE> payload(new JSON_VALUE(rootObject));
	g_game.sendExtendedOpcode(GAME_STORE_OPCODE, JSON_VALUE::encode(payload.get()));
}

static void sendGameStorePurchase(const GameStoreOffer& offer)
{
	JSON_OBJECT purchaseData;
	purchaseData["category"] = new JSON_VALUE(offer.category);
	purchaseData["title"] = new JSON_VALUE(offer.title);
	purchaseData["price"] = new JSON_VALUE(offer.price);

	JSON_OBJECT rootObject;
	rootObject["action"] = new JSON_VALUE("purchase");
	rootObject["data"] = new JSON_VALUE(purchaseData);
	std::unique_ptr<JSON_VALUE> payload(new JSON_VALUE(rootObject));
	g_game.sendExtendedOpcode(GAME_STORE_OPCODE, JSON_VALUE::encode(payload.get()));
}

static void refreshGameStoreDetails(GUI_Window* pWindow)
{
	if(!pWindow || pWindow->getInternalID() != GUI_WINDOW_GAMESTORE)
		return;

	ensureSelections();
	GameStoreCategory* category = getSelectedCategory();
	GameStoreOffer* offer = getSelectedOffer();

	GUI_MultiTextBox* categoryInfo = SDL_static_cast(GUI_MultiTextBox*, pWindow->getChild(GAME_STORE_CATEGORY_INFO_EVENTID));
	if(categoryInfo)
		categoryInfo->setText(category ? category->description : "Waiting for categories from the server...");

	GUI_Label* pointsLabel = SDL_static_cast(GUI_Label*, pWindow->getChild(GAME_STORE_POINTS_EVENTID));
	if(pointsLabel)
		pointsLabel->setName("Premium Points: " + formatNumber(g_gameStorePoints));

	GUI_Label* guildPointsLabel = SDL_static_cast(GUI_Label*, pWindow->getChild(GAME_STORE_GUILD_POINTS_EVENTID));
	if(guildPointsLabel)
		guildPointsLabel->setName("Guild Points: " + formatNumber(g_gameStoreGuildPoints));

	GUI_DynamicLabel* offerTitle = SDL_static_cast(GUI_DynamicLabel*, pWindow->getChild(GAME_STORE_OFFER_TITLE_EVENTID));
	if(offerTitle)
		offerTitle->setName(offer ? offer->title : "Select an offer");

	GUI_Label* offerPrice = SDL_static_cast(GUI_Label*, pWindow->getChild(GAME_STORE_OFFER_PRICE_EVENTID));
	if(offerPrice)
	{
		offerPrice->setName(offer ? ("Price: " + getOfferPriceLabel(*offer)) : "Price: -");
		offerPrice->setColor((offer && offer->currency == "guildpoints") ? 240 : 96, (offer && offer->currency == "guildpoints") ? 208 : 224, 96);
	}

	GUI_Label* offerType = SDL_static_cast(GUI_Label*, pWindow->getChild(GAME_STORE_OFFER_TYPE_EVENTID));
	if(offerType)
		offerType->setName(offer ? ("Type: " + offer->type) : "Type: -");

	GUI_MultiTextBox* descriptionBox = SDL_static_cast(GUI_MultiTextBox*, pWindow->getChild(GAME_STORE_DESCRIPTION_EVENTID));
	if(descriptionBox)
	{
		if(offer)
		{
			std::string description = offer->description;
			if(offer->count > 1)
				description.append("\n\nCount: ").append(std::to_string(offer->count));
			descriptionBox->setText(description);
		}
		else
			descriptionBox->setText("Choose a category on the left and then select an offer to inspect it here.");
	}
}

static void buildGameStoreWindow(GUI_Window* pWindow)
{
	pWindow->clearChilds();
	ensureSelections();
	GameStoreCategory* category = getSelectedCategory();
	std::vector<GameStoreOffer>* offers = getSelectedOfferList();

	GUI_Label* newLabel = new GUI_Label(iRect(18, 32, 0, 0), "Premium Points: " + formatNumber(g_gameStorePoints), GAME_STORE_POINTS_EVENTID, 96, 224, 96);
	pWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(18, 48, 0, 0), "Guild Points: " + formatNumber(g_gameStoreGuildPoints), GAME_STORE_GUILD_POINTS_EVENTID, 240, 208, 96);
	pWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(198, 32, 0, 0), (category ? category->title : "Store Catalog"), 0, 175, 175, 175);
	pWindow->addChild(newLabel);

	GUI_MultiTextBox* newTextBox = new GUI_MultiTextBox(iRect(198, 46, 410, 24), false, (category ? category->description : "Waiting for categories from the server..."), GAME_STORE_CATEGORY_INFO_EVENTID, 175, 175, 175);
	newTextBox->startEvents();
	pWindow->addChild(newTextBox);

	GUI_ListBox* newListBox = new GUI_ListBox(iRect(18, 78, 170, 258), GAME_STORE_CATEGORIES_EVENTID);
	for(std::vector<GameStoreCategory>::iterator it = g_gameStoreCategories.begin(), end = g_gameStoreCategories.end(); it != end; ++it)
		newListBox->add(it->title);
	if(g_gameStoreSelectedCategory >= 0)
		newListBox->setSelect(g_gameStoreSelectedCategory);
	newListBox->setEventCallback(&gameStore_Events, GAME_STORE_CATEGORIES_EVENTID);
	newListBox->startEvents();
	pWindow->addChild(newListBox);

	newListBox = new GUI_ListBox(iRect(198, 78, 210, 258), GAME_STORE_OFFERS_EVENTID);
	if(offers)
	{
		for(std::vector<GameStoreOffer>::iterator it = offers->begin(), end = offers->end(); it != end; ++it)
			newListBox->add(it->title + " - " + getOfferPriceLabel(*it));
	}
	if(g_gameStoreSelectedOffer >= 0)
		newListBox->setSelect(g_gameStoreSelectedOffer);
	newListBox->setEventCallback(&gameStore_Events, GAME_STORE_OFFERS_EVENTID);
	newListBox->startEvents();
	pWindow->addChild(newListBox);

	GUI_GameStorePreview* newPreview = new GUI_GameStorePreview(iRect(422, 78, 186, 96));
	pWindow->addChild(newPreview);

	GUI_DynamicLabel* newDynamicLabel = new GUI_DynamicLabel(iRect(422, 182, 186, 14), "", GAME_STORE_OFFER_TITLE_EVENTID, 214, 214, 214);
	pWindow->addChild(newDynamicLabel);
	newLabel = new GUI_Label(iRect(422, 198, 0, 0), "", GAME_STORE_OFFER_PRICE_EVENTID, 96, 224, 96);
	pWindow->addChild(newLabel);
	newLabel = new GUI_Label(iRect(422, 214, 0, 0), "", GAME_STORE_OFFER_TYPE_EVENTID, 143, 143, 143);
	pWindow->addChild(newLabel);

	newTextBox = new GUI_MultiTextBox(iRect(422, 230, 186, 106), false, "", GAME_STORE_DESCRIPTION_EVENTID, 175, 175, 175);
	newTextBox->startEvents();
	pWindow->addChild(newTextBox);

	GUI_Separator* newSeparator = new GUI_Separator(iRect(13, GAME_STORE_HEIGHT - 40, GAME_STORE_WIDTH - 26, 2));
	pWindow->addChild(newSeparator);

	GUI_Button* newButton = new GUI_Button(iRect(18, GAME_STORE_HEIGHT - 30, GUI_UI_BUTTON_58PX_GRAY_UP_W, GUI_UI_BUTTON_58PX_GRAY_UP_H), "Refresh", 0, "Fetch the latest store offers");
	newButton->setButtonEventCallback(&gameStore_Events, GAME_STORE_REFRESH_EVENTID);
	newButton->startEvents();
	pWindow->addChild(newButton);

	newButton = new GUI_Button(iRect(GAME_STORE_WIDTH - 149, GAME_STORE_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Buy", CLIENT_GUI_ENTER_TRIGGER, "Buy the selected offer");
	newButton->setTextColor(96, 224, 96);
	newButton->setButtonEventCallback(&gameStore_Events, GAME_STORE_PURCHASE_EVENTID);
	newButton->startEvents();
	pWindow->addChild(newButton);

	newButton = new GUI_Button(iRect(GAME_STORE_WIDTH - 96, GAME_STORE_HEIGHT - 30, GUI_UI_BUTTON_43PX_GRAY_UP_W, GUI_UI_BUTTON_43PX_GRAY_UP_H), "Close", CLIENT_GUI_ESCAPE_TRIGGER, "Close the game store");
	newButton->setButtonEventCallback(&gameStore_Events, GAME_STORE_CLOSE_EVENTID);
	newButton->startEvents();
	pWindow->addChild(newButton);

	refreshGameStoreDetails(pWindow);
}

void gameStore_Events(Uint32 event, Sint32 status)
{
	switch(event)
	{
		case GAME_STORE_CLOSE_EVENTID:
		{
			GUI_Window* pWindow = g_engine.getCurrentWindow();
			if(pWindow && pWindow->getInternalID() == GUI_WINDOW_GAMESTORE)
			{
				g_gameStoreHasPendingPurchase = false;
				closeGameStoreConfirmWindow();
				g_haveGameStoreOpen = false;
				g_engine.removeWindow(pWindow);
			}
		}
		break;
		case GAME_STORE_REFRESH_EVENTID:
			requestGameStoreFetch();
		break;
		case GAME_STORE_PURCHASE_EVENTID:
		{
			ensureSelections();
			GameStoreOffer* offer = getSelectedOffer();
			if(!offer)
			{
				UTIL_messageBox("Game Store", "Select an offer first.");
				break;
			}
			createGameStoreConfirmWindow(*offer);
		}
		break;
		case GAME_STORE_CONFIRM_YES_EVENTID:
		{
			if(g_gameStoreHasPendingPurchase)
				sendGameStorePurchase(g_gameStorePendingPurchase);
			g_gameStoreHasPendingPurchase = false;
			closeGameStoreConfirmWindow();
		}
		break;
		case GAME_STORE_CONFIRM_NO_EVENTID:
		{
			g_gameStoreHasPendingPurchase = false;
			closeGameStoreConfirmWindow();
		}
		break;
		case GAME_STORE_CATEGORIES_EVENTID:
		{
			if(status < 0)
				status = ~(status);

			if(status >= 0 && SDL_static_cast(size_t, status) < g_gameStoreCategories.size())
			{
				g_gameStoreSelectedCategory = status;
				g_gameStoreSelectedOffer = -1;
				ensureSelections();
				UTIL_createGameStoreWindow();
			}
		}
		break;
		case GAME_STORE_OFFERS_EVENTID:
		{
			if(status < 0)
				status = ~(status);

			std::vector<GameStoreOffer>* offers = getSelectedOfferList();
			if(offers && status >= 0 && SDL_static_cast(size_t, status) < offers->size())
				g_gameStoreSelectedOffer = status;
			else
				g_gameStoreSelectedOffer = -1;

			GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_GAMESTORE);
			if(pWindow)
				refreshGameStoreDetails(pWindow);
		}
		break;
		default: break;
	}
}

void UTIL_createGameStoreWindow()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_GAMESTORE);
	if(!pWindow)
	{
		pWindow = new GUI_Window(iRect(0, 0, GAME_STORE_WIDTH, GAME_STORE_HEIGHT), GAME_STORE_TITLE, GUI_WINDOW_GAMESTORE);
		buildGameStoreWindow(pWindow);
		g_engine.addWindow(pWindow, true);
	}
	else
		buildGameStoreWindow(pWindow);

	g_haveGameStoreOpen = true;
}

void UTIL_toggleGameStore()
{
	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_GAMESTORE);
	if(pWindow)
	{
		g_haveGameStoreOpen = false;
		g_engine.removeWindow(pWindow);
		return;
	}

	UTIL_createGameStoreWindow();
	requestGameStoreFetch();
}

void UTIL_resetGameStore()
{
	g_gameStoreCategories.clear();
	g_gameStoreOffers.clear();
	g_gameStorePoints = 0;
	g_gameStoreGuildPoints = 0;
	g_gameStoreSelectedCategory = -1;
	g_gameStoreSelectedOffer = -1;
	g_gameStoreHasPendingPurchase = false;
	g_haveGameStoreOpen = false;
	closeGameStoreConfirmWindow();
}

bool UTIL_handleGameStoreExtendedOpcode(Uint8 opcode, const std::string& payload)
{
	if(opcode != GAME_STORE_OPCODE)
		return false;

	std::unique_ptr<JSON_VALUE> dataJson(JSON_VALUE::decode(payload.c_str()));
	JSON_VALUE* dataObject = dataJson.get();
	if(!dataObject || !dataObject->IsObject())
		return true;

	const std::string action = getJsonString(dataObject, "action");
	JSON_VALUE* data = dataObject->find("data");
	if(action == "fetchBase")
	{
		g_gameStoreCategories.clear();
		g_gameStoreOffers.clear();
		g_gameStoreSelectedCategory = -1;
		g_gameStoreSelectedOffer = -1;
		if(data)
		{
			JSON_VALUE* categories = data->find("categories");
			if(categories && categories->IsArray())
			{
				for(size_t i = 0, end = categories->size(); i < end; ++i)
				{
					JSON_VALUE* category = categories->find(i);
					if(!category || !category->IsObject())
						continue;

					GameStoreCategory parsedCategory;
					parsedCategory.title = getJsonString(category, "title");
					parsedCategory.description = getJsonString(category, "description");
					if(!parsedCategory.title.empty())
						g_gameStoreCategories.push_back(parsedCategory);
				}
			}
		}
		ensureSelections();
	}
	else if(action == "fetchOffers")
	{
		if(data && data->IsObject())
		{
			const std::string categoryName = getJsonString(data, "category");
			std::vector<GameStoreOffer>& categoryOffers = g_gameStoreOffers[categoryName];
			categoryOffers.clear();

			JSON_VALUE* offers = data->find("offers");
			if(offers && offers->IsArray())
			{
				for(size_t i = 0, end = offers->size(); i < end; ++i)
				{
					JSON_VALUE* entry = offers->find(i);
					if(!entry || !entry->IsObject())
						continue;

					GameStoreOffer parsedOffer;
					parsedOffer.category = categoryName;
					parsedOffer.type = getJsonString(entry, "type");
					parsedOffer.title = getJsonString(entry, "title");
					parsedOffer.description = getJsonString(entry, "description");
					parsedOffer.currency = getJsonString(entry, "currency");
					parsedOffer.price = getJsonNumber(entry, "price");
					parsedOffer.count = SDL_static_cast(Uint16, getJsonNumber(entry, "count", 1));
					parsedOffer.clientId = SDL_static_cast(Uint16, getJsonNumber(entry, "clientId"));
					parsedOffer.previewOutfit = parseOutfitPreview(entry->find("outfit"));
					if(parsedOffer.currency.empty())
						parsedOffer.currency = "premium_points";
					categoryOffers.push_back(parsedOffer);
				}
			}
		}
		ensureSelections();
	}
	else if(action == "points")
	{
		if(data && data->IsNumber())
			g_gameStorePoints = SDL_static_cast(Uint32, data->AsNumber());
	}
	else if(action == "guildpoints")
	{
		if(data && data->IsNumber())
			g_gameStoreGuildPoints = SDL_static_cast(Uint32, data->AsNumber());
	}
	else if(action == "msg")
	{
		if(data && data->IsObject())
		{
			UTIL_messageBox((getJsonString(data, "type") == "error" ? "Store Error" : "Store Information"), getJsonString(data, "msg"));
			if(getJsonBool(data, "close"))
			{
				GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_GAMESTORE);
				if(pWindow)
				{
					g_haveGameStoreOpen = false;
					g_engine.removeWindow(pWindow);
				}
			}
		}
	}

	GUI_Window* pWindow = g_engine.getWindow(GUI_WINDOW_GAMESTORE);
	if(pWindow)
		buildGameStoreWindow(pWindow);
	return true;
}

GUI_GameStorePreview::GUI_GameStorePreview(iRect boxRect, Uint32 internalID)
{
	setRect(boxRect);
	m_internalID = internalID;
}

void GUI_GameStorePreview::render()
{
	auto& renderer = g_engine.getRender();
	renderer->fillRectangle(m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2, 36, 36, 36, 255);
	renderer->drawRectangle(m_tRect.x1, m_tRect.y1, m_tRect.x2, m_tRect.y2, 1, 96, 96, 96, 255);

	GameStoreOffer* offer = getSelectedOffer();
	if(!offer)
	{
		g_engine.drawFont(CLIENT_FONT_NONOUTLINED, m_tRect.x1 + (m_tRect.x2 / 2), m_tRect.y1 + (m_tRect.y2 / 2) - 6, "No preview", 143, 143, 143, CLIENT_FONT_ALIGN_CENTER);
		return;
	}

	if(offer->type == "item" && offer->clientId != 0)
	{
		std::unique_ptr<ItemUI> item(ItemUI::createItemUI(offer->clientId, 1));
		if(item)
		{
			item->setSubtype((offer->count == 0 ? 1 : offer->count), offer->count > 1);
			item->render(m_tRect.x1 + (m_tRect.x2 / 2) - 16, m_tRect.y1 + (m_tRect.y2 / 2) - 16, 32);
			return;
		}
	}

	GameStoreOutfitPreview preview = getFallbackOutfitPreview(*offer);
	if(preview.valid)
	{
		renderStorePreviewOutfit(preview, m_tRect);
		if(preview.type != 0 || preview.mount != 0)
			return;
	}

	g_engine.drawFont(CLIENT_FONT_NONOUTLINED, m_tRect.x1 + (m_tRect.x2 / 2), m_tRect.y1 + (m_tRect.y2 / 2) - 6, "Text only", 143, 143, 143, CLIENT_FONT_ALIGN_CENTER);
}
