#include <components/openmw-mp/NetworkMessages.hpp>

#include <apps/openmw-mp/Networking.hpp>
#include <apps/openmw-mp/Player.hpp>
#include <apps/openmw-mp/Script/ScriptFunctions.hpp>
#include <apps/openmw-mp/CellController.hpp>
#include <fstream>
#include <utility>

#include <apps/openmw-mp/Utils.hpp>
#include <cstring>

#include "Worldstate.hpp"

using namespace mwmp;

BaseWorldstate *WorldstateFunctions::readWorldstate;
BaseWorldstate WorldstateFunctions::writeWorldstate;
BasePlayer *WorldstateFunctions::readLocalMapFogPlayer;

void WorldstateFunctions::ReadPlayerLocalMapFog(unsigned short pid) noexcept
{
    readLocalMapFogPlayer = Players::getPlayer(pid);
}

int WorldstateFunctions::GetPlayerLocalMapFogAction() noexcept
{
    return readLocalMapFogPlayer ? readLocalMapFogPlayer->localMapFogAction : LOCAL_MAP_FOG_REQUEST;
}

const char* WorldstateFunctions::GetPlayerLocalMapFogCellDescription() noexcept
{
    return readLocalMapFogPlayer ? readLocalMapFogPlayer->localMapFog.cellDescription.c_str() : "";
}

bool WorldstateFunctions::GetPlayerLocalMapFogIsInterior() noexcept
{
    return readLocalMapFogPlayer && readLocalMapFogPlayer->localMapFog.isInterior;
}

double WorldstateFunctions::GetPlayerLocalMapFogNorthAngle() noexcept
{
    return readLocalMapFogPlayer ? readLocalMapFogPlayer->localMapFog.northMarkerAngle : 0.0;
}

double WorldstateFunctions::GetPlayerLocalMapFogMinX() noexcept
{
    return readLocalMapFogPlayer ? readLocalMapFogPlayer->localMapFog.minX : 0.0;
}

double WorldstateFunctions::GetPlayerLocalMapFogMinY() noexcept
{
    return readLocalMapFogPlayer ? readLocalMapFogPlayer->localMapFog.minY : 0.0;
}

double WorldstateFunctions::GetPlayerLocalMapFogMaxX() noexcept
{
    return readLocalMapFogPlayer ? readLocalMapFogPlayer->localMapFog.maxX : 0.0;
}

double WorldstateFunctions::GetPlayerLocalMapFogMaxY() noexcept
{
    return readLocalMapFogPlayer ? readLocalMapFogPlayer->localMapFog.maxY : 0.0;
}

unsigned int WorldstateFunctions::GetPlayerLocalMapFogTextureCount() noexcept
{
    return readLocalMapFogPlayer ? static_cast<unsigned int>(readLocalMapFogPlayer->localMapFog.textures.size()) : 0;
}

int WorldstateFunctions::GetPlayerLocalMapFogTextureX(unsigned int index) noexcept
{
    if (!readLocalMapFogPlayer || index >= readLocalMapFogPlayer->localMapFog.textures.size())
        return 0;
    return readLocalMapFogPlayer->localMapFog.textures[index].x;
}

int WorldstateFunctions::GetPlayerLocalMapFogTextureY(unsigned int index) noexcept
{
    if (!readLocalMapFogPlayer || index >= readLocalMapFogPlayer->localMapFog.textures.size())
        return 0;
    return readLocalMapFogPlayer->localMapFog.textures[index].y;
}

bool WorldstateFunctions::SavePlayerLocalMapFogTextureFile(unsigned int index, const char* filePath) noexcept
{
    if (!readLocalMapFogPlayer || index >= readLocalMapFogPlayer->localMapFog.textures.size() || !filePath)
        return false;
    const std::vector<char>& image = readLocalMapFogPlayer->localMapFog.textures[index].imageData;
    if (image.empty() || image.size() > 8192)
        return false;
    std::ofstream output(filePath, std::ios::binary);
    if (!output)
        return false;
    output.write(image.data(), static_cast<std::streamsize>(image.size()));
    return output.good();
}

void WorldstateFunctions::ClearPlayerLocalMapFog(unsigned short pid) noexcept
{
    Player* player = Players::getPlayer(pid);
    if (!player)
        return;
    player->localMapFog.textures.clear();
    player->localMapFogAction = LOCAL_MAP_FOG_RESPONSE;
}

