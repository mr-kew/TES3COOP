#ifndef OPENMW_BASEPLAYER_HPP
#define OPENMW_BASEPLAYER_HPP

#include <cstdint>

#include <components/esm/loadcell.hpp>
#include <components/esm/loadcrea.hpp>
#include <components/esm/loadnpc.hpp>
#include <components/esm/npcstats.hpp>
#include <components/esm/creaturestats.hpp>
#include <components/esm/loadclas.hpp>
#include <components/esm/loadspel.hpp>

#include <components/openmw-mp/Base/BaseStructs.hpp>

#include <RakNetTypes.h>

namespace mwmp
{
    struct CurrentContainer
    {
        std::string refId;
        unsigned int refNum;
        unsigned int mpNum;
        bool loot;
    };

    struct JournalItem
    {
        std::string quest;
        int index;
        enum JOURNAL_ITEM_TYPE
        {
            ENTRY = 0,
            INDEX = 1,
            TOPIC_ENTRY = 2
        };

        std::string actorRefId;
        std::string topicId;
        std::string infoId;
        std::string text;
        std::string actorName;

        bool hasTimestamp;
        mwmp::Time timestamp;

        int type; // 0 - An entire quest entry, 1 - An index, 2 - A dialogue topic entry
    };

    struct Faction
    {
        std::string factionId;
        int rank;
        int reputation;
        bool isExpelled;
    };

    struct Topic
    {
        std::string topicId;
    };

    struct Book
    {
        std::string bookId;
    };

    struct QuickKey
    {
        std::string itemId;

        enum QUICKKEY_TYPE
        {   
            ITEM = 0,
            MAGIC = 1,
            ITEM_MAGIC = 2,
            UNASSIGNED = 3
        };

        unsigned short slot;
        int type;
    };

    struct CellState
    {
        ESM::Cell cell;
        float lastRespawnHour = 0.f;
        int lastRespawnDay = 0;

        enum CELL_STATE_ACTION
        {
            LOAD = 0,
            UNLOAD = 1
        };

        int type; // 0 - Cell load, 1 - Cell unload
    };

    struct LocalMapFogTexture
    {
        int x = 0;
        int y = 0;
        std::vector<char> imageData;
    };

    struct LocalMapFogCell
    {
        uint32_t updateId = 0;
        std::string cellDescription;
        bool isInterior = false;
        float northMarkerAngle = 0.f;
        float minX = 0.f;
        float minY = 0.f;
        float maxX = 0.f;
        float maxY = 0.f;
        std::vector<LocalMapFogTexture> textures;
    };

    struct CellRespawnedReference
    {
        uint32_t refNum = 0;
        uint32_t mpNum = 0;
        uint8_t type = 0;
        std::string currentCell;
    };

    struct CellRespawnUpdate
    {
        uint32_t updateId = 0;
        std::string cellDescription;
        float lastRespawnHour = 0.f;
        int lastRespawnDay = 0;
        bool shouldRespawn = false;
        std::vector<CellRespawnedReference> references;
    };

    enum LOCAL_MAP_FOG_ACTION
    {
        LOCAL_MAP_FOG_REQUEST = 0,
        LOCAL_MAP_FOG_UPDATE = 1,
        LOCAL_MAP_FOG_RESPONSE = 2,
        LOCAL_MAP_FOG_ACK = 3
    };

    enum CELL_RESPAWN_ACTION
    {
        CELL_RESPAWN_UPDATE = 0,
        CELL_RESPAWN_ACK = 1,
        CELL_RESPAWN_DEFER = 2,
        CELL_RESPAWN_APPLY = 3,
        CELL_RESPAWN_STATE = 4
    };

    struct FactionChanges
    {
        std::vector<Faction> factions;

        enum FACTION_ACTION
        {
            RANK = 0,
            EXPULSION = 1,
            REPUTATION = 2
        };

        int action; // 0 - Rank, 1 - Expulsion state, 2 - Faction reputation
    };

    struct InventoryChanges
    {
        std::vector<Item> items;
        enum ACTION_TYPE
        {
            SET = 0,
            ADD,
            REMOVE
        };
        int action; // 0 - Clear and set in entirety, 1 - Add item, 2 - Remove item
    };

    struct SpellbookChanges
    {
        std::vector<ESM::Spell> spells;
        enum ACTION_TYPE
        {
            SET = 0,
            ADD,
            REMOVE
        };
        int action; // 0 - Clear and set in entirety, 1 - Add spell, 2 - Remove spell
    };

