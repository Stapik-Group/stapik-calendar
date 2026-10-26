# Update

## Auf Version 1.4.0

- Wenn Sie Ende-zu-Ende-Verschlüsselung verwenden, **aktualisieren Sie alle Geräte, bevor der Kalender eines Jahres groß wird** (ein Dokument ab 32 KiB). Größere Dokumente werden in einem neueren Verschlüsselungsformat gespeichert, das Versionen vor 1.4.0 nicht lesen können: Sie melden es als Daten einer neueren Version der Anwendung, statt das Jahr anzuzeigen.
- Beim Beenden wartet die Anwendung jetzt bis zu 3 Sekunden, um die letzten Änderungen an die Cloud zu senden, bevor das Fenster verschwindet.

## Von Version 1.1.x

- Beim ersten Start von Version 1.2.0 wird Ihr Kalender automatisch in das neue Format umgewandelt. Die alte Datei bleibt als `calendar.json.bak` erhalten.
- Auch das in der Cloud gespeicherte Format hat sich geändert. **Aktualisieren Sie alle Geräte, bevor Sie sie erneut mit derselben Cloud verbinden**: Ein Gerät mit 1.1.x kann das neue Format nicht lesen und könnte Ihren Kalender überschreiben.
- Ein von Version 1.1.x in die Cloud geschriebener Kalender wird korrekt gelesen und bei der nächsten Synchronisierung durch das neue Format ersetzt.
- Wenn Sie eine Version mit einem einzigen gemeinsamen API-Schlüssel verwendet haben, verbinden Sie sich erneut mit einem für dieses Gerät ausgestellten Schlüssel (**Datei → Mit Cloud verbinden**).
