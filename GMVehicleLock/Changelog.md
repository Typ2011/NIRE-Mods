# Changelog

All notable changes to GMVehicleLock are documented in this file.

## 0.1.0 – 2026-08-25

First working release. The mod was previously named `GM KeylessGo` and its earlier locking attempt never functioned, so this entry describes what actually ships rather than that history.

### Added

- Game Master context actions **Lock Vehicle** and **Unlock Vehicle** on selected vehicles.
- **Vehicle Locked** checkbox in the Game Master attribute window, category *Vehicle*.
- Replicated lock state on `SCR_BaseLockComponent`. The server owns the value and streams it to all clients and to players joining in progress.
- A reason text on the blocked Get In action, which vanilla only supplies through the spawn protection component.

### Changed

- Locking now sets the vanilla lock component that `SCR_GetInUserAction` already checks, instead of disabling the vehicle's user actions one by one.
- The two Game Master entries are appended to the editor component prefab data at runtime. `EditorModeEdit.et` and `Configs/Editor/ActionLists/Context/TempEdit.conf` are no longer overridden, so all vanilla context actions and attributes remain and other Game Master mods do not conflict.

### Fixed

- The context action never appeared. The shipped copy of `TempEdit.conf` carried a `.meta` GUID that did not match the vanilla resource, so the override never applied.
- Nothing was replicated. The lock was set locally on the server only, so clients kept their Get In actions enabled.
- Turrets and other add-on seats stayed enterable on a locked vehicle. Their compartments live on child entities that carry no lock component, and the lookup has to run at check time because a slot child is not necessarily parented to the vehicle yet while its actions initialise.

### Validated

- All Enfusion MCP checks pass for structure, project file, scripts, prefabs, configs and references.
- Tested in the Workbench and on a dedicated server, driver, cargo and turret seats, join in progress included.
