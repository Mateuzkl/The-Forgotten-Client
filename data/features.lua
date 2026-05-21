-- WARNING:
-- This file controls real protocol packet parsing.
-- Use only true/false values. Restart the client after changing this file.
--
-- true  = the client reads/sends this protocol feature and shows its UI.
-- false = the client does not read/send this protocol feature and hides its UI.
--
-- Important:
-- If your server still sends bytes for a feature that you set to false, packets can
-- desync and the client can freeze, debug, or disconnect. The client and server
-- must agree.
--
-- This profile is for Mateus TFS 1.8 downgrade 8.60.
-- Do not change protocol/clientVersion/fileVersion to 870 for this server.

ClientFeatures = {
    selectedProfile = "tfs860_custom",

    default = {
        protocol = 860,
        host = "127.0.0.1",
        port = 7171,
    },

    serverProfiles = {
        ["127.0.0.1:7171"] = "tfs860_custom",
        ["localhost:7171"] = "tfs860_custom",
    },

    profiles = {
        tfs860_custom = {
            description = "TFS 1.8 downgrade 8.60 custom protocol",
            protocol = 860,
            clientVersion = 860,
            fileVersion = 860,

            -- false below really disables the feature. The server must match.
            inheritDefaults = false,

            features = {
                -- Core 8.60 protocol.
                GameUpdateTile = false,
                GameLoginPacketEncryption = true,
                GameRSA1024 = true,
                GameMessageStatements = true,
                GameLooktypeU16 = true,
                GameNewFluids = true,
                GameMessageLevel = true,
                GamePlayerStateU16 = true,
                GamePlayerAddons = true,
                GamePlayerStamina = true,
                GameNewOutfitProtocol = true,
                GameWritableDate = true,
                GameNpcInterface = true,
                GameProtocolChecksum = true,
                GameAccountNames = true,
                GameChallengeOnLogin = true,
                GameDoubleFreeCapacity = true,
                GameTileAddThingWithStackpos = true,
                GameCreatureEmblems = true,
                GameAttackSeq = true,

                -- Your TFS 1.8 custom 8.60 features.
                -- Example: setting GamePlayerMounts = false hides Mount/Dismount
                -- and blocks the server feature packet from enabling mounts again.
                GamePlayerMounts = true,
                GameMagicEffectU16 = true,
                GameDistanceEffectU16 = true,
                GameDoubleHealth = true,
                GameOfflineTrainingTime = true,
                GameSkillsBase = true,
                GameBaseSkillU16 = true,
                GameDoubleSkills = true,
                GameAdditionalSkills = true,
                GameIdleAnimations = true,
                GameEnhancedAnimations = true,
                GameExtendedClientPing = true,
                GameSpritesU32 = true,
                GameDoublePlayerGoodsMoney = true,
                GameCreatureIcons = true,
                GamePurseSlot = true,
                GamePrey = true,
                GameThingUpgradeClassification = true,

                -- Keep these false unless your server code really sends them.
                GameDeathPenalty = false,
                GameDoubleExperience = false,
                GameSpellList = false,
                GameChatPlayerList = false,
                GameTotalCapacity = false,
                GameRegenerationTime = false,
                GameItemAnimationPhases = false,
                GameEnvironmentEffects = false,
                GameNpcNameOnTrade = false,
                GamePlayerMarket = false,
                GamePing = false,
                GameDoubleShopBuyAmount = false,
                GameDoubleShopSellAmount = false,
                GameMinimapRemoveMark = false,
                GameItemsU16 = false,
                GameLookAtCreature = false,
                GameAdditionalVipInfo = false,
                GamePreviewState = false,
                GameClientVersion = false,
                GameKeepConnectionAfterDeath = false,
                GameLoginPending = false,
                GameVipStatus = false,
                GameNewSpeedLaw = false,
                GameBrowseField = false,
                GameContainerPagination = false,
                GamePVPMode = false,
                GameThingMarks = false,
                GameCreatureType = false,
                GameHideNpcNames = false,
                GamePremiumExpiration = false,
                GameUnjustifiedPoints = false,
                GameExperienceBonus = false,
                GameDeathType = false,
                GameOGLInformation = false,
                GameContentRevision = false,
                GameAuthenticator = false,
                GameSessionKey = false,
                GameEquipHotkey = false,
                GameIngameStore = false,
                GameWrappable = false,
                GameStoreInbox = false,
                GameIngameStoreServiceType = false,
                GameIngameStoreHighlights = false,
                GameDetailedExperienceBonus = false,
                GameStoreHighlights2 = false,
                GameNewFilesStructure = false,
                GameClientSendServerName = false,
                GameImbuing = false,
                GameVipGroups = false,
                GameInspection = false,
                GameSequencedPackets = false,
                GameBlessDialog = false,
                GameQuestTracker = false,
                GameTime = false,
                GameCompedium = false,
                GamePlayerStateU32 = false,
                GameRewardWall = false,
                GameAnalytics = false,
                GameQuickLoot = false,
                GameExtendedCapacity = false,
                GameCyclopediaMonsters = false,
                GameStash = false,
                GameCyclopediaMapAndDetails = false,
                GameDoublePercentSkills = false,
                GameTournaments = false,
                GameAccountEmail = false,
            }
        },
    }
}