void WorldstateFunctions::SetPlayerLocalMapFogCell(unsigned short pid, const char* cellDescription, bool isInterior,
    double northMarkerAngle, double minX, double minY, double maxX, double maxY) noexcept
{
    Player* player = Players::getPlayer(pid);
    if (!player || !cellDescription || std::strlen(cellDescription) > 1024)
        return;
    auto& fog = player->localMapFog;
    fog.cellDescription = cellDescription;
    fog.isInterior = isInterior;
    fog.northMarkerAngle = static_cast<float>(northMarkerAngle);
    fog.minX = static_cast<float>(minX);
    fog.minY = static_cast<float>(minY);
    fog.maxX = static_cast<float>(maxX);
    fog.maxY = static_cast<float>(maxY);
    fog.textures.clear();
    player->localMapFogAction = LOCAL_MAP_FOG_RESPONSE;
}

bool WorldstateFunctions::LoadPlayerLocalMapFogTextureFile(unsigned short pid, int x, int y, const char* filePath) noexcept
{
    Player* player = Players::getPlayer(pid);
    if (!player || !filePath || player->localMapFog.textures.size() >= 128)
        return false;
    std::ifstream input(filePath, std::ios::binary);
    if (!input)
        return false;
    std::vector<char> image((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    if (image.empty() || image.size() > 8192)
        return false;
    size_t total = image.size();
    for (const auto& texture : player->localMapFog.textures)
        total += texture.imageData.size();
    if (total > 128 * 1024)
        return false;
    LocalMapFogTexture texture;
    texture.x = x;
    texture.y = y;
    texture.imageData = std::move(image);
    player->localMapFog.textures.push_back(std::move(texture));
    return true;
}

void WorldstateFunctions::SendPlayerLocalMapFog(unsigned short pid) noexcept
{
    Player* player = Players::getPlayer(pid);
    if (!player)
        return;
    PlayerPacket* packet = Networking::get().getPlayerPacketController()->GetPacket(ID_PLAYER_LOCAL_MAP);
    packet->setPlayer(player);
    packet->Send(false);
}

void WorldstateFunctions::AcknowledgePlayerLocalMapFog(unsigned short pid) noexcept
{
    Player* player = Players::getPlayer(pid);
    if (!player)
        return;
    player->localMapFog.textures.clear();
    player->localMapFogAction = LOCAL_MAP_FOG_ACK;
    PlayerPacket* packet = Networking::get().getPlayerPacketController()->GetPacket(ID_PLAYER_LOCAL_MAP);
    packet->setPlayer(player);
    packet->Send(false);
}


void WorldstateFunctions::ReadReceivedWorldstate() noexcept
{
    readWorldstate = mwmp::Networking::getPtr()->getReceivedWorldstate();
}

void WorldstateFunctions::CopyReceivedWorldstateToStore() noexcept
{
    writeWorldstate = *readWorldstate;
}

void WorldstateFunctions::ClearKillChanges() noexcept
{
    writeWorldstate.killChanges.clear();
}

void WorldstateFunctions::ClearMapChanges() noexcept
{
    writeWorldstate.mapTiles.clear();
}

void WorldstateFunctions::ClearClientGlobals() noexcept
{
    writeWorldstate.clientGlobals.clear();
}

unsigned int WorldstateFunctions::GetKillChangesSize() noexcept
{
    return readWorldstate->killChanges.size();
}

unsigned int WorldstateFunctions::GetMapChangesSize() noexcept
{
    return readWorldstate->mapTiles.size();
}

unsigned int WorldstateFunctions::GetClientGlobalsSize() noexcept
{
    return readWorldstate->clientGlobals.size();
}

const char *WorldstateFunctions::GetKillRefId(unsigned int index) noexcept
{
    return readWorldstate->killChanges.at(index).refId.c_str();
}

int WorldstateFunctions::GetKillNumber(unsigned int index) noexcept
{
    return readWorldstate->killChanges.at(index).number;
}

const char *WorldstateFunctions::GetWeatherRegion() noexcept
{
    return readWorldstate->weather.region.c_str();
}

int WorldstateFunctions::GetWeatherCurrent() noexcept
{
    return readWorldstate->weather.currentWeather;
}

int WorldstateFunctions::GetWeatherNext() noexcept
{
    return readWorldstate->weather.nextWeather;
}

int WorldstateFunctions::GetWeatherQueued() noexcept
{
    return readWorldstate->weather.queuedWeather;
}

double WorldstateFunctions::GetWeatherTransitionFactor() noexcept
{
    return readWorldstate->weather.transitionFactor;
}

int WorldstateFunctions::GetMapTileCellX(unsigned int index) noexcept
{
    return readWorldstate->mapTiles.at(index).x;
}

int WorldstateFunctions::GetMapTileCellY(unsigned int index) noexcept
{
    return readWorldstate->mapTiles.at(index).y;
}

const char *WorldstateFunctions::GetClientGlobalId(unsigned int index) noexcept
{
    return readWorldstate->clientGlobals.at(index).id.c_str();
}

unsigned short WorldstateFunctions::GetClientGlobalVariableType(unsigned int index) noexcept
{
    return readWorldstate->clientGlobals.at(index).variableType;
}

int WorldstateFunctions::GetClientGlobalIntValue(unsigned int index) noexcept
{
    return readWorldstate->clientGlobals.at(index).intValue;
}

double WorldstateFunctions::GetClientGlobalFloatValue(unsigned int index) noexcept
{
    return readWorldstate->clientGlobals.at(index).floatValue;
}

void WorldstateFunctions::SetAuthorityRegion(const char* authorityRegion) noexcept
{
    writeWorldstate.authorityRegion = authorityRegion;
}

void WorldstateFunctions::SetWeatherRegion(const char* region) noexcept
{
    writeWorldstate.weather.region = region;
}

void WorldstateFunctions::SetWeatherForceState(bool forceState) noexcept
{
    writeWorldstate.forceWeather = forceState;
}

void WorldstateFunctions::SetWeatherCurrent(int currentWeather) noexcept
{
    writeWorldstate.weather.currentWeather = currentWeather;
}

void WorldstateFunctions::SetWeatherNext(int nextWeather) noexcept
{
    writeWorldstate.weather.nextWeather = nextWeather;
}

void WorldstateFunctions::SetWeatherQueued(int queuedWeather) noexcept
{
    writeWorldstate.weather.queuedWeather = queuedWeather;
}

void WorldstateFunctions::SetWeatherTransitionFactor(double transitionFactor) noexcept
{
    writeWorldstate.weather.transitionFactor = transitionFactor;
}

void WorldstateFunctions::SetHour(double hour) noexcept
{
    writeWorldstate.time.hour = hour;
}

void WorldstateFunctions::SetDay(int day) noexcept
{
    writeWorldstate.time.day = day;
}

void WorldstateFunctions::SetMonth(int month) noexcept
{
    writeWorldstate.time.month = month;
}

void WorldstateFunctions::SetYear(int year) noexcept
{
    writeWorldstate.time.year = year;
}

void WorldstateFunctions::SetDaysPassed(int daysPassed) noexcept
{
    writeWorldstate.time.daysPassed = daysPassed;
}

void WorldstateFunctions::SetTimeScale(double timeScale) noexcept
{
    writeWorldstate.time.timeScale = timeScale;
}

void WorldstateFunctions::SetPlayerCollisionState(bool state) noexcept
{
    writeWorldstate.hasPlayerCollision = state;
}

void WorldstateFunctions::SetActorCollisionState(bool state) noexcept
{
    writeWorldstate.hasActorCollision = state;
}

void WorldstateFunctions::SetPlacedObjectCollisionState(bool state) noexcept
{
    writeWorldstate.hasPlacedObjectCollision = state;
}

void WorldstateFunctions::UseActorCollisionForPlacedObjects(bool useActorCollision) noexcept
{
    writeWorldstate.useActorCollisionForPlacedObjects = useActorCollision;
}

void WorldstateFunctions::AddKill(const char* refId, int number) noexcept
{
    mwmp::Kill kill;
    kill.refId = refId;
    kill.number = number;

    writeWorldstate.killChanges.push_back(kill);
}

void WorldstateFunctions::AddClientGlobalInteger(const char* id, int intValue, unsigned int variableType) noexcept
{
    mwmp::ClientVariable clientVariable;
    clientVariable.id = id;
    clientVariable.variableType = variableType;
    clientVariable.intValue = intValue;

    writeWorldstate.clientGlobals.push_back(clientVariable);
}

void WorldstateFunctions::AddClientGlobalFloat(const char* id, double floatValue) noexcept
{
    mwmp::ClientVariable clientVariable;
    clientVariable.id = id;
    clientVariable.variableType = mwmp::VARIABLE_TYPE::FLOAT;
    clientVariable.floatValue = floatValue;

    writeWorldstate.clientGlobals.push_back(clientVariable);
}

void WorldstateFunctions::AddSynchronizedClientScriptId(const char *scriptId) noexcept
{
    writeWorldstate.synchronizedClientScriptIds.push_back(scriptId);
}

void WorldstateFunctions::AddSynchronizedClientGlobalId(const char *globalId) noexcept
{
    writeWorldstate.synchronizedClientGlobalIds.push_back(globalId);
}

void WorldstateFunctions::AddEnforcedCollisionRefId(const char *refId) noexcept
{
    writeWorldstate.enforcedCollisionRefIds.push_back(refId);
}

void WorldstateFunctions::AddCellToReset(const char *cellDescription) noexcept
{
    ESM::Cell cell = Utils::getCellFromDescription(cellDescription);
    writeWorldstate.cellsToReset.push_back(cell);
}

void WorldstateFunctions::AddDestinationOverride(const char *oldCellDescription, const char *newCellDescription) noexcept
{
    writeWorldstate.destinationOverrides[oldCellDescription] = newCellDescription;
}

void WorldstateFunctions::ClearSynchronizedClientScriptIds() noexcept
{
    writeWorldstate.synchronizedClientScriptIds.clear();
}

void WorldstateFunctions::ClearSynchronizedClientGlobalIds() noexcept
{
    writeWorldstate.synchronizedClientGlobalIds.clear();
}

void WorldstateFunctions::ClearEnforcedCollisionRefIds() noexcept
{
    writeWorldstate.enforcedCollisionRefIds.clear();
}

void WorldstateFunctions::ClearCellsToReset() noexcept
{
    writeWorldstate.cellsToReset.clear();
}

void WorldstateFunctions::ClearDestinationOverrides() noexcept
{
    writeWorldstate.destinationOverrides.clear();
}

void WorldstateFunctions::SaveMapTileImageFile(unsigned int index, const char *filePath) noexcept
{
    if (index >= readWorldstate->mapTiles.size())
        return;

    const std::vector<char>& imageData = readWorldstate->mapTiles.at(index).imageData;

    std::ofstream outputFile(filePath, std::ios::binary);
    std::ostream_iterator<char> outputIterator(outputFile);
    std::copy(imageData.begin(), imageData.end(), outputIterator);
}

void WorldstateFunctions::LoadMapTileImageFile(int cellX, int cellY, const char* filePath) noexcept
{
    mwmp::MapTile mapTile;
    mapTile.x = cellX;
    mapTile.y = cellY;

    std::ifstream inputFile(filePath, std::ios::binary);
    mapTile.imageData = std::vector<char>(std::istreambuf_iterator<char>(inputFile), std::istreambuf_iterator<char>());

    if (mapTile.imageData.size() > mwmp::maxImageDataSize)
    {
        LOG_MESSAGE_SIMPLE(TimedLog::LOG_ERROR, "Error loading image file for map tile: "
            "%s has a size of %i, which is over the maximum allowed of %i!",
            filePath, mapTile.imageData.size(), mwmp::maxImageDataSize);
    }
    else
    {
        writeWorldstate.mapTiles.push_back(mapTile);
    }
}

void WorldstateFunctions::SendClientScriptGlobal(unsigned short pid, bool sendToOtherPlayers, bool skipAttachedPlayer) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, );

    writeWorldstate.guid = player->guid;

    mwmp::WorldstatePacket *packet = mwmp::Networking::get().getWorldstatePacketController()->GetPacket(ID_CLIENT_SCRIPT_GLOBAL);
    packet->setWorldstate(&writeWorldstate);

    if (!skipAttachedPlayer)
        packet->Send(false);
    if (sendToOtherPlayers)
        packet->Send(true);
}

