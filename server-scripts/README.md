# Server script features

## Friendly fire

Friendly fire is configured as a server multiplayer rule in CoreScripts, separate
from the general `config.gameSettings` list. In
`CoreScripts/scripts/config.lua`, add:

```lua
config.friendlyFire = false
```

`false` prevents allied players from damaging each other; `true` allows the damage.
The default is `true`. Remove any `friendly fire` entry you previously added to
`config.gameSettings` so there is only one setting to maintain.

Install the adapter that sends this server rule to connecting clients:

1. Copy `custom/friendlyFire.lua` into `CoreScripts/scripts/custom/`.
2. Add `require("custom/friendlyFire")` to `CoreScripts/scripts/customScripts.lua`.
3. Restart the server after changing the config or script.

Players must form a mutual alliance for the rule to apply: one uses `/invite <pid>`
and the invited player accepts with `/join <inviter-pid>`. The alliance is saved
in the players' server data. Player IDs appear beside names in chat and can change
between sessions.

Every player must use a TES3MP client build containing this change. The server
sends the configured value through the existing game-settings packet; the C++
server and packet format do not need changes. The client filters allied melee and
ranged hits, on-hit enchantments, and direct or area spell effects. Non-allied PvP
remains enabled when `config.friendlyFire` is `false`. It also blocks attacks by a
player's summoned creatures against that player's allies.

## Topic journal persistence plugin

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

## NPC disposition persistence

Permanent NPC base disposition changes are synchronized back to the player who
made them and stored in that player's save, keyed by NPC reference ID and unique
index rather than cell. On cell load, saved entries are sent in batches. The
client applies matching entries immediately and keeps unmatched entries pending
until the NPC is loaded or moves into an already-loaded cell.
Disposition is personal game state, so one player's quest or dialogue changes do
not overwrite another player's values. Existing saves using the earlier
cell-keyed format are migrated automatically. This requires matching TES3COOP
client and server builds because the dialogue-choice packet includes a
disposition value.

To enable persistence on a CoreScripts 0.8.1 server:

1. Copy `custom/npcDispositionPersistence.lua` into `CoreScripts/scripts/custom/`.
2. Add `require("custom/npcDispositionPersistence")` to
   `CoreScripts/scripts/customScripts.lua`.
3. Add this setting to `CoreScripts/scripts/config.lua`:

   ```lua
   config.persistNpcDispositions = true
   ```

The Windows release workflow includes the plugin, loads it from CoreScripts, and
enables the setting in the packaged server. Set it to `false` to stop saving and
reapplying dispositions; disposition changes still sync to the player who made
them for the current session. Restart the server after changing the setting.
