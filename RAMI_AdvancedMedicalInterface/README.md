# Advanced Medical Interface (AMI)

AMI is a controller-ready UI layer for ACE Medical in Arma Reforger. It shows ACE patient data and delegates treatment to existing ACE and engine actions; ACE stays authoritative for simulation, replication, inventory, and treatment effects. `RAMI` remains the internal code and resource prefix.

## Dependencies

- ACE Medical Circulation Dev (`65AD7D4F994EA327`)
- ACE Medical Hitzones Dev (`65B343F799FB521B`)
- ACE Carrying Dev (`65AD7C379CBD394D`)
- Expanded Saline Bags (`6A05287AF042071B`)
- Transitively: ACE Medical Core Dev and ACE Core Dev

## Status

Functionally complete across five medical pages and the patient selector. AMI follows the game's interface language and includes all 13 Reforger languages. Remaining work: visual and native-speaker localization review, additional documented language/multiplayer edge cases, load testing, and Workshop packaging. See [`ACE_Medical_UI_Reforger_Modplan.md`](ACE_Medical_UI_Reforger_Modplan.md) for full status and [`CHANGELOG.md`](CHANGELOG.md) for dated history.

The user tested the current version on a server and confirmed that everything works as intended on 4 November 2026. This completes functional server acceptance based on the user report. [`Tests/RAMI_ServerTestplan.md`](Tests/RAMI_ServerTestplan.md) records that result and retains the regression matrix; per-case results, server versions, and logs were not supplied.

Translations live in [`Language/RAMI_localization.st`](Language/RAMI_localization.st). After editing translations in Workbench's String Editor, export the runtime tables alongside the source table. Run `node Tests/RAMI_Localization.test.cjs` and `Tests/RAMI_MenuPages.test.ps1` to check localization resources and menu wiring. The separate ACE Breathing compatibility addon maintains its own UI texts.

## Usage

- **H** opens or closes AMI by default. `Open Medical Interface` is remappable in Controls.
- Controller default: hold **R1/RB** and tap **D-Pad Right**. **LT/RT** cycles the five medical pages.
- **Toggle Patient** opens a list containing yourself, patients within 5 m, and every occupant of your vehicle.
- Body region selected on the shared 2D body model.
- **Escape**, **B/Circle**, the mapped open action, or the close button closes AMI.
- Foreign treatment uses the shared 5 m range; occupants of the same vehicle remain valid regardless of distance.

## AMI while unconscious

By default, unconscious players cannot open AMI. To allow observation of their treatment, enable `m_bAllowToOpenAMIMenuWhileUnconscious` in the scenario's Mission Header under **AMI**. Conscious players retain access. This option does not grant treatment permissions.

Workbench derives the checkbox caption from the field name: **Allow To Open AMI Menu While Unconscious**. The hover description explains the default and observation-only behavior. The previous development field `m_bRAMI_AllowMenuWhileUnconscious` was renamed; update existing mission headers and server JSON to the new key.

**World Editor Play/F5 starts the loaded world, not an arbitrary header config.** Editing a `.conf` in the Resource Manager does not activate it for F5. Test header settings by launching that scenario through the game's Scenarios menu, with AMI enabled. Its `World` property must reference the current world resource. Without an active header, AMI stays blocked while unconscious and logs a warning.

Server administrators can override the scenario setting through the native `game.gameProperties.missionHeader` block in their server configuration. Merge this fragment into the existing JSON, preserving other settings:

<!-- RAMI unconscious menu server override -->
```json
{
  "game": {
    "gameProperties": {
      "missionHeader": {
        "m_bAllowToOpenAMIMenuWhileUnconscious": false
      }
    }
  }
}
```

`false` blocks AMI while unconscious and closes an already open AMI window when unconsciousness begins, including an open saline submenu. `true` allows observation. Omitting the override uses the scenario's value, which defaults to `false`. The server distributes the effective setting to clients, including players joining later. Restart the server process after changing the header or JSON value: AMI retains the first valid header and its server override. This is not a live configuration toggle.

Validate with `node Tests/RAMI_UnconsciousMenu.test.cjs`, alongside the menu wiring and localization checks. The [server test plan](Tests/RAMI_ServerTestplan.md) records successful functional server acceptance and preserves the configuration/replication cases for future regression tests. Individual case results were not supplied.

## Pages

- **Triage** — server-shared triage level (None/Minimal/Delayed/Immediate/Expectant) with button-color feedback, plus a scrollable per-patient journal retaining the latest 50 triage changes, medications, and treatments.
- **Diagnose** — patient name, toggleable heart-rate and blood-pressure monitors polling ACE's 1 Hz vitals, animated rhythm line and 30-second BP history chart; monitoring persists across page switches.
- **Bandages & Fractures** — per-zone bleeding/fracture visuals, wound icons, direct Bandage/Tourniquet/Medical Kit actions using inventory items via ACE's consumable flow.
- **Medication** — self/patient inventory grids for epinephrine, morphine, naloxone, phenylephrine, metoprolol, ammonium carbonate, and saline. The Saline submenu order is 250 ml, 500 ml, vanilla US/USSR 750 ml, 1000 ml, 1250 ml, and 1500 ml. Active saline uses a bright-yellow body indicator.
- **Advanced/Misc** — CPR, carry, drag, back position, left/right recovery position, load-into-vehicle, using registered ACE/engine actions and availability checks.
- **Toggle Patient** — selects patients by name, then offers `TREAT` and, for unconscious vehicle occupants, `UNLOAD FROM VEHICLE`.

## Architecture

- `Scripts/Game/RAMI/RAMI_GameMode.c` — `modded class SCR_BaseGameMode` registers the input listener and body-zone snapshot loop in every gamemode; no prefab or manual world placement needed.
- `UI/layouts/` — `RAMI_MedicalGlobal.layout` (navigation, ContentArea) plus one layout per page.
- AMI stores no medical truth locally beyond a frozen visual body-zone snapshot for display after death; ACE remains the source of health, bleeding, medication, and treatment state.

## License

Advanced Medical Interface is licensed under the GNU General Public License version 2. Page-tab icons are adapted from ACE3 artwork under the same license. See [`license.txt`](license.txt) and [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).
