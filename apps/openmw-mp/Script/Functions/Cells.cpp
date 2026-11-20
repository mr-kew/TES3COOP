#include "Cells.hpp"

#include <components/openmw-mp/TimedLog.hpp>
#include <components/openmw-mp/NetworkMessages.hpp>
#include <components/openmw-mp/Controllers/PlayerPacketController.hpp>
#include <components/openmw-mp/Packets/Player/PlayerPacket.hpp>

#include <apps/openmw-mp/Script/ScriptFunctions.hpp>
#include <apps/openmw-mp/Player.hpp>
#include <apps/openmw-mp/Networking.hpp>

#include <iostream>

static std::string tempCellDescription;
mwmp::BasePlayer *CellFunctions::readCellRespawnPlayer = nullptr;
std::string CellFunctions::tempCellRespawnDescription;

unsigned int CellFunctions::GetCellStateChangesSize(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);

    return player->cellStateChanges.size();
}

unsigned int CellFunctions::GetCellStateType(unsigned short pid, unsigned int index) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);

    return player->cellStateChanges.at(index).type;
}

const char *CellFunctions::GetCellStateDescription(unsigned short pid, unsigned int index) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, "");

    if (index >= player->cellStateChanges.size())
        return "invalid";

    tempCellDescription = player->cellStateChanges.at(index).cell.getShortDescription();
    return tempCellDescription.c_str();
}

double CellFunctions::GetCellStateLastRespawnHour(unsigned short pid, unsigned int index) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0.0);

    if (index >= player->cellStateChanges.size())
        return 0.0;

    return player->cellStateChanges[index].lastRespawnHour;
}

int CellFunctions::GetCellStateLastRespawnDay(unsigned short pid, unsigned int index) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);

    if (index >= player->cellStateChanges.size())
        return 0;

    return player->cellStateChanges[index].lastRespawnDay;
}

void CellFunctions::ReadPlayerCellRespawn(unsigned short pid) noexcept
{
    readCellRespawnPlayer = Players::getPlayer(pid);
}

int CellFunctions::GetPlayerCellRespawnAction() noexcept
{
    return readCellRespawnPlayer ? readCellRespawnPlayer->cellRespawnAction : mwmp::CELL_RESPAWN_UPDATE;
}

unsigned int CellFunctions::GetPlayerCellRespawnUpdateId() noexcept
{
    return readCellRespawnPlayer ? readCellRespawnPlayer->cellRespawn.updateId : 0;
}

const char* CellFunctions::GetPlayerCellRespawnCellDescription() noexcept
{
    return readCellRespawnPlayer ? readCellRespawnPlayer->cellRespawn.cellDescription.c_str() : "";
}

double CellFunctions::GetPlayerCellRespawnLastRespawnHour() noexcept
{
    return readCellRespawnPlayer ? readCellRespawnPlayer->cellRespawn.lastRespawnHour : 0.0;
}

int CellFunctions::GetPlayerCellRespawnLastRespawnDay() noexcept
{
    return readCellRespawnPlayer ? readCellRespawnPlayer->cellRespawn.lastRespawnDay : 0;
}

unsigned int CellFunctions::GetPlayerCellRespawnReferenceCount() noexcept
{
    return readCellRespawnPlayer ? static_cast<unsigned int>(readCellRespawnPlayer->cellRespawn.references.size()) : 0;
}

unsigned int CellFunctions::GetPlayerCellRespawnReferenceRefNum(unsigned int index) noexcept
{
    if (!readCellRespawnPlayer || index >= readCellRespawnPlayer->cellRespawn.references.size())
        return 0;
    return readCellRespawnPlayer->cellRespawn.references[index].refNum;
}

unsigned int CellFunctions::GetPlayerCellRespawnReferenceMpNum(unsigned int index) noexcept
{
    if (!readCellRespawnPlayer || index >= readCellRespawnPlayer->cellRespawn.references.size())
        return 0;
    return readCellRespawnPlayer->cellRespawn.references[index].mpNum;
}

unsigned int CellFunctions::GetPlayerCellRespawnReferenceType(unsigned int index) noexcept
{
    if (!readCellRespawnPlayer || index >= readCellRespawnPlayer->cellRespawn.references.size())
        return 0;
    return readCellRespawnPlayer->cellRespawn.references[index].type;
}

