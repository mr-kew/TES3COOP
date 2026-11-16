-- Persist local-map fog per player. This intentionally ignores shareMapExploration:
-- local fog is always personal and is never sent to other players.

local LOCAL_MAP_FOG_REQUEST = 0
local LOCAL_MAP_FOG_UPDATE = 1

local UINT32 = 4294967296

tes3mp.LogMessage(enumerations.log.VERBOSE, "[LocalMap] Persistence script loaded")

local function hashValue(value)
    local first = 5381
    local second = 0

    for index = 1, #value do
        local byte = string.byte(value, index)
        first = (first * 33 + byte) % UINT32
        second = (second * 65599 + byte) % UINT32
    end

    return string.format("%08x%08x", first, second)
end

local function readableFilePart(value)
    local encoded = string.gsub(value, "[^%w%-, _]", function(character)
        return string.format("%%%02X", string.byte(character))
    end)

    -- Keep Windows filenames comfortably below the per-component path limit,
    -- while preserving a readable prefix and a stable disambiguating suffix.
    if #encoded > 96 then
        encoded = string.sub(encoded, 1, 72) .. "_" .. hashValue(value)
    end

    return encoded
end

local function texturePath(pid, cellDescription, textureIndex, isInterior)
    local playerKey = hashValue(tes3mp.GetName(pid))
    if isInterior then
        local cellKey = hashValue(cellDescription)
        return config.dataPath .. "/map/" .. cellKey .. "_local_" .. playerKey .. "_" .. textureIndex .. ".png"
    end

    return config.dataPath .. "/map/" .. readableFilePart(cellDescription) ..
        "_local_" .. playerKey .. "_" .. textureIndex .. ".png"
end

customEventHooks.registerHandler("OnPlayerLocalMap", function(eventStatus, pid)
    tes3mp.ReadPlayerLocalMapFog(pid)
    local action = tes3mp.GetPlayerLocalMapFogAction()
    local cellDescription = tes3mp.GetPlayerLocalMapFogCellDescription()
    local player = Players[pid]

    local actionName = action == LOCAL_MAP_FOG_REQUEST and "request" or
        action == LOCAL_MAP_FOG_UPDATE and "update" or tostring(action)
    tes3mp.LogMessage(enumerations.log.VERBOSE, "[LocalMap] Received fog " .. actionName .. " from pid " .. pid ..
        " in " .. (cellDescription or "<no cell>") ..
        " with " .. tes3mp.GetPlayerLocalMapFogTextureCount() .. " textures")

    if config.persistLocalMap ~= true then
        -- Updates need an acknowledgement so the client can clear its pending
        -- copy, even when the server is configured not to persist the images.
        if action == LOCAL_MAP_FOG_UPDATE then
            tes3mp.AcknowledgePlayerLocalMapFog(pid)
        end
        return eventStatus
    end

    if player == nil then
        tes3mp.LogMessage(enumerations.log.WARN, "[LocalMap] Ignored fog packet because pid " .. pid .. " has no player")
        return eventStatus
    end
    if cellDescription == nil or cellDescription == "" then
        tes3mp.LogMessage(enumerations.log.WARN, "[LocalMap] Ignored fog packet with an empty cell description for pid " .. pid)
        return eventStatus
    end
    player.data = player.data or {}

    if action == LOCAL_MAP_FOG_REQUEST then
        local saved = player.data.localMapFog and player.data.localMapFog[cellDescription]
        if saved == nil then
            tes3mp.LogMessage(enumerations.log.VERBOSE, "[LocalMap] No saved fog for " .. tes3mp.GetName(pid) .. " in " .. cellDescription)
            return eventStatus
        end

        tes3mp.ClearPlayerLocalMapFog(pid)
        tes3mp.SetPlayerLocalMapFogCell(pid, cellDescription, saved.isInterior,
            saved.northMarkerAngle, saved.minX, saved.minY, saved.maxX, saved.maxY)

        for index, texture in ipairs(saved.textures or {}) do
            local path = texturePath(pid, cellDescription, index - 1, saved.isInterior)
            if not tes3mp.DoesFilePathExist(path) or
                not tes3mp.LoadPlayerLocalMapFogTextureFile(pid, texture.x, texture.y, path) then
                tes3mp.LogMessage(enumerations.log.ERROR, "[LocalMap] Could not load fog texture " .. path)
                return eventStatus
            end
        end

        tes3mp.SendPlayerLocalMapFog(pid)
        tes3mp.LogMessage(enumerations.log.VERBOSE, "[LocalMap] Restored fog for " .. tes3mp.GetName(pid) .. " in " .. cellDescription)
    elseif action == LOCAL_MAP_FOG_UPDATE then
        local textureCount = tes3mp.GetPlayerLocalMapFogTextureCount()
        if textureCount == 0 or textureCount > 128 then
            tes3mp.LogMessage(enumerations.log.WARN, "[LocalMap] Rejected fog update with " .. textureCount .. " textures for " .. cellDescription)
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
            local path = texturePath(pid, cellDescription, index, saved.isInterior)
            if not tes3mp.SavePlayerLocalMapFogTextureFile(index, path) or
                not tes3mp.DoesFilePathExist(path) then
                tes3mp.LogMessage(enumerations.log.ERROR, "[LocalMap] Could not save fog texture " .. path)
                return eventStatus
            end
            table.insert(saved.textures, { x = x, y = y })
        end

        if player.data.localMapFog == nil then
            player.data.localMapFog = {}
        end
        local previous = player.data.localMapFog[cellDescription]
        player.data.localMapFog[cellDescription] = saved
        player:QuicksaveToDrive()

        -- Remove extra tiles if the updated map has fewer textures.
        local oldTextureCount = previous and #(previous.textures or {}) or textureCount
        for index = textureCount, oldTextureCount - 1 do
            local oldPath = texturePath(pid, cellDescription, index, saved.isInterior)
            if tes3mp.DoesFilePathExist(oldPath) then
                local removed = os.remove(oldPath)
                if not removed and tes3mp.DoesFilePathExist(oldPath) then
                    tes3mp.LogMessage(enumerations.log.WARN,
                        "[LocalMap] Could not remove unused fog texture " .. oldPath)
                end
            end
        end

        tes3mp.AcknowledgePlayerLocalMapFog(pid)
        tes3mp.LogMessage(enumerations.log.VERBOSE, "[LocalMap] Saved fog for " .. tes3mp.GetName(pid) .. " in " .. cellDescription)
    end

    return eventStatus
end)

-- CoreScripts 0.8.1 has no OnPlayerLocalMap callback bridge. The C++ packet
-- processor looks up a global OnPlayerLocalMap function, so install the
-- eventHandler method and global wrapper here.
eventHandler.OnPlayerLocalMap = function(pid)
    if Players[pid] == nil or not Players[pid]:IsLoggedIn() then
        return
    end

    local eventStatus = customEventHooks.triggerValidators("OnPlayerLocalMap", { pid })
    customEventHooks.triggerHandlers("OnPlayerLocalMap", eventStatus, { pid })
end

function OnPlayerLocalMap(pid)
    eventHandler.OnPlayerLocalMap(pid)
end

return {}
