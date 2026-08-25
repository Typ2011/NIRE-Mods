# Changelog

All notable changes to Expanded Saline Bags are documented here.

## Unreleased

### Added

- US and USSR saline bag prefabs in 250 ml, 500 ml, 1000 ml, 1250 ml, and 1500 ml sizes; vanilla provides the 750 ml variants.
- Regeneration amounts matching all six supported sizes.
- US and USSR arsenal entries for every modded size.
- A test world for development and verification.
- A mod plan and contributor instructions for future development.
- Manual interruption through the existing saline action: `Stop saline` / `Saline stoppen`.
- Server-side remaining-volume calculation in 250 ml steps, returning the matching US or USSR prefab.
- Missing arsenal metadata for the new 250 ml and 1250 ml prefabs.
- A three-second held interaction and native progress circle for `Stop saline` / `Saline stoppen`.

### Tested

- Core functionality tested successfully in multiplayer with no known functional issues.
- Manual interruption, the three-second progress circle, and remaining-volume handling tested successfully on a dedicated server.

### Open

- Standardize physical properties, treatment times, and supply costs across all variants.
- Continue testing consumption and balance values.
- Investigate optional ACE Medical compatibility.

## Sources

Technical references are listed in the [mod plan](Modplan.md#quellen).
