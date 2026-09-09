# Changelog

## 2026-09-09 - Mikes UI text field caret fixed from here instead of forking the dependency

`MUI_TextField` paints its own value text and then a caret placed after the measured width of the whole string, so the caret sits at the end of the text however far left the cursor has actually been moved. The cursor itself is fine - it lives in `MUI_EditBridge`'s hidden native `EditBoxWidget` and the arrow keys move it - but nothing on screen follows it, and it cannot be drawn correctly either: `EditBoxWidget` is `sealed` and exposes neither a cursor position nor a selection to script.

`IBX_MuiEditBridgeFix.c` holds both halves. A `modded class MUI_EditBridge` shows the native box in place rather than hiding it at 2% opacity, so the engine renders the text, the caret and the selection itself, all correct by construction; `IBX_MuiEditBox.layout` supplies that box, because `EditBoxWidget` has no `SetFontSize` and one built by `CreateWidget` cannot be made to match `FONT_BODY`. A `modded class MUI_TextField` then repaints the value area in `theme.Field` at the end of `PaintForeground`, covering the text and fake caret MUI has just drawn.

Covering on MUI's own surface rather than layering an opaque widget over it: a widget has to win a z-order argument against `MUI_RenderSurface`, and an opaque fill behind the native box did not reliably hide the caret. Repainting happens in the same surface, immediately after the thing being covered, so ordering cannot come into it. Only the inner text rect is repainted - the accent bar at the box's left edge, the border stroke and the rounded corners are all outside it and stay MUI's.

`PaintForeground` is extended, never replaced. Suppressing the value would have meant copying its body into this addon, where it would drift silently from upstream's; covering needs nothing from it but the geometry of its input box, mirrored in five named constants.

Two details worth keeping in mind. The box is put at z-order 20 explicitly - the stock bridge passes that same sort order to `CreateWidget`, `CreateWidgets` takes none, and below it the field chrome paints straight over the box. And the placeholder space `Attach` seeds an empty field with is dropped as soon as write mode is live: invisible it only ever leaked into the value, but a visible box would show a space the player never typed.

NiRe Notepad depends on this addon, so it inherits the fix; there is no second copy there. The `StripLeadingSpaces` guards in both addons stay as they are, since a fix that fails to load should not take the search with it.

## 2026-09-09 - Crate editor arsenal search matched string table keys, not the names on screen

Typing a name into the arsenal search found nothing, or found rows whose visible name did not contain the term at all.

`UIInfo.GetName()` returns a string table key, not a name. `AddCatalogItem` and `RefreshCurrentList` cached that key straight into `m_ArsenalLabels`, and `CreateRow` passed it to `TextWidget.SetText`, which translates a leading `#` key on the way to the screen. So the row read `Bandage` while the cached label was still `#AR-Item_Bandage_Name`, and the filter compared the typed text against the key. The crate contents list had the same fault in a quieter form: its label goes through `string.Format("%1  x%2", ...)`, and a key embedded in a longer string is not a key any more, so those rows showed the raw text.

Both cache sites now go through `ResolveName`, which resolves the key once with `WidgetManager.Translate`. `GetLabel` therefore returns something that is both what the row displays and what the search matches.

The search itself is now term-based: `BuildSearchTerms` splits the query on spaces and `MatchesSearchTerms` requires every term to appear somewhere in the name, in any order, so `m16 olive` finds `M16 Carbine - Olive`. It replaces a single `Contains` over the whole query, which could only match an unbroken substring.

Two smaller fixes in the same menu. Changing the arsenal tab, faction filter or search text now calls `ScrollArsenalToTop`, because the list is rebuilt underneath a scroll offset that no longer points at anything; selecting a row deliberately keeps its position, since expanding a weapon's compatible ammunition rebuilds the list too and jumping to the top there would throw the Game Master away from the item just clicked. And `StripLeadingSpaces` guards the search and quantity reads against the stray leading space a Mikes UI text field leaves behind when it takes focus - the quantity was the quieter half of that one, since `" 5".ToInt()` is `0` and clamped back up to `1`, silently turning a typed amount into a single item.

The leading space has since been fixed at its source in Mikes UI, but the guard stays: anyone running this addon against the published Mikes UI still needs it.

## 2026-08-31 - Load Crate no longer needs the vehicle storage to be the navigated one

Reported on Discord: opening a vehicle's inventory with middle mouse and hovering a crate sometimes shows no `Load Crate` entry.

`IBX_FindVehicleStorageUI` accepted a vehicle only while its storage was the *current navigation storage* of the loot UI or of an opened storage container. That is narrower than "the vehicle's inventory is open". `SCR_InventoryMenuUI.SetOpenStorage` traverses the loot UI into the vehicle's storage only if `GetStorageUIFromVicinity` already finds a vicinity slot for it; it is called once from `OnMenuOpen`, its failure is silent, and the menu then opens on the plain vicinity list with the vehicle merely listed in it. Stepping back out of the vehicle storage clears the traversal the same way.

In both states the vehicle is still in the vicinity, so the resolver is now `IBX_FindVehicleStorage`: the two navigation-storage checks unchanged, then a fallback to the first vehicle storage among the loot UI's own slots. It returns the storage as well as its UI, so `IBX_IsPendingTransferValid` re-resolves and compares the target instead of asserting the navigation still sits on it - that assertion would have rejected every transfer coming from the fallback at the end of the five-second hold. The unload check keeps only "the crate is still in the same storage".

Known ceiling, marked `ponytail:` in the source: when the fallback runs and two vehicles are in the vicinity, the first one wins.

Rejected on the way: also walking up the parent chain in `IBX_IsVehicleStorage`, on the theory that a cargo storage might sit on an entity attached to the vehicle rather than on the vehicle root. It is not needed - vanilla puts the storage on the root (`M923A1.et:480`) and `Vehicle_Base.et` is the only prefab declaring `SCR_VehicleInventoryStorageManagerComponent` - and in play it removed the `Load Crate` entry outright. Reverted; that function is unchanged.

## 2026-08-31 - Workshop changelog entry and Discord post for the placement round

`Workshop_Changelog.md` gained a `31 AUGUST 2026 - PLACEMENT AND DISPLAY FIXES` section covering the four entries below it: the surface snap, the tilt, the crates staying drawn inside vehicles, and the fill display. Plain text with no Markdown, matching the rest of that file - the Workshop renders none.

`Discord_Update_Placement.md` is the announcement, same shape as `Discord_Update_Fixes.md`: title, Workshop link, a one-line summary, then a section per area - Placing Crates, Crates in Vehicles, Crate Fill. It covers all four of the day's entries rather than only the placement pair, since they ship together. No feature walkthrough section: the round adds nothing new to explain.

## 2026-08-31 - A dropped crate lies on the slope it lands on

Follow-up to the surface-snap fix below. With the crate landing on the right surface, it was still always set down dead upright, so on any slope it floated at one corner and cut into the ground at the other.

The trace that already finds the surface also reports its normal, so `GetSupportY` now hands it back through an `out` parameter and the two places a crate comes to rest - `StopServer` and `DetachDrag` - tilt to it through a new `OrientToSurface`.

`SCR_EntityHelper.OrientUpToVector` is the vanilla helper for this and is not usable here: it rebuilds the entire basis out of the normal alone, via `newUp.Perpend()`, which throws the crate's yaw away and would spin every dropped crate to an arbitrary new facing. `OrientToSurface` keeps the yaw instead, rebuilding the basis as `right = up x forward`, `forward = right x up` from the crate's own current forward axis.

Two guards on it:

- The normal is normalized before use. A non-unit basis reaching `SetWorldTransform` would resize the crate.
- Nothing steeper than 40 degrees from level is matched (`SUPPORT_MAX_TILT_COS`). That surface is a wall or a rock face rather than something to set a crate on, and matching it would stand the crate on its side; those crates stay upright.

`ResnapDragHeight` deliberately does not orient. It runs every tick while a drag is attached, right after `ApplyDragOffset` has set the crate's rotation from the character's facing, so any tilt written there would be overwritten on the next tick and flicker. A dragged crate stays upright while it is moving and settles onto the surface when the drag detaches.

Confirmed working in play.

## 2026-08-31 - Crates stay where they are put down, including inside buildings

Reported: a crate cannot be placed on a building - it drops to the ground underneath instead. Reproduced from the clip: carry a crate onto a floor above ground level, drop it, and it teleports down to the terrain.

Every one of the three places the carry/drag system settles a crate onto a surface asked `SCR_TerrainHelper`, and `SCR_TerrainHelper` only ever answers with the heightmap. A building floor, a bridge deck, a container roof and a flatbed are all invisible to it, so "put the crate down" meant "put the crate on the ground below wherever you are standing". The three call sites were `IBX_CrateCarryComponent.StopServer` (the drop that ends a carry), `DetachDrag` (the crate coming to rest at the end of a drag step) and `ResnapDragHeight` (the per-tick height correction while dragging).

All three now go through one new `GetSupportY`, which traces straight down from the crate's position and returns the first solid surface it finds, falling back to the terrain height only when the trace finds nothing at all. The trace setup is the one vanilla item placement uses in `SCR_ItemPlacementComponent`: `TraceFlags.WORLD | TraceFlags.ENTS` with `EPhysicsLayerPresets.Projectile`.

Two details the trace needs to get right:

- It starts 0.5 m above the crate rather than at it. A trace starting flush with the surface a crate is already resting on can report zero distance travelled, and the clearance also lets a dragged crate climb a doorstep or a floor edge instead of only ever falling. It reaches 20 m down, which covers dropping a crate over a railing or off a roof.
- It filters out the crate *and its children* through `SCR_Global.FilterCallback_IgnoreEntityWithChildren`, not just the crate entity. The trace starts inside the crate's own collider, and the covered equipment stacks keep their cover in a separate child entity with a collider of its own - excluding only the root would have landed those crates on their own tarp, a little higher on every drop.

The holder character is excluded too, through `TraceParam.Exclude`, which applies on top of the callback.

Carrying itself never needed a fix: a carried crate is positioned relative to the holder's own origin, so it already followed the player up a staircase. Only the moment of setting it down was wrong.

Compile-verified in the Workbench (Game module CRC32 `520ed27f`, no errors). The in-play check - carry a crate onto an upper floor and drop it - is still outstanding.

## 2026-08-31 - One fill number for a crate, on every screen that shows one

Reported: a crate whose own inventory slot draws a full red bar shows its storage header bar at about half, and the Game Master inventory editor gave no fill reading at all.

The item bar was the honest one. Measured in play on a crate holding 43 rounds: `GetOccupiedSpace()` 4300 against a `GetMaxVolumeCapacity()` of 4300 - the crate was exactly full, and the bar said so. The header bar was never updated at all. Vanilla asks for a percentage in `SCR_InventoryStorageBaseUI.Init` and `Refresh`, and the crate panel, a `SCR_InventoryOpenedStorageUI`, reaches neither in the path the world Open action creates it through. An unasked `ProgressBarWidget` keeps its layout default, which is a full bar - and `SCR_InventoryProgressBar` flips its palette, so the header sat at a fixed fraction no matter what the crate held. Confirmed by log: the percentage call fired for the backpack, vest and hitzone panels and never once for the crate.

`IBX_CrateFill` is the single source now, and every surface is driven rather than asked:

- The opened crate's header bar is set from the modded `SCR_InventoryStorageBaseUI.Refresh` and `HandlerAttached` (in `IBX_CrateRename.c`, where that class is already modded). `GetOccupiedVolumePercentage` is still overridden so the insert preview colours from the same number; its `occupiedSpace` preview argument stays vanilla.
- The crate's own inventory slot bar is set from `SCR_InventorySlotUI.Refresh` and `SetSlotVisible` after vanilla has run, looking the widget up rather than using vanilla's cached `m_ProgressBar`, which is only filled in when the slot's own `BaseInventoryStorageComponent.Cast(m_pItem)` resolves.
- Both report the fuller of the two limits `SCR_UniversalInventoryStorageComponent.CanStoreItem` enforces - cumulative volume and contents weight against `m_fMaxWeight` - rather than vanilla's volume alone, so a crate reads full when it starts refusing items on either. Free slots are left out: `UniversalInventoryStorage` scales its slots dynamically, so `GetSlotsCount()` cannot be trusted to mean the configured 100 rather than the slots in use.
- The storage is always resolved through `IBX_GMInventoryEditorComponent.GetStorage()`, which is public for this, so no call site can land on the disabled inherited `SCR_UniversalInventoryStorageComponent {5476A2F100DF4EFF}` the 41 vanilla-derived prefabs still carry.
- The Game Master editor gained a fill line under `CRATE CONTENTS`, reading `100% FULL - 4/1000 KG - 4300/4300 VOLUME`. It takes the top half of the 52 px spacer that was already there, so the column keeps its height. On a server the line comes from the server, travelling as a string with the snapshot in `IBX_RpcDo_InventoryMutationResult`: the item entities a mutation creates arrive at the client after the reply does, so reading the crate locally left the line one step behind, which looked like it never changed. The local reading remains for the listen-server path, where this machine is the authority.

The percent sign in that line is concatenated rather than written into the format string: `string.Format` eats a `%` that follows a placeholder, so `"%1%"` printed the bare number.

## 2026-08-31 - Crates loaded into a vehicle stay drawn in the world

Reported against V1-V5 of the equipment box stacks: loading a covered V1, V2 or V5 into a truck left the cover hanging in the middle of the cargo bed and kept the crate's Game Master icon on screen, while V3 and V4 stayed visible in full. Two separate causes, both fixed at one hook.

`SCR_UniversalInventoryStorageComponent.OnAddedToSlot` calls `ShowOwner()` again for any item whose volume reaches `MIN_VOLUME_TO_SHOW_ITEM_IN_SLOT`, a hard-coded 200,000 cm3 - vanilla's "a big item rides visibly in the trunk" rule. Exactly four crates are above that line and they are exactly the ones reported: V3 (221,500), V3 covered (302,700), V4 (283,800) and V4 covered (415,000). Nothing about those prefabs is wrong; the volumes are what `Tools/crate_sizes.py` measures off the meshes, and lowering them to duck under the threshold would corrupt the cargo cost they exist for.

The second cause is the covers. The engine's own hide covers the item entity only, and the vanilla `EquipmentBoxStack_*_covered` prefabs are a hierarchy - stack root plus a separate cover child entity carrying its own `MeshObject`. The root went away, the child did not. That is why it hit V1, V2 and V5 covered but not their bare twins, and it applies to V6 covered too, which simply was not tested.

`IBX_CrateCarryComponent.ApplyStoredVisibility` now runs on the storage-parent transition the component already polls for the physics restore: `ClearFlags(EntityFlags.VISIBLE, true)` on store and `SetFlags` on retrieve, recursive so the cover child follows the root, plus `SCR_EditableEntityComponent.SetVisible(!stored)` so the Game Master icon goes with it. Recursive flags beat any per-prefab change - one edit covers all 42 crates and both causes, and the poll runs on every machine, which is what visibility needs.

Known ceiling, marked `ponytail:` in the source: the poll is the existing 500 ms one, so a crate above the volume threshold can stay drawn for up to one tick after it is loaded.

## 2026-08-27 - GPLv2 license file and a GitHub wiki page

`license.txt` copied from `AMI - ACE Breathing Compat/license.txt`, which is byte-identical in body to `RAMI_AdvancedMedicalInterface/license.txt` - the two sibling addons in this repo that already ship one. Only the first two lines differ: the mod name, and `Copyright (C) 2026 NIRE-Mods contributors`, matching the wording the NIRE-Mods README uses rather than the "Advanced Medical Interface contributors" the source file carried. Nothing third-party is bundled here, so this mod needs no equivalent of RAMI's `THIRD_PARTY_NOTICES.md` paragraph.

`Wiki_Inventory-Boxes.md` is the wiki page, to be created by hand as `Inventory-Boxes` in the GitHub wiki - the wiki is a separate git repository and is not written from here. It is the long-form home the Workshop description no longer has room for, and it carries the preset-authoring walkthrough that the previous entry cut from the description and promised to GitHub. Markdown stays, since the wiki renders it.

Content is the union of the Workshop description and the changelog's player-facing surface, plus the addon ID, GUID and the Mike's UI dependency (`B3F91C6A4E275D08`), which appear nowhere player-facing today. Structured after `GMVehicleLock/README.md` - what it is, install, feature sections, multiplayer, limitations - rather than inventing a new shape.

## 2026-08-27 - Workshop description trimmed to the 5,000 character limit

The description had grown to 6,766 characters with the sizing/naming/depot features and the links section; the Workshop caps it at 5,000. It now sits at 4,997.

Cut rather than compressed everywhere it was possible. The four-step preset authoring walkthrough is gone and points at GitHub instead - it is developer instructions on a player-facing page, and the longest single block in the file. `MULTIPLAYER AND PERFORMANCE` and `DEVELOPMENT STATUS` merged into one section, since both were saying "pre-alpha, feedback welcome". The `FEATURES` list dropped the bullets that only restated a section heading below them.

The rest is wording: limits stated once instead of in both the feature list and the section that explains them, and no sentence repeating what the heading above it already says. Every feature the mod has is still named.

Worth checking before the next update: `Workshop_Description.md` is now within 3 characters of the cap, so anything added has to displace something. The preset walkthrough it now defers to is not yet in the GitHub repository README.

## 2026-08-27 - Workshop description points at GitHub and the Discord thread

`Workshop_Description.md` gains a `LINKS AND FEEDBACK` section above `DEVELOPMENT STATUS`: the https://github.com/Typ2011/NIRE-Mods repository, and the "NIRE Mods" thread on the official Arma Discord.

Placed above the status section rather than at the very end, because the status section already asks for feedback and now has somewhere to send it. Plain text, matching the rest of the file - the Workshop does not render Markdown links.

## 2026-08-27 - Discord post for the sizing, naming and depot round

`Discord_Update_Sizes_Names_Depot.md`, following the shape of `Discord_Update_Fixes.md`: workshop link under the title, one bold lead line, bullet sections, pre-alpha feedback note at the end. Three sections mirroring the Workshop changelog entry - crate size, crate names, depot requests.

Markdown stays here on purpose. Discord renders it, unlike the Workshop text stripped in the entry above, so headings and bold are the format rather than literal characters on the page.

## 2026-08-27 - Workshop text stripped of Markdown

The Reforger Workshop renders description and changelog as plain text, so every `#` heading, `**bold**` and backtick was showing up literally on the mod page.

`Workshop_Changelog.md` and `Workshop_Description.md` are now plain text: headings are uppercase lines, sub-headings plain title-case lines, bullets stay as `-`, em dashes are flattened to hyphens, and no inline emphasis or code spans remain. Both keep the `.md` extension - they are source files in this repo, and the extension is what the editors here read, not what the Workshop reads.

