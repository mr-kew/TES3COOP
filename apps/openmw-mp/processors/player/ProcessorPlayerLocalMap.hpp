#ifndef OPENMW_PROCESSORPLAYERLOCALMAP_HPP
#define OPENMW_PROCESSORPLAYERLOCALMAP_HPP

#include "../PlayerProcessor.hpp"
#include "apps/openmw-mp/Script/Script.hpp"

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
            Script::Call<Script::CallbackIdentity("OnPlayerLocalMap")>(player.getId());
        }
    };
}

#endif
