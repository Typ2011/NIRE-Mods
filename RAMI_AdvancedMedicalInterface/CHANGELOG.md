# Changelog

## Current release highlights

- Added a patient selector for the local character, nearby patients within 5 m, and occupants of the same vehicle. Unconscious occupants can be unloaded through the game's native casualty action.
- AMI can now be opened and used while driving, riding, or operating a turret, and all Actions keep the interface open.
- Added inventory-aware saline choices for 250 ml, 500 ml, vanilla 750 ml, 1000 ml, 1250 ml, and 1500 ml. Active saline is shown on Treatment and Medication.
- Added immediate `STOP SALINE` controls without delay or additional animation to both Saline submenus. Expanded Saline Bags returns remaining fluid to the user's inventory in 250 ml steps.
- Exact vital values and Diagnose graphs now require a medical kit; without one, AMI provides qualitative readings.
- Redesigned the patient list, vitals, medication cards, buttons, page tabs, active-page marker, and selected-body-zone outline.
- Fixed delayed Diagnose readings, saline-page null pointers, and casualty unloading from the wrong vehicle compartment.

## 2026-08-24

### Added

- Added `STOP SALINE` to both Saline submenus. It executes immediately without delay or additional animation, delegates server-authoritatively to Expanded Saline Bags, and returns remaining fluid in 250 ml steps. Confirmed in game.
- Added a third Saline row with `Saline 250 ml` and `Saline 1250 ml` for self and patient inventories.

### Changed

- Saline submenus now remain open after administering saline, and their category buttons remain available with an empty inventory so active saline can still be stopped. Confirmed in game.
- Ordered Saline buttons by volume: 250/500 ml, 750/1000 ml, and 1250/1500 ml. Confirmed in game.

## 2026-08-23

### Added

- Added `Saline 750 ml` for the vanilla US and USSR saline bags. Confirmed in game.
- Added a persistent active-page outline using fully opaque cyan while hovered and selected button backgrounds remain translucent.
- Replaced the selected body zone's fill with a `#33B3FF` anatomical outline matching the Vitals and Medications headings while preserving medical-state colors.
- Added `TREAT` and `UNLOAD FROM VEHICLE` choices after selecting a name in the patient list. Unloading is available only for unconscious occupants and executes the seat's registered vanilla `SCR_RemoveCasualtyUserAction`.
- Standardized the remaining visible German labels to English: `STOP CPR`, `REMOVE TOURNIQUET`, and `SELECT BODY REGION TO TREAT`.

### Changed

- Moved `Saline 750 ml` to the second position in both Saline submenus.
- Changed the active saline-bag body indicator from blue to bright yellow. Confirmed in game.
- Clarified AMI's GPLv2 license and documented the source, copyright, modifications, and AMI filenames of the adapted ACE3 medical-menu icons in the bundled notices and Workshop description.
- Added `Tools/Launch-RAMI-Workbench.ps1` to load the official ACE-Anvil `dev` source projects instead of their packed Workshop copies, avoiding missing source-stringtable errors during development.
- Removed the orphaned `RAMI_Tourniquet.edds.meta`; the active overlay continues to use `RAMI_Tourniquet_v2.edds`.
- Restricted exact patient-vital numbers and Diagnose graphs to players carrying a medical kit. Without one, AMI shows qualitative levels and hides graph details that would reveal exact values.
- Reduced the shared Toggle Patient selection and treatment range from 10 m to 5 m; characters in the same vehicle remain valid regardless of distance.
- Show the blue pulsing saline-bag indicator on both Treatment and Medication while saline is active; volume and time details remain on Medication.
- Disable and grey each Saline category button when its corresponding inventory contains no saline; an open saline submenu remains available so users can return to standard items.
- Split patient vitals and active medications into separate right-side cards. Both use a fixed compact font; the vitals card sits higher so the scrollable medication list has more space. Medication values align independently to the card's right edge with a 6 px gap to the vertical scrollbar.
- Changed custom mouse-hover and active button backgrounds and patient selections to 60%-opaque cyan; the active-page outline uses fully opaque cyan.
- Outline actively bleeding patient buttons in bright red. Unconscious entries use a grey background and retain the red outline while bleeding; deceased entries use an opaque black background without a bleeding outline.
- Center patient names across the full patient-button width and remove the displaced `+` marker.
- While patient choices are open, hide every other patient name. Clicking the selected name again closes the choices and restores the full list.

