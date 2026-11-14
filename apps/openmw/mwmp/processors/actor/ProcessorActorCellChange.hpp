#ifndef OPENMW_PROCESSORACTORCELLCHANGE_HPP
#define OPENMW_PROCESSORACTORCELLCHANGE_HPP


#include "../ActorProcessor.hpp"
#include "apps/openmw/mwmp/Main.hpp"
#include "apps/openmw/mwmp/CellController.hpp"
#include "apps/openmw/mwmp/Networking.hpp"
#include "apps/openmw/mwmp/ObjectList.hpp"

namespace mwmp
{
    class ProcessorActorCellChange final: public ActorProcessor
    {
    public:
        ProcessorActorCellChange()
        {
            BPP_INIT(ID_ACTOR_CELL_CHANGE);
        }

        virtual void Do(ActorPacket &packet, ActorList &actorList)
        {
            Main::get().getCellController()->readCellChange(actorList);
            for (const auto& actor : actorList.baseActors)
            {
                MWWorld::CellStore* cellStore = Main::get().getCellController()->getCellStore(actor.cell);
                Main::get().getNetworking()->getObjectList()->applyPendingNpcDispositions(cellStore);
            }
        }
    };
}

#endif //OPENMW_PROCESSORACTORCELLCHANGE_HPP
