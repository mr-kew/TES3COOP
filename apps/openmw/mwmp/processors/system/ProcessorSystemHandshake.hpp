#ifndef OPENMW_PROCESSORSYSTEMHANDSHAKE_HPP
#define OPENMW_PROCESSORSYSTEMHANDSHAKE_HPP

#include <components/openmw-mp/Base/BaseSystem.hpp>

#include "apps/openmw/mwmp/Main.hpp"
#include "apps/openmw/mwmp/Networking.hpp"

#include "../SystemProcessor.hpp"

namespace mwmp
{
    class ProcessorSystemHandshake final: public SystemProcessor
    {
    public:
        ProcessorSystemHandshake()
        {
            BPP_INIT(ID_SYSTEM_HANDSHAKE)
        }

        virtual void Do(SystemPacket &packet, BaseSystem *system)
        {
            packet.setSystem(Main::get().getLocalSystem());
            if (isRequest() && packet.Send(serverAddr) != 0)
                Main::get().getNetworking()->setHandshakeComplete();
        }
    };
}

#endif //OPENMW_PROCESSORSYSTEMHANDSHAKE_HPP