`Workshop_Description.md` also gained the three features added on 2026-08-24 while it was being rewritten - crate size scaling, crate names and depot requests - which it had never mentioned.

The other `.md` files are untouched: `changelog.md`, `PROJECT_CONTEXT.md`, `CLAUDE.md` and `AGENTS.md` are read as Markdown, and the Discord posts go to Discord, which supports it.

## 2026-08-27 - Workshop changelog entry for the sizing, naming and depot round

Player-facing writeup of everything landed on 2026-08-24, condensed into one `24 August 2026 - Crate Sizes, Crate Names, and Player Requests` section above the 23 August entry in `Workshop_Changelog.md`.

Three subsections rather than one bullet list per changelog entry, because the sixteen entries collapse into three things a player notices: crate size now drives capacity, transport cost, grid footprint and weight; crates can be named from both the Game Master editor and a world action, and the name shows in all four places it is drawn; and crates can be requested at a Conflict light vehicle depot for 25 supplies.

The internal entries with no player-visible surface are folded into the fix bullets they caused - the missing resource database overrides read as "worked in the editor, missing on a live server", and the `UseCapacityCoefficient` flag reads as volume limits being ignored. No Discord post this round; the 23 August round has `Discord_Update_Fixes.md` and nothing has asked for a new one.

## 2026-08-24 - Crates are exempt from the vehicle depot cooldown

Reported from the first play test of the previous entry: crates placed fine until a vehicle was requested at the same depot, and every crate after that failed. The log agrees - two crates spawned, then `M998_covered_MERDC.et`, then a run of `Entity budget exceeded for player!` followed by `Error when creating entity from prefab ... E_EquipmentBoxStack_US_01_V2.et`.

The message is misleading. `SCR_CampaignBuildingPlacingEditorComponent.CanPlaceEntityServer` runs the budget check first, then a separate provider gate:

```
if (playerID > -1)
{
	if (providerComponent && providerComponent.IsBudgetToEvaluate(EEditableEntityBudget.COOLDOWN) && providerComponent.GetCooldownValue(playerID) > 0)
		return false;
}
```

`blockingBudget` is an `out` parameter that the budget pass already wrote to, so when the cooldown gate returns false the caller still finds it above `-1` and logs a budget error. The cooldown is per player and per provider, not per prefab, so one vehicle locks out everything at that depot - crates included.

The client never saw it coming, which is why it looked like a silent failure rather than a refusal: every client-side cooldown check is budget-driven and only fires for entities that declare a `COOLDOWN` budget cost. Crates do not declare one, so the build menu offered them normally and only the server said no.

`IBX_CrateBuildingPlacing.c` overrides `CanPlaceEntityServer` and passes `playerID = -1` when the prefab carries `IBX_INVENTORY_BOXES`. That single value is the whole exemption: the vanilla body reads `playerID` for nothing but the cooldown gate, and the parent `SCR_PlacingEditorComponent.CanPlaceEntityServer` does not read it at all, so supplies, rank and the label match still run for crates exactly as before. Reimplementing the method was the alternative and a worse one - the parent's budget pass is not reachable from a modded class, so the whole thing would have had to be copied.

Placing a crate does not start or shorten a cooldown either, since crates carry no `COOLDOWN` budget cost and `GetEntityBudgetValue` therefore returns 0 for them.

Compiles clean at CRC32 `4c8ef67a`, and the user confirmed crates placeable after a vehicle request.

## 2026-08-24 - Overrides were missing from the addon resource database

The crates showed up in Workbench but not on a server. Nothing wrong with the prefabs: `Prefabs/Editor/Modes/EditorModeBuilding.et` and `Prefabs/Props/Military/Compositions/VehicleService_base.et` were simply absent from `resourceDatabase.rdb`, while `EditorModeEdit.et` and every crate prefab were listed. Workbench resolves prefabs straight off disk, so it never noticed; a packed build goes through the `.rdb` and shipped without either override - no `IBX_INVENTORY_BOXES` trait on the depot, no crate registry in `BUILDING` mode, nothing to see.

Worth remembering as a class of bug, because it is invisible from the editor and looks exactly like a broken feature: any *new* file this mod adds needs the resource database rescanned before a build, and `wb_resources register`/`rebuild` silently return false while the Workbench window is unfocused. Same focus trap that keeps swallowing script reloads. After a rescan the database went from 81 to 83 entries and both paths appeared.


## 2026-08-24 - Players request crates at a light vehicle depot

Crates were Game Master placement only. They can now be requested by any player at a Conflict light vehicle depot, and they keep the `Inventory Boxes` filter category they already have in the Game Master asset browser.

The route matters, because there are two different systems behind a "vehicle spawn point" and only one of them is the one in front of players today. `SCR_CatalogEntitySpawnerComponent` is the older scroll-list of `SCR_CatalogSpawnerUserAction`s, and those actions cannot be created from script - a spawner offers exactly as many assets as it has actions authored on its prefab, so adding 42 crates there would have silently dropped most of them. The light depot does not use it. Its `Request Vehicle` action is a `SCR_CampaignBuildingStartUserAction`, which opens the campaign building menu with the depot as provider, and that menu is the editor's own content browser running in `EEditorMode.BUILDING`. It has tabs, and its tabs are `EEditableEntityLabel` filters - the same labels the Game Master browser filters on. That is why the crates can carry their existing category straight across.

Three gates decide whether a prefab shows up there, all of them read in `SCR_CampaignBuildingPlacingEditorComponent.AreLabelsMatching`:

1. The prefab has to be in a placeable-entities registry on `EditorModeBuilding.et`. `Prefabs/Editor/Modes/EditorModeBuilding.et` now overrides the vanilla prefab and adds the existing `Configs/Editor/InventoryBoxes.conf` registry to `SCR_CampaignBuildingPlacingEditorComponent.m_Registries`, plus a content browser tab state keyed on `IBX_INVENTORY_BOXES`. Same override pattern as the `EditorModeEdit.et` this mod already ships.
2. The provider has to list the label in `m_aAvailableTraits`. `Prefabs/Props/Military/Compositions/VehicleService_base.et` now overrides the vanilla prefab with `VEHICLE_CAR IBX_INVENTORY_BOXES`. That prefab is the light depot: `VehicleService_US/USSR/FIA.et` and their `_Small` variants inherit it, while `_Medium` (`VEHICLE_APC VEHICLE_TRUCK`) and `_Large` (`VEHICLE_HELICOPTER`) replace the trait array, so heavy depots and helipads are untouched.
3. The prefab's labels have to contain the provider faction's own label. The 15 US and 14 USSR crates already carried `FACTION_US` / `FACTION_USSR` and stay faction-locked; the 13 faction-neutral crates got all of `FACTION_US FACTION_USSR FACTION_FIA` so every depot offers them. An FIA depot therefore offers those 13 and nothing else.

Verified that the same-GUID prefab override really is a merge and not a replacement before relying on it twice: `wb_resources getInfo` on the modded `VehicleService_base.et` returns the full vanilla entity - mesh, rigid body, destruction phases, resource component, actions manager - with our component edit applied on top. Enum arrays are not in that dump, so the trait edit itself is not visible there and still needs an in-game check.

Placing a crate now costs supplies. Every crate gained `m_EntityBudgetCost` `CAMPAIGN 25` and `m_EntityChildrenBudgetCost` `PROPS 1`, copying what vanilla's single-object buildables (for example `E_CzechHedgehog_S_01_painted`) declare. 25 is a flat guess, not derived from crate size the way capacity and transport cost are - the two size labels in use (`SIZE_XS`, `SIZE_S`) are too coarse to tier on, so if it needs balancing it is one number in each of the 42 prefabs.

The crate descriptions still said "Game Master-only crate with finite physical inventory"; they now say "Crate with finite physical inventory".

`mod_validate` passes all five checks. **Not yet tested in game** - this needs a Conflict world, a light vehicle depot, and a non-Game-Master player.


## 2026-08-24 - Crate name where the crate is an item: vicinity hover and vehicle cargo

The two title widgets covered so far are the crate seen as a *storage*. Hovering it in the vicinity list, or looking at it once it is loaded into a vehicle, draws it as an *item* instead, and that text comes from somewhere else again.

No UI mod this time. `SCR_InventoryUIInfo.GetInventoryItemName(InventoryItemComponent item)` is vanilla's own documented extension point - "Function to override to get custom inventory name" - and it takes the specific item, so a per-item answer is correct even though the UIInfo object it lives on is shared between every instance of the prefab. `IBX_CrateInventoryUIInfo` overrides it to return the crate's name, falling through to `super` when there is none, and is now the `ItemDisplayName` class on all 42 prefabs in place of the plain `SCR_InventoryUIInfo`. The GUID stays `{A1E6470C8DBF395F}` so nothing else in the prefabs moves.

Checked before writing it that this actually covers both reported cases: `GetInventoryItemName` has exactly two callers, `SCR_InventoryMenuUI.SetFocusedSlotEffects` (the hover info panel) and `SCR_InventorySlotUI.ShowItemDetails` (slot details), which are the vicinity hover and the vehicle slot respectively.

The prefab edit is a binary read/write substitution with a `demo()`-style self-check, asserting exactly one hit per file, a byte delta equal to the class-name length difference, and idempotence.

`mod_validate` passes all five checks. **Not yet recompiled** - Workbench had lost focus again. The previous round did compile clean at CRC32 `2f1bb536`, which includes the `FindAndSetTitleName` override for the traverse title bar; that one has not been tested in game yet either.

## 2026-08-24 - Crate name in the inventory screen: the other title widget

The previous entry moved the inventory header off the shared prefab UIInfo and onto a modded `SCR_InventoryStorageBaseUI.Init`, writing the `StorageName` widget. That fixed the leak but changed nothing visible, because it is not the widget a player looks at.

A crate reaches the inventory screen two ways. Opened directly it becomes its own panel with a `StorageName` header, which `Init` fills. Opened out of the vicinity - which is where the world Open action actually lands - it is *traversed into* instead, and gets a traverse title bar whose text is written by `FindAndSetTitleName` into a widget called `TextC`. Both read the same shared `ItemAttributeCollection` UIInfo, so both needed overriding; only one was.

`FindAndSetTitleName` is now overridden alongside `Init`, and the crate lookup both share is factored into one helper.

Worth recording while reading this code: `SCR_InventoryStorageLootUI.Init` does not call `super.Init` at all. It replaces it wholesale and hardcodes `#AR-Inventory_Vicinity`, which is right for the vicinity panel but means a base-class `Init` override never runs for it.

Not covered, and out of scope for now: a crate sitting in a vehicle's cargo is drawn as an *item*, and its slot label comes from the same shared collection, so it still shows the generic prefab name there.

Validated: `mod_validate` passes all five checks, compile clean at CRC32 `2f1bb536`, and the user confirmed both title widgets correct in game on 2026-08-24.

## 2026-08-24 - Three crate-naming bugs found on a dedicated server

All three reported from live dedicated-server testing, and all three confirmed fixed there by the user on the same day.

**A crate name leaked into every crate of the same type, and into the next one placed.** Root cause: `BaseInventoryStorageComponent.GetAttributes()` returns an `ItemAttributeCollection` that is shared between all instances of a prefab, not a per-instance copy. Writing the name into its UIInfo therefore renamed every crate of that type on that machine, and a freshly placed crate then captured the already-renamed value as its own "default", so it came out named after its neighbour. This is the same class of trap as `SCR_EditableEntityComponent.GetInfo()` falling back to shared prefab data, which the previous entry already avoided via `SetInfoInstance` - the storage side was missed because it looked like an ordinary component getter.

Nothing writes that UIInfo any more. The inventory screen's storage header is instead overridden per open storage in a modded `SCR_InventoryStorageBaseUI.Init`, which reads the crate's own name off its component and sets the widget text directly. `m_sDefaultName` now stays pristine, so clearing a name restores the real prefab name.

**A Game Master rename did nothing.** The differential is what pinned it: applying a preset named the crate correctly, and preset apply renames entirely server-side, while a manual rename has a client leg. So the broken piece was the client-to-server `[RplRpc]` declared on `IBX_GMInventoryEditorComponent` - it never arrives on a dedicated server, even though that same component's server-to-client broadcast demonstrably does. This sits next to the already-documented finding that `[RplProp]` on these script components never reaches a proxy; something about them is not fully registered for replication, and only the outbound direction happens to work.

Rather than chase it, the rename now travels on the channel vanilla uses for every client request: a `modded SCR_PlayerController` with `IBX_RequestCrateRename(RplId crateId, string name)`, resolving the crate through `Replication.FindItem` exactly as the carry component already resolves a holder. The player controller is owned by the requesting client and exists for a Game Master and a plain player alike, so both entry points use it and the component's own `RpcAsk_Rename` is gone.

**The rename menu did not open for a player.** `IBX_RenameCrateAction` overrode `CanBroadcastScript()` to return false, on the reasoning that a purely local menu has nothing worth replicating. Suppressing the broadcast also stopped `PerformAction` from running on the performing client, so nothing opened. The override is removed; `IBX_CarryCrateAction` is the confirmed-working shape and does not have one either, and the existing local-player guard is what keeps the server's own copy inert.

Also hardened while in there: `RplLoad` no longer returns false on an unreadable name. A false return fails the whole load, which is not a trade worth making for a cosmetic string.

## 2026-08-24 - Applying a preset names the crate after it

A preset already knows what it is; the crate it fills should say so. `ApplyPresetServer` now calls `RenameServer(preset.m_sName)` after the contents land, so the label and the contents cannot drift apart.

Only after they actually land. `ReplaceContentsServer` has four failure exits and one success exit, and its return value is a human-readable message rather than a status, so it now reports success through an `out bool` instead of the caller sniffing the string. A failed apply leaves the existing name alone.

Pasting deliberately does not rename. Pasted inventory is just an item string with no name of its own, and the placeholder preset it is parsed into is called "Clipboard", which is not a name anyone wants on their crate.

The Game Master editor's name field is updated by a push from `ApplyNameLocally` (`IBX_GMInventoryEditorUI.ReportCrateName`) rather than by re-reading the component after the mutation result comes back. The rename broadcast and the mutation result travel as separate RPCs on separate replicated objects - the crate's component and the editor manager entity - so their arrival order is not guaranteed, and a pull would sometimes read the old name. The push is a no-op when the editor is closed or open on a different crate, and on a dedicated server where there is no UI at all.

Validated: `mod_validate` passes all five checks. Compile clean at 11181 classes, CRC32 `053ff569` to `36bfd481`, no `IBX_` diagnostics.

Also confirmed in this session, after the Workbench restart the previous entry asked for: `resourceDatabase.rdb` now carries `Configs/System/chimeraMenus.conf`, and `Menu preset 'IBX_CrateRenameMenu' not found!` is gone from the log while Mike's UI's own `MUI_SampleMenu` still reports it - the same check still runs, and it no longer fails for ours.

## 2026-08-24 - Crate rename: free cursor for players, and a header that fits

Two problems with the first rename build, both reported from live testing.

**The player could not type.** The dialog was a workspace modal added over live gameplay, so the mouse stayed captured by the character controller - the field was visible but not clickable. A workspace modal is fine inside the Game Master editor, which already runs with a free cursor, and useless outside it. Rebuilt as a real menu: `IBX_CrateRenameMenu` now extends Mike's UI's `MUI_MenuBase` and is opened with `GetGame().GetMenuManager().OpenMenu(ChimeraMenuPreset.IBX_CrateRenameMenu)`, which is what frees the cursor and routes keyboard input to the UI. `MUI_MenuBase` owns mounting, ticking, blur and back/Escape, so only `BuildUI` is implemented here - the manual `CallLater` tick, `AddModal`/`RemoveModal` and `OnController` handling all went away, and the layout reuse went with them. The backdrop is now painted by the overlay panel instead of the borrowed layout.

Registering the menu took two non-obvious pieces, both found by reading the two sibling addons in this repo that already ship custom menus:

- `Configs/System/chimeraMenus.conf` must carry **vanilla's GUID `{C747AFB6B750CE9A}`**. The engine merges every addon's copy of that resource into one preset list - confirmed by reading the merged config back, which returns all of vanilla's presets with ours appended. Under a fresh GUID it is silently ignored and `OpenMenu` logs `Menu preset 'IBX_CrateRenameMenu' not found!`. Mike's UI's own `MUI_SampleMenu` uses a fresh GUID and fails in exactly the same way in the same log, so the README snippet alone is not enough.
- The preset needs `ActionContext "MenuContext"`, which both sibling addons set.

**The Game Master header overflowed.** A `MUI_TextField` is a caption stacked on top of its input box and needs the same vertical room the filter and quantity rows give theirs; the header was 48 tall, so the input box spilled out of the header and painted across the crate contents title. Header is now 74, and the `RENAME` and `CLOSE` buttons plus the title are bottom-aligned so they sit level with the input box rather than with its caption, matching the existing filter and quantity rows.

Validated: `mod_validate` passes. Compile is clean at 11181 classes, CRC32 `053ff569`, no `IBX_` diagnostics. Reading `Configs/System/chimeraMenus.conf` back through the resource system returns the merged preset list containing `IBX_CrateRenameMenu` with its layout, class and action context.

Open item: Workbench did not import the newly created `chimeraMenus.conf` into `resourceDatabase.rdb` during the session that created it - it cached a null GUID for the path after reading the file mid-write, after which `register` returns false and `rebuild` reports "Cannot find metafile configuration". Restarting Workbench once should import it; the config itself is correct and resolves.

## 2026-08-24 - Crates can be named, by Game Masters and by players

Every crate can now carry a custom name of up to 32 characters. Game Masters type it into a field in the inventory editor's header and press `RENAME`; any player can walk up to a crate and use its new `Name Crate` world action, which opens a small Mike's UI dialog with the same field plus `SAVE`, `CLEAR NAME` and `CANCEL`.

The name shows up in two places. The inventory screen prints it as the storage header where the prefab's own "Finite Inventory Crate" used to sit, and the world action itself reads `Rename Crate (<name>)` once a name is set, so a player sees it without opening anything. Game Master entity lists and hovers pick it up as well.

The name lives on `IBX_GMInventoryEditorComponent`, which is already on all 42 crates and already owns the crate's authoritative state - adding a component instead would have meant a second block in all 42 prefabs for nothing. It gets its own client-to-server path rather than reusing the `SCR_EditorManagerEntity` route the inventory mutations take, because a plain player renaming a crate has no editor manager at all. `RequestRename` checks authority and either renames directly or sends `RpcAsk_Rename`; the server sanitises, applies locally and broadcasts `RpcDo_SetName`. Applying locally is not redundant - a broadcast RPC does not loop back to its own sender, which is what breaks listen-server hosts.

