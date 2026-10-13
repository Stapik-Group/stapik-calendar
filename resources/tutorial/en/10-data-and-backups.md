# Your data and backups

The calendar lives in your home directory, in `~/.local/share/stapikcalendar/`:

- `calendar-2026.json`, `calendar-2027.json`… — your calendar, one file per year (the old `calendar.json` was split into years and stays as `calendar.json.migrated`)
- `calendar-2026.json.bak`, `.bak.2`, `.bak.3` — the three previous versions of that year, replaced every time that year is saved
- `config.json` — cloud address and key
- `cloud-baseline.json` — technical data about the last synchronization

Your language, theme, font and window size are stored in `~/.config/stapikcalendar/settings.json`. The cloud address is in `~/.config/stapikcalendar/config.json`; the API key is kept in the system keyring.

If the calendar cannot be saved to disk (for example because the disk is full), the application shows a message with the reason. Fix the problem and make any change: the whole calendar is saved again.

## Restoring a backup

Backups are kept for each year separately. Go to the year you want to restore, choose **File → Restore from backup…**, pick one of the saved copies by its date and time and confirm. Only that year is restored; the rest of the calendar does not change. The year you had before becomes the newest copy, so you can go back the same way. The undo history is cleared and, when you are connected to Stapik Cloud, the restored calendar is sent there like any other change.

If the application does not start or the calendar file is damaged, you can restore by hand instead:

1. Close the application.
2. Copy `calendar-2026.json.bak` (or `.bak.2`, `.bak.3`) over `calendar-2026.json` (use the right year).
3. Start the application again.

## Moving the calendar to another computer

Copy the `calendar-*.json` files into the same folder on the other computer. With Stapik Cloud you can instead connect the new computer to the same server and the calendar will download by itself.
