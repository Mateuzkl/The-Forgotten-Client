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

#ifndef __FILE_PROTOCOL_GAME_h_
#define __FILE_PROTOCOL_GAME_h_

#include "protocol.h"

enum GameOpcodes : Uint8
{
	//Send Opcodes
	GameLoginOpcode = 0x0A,
	GameSecondaryLoginOpcode = 0x0B,
	GameEnterGameOpcode = 0x0F,
	GameLeaveOpcode = 0x14,
	GameConnectionPingBackOpcode = 0x1C,
	GamePingOpcode = 0x1D,
	GamePingBackOpcode = 0x1E,
	GameOtclientOpcode = 0x32,
	GameStashActionOpcode = 0x28,
	GameDepotSearchRetrieveOpcode = 0x29,
	GameTrackBestiaryRaceOpcode = 0x2A,
	GamePartyHuntAnalyserOpcode = 0x2B,
	GameTeamFinderAssembleTeamOpcode = 0x2C,
	GameTeamFinderJoinTeamOpcode = 0x2D,
	GameClientCheckOpcode = 0x63,
	GameAutowalkOpcode = 0x64,
	GameWalkNorthOpcode = 0x65,
	GameWalkEastOpcode = 0x66,
	GameWalkSouthOpcode = 0x67,
	GameWalkWestOpcode = 0x68,
	GameStopAutowalkOpcode = 0x69,
	GameWalkNorthEastOpcode = 0x6A,
	GameWalkSouthEastOpcode = 0x6B,
	GameWalkSouthWestOpcode = 0x6C,
	GameWalkNorthWestOpcode = 0x6D,
	GameTurnNorthOpcode = 0x6F,
	GameTurnEastOpcode = 0x70,
	GameTurnSouthOpcode = 0x71,
	GameTurnWestOpcode = 0x72,
	GameTeleportOpcode = 0x73,
	GameCharacterTradeConfigurationActionOpcode = 0x76,
	GameEqupItemOpcode = 0x77,
	GameThrowOpcode = 0x78,
	GameLookInShopOpcode = 0x79,
	GameNPCPurchaseOpcode = 0x7A,
	GameNPCSellOpcode = 0x7B,
	GameNPCCloseOpcode = 0x7C,
	GameRequestTradeOpcode = 0x7D,
	GameLookInTradeOpcode = 0x7E,
	GameAcceptTradeOpcode = 0x7F,
	GameCloseTradeOpcode = 0x80,
	GameFriendSystemActionOpcode = 0x81,
	GameUseItemOpcode = 0x82,
	GameUseItemExOpcode = 0x83,
	GameUseWithCreatureOpcode = 0x84,
	GameRotateItemOpcode = 0x85,
	GameCloseContainerOpcode = 0x87,
	GameUpContainerOpcode = 0x88,
	GameTextWindowOpcode = 0x89,
	GameHouseWindowOpcode = 0x8A,
	GameWrapStateOpcode = 0x8B,
	GameLookAtOpcode = 0x8C,
	GameLookInBattleOpcode = 0x8D,
	GameJoinAggressionOpcode = 0x8E,
	GameQuickLootOpcode = 0x8F,
	GameLootContainerOpcode = 0x90,
	GameQuickLootBlackWhitelistOpcode = 0x91,
	GameOpenDepotSearchOpcode = 0x92,
	GameCloseDepotSearchOpcode = 0x93,
	GameDepotSearchTypeOpcode = 0x94,
	GameOpenParentContainerOpcode = 0x95,
	GameSayOpcode = 0x96,
	GameRequestChannelsOpcode = 0x97,
	GameOpenChannelOpcode = 0x98,
	GameCloseChannelOpcode = 0x99,
	GameOpenPrivateChannelOpcode = 0x9A,
	GameGuldMessageOpcode = 0x9B,
	GameEditGuldMessageOpcode = 0x9C,
	GameOpenRuleViolationChannelOpcode = 0x9B,
	GameCloseRuleViolationChannelOpcode = 0x9C,
	GameCancelRuleViolationChannelOpcode = 0x9D,
	GameCloseNpcChannelOpcode = 0x9E,
	GameAttackModesOpcode = 0xA0,
	GameAttackOpcode = 0xA1,
	GameFollowOpcode = 0xA2,
	GameInviteToPartyOpcode = 0xA3,
	GameJoinToPartyOpcode = 0xA4,
	GameRevokePartyInviteOpcode = 0xA5,
	GamePassPartyLeadershipOpcode = 0xA6,
	GameLeavePartyOpcode = 0xA7,
	GameEnableSharedPartyExperienceOpcode = 0xA8,
	GameCreatePrivateChannelOpcode = 0xAA,
	GameChannelInviteOpcode = 0xAB,
	GameChannelExcludeOpcode = 0xAC,
	GameCyclopediaHouseActionOpcode = 0xAD,
	GameHighscoresOpcode = 0xB1,
	GamePreyHuntingTaskActionOpcode = 0xBA,
	GameCancelAttackAndFollowOpcode = 0xBE,
	GameClaimTournamentReward = 0xC3,
	GameTournamentInformationOpcode = 0xC4,
	GameSubscribeToUpdatesOpcode = 0xC6,
	GameTournamentLeaderboardOpcode = 0xC7,
	GameTournamentTicketActionOpcode = 0xC8,
	GameGetTransactionDetailsOpcode = 0xC9,
	GameUpdateTileOpcode = 0xC9,
	GameUpdateExivaOptionsOpcode = 0xCA,
	GameUpdateContainerOpcode = 0xCA,
	GameBrowseFieldOpcode = 0xCB,
	GameSeekInContainerOpcode = 0xCC,
	GameInspectObjectOpcode = 0xCD,
	GameInspectPlayerOpcode = 0xCE,
	GameBlessingsDialogOpcode = 0xCF,
	GameTrackQuestFlagsOpcode = 0xD0,
	GameMarketStatisticsOpcode = 0xD1,
	GameRequestOutfitOpcode = 0xD2,
	GameSetOutfitOpcode = 0xD3,
	GameMountOpcode = 0xD4,
	GameApplyImbuementOpcode = 0xD5,
	GameClearCharmOpcode = 0xD6,
	GameImbuementLeaveOpcode = 0xD7,
	GameOpenRewardWallOpcode = 0xD8,
	GameOpenRewardHistoryOpcode = 0xD9,
	GameCollectRewardOpcode = 0xDA,
	GameCyclopediaMapActionOpcode = 0xDB,
	GameAddVipOpcode = 0xDC,
	GameRemoveVipOpcode = 0xDD,
	GameEditVipOpcode = 0xDE,
	GameVipGroupActionOpcode = 0xDF,
	GameMarkGameNewsAsReadOpcode = 0xE0,
	GameOpenMonsterCyclopediaOpcode = 0xE1,
	GameOpenMonsterCyclopediaMonstersOpcode = 0xE2,
	GameOpenMonsterCyclopediaRaceOpcode = 0xE3,
	GameMonsterBonusEffectActionOpcode = 0xE4,
	GameOpenCyclopediaCharacterInfoOpcode = 0xE5,
	GameBugReportOpcode = 0xE6,
	GameRuleViolationOpcode = 0xE7,
	GameThankYouOpcode = 0xE7,
	GameDebugAssertOpcode = 0xE8,
	GameGetOfferDescriptionOpcode = 0xE8,
	GameStoreEventOpcode = 0xE9,
	GameFeatureEventOpcode = 0xEA,
	GamePreyActionOpcode = 0xEB,
	GameSetHirelingNameOpcode = 0xEC,
	GameRequestResourceBalanceOpcode = 0xED,
	GameGreetOpcode = 0xEE,
	GameTransferCoinsOpcode = 0xEF,
	GameShowQuestLogOpcode = 0xF0,
	GameShowQuestLineOpcode = 0xF1,
	GameNewRuleViolationOpcode = 0xF2,
	GameRequestItemInfoOpcode = 0xF3,
	GameMarketLeaveOpcode = 0xF4,
	GameMarketBrowseOpcode = 0xF5,
	GameMarketCreateOpcode = 0xF6,
	GameMarketCancelOpcode = 0xF7,
	GameMarketAcceptOpcode = 0xF8,
	GameAnswerModalDialogOpcode = 0xF9,
	GameOpenStoreOpcode = 0xFA,
	GameRequestStoreOpcode = 0xFB,
	GameBuyStoreOpcode = 0xFC,
	GameOpenTransactionStoreOpcode = 0xFD,
	GameRequestTransactionStoreOpcode = 0xFE,

