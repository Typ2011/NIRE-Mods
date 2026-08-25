# Changelog

Alle nennenswerten Änderungen an LogisticMaster werden hier dokumentiert.

## Unreleased

### Added

- InventoryBoxes (`A1E6470C8DBF3952`) als Mod-Abhängigkeit und nahe InventoryBoxes-Lager als gemeinsame Bestandsquellen.
- Serverseitig geschützte Bestandsbearbeitung für Game Master und manuell freigegebene Spieler mit Prefab- und Mengenfeld sowie Hinzufügen/Entfernen-Aktionen.
- Game-Master-Seite zur Auswahl und Freigabe aktuell verbundener Spieler.
- Mike's UI (`B3F91C6A4E275D08`) als Mod-Abhängigkeit.
- Eigene Kategorie `LogisticMaster` im Steuerungsmenü mit der Aktion `LogisticMaster öffnen`.
- Registrierte Enfusion-Metadaten für Inputkonfiguration, Steuerungsmenü und Terminal-Backend-Prefab.
- Globale LogisticMaster-UI standardmäßig auf `F6`, mit `Esc` zum Schließen und frei umbindbarer Öffnen-Aktion.
- Client- und serverseitige Zugriffskontrolle für Gruppenführer, Logistics-/Support-Gruppen und Game Master.
- `AGENTS.md` mit verbindlichen Regeln für Ponytail `full`, Enfusion-MCP-Prüfungen und Projektdokumentation.
- `changelog.md` als zentrale Änderungshistorie.
- `modplan.md` mit Ziel, Arbeitsphasen und Abnahmekriterien der Mod.
- Konkreter MVP-Plan für zentrales Logistikterminal, Bestandsanzeige, Anforderungen, Rollen und serverseitige Ausgabe.
- `LogisticMaster.gproj` mit Arma-Reforger-Abhängigkeit sowie PC- und Headless-Konfiguration.
- Erste responsive Terminal-UI mit Bestand, Anforderungsformular, Auftragsliste und Logistiker-Aktionen.
- Grundlegendes Anforderungsmodell mit Fraktion, Gegenstand, Menge, Beteiligten und Status.
- Physisches Terminal-Prefab auf Basis einer Vanilla-Supply-Crate mit Inventar, Replikation und Benutzeraktion.
- Serverseitiger Anfrageablauf für Erstellen, Bestätigen, Ablehnen und Abschließen.
- Fraktions-, Distanz- und Rollenprüfung für Gruppenführer sowie Logistics-/Support-Gruppen.
- Fraktionsgebundene RPC-Synchronisierung der Auftragsliste.
- Atomare serverseitige Bestandsprüfung mit anschließender Inventarabbuchung.
- Interaktive Bestands- und Auftragslisten im Terminal-UI.
- Konfigurierbarer Umgebungsscan für verfügbare Vanilla-Supplies über nahe `SCR_ResourceComponent`-Container.

### Fixed

- Game Master werden in der Berechtigungsliste sofort als automatisch berechtigt angezeigt; ihr nicht entziehbarer Eintrag ist gesperrt und wartet nicht mehr auf den Client-Snapshot.
- Seitenreiter schalten ihre Zielseite nun über direkte MUI-Button-Ereignisse um, sodass Inhalt und aktive Hervorhebung zuverlässig wechseln.

- `F6` öffnet LogisticMaster nun auch direkt in der aktiven Game-Master-Ansicht.
- Game Master besitzen nun immer vollständige LogisticMaster-Rechte und können weder über die Freigabeliste noch durch fehlende Client-Freigabe ausgesperrt werden.
- `F6` reagiert erst beim Loslassen; der beim Menüwechsel aktivierte `MenuContext` löst dadurch kein sofortiges Schließen mehr aus.
- Das Terminal läuft als natives Enfusion-`MenuBase`; dadurch ist die Maus frei und die Charaktersteuerung bleibt während der Bedienung gesperrt. `F6` schließt es weiterhin.
- Globales Menü erzeugt bei fehlender Platzierung automatisch das replizierte Terminal-Backend; `F6` endet dadurch nicht mehr wegen einer leeren Terminal-Instanz.
- Ungültige `Visible`-Eigenschaften von `FrameWidgetClass` entfernt; die Seitensichtbarkeit wird ausschließlich über `ShowPage()` gesetzt.

### Changed

- Gegenstandszeilen im Bestand von 42 auf 34 Pixel verdichtet, einschließlich angepasster Mindesthöhe.

