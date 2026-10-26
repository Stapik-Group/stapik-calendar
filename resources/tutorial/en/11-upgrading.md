# Upgrading

## To version 1.4.0

- If you use end-to-end encryption, **upgrade every device before the calendar of a year grows large** (a document of 32 KiB or more). Larger documents are stored in a newer encrypted format that versions older than 1.4.0 cannot read: they report it as data from a newer version of the application instead of showing the year.
- When you close the application, it now waits up to 3 seconds to send the last changes to the cloud before the window disappears.

## From version 1.1.x

- The first start of version 1.2.0 converts your calendar to the new format automatically. The old file is kept as `calendar.json.bak`.
- The format stored in the cloud has changed too. **Upgrade every device before you connect them to the same cloud again**: a device still running 1.1.x cannot read the new format and could overwrite your calendar.
- A calendar written to the cloud by version 1.1.x is read correctly and is replaced with the new format at the next synchronization.
- If you used a version that had a single shared API key, connect again with a key issued for this device (**File → Connect to cloud**).
