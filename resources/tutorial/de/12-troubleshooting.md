# Problemlösung

**Ein Klick auf einen Eintrag öffnet keine Seite.**

Prüfen Sie, ob der Eintrag einen Link hat. Es werden nur `http`- und `https`-Links geöffnet, und Ihr System braucht einen Standard-Webbrowser.

**Ein Doppelklick öffnet die Seite statt des Editors.**

Klicken Sie etwas schneller oder verlängern Sie die Doppelklickzeit in den Mauseinstellungen Ihres Systems.

**Ein Rechtsklick auf einen Tag bewirkt nichts.**

Die Zwischenablage muss eine vollständige Webadresse enthalten, die mit `http://` oder `https://` beginnt.

**Der Status ist „Offline“.**

Prüfen Sie Ihre Internetverbindung und die Serveradresse (**Datei → Mit Cloud verbinden**). Die Anwendung versucht es selbstständig erneut, und **Datei → Synchronisieren** versucht es sofort.

**Der Status ist „Synchronisierungsfehler“, oder das Verbinden schlägt fehl.**

Prüfen Sie, ob Adresse und API-Schlüssel stimmen und ob der Schlüssel für dieses Gerät ausgestellt und im Admin-Panel nicht widerrufen wurde.

**Meine Änderungen sind nach einer Synchronisierung verschwunden.**

Sie haben wahrscheinlich *Konflikt: Serverversion beibehalten* gesehen: Ein anderes Gerät hat eine neuere Version gespeichert. Stellen Sie die ersetzte Version aus dem Dokumentverlauf im Stapik-Cloud-Admin-Panel wieder her (siehe [Konflikte](#cloud)) oder spielen Sie eine lokale Sicherung ein (siehe [Ihre Daten](#data-and-backups)).

**Die Anwendung startet mit einem leeren Kalender.**

Die Kalenderdatei konnte nicht gelesen werden. Suchen Sie in `~/.local/share/stapikcalendar/` nach Dateien mit den Namen `calendar-YYYY.json.corrupt-…` oder `calendar-YYYY.json.unreadable-…` (eine Kopie der unlesbaren Datei) sowie nach den Sicherungen `calendar-YYYY.json.bak`; stellen Sie eine davon wie im Abschnitt [Ihre Daten und Sicherungen](#data-and-backups) beschrieben wieder her.