- Bestand, Anfordern und Aufträge sind über direkte Reiter erreichbar; der vierte Reiter `Berechtigungen` wird ausschließlich Game Mastern angezeigt.
- Gegenstandsauswahl nach InventoryBoxes-Vorbild um Arsenal-Kategorien ergänzt und die zyklische Seitentaste entfernt.
- Manuelle Prefab-Pfade bei der Bestandsbearbeitung durch eine durchsuchbare Auswahl aus Reforgers Arsenal-Katalog ersetzt; der Server akzeptiert nur Katalogeinträge.
- Automatische Rechte aus Gruppenrolle und Serveradmin-Status durch sitzungsgebundene manuelle Spielerfreigaben ersetzt; Game Master bleiben automatisch vollständig berechtigt.
- Mike's-UI-Repository als Referenz in `AGENTS.md` ergänzt.
- `AGENTS.md` um LogisticMaster-spezifische Enforce-Struktur-, Stil-, Kontext- und Prüfregeln ergänzt.
- Terminal-UI auf Mike's UI umgestellt; Layout, Navigation, Listen, Eingabe und Aktionen werden nun über dessen Runtime und Komponenten erzeugt.
- Bestand, Anforderung und Aufträge auf drei getrennte Seiten verteilt; eine dauerhaft sichtbare Taste schaltet zyklisch zur nächsten Seite.
- LogisticMaster-UI als klar gegliedertes Einsatzterminal mit Kopfbereich, Bestands-, Anforderungs- und Auftragskarten sowie rollenbezogenen Hinweisen neu gestaltet.
- Input- und Steuerungsmenü-Konfiguration verwenden jetzt die Vanilla-Ressourcen-GUIDs als echte Enfusion-Overrides; dadurch werden Aktion und eigener Reiter geladen.
- Öffnen-Aktion zusätzlich im `InGameMenuContext` registriert, damit Game Master auch ohne aktive Charaktersteuerung zugreifen können.
- Physische Terminal-Interaktion durch einen rollenbeschränkten UI-Einstieg ersetzt; das Prefab bleibt nur als replizierter Lager- und Serverzustandsträger erhalten.
- Modplan um Kernablauf, Berechtigungen, Datenmodell, Multiplayer-Schutz und detaillierte Umsetzungsphasen erweitert.
- Terminal-Layout auf gültige Enfusion-Slot-Syntax und auswählbare Listen umgestellt.
- Veraltete Inventarabfrage durch `InventoryStorageManagerComponent.GetItems()` ersetzt.
- Logistikerberechtigung an native Logistics-/Support-Gruppenrollen gebunden.
- Projektstand vom 10.08.2026 mit verifiziertem Stand und offenen Punkten dokumentiert.

### Verified

- Berechtigungsanzeige, direkte Seitenreiter und kompakte Gegenstandszeilen über Enfusion MCP validiert, mit Mike's UI und InventoryBoxes fehlerfrei als Game-Scripts kompiliert und in `Test_LM.ent` gestartet.

- Seitenreiter, Game-Master-Berechtigungsreiter, InventoryBoxes-Kategorien und gemeinsame Lagerquellen zusammen über Enfusion MCP fehlerfrei kompiliert; Game-Master-Testlevel und F6-UI gestartet.
- Arsenal-Katalog, Suchauswahl, serverseitige Katalogprüfung und Game-Master-Testlevel über Enfusion MCP fehlerfrei kompiliert und gestartet.
- Game-Master-Eingabekontext, Scriptkompilierung und Testlevel-Start über Enfusion MCP geprüft.
- Manuelle Freigabeliste, Game-Master-Spielerauswahl, Berechtigungs-RPCs und Disconnect-Cleanup über Enfusion MCP fehlerfrei kompiliert.
- Rollen-Gate, Bestands-RPC und Mike's-UI-Bearbeitungsfelder über Enfusion MCP fehlerfrei kompiliert; Game-Master-Testlevel geladen.
- Mike's UI als Abhängigkeit aufgelöst, LogisticMaster-Scripts fehlerfrei kompiliert und Game-Master-Testlevel über Enfusion MCP gestartet.
- Enfusion lädt `LM_ToggleLogisticsMaster` in Charakter- und Ingame-Menükontext; derselbe Aktions-Callback öffnet die UI im Game-Master-Testlevel ohne Script- oder GUI-Fehler.
- Globalen UI-Einstieg, rollenbasierte Prüfungen, Scriptkompilierung und Testlevel-Start nach Workbench-Neustart über Enfusion MCP geprüft.
- Enfusion-Scripts ohne LogisticMaster-Fehler kompiliert.
- Terminal-Prefab geladen und Play-Modus gestartet.
- Alle Enfusion-MCP-Projektprüfungen bestanden.