void WorldstateFunctions::SendClientScriptSettings(unsigned short pid, bool sendToOtherPlayers, bool skipAttachedPlayer) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, );

    writeWorldstate.guid = player->guid;

    mwmp::WorldstatePacket *packet = mwmp::Networking::get().getWorldstatePacketController()->GetPacket(ID_CLIENT_SCRIPT_SETTINGS);
    packet->setWorldstate(&writeWorldstate);

    if (!skipAttachedPlayer)
        packet->Send(false);
    if (sendToOtherPlayers)
        packet->Send(true);
}

void WorldstateFunctions::SendWorldKillCount(unsigned short pid, bool sendToOtherPlayers, bool skipAttachedPlayer) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, );

    writeWorldstate.guid = player->guid;

    mwmp::WorldstatePacket *packet = mwmp::Networking::get().getWorldstatePacketController()->GetPacket(ID_WORLD_KILL_COUNT);
    packet->setWorldstate(&writeWorldstate);

    if (!skipAttachedPlayer)
        packet->Send(false);
    if (sendToOtherPlayers)
        packet->Send(true);
}

void WorldstateFunctions::SendWorldMap(unsigned short pid, bool sendToOtherPlayers, bool skipAttachedPlayer) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, );

    writeWorldstate.guid = player->guid;

    mwmp::WorldstatePacket *packet = mwmp::Networking::get().getWorldstatePacketController()->GetPacket(ID_WORLD_MAP);
    packet->setWorldstate(&writeWorldstate);

    if (!skipAttachedPlayer)
        packet->Send(false);
    if (sendToOtherPlayers)
        packet->Send(true);
}

