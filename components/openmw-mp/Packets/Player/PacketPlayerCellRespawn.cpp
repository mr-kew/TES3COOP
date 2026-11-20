#include <components/openmw-mp/NetworkMessages.hpp>
#include "PacketPlayerCellRespawn.hpp"

namespace
{
    constexpr uint32_t maxCellDescriptionSize = 1024;
    constexpr uint32_t maxRespawnReferences = 4096;
    constexpr uint32_t maxCurrentCellSize = 1024;
}

mwmp::PacketPlayerCellRespawn::PacketPlayerCellRespawn(RakNet::RakPeerInterface* peer)
    : PlayerPacket(peer)
{
    packetID = ID_PLAYER_CELL_RESPAWN;
    priority = HIGH_PRIORITY;
    reliability = RELIABLE_ORDERED;
}

void mwmp::PacketPlayerCellRespawn::Packet(RakNet::BitStream* newBitstream, bool send)
{
    PlayerPacket::Packet(newBitstream, send);

    if (!RW(player->cellRespawnAction, send)
        || player->cellRespawnAction < CELL_RESPAWN_UPDATE
        || player->cellRespawnAction > CELL_RESPAWN_STATE)
    {
        packetValid = false;
        return;
    }

    CellRespawnUpdate& update = player->cellRespawn;
    if (!RW(update.updateId, send)
        || !RW(update.cellDescription, send, true, maxCellDescriptionSize)
        || !RW(update.lastRespawnHour, send)
        || !RW(update.lastRespawnDay, send)
        || !RW(update.shouldRespawn, send))
    {
        packetValid = false;
        return;
    }

    uint32_t count = send ? static_cast<uint32_t>(update.references.size()) : 0;
    if (!RW(count, send) || count > maxRespawnReferences)
    {
        packetValid = false;
        return;
    }
    if (!send)
    {
        update.references.clear();
        update.references.resize(count);
    }

    for (CellRespawnedReference& reference : update.references)
    {
        if (!RW(reference.refNum, send)
            || !RW(reference.mpNum, send)
            || !RW(reference.type, send)
            || reference.type > 2
            || !RW(reference.currentCell, send, true, maxCurrentCellSize))
        {
            packetValid = false;
            return;
        }
    }
}
