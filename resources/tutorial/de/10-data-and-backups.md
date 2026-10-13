# Ihre Daten und Sicherungen

Der Kalender liegt in Ihrem Home-Verzeichnis unter `~/.local/share/stapikcalendar/`:

- `calendar.json` — Ihr Kalender
- `calendar.json.bak`, `.bak.2`, `.bak.3` — die drei vorherigen Versionen, bei jedem Speichern aufgefrischt
- `cloud-baseline.json` — technische Daten zur letzten Synchronisierung

Sprache, Thema, Schriftgröße und Fenstergröße werden in `~/.config/stapikcalendar/settings.json` gespeichert. Die Cloud-Adresse steht in `~/.config/stapikcalendar/config.json`, der API-Schlüssel im System-Schlüsselbund.

Lässt sich der Kalender nicht auf der Festplatte speichern (zum Beispiel weil sie voll ist), zeigt die Anwendung eine Meldung mit der Ursache. Beheben Sie das Problem und nehmen Sie irgendeine Änderung vor: Der gesamte Kalender wird erneut gespeichert.

## Eine Sicherung wiederherstellen

Wählen Sie **Datei → Aus Sicherung wiederherstellen…**, suchen Sie eine der gespeicherten Kopien nach Datum und Uhrzeit aus und bestätigen Sie. Der bisherige Stand wird zur neuesten Kopie, die Wiederherstellung lässt sich also auf demselben Weg zurücknehmen. Der Rückgängig-Verlauf wird geleert, und bei bestehender Verbindung zu Stapik Cloud wird der wiederhergestellte Kalender wie jede andere Änderung dorthin gesendet.

Startet die Anwendung nicht oder ist die Kalenderdatei beschädigt, können Sie auch von Hand wiederherstellen:

1. Schließen Sie die Anwendung.
2. Kopieren Sie `calendar.json.bak` (oder `.bak.2`, `.bak.3`) über `calendar.json`.
3. Starten Sie die Anwendung erneut.

## Den Kalender auf einen anderen Computer übertragen

Kopieren Sie `calendar.json` in denselben Ordner auf dem anderen Computer. Mit Stapik Cloud können Sie stattdessen den neuen Computer mit demselben Server verbinden, und der Kalender wird automatisch geladen.
