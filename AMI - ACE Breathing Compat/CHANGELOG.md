# Changelog

Alle nennenswerten Änderungen an **AMI ACE Breathing Compat** werden hier dokumentiert.

## Unreleased

### Added

- Lizenzdatei mit GNU General Public License Version 2 analog zu AMI.
- Workshop-Thumbnail im 16:9-Format für `AMI ACE Breathing Compat`.
- Workbench-Projekt `AMI ACE Breathing Compat` mit Workbench-erzeugter GUID `6A2CBB1E9543DD6F`.
- Abhängigkeiten auf Arma Reforger, AMI und ACE Medical Breathing Dev.
- Gemeinsames Laden aller drei Projekte in Workbench geprüft.
- Repository-Regeln für strikte Trennung zwischen AMI und ACE Medical Breathing.
- Modplan für eigenes Compat-Projekt, Abhängigkeiten, Umsetzung und Abnahme.
- Breathing-Kategorie im Treatment-Fenster mit Chest Seal, NCD Kit, King LT und Oxygen Mask.
- Inventarzähler, Item-Vorschau, Zustandsprüfung, Controller-Navigation und Anwendung über AMIs vorhandenen Behandlungsablauf.
- Airway-Aktionen für Kopf anheben und Erbrochenes entfernen im Actions-Menü.
- Respiratory Rate und Sauerstoffsättigung als überwachte Felder im verkleinerten Diagnose-Bereich.
- Gelbe Statussymbole rechts der Körpergrafik für Atemwegsverschluss und Pneumothorax.
- King LT und Oxygen Mask sind ausschließlich am Kopf anwendbar.
- Automatischer Compat-Test für Projektname, Abhängigkeiten, Breathing-Typen und AMI-Trennung.

### Changed

- Mod in `AMI ACE Breathing Compat` umbenannt.
- Compat-Umsetzung abgeschlossen und per Workbench-PC-Scriptvalidierung geprüft.
- Atemwegs- und Pneumothoraxsymbole stehen nebeneinander direkt am Kopf.
- Respiratory-Rate- und Oxygen-Saturation-Schaltflächen verwenden dieselbe aktive Farbe wie die anderen Diagnose-Schaltflächen.
