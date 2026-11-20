-- Persist each player's permanent NPC dispositions, independently of cell.
-- Requires the matching TES3COOP client/server build.

local NPC_DISPOSITION = 10
local OBJECTS_PER_PACKET = 100
local BaseCell = require("cell.base")

-- CoreScripts logs every dialogue-choice type by looking it up in this table.
-- Register the custom packet type so its normal handler can acknowledge it.
enumerations.dialogueChoice.NPC_DISPOSITION = NPC_DISPOSITION

local function getDispositions(player)
    if player.data.npcDispositions == nil then
        player.data.npcDispositions = {}
    end

    return player.data.npcDispositions
end

local function beginPacket(pid, cellDescription)
    tes3mp.ClearObjectList()
    tes3mp.SetObjectListPid(pid)
    tes3mp.SetObjectListCell(cellDescription)
end

local function addNpcDisposition(refId, uniqueIndex, disposition)
    local refNum, mpNum = string.match(uniqueIndex, "^(%d+)%-(%d+)$")
    if refNum == nil or mpNum == nil then
        return false
    end

    tes3mp.SetObjectRefNum(tonumber(refNum))
    tes3mp.SetObjectMpNum(tonumber(mpNum))
    tes3mp.SetObjectRefId(refId)
    tes3mp.SetObjectDialogueChoiceType(NPC_DISPOSITION)
    tes3mp.SetObjectDialogueChoiceDisposition(disposition)
    tes3mp.AddObject()
    return true
end

local function sendDispositions(pid, cellDescription, entries)
    for firstIndex = 1, #entries, OBJECTS_PER_PACKET do
        beginPacket(pid, cellDescription)
        local lastIndex = math.min(firstIndex + OBJECTS_PER_PACKET - 1, #entries)
        for index = firstIndex, lastIndex do
            local entry = entries[index]
            addNpcDisposition(entry.refId, entry.uniqueIndex, entry.disposition)
        end
        tes3mp.SendObjectDialogueChoice(false, false)
    end
end

local originalLoadMomentaryCellData = BaseCell.LoadMomentaryCellData
BaseCell.LoadMomentaryCellData = function(self, pid)
    originalLoadMomentaryCellData(self, pid)

    if config.persistNpcDispositions ~= true then
        return
    end

    local player = Players[pid]
    if player == nil then
        return
    end

    local dispositions = getDispositions(player)
    local entries = {}

    -- NPC records follow their stable identity, not the cell where the change
    -- was first made. The client applies a record only when that NPC is in the
    -- loaded cell; unrelated identities in the same packet are ignored.
    for refId, byUniqueIndex in pairs(dispositions) do
        for uniqueIndex, disposition in pairs(byUniqueIndex) do
            if string.match(uniqueIndex, "^(%d+)%-(%d+)$") then
                table.insert(entries, {
                    refId = refId,
                    uniqueIndex = uniqueIndex,
                    disposition = disposition
                })
            end
        end
    end

    if #entries > 0 then
        -- Send only to the player whose disposition data is being restored.
        sendDispositions(pid, self.description, entries)
    end
end

-- Cell loading covers NPCs already in a cell. This handler covers actors
-- which enter a cell that was already loaded, such as followers crossing a
-- cell boundary while following a player.
customEventHooks.registerHandler("OnActorCellChange", function(eventStatus)
    if config.persistNpcDispositions ~= true or eventStatus.validDefaultHandler == false then
        return eventStatus
    end

    tes3mp.ReadReceivedActorList()
    local actorsByCell = {}
    for index = 0, tes3mp.GetActorListSize() - 1 do
        local cellDescription = tes3mp.GetActorCell(index)
        actorsByCell[cellDescription] = actorsByCell[cellDescription] or {}
        table.insert(actorsByCell[cellDescription], {
            refId = tes3mp.GetActorRefId(index),
            uniqueIndex = tes3mp.GetActorRefNum(index) .. "-" .. tes3mp.GetActorMpNum(index)
        })
    end

    for cellDescription, actors in pairs(actorsByCell) do
        local cell = LoadedCells[cellDescription]
        if cell ~= nil then
            for _, visitorPid in pairs(cell.visitors) do
                local visitor = Players[visitorPid]
                if visitor ~= nil then
                    local dispositions = getDispositions(visitor)
                    local entries = {}
                    for _, actor in pairs(actors) do
                        local npcDispositions = dispositions[actor.refId]
                        local disposition = npcDispositions and npcDispositions[actor.uniqueIndex]
                        if disposition ~= nil then
                            table.insert(entries, {
                                refId = actor.refId,
                                uniqueIndex = actor.uniqueIndex,
                                disposition = disposition
                            })
                        end
                    end
                    if #entries > 0 then
                        sendDispositions(visitorPid, cellDescription, entries)
                    end
                end
            end
        end
    end

    return eventStatus
end)

customEventHooks.registerHandler("OnObjectDialogueChoice", function(eventStatus, pid, cellDescription)
    if config.persistNpcDispositions ~= true or eventStatus.validDefaultHandler == false then
        return eventStatus
    end

    local player = Players[pid]
    if player == nil then
        return eventStatus
    end

    local dispositions = getDispositions(player)
    local changed = false
    for index = 0, tes3mp.GetObjectListSize() - 1 do
        if tes3mp.GetObjectDialogueChoiceType(index) == NPC_DISPOSITION then
            local refId = tes3mp.GetObjectRefId(index)
            local uniqueIndex = tes3mp.GetObjectRefNum(index) .. "-" .. tes3mp.GetObjectMpNum(index)
            dispositions[refId] = dispositions[refId] or {}
            dispositions[refId][uniqueIndex] = tes3mp.GetObjectDialogueChoiceDisposition(index)
            changed = true
        end
    end

    if changed then
        player:QuicksaveToDrive()
    end

    return eventStatus
end)

return {}
