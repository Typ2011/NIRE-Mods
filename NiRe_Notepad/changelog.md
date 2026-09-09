# Changelog

All notable changes to NiRe Notepad are documented here.

## [Unreleased]

### Added

- Full-screen Logistics workspace built on Mikes UI, laid out like the InventoryBoxes Game Master crate editor: a supply request column, an arsenal column with categories, search and faction filter, and a request-contents column.
- Mikes UI as a project dependency of NiRe Notepad.
- Separate requester and logistician roles with two Game Master character context actions.
- Chat-interface notifications for each request status change, sent to the requester and all same-faction logisticians.
- `Configs/Server/ServerConfig.conf` rules for allowed InventoryBoxes crate prefabs and per-item maximum contents.
- Rebindable notepad action with `F6` as the default.
- Six tabs and a non-modal map overlay.
- Named local journals for CAS, Artillery, and MEDEVAC plus a synchronized Logistics request list.
- Fixed 5-liner and 9-liner templates for CAS and MEDEVAC.
- Fixed 5-liner and 7-liner templates for Artillery.
- Persistent Terrain Baptism canvas with pen, eraser, clear-all, and five drawing colors.
- Read-only comparison of a Logistics request with every crate inventory within 5 metres.
- Shared, faction-scoped Logistics requests with material, quantity, pickup or delivery, coordinate, note, and workflow status.
- InventoryBoxes integration for selecting, spawning, and automatically filling finite crate prefabs from the arsenal catalog.
- Expanded Saline Bags integration for the additional 1000 ml and 1500 ml saline items.
- Delivery tracking with crate ready, in transit, arrived, and completed states; pickup crates use a dedicated ready-for-pickup state.
- Confirmed entry deletion.
- Localization for all 13 supported Arma Reforger languages, driven by the selected game language.
- Immediate local JSON persistence plus temp/backup recovery.
- Backward-compatible migration of version 1 profiles.
- Structural PowerShell validation.
- A 1920×1080 Workshop thumbnail in supported JPG format.
- German and English Workshop summaries, descriptions, and Discord presentation texts.

### Changed

