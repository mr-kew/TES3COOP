#include <components/openmw-mp/NetworkMessages.hpp>
#include "PacketPlayerLocalMap.hpp"

namespace
{
    constexpr uint32_t maxCellDescriptionSize = 1024;
    constexpr uint32_t maxTexturesPerCell = 128;
    constexpr uint32_t maxTextureSize = 8192;
    constexpr uint32_t maxFogPayloadSize = 128 * 1024;
}

mwmp::PacketPlayerLocalMap::PacketPlayerLocalMap(RakNet::RakPeerInterface* peer)
    : PlayerPacket(peer)
{
    packetID = ID_PLAYER_LOCAL_MAP;
}

void mwmp::PacketPlayerLocalMap::Packet(RakNet::BitStream* newBitstream, bool send)
{
    PlayerPacket::Packet(newBitstream, send);

    if (!RW(player->localMapFogAction, send))
    {
        packetValid = false;
        return;
    }
    if (player->localMapFogAction < LOCAL_MAP_FOG_REQUEST || player->localMapFogAction > LOCAL_MAP_FOG_ACK)
    {
        packetValid = false;
        return;
    }

    LocalMapFogCell& fog = player->localMapFog;
    if (!RW(fog.updateId, send)
        || !RW(fog.cellDescription, send, true, maxCellDescriptionSize)
        || !RW(fog.isInterior, send)
        || !RW(fog.northMarkerAngle, send)
        || !RW(fog.minX, send)
        || !RW(fog.minY, send)
        || !RW(fog.maxX, send)
        || !RW(fog.maxY, send))
    {
        packetValid = false;
        return;
    }

    uint32_t textureCount = 0;
    if (send)
        textureCount = static_cast<uint32_t>(fog.textures.size());
    if (!RW(textureCount, send) || textureCount > maxTexturesPerCell)
    {
        packetValid = false;
        return;
    }
    if (!send)
    {
        fog.textures.clear();
        fog.textures.resize(textureCount);
    }

    uint32_t totalImageSize = 0;
    for (LocalMapFogTexture& texture : fog.textures)
    {
        uint32_t imageSize = 0;
        if (send)
            imageSize = static_cast<uint32_t>(texture.imageData.size());

        if (!RW(texture.x, send) || !RW(texture.y, send) || !RW(imageSize, send)
            || imageSize > maxTextureSize || totalImageSize + imageSize > maxFogPayloadSize)
        {
            packetValid = false;
            return;
        }

        totalImageSize += imageSize;
        if (!send)
            texture.imageData.resize(imageSize);

        for (char& byte : texture.imageData)
        {
            if (!RW(byte, send))
            {
                packetValid = false;
                return;
            }
        }
    }
}
