# Twoje dane i kopie zapasowe

Kalendarz znajduje się w Twoim katalogu domowym, w `~/.local/share/stapikcalendar/`:

- `calendar-2026.json`, `calendar-2027.json`… — Twój kalendarz, osobny plik dla każdego roku (stary `calendar.json` został podzielony na lata i zostaje jako `calendar.json.migrated`)
- `calendar-2026.json.bak`, `.bak.2`, `.bak.3` — trzy poprzednie wersje danego roku, odświeżane przy każdym zapisie tego roku
- `config.json` — adres i klucz chmury
- `cloud-baseline.json` — dane techniczne o ostatniej synchronizacji

Język, motyw, czcionka i rozmiar okna są zapisane w `~/.config/stapikcalendar/settings.json`. Adres chmury jest w `~/.config/stapikcalendar/config.json`, a klucz API w systemowym pęku kluczy.

Jeśli kalendarza nie da się zapisać na dysku (na przykład przez brak miejsca), aplikacja pokazuje komunikat z przyczyną. Usuń problem i wprowadź dowolną zmianę: cały kalendarz zostanie zapisany ponownie.

## Przywracanie kopii zapasowej

Kopie zapasowe są osobne dla każdego roku. Przejdź do roku, który chcesz przywrócić, wybierz **Plik → Przywróć z kopii zapasowej…**, wskaż jedną z zapisanych kopii według daty i godziny i potwierdź. Przywracany jest tylko ten rok; reszta kalendarza się nie zmienia. To, co było wcześniej, staje się najnowszą kopią, więc przywrócenie można cofnąć w ten sam sposób. Historia cofania jest czyszczona, a po połączeniu ze Stapik Cloud przywrócony kalendarz jest tam wysyłany jak każda inna zmiana.

Jeśli aplikacja nie startuje albo plik kalendarza jest uszkodzony, możesz przywrócić kopię ręcznie:

1. Zamknij aplikację.
2. Skopiuj `calendar-2026.json.bak` (albo `.bak.2`, `.bak.3`) na `calendar-2026.json` (podaj właściwy rok).
3. Uruchom aplikację ponownie.

## Przenoszenie kalendarza na inny komputer

Skopiuj pliki `calendar-*.json` do tego samego katalogu na drugim komputerze. Ze Stapik Cloud możesz zamiast tego połączyć nowy komputer z tym samym serwerem, a kalendarz pobierze się sam.
