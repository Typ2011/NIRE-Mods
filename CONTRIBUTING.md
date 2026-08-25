# Contributing to NIRE-Mods

Thanks for wanting to help. This repository uses a fork and pull request
workflow: nobody, maintainers included, pushes directly to `main`.

## Workflow

1. **Fork** this repository to your own account.
2. **Branch** from `main` in your fork. Use a short descriptive name, for
   example `fix/gmvehiclelock-late-join` or `feat/inventoryboxes-crate-sizes`.
3. **Commit** your work. Keep the changes for one mod in one pull request
   whenever you can.
4. **Open a pull request** against `main` of this repository and fill in the
   pull request template.
5. A maintainer reviews it. Once it is approved, a maintainer merges it.

Pull requests from a fork are the only way changes get in. Direct pushes to
`main` are rejected by branch protection.

## Before you open a pull request

- **Load the mod in Workbench and let it compile.** Check `script.log` for
  compile errors. A green Workbench message alone is not proof; verify the
  Game module CRC32 in `script.log` actually changed.
- **New or renamed files need a resource database rescan** in Workbench,
  otherwise they work in Workbench but go missing from a packed build.
- **Test the change in the mode it affects.** Anything that touches
  replication, Game Master tools or player actions needs a multiplayer test
  with at least a hosted server and one client, not just singleplayer.
- **Update the mod's changelog.** Every mod folder has one; add an entry under
  `Unreleased`.
- **Do not commit generated files.** `resourceDatabase.rdb`, `UserMaps.desc`
  and `Scripts/WorkbenchGame` are ignored on purpose. Workbench regenerates
  them.

## Code style

Follow the style of the file you are editing. Enfusion Script conventions used
throughout this repository:

- Mod-specific prefixes on every class and resource name, matching the mod you
  are working in (`DGC_`, `IBX_`, `RAMI_`, `GMVL_`, and so on). Prefixes stop
  mods from colliding with each other and with vanilla.
- `modded class` over copying vanilla code. If a vanilla method has to be
  reimplemented, say why in the pull request.
- Server-authoritative logic. Never trust a value that came from a client
  without validating it on the server.
- Keep localization strings in the mod's `Language` folder, not hardcoded in
  script.

## Reporting bugs and asking for features

Open an issue and use one of the templates. The bug template asks for the game
version, the mod version, the other mods you had loaded and a `script.log`
excerpt. Those four things decide whether a bug can be reproduced at all,
so please fill them in.

## License

By contributing you agree that your contribution is licensed under the GNU
General Public License, version 2 only, the same license as the rest of this
repository. See [LICENSE](LICENSE).
