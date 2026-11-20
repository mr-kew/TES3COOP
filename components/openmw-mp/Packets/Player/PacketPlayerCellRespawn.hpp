#ifndef OPENMW_PACKETPLAYERCELLRESPAWN_HPP
#define OPENMW_PACKETPLAYERCELLRESPAWN_HPP

#include "PlayerPacket.hpp"

namespace mwmp
{
    class PacketPlayerCellRespawn final : public PlayerPacket
    {
    public:
        explicit PacketPlayerCellRespawn(RakNet::RakPeerInterface* peer);
        void Packet(RakNet::BitStream* newBitstream, bool send) override;
    };
}

#endif