- The Logistics tab no longer opens inside the corner notepad; its button opens the full-screen supply request workspace, so item names have roughly three times the row width they had before.
- Request selection, material selection, per-item quantity, pickup/delivery, coordinate, the four 45-character NOTE lines, submit, accept/hold/reject and crate creation all moved from the notepad into that workspace.
- Check Crate opens a full-screen card overlay instead of taking over the notepad picker.
- Request-contents rows use 18-pixel item names, matching the arsenal rows, instead of the 13-pixel notepad size.
- The arsenal column is wider than the request-contents column, so the search field and the item names get the extra room.
- Ammunition keeps its own item name everywhere instead of being renamed to its caliber, in the arsenal list, the crate cards and the material names stored on a request.
- Escape closes the notepad on the key up edge, so the vanilla pause action still sees an open menu on the way down and stands aside instead of opening the pause menu behind it.
- Escape closes exactly one layer: the logistics workspace if it is open, otherwise the notepad. The workspace marks the press as spent so the notepad's own Escape action cannot close it as well.
- The logistics workspace is its own menu preset rather than a bare workspace modal, so the engine frees the cursor and takes movement off the player while it is open.
- Opening the logistics workspace closes the notepad and closing the workspace opens it again, so the notepad is never left open behind a modal where the engine hides it with no way to show it again.
- The Game Master pause action is intercepted like the InventoryBoxes crate editor does, because it has no open-menu check of its own.
- Changing the arsenal category, faction filter or search text scrolls the arsenal list back to the top; selecting a row keeps its position.
- The stray leading space the activating keystroke leaves in a Mikes UI text field is removed, so the first search term matches and a typed quantity no longer parses as zero.
- The arsenal search matches every typed term separately and in any order, so "m16 olive" finds "M16 Carbine - Olive".
- Crate creation is a dropdown under the request contents; the server configuration decides which crates it lists.
- The notepad keeps General, CAS, Artillery, MEDEVAC and Terrain Baptism; profiles that stored Logistics as the active tab open on General instead.
- Replaced the Workshop thumbnail with a completely new outdoor observation-point scene featuring the handheld black-and-gold notepad at 1920×1080.
- Template prompt/answer rows now use vertically centered 20-pixel text in compact 40-pixel rows.
- General Free Note uses ten individual text rows; Logistics reuses four rows limited to 45 characters each.
- The rounded outer outline is uniformly 5 pixels thick and gold over an 80% opaque black background; the material Quantity outline is one pixel for readable digits.
- CAS, Artillery, and MEDEVAC Free Text now use the same ten individual writing rows as General.
- Game Master requester/logistician actions now include localized descriptions and permission icons.
- Check Crate and New Request share the bottom of the Logistics request list; New Request opens a clean requester draft.
- Ammunition entries use `MagazineUIInfo` caliber data and never fall back to prefab or class names.
- Quantity and remove controls sit at the far-right edge of selected-material rows.
- Logistics NOTE rows align with the Coordinate field, and their label is inset farther from the left edge.
- CAS, Artillery, and MEDEVAC templates use one editable prompt and answer field per point and persist immediately in profile version 6.
- Requesters can create new requests and read their own submissions, but submitted fields, status controls, crate comparison, and crate creation are logistician-only.
- Logisticians receive and manage every request from their faction; requesters receive only their own requests.
- Selected-material rows are shorter and closer together; three-digit quantities and compact remove buttons now sit together at the right edge.
- Logistics material and crate selection now use a focused full-notepad picker with InventoryBoxes item previews, search, and arsenal categories.
- Logistics material selection now supports faction filtering and multiple item types with an individual quantity per selection.
- The material picker now shows selected items in a right-side list with inline quantity editing and removal; selected weapons expand compatible ammunition directly below them.
- Material search keeps the normal focus outline; selected-item quantities use a thinner one-pixel outline, aligned labels, and a smaller remove control.
- The draft button keeps the `Select Material` label instead of replacing it with the current selection.
- The `Select Material` button now matches the other Logistics action colors.
- Pickup requests use an editable Pick Up Coordinate field; the coordinate uses the normal focus outline and material quantities use a thin one-pixel outline.
- Logistics coordinates are limited to seven characters, leaving enough room for their label.
- The Logistics draft now uses a full-height selected-items column; the standalone Quantity field was removed and coordinate/note fields use compact aligned widths.
- Supply Request rows now show `Request N` and status in separate columns; the unused Logistics `New` button was removed.
- Crate selection uses the full picker area, while Check Crate renders one named contents card for every nearby crate.
- Logistics access actions now show `Logistics Access granted` or `Remove Access` in mixed case and treat admins and full Game Masters as permanently authorized.
- Per-item quantity controls align to the right edge of each selected-material row.
- Per-item quantity controls are more compact, ammunition rows show caliber-only labels, and the selected-material pane now reaches the bottom edge.
- Existing Supply Requests remain editable in every workflow state through the same server-authoritative save path.
- Supply Request rows start directly below the `Supply Requests | Status` heading.
- Operational tab subtitles were removed and both content panes now extend upward.
- All source UI copy and templates are English; player-facing strings now come from complete 13-language runtime tables.
- Logistics entries are named Supply Requests.
- The former Transport tab is now Terrain Baptism in English and Geländetaufe in German.
- Templates are selected only by clicking a mission row, not by changing tabs.
- Template points appear as separate editable prompt and answer pairs.
- Selecting Free Text clears the active entry before opening the editor.
- Operational entries appear in left-side journals with compact rows and visible selected/hover states.
- Template selection appears directly below the tabs after a mission-row click while other content is hidden.
- `Done` sits at the bottom right.
- The notepad remains in the lower-right corner; its title is centered and template points use compact paired fields.
- `Done` now matches the tab colors and highlights on hover.
- Note-editor deactivation now transfers focus to the active tab; a non-consuming pointer action detects clicks outside the editor.
- Template prompt and answer fields use native caret movement, wrapping, and Enter line breaks; Free Text uses ten native single-line rows.
- Logistics request submission, status changes, permissions, and crate creation are validated by the server and synchronized to authorized faction members.
- The former local Logistics journal is replaced by the faction-scoped shared request workflow.

### Fixed

