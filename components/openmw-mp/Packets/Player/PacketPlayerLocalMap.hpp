#ifndef OPENMW_PACKETPLAYERLOCALMAP_HPP
#define OPENMW_PACKETPLAYERLOCALMAP_HPP

#include "PlayerPacket.hpp"

namespace mwmp
{
    class PacketPlayerLocalMap : public PlayerPacket
    {
    public:
        explicit PacketPlayerLocalMap(RakNet::RakPeerInterface* peer);
        void Packet(RakNet::BitStream* newBitstream, bool send) override;
    };
}

#endif
