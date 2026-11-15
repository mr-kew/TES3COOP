#ifndef OPENMW_PROCESSORPLAYERLOCALMAP_HPP
#define OPENMW_PROCESSORPLAYERLOCALMAP_HPP

#include "../PlayerProcessor.hpp"
#include <components/esm/fogstate.hpp>
#include "apps/openmw/mwbase/environment.hpp"
#include "apps/openmw/mwbase/world.hpp"
#include "apps/openmw/mwbase/windowmanager.hpp"
#include "apps/openmw/mwworld/cellstore.hpp"

#include <memory>
#include <sstream>
#include <utility>

namespace mwmp
{
    class ProcessorPlayerLocalMap final : public PlayerProcessor
    {
    public:
        ProcessorPlayerLocalMap()
        {
            BPP_INIT(ID_PLAYER_LOCAL_MAP)
        }

        void Do(PlayerPacket&, BasePlayer* player) override
        {
            if (!isLocal() || !player)
                return;

            const LocalMapFogCell& source = player->localMapFog;
            if (player->localMapFogAction == LOCAL_MAP_FOG_ACK)
            {
                MWBase::Environment::get().getWindowManager()->acknowledgeLocalMapFog(source.cellDescription, source.updateId);
                return;
            }
            if (player->localMapFogAction != LOCAL_MAP_FOG_RESPONSE)
                return;
            MWWorld::CellStore* cell = nullptr;
            MWBase::World* world = MWBase::Environment::get().getWorld();
            if (source.isInterior)
                cell = world->getInterior(source.cellDescription);
            else
            {
                std::istringstream coordinates(source.cellDescription);
                int x = 0;
                int y = 0;
                char comma = 0;
                if (!(coordinates >> x >> comma >> y) || comma != ',')
                    return;
                cell = world->getExterior(x, y);
            }

            if (!cell)
                return;

            std::unique_ptr<ESM::FogState> fog(new ESM::FogState());
            fog->mNorthMarkerAngle = source.northMarkerAngle;
            fog->mBounds.mMinX = source.minX;
            fog->mBounds.mMinY = source.minY;
            fog->mBounds.mMaxX = source.maxX;
            fog->mBounds.mMaxY = source.maxY;
            fog->mFogTextures.reserve(source.textures.size());
            for (const LocalMapFogTexture& sourceTexture : source.textures)
            {
                ESM::FogTexture texture;
                texture.mX = sourceTexture.x;
                texture.mY = sourceTexture.y;
                texture.mImageData = sourceTexture.imageData;
                fog->mFogTextures.push_back(std::move(texture));
            }
            cell->setFog(fog.release());
            MWBase::Environment::get().getWindowManager()->refreshLocalMapFog(cell);
        }
    };
}

#endif
