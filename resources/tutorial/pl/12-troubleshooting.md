# Rozwiązywanie problemów

**Kliknięcie symbolu linku nie otwiera strony.**

Otwierane są tylko linki `http` i `https`, a system musi mieć ustawioną domyślną przeglądarkę.

**Prawy przycisk na dniu nic nie robi.**

W schowku musi być pełny adres strony zaczynający się od `http://` lub `https://`.

**Stan to „Offline”.**

Sprawdź połączenie z internetem i adres serwera (**Plik → Połącz z chmurą**). Aplikacja ponawia próby sama, a **Plik → Synchronizuj** próbuje od razu.

**Stan to „Błąd synchronizacji” albo łączenie się nie udaje.**

Sprawdź, czy adres i klucz API są poprawne oraz czy klucz został wystawiony dla tego urządzenia i nie został unieważniony w panelu administracyjnym.

**Moje zmiany zniknęły po synchronizacji.**

Prawdopodobnie widziałeś *Konflikt: zachowano wersję z serwera*: inne urządzenie zapisało nowszą wersję. Przywróć zastąpioną wersję z historii dokumentu w panelu administracyjnym Stapik Cloud (zobacz [Konflikty](#cloud)) albo przywróć lokalną kopię zapasową (zobacz [Twoje dane](#data-and-backups)).

**Aplikacja uruchamia się z pustym kalendarzem.**

Nie udało się odczytać pliku kalendarza. Poszukaj w `~/.local/share/stapikcalendar/` plików o nazwach `calendar-YYYY.json.corrupt-…` lub `calendar-YYYY.json.unreadable-…` (kopia nieczytelnego pliku) oraz kopii zapasowych `calendar-YYYY.json.bak`; przywróć jedną z nich zgodnie z opisem w rozdziale [Twoje dane i kopie zapasowe](#data-and-backups).
