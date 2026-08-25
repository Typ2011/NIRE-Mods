# Inventory Boxes — Full Pre-Alpha Changelog

## How to Load and Unload a Crate

1. Open your inventory.
2. **Middle-click the vehicle** to open its inventory ("Open New Inventory"). On a controller, use the equivalent button.
3. Hover over the crate you want to move.
4. Hold the transfer key — default **G** — for **5 seconds**.

Unloading works exactly the same way: hover over the crate inside the vehicle inventory and hold **G** for 5 seconds.

Progress is shown at the bottom of the screen, in the control hints bar. Releasing the key early cancels the transfer.

**Important:** loading and unloading only work when the vehicle inventory was opened with a middle-click ("Open New Inventory"), or the controller equivalent. It is not available through the normal vehicle inventory view.

## 23 August 2026 — Carry, Physics and Destruction Fixes

### Crates

- Crates are **destructible** — every one of the 42 variants. Explosives destroy them quickly, small-arms fire only slowly. A destroyed crate is removed along with its contents.
- Held crates no longer block, push, or lift players — collision is suspended for everyone while a crate is carried or dragged.
- Fixed crates unloaded from a vehicle staying walk-through: their physics are restored when they leave a vehicle storage.

### Carrying

- A crate is now dropped automatically when the carrier **falls unconscious**, as it already is on death.
- Fixed a crate staying stuck to a player who died while carrying it, leaving it frozen in place and hidden from every action.
- Fixed players being unable to pick up any crate after dying and respawning while carrying one.
- Fixed a carried crate keeping the movement speed penalty on the player after being loaded into a vehicle.
- Boarding a vehicle or disconnecting while holding a crate now releases it properly.
- Fixed players who joined a running server still colliding with crates that were already being carried.

## 21 August 2026 — Physical Crate Carry and Drag

### Carrying and Dragging

- Added **Carry Crate** action — pick a crate up and hold it in front of you.
- Added **Drag Crate** action — hold to drag a crate along the ground while walking backward.
- Added **Drop Crate** action to put a held crate down in either mode.
- Added **Raise**, **Lower**, **Rotate Left**, and **Rotate Right** actions while carrying.
- Carried and dragged crates are placed on the ground surface when dropped.
- Dragged crates follow the terrain continuously, including across slopes.
- Dragging requires the crate to be in front of you and only advances while moving backward; the crate stays put if it ends up behind you.
- Dragging stops automatically beyond **2.5 m** from the crate.
- Movement speed is reduced while holding a crate: **35%** while carrying, **65%** while dragging.
- Carried and dragged crates no longer collide with the player holding them.
- **Carry** and **Drag** are hidden while you are already holding a crate.
- Crate actions now display in a fixed order: Open Storage, Carry, Drag.
- Carry and drag are fully server-authoritative and work on dedicated servers, listen servers, and in single player.

### Controls

- Height, rotation, and drop are performed through world actions instead of dedicated keys.
- Removed the earlier **Carry Height Up / Down**, **Rotate Modifier**, and **Drop** key bindings, along with their entries in the Controls menu.

### Fixes

- Fixed carried and dragged crates becoming invisible from certain camera angles.
- Fixed dragged crates snapping to the wrong position or lagging behind at higher movement speeds.
- Fixed dragged crates being climbable, which could interrupt dragging.
- Fixed crates ending up underground or floating after being dropped on uneven terrain.
- Fixed the drag button remaining active after dragging was stopped by walking out of range.
- Fixed inventory editor buttons stretching across the full row instead of sharing it evenly.

### Multiplayer and Performance

Carrying and dragging are server-authoritative and should work in multiplayer, but performance has not been tested yet. Both send continuous position updates while a crate is held, so some performance impact is to be expected — feedback from populated servers is welcome.

## 16 August 2026 — Controls and UI Fixes

- Changed default **Load / Unload Crate** key from **F** to **G**.
- Restored normal **F** behavior for equipping inventory items into hands.
- Fixed crate contents disappearing from Game Master inventory editor after moving populated crates.
- Fixed **Escape** opening pause menu when closing custom inventory editor.

## 15 August 2026 — Presets, Copy/Paste, and Multiplayer