### Fixed

- Fixed patient-list controller navigation so Down moves from the selected patient to `TREAT`, then to enabled `UNLOAD FROM VEHICLE` or directly to `CLOSE` when unloading is unavailable. Confirmed in game.
- Show the current exact or qualitative Diagnose reading immediately when its monitor button is pressed instead of waiting for the first polling interval.
- Resolve unloading from the occupant's actual compartment instead of reconstructing its seat from the root vehicle; use the seat's registered vanilla `SCR_RemoveCasualtyUserAction` and remove RAMI's unsuccessful custom ejection paths.

## 2026-08-22

### Changed

- Restyled the right-side patient-vitals display as a compact dark card with a cyan heading, divider, clearer spacing, and separated label/value colors.
- Labeled both 500 ml saline choices explicitly as `Saline 500 ml`.
- Moved the Saline category button to the first full-width position for self and patient inventories. It now switches between the standard medication list and the 500 ml, 1000 ml, and 1500 ml saline choices like the AMI ACE Breathing Compat category menu.
- Restyled the patient selector with a cyan title and divider, a subdued subtitle and inset list surface, medical row markers, left-aligned names, and equal button spacing on both sides. Hovered or selected entries remain red with bold white labels.
- Added the same red hover treatment with bold white labels to Close, both Diagnose monitor buttons, every Medication button, and every Actions button; active monitors remain highlighted.

## 2026-08-21

### Changed

- Replaced page-tab text with compact icons imported from the official ACE3 medical-menu artwork while preserving the existing button sizes and documenting source and GPLv2 attribution.
- Collapsed the three saline volume entries into one Saline control that expands to 500 ml, 1000 ml, and 1500 ml choices.
- Made the existing close action collapse expanded Saline choices before it closes the medical menu.
- Colored the selected NONE triage button gray.
- Swapped the Diagnose and Actions page-tab icons.
- Increased the shared Toggle Patient selection and treatment range from 3 m to 10 m; characters in the same vehicle remain valid regardless of distance.

### Fixed

- Prevented saline submenu visibility updates from dereferencing empty medication rows while another page is being bound.

## 2026-08-20

### Added

- Added a patient-selection window beside AMI. Toggle Patient opens or closes it; the list contains the local character plus every currently treatable nearby character, including every occupant of the same vehicle, and selects patients directly by name. Confirmed working in game on 2026-08-20.
- Added same-vehicle treatment authorization to RAMI's shared server-side range check, so all existing treatments, inventories, vitals, medications, and triage paths work for vehicle occupants.

### Changed

- Moved AMI's opening action to the global in-game input context so it remains available while driving, riding, or operating a turret. Confirmed working in game on 2026-08-20.
- Changed Toggle Patient from direct self/target switching to opening the patient-selection window.
- Matched the patient-selection window to AMI's rounded 75% black canvas background and uniform opaque 6 px black outline.
- Replaced the AMI background panel with a style-independent rounded canvas: a 75% opaque black fill and a fully opaque black outline rendered as a uniform 6 px ring on every side.
- Removed the SELECTED // prefix from the selected body-zone label.

## 2026-08-16

### Changed

- Styled usable buttons white and unavailable buttons gray at 90% opacity with black default text. Selected Triage buttons retain their medical level color and readable contrast text; invisible hit targets remain transparent.
- Kept AMI open after every Actions button, including carry, drag, positioning, vehicle loading, and CPR.

## 2026-08-15

### Added

- Added separate 500 ml, 1000 ml, and 1500 ml saline treatment options for self and patient inventories using Expanded Saline Bags prefabs, including US and USSR item matching.

## 2026-08-13

