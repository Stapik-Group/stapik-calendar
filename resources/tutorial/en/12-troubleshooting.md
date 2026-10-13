# Troubleshooting

**Clicking an entry does not open a page.**

Check that the entry has a link. Only `http` and `https` links are opened, and your system needs a default web browser.

**A double-click opens the page instead of the editor.**

Click a little faster, or make the double-click time longer in your system's mouse settings.

**Right-clicking a day does nothing.**

The clipboard must contain a complete web address starting with `http://` or `https://`.

**The status is “Offline”.**

Check your internet connection and the server address (**File → Connect to cloud**). The application retries by itself, and **File → Sync** tries immediately.

**The status is “Sync error”, or connecting fails.**

Check that the address and the API key are correct and that the key was issued for this device and has not been revoked in the admin panel.

**My changes disappeared after a synchronization.**

You probably saw *Conflict: server version kept*: another device saved a newer version. Restore the replaced version from the document history in the Stapik Cloud admin panel (see [Conflicts](#cloud)), or restore a local backup (see [Your data](#data-and-backups)).

**The application starts with an empty calendar.**

The calendar file could not be read. Look in `~/.local/share/stapikcalendar/` for files named `calendar-YYYY.json.corrupt-…` or `calendar-YYYY.json.unreadable-…` (a copy of the unreadable file) and for the backups `calendar-YYYY.json.bak`; restore one as described in [Your data and backups](#data-and-backups).
