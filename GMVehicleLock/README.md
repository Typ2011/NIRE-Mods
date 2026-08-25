# GMVehicleLock

A small Arma Reforger mod that lets a Game Master lock and unlock vehicles. Locked vehicles cannot be entered by players, on any seat.

## Usage

Select one or more vehicles in the Game Master, then either

- right click and pick **Lock Vehicle** or **Unlock Vehicle**, or
- open the attribute window and toggle **Vehicle Locked** in the *Vehicle* category.

The lock is applied on the server and replicated, so clients see it immediately and players joining later receive the current state.

## How it works

Every vanilla vehicle inherits an `SCR_BaseLockComponent` from `Prefabs/Vehicles/Core/Vehicle_Base.et`, and `SCR_GetInUserAction` already checks it before letting a player in. The vanilla `SetLocked()` only writes a local member, so the mod adds a replicated property to that component and mirrors the state onto every machine.

Turrets and other add-on seats live on child entities of the vehicle, which carry no lock component of their own. The mod resolves the lock from the vehicle root when the action is checked, so those seats respect the lock as well.

No vanilla prefab or config is overridden. The two Game Master entries are appended to the editor component prefab data at runtime, which keeps every vanilla context action and attribute intact and avoids conflicts with other Game Master mods.

## Installation

Load the mod on the server and on all clients.

## Testing

1. Add and open `addon.gproj` in the Arma Reforger Workbench launcher.
2. Load `Worlds/Editor/GM_Eden.ent` and start it with `F5`.
3. Place a vehicle with a turret, select it, and lock it.
4. Verify that the Get In action is blocked on the driver, cargo and gunner seats, and that unlocking restores all of them.

For a multiplayer test, lock a vehicle before a client connects and confirm that the client cannot enter it.

## Limitations

- Locks do not survive a server restart.
- Occupants who are already seated when the vehicle is locked stay in and keep control of it.
- No keys, owners or permission lists.
- Vehicles are not locked automatically when placed.