- VON transmit, direct-speech toggle, transceiver cycle, and long-range toggle stay usable while the notepad is open, including while a note is being typed, because `VONContext` is raised above the blocking text-edit and active-widget menu contexts.
- Removed unsupported `Min Font Size` property from the single-line note editor, preventing its GUI layout parser error.
- General rows five through ten are re-enabled when leaving Logistics, instead of retaining the hidden Logistics-row state.
- Corrected rounded-rectangle size handling so the gold frame appears equally on all four sides.
- Replaced the full-size gold polygon with a separate five-pixel line, so no gold can show through the 80% black background.
- Removed the Logistics NOTE row's right padding so its outline ends with the Coordinate field.
- Extended Logistics NOTE rows slightly to the right edge of the visible Coordinate outline.
- Removed vertical row padding that clipped the lower edge of the fourth Logistics NOTE line.
- Logistics NOTE rows end above Submit and management controls.
- Terrain Baptism now registers its drawing input surface with the shared widget handler.
- Requester and logistician Game Master menu entries now use distinct action types so both remain visible in the character context menu.
- Dedicated-server clients now register the notepad input after their local player is registered.
- Widget validation conditions stay below Enfusion's formula-complexity limit.
- A single click anywhere in the NOTE area now focuses the editor instead of being intercepted or immediately deactivated.
- Split the oversized Logistics widget-validation expression to stay within Enfusion's formula-complexity limit.
- The Logistics note focus outline now covers the complete note field.
- Supply Request names and statuses now have a visible divider.
- Create Crate now reads InventoryBoxes' own placeable-prefab registry, so all finite crate choices appear.
- Created Logistics crates now use the terrain surface height instead of spawning above the ground.
- Supply Requests is applied before the Logistics refresh path returns, replacing the stale Fire Missions heading.
- Check Crate now shows its result alone in the focused picker instead of overlapping the request form.
- Localized 5-liner and 9-liner escape sequences are converted to real line breaks before editing.
- Template prompts and answers render in separate multiline fields.
- The map remains interactive behind the notepad.
- The active editor has a clear gold focus border.
- Notes survive closing, disconnects, reconnects, crashes, and restarts.
- Escape closes the notepad first without leaving `PauseMenuUI.m_InputManager` uninitialized.
- The open/close action remains available during text editing and on the map.
- Terrain Baptism drawing now uses a dedicated pointer surface; the eraser removes stored strokes instead of painting gray.
- MEDEVAC templates use the same per-point prompt/answer rows as CAS and Artillery.
- Logistics crate comparison ignores the player and carried inventory, then reads every storage within 5 metres.
- Restored the complete fixed-size lower-right root slot required by the GUI parser.
- The CAS 9-liner now contains exactly nine lines in every supported language.
- Removed the 16 ms row-selection tracker that rewrote editor text and reset the caret.
- Arsenal crates are recognized through their recursive arsenal component and their physical inventory can be compared within 5 metres.
- Unchanged legacy MEDEVAC templates migrate safely into per-point prompt fields; non-template notes are preserved.
- Removed manual character filtering; native Enter remains available in template prompt and answer fields.

### Validation

- Shared ten-row General/operational Free Text, extended Logistics NOTE rows, 80% black background, and uniform 5-pixel gold frame pass structural checks, Workbench validation, and dedicated-server startup through `Entered online game state`.
- Compact template/note rows and rounded thick outlines pass structural checks, Workbench script/resource validation, and dedicated-server startup through `Entered online game state`.
- Native multiline editing, per-point templates, Logistics NOTE spacing, profile version 6 migration, and all 13 runtime languages pass structural checks, Workbench compilation, and dedicated-server startup through `Entered online game state`.
- Context descriptions/icons, Logistics layout, caliber-only ammunition, Terrain input, and editable templates pass structural checks, Workbench script compilation, and dedicated-server startup through `Entered online game state`.
- Distinct requester/logistician context actions pass structural checks, Workbench script compilation, and dedicated-server startup through `Entered online game state`.
- Requester/logistician role separation, chat notifications, and Logistics server configuration pass structural checks, Enfusion MCP validation, Workbench script compilation, and dedicated-server startup through `Entered online game state`.
- Taller NOTE field, compact three-digit quantity rows, dedicated-client input registration, and split widget validation pass structural checks and Workbench PC script validation.
- Updated Logistics picker, permissions, coordinates, and layout pass structural checks and Workbench PC script validation.
- Multiple Logistics materials, faction filtering, Expanded Saline Bags, and grounded crate spawning pass structural checks and Workbench PC script validation.
- The selected-material list, inline controls, and compatible-ammunition expansion pass structural checks and Workbench PC script validation.
- Split Supply Request rows, full-size crate selection, multi-crate cards, and the revised Logistics form pass structural checks and Workbench PC script validation.
- Editable requests, delivery/pickup workflow states, caliber labels, and expanded pane layouts pass structural checks and Workbench PC script validation.
- Structural test covers fixed templates, structured Logistics requests, Artillery 7-liner, Terrain Baptism drawing, localization, and profile migrations through version 6.
- Structural test and Workbench PC validation pass; only unchanged third-party/profile-resource warnings remain.
- New Workshop thumbnail visually inspected and verified at 1920×1080 and 0.35 MB; publication copy remains within the official character limits.
- World startup was confirmed without the note-row GUI parser error or stale InventoryBoxes source-directory warning.
- Current ten-row Free Text, template rows, Logistics layout, 80% black background, and separate 5-pixel gold frame were confirmed in game by the user.
