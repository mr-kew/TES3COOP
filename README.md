# TES3COOP

TES3COOP is a custom cooperative multiplayer build based on [TES3MP](https://github.com/TES3MP/TES3MP) and [OpenMW](https://github.com/OpenMW/openmw). It adds client and server changes for a more cooperative Morrowind experience while keeping TES3MP's multiplayer foundation.

> [!WARNING]
> Dot't use this, it's AI slop! Use regular TES3MP instead.

This repository targets TES3MP 0.8.1 and OpenMW 0.47.0. TES3COOP builds are not interchangeable with unmodified TES3MP builds when they use custom network features; players should use matching client and server builds.

## Included features

- Configurable allied-player friendly fire, including attacks from summoned creatures.
- Per-player persistence for NPC disposition changes.
- Dialogue topic response history in the Journal, following the server's `shareJournal` setting.
- Windows x64 release builds that package the matching server scripts.

See [server-scripts/README.md](server-scripts/README.md) for feature behavior and manual server setup.

## Downloads

Windows x64 builds are produced by the [Windows release workflow](.github/workflows/windows-release.yml). Open the repository's **Actions** tab, select **Build Windows TES3MP release**, choose a branch, and run the workflow. The resulting `TES3MP-Windows-x64` artifact contains the client, support applications, server, and configured server scripts.

## Building

The Windows workflow documents and automates the supported release build. It builds against pinned CrabNet and CoreScripts revisions and packages the custom server plugins. For local development, see the upstream [TES3MP build instructions](https://github.com/TES3MP/TES3MP#building) and the repository's `CI/` scripts.

## License and acknowledgements

TES3COOP is based on TES3MP and OpenMW. Copyright and license terms are listed in [LICENSE](LICENSE); third-party components retain their respective licenses.