`[RplProp]` is deliberately not used. It has never once fired on a proxy copy of a `ScriptComponent` in this addon (the full trail is in `PROJECT_CONTEXT.md` under Physical Carry/Drag), so join-in-progress goes through `RplSave`/`RplLoad` instead, which is the mechanism vanilla itself uses for exactly this on `SCR_AIGroup`'s custom group name.

Both display surfaces are per-instance overrides on purpose. `SCR_EditableEntityComponent.GetInfo()` falls back to `SCR_EditableEntityComponentClass.GetInfo()`, which is shared prefab data, so writing a name into it would rename every crate of that prefab at once - `SetInfoInstance` is the documented per-instance path and is what vanilla's own editable components use. The storage header goes through the storage's own `ItemAttributeCollection` UIInfo.

Server-side sanitising strips `#`, because a string starting with `#` makes the engine treat the whole thing as a localization key, flattens newlines and tabs to spaces, trims, and clamps to 32 characters. The client clamps length too, so an oversized string never reaches the wire.

The player dialog reuses `GMInventoryEditor.layout` rather than adding a second layout and resource GUID: it already provides the backdrop, the Mike's UI host and the `EditBoxWidget` the workspace modal needs for keyboard focus, and its two native scroll lists are just hidden. The Game Master path renames inline from the editor header rather than through an overlay, so the editor never has to stack a second modal on top of itself.

`IBX_RenameCrateAction` was added to all 42 prefabs at sort priority 9, after the six carry/drag actions. The bulk edit was a one-shot script doing binary read/write, so the mixed CRLF/LF line endings inside individual `.et` files survive and no BOM is introduced; it carries a `demo()` self-check, refuses to run if the anchor is not found exactly once, and is a no-op on a second run.

Validated: `mod_validate` passes structure, gproj, prefabs, configs and references. Workbench recompiled cleanly - 5714 files and 11180 classes against 5713/11178 before, CRC32 `684db8fe` to `0cd11ef1`, no `IBX_` diagnostics. Play mode started with no `Can't load prefab` or `Unexpected data` in `error.log`, and a patched crate prefab spawns in the editor with all 28 components intact.

Three follow-up rounds were needed before this worked on a dedicated server - see the later entries above for the cursor, the client-to-server leg, the shared prefab data and the four separate places a crate name is drawn.

## 2026-08-24 - Crate tare weight scales with the crate

Last flat number on the crates: every one weighed 10 kg, from the .50 cal can to the 3.3 cubic metre pallet stack.

A crate is a hollow shell, so its mass tracks surface area rather than the volume it encloses - weight goes with `litres ** (2/3)`, not with litres. `TARE_WEIGHT_COEFFICIENT` is 0.47, anchored on the wooden weapon crate at 20 kg, which is what a real 48-inch ammo crate weighs empty. The same coefficient independently puts the .50 cal can at 2 kg against a real M2A1's 2.7 kg, which is the check the shell law had to pass - a linear law would have made the can ten times too light for its size. Range is now 2 kg to 105 kg.

`as_item()` returns a `NamedTuple` rather than a widening plain tuple, so call sites read `item.weight` instead of indexing.

Two things this deliberately does not touch. `m_fMaxWeight` stays at 1,000 kg on all 42 - that is what a crate can carry, not what it weighs, and it is still far from binding. `RigidBody.Mass` stays at the inherited 10 as well; that is physics, and carry/drag behaviour is tuned around it.

Checked before writing: `IBX_CrateCarryComponent` never reads weight, so carry and drag are unaffected. Vehicle weight limits are 4,500 kg on the trucks and 360-400 kg on the jeeps, against 50,000-1,000,000 units of cargo volume, so volume stays the binding limit everywhere. `demo()` now asserts that outright across all 42 crates against the tightest vehicle, so a future weight change cannot quietly turn a volume-limited vehicle into a weight-limited one.

Known ceiling, recorded in the script: the pallet stacks are piles of separate crates rather than one hollow box, so treating them as a single shell undercounts them - the largest lands at 105 kg where summing its crates suggests roughly twice that. Left as the simpler single law because nothing gates on crate weight.

Also fixed while adding the substitution: `Weight [\d.]+` matches inside `m_fMaxWeight 1000`. The pattern carries a negative lookbehind, verified as exactly one match per prefab before the first write.

Validated: `mod_validate` passes structure, prefabs and references; `demo()` passes; a second `--write` run produces no further change; `m_fMaxWeight 1000` intact on all 42. Confirmed working in play by the user on the same day, which closes out the crate sizing work - capacity, transport cost, grid footprint and tare weight are all verified in game.

## 2026-08-24 - Crate sizing confirmed in play, written up in PROJECT_CONTEXT.md

Confirmed working by the user: capacity, transport cost and grid footprint all scale per crate. The five entries below this one each closed with "Behaviour not yet confirmed in play" - that is now settled for all of them.

`PROJECT_CONTEXT.md` gets a dedicated `Crate Sizing` section rather than the single sprawling bullet the feature had accumulated under Current Progress, which is now a two-line pointer. The section records the generator and how to re-run it, the mandatory `UseCapacityCoefficient 0`, both unit scales with their anchors and why they cannot be swapped, the `ItemDimensions` and `m_Size` rules including `SLOT_OVERRIDES`, the two invariants the generator enforces, why nesting cannot be exploited, and what is deliberately not scaled.

Two entries added to `Key lessons from getting here`, both general enough to matter beyond this feature: that an engine-side prefab attribute invisible to script can silently neutralise everything tuned next to it, found only by diffing against a vanilla block that works; and that vanilla's abstract unit scales do not transfer between contexts, so each needs its own constant anchored to a vanilla example from that same context.

One line added to `Known Constraints`: the crate storage numbers are generated, so edit `Tools/crate_sizes.py` and re-run rather than editing a prefab directly, or the next run overwrites the change.

## 2026-08-24 - Hand-set grid footprints for the V1, V2 and V3 pallet stacks

Requested after looking at the stacks in the inventory: V1 and V2 should sit at `SLOT_2x2`, V3 at `SLOT_3x3`, the rest as they were.

Volume alone cannot produce that. Every stack measures over the 250 litre tier, so all twenty-four landed on `SLOT_3x3`, and the ordering it implies is wrong anyway - the V2 stack at 1,061 litres and the V5 stack at 1,062 litres are within a litre of each other, yet only V2 should shrink. The V1 and V2 stacks are broad but low, which the grid should show and a bounding-box volume cannot.

Added `SLOT_OVERRIDES`, a name-keyed table checked before `SLOT_TIERS`. Keys are the `_Vn` suffix, which only the stacks carry, and each key also catches its `_covered` twin - eight prefabs move to `SLOT_2x2`, four confirm `SLOT_3x3`. `as_item()` takes the prefab stem to match against; everything else is unchanged.

Distribution across all 42 is now 2 at `SLOT_1x1`, 5 at `SLOT_2x1`, 16 at `SLOT_2x2` and 19 at `SLOT_3x3`. `m_Size` feeds `SCR_ItemAttributeCollection.GetSlotSum()` and the inventory UI only, so none of this touches what a crate holds or costs to transport - those numbers are untouched.

Validated: `mod_validate` passes structure, prefabs and references; `demo()` now checks every override against every matching prefab, including a covered twin and a USSR variant by name; a second `--write` run produces no further change. Behaviour not yet confirmed in play.

## 2026-08-24 - Every crate now fits in a transport truck

Requirement: all 42 crates must be loadable, and the biggest must still fit a truck. The previous entry left the two `_V4_covered` pallet stacks at 1,105,500, above every cargo hold in the game.

The crate's transport cost moves off the storage scale onto its own constant, `ITEM_UNITS_PER_LITRE = 125`, anchored to the transport truck rather than to backpacks: `M923A1_transport` and `Ural4320_transport` hold 1,000,000, and their cargo bed is roughly 8 cubic metres. That rate puts about two of the largest pallet stacks, or thirty wooden crates, on one truck - which is what fits in reality. Reusing the storage side's 333 was the mistake; vehicle cargo is authored on a far coarser scale than worn containers, and the two anchors do not transfer.

`main()` now refuses to write a prefab that no transport truck could take, naming the offender and the constant to lower, so a future oversized crate fails loudly instead of shipping unloadable.

Result across all 42: every crate fits a truck, none is unloadable, and the spread stays proportional - 1,100 for the .50 cal can up to 415,000 for the largest covered stack. Thirteen crates are small or flat enough for a jeep's `MaxItemSize 50 200 50`, including both wooden launcher boxes at 113 x 38 x 23 cm. The eight tallest stack variants are truck-only.

Side effect worth recording: a crate now costs less room than it offers, so hauling crates beats hauling loose items, which is the point of a logistics crate. This cannot be exploited by nesting - every crate's dimensions exceed every crate's `MaxItemSize`, so no crate fits inside another one or inside a backpack. The self-check no longer asserts the old cost-exceeds-capacity invariant and asserts truck fit instead, including headroom for a second copy of the bulkiest crate.

Validated: `mod_validate` passes structure, prefabs and references; `demo()` passes; a second `--write` run produces no further change. Behaviour not yet confirmed in play.

## 2026-08-24 - Crates now cost room to transport in proportion to their size

Volume limits confirmed working in play after the `UseCapacityCoefficient` fix. Follow-up: every crate still cost the same to transport - `ItemDimensions 100 50 50`, `ItemVolume 25000`, `m_Size SLOT_2x2` on all 42 - so a .50 cal can took as much room in a truck as a pallet stack, and the small crates had no reason to exist.

The crate's own item attributes now come from the same measured bounding box as its storage caps, added to `Tools/crate_sizes.py` as `as_item()`:

- `ItemVolume` is the full outer box at 333 units per litre, with no fill factor. A crate therefore always costs more room than it offers - the difference is exactly `FILL`. Range is 3,000 for the .50 cal can to 1,105,500 for the largest covered stack, against 25,000 flat before.
- `ItemDimensions` is real centimetres, sorted longest first. This is the form vehicle gates are written in - the Humvee, UAZ and S105 all use `MaxItemSize 50 200 50` - so a long flat launcher box (113 38 23) slides into a jeep while a fat equipment box (93 49 49) only just does and the weapon crate (122 57 40) needs a truck.
- `m_Size` scales the inventory grid footprint across all four `ESlotSize` tiers by outer volume: under 15 L `SLOT_1x1`, under 60 L `SLOT_2x1`, under 250 L `SLOT_2x2`, above that `SLOT_3x3`. Ten of the stack prefabs stored this numerically as `m_Size 3`; the substitution accepts both spellings and normalises to the named form.

Consequence worth knowing: the two largest crates, `E_EquipmentBoxStack_US_01_V4_covered` and its USSR twin, come out at 1,105,500 and now exceed every vehicle in the game - the biggest cargo holds in vanilla are the transport trucks and the Mi8 at 1,000,000. They are 1.52 x 1.65 x 1.32 m pallet stacks, so refusing them is physically honest, but it does make them untransportable. Lower `UNITS_PER_LITRE`, or cap the value in `as_item()`, if they should be loadable.

`Weight` is left at 10 kg on every crate - out of scope for this change, and a knob of its own.

Validated: `mod_validate` passes structure, prefabs and references; `demo()` covers the new path (a crate costs more than it holds, a can costs under a tenth of a crate, dimensions come out `122 57 40` for the weapon crate, slot tiers span `SLOT_1x1` to `SLOT_3x3`); a second `--write` run produces no further change. Behaviour not yet confirmed in play.

## 2026-08-24 - Volume limits were being ignored: UseCapacityCoefficient

Reported from play: a .50 cal ammo can still swallowed a whole player inventory despite being authored at `MaxCumulativeVolume 1800`.

The volume was never consulted. `SCR_UniversalInventoryStorageComponent` has an engine-side `UseCapacityCoefficient` flag that sizes the storage from its slot count instead of from `MaxCumulativeVolume`; with `NumSlots 100` that is effectively unlimited. Vanilla writes `UseCapacityCoefficient 0` in 108 prefabs and never writes `1` - including both containers this mod's numbers are calibrated against, the ALICE Medium backpack and the Humvee trunk - so the class default is the coefficient path. The crates inherit from `AmmoBox_Base.et` through `Props_Base.et`, and no prefab in that chain turns it off.

Fixed by writing `UseCapacityCoefficient 0` into the custom storage of all 42 crates, immediately above `MaxCumulativeVolume`, where vanilla puts it. `Tools/crate_sizes.py` now emits the line as part of the volume substitution, and the substitution absorbs an existing line rather than stacking a second one, so repeated runs are idempotent.

The flag is not exposed to script - it appears in no `.c` file and in no entry of the script API - so this was found by diffing the crate storage block against vanilla containers that do enforce their volume.

Note the attribute was missing since the crates were first authored, so the previous entry's measured values were correct but inert.

Validated: `mod_validate` passes structure, prefabs and references; `demo()` self-check passes; all 42 prefabs carry exactly one `UseCapacityCoefficient 0`; a second `--write` run produces no further change. Behaviour not yet confirmed in play.

## 2026-08-24 - Crate inventory capacity now follows the crate's physical size

All 42 crates shared one storage configuration - `MaxCumulativeVolume 10000000` and `MaxItemSize 200 200 200` - so a .50 cal ammo can held exactly as much as a pallet of equipment boxes, and ten times what a truck holds.

Capacity is now derived from each crate's actual mesh. The engine has no setter for `MaxCumulativeVolume`, `MaxItemSize` or `NumSlots` (`BaseInventoryStorageComponent` exposes `GetMaxVolumeCapacity()` and no counterpart), so this cannot be done at runtime; the numbers are measured once and baked into the prefabs by `Tools/crate_sizes.py`.

The measurement reads the bounding box straight out of the `.xob` header - `FORM`/`XOB9`/`HEAD`, six little-endian floats at offset `0x18` - for every mesh a crate previews, unioning the covered stack variants with their covers. The wooden weapon crate comes out at 1.219 x 0.398 x 0.568 m, the real 48-inch rifle box, which is the script's self-check.

Two things were rejected first. The editor's `SIZE_` auto-labels sort all 42 crates into just `SIZE_XS` and `SIZE_S`, and give a six-crate stack the same label as a single wooden box - too coarse and wrong. The inherited vanilla storage (`{5476A2F100DF4EFF}`) carries no volume tuning of its own either; `AmmoBox_Base.et` leaves it at engine defaults, so there was nothing to reuse.

Calibration is anchored to vanilla containers rather than to reality, because Reforger inflates item volumes: the ALICE Medium backpack holds about 30 real litres and is authored as `MaxCumulativeVolume 10000`, giving 333 units per litre. A fill factor of 0.6 accounts for walls, lids and the gaps in a stack. Both constants sit at the top of the script.

`MaxItemSize` splits on the crate's longest dimension at 0.9 m. Below that, `20 20 20` admits magazines (`15`), mortar shells (`8 8 20`), PG-7 rockets (`20`) and carbines, and refuses full-size rifles (`25`) and launchers (`35`). At or above it, `40 40 40` admits everything in the game.

Resulting values check out against vanilla: the wooden weapon crate at 55,000 is 5.5 backpacks, the arsenal box at 197,700 lands next to a BTR-70's 200,000, and the largest covered stack at 663,300 stays under `Vehicle_Cargo_Base`'s 1,000,000. Small ammo cans are the tight end - the .50 cal can at 1,800 holds roughly two rifle magazines, which is internally consistent with the backpack anchor but stricter than the real can; raise `FILL` or `UNITS_PER_LITRE` if that reads badly in play.

`m_fMaxWeight` (1000 kg) and `NumSlots` (100) are unchanged - volume binds well before either does.

Validated: `mod_validate` passes structure, prefabs and references; the script's `demo()` self-check passes. Behaviour not yet confirmed in play.

## 2026-08-23 - Workshop changelog and Discord post for the fix round

Player-facing writeups for everything fixed since the 21 August carry/drag release: destructibility restored on all 42 crates, held-crate collision suspended everywhere, physics restored on unload from a vehicle, holds released on death/unconscious/vehicle boarding/disconnect, the respawn lockout, join-in-progress collision, and the listen-server `Carry Crate` visibility bug.

`Workshop_Changelog.md` gets a `23 August 2026 - Carry, Physics and Destruction Fixes` section above the 21 August entry. The Discord post is a new `Discord_Update_Fixes.md`, following the same shape as `Discord_Update_CarryDrag.md` (workshop link under the title, short bold lead, bullet sections, pre-alpha feedback note at the end).

## 2026-08-23 - Unconscious holders drop the crate

A holder who fell unconscious kept the crate latched in front of them: `IsHoldStillValid` only rejected dead holders (`CharacterControllerComponent.IsDead()`), and an unconscious player cannot reach the drop action either, so the crate stayed floating over a ragdolled body until they died or bled out.

Fixed with the same guard the death case uses - `IsHoldStillValid` now also rejects `controller.IsUnconscious()`. Every hold routes through `TickHoldCommon`, so carry and drag are both covered by the one check; `StopServer` restores the crate's physics as usual.

Validated: recompiled clean, `Module: Game; ... CRC32: 2e0e322f`, no errors. Behaviour not yet confirmed in play.

## 2026-08-21 - Feedback round 5: unloaded crates come back out of storage with physics still suspended

Rounds 2 and 4 both blamed collider interaction layers for unloaded crates being walk-through while still stopping bullets. Both were wrong, and round 4's fix never even executed - the `InventoryItemComponent.m_OnParentSlotChangedInvoker` subscription it hung everything on silently found no component, because the inventory system does not create that component until after the frame the subscription was made. Nothing was ever measured on the unload path.

Replacing the event hook with a poll that prints only on state change gave the answer in one run:

```
parented=0  sim=1  layers=10486528,49152     loose in the world, blocks characters
parented=1  sim=0  layers=10486528,49152     inside the vehicle storage
parented=0  sim=0  layers=10486528,49152     unloaded - back in the world, still NONE
```

The collider layers are byte-identical at every step, so they were never the problem. The engine drops an item's physics into `SimulationState.NONE` when it enters a storage and does not bring it back when the item leaves one; vanilla item components restore it themselves (`SCR_HeadgearInventoryItemComponent` is the clearest example), and these crates are props with no such component. `NONE` means the body is not in the collision world, which is exactly why the crate stopped blocking characters.

Fixed by restoring the simulation state the crate had while loose. The state is captured from the crate itself rather than hardcoded, skipped while a hold has deliberately parked it in `NONE`, and re-applied when the crate stops being parented.

