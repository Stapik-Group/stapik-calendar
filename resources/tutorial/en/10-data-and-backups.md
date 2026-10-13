# Your data and backups

The calendar lives in your home directory, in `~/.local/share/stapikcalendar/`:

- `calendar.json` — your calendar
- `calendar.json.bak`, `.bak.2`, `.bak.3` — the three previous versions, replaced every time you save
- `config.json` — cloud address and key
- `cloud-baseline.json` — technical data about the last synchronization

Your language, theme, font size and window size are stored in `~/.config/stapikcalendar/settings.json`. The cloud address is in `~/.config/stapikcalendar/config.json`; the API key is kept in the system keyring.

If the calendar cannot be saved to disk (for example because the disk is full), the application shows a message with the reason. Fix the problem and make any change: the whole calendar is saved again.

## Restoring a backup

Choose **File → Restore from backup…**, pick one of the saved copies by its date and time and confirm. The calendar you had before becomes the newest copy, so you can go back the same way. The undo history is cleared and, when you are connected to Stapik Cloud, the restored calendar is sent there like any other change.

If the application does not start or the calendar file is damaged, you can restore by hand instead:

1. Close the application.
2. Copy `calendar.json.bak` (or `.bak.2`, `.bak.3`) over `calendar.json`.
3. Start the application again.

## Moving the calendar to another computer

Copy `calendar.json` into the same folder on the other computer. With Stapik Cloud you can instead connect the new computer to the same server and the calendar will download by itself.
