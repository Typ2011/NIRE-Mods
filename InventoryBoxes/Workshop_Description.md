# Inventory Boxes — Pre-Alpha

Inventory Boxes adds physical crates with proper finite inventories, like Arma 3, to Arma Reforger.

No unlimited arsenal stock: weapons, ammunition, medical supplies, clothing, explosives and equipment exist as real stored items with limited quantities. Crates are filled by Game Masters, opened by players, carried and dragged around, and loaded into vehicles. All changes are validated by the server; storage capacity, weight and vehicle inventory limits stay enforced.

## Features

- 42 crate appearances in a dedicated **Inventory Boxes** Game Master category
- Up to 100 distinct item types and 1,000 kg per crate
- Server-authoritative Game Master inventory editor: search, faction filter, six categories, 3D previews, compatible-ammunition display
- Add or remove up to 1,000 items per operation
- Named inventory presets, plus copy, export and paste of complete crate contents
- Carry and drag crates by hand
- Load and unload crates into vehicles with a 5-second hold
- Players open crates through the normal vanilla storage interaction

## Editing a Crate (Game Master)

Place a crate from the **Inventory Boxes** category and use the **Edit Inventory** context action. Search the arsenal, filter by faction, switch between Weapons, Ammunition, Clothing, Medical, Explosives and Equipment, set a quantity, then add or remove items. **Empty Crate** clears it. Selecting a weapon shows its compatible ammunition, highlighted in green.

## Carrying and Dragging

Look at a crate and use its world actions:

- **Carry Crate** holds it in front of you. **Raise**, **Lower**, **Rotate Left** and **Rotate Right** adjust it, **Drop Crate** puts it down.
- **Drag Crate** drags it along the ground. The crate only moves while you walk backward with the crate in front of you, and dragging stops past 2.5 m.

Speed is reduced while holding a crate: 35% carrying, 65% dragging.

## Loading and Unloading Crates into Vehicles

1. Open your inventory.
2. **Middle-click the vehicle** to open its inventory ("Open New Inventory"). On controller, use the equivalent button.
3. Hover over the crate you want to move.
4. Hold the transfer key — default **G** — for **5 seconds**.

Unloading works the same way, from the vehicle inventory. Progress is shown at the bottom of the screen in the control hints bar, and releasing early cancels. Vanilla **F** is unchanged, and quick-move is blocked so the timer cannot be bypassed.

**Important:** this only works when the vehicle inventory was opened with a middle-click ("Open New Inventory"), or the controller equivalent — not through the normal vehicle inventory view.

## Inventory Presets

Presets are named, ready-made crate loadouts. Open a crate's **Edit Inventory**, pick a preset and apply it — this **replaces** the crate's contents. A preset can hold up to 100 unique item types, 1,000 items per type and 10,000 items total; everything is validated before the crate is cleared, so an invalid preset cannot empty a crate.

To copy between crates, use **Copy** (or the **Export Inventory Preset** action) and **Paste** (or the **Paste Inventory** action). Both use the same format as presets. Clipboard support is PC-only, with an in-session buffer as fallback.

## Creating Your Own Presets

Presets live in `Configs/Inventory/CratePresets.conf`, and own presets are always added by **overriding that file in your own addon** — there is no in-editor preset creation yet, and editing this mod's copy is overwritten on every update.

1. In the Enfusion Workbench, open your own addon with Inventory Boxes as a dependency.
2. Override `Configs/Inventory/CratePresets.conf` into it.
3. Add an entry to the **Presets** array, set **Name**, and set **Items** to `Count=Prefab;Count=Prefab`, for example `30={GUID}Prefabs/Weapons/Magazines/....et;5={GUID}Prefabs/Weapons/Rifles/....et` This is your copied preset string.
4. Rebuild/reload the addon resources.

Quickest way to build that string: fill a crate in-game, use **Export Inventory Preset** or **Copy**, and paste the value into **Items**. Limits: 100 unique prefabs, 1 to 1,000 per entry, 10,000 total. Duplicate prefabs, empty names and out-of-range counts are rejected.

## Multiplayer and Performance

Inventory changes, carry, drag and vehicle transfers are server-authoritative and work on dedicated servers, listen servers and in single player.

Carrying and dragging are **not performance-tested yet**. Both send continuous position updates while a crate is held, so a performance impact is to be expected on populated servers. Feedback on this is very welcome.

## Development Status

**Pre-Alpha.** Persistence and vehicle transport need broader testing.

Useful feedback: carrying and dragging in multiplayer, presets and copy/paste, vehicle transfers, missing items or previews, editor usability, and reproducible bugs with screenshots and logs.
