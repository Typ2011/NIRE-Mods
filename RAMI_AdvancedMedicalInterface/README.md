# Advanced Medical Interface (AMI)

AMI is a controller-ready UI layer for ACE Medical in Arma Reforger. It shows ACE patient data and delegates treatment to existing ACE and engine actions; ACE stays authoritative for simulation, replication, inventory, and treatment effects. `RAMI` remains the internal code and resource prefix.

## Dependencies

- ACE Medical Circulation Dev (`65AD7D4F994EA327`)
- ACE Medical Hitzones Dev (`65B343F799FB521B`)
- ACE Carrying Dev (`65AD7C379CBD394D`)
- Expanded Saline Bags (`6A05287AF042071B`)
- Transitively: ACE Medical Core Dev and ACE Core Dev

## Status

Functionally complete across five medical pages and the patient selector. Remaining work: dedicated-server regression testing, localization, and Workshop packaging. See [`ACE_Medical_UI_Reforger_Modplan.md`](ACE_Medical_UI_Reforger_Modplan.md) for full status and [`CHANGELOG.md`](CHANGELOG.md) for dated history.

## Usage

- **H** opens or closes AMI by default. `Open Medical Interface` is remappable in Controls.
- Controller default: hold **R1/RB** and tap **D-Pad Right**. **LT/RT** cycles the five medical pages.
- **Toggle Patient** opens a list containing yourself, patients within 5 m, and every occupant of your vehicle.
- Body region selected on the shared 2D body model.
- **Escape**, **B/Circle**, the mapped open action, or the close button closes AMI.
- Foreign treatment uses the shared 5 m range; occupants of the same vehicle remain valid regardless of distance.

## Pages

- **Triage** — server-shared triage level (None/Minimal/Delayed/Immediate/Expectant) with button-color feedback, plus a scrollable per-patient journal retaining the latest 50 triage changes, medications, and treatments.
- **Diagnose** — patient name, toggleable heart-rate and blood-pressure monitors polling ACE's 1 Hz vitals, animated rhythm line and 30-second BP history chart; monitoring persists across page switches.
- **Bandages & Fractures** — per-zone bleeding/fracture visuals, wound icons, direct Bandage/Tourniquet/Medical Kit actions using inventory items via ACE's consumable flow.
- **Medication** — self/patient inventory grids for epinephrine, morphine, naloxone, phenylephrine, metoprolol, ammonium carbonate, and saline. The Saline submenu order is 500 ml, vanilla US/USSR 750 ml, 1000 ml, and 1500 ml. Active saline uses a bright-yellow body indicator.
- **Advanced/Misc** — CPR, carry, drag, back position, left/right recovery position, load-into-vehicle, using registered ACE/engine actions and availability checks.
- **Toggle Patient** — selects patients by name, then offers `TREAT` and, for unconscious vehicle occupants, `UNLOAD FROM VEHICLE`.

## Architecture

- `Scripts/Game/RAMI/RAMI_GameMode.c` — `modded class SCR_BaseGameMode` registers the input listener and body-zone snapshot loop in every gamemode; no prefab or manual world placement needed.
- `UI/layouts/` — `RAMI_MedicalGlobal.layout` (navigation, ContentArea) plus one layout per page.
- AMI stores no medical truth locally beyond a frozen visual body-zone snapshot for display after death; ACE remains the source of health, bleeding, medication, and treatment state.

## License

Advanced Medical Interface is licensed under the GNU General Public License version 2. Page-tab icons are adapted from ACE3 artwork under the same license. See [`license.txt`](license.txt) and [`THIRD_PARTY_NOTICES.md`](THIRD_PARTY_NOTICES.md).