	//Recv Opcodes
	RecvCreatureDataOpcode = 0x03,
	RecvSessionDumpStartOpcode = 0x04,
	RecvLoginOrPendingOpcode = 0x0A,
	RecvGMActionsOpcode = 0x0B,
	RecvEnterGameOpcode = 0x0F,
	RecvUpdateNeededOpcode = 0x11,
	RecvLoginErrorOpcode = 0x14,
	RecvLoginAdviceOpcode = 0x15,
	RecvLoginWaitOpcode = 0x16,
	RecvLoginSuccessOpcode = 0x17,
	RecvLoginTokenOpcode = 0x18,
	RecvStoreButtonIndicatorsOpcode = 0x19,
	RecvPingBackOpcode = 0x1D,
	RecvPingOpcode = 0x1E,
	RecvChallengeOpcode = 0x1F,
	RecvDeathOpcode = 0x28,
	RecvStashOpcode = 0x29,
	RecvSpecialContainersAvailableOpcode = 0x2A,
	RecvPartyHuntAnalyserOpcode = 0x2B,
	RecvTeamFinderTeamLeaderOpcode = 0x2C,
	RecvTeamFinderTeamMemberOpcode = 0x2D,
	RecvOtclientOpcode = 0x32,
	RecvServerFeaturesOpcode = 0x43,
	RecvClientCheckOpcode = 0x63,
	RecvMapDescriptionOpcode = 0x64,
	RecvMapNorthOpcode = 0x65,
	RecvMapEastOpcode = 0x66,
	RecvMapSouthOpcode = 0x67,
	RecvMapWestOpcode = 0x68,
	RecvUpdateTileOpcode = 0x69,
	RecvTileAddThingOpcode = 0x6A,
	RecvTileTransformThingOpcode = 0x6B,
	RecvTileRemoveThingOpcode = 0x6C,
	RecvTileMoveCreatureOpcode = 0x6D,
	RecvContainerOpenOpcode = 0x6E,
	RecvContainerCloseOpcode = 0x6F,
	RecvContainerAddItemOpcode = 0x70,
	RecvContainerTransformItemOpcode = 0x71,
	RecvContainerRemoveItemOpcode = 0x72,
	RecvFriendSystemDataOpcode = 0x74,
	RecvScreenshotEventOpcode = 0x75,
	RecvInspectionListOpcode = 0x76,
	RecvInspectionStateOpcode = 0x77,
	RecvInventoryTransformItemOpcode = 0x78,
	RecvInventoryRemoveItemOpcode = 0x79,
	RecvNPCOpenTradeOpcode = 0x7A,
	RecvNPCPlayerGoodsOpcode = 0x7B,
	RecvNPCCloseTradeOpcode = 0x7C,
	RecvTradeOwnOpcode = 0x7D,
	RecvTradeCounterOpcode = 0x7E,
	RecvTradeCloseOpcode = 0x7F,
	RecvCharacterTradeConfigurationOpcode = 0x80,
	RecvAmbientWorldLightOpcode = 0x82,
	RecvMagicEffectOpcode = 0x83,
	RecvAnimatedTextOpcode = 0x84,
	RecvDistanceEffectOpcode = 0x85,
	RecvCreatureMarkOpcode = 0x86,
	RecvCreatureTrappersOpcode = 0x87,
	RecvCreatureUpdateOpcode = 0x8B,
	RecvCreatureHealthOpcode = 0x8C,
	RecvCreatureLightOpcode = 0x8D,
	RecvCreatureOutfitOpcode = 0x8E,
	RecvCreatureSpeedOpcode = 0x8F,
	RecvCreatureSkullOpcode = 0x90,
	RecvCreaturePartyOpcode = 0x91,
	RecvCreatureUnpassableOpcode = 0x92,
	RecvCreatureMarksOpcode = 0x93,
	RecvCreatureHelpersOpcode = 0x94,
	RecvCreatureTypeOpcode = 0x95,
	RecvEditTextWindowOpcode = 0x96,
	RecvEditHouseWindowOpcode = 0x97,
	RecvGameNewsOpcode = 0x98,
	RecvDepotSearchDetailListOpcode = 0x99,
	RecvCloseDepotSearchOpcode = 0x9A,
	RecvBlessingsDialogOpcode = 0x9B,
	RecvBlessingsOpcode = 0x9C,
	RecvPresetOpcode = 0x9D,
	RecvPremiumTriggerOpcode = 0x9E,
	RecvPlayerDataBasicOpcode = 0x9F,
	RecvPlayerDataOpcode = 0xA0,
	RecvPlayerSkillsOpcode = 0xA1,
	RecvPlayerIconsOpcode = 0xA2,
	RecvPlayerCancelTargetOpcode = 0xA3,
	RecvPlayerSpellDelayOpcode = 0xA4,
	RecvPlayerSpellGroupDelayOpcode = 0xA5,
	RecvPlayerMultiUseDelayOpcode = 0xA6,
	RecvPlayerModesOpcode = 0xA7,
	RecvPlayerStoreDeepLinkOpcode = 0xA8,
	RecvRestingAreaStateOpcode = 0xA9,
	RecvPlayerTalkOpcode = 0xAA,
	RecvChannelsOpcode = 0xAB,
	RecvOpenChannelOpcode = 0xAC,
	RecvOpenPrivateChannelOpcode = 0xAD,
	RecvRuleViolationChannelOpcode = 0xAE,
	RecvRuleViolationRemoveOpcode = 0xAF,
	RecvRuleViolationCancelOpcode = 0xB0,
	RecvRuleViolationLockOpcode = 0xB1,
	RecvOpenOwnChannelOpcode = 0xB2,
	RecvCloseChannelOpcode = 0xB3,
	RecvTextMessageOpcode = 0xB4,
	RecvCancelWalkOpcode = 0xB5,
	RecvWalkWaitOpcode = 0xB6,
	RecvUnjustifiedStatsOpcode = 0xB7,
	RecvPvpSituationsOpcode = 0xB8,
	RecvBestiaryTrackerOpcode = 0xB9,
	RecvPreyHuntingTaskBaseDataOpcode = 0xBA,
	RecvPreyHuntingTaskDataOpcode = 0xBB,
	RecvFloorChangeUpOpcode = 0xBE,
	RecvFloorChangeDownOpcode = 0xBF,
	RecvUpdateLootContainersOpcode = 0xC0,
	RecvPlayerDataTournamentOpcode = 0xC1,
	RecvCyclopediaHouseActionResultOpcode = 0xC3,
	RecvTournamentInformationOpcode = 0xC4,
	RecvTournamentLeaderboardOpcode = 0xC5,
	RecvCyclopediaStaticHouseDataOpcode = 0xC6,
	RecvCyclopediaCurrentHouseDataOpcode = 0xC7,
	RecvChooseOutfitOpcode = 0xC8,
	RecvExivaSuppressedOpcode = 0xC9,
	RecvUpdateExivaOptionsOpcode = 0xCA,
	RecvTransactionDetailsOpcode = 0xCB,
	RecvImpactTrackingOpcode = 0xCC,
	RecvMarketStatisticsOpcode = 0xCD,
	RecvItemWastedOpcode = 0xCE,
	RecvItemLootedOpcode = 0xCF,
	RecvTrackedQuestFlagsOpcode = 0xD0,
	RecvKillTrackingOpcode = 0xD1,
	RecvVipAddOpcode = 0xD2,
	RecvVipStatusOpcode = 0xD3,
	RecvVipStatusLogoutOpcode = 0xD4,
	RecvMonsterCyclopediaOpcode = 0xD5,
	RecvMonsterCyclopediaMonstersOpcode = 0xD6,
	RecvMonsterCyclopediaRaceOpcode = 0xD7,
	RecvMonsterCyclopediaBonusEffectsOpcode = 0xD8,
	RecvMonsterCyclopediaNewDetailsOpcode = 0xD9,
	RecvCyclopediaCharacterInfoOpcode = 0xDA,
	RecvHirelingNameChangeOpcode = 0xDB,
	RecvTutorialHintOpcode = 0xDC,
	RecvAutomapFlagOpcode = 0xDD,
	RecvDailyRewardCollectionStateOpcode = 0xDE,
	RecvStoreCoinBalanceOpcode = 0xDF,
	RecvStoreErrorOpcode = 0xE0,
	RecvStoreRequestPurchaseOpcode = 0xE1,
	RecvOpenRewardWallOpcode = 0xE2,
	RecvCloseRewardWallOpcode = 0xE3,
	RecvDailyRewardBasicOpcode = 0xE4,
	RecvDailyRewardHistoryOpcode = 0xE5,
	RecvPreyFreeListRerollAvailabilityOpcode = 0xE6,
	RecvPreyTimeLeftOpcode = 0xE7,
	RecvPreyDataOpcode = 0xE8,
	RecvPreyPricesOpcode = 0xE9,
	RecvOfferDescriptionOpcode = 0xEA,
	RecvRefreshImbuingDialogOpcode = 0xEB,
	RecvCloseImbuingDialogOpcode = 0xEC,
	RecvShowMessageDialogOpcode = 0xED,
	RecvResourceBalanceOpcode = 0xEE,
	RecvTimeOpcode = 0xEF,
	RecvQuestLogOpcode = 0xF0,
	RecvQuestLineOpcode = 0xF1,
	RecvStoreCoinBalanceUpdatingOpcode = 0xF2,
	RecvChannelEventOpcode = 0xF3,
	RecvItemInfoOpcode = 0xF4,
	RecvPlayerInventoryOpcode = 0xF5,
	RecvMarketEnterOpcode = 0xF6,
	RecvMarketLeaveOpcode = 0xF7,
	RecvMarketDetailOpcode = 0xF8,
	RecvMarketBrowseOpcode = 0xF9,
	RecvModalWindowOpcode = 0xFA,
	RecvStoreOpenOpcode = 0xFB,
	RecvStoreOffersOpcode = 0xFC,
	RecvStoreTransactionHistoryOpcode = 0xFD,
	RecvStoreCompletePurchaseOpcode = 0xFE
};