void WorldstateFunctions::SendWorldTime(unsigned short pid, bool sendToOtherPlayers, bool skipAttachedPlayer) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, );

    writeWorldstate.guid = player->guid;

    mwmp::WorldstatePacket *packet = mwmp::Networking::get().getWorldstatePacketController()->GetPacket(ID_WORLD_TIME);
    packet->setWorldstate(&writeWorldstate);
    
    if (!skipAttachedPlayer)
        packet->Send(false);
    if (sendToOtherPlayers)
        packet->Send(true);
}

void WorldstateFunctions::SendWorldWeather(unsigned short pid, bool sendToOtherPlayers, bool skipAttachedPlayer) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, );

    writeWorldstate.guid = player->guid;

    mwmp::WorldstatePacket *packet = mwmp::Networking::get().getWorldstatePacketController()->GetPacket(ID_WORLD_WEATHER);
    packet->setWorldstate(&writeWorldstate);

    if (!skipAttachedPlayer)
        packet->Send(false);
    if (sendToOtherPlayers)
        packet->Send(true);
}

void WorldstateFunctions::SendWorldCollisionOverride(unsigned short pid, bool sendToOtherPlayers, bool skipAttachedPlayer) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, );

    writeWorldstate.guid = player->guid;

    mwmp::WorldstatePacket *packet = mwmp::Networking::get().getWorldstatePacketController()->GetPacket(ID_WORLD_COLLISION_OVERRIDE);
    packet->setWorldstate(&writeWorldstate);

    if (!skipAttachedPlayer)
        packet->Send(false);
    if (sendToOtherPlayers)
        packet->Send(true);
}