Detection is a 500 ms parent check per crate, marked with a `ponytail:` comment. It is polling where an event would be tidier, but the invoker hook demonstrably never fired and no `EntityEvent` covers a parent change - `EntityEvent` has `INIT`, `FRAME`, `CONTACT`, `PHYSICSMOVE` and friends, nothing for reparenting. One `GetParent()` per crate twice a second is cheap; the note records the upgrade path if the invoker's timing can be pinned down.

Removed along the way: the whole `m_aWorldGeomLayers` capture/restore added in round 4, the parent-slot invoker subscription, and every temporary diagnostic. The file contains no `Print()` calls again.

Also fixed, found while re-reading the same path and unrelated to the walk-through: `SetCrateCollisionEnabled(true)` returned early when the crate had no physics, which is exactly the case for a crate carried straight into a vehicle storage. `m_bSimulationSuspended` was left stuck `true`, so the next carry of that crate skipped suspending collision entirely - it would have gone back to lifting players while carried. The flag is now cleared even when there is no body to restore.

Validated: recompiled clean, `Module: Game; ... CRC32: e123c52f`, no errors. Behaviour not yet confirmed in play.

## 2026-08-21 - Feedback round 4: crate stops blocking characters after a vehicle round trip

User confirmed round 3: dying while carrying now releases the crate, and a respawned player can carry again. Remaining report: after unloading a crate from a vehicle, players walk straight through it, while bullets still hit it and can destroy it.

Round 2 dismissed this as stock dropped-item behaviour. That was wrong - it is specific to crates that have been through a storage, and a crate placed directly in the world blocks characters normally.

Bullets hitting while characters pass through means the crate's colliders are present but no longer on a character-blocking layer. Reading the vanilla data explains how that happens: these crates declare no `InventoryItemComponent` at all (neither do vanilla backpacks) - the inventory system creates one at runtime for any entity whose storage carries item attributes. Coming back out of a storage the crate is handed a newly built physics body, and that body's colliders do not carry the prop layers the prefab's own body had.

Fixed by capturing the crate's per-collider interaction layers while it sits loose in the world, and re-applying them when it leaves a storage. The capture is lazy rather than spawn-only, so a crate that begins life inside a storage still picks up a baseline the first time it is seen loose. The hook is `InventoryItemComponent.m_OnParentSlotChangedInvoker` with `newSlot == null`; subscription happens a frame after init, because the component does not exist yet at init time.

The re-apply is deliberately deferred by a frame. Round 2 measured `SetGeomInteractionLayer` on the stacked-crate prefabs being ignored outright - the masks read back unchanged immediately after the write - and vanilla's own `SCR_HybridPhysicsComponent` only ever sets layer masks in the statements directly following `Physics.CreateStatic`/`CreateDynamic`. Writing to a freshly created body appears to be the supported path, so the restore waits for the engine to finish building it. If that turns out still not to stick, the fallback is the full `SCR_HybridPhysicsComponent` treatment: capture mass and masks, `physics.Destroy()`, recreate, then set the masks.

Validated: recompiled clean, `Module: Game; ... CRC32: 0d7c74d2` (from `27f043ea`), no errors. Behaviour not yet confirmed in play.

## 2026-08-21 - Feedback round 3: crate still latched to a player who dies while carrying

User confirmed round 2: crates are destructible again, and a carried crate no longer collides with characters. Remaining report: dying while carrying leaves the crate stuck in the carried state, and after respawning that player can no longer pick up any crate at all.

Round 1 had already added a death check (`IsHoldStillValid`, polled from the mode timers), so this is that check failing rather than an unhandled case. Three separate places could produce the reported behaviour, and all three are fixed rather than guessing which one fires - each is a defect on its own terms.

**The stop broadcast never reached the client.** `ApplyHoldChangedLocally` resolved the holder's `RplId` back to an entity and returned early unless it matched `SCR_PlayerController.GetLocalControlledEntity()`. That comparison holds while carrying normally, but a player who died is a corpse or an already-respawned entity by the time the stop arrives, so it fails exactly in the case being fixed and `IBX_GMCarryClient.OnHoldEnded` was never called. The client then believes forever that it is still holding something, and `CanToggleCarry`/`CanStartDrag` hide Carry on every crate - which is precisely "can't pick up any more crates". The stop path no longer consults the local player at all: `OnHoldEnded` already ignores crates this client was not tracking, and that is the only check it needs. The start path still resolves the holder, since it must only apply the speed cap on the holder's own machine.

**`StopServer` could abandon the hold half-released.** It repositioned the crate (`SnapToTerrain`, `SyncCrateTransform`, `ResyncPhysics`) *before* clearing the mode, releasing the holder registration and broadcasting the stop. With a dead or deleted holder the reposition is the most likely step to fail, and anything failing there left `m_eMode` still `CARRY` - the crate latched to a corpse, hidden from every action because its mode never changed. Reordered so the hold is fully released first and the world is touched last. `ReleaseHolderRegistration` now takes the holder as an argument, since `m_HolderCharacter` is cleared before it runs.

**`IsHoldStillValid` was permissive about a holder that stopped looking like a character.** It returned `true` when the `ChimeraCharacter` cast failed, and `!controller || !controller.IsDead()` also returned `true` when the controller was missing. Holders are always player characters, so both of those mean the entity is gone or going, not that the hold is still good. Both now end the hold.

Added as a backstop on the client: `IBX_GMCarryClient` records which character started the hold, and `IsHoldingAnything()` self-heals when that no longer matches the locally controlled entity - the player died and respawned, so whatever was held belonged to the old body. This guarantees a respawned player can carry again even if a stop broadcast is lost entirely, independent of the three fixes above.

Removed the temporary `IBX_DiagDump` diagnostic block along with the `OnPostInit`/`EOnInit` hooks that drove it. It had answered both questions it was added for - that the crates' destruction component was disabled rather than absent, and that interaction-layer writes were being ignored on the stacked-crate prefabs. The file again contains no `Print()` calls.

## 2026-08-21 - Feedback round 2: destruction enabled on every crate, collision suspended via simulation state

Follow-up after testing round 1. Two of the four reported issues were confirmed fixed (loading a carried crate into a vehicle, and join-in-progress). The remaining two - indestructible crates and crates still colliding while carried - were both diagnosed wrongly the first time. A temporary `IBX_DIAG` dump added to `IBX_CrateCarryComponent` settled both against real runtime data instead of inference; the user also extracted the game data to `F:\Reforger extracted`, which made the vanilla prefab chain readable for the first time.

**Indestructible crates: the inherited destruction component was disabled, not missing.** The runtime dump reported `damageMgr=0 multiPhase=0` on every crate variant, including ones whose vanilla base prefab plainly declares `SCR_DestructionMultiPhaseComponent`. Tracing the chain explains it: the whole ammo-box family roots at `Prefabs/Props/Core/AmmoBox_Base.et`, which declares the component from the component template `Prefabs/Props/Core/DestructionMultiPhase_Base.ct` - and that template ships with `Enabled 0`. No prefab in the chain ever sets it back to 1, and a disabled component is never instantiated, so `FindComponent` correctly finds nothing. This is vanilla behaviour, not something the mod broke: the stock ammo boxes and equipment box stacks are indestructible for the same reason. The template also supplies the `Default` hit zone (`HZDefault 1`), which matters because `SCR_DestructionDamageManagerComponent.OnPostInit` bails out with `if (!GetDefaultHitZone()) return;`.

Fixed by adding an explicit `SCR_DestructionMultiPhaseComponent "{58F89684469E44B1}" { Enabled 1 }` override to all 41 vanilla-derived crate prefabs. That GUID is the component id used at every level of the chain, verified identical across all 41, so a single uniform override lands on the inherited component rather than creating a second one.

The forty-second crate, `E_ArsenalBox_US_Weaponslllllll.et`, is the one prefab built from scratch as a bare `GenericEntity` with no base to inherit from - it genuinely had no damage manager of any kind, matching vanilla's own arsenal boxes. It gets the component added outright, referencing the same `DestructionMultiPhase_Base.ct` template so it picks up the default hit zone, with `Enabled 1` and `m_fBaseHealth 400`.

Balance note, inherited rather than chosen: the template sets `Kinetic multiplier` 1 against `Explosive multiplier` 90, over a base health of 400-800 depending on variant. Crates are therefore destroyed readily by explosives and only slowly by small-arms fire. Destroyed crates are deleted along with whatever is inside them. Both are tunable per prefab if the vanilla balance turns out to be wrong for this mod.

**Collision while carrying: interaction-layer masks were the wrong mechanism.** Round 1 cleared the body mask and every collider mask and read them straight back to prove the write landed. On `E_AmmoBox_545x39_2160rnd` it did (`body=0 layers=0,0`). On `E_EquipmentBoxStack_USSR_01_V1` the identical code read back completely unchanged (`body=10535680 layers=10486528,49152`) - the engine ignored the writes outright on that body. Since the stacked-crate prefabs are exactly the ones players reported being lifted by, the layer approach was never going to work there no matter how carefully it was applied.

Replaced with `SCR_PhysicsHelper.ChangeSimulationState(owner, SimulationState.NONE, true)` while held, restoring the previously captured `GetSimulationState()` on drop. `SimulationState.NONE` is documented as "body is not in simulation, nor in collision world", so it removes the crate from collision regardless of layer masks, and it is the same pair vanilla uses when an inventory item is picked up and dropped again (`SCR_HeadgearInventoryItemComponent`). Recursive, so crate appearances whose colliders sit on child entities are covered. `m_aSavedGeomLayers` is replaced by `m_bSimulationSuspended` plus `m_eSavedSimulationState`; the boolean keeps the same re-entry guard role, preventing a second disable from capturing `NONE` and turning the restore into a no-op.

**Not a bug:** unloading a crate from a vehicle leaves it walk-through but still solid to bullets. That is stock behaviour for a dropped inventory item (the `ItemFireView` collision-layer preset resolves to `CharNoCollide` plus fire and view geometry), not something this mod introduces.

The `IBX_DiagDump` block and the `OnPostInit`/`EOnInit` hooks that drive it are marked TEMPORARY and stay in place only until the two fixes above are confirmed in play; the dump now also reports the physics simulation state so the collision fix can be verified the same way.

Validated so far: recompiled clean, `Module: Game; ... CRC32: 27f043ea` (from `61f130cd`), no errors. Destruction and carry-collision behaviour still need confirming in play.

## 2026-08-21 - Carry/drag feedback round: indestructible crates, dead holders, vehicle loading, players lifted

Four issues reported against the published carry/drag build. All four came out of `IBX_CrateCarryComponent.c`; three root causes between them.

**Crates became indestructible after being placed.** `StartCarryServer`/`StartDragServer` saved `physics.GetInteractionLayer()` and set the body mask to `0` for the duration of the hold, then `StopServer` wrote the saved value back. That mask is body-level, but a crate's colliders sit on different layers (collision hull, fire geometry, view geometry - see the `PropFireView`/`LargeDestructible` presets on these prefabs), so restoring one number across all of them is not a symmetric round-trip: the fire geometry can come back disabled while the character-collision hull still works. That is exactly the reported symptom - the crate can no longer be shot but still blocks and lifts characters. Nothing guarded a second disable either, so a carry-then-drag re-entry captured the already-zeroed mask and made the restore permanent.

Replaced with per-collider save and restore in a new `SetCrateCollisionEnabled(bool)`, iterating `Physics.GetNumGeoms()` and using `GetGeomInteractionLayer`/`SetGeomInteractionLayer`. The saved layers live in `m_aSavedGeomLayers`, which doubles as the re-entry guard: a disable while already disabled returns early instead of re-saving zeros. `m_iSavedInteractionLayer` is gone.

**Players could be raised into the air by a carried crate.** The interaction-layer change ran server-side only and was never replicated. Characters are simulated locally, so every other client kept full collision on its copy of the crate and got shoved by it; the carrier passed through walls because the server, which owns the crate transform, did have collision off. `ApplyHoldChangedLocally` now applies the collision state on *every* machine and keeps the `IBX_GMCarryClient` speed cap restricted to the holder's own machine. `BroadcastHoldChanged` was moved ahead of the first placement in both start paths, so collision is already off everywhere by the time the crate teleports into position, and it now tolerates a holder that cannot be resolved (`RplId.Invalid()`) instead of returning early - collision has to come back even when the holder disconnected.

**A crate held by a player who then died stayed frozen in place forever.** `StopServer` was only ever reachable from a player action. On death the character entity survives as a corpse, so `TickCarryServer`'s `!m_HolderCharacter` guard never tripped: the mode stayed `CARRY`, `s_HeldByCharacter` kept the dead character, `CanToggleCarry` returned false for everyone, and `IsHeldBy` hid `Drop Crate` from everyone including the respawned player, who is a different entity.

**Loading a carried crate into a vehicle left the carry effect on the player.** `IBX_CrateTransferMenu` inserts the crate into vehicle storage without telling the carry component, so the mode stayed `CARRY`, the 50 ms timer kept calling `SetWorldTransform` on a now-parented entity, and `OverrideMaxSpeed(0.35)` was never cleared. The holder also stayed registered in `s_HeldByCharacter` and could never carry anything again.

Both are fixed at the same choke point rather than in each caller: the two mode timers are the only places a live hold passes through, so a new `IsHoldStillValid()` runs there via `TickHoldCommon()` and calls `StopServer()` when the holder is gone, dead (`CharacterControllerComponent.IsDead()`), has boarded a vehicle (`ChimeraCharacter.IsInVehicle()`), or the crate has been parented into a storage (`IEntity.GetParent()`). `StopServer` now skips its snap-to-terrain and reposition when the crate is parented, which would otherwise yank a just-loaded crate straight back out of the vehicle.

Two smaller issues fixed in the same pass:

- **Join-in-progress.** A client connecting mid-hold missed the one-shot hold broadcast and kept full collision on its copy of the crate. `TickHoldCommon` re-broadcasts the current hold state every 40 ticks (2 s, `HOLD_REBROADCAST_TICKS`) while a crate is held. Both halves of `ApplyHoldChangedLocally` are idempotent, so a repeat changes nothing for receivers that already have it. Chosen over an ask-on-init RPC because RPCs issued outside an action callback are not reliable in this codebase (see the earlier `CallLater` finding).
- **`s_HeldByCharacter` leak.** The map is keyed by holder entity; when a holder disconnected, `m_HolderCharacter` was already null and `s_HeldByCharacter.Remove(null)` no-opped, leaving a dangling key behind forever. `ReleaseHolderRegistration()` falls back to finding the entry by value when the holder pointer is gone.

Validated: recompiled clean via `wb_reload`, confirmed by a fresh `Compiling Game scripts` line and `Module: Game; ... CRC32: f7c594d6` in `logs/logs_2026-08-21_16-28-50/script.log` with no `IBX_` errors. Runtime behaviour is not yet play-tested - the four scenarios (shoot a placed crate, carry past a second player, die while carrying, load a carried crate into a vehicle) need a human at the controls.

## 2026-08-21 - Workshop changelog entry for the carry/drag release

Documentation only. Added a `21 August 2026 - Physical Crate Carry and Drag` section at the top of `Workshop_Changelog.md`, covering everything user-facing since the last posted (16 August) entry: the carry/drag world actions, drop/raise/lower/rotate, the speed caps and range limits, the removal of the dedicated height/rotate/drop key bindings, and the visible fixes. The many intermediate debugging rounds from 18-21 August (BOM corruption, `[RplProp]` investigation, listen-server divergences, diagnostics cleanup, script file split) are deliberately omitted - they are internal churn or regressions that never reached a published build. Full detail stays in this file.

Added `Workshop_Description.md`: a full replacement description for the Workshop page - feature list, Game Master editor usage, carry/drag, the vehicle load/unload steps, how presets are used in the editor, and how to author new ones in `Configs/Inventory/CratePresets.conf` (`m_aPresets` entry, `m_sName`, `m_sItems` in `Count=Prefab;Count=Prefab` form, resource rebuild/reload, and the shortcut of exporting a filled crate to generate the string). Preset limits quoted from `IBX_CratePreset.Parse()`: 100 unique prefabs, 1-1,000 per entry, 10,000 total, duplicates and empty names rejected. Per the user's correction, the preset-authoring steps lead with the override: custom presets are always made by overriding `Configs/Inventory/CratePresets.conf` in your own addon at the same path and GUID (`{A1E6470C8DBF4260}`, the value `IBX_CratePresetConfig.CONFIG` loads) - editing the shipped copy is overwritten on mod update, and there is no in-editor preset creation yet.

The description was then condensed to **4,998 characters** to fit the Workshop's 5,000-character description limit; the cuts were prose only, no instruction or limit was dropped.

Note: during this session the user moved all documentation and marketing markdown out of the addon folder into a sibling `!TEMP/` directory (`AGENTS.md`, `CLAUDE.md`, `PROJECT_CONTEXT.md`, `changelog.md`, `Workshop_Changelog.md`, `Workshop_Description.md`, both Discord files). They now live at `NTF-Arma-Reforger/!TEMP/`, not in `InventoryBoxes/`, and the deletions show as such in `git status`.

Added `Discord_Update_CarryDrag.md`: the same update written as a single Discord post (1,816 characters, under the 2,000-character non-Nitro limit), using only markdown Discord actually renders (headers, bold, bullets, blockquote - no tables). The file is the post itself, ready to paste with no surrounding notes. Condensed from an earlier three-message split; the per-fix list was dropped to fit, since the individual carry/drag bugs never existed in a published build.

Also added, at the user's request: a `How to Load and Unload a Crate` section at the top of the file (middle-click the vehicle to open its inventory, hover the crate, hold `G` for 5 seconds; progress shows in the control-hints bar; only works via the middle-click "Open New Inventory" path), and a `Multiplayer and Performance` note stating that carry/drag should work in multiplayer but is performance-untested, with an expected impact from the continuous position updates.

## 2026-08-21 - Documented the Workbench script-reload focus requirement

Documentation only. User pointed out that Workbench only picks up changed script files while its window has OS focus - which is why the previous fix appeared validated before it had actually been rebuilt. `wb_reload` returns "Reload Complete / compilation triggered (`ExecuteAction=false`)" whether or not a compile happened, and also does not recompile while Workbench sits in play mode, so its return value proves nothing.

Added to `PROJECT_CONTEXT.md` "Key lessons", beside the existing `mod_validate` caveat: confirm every script change in `logs/logs_<newest>/script.log` via a fresh `Compiling Game scripts` line **and** a changed `Module: Game; … CRC32:` value. An unchanged CRC means the edit never loaded.

## 2026-08-21 - Fixed "Carry Crate" staying visible while carrying, on a listen-server host only

