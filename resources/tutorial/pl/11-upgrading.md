# Aktualizacja

## Do wersji 1.4.0

- Jeśli używasz szyfrowania end-to-end, **zaktualizuj wszystkie urządzenia, zanim kalendarz jednego roku urośnie** (dokument od 32 KiB). Większe dokumenty są zapisywane w nowszym formacie szyfrowania, którego wersje starsze niż 1.4.0 nie odczytają: zgłoszą go jako dane z nowszej wersji aplikacji zamiast pokazać ten rok.
- Przy zamykaniu aplikacja czeka teraz do 3 sekund na wysłanie ostatnich zmian do chmury, zanim okno zniknie.

## Z wersji 1.1.x

- Pierwsze uruchomienie wersji 1.2.0 automatycznie przekształca kalendarz do nowego formatu. Stary plik zostaje zachowany jako `calendar.json.bak`.
- Format przechowywany w chmurze też się zmienił. **Zaktualizuj wszystkie urządzenia, zanim ponownie połączysz je z tą samą chmurą**: urządzenie z wersją 1.1.x nie odczyta nowego formatu i mogłoby nadpisać Twój kalendarz.
- Kalendarz zapisany w chmurze przez wersję 1.1.x jest odczytywany poprawnie i zostanie zastąpiony nowym formatem przy następnej synchronizacji.
- Jeśli używałeś wersji z jednym wspólnym kluczem API, połącz się ponownie kluczem wystawionym dla tego urządzenia (**Plik → Połącz z chmurą**).
