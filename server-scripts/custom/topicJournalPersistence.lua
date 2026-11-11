-- Persist the dialogue responses shown in the Topics section of the journal.
-- Requires a TES3MP build with TOPIC_ENTRY support in PlayerJournal packets.

local TOPIC_ENTRY = 2

local packetReader = require("packetReader")
local originalGetPlayerPacketTables = packetReader.GetPlayerPacketTables

packetReader.GetPlayerPacketTables = function(pid, packetType)
    local packetTable = originalGetPlayerPacketTables(pid, packetType)

    if packetType == "PlayerJournal" then
        local journalIndex = 1

        for changesIndex = 0, tes3mp.GetJournalChangesSize(pid) - 1 do
            if tes3mp.GetJournalItemType(pid, changesIndex) == TOPIC_ENTRY then
                local journalItem = packetTable.journal[journalIndex]
                journalItem.topicId = tes3mp.GetJournalItemTopicId(pid, changesIndex)
                journalItem.infoId = tes3mp.GetJournalItemInfoId(pid, changesIndex)
                journalItem.text = tes3mp.GetJournalItemText(pid, changesIndex)
                journalItem.actorName = tes3mp.GetJournalItemActorName(pid, changesIndex)
            end

            journalIndex = journalIndex + 1
        end
    end

    return packetTable
end

-- Replace only the journal load method. The standard method still handles
-- quest journal entries and indices; this adds dialogue response records.
local StateHelper = require("stateHelper")
local originalLoadJournal = StateHelper.LoadJournal

StateHelper.LoadJournal = function(self, pid, stateObject)
    local regularJournal = {}
    local topicJournal = {}

    for _, journalItem in ipairs(stateObject.data.journal or {}) do
        if journalItem.type == TOPIC_ENTRY then
            table.insert(topicJournal, journalItem)
        else
            table.insert(regularJournal, journalItem)
        end
    end

    local regularState = { data = { journal = regularJournal } }
    originalLoadJournal(self, pid, regularState)

    if #topicJournal > 0 then
        tes3mp.ClearJournalChanges(pid)

        for _, journalItem in ipairs(topicJournal) do
            tes3mp.AddTopicJournalEntry(pid, journalItem.topicId, journalItem.infoId,
                journalItem.text, journalItem.actorName or "")
        end

        tes3mp.SendJournalChanges(pid)
    end
end

return {}