### Fixed

- Matched the vanilla combo preset so RAMI's default R1/RB + D-pad Right binding is shown correctly in Controls while retaining hold R1/RB + tap D-pad Right behavior.
- Made the remappable Open Medical Interface action close RAMI too, removing the separate hardcoded H close binding while retaining Escape and B/Circle.
- Added RAMI's opening combo to the native bottom-left available-controls list while R1/RB is held, beside controls such as Toggle Weapon Safety.
- Added instant controller page cycling with LT for previous and RT for next, wrapping across the five content pages.
- Made Close + D-pad Up on Actions select the last enabled action, or return to the Actions tab when none is available.
- Made D-pad Down on the Actions tab select the first enabled action, or Close when no action is available.
- Made Close + D-pad Up return to each page's rightmost final control, including the Triage activity log.
- Completed controller edge navigation between the treatment body, each page's final controls, and the Close button.
- Made D-pad Down enter the Actions page controls from its page tab, matching Diagnose and Triage navigation.
- Attached the native gamepad-scroll component to the triage activity widget so selecting the activity log no longer causes repeated null-pointer errors.
- Added controller support for RAMI: hold R1/RB + D-pad Right to open without conflicting with binoculars, use predictable menu navigation—including body-zone, available-item, close-button, and triage activity selection—scroll the focused activity log with the right stick, and press B/Circle to close. One prominent cyan highlight marks controller focus and disappears when mouse or keyboard input resumes.

## 2026-08-08

### Added

- Added a dedicated RAMI group to the game's Interface settings with persistent controls for colorblind mode and ten button-text presets (`Default`, `White`, `Yellow`, `Cyan`, `Black`, `Red`, `Orange`, `Green`, `Blue`, and `Magenta`). The colorblind-safe palette covers triage, bleeding, wounds, and fractures. Compiled in the Workbench on 2026-08-08.
- Increased the retained patient activity history from 20 to 50 entries and made the text size itself to its full content height so overflowing entries can actually be reached by scrolling. Confirmed working in game on 2026-08-08.
- Limited patient activity entries to triage changes, medications, and treatments; injury, vital-state, CPR, and life-state events are no longer recorded.

### Changed

- Reordered the patient activity log chronologically, with the oldest entries at the top and the newest entries at the bottom. Confirmed working in game on 2026-08-08.
- Added gray backgrounds to the Treatment, Medication, Diagnose, and Triage buttons, reduced button-background opacity by 50% across those pages and Advanced/Misc, made their button text bold, and added subtle automatic contrasting shadows to all button text.

### Fixed

- Attached the RAMI Interface settings group to the native settings scroll content instead of relying on a nonexistent widget named `Content`.
- Stored RAMI values in a proper `ModuleGameSettings` module and reloaded them through `OnTabShow` whenever the Interface tab opens.
- Bound RAMI controls to the inherited spinbox components instead of duplicate hidden components, allowing selections to trigger and persist correctly. Confirmed in game on 2026-08-08.

## 2026-08-07

### Changed

- Renamed the visible Triage button label from `EXPECTANT` to `DECEASED`; the underlying `EXPECTANT` triage value remains unchanged.
- Changed the Advanced CPR button into a `START CPR` / `STOP CPR` toggle that keeps RAMI open. Its running state now comes from ACE vitals instead of view-dependent action availability, and cancellation terminates the executing player's native ACE CPR helper compartment.
- Removed the redundant treatment-page instruction text and expanded the page title into its space.

## 2026-08-03 2

### Added

- Added the completed Triage page with selectable None, Minimal, Delayed, Immediate, and Expectant levels shared through the server.
- Added a bounded server-side patient activity journal for triage changes, injuries, life/vital-state transitions, CPR, tourniquets, medication, and consumable treatments. Events store UTC Unix timestamps and display using each client's native local system time.

### Changed

