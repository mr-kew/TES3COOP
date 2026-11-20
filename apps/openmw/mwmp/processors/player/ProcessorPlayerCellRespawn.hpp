#ifndef OPENMW_PROCESSORPLAYERCELLRESPAWN_HPP
#define OPENMW_PROCESSORPLAYERCELLRESPAWN_HPP

#include "../PlayerProcessor.hpp"
#include "apps/openmw/mwbase/environment.hpp"
#include "apps/openmw/mwbase/world.hpp"
#include "apps/openmw/mwworld/cellstore.hpp"
#include "../Main.hpp"
#include "../Networking.hpp"

#include <sstream>

namespace mwmp
{
    class ProcessorPlayerCellRespawn final : public PlayerProcessor
    {
    public:
        ProcessorPlayerCellRespawn()
        {
            BPP_INIT(ID_PLAYER_CELL_RESPAWN)
        }

        void Do(PlayerPacket&, BasePlayer* player) override
        {
            if (!player)
                return;

            const CellRespawnUpdate& update = player->cellRespawn;
            if (player->cellRespawnAction == CELL_RESPAWN_ACK)
            {
                if (isLocal())
                    static_cast<LocalPlayer*>(player)->acknowledgeCellRespawn(update.updateId);
                return;
            }
            if (player->cellRespawnAction == CELL_RESPAWN_DEFER)
            {
                if (isLocal())
                    static_cast<LocalPlayer*>(player)->deferCellRespawn(update.updateId);
                return;
            }
            if (player->cellRespawnAction == CELL_RESPAWN_STATE && !isLocal())
                return;

            MWBase::World* world = MWBase::Environment::get().getWorld();
            MWWorld::CellStore* cellStore = world->getInterior(update.cellDescription);
            if (!cellStore)
            {
                std::istringstream coordinates(update.cellDescription);
                int x = 0;
                int y = 0;
                char comma = 0;
                if (!(coordinates >> x >> comma >> y) || comma != ',')
                    return;
                cellStore = world->getExterior(x, y);
            }
            if (!cellStore)
                return;

            cellStore->setLastRespawn(MWWorld::TimeStamp(update.lastRespawnHour, update.lastRespawnDay));
            if (player->cellRespawnAction == CELL_RESPAWN_STATE)
            {
                static_cast<LocalPlayer*>(player)->deferCellRespawnState(update);
                return;
            }
            if (player->cellRespawnAction != CELL_RESPAWN_APPLY)
                return;

            for (const CellRespawnedReference& reference : update.references)
            {
                MWWorld::CellStore* currentCell = world->getInterior(reference.currentCell);
                if (!currentCell)
                {
                    std::istringstream coordinates(reference.currentCell);
                    int x = 0;
                    int y = 0;
                    char comma = 0;
                    if (!(coordinates >> x >> comma >> y) || comma != ',')
                        continue;
                    currentCell = world->getExterior(x, y);
                }
                if (!currentCell || currentCell->getState() != MWWorld::CellStore::State_Loaded)
                    continue;

                MWWorld::Ptr ptr = currentCell->searchExact(reference.refNum, reference.mpNum);
                if (!ptr.isEmpty())
                    ptr.getClass().respawn(ptr);
            }
        }
    };
}

#endif
