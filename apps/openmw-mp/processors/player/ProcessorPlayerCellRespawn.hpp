#ifndef OPENMW_PROCESSORPLAYERCELLRESPAWN_HPP
#define OPENMW_PROCESSORPLAYERCELLRESPAWN_HPP

#include "../PlayerProcessor.hpp"
#include "apps/openmw-mp/Script/Script.hpp"
#include <components/openmw-mp/TimedLog.hpp>

namespace mwmp
{
    class ProcessorPlayerCellRespawn final : public PlayerProcessor
    {
    public:
        ProcessorPlayerCellRespawn()
        {
            BPP_INIT(ID_PLAYER_CELL_RESPAWN)
        }

        void Do(PlayerPacket&, Player& player) override
        {
            if (player.cellRespawnAction != CELL_RESPAWN_UPDATE)
                return;

            LOG_MESSAGE_SIMPLE(TimedLog::LOG_VERBOSE, "Received vanilla respawn report from %s for %s (%u references)",
                player.npc.mName.c_str(), player.cellRespawn.cellDescription.c_str(),
                static_cast<unsigned int>(player.cellRespawn.references.size()));
            Script::Call<Script::CallbackIdentity("OnPlayerCellRespawn")>(player.getId());
        }
    };
}

#endif