/*
	QT Client outputmessage functions
	msg+0x10 - addU8()
	msg+0x14 - addU16()
	msg+0x18 - addU32()
	msg+0x1C - addU64()
	msg+0x24 - addString()
	msg+0x28 - addBytes(size)
	msg+0x2C - addBool()
	
	QT Client(12.50.10109) Gameclient payload functions
	{0x0A, "protobuf.protocol.GameclientMessageLogin"},									base+4BC540
	{0x0B, "protobuf.protocol.GameclientMessageSecondaryLogin"},						base+4BC950
	{0x0F, "protobuf.protocol.GameclientMessageEnterWorld"},							no payload
	{0x14, "protobuf.protocol.GameclientMessageQuitGame"},								no payload
	{0x1C, "protobuf.protocol.GameclientMessageConnectionPingBack"},					no payload
	{0x1D, "protobuf.protocol.GameclientMessagePing"},									no payload
	{0x1E, "protobuf.protocol.GameclientMessagePingBack"},								no payload
	{0x1F, "protobuf.protocol.GameclientMessage"},										placeholder
	{0x28, "protobuf.protocol.GameclientMessageStashAction"},							base+4C0BA0
	{0x29, "protobuf.protocol.GameclientMessageDepotSearchRetrieve"},					base+4C0F40
	{0x2A, "protobuf.protocol.GameclientMessageTrackBestiaryRace"},						base+4BCCA0
	{0x2B, "protobuf.protocol.GameclientMessagePartyHuntAnalyser"},						base+4C1180
	{0x2C, "protobuf.protocol.GameclientMessageTeamFinderAssembleTeam"},				base+4C14B0
	{0x2D, "protobuf.protocol.GameclientMessageTeamFinderJoinTeam"},					base+4C1840
	{0x63, "protobuf.protocol.GameclientMessageClientCheck"},							base+4BCD00
	{0x64, "protobuf.protocol.GameclientMessageGoPath"},								base+4BCBF0
	{0x65, "protobuf.protocol.GameclientMessageGoNorth"},								no payload
	{0x66, "protobuf.protocol.GameclientMessageGoEast"},								no payload
	{0x67, "protobuf.protocol.GameclientMessageGoSouth"},								no payload
	{0x68, "protobuf.protocol.GameclientMessageGoWest"},								no payload
	{0x69, "protobuf.protocol.GameclientMessageStop"},									no payload
	{0x6A, "protobuf.protocol.GameclientMessageGoNorthEast"},							no payload
	{0x6B, "protobuf.protocol.GameclientMessageGoSouthEast"},							no payload
	{0x6C, "protobuf.protocol.GameclientMessageGoSouthWest"},							no payload
	{0x6D, "protobuf.protocol.GameclientMessageGoNorthWest"},							no payload
	{0x6F, "protobuf.protocol.GameclientMessageRotateNorth"},							no payload
	{0x70, "protobuf.protocol.GameclientMessageRotateEast"},							no payload
	{0x71, "protobuf.protocol.GameclientMessageRotateSouth"},							no payload
	{0x72, "protobuf.protocol.GameclientMessageRotateWest"},							no payload
	{0x73, "protobuf.protocol.GameclientMessageTeleport"},								base+4BFC60
	{0x76, "protobuf.protocol.GameclientMessageCharacterTradeConfigurationAction"},		base+4C18B0
	{0x77, "protobuf.protocol.GameclientMessageEquipObject"},							base+4BCDB0
	{0x78, "protobuf.protocol.GameclientMessageMoveObject"},							base+4BCE30
	{0x79, "protobuf.protocol.GameclientMessageLookNPCTrade"},							base+4C0510
	{0x7A, "protobuf.protocol.GameclientMessageBuyObject"},								base+4C0590
	{0x7B, "protobuf.protocol.GameclientMessageSellObject"},							base+4C0690
	{0x7C, "protobuf.protocol.GameclientMessageCloseNPCTrade"},							no payload
	{0x7D, "protobuf.protocol.GameclientMessageTradeObject"},							base+4BE960
	{0x7E, "protobuf.protocol.GameclientMessageLookTrade"},								base+4BEA80
	{0x7F, "protobuf.protocol.GameclientMessageAcceptTrade"},							no payload
	{0x80, "protobuf.protocol.GameclientMessageRejectTrade"},							no payload
	{0x81, "protobuf.protocol.GameclientMessageFriendSystemAction"},					base+4BEAE0
	{0x82, "protobuf.protocol.GameclientMessageUseObject"},								base+4BD3A0
	{0x83, "protobuf.protocol.GameclientMessageUseTwoObjects"},							base+4BD4F0
	{0x84, "protobuf.protocol.GameclientMessageUseOnCreature"},							base+4BD6E0
	{0x85, "protobuf.protocol.GameclientMessageTurnObject"},							base+4BD2A0
	{0x87, "protobuf.protocol.GameclientMessageCloseContainer"},						base+4BCF80
	{0x88, "protobuf.protocol.GameclientMessageUpContainer"},							base+4BCF80
	{0x89, "protobuf.protocol.GameclientMessageEditText"},								base+4BEFA0
	{0x8A, "protobuf.protocol.GameclientMessageEditList"},								base+4BF030
	{0x8B, "protobuf.protocol.GameclientMessageToggleWrapState"},						base+4BD1A0
	{0x8C, "protobuf.protocol.GameclientMessageLook"},									base+4BD800
	{0x8D, "protobuf.protocol.GameclientMessageLookAtCreature"},						base+4BD900
	{0x8E, "protobuf.protocol.GameclientMessageJoinAggression"},						base+4BD900
	{0x8F, "protobuf.protocol.GameclientMessageQuickLoot"},								base+4C0030
	{0x90, "protobuf.protocol.GameclientMessageLootContainer"},							base+4C01E0
	{0x91, "protobuf.protocol.GameclientMessageQuickLootBlackWhitelist"},				base+4C0130
	{0x92, "protobuf.protocol.GameclientMessageOpenDepotSearch"},						no payload
	{0x93, "protobuf.protocol.GameclientMessageCloseDepotSearch"},						no payload
	{0x94, "protobuf.protocol.GameclientMessageDepotSearchType"},						base+4BDD60
	{0x95, "protobuf.protocol.GameclientMessageOpenParentContainer"},					base+4C0F00
	{0x96, "protobuf.protocol.GameclientMessageTalk"},									base+4BDC50
	{0x97, "protobuf.protocol.GameclientMessageGetChannels"},							no payload
	{0x98, "protobuf.protocol.GameclientMessageJoinChannel"},							base+4BDD60
	{0x99, "protobuf.protocol.GameclientMessageLeaveChannel"},							base+4BDD60
	{0x9A, "protobuf.protocol.GameclientMessagePrivateChannel"},						base+4BDDE0
	{0x9B, "protobuf.protocol.GameclientMessageGuildMessage"},							no payload
	{0x9C, "protobuf.protocol.GameclientMessageEditGuildMessage"},						base+4BDDE0
	{0x9E, "protobuf.protocol.GameclientMessageCloseNPCChannel"},						no payload
	{0xA0, "protobuf.protocol.GameclientMessageSetTactics"},							base+4BDB40
	{0xA1, "protobuf.protocol.GameclientMessageAttack"},								base+4BDD90
	{0xA2, "protobuf.protocol.GameclientMessageFollow"},								base+4BDD90
	{0xA3, "protobuf.protocol.GameclientMessageInviteToParty"},							base+4BD900
	{0xA4, "protobuf.protocol.GameclientMessageJoinParty"},								base+4BD900
	{0xA5, "protobuf.protocol.GameclientMessageRevokeInvitation"},						base+4BD900
	{0xA6, "protobuf.protocol.GameclientMessagePassLeadership"},						base+4BD900
	{0xA7, "protobuf.protocol.GameclientMessageLeaveParty"},							no payload
	{0xA8, "protobuf.protocol.GameclientMessageShareExperience"},						base+4BDF00
	{0xA9, "protobuf.protocol.GameclientMessage"},										placeholder
	{0xAA, "protobuf.protocol.GameclientMessageOpenChannel"},							no payload
	{0xAB, "protobuf.protocol.GameclientMessageInviteToChannel"},						base+4BDE50
	{0xAC, "protobuf.protocol.GameclientMessageExcludeFromChannel"},					base+4BDE50
	{0xAD, "protobuf.protocol.GameclientMessageCyclopediaHouseAction"},					base+4C0FB0
	{0xB1, "protobuf.protocol.GameclientMessageHighscores"},							base+4C1360
	{0xBA, "protobuf.protocol.GameclientMessagePreyHuntingTaskAction"},					base+4C12A0
	{0xBE, "protobuf.protocol.GameclientMessageCancel"},								no payload
	{0xC3, "protobuf.protocol.GameclientMessageClaimTournamentReward"},					base+4BCF80
	{0xC4, "protobuf.protocol.GameclientMessageTournamentInformation"},					no payload
	{0xC6, "protobuf.protocol.GameclientMessageSubscribeToUpdates"},					base+4BCFC0
	{0xC7, "protobuf.protocol.GameclientMessageTournamentLeaderboard"},					base+4BD030
	{0xC8, "protobuf.protocol.GameclientMessageTournamentTicketAction"},				base+4C0E10
	{0xC9, "protobuf.protocol.GameclientMessageGetTransactionDetails"},					base+4BD900
	{0xCA, "protobuf.protocol.GameclientMessageUpdateExivaOptions"},					base+4C0760
	{0xCB, "protobuf.protocol.GameclientMessageBrowseField"},							base+4BDC00
	{0xCC, "protobuf.protocol.GameclientMessageSeekInContainer"},						base+4BDAE0
	{0xCD, "protobuf.protocol.GameclientMessageInspectObject"},							base+4BD9B0
	{0xCE, "protobuf.protocol.GameclientMessageInspectPlayer"},							base+4BD930
	{0xCF, "protobuf.protocol.GameclientMessageBlessingsDialog"},						no payload
	{0xD0, "protobuf.protocol.GameclientMessageTrackQuestflags"},						base+4BFED0
	{0xD1, "protobuf.protocol.GameclientMessageMarketStatistics"},						no payload
	{0xD2, "protobuf.protocol.GameclientMessageGetOutfit"},								base+4BDF40
	{0xD3, "protobuf.protocol.GameclientMessageSetOutfit"},								base+4BDFE0
	{0xD4, "protobuf.protocol.GameclientMessageMount"},									base+4BDF00
	{0xD5, "protobuf.protocol.GameclientMessageApplyImbuement"},						base+4BF9C0
	{0xD6, "protobuf.protocol.GameclientMessageApplyClearingCharm"},					base+4BCF80
	{0xD7, "protobuf.protocol.GameclientMessageCloseImbuingDialog"},					no payload
	{0xD8, "protobuf.protocol.GameclientMessageOpenRewardWall"},						no payload
	{0xD9, "protobuf.protocol.GameclientMessageDailyRewardHistory"},					no payload
	{0xDA, "protobuf.protocol.GameclientMessageCollectDailyReward"},					base+4BFCB0
	{0xDB, "protobuf.protocol.GameclientMessageCyclopediaMapAction"},					base+4C0D40
	{0xDC, "protobuf.protocol.GameclientMessageAddBuddy"},								base+4BDDE0
	{0xDD, "protobuf.protocol.GameclientMessageRemoveBuddy"},							base+4BD900
	{0xDE, "protobuf.protocol.GameclientMessageEditBuddy"},								base+4BE2B0
	{0xDF, "protobuf.protocol.GameclientMessageBuddyGroup"},							base+4BE470
	{0xE0, "protobuf.protocol.GameclientMessageMarkGameNewsAsRead"},					base+4BE690
	{0xE1, "protobuf.protocol.GameclientMessageOpenMonsterCyclopedia"},					no payload
	{0xE2, "protobuf.protocol.GameclientMessageOpenMonsterCyclopediaMonsters"},			base+4BE6F0
	{0xE3, "protobuf.protocol.GameclientMessageOpenMonsterCyclopediaRace"},				base+4BDD60
	{0xE4, "protobuf.protocol.GameclientMessageMonsterBonusEffectAction"},				base+4BE820
	{0xE5, "protobuf.protocol.GameclientMessageOpenCyclopediaCharacterInfo"},			base+4BE8B0
	{0xE6, "protobuf.protocol.GameclientMessageBugReport"},								base+4BF6C0
	{0xE7, "protobuf.protocol.GameclientMessageThankYou"},								base+4BD900
	{0xE8, "protobuf.protocol.GameclientMessageGetOfferDescription"},					base+4BD900
	{0xE9, "protobuf.protocol.GameclientMessageStoreEvent"},							base+4BF850
	{0xEA, "protobuf.protocol.GameclientMessageFeatureEvent"},							base+4BFFA0
	{0xEB, "protobuf.protocol.GameclientMessagePreyAction"},							base+4BFA40
	{0xEC, "protobuf.protocol.GameclientMessageSetHirelingName"},						base+4BE1E0
	{0xED, "protobuf.protocol.GameclientMessageRequestResourceBalance"},				base+4BCF80
	{0xEE, "protobuf.protocol.GameclientMessageGreet"},									base+4BD900
	{0xEF, "protobuf.protocol.GameclientMessageTransferCurrency"},						base+4BF610
	{0xF0, "protobuf.protocol.GameclientMessageGetQuestLog"},							no payload
	{0xF1, "protobuf.protocol.GameclientMessageGetQuestLine"},							base+4BDD60
	{0xF2, "protobuf.protocol.GameclientMessageRuleViolationReport"},					base+4BEDE0
	{0xF3, "protobuf.protocol.GameclientMessageGetObjectInfo"},							base+4BFB10
	{0xF4, "protobuf.protocol.GameclientMessageMarketLeave"},							no payload
	{0xF5, "protobuf.protocol.GameclientMessageMarketBrowse"},							base+4BDD60
	{0xF6, "protobuf.protocol.GameclientMessageMarketCreate"},							base+4C0440
	{0xF7, "protobuf.protocol.GameclientMessageMarketCancel"},							base+4C0340
	{0xF8, "protobuf.protocol.GameclientMessageMarketAccept"},							base+4C03B0
	{0xF9, "protobuf.protocol.GameclientMessageAnswerModalDialog"},						base+4BED60
	{0xFA, "protobuf.protocol.GameclientMessageRequestStoreCategories"},				no payload
	{0xFB, "protobuf.protocol.GameclientMessageRequestStoreOffers"},					base+4BF0E0
	{0xFC, "protobuf.protocol.GameclientMessageBuyStoreOffer"},							base+4BF380
	{0xFD, "protobuf.protocol.GameclientMessageOpenTransactionHistory"},				base+4BCF80
	{0xFE, "protobuf.protocol.GameclientMessageGetTransactionHistory"},					base+4BF5B0
*/

