-- Persist vanilla cell respawn state without resetting unrelated quest data.
-- The cell authority runs the native OpenMW respawn routines; the server
-- validates that authority, persists the narrow delta, and distributes it.

local ACTION_UPDATE = 0
local ACTION_ACK = 1
local ACTION_DEFER = 2
local ACTION_APPLY = 3
local ACTION_STATE = 4

local function sendAction(pid, action)
    tes3mp.SetPlayerCellRespawnAction(pid, action)
    tes3mp.SendPlayerCellRespawn(pid, false, false)
end

local function stateForCellLoad(pid, cellDescription, cell)
    local lastRespawn = cell.data.loadState.lastRespawn
    if lastRespawn == nil then
        for index = 0, tes3mp.GetCellStateChangesSize(pid) - 1 do
            if tes3mp.GetCellStateType(pid, index) == 0 and
                tes3mp.GetCellStateDescription(pid, index) == cellDescription then
                lastRespawn = {
                    hour = tes3mp.GetCellStateLastRespawnHour(pid, index),
                    day = tes3mp.GetCellStateLastRespawnDay(pid, index)
                }
                cell.data.loadState.lastRespawn = lastRespawn
                cell:QuicksaveToDrive()
                break
            end
        end
    end

    -- A cell with no prior server save starts with the vanilla default
    -- timestamp. Always answer the load so stale client save data cannot win.
    if lastRespawn == nil then
        lastRespawn = { hour = 0, day = 0 }
        cell.data.loadState.lastRespawn = lastRespawn
        cell:QuicksaveToDrive()
    end
    tes3mp.SetPlayerCellRespawnState(pid, cellDescription, lastRespawn.hour, lastRespawn.day,
        cell:GetVisitorCount() == 1)
    tes3mp.SetPlayerCellRespawnAction(pid, ACTION_STATE)
    tes3mp.SendPlayerCellRespawn(pid, false, false)
end

local function updateCellStateFromPacket(pid, cellDescription, cell, requiredAction)
    for index = 0, tes3mp.GetCellStateChangesSize(pid) - 1 do
        if tes3mp.GetCellStateType(pid, index) == requiredAction and
            tes3mp.GetCellStateDescription(pid, index) == cellDescription then
            local hour = tes3mp.GetCellStateLastRespawnHour(pid, index)
            local day = tes3mp.GetCellStateLastRespawnDay(pid, index)
            local previous = cell.data.loadState.lastRespawn
            if previous == nil or day > previous.day or (day == previous.day and hour > previous.hour) then
                cell.data.loadState.lastRespawn = { hour = hour, day = day }
                cell:QuicksaveToDrive()
            end
            return
        end
    end
end

local function removeFromPacketLists(cell, uniqueIndex, packetNames)
    for _, packetName in ipairs(packetNames) do
        local list = cell.data.packets[packetName]
        if list ~= nil then
            tableHelper.removeValue(list, uniqueIndex)
        end
    end
end

local function clearRespawnedReference(cell, uniqueIndex, respawnType)
    local objectData = cell.data.objectData[uniqueIndex]
    if objectData == nil then
        return
    end

    if respawnType == 0 then
        objectData.inventory = nil
        removeFromPacketLists(cell, uniqueIndex, {"container"})
        return
    end

    -- Match the fields reset by native NPC/creature respawn. Keep refId,
    -- script locals, locks, dispositions, and unrelated quest state.
    objectData.location = nil
    objectData.stats = nil
    objectData.equipment = nil
    objectData.inventory = nil
    objectData.spellsActive = nil
    objectData.ai = nil
    objectData.death = nil
    removeFromPacketLists(cell, uniqueIndex,
        {"position", "statsDynamic", "equipment", "container", "spellsActive", "ai", "death"})
end

local function reconcileMovedReference(origin, currentCellDescription, uniqueIndex)
    if currentCellDescription == "" or currentCellDescription == origin.description then
        return
    end

    local temporarilyLoaded = false
    if LoadedCells[currentCellDescription] == nil then
        logicHandler.LoadCell(currentCellDescription)
        temporarilyLoaded = true
    end

    local currentCell = LoadedCells[currentCellDescription]
    local originData = origin.data.objectData[uniqueIndex]
    local currentData = currentCell.data.objectData[uniqueIndex]

    if originData ~= nil then
        originData.cellChangeTo = nil
        tableHelper.removeValue(origin.data.packets.cellChangeTo, uniqueIndex)
    end
    if currentData ~= nil and currentData.cellChangeFrom == origin.description then
        if originData ~= nil and currentData.variables ~= nil then
            originData.variables = originData.variables or {}
            for variableType, variables in pairs(currentData.variables) do
                originData.variables[variableType] = originData.variables[variableType] or {}
                for internalIndex, value in pairs(variables) do
                    originData.variables[variableType][internalIndex] = value
                end
            end
        end
        currentCell:DeleteObjectData(uniqueIndex)
        tableHelper.removeValue(currentCell.data.packets.cellChangeFrom, uniqueIndex)
        currentCell:QuicksaveToDrive()
    end

    if temporarilyLoaded then
        logicHandler.UnloadCell(currentCellDescription)
    end