Reported: the `Carry Crate` action is still shown on the crate being carried in Workbench Play, but correctly hidden when playing as a client against the dedicated server.

Root cause: `CanToggleCarry()` has two branches, and only the remote-client one had been updated when Carry/Drag were made hidden-while-holding. The authority branch still ended with `return m_eMode == CARRY && IsUser(user)`, which deliberately returns *true* for the crate you are carrying. A listen-server host is the authority for the crate, so it took that branch and kept showing the action; a dedicated-server client is a proxy and took the client branch, which hides it. Same class of listen-server-only divergence as the `RpcDo_HoldChanged` broadcast-loopback bug: an authority-branch behavior that no dedicated-server test can ever exercise. `CanStartDrag()` was checked for the same divergence and does not have it - its authority branch already hides on every crate except the one actively being dragged.

That line could not simply be deleted: `CanToggleCarry()` was doing double duty as both the action's visibility check and the authorization inside `ToggleCarryServer()`, where it was the only thing permitting a toggle-*off*. Removing it alone would have made a carried crate impossible to release by re-toggling Carry.

Fixed by separating the two responsibilities: `CanToggleCarry()` now answers only "can this user *start* a carry" and returns false while holding anything in both branches, while `ToggleCarryServer()` authorizes the stop path itself, gated on `IsUser(user)` so a client with a stale UI can still legitimately toggle off.

Validated: `wb_reload` recompiled clean (Game module CRC32 `6267f448` → `fc49b1c7`, confirming the new code actually loaded - a reload issued while Workbench sits in play mode silently does not recompile). User confirmed live in Workbench Play.

## 2026-08-21 - Release cleanup: diagnostics stripped, dead code removed, script file split

Executed `CLEANUP_PLAN.md` against the release-ready carry/drag code. No behavior changes.

Removed all 11 `IBX_DIAG` diagnostic `Print()` calls (`BroadcastHoldChanged`, `ApplyHoldChangedLocally`, `RpcAsk_Drop`, `RequestDrop`, `RequestStartDrag`, `TickDragServer`, `IBX_DropCrateAction.CanBePerformedScript`) along with the diagnostic-only `OnModeReplicated()` callback and the `onRplName` argument on `m_eMode`'s `[RplProp]`. The file now contains no `Print()` calls at all.

Deleted dead code: `IBX_CrateCarryComponent.GetHolderCharacter()` (never called), the `m_iDragTickDiagCounter` field, and the `Close()` callqueue removal for `RefreshCurrentList` (never scheduled via `CallLater`). The unused `CARRY_SPEED_FRACTION`/`DRAG_SPEED_FRACTION` constants moved from `IBX_CrateCarryComponent` to `IBX_GMCarryClient`, where `GetSpeedFractionFor()` previously hardcoded the same 0.35/0.65 values. The `[RplProp]` attributes on `m_eMode`/`m_HolderRplId` were kept (the plan's optional, behavior-affecting removal was skipped - it needs a dedicated-server retest).

Condensed the carry/drag section's long historical/debugging comments to the non-obvious constraint each one encodes; the full history stays in `PROJECT_CONTEXT.md`. Comment density outside carry/drag was left untouched.

Split `Scripts/Game/IBX_GMInventoryEditor.c` (3,172 lines) into `Scripts/Game/InventoryBoxes/`: `IBX_CratePresets.c`, `IBX_GMInventoryEditorComponent.c`, `IBX_CrateCarryComponent.c`, `IBX_CrateCarryActions.c`, `IBX_EditorContextActions.c`, `IBX_EditorManagerRpc.c`, `IBX_GMInventoryEditorUI.c`, `IBX_CrateTransferMenu.c`. Classes moved verbatim, no reordering or signature changes. No prefab edits needed - prefabs reference component class names, not script paths.

Validated: `mod_validate` passes structure/gproj/prefabs/configs/references; `wb_reload` recompiles the Game module clean (5,713 files, zero errors); `wb_play` in `worlds/GM_Testwelt.ent` starts with no new `error.log` entries (the `Wrong GUID … Prefabs/InventoryBoxes` and `Unknown keyword 'Parent'` lines predate this work). In-game smoke test (open crate, GM `Edit Inventory`, carry/drag/drop, speed cap) confirmed working by the user.

## 2026-08-21 - Physical carry/drag confirmed fully working live; committed

User confirmed the Workbench-Play speed-cap fix (previous entry) works. All of today's carry/drag work - pickup/drag/drop/raise/lower/rotate, hiding Carry/Drag while holding, drag no longer self-cancelling, carry distance, and the listen-server broadcast-loopback fix - is now confirmed working end to end, on both the dedicated server and a listen-server/Workbench-Play host. Committed (`feat(inventory): physical crate carry/drag world actions` plus this session's fixes). `PROJECT_CONTEXT.md` "Physical Carry/Drag" section condensed to reflect the confirmed state; remaining `IBX_DIAG` diagnostic cleanup and minor RPC hardening tracked there under "Known limitations."

## 2026-08-21 - Fixed speed cap (and everything else driven by RpcDo_HoldChanged) not applying to a listen-server host's own local player

Reported: movement-speed cap works correctly connecting to the dedicated server, but not when playing via Workbench Play.