/*
	QT Client inputmessage functions
	msg+0x10 - getU8()
	msg+0x18 - getU16()
	msg+0x1C - getU32()
	msg+0x20 - getU64()
	msg+0x24 - getS8()
	msg+0x28 - getS16()
	msg+0x2C - getS32()
	msg+0x30 - getS64()
	msg+0x34 - getString()
	msg+0x38 - getBytes(size)
	msg+0x40 - getBool()
	msg+0x44 - getDouble()
	
	QT Client(12.50.10109) Gameserver payload functions
	{0x03, "protobuf.protocol.GameserverMessageCreatureData"},							base+49FD40
	{0x04, "protobuf.protocol.GameserverMessageSessionDumpStart"},						base+495DC0
	{0x0A, "protobuf.protocol.GameserverMessagePendingStateEntered"},					no payload
	{0x0B, "protobuf.protocol.GameserverMessageReadyForSecondaryConnection"},			base+4AD3F0
	{0x0F, "protobuf.protocol.GameserverMessageWorldEntered"},							no payload
	{0x14, "protobuf.protocol.GameserverMessageLoginError"},							base+49FE50
	{0x15, "protobuf.protocol.GameserverMessageLoginAdvice"},							base+49FE50
	{0x16, "protobuf.protocol.GameserverMessageLoginWait"},								base+4A2590
	{0x17, "protobuf.protocol.GameserverMessageLoginSuccess"},							base+49EA60
	{0x19, "protobuf.protocol.GameserverMessageStoreButtonIndicators"},					base+4A26A0
	{0x1D, "protobuf.protocol.GameserverMessagePing"},									no payload
	{0x1E, "protobuf.protocol.GameserverMessagePingBack"},								no payload
	{0x1F, "protobuf.protocol.GameserverMessageLoginChallenge"},						base+49FDC0
	{0x28, "protobuf.protocol.GameserverMessageDead"},									base+4A2730
	{0x29, "protobuf.protocol.GameserverMessageStash"},									base+4B3560
	{0x2A, "protobuf.protocol.GameserverMessageSpecialContainersAvailable"},			base+4A26A0
	{0x2B, "protobuf.protocol.GameserverMessagePartyHuntAnalyser"},						base+4B6720
	{0x2C, "protobuf.protocol.GameserverMessageTeamFinderTeamLeader"},					base+4B8310
	{0x2D, "protobuf.protocol.GameserverMessageTeamFinderTeamMember"},					base+4B8600
	{0x63, "protobuf.protocol.GameserverMessageClientCheck"},							base+494CE0
	{0x64, "protobuf.protocol.GameserverMessageFullMap"},								base+4969D0
	{0x65, "protobuf.protocol.GameserverMessageTopRow"},								base+496B50
	{0x66, "protobuf.protocol.GameserverMessageRightColumn"},							base+496AF0
	{0x67, "protobuf.protocol.GameserverMessageBottomRow"},								base+496BB0
	{0x68, "protobuf.protocol.GameserverMessageLeftColumn"},							base+496A90
	{0x69, "protobuf.protocol.GameserverMessageFieldData"},								base+496C10
	{0x6A, "protobuf.protocol.GameserverMessageCreateOnMap"},							base+49DB00
	{0x6B, "protobuf.protocol.GameserverMessageChangeOnMap"},							base+49DD50
	{0x6C, "protobuf.protocol.GameserverMessageDeleteOnMap"},							base+49DF80
	{0x6D, "protobuf.protocol.GameserverMessageMoveCreature"},							base+49E120
	{0x6E, "protobuf.protocol.GameserverMessageContainer"},								base+4A2800
	{0x6F, "protobuf.protocol.GameserverMessageCloseContainer"},						base+4A09B0
	{0x70, "protobuf.protocol.GameserverMessageCreateInContainer"},						base+4A2BD0
	{0x71, "protobuf.protocol.GameserverMessageChangeInContainer"},						base+4A0C20
	{0x72, "protobuf.protocol.GameserverMessageDeleteInContainer"},						base+4A2BD0
	{0x74, "protobuf.protocol.GameserverMessageFriendSystemData"},						base+4B2250
	{0x75, "protobuf.protocol.GameserverMessageScreenshotEvent"},						base+4A09B0
	{0x76, "protobuf.protocol.GameserverMessageInspectionList"},						base+49FFD0
	{0x77, "protobuf.protocol.GameserverMessageInspectionState"},						base+4A08D0
	{0x78, "protobuf.protocol.GameserverMessageSetInventory"},							base+49FF20
	{0x79, "protobuf.protocol.GameserverMessageDeleteInventory"},						base+4A09B0
	{0x7A, "protobuf.protocol.GameserverMessageNPCOffer"},								base+4A2D00
	{0x7B, "protobuf.protocol.GameserverMessagePlayerGoods"},							base+4A32C0
	{0x7C, "protobuf.protocol.GameserverMessageCloseNPCTrade"},							no payload
	{0x7D, "protobuf.protocol.GameserverMessageOwnOffer"},								base+4A33D0
	{0x7E, "protobuf.protocol.GameserverMessageCounterOffer"},							base+4A33D0
	{0x7F, "protobuf.protocol.GameserverMessageCloseTrade"},							no payload
	{0x80, "protobuf.protocol.GameserverMessageCharacterTradeConfiguration"},			base+4B8BB0
	{0x82, "protobuf.protocol.GameserverMessageAmbientLight"},							base+49D3D0
	{0x83, "protobuf.protocol.GameserverMessageGraphicalEffects"},						base+49D460
	{0x84, "protobuf.protocol.GameserverMessageRemoveGraphicalEffect"},					base+49D8F8
	{0x8B, "protobuf.protocol.GameserverMessageCreatureUpdate"},						base+49EE20
	{0x8C, "protobuf.protocol.GameserverMessageCreatureHealth"},						base+49F1E0
	{0x8D, "protobuf.protocol.GameserverMessageCreatureLight"},							base+4A0D10
	{0x8E, "protobuf.protocol.GameserverMessageCreatureOutfit"},						base+49F2C0
	{0x8F, "protobuf.protocol.GameserverMessageCreatureSpeed"},							base+49F3D0
	{0x90, "protobuf.protocol.GameserverMessageCreatureSkull"},							base+49F500
	{0x91, "protobuf.protocol.GameserverMessageCreatureParty"},							base+49F5E0
	{0x92, "protobuf.protocol.GameserverMessageCreatureUnpass"},						base+49F6C0
	{0x93, "protobuf.protocol.GameserverMessageCreatureMarks"},							base+49F7A0
	{0x94, "protobuf.protocol.GameserverMessageDepotSearchResult"},						base+4B5570
	{0x95, "protobuf.protocol.GameserverMessageCreatureType"},							base+4A3550
	{0x96, "protobuf.protocol.GameserverMessageEditText"},								base+4A3620
	{0x97, "protobuf.protocol.GameserverMessageEditList"},								base+4A38F0
	{0x98, "protobuf.protocol.GameserverMessageShowGameNews"},							base+4A3A30
	{0x99, "protobuf.protocol.GameserverMessageDepotSearchDetailList"},					base+4B5660
	{0x9A, "protobuf.protocol.GameserverMessageCloseDepotSearch"},						no payload
	{0x9B, "protobuf.protocol.GameserverMessageBlessingsDialog"},						base+4A0E30
	{0x9C, "protobuf.protocol.GameserverMessageBlessings"},								base+4A14E0
	{0x9D, "protobuf.protocol.GameserverMessageSwitchPreset"},							base+4A3AC0
	{0x9E, "protobuf.protocol.GameserverMessagePremiumTrigger"},						base+4A1650
	{0x9F, "protobuf.protocol.GameserverMessagePlayerDataBasic"},						base+4A0A10
	{0xA0, "protobuf.protocol.GameserverMessagePlayerDataCurrent"},						base+49F870
	{0xA1, "protobuf.protocol.GameserverMessagePlayerSkills"},							base+4A1E20
	{0xA2, "protobuf.protocol.GameserverMessagePlayerState"},							base+4A1780
	{0xA3, "protobuf.protocol.GameserverMessageClearTarget"},							base+4A3AC0
	{0xA4, "protobuf.protocol.GameserverMessageSpellDelay"},							base+4A3B10
	{0xA5, "protobuf.protocol.GameserverMessageSpellGroupDelay"},						base+4A3B10
	{0xA6, "protobuf.protocol.GameserverMessageMultiUseDelay"},							base+4A3AC0
	{0xA7, "protobuf.protocol.GameserverMessageSetTactics"},							base+4A3BA0
	{0xA8, "protobuf.protocol.GameserverMessageSetStoreButtonDeeplink"},				base+4A1760
	{0xA9, "protobuf.protocol.GameserverMessageRestingAreaState"},						base+4AC5A0
	{0xAA, "protobuf.protocol.GameserverMessageTalk"},									base+49E300
	{0xAB, "protobuf.protocol.GameserverMessageChannels"},								base+4A3CB0
	{0xAC, "protobuf.protocol.GameserverMessageOpenChannel"},							base+4A1900
	{0xAD, "protobuf.protocol.GameserverMessagePrivateChannel"},						base+49FE50
	{0xAE, "protobuf.protocol.GameserverMessageEditGuildMessage"},						base+49FE50
	{0xB1, "protobuf.protocol.GameserverMessageHighscores"},							base+4B7470
	{0xB2, "protobuf.protocol.GameserverMessageOpenOwnChannel"},						base+4A1900
	{0xB3, "protobuf.protocol.GameserverMessageCloseChannel"},							base+4A3E70
	{0xB4, "protobuf.protocol.GameserverMessageMessage"},								base+49E680
	{0xB5, "protobuf.protocol.GameserverMessageSnapBack"},								base+4A09B0
	{0xB6, "protobuf.protocol.GameserverMessageWait"},									base+4A3E70
	{0xB7, "protobuf.protocol.GameserverMessageUnjustifiedPoints"},						base+4A1D90
	{0xB8, "protobuf.protocol.GameserverMessagePvpSituations"},							base+4A09B0
	{0xB9, "protobuf.protocol.GameserverMessageBestiaryTracker"},						base+4A2320
	{0xBA, "protobuf.protocol.GameserverMessagePreyHuntingTaskBaseData"},				base+4B6BF0
	{0xBB, "protobuf.protocol.GameserverMessagePreyHuntingTaskData"},					base+4B7190
	{0xBC, "protobuf.protocol.GameserverMessageStop"},									no payload
	{0xBE, "protobuf.protocol.GameserverMessageTopFloor"},								base+49DA20
	{0xBF, "protobuf.protocol.GameserverMessageBottomFloor"},							base+49DA90
	{0xC0, "protobuf.protocol.GameserverMessageUpdateLootContainers"},					base+4AD900
	{0xC1, "protobuf.protocol.GameserverMessagePlayerDataTournament"},					base+4B3680
	{0xC3, "protobuf.protocol.GameserverMessageCyclopediaHouseActionResult"},			base+4B6650
	{0xC4, "protobuf.protocol.GameserverMessageTournamentInformation"},					base+4B3F30
	{0xC5, "protobuf.protocol.GameserverMessageTournamentLeaderboard"},					base+4B3760
	{0xC6, "protobuf.protocol.GameserverMessageCyclopediaStaticHouseData"},				base+4B5B20
	{0xC7, "protobuf.protocol.GameserverMessageCyclopediaCurrentHouseData"},			base+4B5F30
	{0xC8, "protobuf.protocol.GameserverMessageOutfit"},								base+4A3ED0
	{0xC9, "protobuf.protocol.GameserverMessageExivaSuppressed"},						no payload
	{0xCA, "protobuf.protocol.GameserverMessageUpdateExivaOptions"},					base+4B16F0
	{0xCB, "protobuf.protocol.GameserverMessageTransactionDetails"},					base+4A49B0
	{0xCC, "protobuf.protocol.GameserverMessageImpactTracking"},						base+4A3B10
	{0xCD, "protobuf.protocol.GameserverMessageMarketStatistics"},						base+4A5180
	{0xCE, "protobuf.protocol.GameserverMessageItemWasted"},							base+4A3E70
	{0xCF, "protobuf.protocol.GameserverMessageItemLooted"},							base+4A47A0
	{0xD0, "protobuf.protocol.GameserverMessageTrackQuestflags"},						base+4AD510
	{0xD1, "protobuf.protocol.GameserverMessageKillTracking"},							base+4A48B0
	{0xD2, "protobuf.protocol.GameserverMessageBuddyData"},								base+4A5260
	{0xD3, "protobuf.protocol.GameserverMessageBuddyStatusChange"},						base+4A5730
	{0xD4, "protobuf.protocol.GameserverMessageBuddyGroupData"},						base+4A5840
	{0xD5, "protobuf.protocol.GameserverMessageMonsterCyclopedia"},						base+4A5A80
	{0xD6, "protobuf.protocol.GameserverMessageMonsterCyclopediaMonsters"},				base+4A5C90
	{0xD7, "protobuf.protocol.GameserverMessageMonsterCyclopediaRace"},					base+4A5E90
	{0xD8, "protobuf.protocol.GameserverMessageMonsterCyclopediaBonusEffects"},			base+4A6950
	{0xD9, "protobuf.protocol.GameserverMessageMonsterCyclopediaNewDetails"},			base+4A3E70
	{0xDA, "protobuf.protocol.GameserverMessageCyclopediaCharacterInfo"},				base+4A6F90
	{0xDB, "protobuf.protocol.GameserverMessageHirelingNameChange"},					base+4A8CF0
	{0xDC, "protobuf.protocol.GameserverMessageTutorialHint"},							base+4A09B0
	{0xDD, "protobuf.protocol.GameserverMessageCyclopediaMapData"},						base+4A8D70
	{0xDE, "protobuf.protocol.GameserverMessageDailyRewardCollectionState"},			base+4AD4B0
	{0xDF, "protobuf.protocol.GameserverMessageCreditBalance"},							base+4B1190
	{0xE0, "protobuf.protocol.GameserverMessageStoreError"},							base+4AC3A0
	{0xE1, "protobuf.protocol.GameserverMessageRequestPurchaseData"},					base+4B0510
	{0xE2, "protobuf.protocol.GameserverMessageOpenRewardWall"},						base+4AC6F0
	{0xE3, "protobuf.protocol.GameserverMessageCloseRewardWall"},						no payload
	{0xE4, "protobuf.protocol.GameserverMessageDailyRewardBasic"},						base+4AC970
	{0xE5, "protobuf.protocol.GameserverMessageDailyRewardHistory"},					base+4AD380
	{0xE6, "protobuf.protocol.GameserverMessagePreyFreeListRerollAvailability"},		base+4AAFF0
	{0xE7, "protobuf.protocol.GameserverMessagePreyTimeLeft"},							base+4AAFF0
	{0xE8, "protobuf.protocol.GameserverMessagePreyData"},								base+4AB080
	{0xE9, "protobuf.protocol.GameserverMessagePreyPrices"},							base+4AB650
	{0xEA, "protobuf.protocol.GameserverMessageOfferDescription"},						base+4AB810
	{0xEB, "protobuf.protocol.GameserverMessageImbuingDialogRefresh"},					base+4AB920
	{0xEC, "protobuf.protocol.GameserverMessageCloseImbuingDialog"},					no payload
	{0xED, "protobuf.protocol.GameserverMessageShowMessageDialog"},						base+4AC3A0
	{0xEE, "protobuf.protocol.GameserverMessageResourceBalance"},						base+4AC4B0
	{0xEF, "protobuf.protocol.GameserverMessageTibiaTime"},								base+49D3D0
	{0xF0, "protobuf.protocol.GameserverMessageQuestLog"},								base+4A9A80
	{0xF1, "protobuf.protocol.GameserverMessageQuestLine"},								base+4A9C90
	{0xF2, "protobuf.protocol.GameserverMessageUpdatingShopBalance"},					base+4B1360
	{0xF3, "protobuf.protocol.GameserverMessageChannelEvent"},							base+4A9F60
	{0xF4, "protobuf.protocol.GameserverMessageObjectInfo"},							base+4AA0B0
	{0xF5, "protobuf.protocol.GameserverMessagePlayerInventory"},						base+4AA280
	{0xF6, "protobuf.protocol.GameserverMessageMarketEnter"},							base+4AA390
	{0xF7, "protobuf.protocol.GameserverMessageMarketLeave"},							no payload
	{0xF8, "protobuf.protocol.GameserverMessageMarketDetail"},							base+4AA4E0
	{0xF9, "protobuf.protocol.GameserverMessageMarketBrowse"},							base+4ADA20
	{0xFA, "protobuf.protocol.GameserverMessageShowModalDialog"},						base+4AE350
	{0xFB, "protobuf.protocol.GameserverMessageStoreCategories"},						base+4AE9A0
	{0xFC, "protobuf.protocol.GameserverMessageStoreOffers"},							base+4AEE60
	{0xFD, "protobuf.protocol.GameserverMessageTransactionHistory"},					base+4B13B0
	{0xFE, "protobuf.protocol.GameserverMessageStoreSuccess"},							base+4AC3A0
*/