    enum RESURRECT_TYPE
    {
        REGULAR = 0,
        IMPERIAL_SHRINE,
        TRIBUNAL_TEMPLE
    };

    enum MISCELLANEOUS_CHANGE_TYPE
    {
        MARK_LOCATION = 0,
        SELECTED_SPELL
    };

    class BasePlayer
    {
    public:

        struct CharGenState
        {
            int currentStage, endStage;
            bool isFinished;
        };

        struct GUIMessageBox
        {
            int id;
            int type;
            enum GUI_TYPE
            {
                MessageBox = 0,
                CustomMessageBox,
                InputDialog,
                PasswordDialog,
                ListBox
            };
            std::string label;
            std::string note;
            std::string buttons;

            std::string data;
        };

        BasePlayer(RakNet::RakNetGUID guid) : guid(guid)
        {
            inventoryChanges.action = 0;
            spellbookChanges.action = 0;

            exchangeFullInfo = false;
            displayCreatureName = false;
            resetStats = false;
            enforcedLogLevel = -1;
        }

        BasePlayer()
        {

        }

        RakNet::RakNetGUID guid;

        GUIMessageBox guiMessageBox;

        // Track only the indexes of the attributes that have been changed,
        // with the attribute values themselves being stored in creatureStats.mAttributes
        std::vector<uint8_t> attributeIndexChanges;

        // Track only the indexes of the skills that have been changed,
        // with the skill values themselves being stored in npcStats.mSkills
        std::vector<uint8_t> skillIndexChanges;

        // Track only the indexes of the dynamic states that have been changed,
        // with the dynamicStats themselves being stored in creatureStats.mDynamic
        std::vector<uint8_t> statsDynamicIndexChanges;

        // Track only the indexes of the equipment items that have been changed,
        // with the items themselves being stored in equipmentItems
        std::vector<int> equipmentIndexChanges;

        bool exchangeFullInfo;

        InventoryChanges inventoryChanges;
        SpellbookChanges spellbookChanges;
        std::vector<SpellCooldown> cooldownChanges;
        SpellsActiveChanges spellsActiveChanges;
        std::vector<QuickKey> quickKeyChanges;
        std::vector<JournalItem> journalChanges;
        FactionChanges factionChanges;
        std::vector<Topic> topicChanges;
        std::vector<Book> bookChanges;
        std::vector<CellState> cellStateChanges;
        LocalMapFogCell localMapFog;
        int localMapFogAction = LOCAL_MAP_FOG_REQUEST;
        CellRespawnUpdate cellRespawn;
        int cellRespawnAction = CELL_RESPAWN_UPDATE;

        std::vector<RakNet::RakNetGUID> alliedPlayers;
        CurrentContainer currentContainer;

        int difficulty = 0;
        int enforcedLogLevel;
        float physicsFramerate = 60.0;
        bool consoleAllowed = false;
        bool bedRestAllowed = true;
        bool wildernessRestAllowed = true;
        bool waitAllowed = true;

        bool ignorePosPacket;

        unsigned int movementFlags = 0;
        char drawState;
        bool isJumping = false;
        bool isFlying = false;
        bool hasTcl = false;

        ESM::Position position;
        ESM::Position direction;
        ESM::Position previousCellPosition;
        ESM::Position momentum;
        ESM::Cell cell;
        ESM::NPC npc;
        ESM::NpcStats npcStats;
        ESM::Creature creature;
        ESM::CreatureStats creatureStats;
        ESM::Class charClass;
        Item equipmentItems[19];
        Attack attack;
        Cast cast;
        std::string birthsign;
        std::string chatMessage;
        CharGenState charGenState;
        std::map<std::string, std::string> gameSettings;
        std::map<std::string, std::string> vrSettings;

        std::string sound;
        Animation animation;
        char deathState;

        bool resetStats = false;
        float scale = 1;
        bool isWerewolf = false;

        bool displayCreatureName;
        std::string creatureRefId;

        bool isChangingRegion;

        Target killer;

        int jailDays;
        bool ignoreJailTeleportation;
        bool ignoreJailSkillIncreases;
        std::string jailProgressText;
        std::string jailEndText;

        unsigned int resurrectType;
        unsigned int miscellaneousChangeType;

        ESM::Cell markCell;
        ESM::Position markPosition;
        std::string selectedSpellId;

        mwmp::Item usedItem;
        bool usingItemMagic;
        char itemUseDrawState;
    };
}

#endif //OPENMW_BASEPLAYER_HPP