- Replaced the Triage page's "CURRENT TRIAGE // ..." text readout with direct button feedback: the selected level's button now fills with that level's color (white for None, green for Minimal, yellow for Delayed, red for Immediate, black for Expectant) and its label switches to a readable contrasting color, while unselected buttons dim to gray.
- Confirmed RAMI is functionally complete across all six pages (Triage, Diagnose, Bandages & Fractures, Medication, Advanced/Misc, Toggle Self). Remaining work is dedicated-server multiplayer regression testing, localization, and Workshop packaging.
- RAMI no longer needs a manually placed `RAMI_MedicalMenu.et` entity in each mission world. `modded class SCR_BaseGameMode` (`RAMI_GameMode.c`) now registers the input listener and starts the body-zone snapshot loop directly, so RAMI works in every gamemode automatically. Compiled and confirmed working in the Workbench on 2026-08-03.

### Removed

- Removed `RAMI_MedicalMenuComponent.c`, `Prefabs/RAMI_MedicalMenu.et`, and the standalone entity placement in the GameMaster test world; their logic moved to `RAMI_GameMode.c`.


## 2026-08-03

### Added

- Added a Medical Kit button below Tourniquet. It equips an available ACE/vanilla medical kit and executes the native heal-support action for the selected patient body zone. RAMI stays open so additional healing cycles can be started when needed. Confirmed in game through full multi-cycle healing and automatic stowing.
- Added per-body-zone injury indicators using the existing `Wound_1_UI` through `Wound_4_UI` inventory icons, selected with the same intensity thresholds as inventory and tinted continuously from yellow to red by remaining hit-zone damage. Confirmed in game.
- Added selected-zone injury percentage plus impaired aiming for arms or impaired movement for legs to Treatment patient vitals.
- Added a blue pulsing `Saline-bag_UI` indicator while saline is active, with remaining volume and time in Medication patient vitals. Confirmed in game using the server-authoritative `SCR_SalineDamageEffect`.

### Changed

- Hid the Medical Kit treatment row when the treating character has no medical kit.
- Doubled the saline status icon size for clearer visibility.
- Added the active character name above the Diagnose vital monitors, including live self/patient switching.
- Added a smoothly animated P-QRS-T-style rhythm line synchronized to monitored ACE heart rate and a labeled X/Y blood-pressure chart showing 30 seconds of systolic/diastolic history. The rhythm shape represents rate only; graphs freeze when stopped and reset for a new patient or monitoring run.
- Confirmed the complete Diagnose page in game with AI patients and on a server: both monitors, values, graph sweeps, patient name, start/stop controls, and monitoring persistence across page switches work as intended.

### Fixed

- Scoped **H** and **Escape** closing to RAMI's private input action and removed the global pause-menu override, preventing **H** from closing unrelated menus and one **Escape** press from affecting two menu layers.
- Removed the unused medical-menu instance state left by the previous toggle-input implementation.
- Fixed saline status being absent by reading the exact server-authoritative `SCR_SalineDamageEffect` through the existing medication-summary RPC.
- Drove the native medical-kit action continuously while RAMI is open, allowing ACE to execute as many healing cycles as the selected body zone needs before the action is released and the kit is stowed.
- Waited for item-switch animations to finish before starting one-click treatments, so a newly equipped medical kit heals on the first button press.
- Prevented the continuous medical-kit updater from completing a treatment while the kit is still only being equipped.
- Prevented heart-rhythm jumps when updated ACE heart-rate samples differ by accumulating waveform phase continuously. Reduced Diagnose monitor button height and font size so blood-pressure controls no longer overlap the chart.
- Removed the hidden horizontal `ContentArea` size offset so page layouts, including Diagnose, receive the exact 1804.8 × 745.2 area defined by the 0.03/0.18/0.97/0.87 anchors at 1920 × 1080.
- Rebuilt Diagnose for the 1804.8 × 745.2 content aspect ratio: heart rhythm and blood-pressure charts now use side-by-side panels instead of the vertically compressed 1920 × 1080 arrangement.
- Stopped rebuilding the heart graph's Canvas command array every frame. The rhythm now retains one draw command and updates its vertex buffer in place, preventing renderer flicker and jumps.
- Replaced whole-waveform regeneration with a sampled 60 Hz rhythm trace, eliminating BPM-update warping and jumps.
- Inset the heart trace by 12 pixels and added a bright leading dot. The trace now sweeps across the field and restarts after six seconds, keeping motion visible at an unchanged heart rate.
- Matched the blood-pressure chart to the heart display with inset axes, colored systolic/diastolic leading dots, and a left-to-right 30-second sweep that restarts when full.
- Kept active heart-rate and blood-pressure monitoring alive across page switches. Returning to Diagnose restores button state, current values, and graph history; monitoring now ends only through its button or menu closure.