class Thing;
class Creature;
class Item;
class ItemUI;
class Tile;
class ProtocolGame : public Protocol
{
	public:
		ProtocolGame() = default;

		// non-copyable
		ProtocolGame(const ProtocolGame&) = delete;
		ProtocolGame& operator=(const ProtocolGame&) = delete;

		// non-moveable
		ProtocolGame(ProtocolGame&&) = delete;
		ProtocolGame& operator=(ProtocolGame&&) = delete;

		virtual ProtocolGame* getProtocolGame() {return this;}
		virtual const ProtocolGame* getProtocolGame() const {return this;}

		bool canPerformAction() {return true;}

		virtual void parseMessage(InputMessage& msg);
		virtual void onConnect();
		virtual void onConnectionError(ConnectionError error);
		virtual void onDisconnect();

		//Main Functions
		void sendServerName(const std::string& serverName);
		void sendLogin(Uint32 challengeTimestamp, Uint8 challengeRandom);
		void sendSecondaryLogin();
		void sendEnterGame();
		void sendLogout();
		void sendPing();
		void sendPingBack();
		void sendConnectionPingBack();
		void sendUpdateTile(const Position& position);

		//Walking Events
		void sendAutoWalk(const std::vector<Direction>& path);
		void sendWalk(Direction dir);
		void sendTurnNorth();
		void sendTurnEast();
		void sendTurnSouth();
		void sendTurnWest();