const char* CellFunctions::GetPlayerCellRespawnReferenceCurrentCell(unsigned int index) noexcept
{
    if (!readCellRespawnPlayer || index >= readCellRespawnPlayer->cellRespawn.references.size())
        return "";
    tempCellRespawnDescription = readCellRespawnPlayer->cellRespawn.references[index].currentCell;
    return tempCellRespawnDescription.c_str();
}

void CellFunctions::SetPlayerCellRespawnAction(unsigned short pid, int action) noexcept
{
    Player *player;
    GET_PLAYER(pid, player,);
    if (action >= mwmp::CELL_RESPAWN_UPDATE && action <= mwmp::CELL_RESPAWN_STATE)
        player->cellRespawnAction = action;
}

void CellFunctions::SetPlayerCellRespawnState(unsigned short pid, const char* cellDescription, double hour, int day, bool shouldRespawn) noexcept
{
    Player *player;
    GET_PLAYER(pid, player,);
    if (!cellDescription)
        return;

    player->cellRespawn.updateId = 0;
    player->cellRespawn.cellDescription = cellDescription;
    player->cellRespawn.lastRespawnHour = static_cast<float>(hour);
    player->cellRespawn.lastRespawnDay = day;
    player->cellRespawn.shouldRespawn = shouldRespawn;
    player->cellRespawn.references.clear();
}

void CellFunctions::SetPlayerCellRespawnLastRespawn(unsigned short pid, double hour, int day) noexcept
{
    Player *player;
    GET_PLAYER(pid, player,);
    player->cellRespawn.lastRespawnHour = static_cast<float>(hour);
    player->cellRespawn.lastRespawnDay = day;
}

void CellFunctions::SendPlayerCellRespawn(unsigned short pid, bool sendToOtherPlayers, bool skipAttachedPlayer) noexcept
{
    Player *player;
    GET_PLAYER(pid, player,);
    mwmp::PlayerPacket *packet = mwmp::Networking::get().getPlayerPacketController()->GetPacket(ID_PLAYER_CELL_RESPAWN);
    packet->setPlayer(player);
    if (!skipAttachedPlayer)
        packet->Send(false);
    if (sendToOtherPlayers)
        packet->Send(true);
}

const char *CellFunctions::GetCell(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);

    tempCellDescription = player->cell.getShortDescription().c_str();
    return tempCellDescription.c_str();
}

int CellFunctions::GetExteriorX(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);
    return player->cell.mData.mX;
}

int CellFunctions::GetExteriorY(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);
    return player->cell.mData.mY;
}

bool CellFunctions::IsInExterior(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, false);

    return player->cell.isExterior();
}

const char *CellFunctions::GetRegion(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, 0);

    return player->cell.mRegion.c_str();
}

bool CellFunctions::IsChangingRegion(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, false);

    return player->isChangingRegion;
}

void CellFunctions::SetCell(unsigned short pid, const char *cellDescription) noexcept
{
    Player *player;
    GET_PLAYER(pid, player,);

    LOG_MESSAGE_SIMPLE(TimedLog::LOG_INFO, "Script is moving %s from %s to %s", player->npc.mName.c_str(),
                       player->cell.getShortDescription().c_str(), cellDescription);

    player->cell = Utils::getCellFromDescription(cellDescription);
}

void CellFunctions::SetExteriorCell(unsigned short pid, int x, int y) noexcept
{
    Player *player;
    GET_PLAYER(pid, player,);

    LOG_MESSAGE_SIMPLE(TimedLog::LOG_INFO, "Script is moving %s from %s to %i,%i", player->npc.mName.c_str(),
                       player->cell.getShortDescription().c_str(), x, y);

    // If the player is currently in an interior, turn off the interior flag
    // from the cell
    if (!player->cell.isExterior())
        player->cell.mData.mFlags &= ~ESM::Cell::Interior;

    player->cell.mData.mX = x;
    player->cell.mData.mY = y;
}

void CellFunctions::SendCell(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, );

    mwmp::PlayerPacket *packet = mwmp::Networking::get().getPlayerPacketController()->GetPacket(ID_PLAYER_CELL_CHANGE);
    packet->setPlayer(player);

    packet->Send(false);
}