## 2026-08-02

### Changed

- Added toggleable heart-rate and blood-pressure monitors to Diagnose. Both request authoritative ACE patient data at ACE's one-second vital update interval and stop polling when disabled or when leaving the page.
- Merged CPR and patient movement into one Advanced/Misc page. CPR, carry, drag, back position, left/right recovery position, and load-into-vehicle buttons use the registered ACE/engine actions and availability checks, with grey backgrounds while disabled. Confirmed the page is functionally complete in game, on a Dedicated Server, and with AI patients.
- Medication-page patient vitals now show each administered dose as its own three-line block with peak countdown, half-life countdown, and current strength. Medication names occupy the middle line. Values are calculated from ACE's live medication state and runtime pharmacokinetics settings.
- Medication strength now applies ACE's configured pharmacodynamic antagonist scaling and names active counter-medications. Pharmacokinetic peak and half-life timers remain unchanged because ACE antagonists modify effects, not drug activation or clearance. Per-dose display, countdowns, and counter-medication calculation were confirmed in game.
- Confirmed the medication page is functionally complete and works in game, on a Dedicated Server, and with AI patients; no medication-page regression checks remain open.
- Added medication controls for epinephrine, morphine, naloxone, phenylephrine, metoprolol, ammonium carbonate, and saline, with compact two-column self and patient inventory grids.
- Kept medication controls inside the inventory column so the shared clickable body model and patient vitals remain visible, and restored medication item previews.
- Disabled and greyed unavailable medication controls; injections use arms or legs, saline only arms, and ammonia only the head of an unconscious patient.
- Prevented foreign Bandage and Tourniquet controls from reappearing on the medication page during patient refreshes.
- Moved treatment and medication inventory controls into separate child layouts hosted by the shared treatment page's `InventoryFrame`; control placement is now layout-owned with explicit zero offsets.
- Normalized both inventory-control layouts to use their complete width and height instead of retaining page-relative anchor margins.
- Confirmed the medication screen works in game with the shared body model, patient vitals, layout-hosted self/foreign inventory controls, item previews, counts, and availability states; the screen is now nearly ready.
- Delegated medication eligibility, including saline and ammonia restrictions, to ACE's existing consumable validation.
- Aligned all patient-vital values in a dedicated right-aligned column.
- Made bleeding body zones pulse smoothly between their previous opacity and a brighter highlight while living, and remain static after death.
- Added the patient name to the foreign-inventory heading.

### Fixed

- Fixed **H** and **Escape** closing the medical menu, and prevented **Escape** from reopening the pause menu.
- Corrected displayed blood volume, blood loss, and bleeding rates to use ACE's 6000 ml total blood volume.
- Prevented opening foreign inspection outside treatment range and closed it when either character moves out of range; verified in game.

## 2026-08-01

### Added

- Completed the treatment screen functionality.
- Added direct bandage and tourniquet actions.
- Added separate self and patient inventory selection.
- Added inventory counts and 3D item previews.
- Added tourniquet overlays and direct tourniquet removal.
- Added blood-loss hemorrhage classifications.

### Changed

- Pressing **H** now closes the medical menu.
- The cursor now centers when opening the menu.
- The menu now stays open after starting a treatment.
- Invalid treatments are automatically disabled.
- Treatments are blocked for deceased patients.

### Fixed

- Fixed foreign inventory support and server-authoritative item transfers.
- Fixed empty item-preview flickering.
