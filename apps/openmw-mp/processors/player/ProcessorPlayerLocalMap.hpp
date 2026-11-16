#ifndef OPENMW_PROCESSORPLAYERLOCALMAP_HPP
#define OPENMW_PROCESSORPLAYERLOCALMAP_HPP

#include "../PlayerProcessor.hpp"
#include "apps/openmw-mp/Script/Script.hpp"
#include <components/openmw-mp/TimedLog.hpp>

namespace mwmp
{
    class ProcessorPlayerLocalMap : public PlayerProcessor
    {
    public:
        ProcessorPlayerLocalMap()
        {
            BPP_INIT(ID_PLAYER_LOCAL_MAP)
        }

        void Do(PlayerPacket&, Player& player) override
        {
            LOG_MESSAGE_SIMPLE(TimedLog::LOG_VERBOSE, "Received %s from %s", strPacketID.c_str(), player.npc.mName.c_str());
            LOG_APPEND(TimedLog::LOG_VERBOSE, "- action: %i\n- cell: %s\n- textures: %u", player.localMapFogAction,
                player.localMapFog.cellDescription.c_str(), static_cast<unsigned int>(player.localMapFog.textures.size()));
            Script::Call<Script::CallbackIdentity("OnPlayerLocalMap")>(player.getId());
        }
    };
}

#endif
