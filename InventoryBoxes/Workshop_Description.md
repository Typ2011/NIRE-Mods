INVENTORY BOXES - PRE-ALPHA

Inventory Boxes adds physical crates with proper finite inventories, like Arma 3, to Arma Reforger.

No unlimited arsenal stock: weapons, ammunition, medical supplies, clothing, explosives and equipment exist as real stored items with limited quantities. Crates are filled by Game Masters, opened by players, carried, dragged, requested at vehicle depots and loaded into vehicles. Everything is server-validated.


FEATURES

- 42 crate appearances in a dedicated Inventory Boxes Game Master category
- Capacity, transport size, weight and grid footprint all scale with the real size of the crate
- Up to 100 distinct item types and 1,000 kg per crate
- Game Master editor: search, faction filter, six categories, 3D previews, compatible ammunition
- Named inventory presets, plus copy, export and paste of complete crate contents
- Custom crate names, set by Game Masters or by any player
- Carry and drag crates by hand, or load them into vehicles with a 5-second hold
- Request crates at a Conflict light vehicle depot for 25 supplies


EDITING A CRATE (GAME MASTER)

Place a crate and use its Edit Inventory action. Search the arsenal, filter by faction, switch between Weapons, Ammunition, Clothing, Medical, Explosives and Equipment, set a quantity, then add or remove up to 1,000 items at a time. Empty Crate clears it. Selecting a weapon shows its compatible ammunition in green.


CRATE SIZE

Every crate is measured from its own model, so a .50 cal ammo can and a pallet of equipment boxes are no longer interchangeable.

- Storage capacity follows the crate's real volume.
- A crate costs room to transport in proportion to its size, and takes matching space in the inventory grid.
- Weight ranges from 2 kg for a .50 cal can to 105 kg for the largest pallet stack.
- All 42 crates fit a transport truck. The 13 smallest and flattest also fit a jeep; the tallest stacks are truck-only.
- Crates cannot be stored inside each other or inside a backpack.


CRATE NAMES

Every crate can be given a custom name of up to 32 characters. Game Masters set it in the inventory editor header; any player can use a crate's Name Crate action to set, change or clear it.

The name shows on the world action, the inventory header, the vicinity hover and in vehicle cargo. Applying a preset names the crate after it.


CARRYING AND DRAGGING

Look at a crate and use its world actions:

- Carry Crate holds it in front of you. Raise, Lower, Rotate Left and Rotate Right adjust it, Drop Crate puts it down.
- Drag Crate drags it along the ground. It only moves while you walk backward with the crate in front of you, and dragging stops past 2.5 m.

Speed is reduced while holding a crate: 35% carrying, 65% dragging.


LOADING CRATES INTO VEHICLES

1. Open your inventory.
2. Middle-click the vehicle to open its inventory ("Open New Inventory"). On controller, use the equivalent button.
3. Hover the crate and hold the transfer key - default G - for 5 seconds.

Unloading works the same way, from the vehicle inventory. Progress shows in the control hints bar, and releasing early cancels. Vanilla F is unchanged, and quick-move is blocked so the timer cannot be bypassed.

Important: this only works through the middle-click "Open New Inventory" view, not the normal vehicle inventory.


REQUESTING CRATES IN CONFLICT

Any player can request a crate at a light vehicle depot, not just place it as a Game Master. Crates get their own Inventory Boxes tab in the depot build menu, at 25 supplies each.

US and USSR crates stay faction-locked. The 13 faction-neutral crates are available at every depot, FIA included. Crates are exempt from the depot's vehicle cooldown.


INVENTORY PRESETS

Presets are named, ready-made crate loadouts. Open a crate's Edit Inventory, pick a preset and apply it - this replaces the contents. A preset holds up to 100 unique item types and 10,000 items total, and is validated first, so an invalid preset cannot empty a crate.

To copy between crates, use Copy (or Export Inventory Preset) and Paste (or Paste Inventory). Clipboard support is PC-only, with an in-session buffer as fallback.

Own presets are added by overriding Configs/Inventory/CratePresets.conf in your own addon - there is no in-editor creation yet. Instructions are on GitHub, linked below.


MULTIPLAYER AND DEVELOPMENT STATUS

Everything is server-authoritative and works on dedicated servers, listen servers and in single player.

Pre-Alpha. Carry and drag are not performance-tested yet - both send continuous position updates while a crate is held, so expect some impact on busy servers.

Useful feedback: carry and drag in multiplayer, presets, vehicle transfers, depot requests, missing items or previews, editor usability, and reproducible bugs with screenshots and logs.


LINKS

Source code, issues and all NIRE mods: https://github.com/Typ2011/NIRE-Mods

Discuss the mod, report bugs or follow development in the "NIRE Mods" thread on the official Arma Discord.
