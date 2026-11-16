-- Persist local-map fog per player. This intentionally ignores shareMapExploration:
-- local fog is always personal and is never sent to other players.

local LOCAL_MAP_FOG_REQUEST = 0
local LOCAL_MAP_FOG_UPDATE = 1

local UINT32 = 4294967296

tes3mp.LogMessage(0, "[LocalMap] Persistence script loaded")

local function fogFileKey(playerName, cellDescription)
    local value = playerName .. "\0" .. cellDescription
    local first = 5381
    local second = 0

    for index = 1, #value do
        local byte = string.byte(value, index)
        first = (first * 33 + byte) % UINT32
        second = (second * 65599 + byte) % UINT32
    end

    return string.format("%08x%08x", first, second)
end

local function texturePath(pid, cellDescription, textureIndex)
    local playerName = tes3mp.GetName(pid)
    local key = fogFileKey(playerName, cellDescription)
    return config.dataPath .. "/map/local_" .. key .. "_" .. textureIndex .. ".png"
end

local function legacyTexturePath(pid, cellDescription, textureIndex, version)
    local playerName = tes3mp.GetName(pid)
    local key = fogFileKey(playerName, cellDescription)
    return config.dataPath .. "/map/local_" .. key .. "_" .. version .. "_" .. textureIndex .. ".png"
end

customEventHooks.registerHandler("OnPlayerLocalMap", function(eventStatus, pid)
    tes3mp.ReadPlayerLocalMapFog(pid)
    local action = tes3mp.GetPlayerLocalMapFogAction()
    local cellDescription = tes3mp.GetPlayerLocalMapFogCellDescription()
    local player = Players[pid]

    tes3mp.LogMessage(0, "[LocalMap] Received action " .. action .. " for pid " .. pid ..
        " in " .. (cellDescription or "<no cell>") ..
        " with " .. tes3mp.GetPlayerLocalMapFogTextureCount() .. " textures")

    if player == nil then
        tes3mp.LogMessage(2, "[LocalMap] Ignored fog packet because pid " .. pid .. " has no player")
        return eventStatus
    end
    if cellDescription == nil or cellDescription == "" then
        tes3mp.LogMessage(2, "[LocalMap] Ignored fog packet with an empty cell description for pid " .. pid)
        return eventStatus
    end
    player.data = player.data or {}

    if action == LOCAL_MAP_FOG_REQUEST then
        local saved = player.data.localMapFog and player.data.localMapFog[cellDescription]
        if saved == nil then
            tes3mp.LogMessage(0, "[LocalMap] No saved fog for " .. tes3mp.GetName(pid) .. " in " .. cellDescription)
            return eventStatus
        end

        tes3mp.ClearPlayerLocalMapFog(pid)
        tes3mp.SetPlayerLocalMapFogCell(pid, cellDescription, saved.isInterior,
            saved.northMarkerAngle, saved.minX, saved.minY, saved.maxX, saved.maxY)

        for index, texture in ipairs(saved.textures or {}) do
            local path = texturePath(pid, cellDescription, index - 1)
            if not tes3mp.DoesFilePathExist(path) and saved.version ~= nil then
                path = legacyTexturePath(pid, cellDescription, index - 1, saved.version)
            end
            if not tes3mp.DoesFilePathExist(path) or
                not tes3mp.LoadPlayerLocalMapFogTextureFile(pid, texture.x, texture.y, path) then
                tes3mp.LogMessage(3, "[LocalMap] Could not load fog texture " .. path)
                return eventStatus
            end
        end

        tes3mp.SendPlayerLocalMapFog(pid)
        tes3mp.LogMessage(0, "[LocalMap] Restored fog for " .. tes3mp.GetName(pid) .. " in " .. cellDescription)
    elseif action == LOCAL_MAP_FOG_UPDATE then
        local textureCount = tes3mp.GetPlayerLocalMapFogTextureCount()
        if textureCount == 0 or textureCount > 128 then
            tes3mp.LogMessage(2, "[LocalMap] Rejected fog update with " .. textureCount .. " textures for " .. cellDescription)
            return eventStatus
        end

        player.data = player.data or {}
        local saved = {
            isInterior = tes3mp.GetPlayerLocalMapFogIsInterior(),
            northMarkerAngle = tes3mp.GetPlayerLocalMapFogNorthAngle(),
            minX = tes3mp.GetPlayerLocalMapFogMinX(),
            minY = tes3mp.GetPlayerLocalMapFogMinY(),
            maxX = tes3mp.GetPlayerLocalMapFogMaxX(),
            maxY = tes3mp.GetPlayerLocalMapFogMaxY(),
            textures = {}
        }

        for index = 0, textureCount - 1 do
            local x = tes3mp.GetPlayerLocalMapFogTextureX(index)
            local y = tes3mp.GetPlayerLocalMapFogTextureY(index)
            local path = texturePath(pid, cellDescription, index)
            if not tes3mp.SavePlayerLocalMapFogTextureFile(index, path) or
                not tes3mp.DoesFilePathExist(path) then
                tes3mp.LogMessage(3, "[LocalMap] Could not save fog texture " .. path)
                return eventStatus
            end
            table.insert(saved.textures, { x = x, y = y })
        end

        if player.data.localMapFog == nil then
            player.data.localMapFog = {}
        end
        player.data.localMapFog[cellDescription] = saved
        player:QuicksaveToDrive()

        tes3mp.AcknowledgePlayerLocalMapFog(pid)
        tes3mp.LogMessage(0, "[LocalMap] Saved fog for " .. tes3mp.GetName(pid) .. " in " .. cellDescription)
    end

    return eventStatus
end)

return {}
