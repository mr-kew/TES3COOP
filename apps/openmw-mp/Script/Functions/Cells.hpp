#ifndef OPENMW_CELLAPI_HPP
#define OPENMW_CELLAPI_HPP

#include "../Types.hpp"
#include <components/openmw-mp/Base/BasePlayer.hpp>

#define CELLAPI \
    {"GetCellStateChangesSize", CellFunctions::GetCellStateChangesSize},\
    \
    {"GetCellStateType",        CellFunctions::GetCellStateType},\
    {"GetCellStateDescription", CellFunctions::GetCellStateDescription},\
    {"GetCellStateLastRespawnHour", CellFunctions::GetCellStateLastRespawnHour},\
    {"GetCellStateLastRespawnDay", CellFunctions::GetCellStateLastRespawnDay},\
    \
    {"GetCell",                 CellFunctions::GetCell},\
    {"GetExteriorX",            CellFunctions::GetExteriorX},\
    {"GetExteriorY",            CellFunctions::GetExteriorY},\
    {"IsInExterior",            CellFunctions::IsInExterior},\
    \
    {"GetRegion",               CellFunctions::GetRegion},\
    {"IsChangingRegion",        CellFunctions::IsChangingRegion},\
    \
    {"SetCell",                 CellFunctions::SetCell},\
    {"SetExteriorCell",         CellFunctions::SetExteriorCell},\
    \
    {"SendCell",                CellFunctions::SendCell},\
    {"ReadPlayerCellRespawn", CellFunctions::ReadPlayerCellRespawn},\
    {"GetPlayerCellRespawnAction", CellFunctions::GetPlayerCellRespawnAction},\
    {"GetPlayerCellRespawnUpdateId", CellFunctions::GetPlayerCellRespawnUpdateId},\
    {"GetPlayerCellRespawnCellDescription", CellFunctions::GetPlayerCellRespawnCellDescription},\
    {"GetPlayerCellRespawnLastRespawnHour", CellFunctions::GetPlayerCellRespawnLastRespawnHour},\
    {"GetPlayerCellRespawnLastRespawnDay", CellFunctions::GetPlayerCellRespawnLastRespawnDay},\
    {"GetPlayerCellRespawnReferenceCount", CellFunctions::GetPlayerCellRespawnReferenceCount},\
    {"GetPlayerCellRespawnReferenceRefNum", CellFunctions::GetPlayerCellRespawnReferenceRefNum},\
    {"GetPlayerCellRespawnReferenceMpNum", CellFunctions::GetPlayerCellRespawnReferenceMpNum},\
    {"GetPlayerCellRespawnReferenceType", CellFunctions::GetPlayerCellRespawnReferenceType},\
    {"GetPlayerCellRespawnReferenceCurrentCell", CellFunctions::GetPlayerCellRespawnReferenceCurrentCell},\
    {"SetPlayerCellRespawnAction", CellFunctions::SetPlayerCellRespawnAction},\
    {"SetPlayerCellRespawnState", CellFunctions::SetPlayerCellRespawnState},\
    {"SetPlayerCellRespawnLastRespawn", CellFunctions::SetPlayerCellRespawnLastRespawn},\
    {"SendPlayerCellRespawn", CellFunctions::SendPlayerCellRespawn}


class CellFunctions
{
public:

    static mwmp::BasePlayer *readCellRespawnPlayer;
    static std::string tempCellRespawnDescription;

    /**
    * \brief Get the number of indexes in a player's latest cell state changes.
    *
    * \param pid The player ID whose cell state changes should be used.
    * \return The number of indexes.
    */
    static unsigned int GetCellStateChangesSize(unsigned short pid) noexcept;

    /**
    * \brief Get the cell state type at a certain index in a player's latest cell state changes.
    *
    * \param pid The player ID whose cell state changes should be used.
    * \param index The index of the cell state.
    * \return The cell state type (0 for LOAD, 1 for UNLOAD).
    */
    static unsigned int GetCellStateType(unsigned short pid, unsigned int index) noexcept;

    /**
    * \brief Get the cell description at a certain index in a player's latest cell state changes.
    *
    * \param pid The player ID whose cell state changes should be used.
    * \param index The index of the cell state.
    * \return The cell description.
    */
    static const char *GetCellStateDescription(unsigned short pid, unsigned int index) noexcept;