end

customEventHooks.registerHandler("OnCellLoad", function(eventStatus, pid, cellDescription)
    if eventStatus.validDefaultHandler == false or Players[pid] == nil or not Players[pid]:IsLoggedIn() then
        return eventStatus
    end

    local cell = LoadedCells[cellDescription]
    if cell ~= nil then
        stateForCellLoad(pid, cellDescription, cell)
    end
    return eventStatus
end)

customEventHooks.registerHandler("OnCellUnload", function(eventStatus, pid, cellDescription)
    if Players[pid] == nil or not Players[pid]:IsLoggedIn() then
        return eventStatus
    end
    local cell = LoadedCells[cellDescription]
    if cell ~= nil then
        updateCellStateFromPacket(pid, cellDescription, cell, 1)
    end
    return eventStatus
end)

customEventHooks.registerHandler("OnPlayerCellRespawn", function(eventStatus, pid)
    if eventStatus.validDefaultHandler == false or Players[pid] == nil or not Players[pid]:IsLoggedIn() then
        return eventStatus
    end

    tes3mp.ReadPlayerCellRespawn(pid)
    if tes3mp.GetPlayerCellRespawnAction() ~= ACTION_UPDATE then
        return eventStatus
    end

    local cellDescription = tes3mp.GetPlayerCellRespawnCellDescription()
    local cell = LoadedCells[cellDescription]
    if cell == nil or cell.authority ~= pid then
        sendAction(pid, ACTION_DEFER)
        return eventStatus
    end

    local reportedHour = tes3mp.GetPlayerCellRespawnLastRespawnHour()
    local reportedDay = tes3mp.GetPlayerCellRespawnLastRespawnDay()
    local canonical = cell.data.loadState.lastRespawn or { hour = 0, day = 0 }
    if reportedDay > canonical.day or (reportedDay == canonical.day and reportedHour > canonical.hour) then
        canonical = { hour = reportedHour, day = reportedDay }
        cell.data.loadState.lastRespawn = canonical
        cell:QuicksaveToDrive()
    end

    local referenceCount = tes3mp.GetPlayerCellRespawnReferenceCount()
    for index = 0, referenceCount - 1 do
        local refNum = tes3mp.GetPlayerCellRespawnReferenceRefNum(index)
        local mpNum = tes3mp.GetPlayerCellRespawnReferenceMpNum(index)
        local respawnType = tes3mp.GetPlayerCellRespawnReferenceType(index)
        local uniqueIndex = refNum .. "-" .. mpNum
        local currentCell = tes3mp.GetPlayerCellRespawnReferenceCurrentCell(index)

        if respawnType <= 2 then
            clearRespawnedReference(cell, uniqueIndex, respawnType)
            if respawnType == 1 then
                reconcileMovedReference(cell, currentCell, uniqueIndex)
            end
        end
    end

    cell:QuicksaveToDrive()

    tes3mp.ReadPlayerCellRespawn(pid)
    tes3mp.SetPlayerCellRespawnAction(pid, ACTION_ACK)
    tes3mp.SendPlayerCellRespawn(pid, false, false)
    tes3mp.SetPlayerCellRespawnLastRespawn(pid, canonical.hour, canonical.day)
    tes3mp.SetPlayerCellRespawnAction(pid, ACTION_APPLY)
    tes3mp.SendPlayerCellRespawn(pid, true, false)

    return eventStatus
end)

eventHandler.OnPlayerCellRespawn = function(pid)
    if Players[pid] == nil or not Players[pid]:IsLoggedIn() then
        return
    end

    local eventStatus = customEventHooks.triggerValidators("OnPlayerCellRespawn", { pid })
    customEventHooks.triggerHandlers("OnPlayerCellRespawn", eventStatus, { pid })
end

function OnPlayerCellRespawn(pid)
    eventHandler.OnPlayerCellRespawn(pid)
end

return {}
