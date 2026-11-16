# Server script features

## Tamriel Rebuilt whitelist data

The Tamriel Rebuilt 26.08b whitelist source files live in
`server-scripts/whitelists/tr_26_08b/`. Edit the global categories in
`clientVariableScopes.lua` or the synchronized script IDs in
`synchronizedClientScriptIds.lua` when a campaign bug needs another entry.

The Windows release workflow runs `whitelists/compile_whitelists.py` after
copying CoreScripts. It merges the global additions into
`server/scripts/clientVariableScopes.lua` when `TR_Mainland.esm` is enabled and
adds the script IDs to `server/scripts/config.lua`, keeping existing CoreScripts
entries. The compiler is safe to run more than once on the same files.

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

## Per-player local-map persistence

Explored local-map fog is saved in each player's CoreScripts player data and its
PNG images are stored separately under `data/map/`. Fog is always private to the
player who explored it. This feature deliberately ignores `config.shareMapExploration`;
it does not combine or share players' local-map fog.

It requires matching TES3COOP client and server builds. To enable it on a
CoreScripts 0.8.1 server:

1. Copy `custom/localMapPersistence.lua` into `CoreScripts/scripts/custom/`.
2. Add `require("custom/localMapPersistence")` to
   `CoreScripts/scripts/customScripts.lua`.
3. Set `config.persistLocalMap = true` in `CoreScripts/scripts/config.lua`.
   It defaults to `false` in the Windows release package.
4. Ensure the server's `data/map/` directory exists and is writable.
5. Restart the server.

The Windows release ZIP includes and loads the plugin automatically. When
`config.persistLocalMap` is `false`, the server acknowledges fog updates but does
not save or restore them. Fog is requested as cells become active and uploaded
when changed cells unload (with periodic syncs while exploring), so reconnecting
does not download the player's entire explored map at once.
Back up `data/map/` along with the normal player save data to preserve these maps.

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