Root cause, confirmed via the existing `IBX_DIAG` diagnostics in the current session's log: `BroadcastHoldChanged`'s own "sending" print fires every time carry/drag starts or stops, but `RpcDo_HoldChanged`'s "received" print never once appears in the same log. A `[RplRpc(..., RplRcver.Broadcast)]` does not loop back to invoke its handler on the sending machine itself, only on genuinely separate connected peers. This is invisible on a real dedicated server (which has no local player of its own to miss the callback for) - it only shows up when the machine sending the broadcast is *also* somebody's local player, i.e. a listen-server host. Per the BIKI (`Multiplayer_Scripting`): "In single player, the local machine is considered a player-hosted server" - architecturally the same situation as Workbench Play, so **this almost certainly also means the speed cap (and, more importantly, everything this session's fixes added via the same `IBX_GMCarryClient` cache - Drop/Raise/Lower/Rotate visibility, hiding Carry/Drag while holding) would silently fail for the host's own character in a real player-hosted/singleplayer game too**, not just in Workbench.

Fixed the same way this file already handles the identical situation for crate position (`SyncCrateTransform` applies locally before broadcasting; `RpcDo_SetCrateTransform`'s handler skips re-applying on the sender via `IsAuthority()`): extracted the mode-matching logic into `ApplyHoldChangedLocally()`, called directly from `BroadcastHoldChanged` (covers the sending machine) in addition to being the RPC handler's body (covers every other, genuinely remote client). Both paths already no-op when the local player isn't the holder, so this can't double-apply.

Validated: `wb_reload` recompiled clean, no new `error.log` entries. Needs a live re-test in Workbench Play specifically (host carrying a crate) to confirm the speed cap now engages there too, plus a check that Drop/Raise/Lower/Rotate/hidden-Carry-Drag still all work correctly for a listen-server host, not just a dedicated-server client.

## 2026-08-21 - Carry distance increased

Carry/drag/drop/height/rotate all confirmed working live. Requested tweak: hold the crate a little further from the character. `CARRY_FORWARD_DISTANCE` 1.1 → 1.6. `wb_reload` clean. Not yet live-tested.

## 2026-08-21 - Fixed drag starting-and-stopping instantly (regression from hiding Drag Crate while holding)

Carry confirmed working live. Drag regressed: starts and stops instantly again - the same symptom as an earlier, already-fixed bug ("Drag died one tick after every attach").

Root cause: `CanStartDrag()`'s client branch was simplified to `!IsHoldingAnything()` in the previous change. `IBX_DragCrateAction` is a held action (`Duration -1`/`PerformPerFrame 1`), so the framework polls `CanBePerformedScript` (→ `CanStartDrag`) continuously for the entire hold, not just at the start. The instant `RpcDo_HoldChanged` sets `IBX_GMCarryClient`'s cache to `DRAG` after attaching, `IsHoldingAnything()` becomes true, `CanStartDrag` starts returning `false` for the very crate being dragged, and the framework cancels the hold one tick later. `CanToggleCarry` doesn't have this problem - it's a one-shot action, not continuously polled while "active."

Fixed by restoring the per-crate exception in `CanStartDrag`'s client branch: still returns `true` for the crate currently being dragged (checked via `IBX_GMCarryClient.IsHolding(GetOwner())` + mode `DRAG`), only falls back to `!IsHoldingAnything()` for every other crate (or this one in `CARRY` mode).

Validated: `wb_reload` recompiled clean, no new `error.log` entries. Not yet live-tested.

## 2026-08-21 - Carry/Drag toggle actions now hidden on every crate while already holding one

Confirmed live: pickup/drag/drop/height/rotate all work now. Follow-up request: hide "Carry Crate"/"Drag Crate" while already carrying/dragging (Drop now covers letting go, so the toggle is redundant clutter - and it's also meaningless on any other crate the player could otherwise walk up to while their hands are already full).

`CanToggleCarry()`/`CanStartDrag()`'s client branch simplified from "available unless I'm holding *this* crate in the wrong mode" to "hidden outright whenever I'm holding anything" - added `IBX_GMCarryClient.IsHoldingAnything()` (reads the existing `s_eMode` cache) and both methods now just return `!IsHoldingAnything()` on a client. Server/host logic unchanged (still allows the original toggle-to-drop as a fallback, and still blocks a second pickup).

Validated: `wb_reload` recompiled clean, no new `error.log` entries. Not yet live-tested.

## 2026-08-21 - Raise/Lower/Rotate now use a server-tick start/stop pattern; fixed Carry/Drag buttons staying visible while already held

Live-tested the previous fix: world actions now appear (visibility bug fixed), and Drop works, but Raise/Lower/Rotate did nothing, and "Carry Crate"/"Drag Crate" both stayed visible/available even while a crate was already being carried.

- **Raise/Lower/Rotate not working**: `IBX_CarryAdjustActionBase` called `RequestAdjust()` once per `PerformAction` frame (Duration -1/PerformPerFrame 1). Nothing in this file actually confirms `PerformAction` repeats reliably for a held action - `IBX_DragCrateAction` was specifically restructured earlier to *not* depend on that, driving movement from `OnActionStart`/`OnActionCanceled` starting/stopping a server-side `CallLater` tick instead, which is the only continuous-hold mechanism proven live in this project. Rebuilt Raise/Lower/Rotate the same way: `OnActionStart` sends `RequestAdjustStart(heightRate, yawRate)`, `OnActionCanceled` sends `RequestAdjustStop(...)`; the server runs a new `TickAdjustServer()` `CallLater` loop (50ms, matching `TickCarryServer`'s own cadence) that applies the rate via the existing `AdjustCarryServer()` clamp/apply logic every tick. Removed the now-dead `RequestAdjust`/`RpcAsk_AdjustCarry`/single-shot path entirely rather than leaving it unused.
- **Carry/Drag buttons showing while already held**: `CanToggleCarry()`/`CanStartDrag()` read `m_eMode`/`s_HeldByCharacter`, which (like `m_eMode`/`m_HolderRplId` before this) never reach a client's proxy copy - a holding client's own `CanToggleCarry`/`CanStartDrag` always evaluated the "nothing held yet" branch, so both buttons always looked available regardless of actual state. Fixed the same way as `IsHeldBy`/`GetMode`: on a client (`!IsAuthority()`), ask `IBX_GMCarryClient`'s already-reliable hold cache whether this client holds this crate, and if so, only report the toggle available when it's actually re-toggleable (`CanToggleCarry` true only in `CARRY`, `CanStartDrag` true only in `DRAG`). Not holding it locally still falls back to "available" (a client can't see other players' hold state at all), same resilience the original code already relied on.

Validated: `wb_reload` (scripts) recompiled clean, no new `error.log` entries for `IBX_GMInventoryEditor.c` (checked for both `SCRIPT (E)` errors and any `IBX`-tagged log line). Still needs a live re-test of these two fixes specifically.

## 2026-08-21 - Drop/height/rotate world actions now driven by the proven RpcDo_HoldChanged cache instead of [RplProp]

Applied the recommended next step from the 2026-08-19 pause: stopped relying on `[RplProp]` on `m_eMode`/`m_HolderRplId` for client-side display, since that path is confirmed to never fire for this component. `IBX_GMCarryClient` (client-only, static) already reliably tracks which crate the local player holds and in what mode via `RpcDo_HoldChanged` (used for the movement-speed override, confirmed working) - added `IBX_GMCarryClient.IsHolding(IEntity)` / `GetModeFor(IEntity)` static helpers that read that same cache, and changed `IBX_CrateCarryComponent.IsHeldBy()`/`GetMode()` to use them on a client (`!IsAuthority()`), while the server/host path keeps reading `m_eMode`/`m_HolderRplId` directly as before (always correct there).

This is what `IBX_DropCrateAction`/`IBX_CarryAdjustActionBase` (raise/lower/rotate) `CanBePerformedScript`/`CanBeShownScript` call through `IBX_CrateCarryComponent`, so this should be the fix for "world actions for drop/height/rotate never appear in-game." Not yet live-tested on the dedicated server - next step is a fresh play test to confirm the five world actions now show and function, then strip the `IBX_DIAG` prints throughout the file per this project's established pattern. The `[RplProp]`/`onRplName` attributes on `m_eMode`/`m_HolderRplId` and the `OnModeReplicated` diagnostic are left in place for now (harmless, and still useful if the underlying `[RplProp]` mystery is ever revisited) since they're no longer load-bearing for this fix.

Validated: `wb_reload` (scripts) recompiled clean, no new `error.log` entries for `IBX_GMInventoryEditor.c`.

## 2026-08-19 - Session paused: onRplName callback confirms [RplProp] never fires for this component

Retested with the `OnModeReplicated` `onRplName` callback added last entry - confirmed definitively (0 occurrences across a full hold, checked file-wide) that `[RplProp]` on `m_eMode` never fires client-side at all, despite the same client demonstrably tracking this exact entity correctly (crate position updates via the separate, already-proven `SyncCrateTransform` RPC broadcast arrive fine). Ruled out: enum serialization (already proven via `RpcDo_HoldChanged`'s own parameter), wrong/stale entity reference, and missing prefab-side registration (checked against the M777 reference's own working `[RplProp]` usage - no extra config there either).

Session paused here at the user's request. Full trail, what's ruled out, and the recommended next step (stop pursuing `[RplProp]`, drive the new actions' visibility from the already-reliable `RpcDo_HoldChanged` broadcast instead via a client-only cached flag) written up in `PROJECT_CONTEXT.md` under "Physical Carry/Drag" → "Dedicated-server debugging log" for continuation. `IBX_DIAG` diagnostics are still in place throughout and listed there too, pending final fix + cleanup.

## 2026-08-19 - [RplProp] fix didn't help either; added an onRplName callback to confirm whether it fires at all

Retested after marking `m_eMode`/`m_HolderRplId` `[RplProp]`. Still the same - `IBX_DropCrateAction.CanBePerformedScript` is confirmed called repeatedly every frame while looking at the crate (its own diagnostic fires, `comp` resolves correctly, and the logged crate position is visibly updating - proving this entity is fully relevant/tracked by this client, since our own `SyncCrateTransform` broadcast to it is working), but `IsHeldBy`'s own print never once appears after that - meaning it's hitting its early return (`m_eMode == NONE`) every time, i.e. `m_eMode` still isn't reading correctly client-side even with `[RplProp]` on it.

Since this same enum type is already proven to serialize correctly over the network (it's a parameter of `RpcDo_HoldChanged`/`BroadcastHoldChanged`, confirmed received correctly in every prior test), the enum type itself is unlikely to be the problem - something more specific to how `[RplProp]` behaves on this particular field isn't landing.

- Added `onRplName: "OnModeReplicated"` to `m_eMode`'s `[RplProp]` with a diagnostic-only callback, to get a direct yes/no on whether this replication path fires on the client at all, rather than continuing to infer it indirectly through `IsHeldBy`.
- Not yet fixed - needs this diagnostic's evidence before the next change.

## 2026-08-19 - Found it: m_eMode/m_HolderCharacter were never replicated to clients at all

The diagnostic confirmed it directly: `IsHeldBy`'s log line read `m_HolderCharacter=NULL m_eMode=0` on the holder's own client for the entire hold, 11+ seconds after actually picking up the crate (server-side state was correct the whole time). `m_eMode` and `m_HolderCharacter` were plain fields, never marked `[RplProp]` - a client's own proxy copy of `IBX_CrateCarryComponent` never received the server's mutations to them at all. `CanBeShownScript` for the 5 new world actions runs client-side (that's how the interact UI decides what to show), and it reads exactly these two fields via `IsHeldBy`/`GetMode` - so they always evaluated as "not carrying anything," and the new actions never appeared, regardless of the crate's actual server-side state.

The original toggle actions (Carry/Drag) never hit this because they're accidentally resilient to it: `CanToggleCarry` falls back to `!s_HeldByCharacter.Contains(user)` when it thinks `m_eMode == NONE` (which, client-side, it always incorrectly did) - so "Carry Crate" always looked available regardless of the real state, and clicking it always round-tripped to the server, which decides correctly using its own genuinely-correct fields either way. The new actions require the client to correctly know "I'm holding this" just to show at all, which exposed the gap for the first time.

- Marked `m_eMode` `[RplProp]` directly (a simple enum, replicates fine as-is).
- `m_HolderCharacter` (a raw `IEntity` reference) can't be sent directly - this file already consistently converts entities to `RplId` for anything that crosses the network (every `RpcAsk_*` handler resolves one via `Replication.FindItem`). Added a parallel `[RplProp] RplId m_HolderRplId`, set together with `m_HolderCharacter` through a new `SetHolder()` helper so they can never drift apart.
- `IsHeldBy`, `CanToggleCarry`, and `CanStartDrag` now all compare via a shared `IsUser()` helper (the local user's own `RplId` against `m_HolderRplId`) instead of `m_HolderCharacter == user` - the latter would have gone from "always wrong but harmlessly so" (client-side `m_eMode` was ALSO always wrong before, so the two wrongs cancelled out) to "always wrong and now visibly broken" the moment `m_eMode` started replicating correctly on its own.
- Every server-only method (`TickCarryServer`, `ApplyCarryOffset`, `TickDragServer`, `StopServer`) still reads the raw `m_HolderCharacter` reference directly, unchanged - those only ever run authoritatively on the server, where it was never the problem.
- Compile not re-verified live again this round (same stuck Workbench reload bridge); relying on the dedicated-server test's own independent fresh compile, as it has for every fix this session regardless of Workbench's own reload state.

## 2026-08-19 - New world actions don't show at all; added diagnostics

Confirmed via the dedicated-server logs (both server and client) that the 5 new actions loaded with zero errors - the class registration itself is fine. But live, the user sees none of them while carrying - not even a scrollable/cycling list, just the exact same 3 entries as before (Open Storage/Carry Crate/Drag Crate, minus Drag correctly hidden while carrying). Ruled out a UI display-count cap by asking directly - confirmed it's "same as before, no new entries" rather than "list present but can't scroll to them".

- Reworked `IsHeldBy()` to use an explicit `if (!user) return false;` null check instead of `return user && ...`, matching this file's own established style everywhere else - not confirmed as the actual bug, but a legitimate risk given every other null check in this file is explicit and this was the only place using an object reference directly in a boolean expression.
- Added `IBX_DIAG` prints to `IsHeldBy()` and `IBX_DropCrateAction.CanBePerformedScript()` to see directly whether these are even being called for the new actions, and with what values, rather than guessing further.
- Not yet fixed - needs the diagnostic evidence from a live retest (carry the crate, look at it) first.

## 2026-08-19 - Replaced the broken dedicated-key drop/height/rotate controls with world actions

The dummy-parameter fix from the previous entry didn't help - a focused retest with `RequestDrop`/`RpcAsk_Drop` diagnostics on both ends proved the client-side `Rpc(RpcAsk_Drop, ...)` call still never arrives server-side, while every other RPC in the file (all sent from inside an actual `ScriptedUserAction` callback) consistently arrives in under 100ms. Re-checked the M777 reference mod specifically for this: it never sends an RPC from a raw input poll either - even its continuous push/pull/turn is entirely driven by `OnActionStart`/`OnActionCanceled`/`PerformAction`. The raw-keybind approach (`IBX_GMCarryClient.Tick()` polling `InputManager` every 50ms via `CallLater` and calling `Rpc()` directly) was the only place in this file departing from that pattern, and empirically the only one that never worked.

- Removed the dedicated `IBX_CarryHeightUp`/`Down`/`RotateModifier`/`Drop` input actions and their keybinds entirely (`chimeraInputCommon.conf`, `keyBindingMenu.conf`).
- Added five new world actions using the same `Duration -1`/`PerformPerFrame 1` mechanism `IBX_DragCrateAction` already proved reliable: `IBX_DropCrateAction` (one-shot, either mode), `IBX_RaiseCrateAction`/`IBX_LowerCrateAction`/`IBX_RotateCrateLeftAction`/`IBX_RotateCrateRightAction` (held, carry only). All route through the already-authority-gated `RequestDrop()`/`RequestAdjust()`. The original justification for avoiding world actions ("crate held in front of the camera can't be re-targeted by a look-at action") turned out to be wrong - re-clicking Carry Crate to drop had been working as a look-at action the whole time.
- `RequestAdjust()` didn't have the `IsAuthority()` gate every other `RequestX` method has (always sent an RPC, even when already running as the server); fixed to match, since `PerformAction` also fires directly on the server.
- `IBX_GMCarryClient` trimmed down to just the local speed-override bookkeeping (still needed, still broadcast-driven, unaffected by this change).
- Bulk-added the 5 new actions to all 42 crate `.et` files (verified each had exactly one match for the insertion point before writing, learned from the earlier corruption incident).
- Compile could not be re-verified live this round - Workbench's own reload bridge got stuck again (no new "Compiling Game scripts" line despite repeated `wb_reload`/`wb_stop` calls, and its own internal preview world logged "Unknown class" for all 5 new actions using its stale pre-edit class table). This does not affect the dedicated-server test flow, which always compiles fresh from disk at its own process startup regardless of Workbench's internal cache state - confirmed by every prior fix in this session reaching the test correctly despite the same reload flakiness. Reviewed the new code manually; not yet confirmed by a live test.

## 2026-08-19 - Drag and carry movement confirmed working live; drop-key RPC still not landing server-side

User confirmed both drag and carry now move correctly on the dedicated server (the `SyncCrateTransform` broadcast fix worked). Remaining issue: pressing the dedicated `IBX_CarryDrop` key does not drop the crate - only re-clicking the "Carry Crate" world action does.

- Log timeline shows `IBX_GMCarryClient.Tick()` DID read `dropDown=1` from the key press and stopped its own polling loop right after (no more `Tick` diagnostic lines follow), consistent with `RequestDrop()` running and calling `Reset()` - but the actual `BroadcastHoldChanged sending mode=0` (the real stop) didn't happen until ~11 seconds later, matching the timestamp of the user re-clicking Carry instead, not the key press. This means `RequestDrop()`'s `Rpc(RpcAsk_Drop)` call - made from the static `IBX_GMCarryClient` helper via a resolved component reference, unlike every other working `RequestX` call in this file, which are all called from an actual `ScriptedUserAction` instance - isn't reaching the server, while the exact same `RequestDrop()` method works fine when called from `IBX_DragCrateAction.OnActionCanceled`.
- Added `IBX_DIAG` prints to `RequestDrop()` (call + `IsAuthority()` result) and `RpcAsk_Drop` (receipt) to confirm exactly where this call is failing.
- Not yet fixed - needs the diagnostic evidence from a focused retest (pick up, then immediately try the drop key alone) before making a further change.

## 2026-08-19 - Two more dedicated-server bugs found via diagnostics: drag self-cancelled every frame, carry never replicated to the holder

Retested with more targeted diagnostics (server-side attach state, client-side Tick() state/inputs). Found two separate, unrelated bugs:

- **Drag died one tick after attaching.** `IBX_DragCrateAction.PerformAction` unconditionally called `RequestDrop()` "as a safety net" mirroring `OnActionCanceled`. With `Duration -1`/`PerformPerFrame 1`, the framework calls `PerformAction` every single frame the key is held, not once on release - so every attach was undone within one server tick (confirmed live: diagnostic logged `TickDragServer ... attached=1`, then a stop broadcast 17ms later, every time). Removed the `PerformAction` override entirely; `OnActionCanceled` (release) and `CanBePerformedScript`'s own continuous re-check (out of range) already cover both real stop conditions.
- **Carry never visibly moved for the holder.** `StartCarryServer` used `IEntity.AddChild` to reparent the crate onto the character, same as drag's old attach. Diagnostic proved the server-side reparent does NOT replicate the parent/child relationship to clients at all: the holder's own client read the crate's parent as `NULL` for the entire hold (6+ consecutive ticks), so it never moved on their screen even though the server's own state was correct the whole time. Replaced reparenting (both carry and drag) with periodic (50ms) explicit `SetWorldTransform` calls computed from the holder's live position/orientation each tick - the same mechanism any other server-moved networked prop already relies on to replicate, instead of implicit hierarchy-attach replication (which doesn't happen for this entity). Carry keeps its own independent visual yaw offset (rotates in place) while its position offset always follows the character's actual facing. Drag's attach now stores a captured forward-distance/height offset and reapplies it from the character's current transform every tick, rotation matching the character live (previously an identity-local rotation under the parent).
- The `crateParent=NULL` observation also explains why the previous entry's authority-gate fix for carry looked correct in isolation (single clean toggle, no double-dispatch) but still "did nothing" visibly.
- Validated with `wb_reload` (clean compile); not yet re-tested live.
- `IBX_DIAG` diagnostics remain in place (now printing crate world origin instead of the now-meaningless parent) pending that retest.

## 2026-08-19 - Found the real dedicated-server bug: carry's PerformAction ran on the client too

Inspected the dedicated-server log files (`My Games/ArmaReforger/logs` for the server, `My Games/PeerPlugin1/logs` and `PeerPlugin22/logs` for the two client processes - not Workbench's own `logs` folder) from the user's last test, using the `IBX_DIAG` diagnostics added in the previous entry.

- The logs proved the fresh-`IsAuthority()` fix from the previous entry does work: the server correctly detects itself as authoritative, the client correctly detects it isn't, and the broadcast (`RpcDo_HoldChanged`) is sent, received, and correctly matched to the local player multiple times for both carry and drag.
- But the same client's log also showed it independently sending its own `BroadcastHoldChanged` for the carry toggle, at nearly the same instant as the server did. That call only exists inside `ToggleCarryServer`, which `IBX_CarryCrateAction.PerformAction` called directly, with no authority guard - unlike drag's `OnActionStart`/`OnActionCanceled`, which already used the `IsAuthority()`+RPC pattern. `PerformAction` was assumed (based only on listen-server testing, where client and server are the same process) to run exclusively on the server; the dedicated-server log disproves that. The client was reparenting and toggling mode on its own non-authoritative proxy copy of the crate entity at the same time the server was doing the same thing legitimately - two competing mutations of an entity the client doesn't own, the likely cause of the reported regression ("now i cant drag anymore") since it corrupts the crate's hierarchy/replication state for whatever is tried on it next, not just carry itself.
- Fixed by adding `RequestToggleCarry`/`RpcAsk_ToggleCarry`, mirroring `RequestStartDrag`/`RpcAsk_StartDrag` exactly, and pointing `IBX_CarryCrateAction.PerformAction` at it instead of calling `ToggleCarryServer` directly.
- Validated with `wb_reload` (clean compile, no script errors); not yet re-tested live on the dedicated server.
- `IBX_DIAG` diagnostics are still in place pending that retest; remove once confirmed fixed.

## 2026-08-19 - Dedicated-server fix didn't work; hardened IsAuthority() and added diagnostics

- User retested on the dedicated server: still no height/rotate/drop, and drag now fully broken (regression from the previous fix). Likely cause: `IsAuthority()` cached its `RplComponent` reference in `OnPostInit`, which may not be reliably resolvable that early on a replicated proxy entity; a null cache defaulted to `true` ("is authority"), which is exactly backwards for a client - it would mutate its own non-authoritative copy directly instead of asking the server, and the next replication update silently overwrites that.
- Changed `IsAuthority()` to resolve `RplComponent` fresh via `FindComponent()` on every call instead of a cached field, removing the init-order dependency entirely.
- Added temporary `IBX_DIAG` diagnostics to `BroadcastHoldChanged`/`RpcDo_HoldChanged` (send/receive of the hold-notification broadcast) and `RequestStartDrag` (its `IsAuthority()` result), since guessing blind through another round isn't productive - next test's log will show directly whether the broadcast is sent, received, and matched correctly. Remove once confirmed working.
- Validated with `mod_validate` and `wb_reload`; not yet re-tested live.

## 2026-08-19 - Fixed dedicated-server multiplayer: client effects and drag state were never networked

Tested on a real dedicated server (separate server + client processes) for the first time. Carrying moved correctly for everyone (entity-hierarchy replication, server-authoritative), but height/rotate/drop did nothing and drag wasn't synced at all — exactly the gap already flagged as a known limitation from listen-server-only testing.

- The `SCR_PlayerController.GetLocalControlledEntity() == user` check used to start `IBX_GMCarryClient` (the client-only speed cap + height/rotate/drop poll) only ever matched on a listen server, where server and client are the same process. On a real dedicated server the server process has no local player at all, so it never fired anywhere - no client ever heard "you're now carrying/dragging", so none of those keys ever worked, even though the crate itself moved correctly for everyone.
- Fixed with a proper broadcast: `IBX_CrateCarryComponent` now sends an `[RplRpc(..., RplRcver.Broadcast)]` (`RpcDo_HoldChanged`) whenever a hold starts or stops, carrying the holder's own `RplId`; every client receives it and starts/stops `IBX_GMCarryClient` only if the holder resolves to their own locally-controlled character.
- `IBX_DragCrateAction.OnActionStart`/`OnActionCanceled` were calling `StartDragServer`/`StopServer` directly, assuming (based on a listen-server test) that these hooks run authoritatively. They don't, reliably: added the reference mod's own defensive pattern (`IsAuthority()` check on the crate's `RplComponent.IsProxy()` - direct call if this is really the server, `Rpc()` to the server otherwise) via new `RequestStartDrag()`/`RequestDrop()` entry points, matching how `RequestAdjust()` already worked.
- Validated with `mod_validate`, `wb_reload`, and a live `wb_play` log check (no compile/prefab errors) against the actual dedicated-server + client launch command this project uses for testing.

## 2026-08-18 - Consolidated carry/drag documentation; confirmed working end-to-end

- Carry/drag confirmed working end-to-end by the user (pickup, put-down, height/rotate, drag). Replaced `PROJECT_CONTEXT.md`'s thirteen-round, turn-by-turn debugging log for this feature with a single "Physical Carry/Drag" section: final architecture, a condensed list of the reusable lessons learned (BOM/encoding gotchas, `mod_validate`'s blind spots, `OnConfirmed` not firing, `Sort Priority`, `AddChild` not preserving world position, etc.), and the remaining known limitations. Updated the "Not implemented"/"Recommended Next Work"/"Known Constraints" sections to point at it instead of duplicating or going stale.

## 2026-08-18 - Display order needs "Sort Priority", not config file order

- Reordering the `additionalActions` block in the `.et` file had no effect on the in-game display order (still alphabetical: Carry, Drag, Open). The reference mod (M777) config-file properties, not just its script, showed the actual mechanism: each of its actions sets `"Sort Priority" N`. Added `"Sort Priority" 1/2/3` to `SCR_OpenStorageAction`/`IBX_CarryCrateAction`/`IBX_DragCrateAction` in all 42 crate `.et` files. Verified with `mod_validate` plus a live `wb_play` log check (no prefab load errors) before reporting done, per the lesson from the reorder corruption.

## 2026-08-18 - Reordered crate actions (Open Storage first); fixed a corruption from doing it

- Reordered `additionalActions` in all 42 crate `.et` files so `SCR_OpenStorageAction` (Open) comes first, then `IBX_CarryCrateAction`, then `IBX_DragCrateAction`.
- The first reorder attempt corrupted all 42 files: the script's brace-matching found the `{` *inside* the quoted resource GUID (e.g. `"{5476DE57AA50402F}"`) instead of the action block's real opening brace, splicing the carry/drag block into the middle of the GUID string. Caught immediately via live log inspection (`wb_play`, checked `error.log` for prefab load failures) before reporting success, rather than trusting `mod_validate`, which passed on the corrupted files without complaint. Repaired in two passes: reassembled the split GUID/block structure, then collapsed a stray line break the first repair left inside the quoted GUID (turned out to be a mixed LF-only boundary in an otherwise CRLF file - `\r\n`-only regexes silently didn't match it). Verified clean via a full `wb_play` cycle with no prefab-load errors this time, not just a passing `mod_validate`.

## 2026-08-18 - Fixed the drag button UI staying "active" after a range force-stop

- The previous range cap stopped the crate but only from the component's own polling timer, out of band from the actual held action - the client's button/progress UI had no idea the hold had ended. Moved the range check into `IBX_DragCrateAction.CanBePerformedScript()`, which the action framework re-checks continuously while a hold is active; going out of range now fails that check and lets the framework cancel the hold itself through the normal `OnActionCanceled` path, which is what actually updates the client UI. The component's own range check in `TickDragServer` stays as a fallback (via the new shared `IsWithinDragRange()`) in case that continuous re-check assumption doesn't hold in practice.

## 2026-08-18 - Capped how far you can wander from the crate while holding the drag key

- The world-action system only checks `VisibilityRange` when a hold starts, not continuously while it's held - sidestepping away from the crate (staying detached, since sideways isn't backward movement) let the hold continue with the character wandering arbitrarily far, key still held down. Added a `DRAG_MAX_RANGE` (2.5m) check at the top of `TickDragServer` that force-ends the whole hold via `StopServer()` once exceeded, same effect as releasing the key. Note: since the client doesn't know the server force-ended it, the held key's own progress UI keeps showing until actually released and re-pressed - re-engaging requires walking back in range and pressing the drag key again, not just walking back while still holding it.

## 2026-08-18 - Lowered the drag action's interact range

- `IBX_DragCrateAction`'s `VisibilityRange` lowered from 6m to 2m in all 42 crate `.et` files - the drag hold could be started/held from much further away than intended. Carry's own `VisibilityRange` (still 6) is untouched, only drag was reported as too permissive.

## 2026-08-18 - Re-center the crate in front of the character on (re)attach

- `AttachDrag` no longer preserves the crate's exact captured world position on attach. Sidestepping a few meters while detached (crate stays put, character moves) left it, once reattached, off to one side of the character instead of directly ahead - visually as if dragging from the wrong spot. Now recenters the crate onto the character's forward axis (zero sideways offset) while keeping its current forward distance and height, so it always resumes directly in front regardless of any lateral drift while it was sitting detached.

## 2026-08-18 - Widened dead zone further (1.2m -> 1.5m); widened backward-direction cone

- Dead zone radius bumped again to 1.5m.
- `DRAG_BACKWARD_TOLERANCE` widened from 55° to 70°, so a diagonal backward-left or backward-right step (not just dead-straight backward) still counts. It was already symmetric (`Math.AbsFloat`), so both diagonals were already treated equally - the actual change here is just a wider cone, not fixing an asymmetry.

## 2026-08-18 - Widened the front/behind dead zone further (0.6m -> 1.2m radius)

- Doubled the too-close-to-judge-direction radius again on user feedback ("getting better, make it higher").

## 2026-08-18 - Widened the front/behind dead zone to cover standing inside the crate

- With collision off for the whole drag hold, the character can walk right into the crate's footprint (not just up to it); passing through its center flips which side reads as "in front" from one tick to the next, resuming movement while standing in the middle of it. Widened the too-close-to-judge-direction radius from 0.3m to 0.6m so it covers standing anywhere in/on a typical crate, not just its exact center point.

## 2026-08-18 - Fixed being able to step up onto the crate while dragging

- Collision was only disabled while the crate was actually attached (following); detached (e.g. not currently in front, or briefly paused) it had normal collision again, and Reforger characters auto-step onto small obstacles - letting the character climb onto the crate mid-drag, which also put them inside the "too close to judge direction" dead zone and stalled dragging. Collision is now disabled for the entire drag hold (`OnActionStart` to `OnActionCanceled`), independent of the attach/detach toggling, matching how carry already disables it for its whole duration; restored unconditionally in `StopServer` either way.

## 2026-08-18 - Fixed drag resuming just from standing near/on the crate

- `IsInFrontOfUser`'s too-close-to-tell-direction fallback defaulted to `true`, so getting close enough to the crate (standing on it, touching it) made the front/behind check pass regardless of actual facing, resuming drag. Widened the dead zone (1cm radius was too tight to actually catch "standing on it") and changed the fallback to `false` - being too close to reliably judge direction now means "don't drag", not "assume it's in front".

## 2026-08-18 - Drag: gated on crate being in front, and continuous terrain re-snap

- Added an "is the crate in front of the character" check (dot product of the horizontal direction to the crate against the character's forward vector), required alongside walking-backward for the crate to attach or stay attached. Dragging while the crate is behind you no longer moves it; it detaches (freezes in place) if it ends up behind you (e.g. after turning) while still attached.
- While attached and dragging, the crate's height is now corrected every tick to the terrain directly under its own current position (`SCR_TerrainHelper.GetTerrainY`), not just inherited from the character's own ground-following. A rigid, fixed local offset only tracks the character's own height, so crossing a slope the character and the trailing crate aren't equally on drifted the crate underground or into the air; this fixes that while dragging (not just on drop).
- Validated with `mod_validate` (pass) and `wb_reload`; not yet re-tested live.

## 2026-08-18 - Fixed AddChild not actually preserving world position on attach

- `IEntity.AddChild(child, -1)`'s default flags do not preserve world position the way assumed — removing the hardcoded offset last round exposed this as the crate landing at some unrelated, wildly wrong transform (reported as spawning in the ocean, in the air). Carry never showed this because it immediately overwrites the transform with its own fixed offset right after attaching, masking the same underlying behavior.
- Fixed by capturing the crate's world transform before calling `AddChild`, then explicitly restoring it with `SetWorldTransform` right after. This no longer depends on whatever `AddChild` does internally to the transform.

## 2026-08-18 - Fixed the actual teleport cause: a hardcoded attach offset, not timing

- The earlier "attach immediately on pickup" fix didn't stop the jump because the real cause wasn't timing at all - `AttachDrag` was overwriting the crate's local transform with a hardcoded "1.4m directly behind the character" offset on every attach. Since the crate is normally in front of the character (that's where you're looking to grab it), snapping it to a fixed spot behind is an inherent ~180° relocation regardless of when it happens.
- Fixed by no longer setting a fixed offset at all: `AddChild`'s default `AUTO_TRANSFORM` behavior preserves whatever world position/rotation the crate already has at the moment it's parented, so it now just keeps following from wherever it was actually sitting. Removed the now-unused `DRAG_FOLLOW_DISTANCE` constant.

## 2026-08-18 - Fixed the dragged crate teleporting in when backward movement started

- Attach now happens immediately in `StartDragServer` (same as carry attaches immediately on pickup), not on the first backward step in `TickDragServer`. It was only attaching once backward movement was first detected, which could be well after the drag hold started and several meters from wherever the crate actually was, so it visibly snapped/"spawned" in behind the character. `TickDragServer` now only handles re-attaching after a later detach (stopped and resumed walking backward), not the initial attach.

## 2026-08-18 - Fixed the dragged crate lagging behind at higher movement speed

- Changed drag from a per-tick "step toward a leash distance, capped at a fixed speed" follow to the same reparenting mechanism carry uses: while walking backward, the crate is a child of the character at a constant local offset, so it moves in exact lockstep every frame regardless of how fast the character is moving. The previous approach had its own speed cap (`DRAG_SPEED_MPS`) independent of the character's actual speed, so outrunning that cap made the crate fall behind. Detaches (freezes in place, snapped to terrain) the moment backward movement stops; re-attaches at the same fixed offset if backward movement resumes.
- `StopServer` now detaches based on "is the crate actually parented to its holder" rather than "is the mode CARRY", since drag can now also leave it attached.
- Validated with `mod_validate` (pass) and `wb_reload`; not yet re-tested live.

## 2026-08-18 - Removed mouse wheel control; found why drag stopped working entirely

- Removed `IBX_CarryWheel` entirely (action definition, context registration, and script read) at user's request. Height/rotate are keys-only now (Numpad 8/2, hold Numpad 5 to swap to rotate).
- Found why drag did nothing after switching it to `OnActionStart`/`OnActionCanceled`: the reference mod's `.et` config sets `Duration -1` and `PerformPerFrame 1` on each of its hold-action instances — properties `IBX_DragCrateAction` never had. Without them the action likely defaults to instant/click semantics, so `OnActionStart` (which only fires once a hold "progress bar" actually starts) never engages at all. Added `Duration -1` and `PerformPerFrame 1` to `IBX_DragCrateAction` in all 42 crate `.et` files, matching the reference mod's own action config exactly.
- Validated with `mod_validate` (pass) and `wb_reload`; not yet re-tested live.

## 2026-08-18 - Fixed disappearing crate, drag speed cap, wheel jump-to-max, and drop needing spam

Checked a working shipped mod's own crate-move implementation (M777's `TOFTS_StorageBoxMoveComponent`/`TOFTS_StorageBoxMoveActions`, extracted for reference only, not copied - different license) after several of our own fixes still didn't resolve remaining bugs. Concrete, applicable differences found:

- **Crate disappearing (view-angle dependent, and when dropped in the air)**: the reference mod calls `owner.Update()` after every manual transform change, and resets physics velocity via `Physics.SetVelocity(vector.Zero)`. We were calling `SetOrigin`/`SetLocalTransform` directly with neither — a moved entity's cached bounds/culling data can go stale without an explicit `Update()`, which reads as exactly this symptom (visible from some angles, not others). Added an `Update()`+velocity-reset call (`ResyncPhysics`) after every carry-offset, drag-tick, and drop transform change.
- **Terrain snapping**: switched from manually reading `BaseWorld.GetSurfaceY()` and overwriting just the Y component to `SCR_TerrainHelper.SnapToTerrain(transform[4], world)` on the full world-transform matrix, matching the reference mod exactly — more robust than a bare Y overwrite.
- **Drag speed cap not applying**: found by comparing to the reference mod's `OnActionStart`/`OnActionCanceled` hooks, which are what it uses to start/stop its own continuous crate movement — confirmed reliable, unlike `PerformAction`+`OnConfirmed` for a single tap. Changed `IBX_DragCrateAction` to start/stop (and trigger the client speed cap) from `OnActionStart`/`OnActionCanceled` instead of a single-tap `PerformAction`, matching the reference mod's structure; `CanBroadcastScript() { return false; }` also copied for the same reason it uses it (client-predicted, not broadcast).
- **Mouse wheel inconsistently tracked / one scroll maxes out height**: the raw `IBX_CarryWheel` value per notch turned out to be far larger than expected, so a single scroll click multiplied straight to the height-offset clamp. Clamped the wheel's per-tick contribution to a small fixed step, so multiple notches now take multiple ticks instead of one jump.
- **Drop needing several presses to register**: was reading `InputManager.GetActionTriggered()` (a "click" filter's one-native-frame pulse) on a 50 ms poll, which can miss the pulse entirely — matches "have to spam the button". Changed the action's filter from `click`/`InputFilterClick` to `pressed`/`InputFilterPressed` (stays high for the whole keypress) and added manual rising-edge detection in script, so a single press can't be missed by the poll interval.

Not yet re-tested live. Height/rotate readability itself (context registration, presets) was already fixed in the previous round; this round targets the remaining reported symptoms on top of that.

## 2026-08-18 - Found the actual root cause: OnConfirmed never fires for these actions

- Root cause of everything client-side never working (speed cap, height/rotate/drop reads) since the very first version: `ScriptedUserAction.OnConfirmed()` never fires for `IBX_CarryCrateAction`/`IBX_DragCrateAction`. Proved this with a diagnostic `Print()` in `OnHoldStarted` (called only from `OnConfirmed`) plus a per-second dump of the raw action values in the tick loop: zero log output across an entire session of successfully picking up and carrying a crate. Everything that ran through server-side `PerformAction` (reparenting, collision disable, distance fix) worked the whole time; everything gated behind the client-only `OnConfirmed` callback never ran at all.
- Fixed by dropping the dependency on `OnConfirmed` entirely: `IBX_CrateCarryComponent.StartCarryServer`/`StartDragServer`/`StopServer` now call `IBX_GMCarryClient.OnHoldStarted`/`OnHoldEnded` directly, guarded by `SCR_PlayerController.GetLocalControlledEntity() == user`. This works because Workbench "Play" testing is a listen server (host and client are the same process), so a server-side check for "is this the local player" reaches the right place. Removed the dead `OnConfirmed` overrides from both actions.
- Known limitation this introduces: on a real dedicated server (client and server in separate processes), this direct call would only run on the server process and never reach a remote client. Flagged in `PROJECT_CONTEXT.md` — if remote-client multiplayer testing shows the same "keys do nothing" symptom, this needs to become a targeted RPC to the holding player instead of a direct call.
- Left the periodic diagnostic log in `Tick()` in for one more live check; remove once height/rotate/drop are confirmed actually working end to end.

## 2026-08-18 - Switched to registering carry keys into vanilla's CharacterGeneralContext

- Deleted the custom `IBX_CarryContext` and the `InputManager.ActivateContext()` call that had to reassert it every tick. Registered `IBX_CarryHeightUp/Down/RotateModifier/Drop/Wheel` directly into vanilla's own `CharacterGeneralContext` instead (`ActionRefs +{ }`), matching this project family's own `RAMI_AdvancedMedicalInterface` addon, which registers its medical-menu toggle key (`RAMI_ToggleMedicalMenu`) the same way and reads it with a plain `GetActionValue()` call, no activation bookkeeping at all. `CharacterGeneralContext` is active by default during normal on-foot play, so this removes an entire (evidently unreliable) layer of my own custom context-activation code.

## 2026-08-18 - Found the real "greyed out" cause against extracted vanilla source; added real mousewheel support

- User extracted the game's data to compare against (`F:\Reforger extracted`). Checked every vanilla `SCR_KeyBindingEntry` in `keyBindingMenu.conf`: **every single one sets `m_sPreset`**, matching a `FilterPreset` + `Filter` class actually present on the corresponding action (e.g. Reload uses `FilterPreset "click"` + `Filter InputFilterClick {}`, matching keybind `m_sPreset "click"`). My 4 new actions had neither a filter nor a preset — almost certainly why their rows rendered greyed/disabled. Fixed: `IBX_CarryHeightUp/Down` and `IBX_CarryRotateModifier` now use `FilterPreset "pressed"` + `Filter InputFilterPressed {}` (continuous held-key read, same pattern vanilla's `PerformAction`/interact key uses), `IBX_CarryDrop` uses `FilterPreset "click"` + `Filter InputFilterClick {}` (one-shot trigger, same as Reload); `keyBindingMenu.conf` updated to match.
- Confirmed the real vanilla mouse wheel token from `Action MouseWheel` in the extracted `chimeraInputCommon.conf`: `Type AnalogRelative`, `Input "mouse:wheel"`, `Filter InputFilterValue { Multiplier 1 }`. Added `IBX_CarryWheel` using that exact pattern and wired it into height/rotate alongside the Numpad keys, so mouse wheel now actually works for height/rotate as originally requested (previously undeliverable without a confirmed token).
- Confirmed against `SCR_ItemPlacementComponent.EOnFrame` (a real vanilla per-frame context activation, structurally identical to my per-tick `ActivateContext` loop) that calling `ActivateContext(name)` with no duration every frame/tick — not a duration value that needs guessing — is the correct vanilla pattern; simplified to match.
- Also confirmed `R` is bound by *many* vanilla actions (Reload, ChangeAmmo, UseItem, Inspect, vehicle engine start/stop...) — validates the earlier move off it as a real conflict risk, not just caution.

## 2026-08-18 - Confirmed the new actions register correctly; changed default keys off likely conflicts

- Added a one-time diagnostic dump (`InputManager.GetActionCount()`/`GetActionName()`/`IsContextActive()`) at crate init to settle whether the new input actions were actually registering. Confirmed live in Workbench: `IBX_CarryHeightUp`, `IBX_CarryHeightDown`, `IBX_CarryRotateModifier`, and `IBX_CarryDrop` all register correctly, and `IBX_CarryContext` reports active. So the action/context config itself is not broken — removed the diagnostic afterward.
- Changed the default keys off Page Up/Down, `R`, and Backspace onto Numpad 8/2/5/0. `R` in particular is reload in most military shooters and was a plausible conflict explanation for keys appearing unusable; picked keys with the lowest realistic chance of colliding with an existing vanilla bind instead of continuing to guess at the exact cause of the "greyed out" appearance.
- Still open: why the Controls-menu rows show greyed out even though the underlying actions are confirmed valid. That's a keybinding-menu UI presentation detail I can't diagnose further without seeing the screen — see `PROJECT_CONTEXT.md`.

## 2026-08-18 - Fixed drag stacking timers; fixed a likely-bogus config merge on the new input context

- Fixed the drag crate disappearing: `StartDragServer` could schedule a second 50 ms movement timer on top of an already-running one (no re-trigger guard, no `Remove` before `CallLater`), so repeated/duplicate action performs stacked timers that each moved the crate the same tick, compounding into a runaway teleport. Added a same-holder-already-dragging guard and a `Remove` before every `CallLater`.
- Changed the new `IBX_CarryContext` to declare `ActionRefs { ... }` instead of `ActionRefs +{ ... }`. The `+{ }` merge syntax is for extending an *existing* vanilla array (that's why the existing `InventoryMenuContext` override correctly uses it); `IBX_CarryContext` is a brand new context with nothing to merge into, so `+{ }` may have silently produced an empty ref list — which would explain the height/rotate/drop keys reading as inactive even with the context correctly activated.
- Still unconfirmed: whether the height/rotate/drop keys and the drop button's "locked" appearance in the Controls menu are fully explained by this, or whether Workbench needs a full close-and-reopen (not just Stop/Play) to pick up the corrected `.conf`. Validated with `mod_validate` (pass) and `wb_reload` (both); needs a live retest.

## 2026-08-18 - Reworked drag off the user-action loop mechanism; unified carry/drag client state

- Replaced `IBX_DragCrateAction`'s reliance on `IsActionLooping()`/`PerformContinuousAction()` (never confirmed to actually engage the native hold/loop machinery, and empirically inert) with the same pattern already proven for pickup: a single-tap `PerformAction` that starts drag server-side, then a plain repeating `GetGame().GetCallqueue()` timer (50 ms) drives the follow movement — no dependency on action-system loop internals at all.
- Unified the client-side carry/drag helper (`IBX_GMCarryClient`) into one mode-tracked poll loop instead of separate carry/drag booleans, so dropping and the movement-speed cap work identically for both modes.
- Rebuilt the `chimeraInputCommon.conf`/`keyBindingMenu.conf` resource database and did a full stop/play cycle via Enfusion MCP (`wb_resources rebuild`, `wb_reload both`, `wb_stop`, `wb_play`) since the drop/height/rotate keys and speed cap depend on config that plain script reload does not pick up. Could not fully confirm from the log whether this actually reloaded the input config live — if the keys are still unresponsive or show as non-rebindable in the Controls menu after this, a full close-and-reopen of Workbench (not just Stop/Play) is the next thing to try, per `PROJECT_CONTEXT.md`'s existing "config changes require rebuild/reload" note.
- Validated with `mod_validate` (scripts/prefabs/configs/references — pass).

## 2026-08-18 - Fixed carry distance, drop, height/rotate, drag movement, and speed

- Fixed the carried crate colliding with its own holder by zeroing its `Physics` interaction layer for the duration of the carry (restored on drop) instead of relying on distance alone; also increased the forward carry offset from 0.75m to 1.1m.
- Added a dedicated `IBX_CarryDrop` key (default Backspace) to drop a carried crate. The world "Carry / Put Down Crate" action can't be re-targeted once the crate is a child of the player standing right in front of the camera, so dropping no longer depends on it.
- Fixed height/rotate and the new drop key doing nothing: they were defined in `chimeraInputCommon.conf` but never attached to any active `ActionContext`, so `InputManager.GetActionValue()`/`GetActionTriggered()` always read zero. Added a dedicated `IBX_CarryContext` and the client now calls `InputManager.ActivateContext()` every tick while carrying (matching how vanilla explicitly activates `InventoryMenuContext` for the existing crate-transfer action).
- Fixed the movement-speed override doing nothing beyond the first instant: `CharacterControllerComponent.OverrideMaxSpeed()` scales the character's *current* max speed at the moment it's called rather than persisting, so it's now reasserted every carry tick (50 ms) and every drag tick (200 ms) instead of once on pickup/start.
- Fixed dragging never moving the crate: `TickDragServer` looked up `CharacterCommandHandlerComponent` via `IEntity.FindComponent()`, which doesn't find it; it's obtained from `ChimeraCharacter.GetCommandHandler()` instead.
- Validated with `mod_validate` (scripts/prefabs/configs/references — pass) and `wb_reload` (scripts, no new errors in `error.log`); still needs a live re-test of all four fixes.

## 2026-08-18 - Fixed carry/drag actions not appearing in-world

- Added `ParentContextList { "default" }` to `IBX_CarryCrateAction` and `IBX_DragCrateAction` in all 42 crate `.et` files. Every crate's existing `SCR_OpenStorageAction` either explicitly lists the `default` context or relies on being the crate's only action; a second/third custom action apparently needs it stated explicitly to attach to that context and be offered to the player at all.
- Added an explicit `ContextName "default"` to the crate's `UserActionContext` in the 41 files that left it unset (only the standalone arsenal-box variant had it), so the new actions' `ParentContextList` reference matches an explicit name instead of relying on an assumed default.
- Added `UIInfo SCR_ActionUIInfo { Name "Carry Crate" }` / `"Drag Crate"` to both actions in all 42 files, matching the pattern the existing `SCR_OpenStorageAction` uses on the arsenal-box variant; the script-side `GetActionNameScript()` override alone was not enough for the action to render.
- Validated with `mod_validate` (prefabs/configs/references — pass) and `wb_reload` (scripts); needs a live Workbench recheck.

## 2026-08-18 - Fixed BOM corruption from the carry/drag prefab edit

- Fixed all 42 crate `.et` files failing to load in Game Master ("Unexpected data 'null' at offset 0(0x0)", "Provided prefab ... is invalid!" for every crate, logged in `error.log`). The PowerShell script used to insert `IBX_CrateCarryComponent`/`IBX_CarryCrateAction`/`IBX_DragCrateAction` into all 42 files wrote them back with `-Encoding UTF8`, which in Windows PowerShell 5.1 prepends a UTF-8 BOM; the engine's config parser chokes on the leading BOM bytes. Stripped the BOM from all 42 files (content unchanged otherwise). `mod_validate` (prefabs/configs) did not catch this — it doesn't check for a leading BOM — so the failure only showed up in the live Workbench log.

## 2026-08-18 - Physical crate carry and drag

- Added `IBX_CrateCarryComponent` to all 42 crate prefabs: server-authoritative carry/drag state, holder tracking, and the offset/position math for both modes.
- Added `IBX_CarryCrateAction`, a toggle world action (added next to the existing `SCR_OpenStorageAction` on every crate) that picks a crate up or puts it down. Picking up reparents the crate onto the character (`IEntity.AddChild`) at a fixed forward offset; putting down detaches it in place and snaps it to the terrain surface.
- Added `IBX_DragCrateAction`, a hold world action that drags a crate along the ground: it only advances while the dragging player's movement input is backward (`CharacterCommandMove.GetCurrentInputAngle`), keeps the crate snapped to terrain height, and self-clears if continuous ticks stop arriving (dedicated player disconnect/death safety net) instead of relying solely on the cancel callback.
- Added client-side height (`IBX_CarryHeightUp`/`IBX_CarryHeightDown`, default Page Up/Page Down) and rotate-modifier (`IBX_CarryRotateModifier`, default `R`) input actions to `chimeraInputCommon.conf`, polled while carrying to send small server-authoritative adjustment RPCs; holding the modifier repurposes the same two keys to rotate instead of raising/lowering. Exposed all three under the existing `Inventory Boxes` key-binding category. Mouse wheel is not bound by default (no confirmed vanilla wheel input token available to verify) but can be assigned to either action through the normal key-binding menu.
- Movement is slowed with `CharacterControllerComponent.OverrideMaxSpeed()`: carrying caps speed to 35% (very slow, matching the request), dragging to 65% (slower than full speed but faster than carrying); both reset to 100% on drop/release.
- Known limitation: the carry-adjust RPC does not verify the calling client is the crate's actual holder beyond the crate's own `CARRY` state check, so a malicious client could in principle nudge another player's carried crate's height/rotation; low severity (cosmetic only, no duplication/economy impact) but worth hardening later if abused.
- Validated with Enfusion MCP `mod_validate` (prefabs, configs, references — all passed) and `wb_reload` (scripts) against the InventoryBoxes project path; `wb_play` started without reported script errors. Live pickup/drag/height/rotate behavior in Workbench still needs a hands-on pass — see `PROJECT_CONTEXT.md`.

## 2026-08-18 - Mikes-UI button layout fix

- Fixed every Mikes-UI action/tab button (arsenal tabs, Add, preset filter, Remove, Empty Crate, Copy, Export Preset, Paste, export overlay Copy) calling both `SetFillWidth()` and `SetGrow(1)`, which Mikes-UI's documented button pattern says causes each button to measure as 100% of its row instead of splitting evenly. Removed the `SetFillWidth()` calls and kept `SetGrow(1)`, matching the framework's Hug-width-plus-Grow convention.
- Validated with Enfusion MCP `mod_validate` (scripts, references) and `wb_reload` (scripts) against the InventoryBoxes project path; no errors.

## 2026-08-16 - Crate transfer binding, inventory refresh, and Escape handling

- Changed default keyboard binding for `Load / Unload Crate` from `F` to `G`, preserving vanilla `F` inventory equip/use behavior.
- Changed Game Master inventory editor opening to request an authoritative server snapshot, so crate contents remain visible after crate movement even when local replicated storage children are stale.
- Fixed Escape closing the inventory editor and then opening the pause menu from the same input event.
- Saved the user-verified binding, moved-crate inventory refresh, Escape behavior, architecture notes, and latest Enfusion validation checkpoint in `PROJECT_CONTEXT.md`.
- Added `Workshop_Changelog.md` with a cleaned, player-facing history for the Workshop page.

## 2026-08-15 - Dedicated-server Game Master inventory editing

- Added named crate presets from `Configs/Inventory/CratePresets.conf`, with a Game Master dropdown that replaces crate contents through server-authoritative validation and mutation.
- Changed preset configuration from encoded name/item strings to explicit objects with separate `m_sName` and `m_sItems` fields; exports now contain only the item-content value for `m_sItems`.
- Added an `Export Inventory Preset` crate context action and editor button that request authoritative contents, display the exact config entry, and copy it to the PC clipboard.
- Added `Paste Inventory` to the crate context menu and editor; it consumes exported item strings and replaces the target crate only after server-side format, limit, and resource validation.
- Added a dedicated Game Master inventory editor `Copy` button that copies the authoritative crate contents directly without opening the preset export modal.
- Fixed the export overlay remaining painted after closing by removing its retained clip surface; the complete editor frame is hidden while export is open so layered button text cannot render above the modal.
- Limited preset definitions to 100 unique prefab types, 1,000 items per type, and 10,000 total items per application; invalid or unavailable resources are rejected before the crate is cleared.
- Changed Game Master capacity enforcement from 100 total item entities to 100 distinct prefab types, allowing up to 1,000 items per mutation while storage weight and volume remain the total-item limits.
- Updated the crate-content heading to show distinct type usage and total item count separately.
- Compressed authoritative inventory refreshes to at most 100 prefab/count pairs instead of transmitting one resource entry per item.
- Replaced the failed context-action-component route after live dedicated testing with RPCs on the requesting player's native `SCR_EditorManagerEntity`.
- Fixed requests returning before their RPC because the crate entity itself had no replication item ID; requests now serialize and resolve its replicated `SCR_EditableEntityComponent`, matching vanilla Game Master actions.
- Required the player's editor to be open before mutating inventory.
- Updated replication ID lookup to the current `Replication.FindItemId()` API.
- Added client/server mutation diagnostics and owner-result RPC feedback to the editor status line, covering button handling, replication lookup, RPC receipt, authorization, target resolution, and inventory results.
- Replaced the fixed 300 ms client-storage refresh with an authoritative server inventory snapshot returned with each successful mutation, preventing stale replication state from clearing or reverting the Game Master list.
- Removed temporary client/server diagnostic logging after dedicated-server validation while retaining concise editor status feedback.

## 2026-08-14 - Mikes-UI Game Master shell

- Rebuilt the Game Master crate editor title, filters, tabs, text fields, faction menu, action buttons, status text, and responsive two-column layout with interactive Mike's UI components.
- Kept only the two inventory item lists native so their `ItemPreviewWidget` previews continue working.
- Fixed square background corners by removing the framework root overlay fill and letting the rounded glass card own the complete panel background.
- Added native preview-list positioning from Mike's UI viewport geometry and safe framework tick, close, and unmount lifecycle handling.
- Fixed horizontal and vertical overflow by giving both columns and the body minimal flex bases; restored the visible crate-inventory column and bottom action controls.
- Replaced the framework card shell with nested rounded theme panels so the background follows the rounded border instead of leaving sharp corners beneath it.
- Replaced the faction dropdown's clipped scroll view with a normal panel so closing it clears its buttons instead of leaving stale clip-canvas contents behind the editor.
- Saved the current Mikes-UI layout, rounded-shell fixes, faction-dropdown workaround, and latest Enfusion validation state in `PROJECT_CONTEXT.md`.

## 2026-08-10 - Timed vehicle crate transfers

- Saved the completed transfer implementation as a runtime-verified checkpoint in `PROJECT_CONTEXT.md`: custom button only, synchronized five-second hold circle, and working load/unload behavior.
- Fixed the custom transfer button's hold circle starting late and ending partially filled by bypassing vanilla's initial progress threshold only for `IBX_InventoryCrateTransfer`.
- Hid the vanilla Use/F navigation entry while an eligible crate transfer is available, leaving only the dedicated custom transfer button.
- Added matching `hold` and `gamepad:hold` source presets to the custom action so configurable bindings retain its native 5000 ms `InputFilterHoldOnce` duration.
- Changed the controls category override to append `Inventory Boxes` without replacing vanilla keybinding categories.
- Added `Load Crate` and `Unload Crate` to the native inventory Use hint when hovering an eligible InventoryBoxes crate; no separate inventory button is used.
- Added a five-second countdown in the normal inventory navigation hint before each transfer.
- Changed transfer timing to require continuously holding the configured inventory Use input for all five seconds; releasing early or invalidating the crate/vehicle cancels progress immediately.
- Fixed hold progress canceling after one frame because the UI action value is a short pulse; release is now tracked through the native `Inventory_Use` input-up event.
- Replaced the failed scripted timer/release tracking with a crate-only native `InputFilterHoldOnce` action set to 5000 ms. The existing Use hint switches to this action only for eligible crates, so other inventory Use actions keep their vanilla duration.
- Removed the unbound custom action that produced a crossed-circle icon. The existing `Inventory_Use` action now has a 5000 ms native hold and directly performs crate loading/unloading on completion; this intentionally also lengthens Use for other inventory items.
- Tried registering the crate-only action in `InventoryContext`; runtime testing still showed the crossed-circle state because the button was being dynamically repurposed.
- Replaced dynamic `ButtonUse` action swapping with a dedicated navigation-bar entry initialized from config. Registered its five-second action in the existing `InventoryMenuContext`, which vanilla explicitly activates when inventory opens; the normal Use button is untouched.
- Fixed the dedicated button not appearing by overriding the navigation-bar resource with its vanilla GUID `{E09350C3FD7F0812}` instead of an unrelated new GUID.
- Added an `Inventory Boxes` controls category with a configurable `Load / Unload Crate` binding.
- Recreated the system config files as proper Workbench overrides using the real vanilla resource GUIDs (`chimeraInputCommon` `{795184CF9AD764DB}` and `keyBindingMenu` `{4EE7794C9A3F11EF}`), then inserted the custom action/context and controls entries into those overrides.
- Fixed loading crashing in `MoveItem_VirtualArsenal` because the delayed UI move received a vehicle UI without `m_Storage`; loading now calls the native storage manager directly, while unloading retains vanilla move-to-vicinity ground placement.
- Disabled normal crate dragging and blocked quick-move actions so the timer cannot be bypassed; other items remain unchanged.
- Revalidated the hovered crate snapshot and its original vehicle/source target when the countdown completes.
- Removed the obsolete custom transfer-button layout and updated `PROJECT_CONTEXT.md`.

## 2026-08-10 - Vanilla vehicle unloading restored

- Removed all custom vehicle unloading, vicinity UI, external-storage drop, and inventory-action overrides after confirming vanilla Reforger already supports unloading.
- Kept only crate drag capability required for loading into compatible vehicle inventory.

## 2026-08-10 - Vehicle inventory crate transport

- Enabled normal inventory drag-and-drop for all 42 crate variants, allowing each crate entity and its contents to be loaded into compatible vehicle inventories.
- Kept native vehicle storage size and weight checks; no custom cargo framework or replacement spawning was added.
- Updated `PROJECT_CONTEXT.md` with the native inventory transport behavior and remaining validation scope.

## 2026-08-09 - Game Master crate placement

- Updated `PROJECT_CONTEXT.md` with the completed editable-prefab migration, finite-storage overrides, inherited action-context fix, interaction settings, and latest validation state.
- Raised every crate's open-storage action point to the box center, increased its context radius to 2.5 m, and set its visibility range to 10 m by overriding the vanilla-derived crates' existing action context instead of adding a duplicate `default` context.
- Enabled the inherited `ActionsManagerComponent` on all 36 equipment-stack and wooden-box subfolder prefabs so players receive the vanilla open-storage action.
- Disabled the inherited vanilla `SCR_UniversalInventoryStorageComponent` on all 41 vanilla-derived crates, leaving only the unrestricted 100-slot finite storage active.
- Fixed Game Master inventory insertion and display by resolving the single enabled storage after disabling the inherited vanilla storage.
- Fixed Game Master placement failing with `SCR_PlacingEditorComponent.CreateEntityServer` by removing the duplicate `ActionsManagerComponent` from vanilla-derived crates and retaining their inherited manager.
- Restored and retained the Enfusion MCP Workbench handler scripts for regular local development; remove them manually only before upload.
- Replaced the old 15 custom visual variants with all 42 user-duplicated vanilla editable crate prefabs under `PrefabsEditable`.
- Preserved vanilla Game Master preview images and preview meshes while adding finite 100-slot storage, player open-storage interaction, and the custom Game Master inventory editor.
- Renamed all 42 Game Master entries with a clear `Finite Inventory Crate` prefix and registered them in the dedicated placeables registry and filter.
- Made the duplicated arsenal-box appearance standalone and finite-only instead of inheriting the vanilla unlimited-arsenal composition.
- Updated the Game Master test world to use the duplicated US wooden launcher crate.
- Registered all 15 visual inventory crate variants in an exposed, dedicated Game Master placeables registry.
- Enabled Game Master placement and added modded object browser labels on the shared finite inventory box prefab so every crate variant appears in the asset browser and remains spawnable only through Game Master.
- Added a native `Inventory Boxes` Game Master asset filter and tagged every crate variant through the shared base prefab.
- Renamed every Game Master entry with a `Finite Inventory Crate` prefix so custom finite-storage crates cannot be mistaken for vanilla arsenal or equipment boxes.

## 2026-08-09 - Persistent project context

- Added `PROJECT_CONTEXT.md` with current feature progress, architecture, constraints, test content, and phased plans for crate carrying and vehicle loading.
- Updated `AGENTS.md` so future agents always load the project context before working.

## 2026-08-08 - Inventory editor checkpoint

- Saved the current mod, editor UI, Discord thumbnail, and announcement content as the latest Pre-Alpha progress checkpoint.
- Replaced the German-only Discord announcement with a refreshed bilingual German and English version.
- Added a polished 1920x1080 Discord Pre-Alpha thumbnail based on the current Game Master inventory editor screenshot.
- Reverted the dark-blue borders from all editor controls and item rows while retaining the blue 60% opacity fills.
- Added a consistent two-pixel dark-blue border to all editor buttons, inputs, category tabs, arsenal rows, crate-item rows, and faction-dropdown rows.
- Applied the blue 60% opacity styling to arsenal and crate item rows.
- Replaced the non-rendering Quantity and number field background tints with actual blue ButtonWidget background surfaces.
- Restyled regular editor controls, category tabs, inputs, and the faction dropdown in blue at 60% opacity while preserving the Add, Remove, and Empty Crate action colors.
- Added a matching blue background behind both the Quantity label and its number input.
- Forced the faction dropdown closed when the editor opens so it appears only after an explicit click.
- Removed the All category tab and made Weapons the default category.
- Translated all visible Game Master inventory editor labels, actions, categories, hints, and status messages from German to English.
- Saved the current implementation as the Discord Pre-Alpha presentation checkpoint.
- Added a reusable German Discord announcement covering features, Pre-Alpha status, and requested testing feedback.
- Changed the working faction selector into a dropdown menu populated from all available faction arsenal catalogs.
- Aligned every inventory row to a fixed 56-pixel preview column followed by vertically centred item text.
- Replaced the unreliable faction combo box with a native clickable faction field that cycles through available factions and refreshes the arsenal immediately.
- Fixed faction selection interaction by registering the editor as a modal workspace and raising the faction combo box above the arsenal list.
- Replaced sparse `UIInfo` texture icons with native `ItemPreviewWidget` prefab renders so all inventory item types receive previews.
- Fixed inventory opening by placing preview and label inside the single child container required by `ButtonWidget`.
- Added click-again toggling to collapse the compatible ammunition shown below a selected weapon.
- Fixed faction filtering by handling the `XComboBoxWidget` through its native `OnChange` event instead of the listbox-only selection event.
- Added compact 48-pixel preview images beside arsenal and crate inventory items using their vanilla `UIInfo` icons.
- Kept a selected weapon's ammunition rows expanded when selecting one of its magazines, and collapsed them only after selecting another weapon, changing tabs, or closing the editor.
- Consumed the first Escape press in the Game Master menu by closing the inventory editor before allowing a later Escape press to open the vanilla pause menu.
- Removed the `[PASSEND]` label prefix while retaining green highlighting for compatible ammunition.
- Displayed a selected arsenal weapon's compatible ammunition directly below it in the weapons tab, with each magazine remaining selectable for insertion.
- Fixed the Workbench script compilation error by using Enforce Script's lowercase `typename` keyword for magazine-well types.
- Removed the orphaned `InventorySupplyCrate_01.et.meta` resource metadata file.
- Highlighted ammunition compatible with the Game Master's equipped weapons in the existing ammunition tab using vanilla magazine-well types.
- Made every item listed in the crate inventory directly selectable for removal or multiplication with the existing quantity and add controls.
- Prevented background brightness changes by replacing the interactive full-screen backdrop with a static frame.
- Added Escape/back navigation to close the Game Master inventory editor without propagating the input.
- Fixed requested quantities filling the crate by counting and mutating only direct storage items and suppressing duplicate add-click events while a request is processed.
- Fixed the Game Master add operation hanging the process by replacing the unbounded native multi-spawn call with bounded single-item spawning, insertion, and cleanup on failure.
- Replaced displayed prefab resource IDs with the vanilla localized inventory item names in the Game Master inventory editor.
- Added a faction selector that filters arsenal items by their vanilla faction catalog while retaining faction-independent items.
- Split the Game Master arsenal list into tabs for weapons, ammunition, clothing, medical items, explosives, equipment, and all items using vanilla arsenal type and mode metadata.
- Added a Game Master context action to edit the finite physical inventory of every InventoryBoxes crate.
- Added a compact Game Master inventory editor backed by the vanilla arsenal catalog of all factions.
- Added server-authoritative add, remove, and clear operations with selectable finite quantities.
- Restricted inventory mutation requests to players with an active Game Master editor manager.
- Added inventory-enabled variants for all nine FIA, US, and USSR ArsenalBox appearances.
- Added inventory-enabled variants for the six gameplay wooden ammunition, equipment, explosives, launcher, medical, and weapon boxes.
- Reused the proven finite 100-slot storage prefab for every new box variant.
- Fixed item insertion by raising the universal storage weight limit from the zero-kilogram default to 1,000 kg.
- Expanded the storage volume and maximum item dimensions so weapons and other large inventory items fit into the 100 slots.
- Removed the ten preconfigured 5.56 mm magazines and increased the empty universal inventory capacity to 100 slots.
- Added 40 empty universal inventory slots so players can deposit arbitrary weapons, ammunition, medical items, and clothing alongside the initial stock.
- Made the finite inventory box a replicated, selectable Game Master editable entity.
- Renamed the corrected prefab to `FiniteInventoryBox.et` to avoid stale Workbench resource instances.
- Fixed the empty open-inventory view by adding visible inventory item attributes required by the vicinity UI.
- Fixed a placement-time null pointer by replacing the character-only inventory manager with the generic container inventory manager.
- Added a finite, configurable initial stock using inventory multi-slots.
- Added ten 30-round 5.56 mm STANAG magazines as the example crate stock.
- Removed the supplies-system inheritance from the inventory crate.
- Changed the crate to a neutral vanilla model with unrestricted storage for weapons, ammunition, medical items, and clothing.
- Added an inventory-enabled variant of the vanilla `SupplyCrate_01` prefab.
- Added vanilla inventory storage and open-inventory interaction components.
- Added the `InventoryBoxes` Arma Reforger addon project.
- Added `changelog.md` to record all project changes.
- Added `AGENTS.md` with the required Enfusion MCP testing rule.