- Added named crate inventory presets.
- Added preset selection directly inside Game Master inventory editor.
- Added **Export Inventory Preset** action.
- Added **Paste Inventory** action.
- Added direct **Copy** and **Paste** buttons inside inventory editor.
- Added PC clipboard support with an in-session fallback buffer.
- Added server-authoritative validation for all inventory changes.
- Improved dedicated-server support for adding, removing, clearing, and replacing crate contents.
- Fixed inventory changes failing when crate entity lacked a direct replication ID.
- Fixed stale inventory lists after multiplayer inventory changes.
- Inventory editor now receives updated contents directly from server after every successful change.
- Increased Game Master limit from 100 total items to **100 distinct item types**.
- Added support for mutations of up to **1,000 items** at once.
- Added preset limits and validation to prevent invalid or unavailable items from clearing a crate.
- Updated crate contents heading to display item types and total item count separately.
- Improved status messages for failed or rejected inventory changes.

## 14 August 2026 — Game Master UI Overhaul

- Rebuilt Game Master inventory editor using Mike's UI.
- Added rounded panels, improved spacing, and responsive two-column layout.
- Improved arsenal, crate contents, filters, tabs, quantity field, presets, and action buttons.
- Kept native 3D item previews for arsenal and crate items.
- Fixed controls and lists overflowing outside modal window.
- Fixed crate contents column and bottom actions becoming hidden.
- Fixed square background corners around rounded editor panel.
- Fixed faction dropdown leaving stale buttons visible after closing.
- Fixed export overlay remaining visible after closing.
- Improved layout behavior across different Workbench resolutions.

## 10 August 2026 — Vehicle Crate Transfers

- Added loading crates from vicinity into compatible vehicle inventories.
- Added unloading crates from vehicle inventory back into world.
- Added dedicated **Load Crate** and **Unload Crate** inventory actions.
- Added configurable keyboard and gamepad bindings.
- Added required continuous **five-second hold** for crate transfers.
- Added immediate hold-circle progress feedback.
- Releasing button early now cancels transfer.
- Crate, source storage, and target vehicle are revalidated before transfer completes.
- Vehicle inventory size and weight limits remain enforced.
- Disabled normal crate dragging and quick-move shortcuts to prevent bypassing transfer timer.
- Other item types retain normal inventory behavior.
- Fixed loading crashes caused by invalid vehicle inventory UI storage.
- Fixed unloading through vanilla move-to-vicinity placement.
- Fixed custom transfer action appearing with invalid crossed-circle icon.
- Fixed transfer button failing to appear in navigation bar.
- Fixed transfer hold circle starting late or ending partially filled.
- Hid conflicting vanilla Use action only when valid crate transfer is available.

## 9 August 2026 — Game Master Crates and Placement

- Expanded mod to **42 finite inventory crate appearances**.
- Added ammunition boxes, equipment stacks, wooden equipment boxes, and arsenal-box appearance.
- Added dedicated **Inventory Boxes** category to Game Master asset browser.
- Renamed entries clearly as **Finite Inventory Crate** variants.
- Added Game Master placement and selection support.
- Preserved vanilla preview images and models.
- Added **Edit Inventory** context action to every crate.
- Added player-accessible vanilla storage interaction.
- Increased interaction radius, height, and visibility range.
- Fixed inherited storage conflicts on vanilla-derived crates.
- Disabled unlimited vanilla arsenal/storage components where required.
- Fixed duplicate action managers preventing Game Master placement.
- Fixed inventory insertion and display selecting wrong inherited storage.
- Added replicated editable-entity support.
- Updated Game Master test world with finite inventory crate.

## 8 August 2026 — Initial Pre-Alpha Release

- Added physical crates with normal finite inventory.
- Added storage for weapons, ammunition, clothing, medical supplies, explosives, and equipment.
- Added **100 inventory slots**.
- Added storage capacity up to **1,000 kg**.
- Increased supported item dimensions and volume for large equipment.
- Added vanilla open-storage interaction.
- Added Game Master inventory editor.
- Added arsenal search.
- Added faction filtering.
- Added category tabs for:

  - Weapons
  - Ammunition
  - Clothing
  - Medical items
  - Explosives
  - Equipment

- Added item quantity controls.
- Added item insertion, removal, and **Empty Crate** actions.
- Added selectable items in both arsenal and crate lists.
- Added localized vanilla item names.
- Added 3D item previews.
- Added compatible ammunition detection for equipped weapons.
- Added expandable compatible-ammunition rows below selected weapons.
- Added green highlighting for compatible ammunition.
- Added Escape/back support for closing editor.
- Added multiplayer-authoritative inventory operations.
- Fixed item insertion failing because default weight limit was zero.
- Fixed large items not fitting inside storage.
- Fixed empty inventory view lacking display attributes.
- Fixed crate placement null-pointer errors.
- Fixed large quantity requests filling crate incorrectly.
- Fixed large add operations hanging game.
- Fixed background brightness changing while editor was open.
- Added initial US, USSR, and FIA crate variants.
- Added Pre-Alpha Workshop and Discord presentation assets.