		//Item Move Events
		void sendEquipItem(Uint16 itemid, Uint16 count);
		void sendMove(const Position& fromPos, Uint16 itemid, Uint8 stackpos, const Position& toPos, Uint16 count);

		//NPC Trade System
		void sendLookInShop(Uint16 itemid, Uint16 count);
		void sendNPCPurchase(Uint16 itemid, Uint8 subtype, Uint16 amount, bool ignoreCapacity, bool buyWithBackpack);
		void sendNPCSell(Uint16 itemid, Uint8 subtype, Uint16 amount, bool ignoreEquipped);
		void sendNPCClose();

		//Trade System
		void sendRequestTrade(const Position& position, Uint16 itemId, Uint8 stackpos, Uint32 creatureId);
		void sendLookInTrade(bool counterOffer, Uint8 index);
		void sendAcceptTrade();
		void sendCloseTrade();

		//Use Item Events
		void sendUseItem(const Position& position, Uint16 itemId, Uint8 stackpos, Uint8 index);
		void sendUseItemEx(const Position& fromPos, Uint16 itemId, Uint8 fromStackPos, const Position& toPos, Uint16 toItemId, Uint8 toStackPos);
		void sendUseOnCreature(const Position& position, Uint16 itemId, Uint8 stackpos, Uint32 creatureId);
		void sendRotateItem(const Position& position, Uint16 itemId, Uint8 stackpos);
		void sendRequestItemInfo(std::vector<RequestItems>& requestItems);
		void sendWrapState(const Position& position, Uint16 itemId, Uint8 stackpos);

		//Container System
		void sendCloseContainer(Uint8 containerId);
		void sendUpContainer(Uint8 containerId);
		void sendUpdateContainer(Uint8 containerId);
		void sendSeekInContainer(Uint8 containerId, Uint16 index);
		void sendBrowseField(const Position& position);
		void sendOpenParentContainer(const Position& position);
		void sendOpenDepotSearch();
		void sendCloseDepotSearch();
		void sendDepotSearchType(Uint16 itemId);
		void sendDepotSearchRetrieve(Uint16 itemId, Uint8 itemCount);
		void sendStashAction(Sint8 unk1);

		//Text Windows Events
		void sendTextWindow(Uint32 windowId, const std::string& text);
		void sendHouseWindow(Uint32 windowId, Uint8 doorId, const std::string& text);

		//Looks Events
		void sendLookAt(const Position& position, Uint16 thingId, Uint8 stackpos);
		void sendLookInBattle(Uint32 creatureId);

		//Talk Event
		void sendSay(MessageMode mode, Uint16 channelId, const std::string& receiver, const std::string& message);
		void sendUpdateExivaOptions();

		//Chat System
		void sendRequestChannels();
		void sendOpenChannel(Uint16 channelId);
		void sendCloseChannel(Uint16 channelId);
		void sendOpenPrivateChannel(const std::string& receiver);
		void sendOpenRuleViolationChannel(const std::string& reporter);
		void sendCloseRuleViolationChannel(const std::string& reporter);
		void sendCancelRuleViolationChannel();
		void sendCloseNPCChannel();
		void sendCreatePrivateChannel();
		void sendChannelInvite(const std::string& target, Uint16 channelId);
		void sendChannelExclude(const std::string& target, Uint16 channelId);

