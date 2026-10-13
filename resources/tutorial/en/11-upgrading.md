# Upgrading from version 1.1.x

- The first start of version 1.2.0 converts your calendar to the new format automatically. The old file is kept as `calendar.json.bak`.
- The format stored in the cloud has changed too. **Upgrade every device before you connect them to the same cloud again**: a device still running 1.1.x cannot read the new format and could overwrite your calendar.
- A calendar written to the cloud by version 1.1.x is read correctly and is replaced with the new format at the next synchronization.
- If you used a version that had a single shared API key, connect again with a key issued for this device (**File → Connect to cloud**).
