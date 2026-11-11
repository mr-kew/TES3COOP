# Topic journal persistence plugin

This plugin stores the dialogue responses shown in the Journal's **Topics** section
alongside quest journal entries. It follows the standard CoreScripts `shareJournal`
setting: shared when enabled, otherwise stored per player.

It requires both the matching TES3MP client/server C++ changes and CoreScripts 0.8.1.
The unmodified TES3MP 0.8.1 binaries do not send these response records, so installing
the plugin alone on a standard release cannot enable the feature.

To install it on a server using the matching custom build:

1. Copy `custom/topicJournalPersistence.lua` into `CoreScripts/scripts/custom/`.
2. Add `require("custom/topicJournalPersistence")` to `CoreScripts/scripts/customScripts.lua`.
3. Keep `config.shareJournal` set to the desired shared or per-player behavior.

The separate `shareTopics` setting continues to control synchronization of learned
dialogue topic IDs. It does not control the response history shown in the journal.