		//Fight System
		void sendAttackModes(Uint8 attackMode, Uint8 chaseMode, Uint8 secureMode, Uint8 pvpMode);
		void sendAttack(Uint32 creatureId, Uint32 sequence);
		void sendFollow(Uint32 creatureId, Uint32 sequence);
		void sendCancelAttackAndFollow();

		//PvP System
		void sendJoinAggression(Uint32 creatureId);

		//Party System
		void sendInviteToParty(Uint32 creatureId);
		void sendJoinToParty(Uint32 creatureId);
		void sendRevokePartyInvitation(Uint32 creatureId);
		void sendPassPartyLeadership(Uint32 creatureId);
		void sendLeaveParty();
		void sendEnableSharedPartyExperience(bool active);
		
		//Outfit Events
		void sendRequestOutfit();
		void sendSetOutfit(Uint16 lookType, Uint8 lookHead, Uint8 lookBody, Uint8 lookLegs, Uint8 lookFeet, Uint8 lookAddons, Uint16 lookMount);
		void sendMount(bool active);

		//Vip System
		void sendAddVip(const std::string& name);
		void sendRemoveVip(Uint32 playerGUID);
		void sendEditVip(Uint32 playerGUID, const std::string& description, Uint32 iconId, bool notifyLogin, std::vector<Uint8>& groupIds);
		void sendVipGroupAction(VipGroupActions groupAction, Uint8 groupId, const std::string& name);
		void sendFriendSystemAction();

		//Reports System
		void sendNewBugReport(BugReportCategory category, const std::string& comment, const std::string& typoSpeaker, const std::string& typoText);
		void sendBugReport(const std::string& comment);
		void sendRuleViolation(const std::string& target, Uint8 reason, Uint8 action, const std::string& comment, const std::string& statement, Uint16 statementId, bool ipBanishment);
		void sendNewRuleViolation(Uint8 reason, Uint8 action, const std::string& characterName, const std::string& comment, const std::string& translation, Uint32 statementId);
		void sendDebugReport(const std::string& line, const std::string& date, const std::string& decription, const std::string& comment);

		//Questlog System
		void sendShowQuestLog();
		void sendShowQuestLine(Uint16 questId);

		//Modal Dialog Window
		void sendAnswerModalDialog(Uint32 dialogId, Uint8 button, Uint8 choice);

		//Store System
		void sendOpenStore(Uint8 serviceType, const std::string& category);
		void sendRequestStore(Uint8 serviceType, const std::string& category);
		void sendBuyInStore(Uint32 offerId, StoreProductTypes productType, const std::string& name);
		void sendOpenTransactionHistory(Uint8 entriesPerPage);
		void sendRequestTransactionHistory(Uint32 page, Uint8 entriesPerPage);
		void sendTransferCoins(const std::string& recipient, Uint32 amount);
		void sendGetTransactionDetails(Uint32 unknown);
		void sendStoreEvent();

		//Market System
		void sendMarketLeave();
		void sendMarketBrowse(Uint16 browseId);
		void sendMarketCreateOffer(Uint8 type, Uint16 thingId, Uint16 amount, Uint32 price, bool anonymous);
		void sendMarketCancelOffer(Uint32 timestamp, Uint16 counter);
		void sendMarketAcceptOffer(Uint32 timestamp, Uint16 counter, Uint16 amount);

		//Analytics System
		void sendTrackQuestFlags(std::vector<Uint16>& missionIds);
		void sendOpenCyclopediaCharacterInfo(CyclopediaCharacterInfoType characterInfoType);
		void sendTrackBestiaryRace();
		void sendPartyHuntAnalyser();
		void sendFeatureEvent(FeatureEventType type, bool active);
		void sendTeamFinderAssembleTeam();
		void sendTeamFinderJoinTeam();

		//Highscores
		void sendHighscores();

		//Inspection System
		void sendInspectPlayer(InspectPlayerState playerState, Uint32 playerGUID = 0);
		void sendInspectObject(InspectObjectData& inspectData);

		//Imbuing System
		void sendApplyImbuement(Uint8 slot, Uint32 imbuementId, bool protectiveCharm);
		void sendClearCharm(Uint8 slot);
		void sendImbuementLeave();

		//Prey system
		void sendPreyAction(Uint8 preyId, PreyAction action, Uint8 listIndex);

		//Cyclopedia System
		void sendMarketStatistics();
		void sendOpenMonsterCyclopedia();
		void sendOpenMonsterCyclopediaMonsters(const std::string& race);
		void sendOpenMonsterCyclopediaRace(Uint16 raceId);
		void sendCyclopediaMapAction();
		void sendMonsterBonusEffectAction(CyclopediaBonusEffectAction action, Uint8 charmId, Uint16 raceId);
		void sendSetHirelingName();
		void sendCyclopediaHouseAction();
		void sendPreyHuntingTaskAction();

		//Reward Wall System
		void sendOpenRewardWall();
		void sendOpenRewardHistory();
		void sendColectReward(bool shrine, std::vector<RewardCollection>& rewards);

		//Quick Loot System
		void sendQuickLoot(const Position& position, Uint16 itemId, Uint8 stackpos);
		void sendLootContainer(LootContainerData& lootData);
		void sendQuickLootBlackWhitelist(QuickLootFilter filter, std::vector<Uint16>& itemIds);

		//Compedium System
		void sendSubscribeToUpdates(bool unknownB, Uint8 unknownU);
		void sendMarkGameNewsAsRead(Uint32 newsId, bool readed);

		//Tournament System
		void sendClaimTournamentReward(Uint8 rewardId);
		void sendTournamentInformation();
		void sendTournamentLeaderboard(const std::string& worldName, Uint16 currentPage, const std::string& searchCharacter, Uint8 elementsPerPage = 20);
		void sendTournamentTicketAction();

		//Character Auction System
		void sendCharacterTradeConfigurationAction();

		//Some Ingame Stuff Sends
		void sendThankYou(Uint32 statementId);
		void sendGetOfferDescription(Uint32 offerId);
		void sendRequestResourceBalance(ResourceBalanceTypes resource);
		void sendGreet(Uint32 statementId);
		void sendGuildMessage();
		void sendEditGuildMessage(const std::string& message);
		void sendBlessingsDialog();
		void sendClientCheck(std::vector<Uint8>& data);
		void sendTeleport(const Position& position);

	private:
		MessageMode translateMessageModeFromServer(Uint8 mode);
		Uint8 translateMessageModeToServer(MessageMode mode);

		//Main Functions
		void parseOtclient(InputMessage& msg);
		void parseOtclientV8LuaPacket(InputMessage& msg, Uint8 opcode);
		void parseServerFeatures(InputMessage& msg);
		void parseCreatureData(InputMessage& msg);
		void parseSessionDumpStart(InputMessage& msg);
		void parseLogin(InputMessage& msg);
		void parseLoginOrPending(InputMessage& msg);
		void parseGMActions(InputMessage& msg);
		void parseReadyForSecondaryConnection(InputMessage& msg);
		void parseEnterGame(InputMessage& msg);
		void parseUpdateNeeded(InputMessage& msg);
		void parseLoginError(InputMessage& msg);
		void parseLoginAdvice(InputMessage& msg);
		void parseLoginWait(InputMessage& msg);
		void parseLoginToken(InputMessage& msg);
		void parseStoreButtonIndicators(InputMessage& msg);
		void parsePingBack(InputMessage& msg);
		void parsePing(InputMessage& msg);
		void parseChallenge(InputMessage& msg);
		void parseDeath(InputMessage& msg);
		void parseClientCheck(InputMessage& msg);
		void parseTime(InputMessage& msg);
		void parseScreenshotEvent(InputMessage& msg);

		//Map Parser
		void parseMapDescription(InputMessage& msg);
		void parseMapNorth(InputMessage& msg);
		void parseMapEast(InputMessage& msg);
		void parseMapSouth(InputMessage& msg);
		void parseMapWest(InputMessage& msg);
		void parseUpdateTile(InputMessage& msg);
		void parseTileAddThing(InputMessage& msg);
		void parseTileTransformThing(InputMessage& msg);
		void parseTileRemoveThing(InputMessage& msg);
		void parseTileMoveCreature(InputMessage& msg);
		void parseFloorChangeUp(InputMessage& msg);
		void parseFloorChangeDown(InputMessage& msg);

		//Container Parser
		void parseContainerOpen(InputMessage& msg);
		void parseContainerClose(InputMessage& msg);
		void parseContainerAddItem(InputMessage& msg);
		void parseContainerTransformItem(InputMessage& msg);
		void parseContainerRemoveItem(InputMessage& msg);
		void parseStash(InputMessage& msg);
		void parseSpecialContainersAvailable(InputMessage& msg);
		void parseDepotSearchResult(InputMessage& msg);
		void parseDepotSearchDetailList(InputMessage& msg);
		void parseCloseDepotSearch(InputMessage& msg);

		//Inspection Parser
		void parseInspectionList(InputMessage& msg);
		void parseInspectionState(InputMessage& msg);

		//Inventory Parser
		void parseInventoryTransformItem(InputMessage& msg);
		void parseInventoryRemoveItem(InputMessage& msg);

