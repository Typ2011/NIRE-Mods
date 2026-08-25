# Changelog

Alle relevanten Änderungen an DisableGlobalChat werden hier dokumentiert.

## Unreleased

### Known issues

- Erweiterte Mehrspieler-Tests für alle Kanäle, Rollenwechsel, Join-in-progress und manipulierte Clients bleiben offen.

### Added

- Native String-Table-Lokalisierung für alle 13 von Arma Reforger unterstützten Sprachen; Chat-Rückmeldungen werden auf dem jeweiligen Client übersetzt.
- Neues Workshop-Thumbnail in 1920×1080 mit klarer Darstellung der standardmäßigen Gruppenchat-Freigabe.
- Serverautoritativ ausgewertete `#chat`-Befehle für Game Master und Server-Admins.
- `#chat <Kanal>` schaltet Global-, Fraktions-, Gruppen-, lokalen, Fahrzeug- oder Direktchat für normale Spieler um.
- `#chat status`, `#chat all` und `#chat none` zeigen den Zustand beziehungsweise schalten alle Kanäle gemeinsam.
- Autoritative Befehlsbestätigung und Ablehnung gesperrter Nachrichten als private Chat-Rückmeldung an den Absender.
- Kanalbezogene native Chat-Hilfe beim normalen Sendeversuch in einem gesperrten Kanal.
- Empfangsfilter für gesperrte allgemeine und direkte Nachrichten.
- Statischer Hook-Test für Server-, Empfangs- und UI-Schutz.

### Changed

- Dynamische GameMaster-Mehrfachauswahl vollständig durch `#chat`-Befehle ersetzt.
- Normale Spieler folgen weiterhin der replizierten Server-Kanalmaske; standardmäßig bleibt nur Gruppenchat aktiv.
- GameMaster und Server-Admins behalten unabhängig von der Auswahl Vollzugriff.
- `#login` und Systemkanal-Rückmeldungen bleiben nutzbar.
- Sende-, Empfangs- und UI-Filter verwenden gemeinsam `DGC_GameMasterAccess`.
- Gruppenchat prüft serverseitig weiterhin eine gültige Gruppenmitgliedschaft.

### Fixed

- Freigabe, Sperre, Status und Befehlsfehler erscheinen zuverlässig als private Systemmeldung im Chat des ausführenden Game Masters oder Admins.
- Der native Global-Kanal wird über `BaseChatChannel` und den Namen `Global` erkannt; die UI hebt ihre eigene Sperre nach der serverseitigen Freigabe jetzt tatsächlich auf.
- `#chat` nutzt nun den bereits replizierten `SCR_PlayerControllerGroupComponent`, dessen Server-RPC-Pfad vom Basisspiel für Spieleranfragen verwendet wird.
- Nicht berechtigte serverseitige `#chat`-Anfragen erhalten eine private Ablehnung statt eines stillen Abbruchs.
- Listen-Server führen lokale `#chat`-Anfragen direkt serverseitig aus, statt einen RPC an denselben Server zu senden.
- `#chat`-Befehle hängen nicht mehr von konfigurierten Chatkanal-Instanzen ab.
- Gesperrte Nachrichten werden nicht mehr nur still serverseitig verworfen; der Absender erhält eine Rückmeldung.
- UI-Hinweise verwenden den nativen `SCR_ChatPanelManager.ShowHelpMessage`-Pfad statt des Debug-Hint-Pfads.
- Befehle werden unabhängig vom aktuell ausgewählten Chatkanal zum Server gesendet und dort gegen die aktuelle GameMaster-/Admin-Rolle geprüft.
- Enforce-Live-Reload-Syntaxfehler durch Ersetzen des ternären Fallbacks in `IsChannelAllowed`.
- Admin-Anmeldung wird nicht mehr durch die Textchat-Sperre blockiert.

### Validation

- Projekt-, Ressourcen- und Sprachreferenzprüfung für alle 13 Sprachen bestanden.
- Automatische clientseitige Übersetzung der Chat-Rückmeldungen im Spiel vom Nutzer bestätigt.
- Workbench-Skriptkompilierung unter Arma Reforger 1.8.0.10 ohne Mod-Fehler bestanden.
- Statischer Test `Tests\DGC_DisableGlobalChat.test.ps1` bestanden.
- Dedizierter Server: Global-Chat-Freigabe, erneute Sperre und sichtbare Befehlsbestätigung vom Nutzer bestätigt.
- Vollständiger Mehrspieler-Kanaltest bleibt offen.
