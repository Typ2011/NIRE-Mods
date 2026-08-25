Workshop Summary (short, 173/200 chars)

Alpha ACE Medical UI: patient overview, triage, diagnostics, wound treatment, medication, CPR/carry in one menu. Works in multiplayer; performance under load not yet tested.

Workshop Description (full, plain text, no markdown)

Advanced Medical Interface (AMI) - Alpha

AMI is a UI overlay for ACE Medical. It does not replace or duplicate ACE's simulation, replication, or treatment logic - it reads ACE's patient data and triggers ACE's own actions through one consolidated menu.

This mod is in alpha. Core functionality works, including in multiplayer, but performance under real server load (many players, many patients, sustained sessions) has not been tested yet. You may see a performance decrease; please report it.

Requires: ACE Medical Circulation Dev, ACE Medical Hitzones Dev, ACE Carrying Dev, and Expanded Saline Bags. ACE Medical Core Dev and ACE Core Dev are transitive dependencies.

Usage: Press H to open or close AMI. Open Medical Interface is remappable in Controls. On controller, hold R1/RB and tap D-Pad Right; LT/RT cycles pages. Escape, B/Circle, or the close button also closes the menu.

Toggle Patient opens a list containing yourself, patients within 5 m, and every occupant of your vehicle. Select a name, then choose TREAT. UNLOAD FROM VEHICLE is available for unconscious occupants.

Pages:

Triage - shared triage level (None/Minimal/Delayed/Immediate/Expectant) plus a per-patient activity log.
Diagnose - live heart-rate and blood-pressure monitors with rhythm line and BP history chart.
Bandages & Fractures - per-zone bleeding/fracture display with direct Bandage, Tourniquet, and Medical Kit actions.
Medication - inventory-driven controls for epinephrine, morphine, naloxone, phenylephrine, metoprolol, ammonium carbonate, and saline, with dose timers. The Saline submenu order is 500 ml, vanilla US/USSR 750 ml, 1000 ml, and 1500 ml. Active saline has a bright-yellow body indicator.
Advanced/Misc - CPR, carry, drag, recovery positions, and vehicle loading.
Toggle Patient - choose yourself, a nearby patient, or a fellow vehicle occupant.

Known limitations: no localization yet, no dedicated performance/regression pass. Feedback on multiplayer stability and performance is welcome.

License and credits:
AMI is licensed under the GNU General Public License version 2. Page-tab icons are adapted from ACE3 medical-menu artwork, copyright ACE3 contributors, under GPLv2. ACE3 source: https://github.com/acemod/ACE3