void WorldstateFunctions::SendCellReset(unsigned short pid, bool sendToOtherPlayers) noexcept
{
    mwmp::WorldstatePacket *packet = mwmp::Networking::get().getWorldstatePacketController()->GetPacket(ID_CELL_RESET);

    Player *player;
    GET_PLAYER(pid, player, );

    writeWorldstate.guid = player->guid;

    packet->setWorldstate(&writeWorldstate);

    packet->Send(sendToOtherPlayers);

    if (sendToOtherPlayers)
    {
        packet->Send(false);
    }
}

void WorldstateFunctions::SendWorldDestinationOverride(unsigned short pid, bool sendToOtherPlayers, bool skipAttachedPlayer) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, );

    writeWorldstate.guid = player->guid;

    mwmp::WorldstatePacket *packet = mwmp::Networking::get().getWorldstatePacketController()->GetPacket(ID_WORLD_DESTINATION_OVERRIDE);
    packet->setWorldstate(&writeWorldstate);

    if (!skipAttachedPlayer)
        packet->Send(false);
    if (sendToOtherPlayers)
        packet->Send(true);
}

void WorldstateFunctions::SendWorldRegionAuthority(unsigned short pid) noexcept
{
    Player *player;
    GET_PLAYER(pid, player, );

    writeWorldstate.guid = player->guid;

    mwmp::WorldstatePacket *packet = mwmp::Networking::get().getWorldstatePacketController()->GetPacket(ID_WORLD_REGION_AUTHORITY);
    packet->setWorldstate(&writeWorldstate);

    packet->Send(false);

    // This packet should always be sent to all other players
    packet->Send(true);
}

// All methods below are deprecated versions of methods from above

void WorldstateFunctions::ReadLastWorldstate() noexcept
{
    ReadReceivedWorldstate();
}

void WorldstateFunctions::CopyLastWorldstateToStore() noexcept
{
    CopyReceivedWorldstateToStore();
}