    static double GetCellStateLastRespawnHour(unsigned short pid, unsigned int index) noexcept;
    static int GetCellStateLastRespawnDay(unsigned short pid, unsigned int index) noexcept;

    static void ReadPlayerCellRespawn(unsigned short pid) noexcept;
    static int GetPlayerCellRespawnAction() noexcept;
    static unsigned int GetPlayerCellRespawnUpdateId() noexcept;
    static const char* GetPlayerCellRespawnCellDescription() noexcept;
    static double GetPlayerCellRespawnLastRespawnHour() noexcept;
    static int GetPlayerCellRespawnLastRespawnDay() noexcept;
    static unsigned int GetPlayerCellRespawnReferenceCount() noexcept;
    static unsigned int GetPlayerCellRespawnReferenceRefNum(unsigned int index) noexcept;
    static unsigned int GetPlayerCellRespawnReferenceMpNum(unsigned int index) noexcept;
    static unsigned int GetPlayerCellRespawnReferenceType(unsigned int index) noexcept;
    static const char* GetPlayerCellRespawnReferenceCurrentCell(unsigned int index) noexcept;
    static void SetPlayerCellRespawnAction(unsigned short pid, int action) noexcept;
    static void SetPlayerCellRespawnState(unsigned short pid, const char* cellDescription, double hour, int day, bool shouldRespawn) noexcept;
    static void SetPlayerCellRespawnLastRespawn(unsigned short pid, double hour, int day) noexcept;
    static void SendPlayerCellRespawn(unsigned short pid, bool sendToOtherPlayers, bool skipAttachedPlayer) noexcept;

    /**
    * \brief Get the cell description of a player's cell.
    *
    * \param pid The player ID.
    * \return The cell description.
    */
    static const char *GetCell(unsigned short pid) noexcept;

    /**
    * \brief Get the X coordinate of the player's exterior cell.
    *
    * \param pid The player ID.
    * \return The X coordinate of the cell.
    */
    static int GetExteriorX(unsigned short pid) noexcept;

    /**
    * \brief Get the Y coordinate of the player's exterior cell.
    *
    * \param pid The player ID.
    * \return The Y coordinate of the cell.
    */
    static int GetExteriorY(unsigned short pid) noexcept;

    /**
    * \brief Check whether the player is in an exterior cell or not.
    *
    * \param pid The player ID.
    * \return Whether the player is in an exterior cell.
    */
    static bool IsInExterior(unsigned short pid) noexcept;

    /**
    * \brief Get the region of the player's exterior cell.
    *
    * A blank value will be returned if the player is in an interior.
    *
    * \param pid The player ID.
    * \return The region.
    */
    static const char *GetRegion(unsigned short pid) noexcept;

    /**
    * \brief Check whether the player's last cell change has involved a region change.
    *
    * \param pid The player ID.
    * \return Whether the player has changed their region.
    */
    static bool IsChangingRegion(unsigned short pid) noexcept;

    /**
    * \brief Set the cell of a player.
    *
    * This changes the cell recorded for that player in the server memory, but does not by itself
    * send a packet.
    *
    * The cell is determined to be an exterior cell if it fits the pattern of a number followed
    * by a comma followed by another number.
    *
    * \param pid The player ID.
    * \param cellDescription The cell description.
    * \return void
    */
    static void SetCell(unsigned short pid, const char *cellDescription) noexcept;

    /**
    * \brief Set the cell of a player to an exterior cell.
    *
    * This changes the cell recorded for that player in the server memory, but does not by itself
    * send a packet.
    *
    * \param pid The player ID.
    * \param x The X coordinate of the cell.
    * \param y The Y coordinate of the cell.
    * \return void
    */
    static void SetExteriorCell(unsigned short pid, int x, int y) noexcept;

    /**
    * \brief Send a PlayerCellChange packet about a player.
    *
    * It is only sent to the affected player.
    *
    * \param pid The player ID.
    * \return void
    */
    static void SendCell(unsigned short pid) noexcept;

};

#endif //OPENMW_CELLAPI_HPP