		//Npc Trade Parser
		void parseNpcOpenTrade(InputMessage& msg);
		void parseNpcPlayerGoods(InputMessage& msg);
		void parseNpcCloseTrade(InputMessage& msg);

		//Trade Parser
		void parseTradeOwn(InputMessage& msg);
		void parseTradeCounter(InputMessage& msg);
		void parseTradeClose(InputMessage& msg);

		//World Light Parser
		void parseAmbientWorldLight(InputMessage& msg);

		//Effects Pareser
		void parseMagicEffects(InputMessage& msg);
		void parseMagicEffect(InputMessage& msg);
		void parseRemoveMagicEffect(InputMessage& msg);
		void parseAnimatedText(InputMessage& msg);
		void parseDistanceEffect(InputMessage& msg);

		//Creature Parser
		void parseCreatureMark(InputMessage& msg);
		void parseCreatureTrappers(InputMessage& msg);
		void parseCreatureUpdate(InputMessage& msg);
		void parseCreatureHealth(InputMessage& msg);
		void parseCreatureLight(InputMessage& msg);
		void parseCreatureOutfit(InputMessage& msg);
		void parseCreatureSpeed(InputMessage& msg);
		void parseCreatureSkull(InputMessage& msg);
		void parseCreatureParty(InputMessage& msg);
		void parseCreatureUnpassable(InputMessage& msg);
		void parseCreatureMarks(InputMessage& msg);
		void parseCreatureHelpers(InputMessage& msg);
		void parseCreatureType(InputMessage& msg);

		//Text Window Parser
		void parseEditTextWindow(InputMessage& msg);
		void parseEditHouseWindow(InputMessage& msg);

		//Some Ingame Stuff Parser
		void parseBlessingsDialog(InputMessage& msg);
		void parseBlessings(InputMessage& msg);
		void parsePreset(InputMessage& msg);
		void parsePremiumTrigger(InputMessage& msg);
		void parseTextMessage(InputMessage& msg);
		void parseCancelWalk(InputMessage& msg);
		void parseWalkWait(InputMessage& msg);
		void parseUnjustifiedStats(InputMessage& msg);
		void parsePvpSituations(InputMessage& msg);
		void parseUpdateLootContainers(InputMessage& msg);
		void parseChooseOutfit(InputMessage& msg);
		void parseExivaSuppressed(InputMessage& msg);
		void parseUpdateExivaOptions(InputMessage& msg);
		void parseTutorialHint(InputMessage& msg);
		void parseAutomapFlag(InputMessage& msg);
		void parseModalWindow(InputMessage& msg);
		void parseShowMessageDialog(InputMessage& msg);
		void parseOfferDescription(InputMessage& msg);
		void parseEditGuildMessage(InputMessage& msg);

		//Player Parser
		void parsePlayerDataBasic(InputMessage& msg);
		void parsePlayerData(InputMessage& msg);
		void parsePlayerSkills(InputMessage& msg);
		void parsePlayerIcons(InputMessage& msg);
		void parsePlayerCancelTarget(InputMessage& msg);
		void parsePlayerSpellDelay(InputMessage& msg);
		void parsePlayerSpellGroupDelay(InputMessage& msg);
		void parsePlayerMultiUseDelay(InputMessage& msg);
		void parsePlayerModes(InputMessage& msg);
		void parsePlayerStoreDeepLink(InputMessage& msg);
		void parsePlayerTalk(InputMessage& msg);
		void parseItemInfo(InputMessage& msg);
		void parsePlayerInventory(InputMessage& msg);
		void parseRestingAreaState(InputMessage& msg);

		//Chat Parser
		void parseChannels(InputMessage& msg);
		void parseOpenChannel(InputMessage& msg);
		void parseOpenPrivateChannel(InputMessage& msg);
		void parseRuleViolationChannel(InputMessage& msg);
		void parseRuleViolationRemove(InputMessage& msg);
		void parseRuleViolationCancel(InputMessage& msg);
		void parseRuleViolationLock(InputMessage& msg);
		void parseOpenOwnChannel(InputMessage& msg);
		void parseCloseChannel(InputMessage& msg);
		void parseChannelEvent(InputMessage& msg);

		//Highscore Parser
		void parseHighscores(InputMessage& msg);

		//Vip Parser
		void parseVipAdd(InputMessage& msg);
		void parseVipStatus(InputMessage& msg);
		void parseVipStatusLogout(InputMessage& msg);
		void parseFriendSystemData(InputMessage& msg);

		//Quest Parser
		void parseQuestLog(InputMessage& msg);
		void parseQuestLine(InputMessage& msg);
		void parseTrackedQuestFlags(InputMessage& msg);

		//Market Parser
		void parseMarketStatistics(InputMessage& msg);
		void parseMarketEnter(InputMessage& msg);
		void parseMarketLeave(InputMessage& msg);
		void parseMarketDetail(InputMessage& msg);
		void parseMarketBrowse(InputMessage& msg);

		//Store Parser
		void parseStoreCoinBalance(InputMessage& msg);
		void parseStoreError(InputMessage& msg);
		void parseStoreRequestPurchase(InputMessage& msg);
		void parseStoreCoinBalanceUpdating(InputMessage& msg);
		void parseStoreOpen(InputMessage& msg);
		void parseStoreOffers(InputMessage& msg);
		void parseStoreTransactionHistory(InputMessage& msg);
		void parseStoreCompletePurchase(InputMessage& msg);
		void parseTransactionDetails(InputMessage& msg);
		
		//Analytics Parser
		void parseImpactTracking(InputMessage& msg);
		void parseItemWasted(InputMessage& msg);
		void parseItemLooted(InputMessage& msg);
		void parseKillTracking(InputMessage& msg);
		void parsePartyHuntAnalyser(InputMessage& msg);
		void parseBestiaryTracker(InputMessage& msg);
		void parsePreyHuntingTaskBaseData(InputMessage& msg);
		void parsePreyHuntingTaskData(InputMessage& msg);

		//Social Parser
		void parseTeamFinderTeamLeader(InputMessage& msg);
		void parseTeamFinderTeamMember(InputMessage& msg);

		//Imbuing Parser
		void parseRefreshImbuingDialog(InputMessage& msg);
		void parseCloseImbuingDialog(InputMessage& msg);
		void parseResourceBalance(InputMessage& msg);

		//Prey Parser
		void parsePreyFreeListRerollAvailability(InputMessage& msg);
		void parsePreyTimeLeft(InputMessage& msg);
		void parsePreyData(InputMessage& msg);
		void parsePreyPrices(InputMessage& msg);

		//Cyclopedia Parser
		void parseMonsterCyclopedia(InputMessage& msg);
		void parseMonsterCyclopediaMonsters(InputMessage& msg);
		void parseMonsterCyclopediaRace(InputMessage& msg);
		void parseMonsterCyclopediaBonusEffects(InputMessage& msg);
		void parseMonsterCyclopediaNewDetails(InputMessage& msg);
		void parseCyclopediaCharacterInfo(InputMessage& msg);
		void parseHirelingNameChange(InputMessage& msg);
		void parseCyclopediaMapData(InputMessage& msg);
		void parseCyclopediaHouseActionResult(InputMessage& msg);
		void parseCyclopediaStaticHouseData(InputMessage& msg);
		void parseCyclopediaCurrentHouseData(InputMessage& msg);

		//Reward Wall Parser
		void parseOpenRewardWall(InputMessage& msg);
		void parseCloseRewardWall(InputMessage& msg);
		void parseDailyRewardBasic(InputMessage& msg);
		void parseDailyRewardHistory(InputMessage& msg);
		void parseDailyRewardCollectionState(InputMessage& msg);
		
		//Compedium Parser
		void parseGameNews(InputMessage& msg);

		//Tournament Parser
		void parsePlayerDataTournament(InputMessage& msg);
		void parseTournamentInformation(InputMessage& msg);
		void parseTournamentLeaderboard(InputMessage& msg);

		//Character Auction Parser
		void parseCharacterTradeConfigurationLight(InputMessage& msg);

		Thing* internalGetThing(InputMessage& msg, const Position& pos);
		Creature* getCreature(InputMessage& msg, Uint16 thingId, const Position& pos);
		Item* getItem(InputMessage& msg, Uint16 thingId, const Position& pos);
		ItemUI* getItemUI(InputMessage& msg, Uint16 thingId);

		void setMapDescription(InputMessage& msg, Sint32 x, Sint32 y, Uint8 z, Sint32 width, Sint32 height);
		Sint32 setFloorDescription(InputMessage& msg, Sint32 x, Sint32 y, Uint8 z, Sint32 width, Sint32 height, Sint32 offset, Sint32 skip);
		Sint32 setTileDescription(InputMessage& msg, const Position& position, Sint32 offset);
		void updateMinimapTile(const Position& position, Tile* tile);

		Uint32 m_expectChannels = 0;
		Uint32 m_pingReceived = 0;
		Uint32 m_pingSent = 0;
		Uint32 m_pingTime = 0;

		ConnectionError m_lastError = CONNECTION_ERROR_INVALID;
		bool m_skipErrors = false;
		bool m_gameInitialized = false;
		bool m_pendingGame = false;
};

#endif /* __FILE_PROTOCOL_GAME_h_ */
