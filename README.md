# Stapik Calendar

A desktop calendar application for Linux, written in C++20 using GTK4/gtkmm. Supports multiple visual themes, from a retro look to a modern one.

![Screenshot](screenshots/screenshot_1.png)

## Features

- **Monthly view** - calendar grid with month and year navigation
- **Calendar entries** - add, edit and delete entries with a name, optional link and color
- **Entry colors** - assign one of several predefined colors to an entry, either from the entry dialog or via a quick right-click popover on an existing entry
- **Drag and drop** - drag an entry to another day to move it; hold **Ctrl** while dragging to copy it instead
- **Quick add from clipboard** - right-clicking an empty cell automatically fetches the page title from a URL copied to the clipboard and creates an entry
- **Undo/Redo** - full operation history for adding, editing, deleting, moving and copying entries
- **Cloud sync** - save and load data via an external API (compatible with a self-hosted server), with automatic conflict resolution based on timestamps
- **Multilingual UI** - Polish, English and German interface with instant switching
- **Themes** - switch between **Classic** (retro Win98), **Modern** (light, rounded, macOS-inspired) and **Classic Pink** (Win98 layout with a vaporwave pink/purple/cyan palette) from **Settings → Theme**; the choice is remembered between launches
- **Auto-save** - calendar data saved locally after every change

## Dependencies

- `gtkmm-4.0`
- `libcurl`
- [`stapik-common`](https://github.com/stapik/stapik-common) (fetched automatically via CMake FetchContent)
- `nlohmann/json` (fetched automatically via CMake FetchContent, transitively provided by `stapik-common`)

On Ubuntu/Debian:
```bash
sudo apt install libgtkmm-4.0-dev libcurl4-openssl-dev
```

Building a `.deb` package additionally requires `dpkg-dev` (used to auto-detect runtime dependencies):
```bash
sudo apt install dpkg-dev
```

## Building

```bash
git clone https://github.com/Stapik-Group/stapik-calendar
cd stapik-calendar
cmake -B cmake-build-release -DCMAKE_BUILD_TYPE=Release
cmake --build cmake-build-release
```

## Installation

### Option 1 — Download prebuilt `.deb` (recommended)

Download the latest `.deb` package from the [Releases page](https://github.com/Stapik-Group/stapik-calendar/releases), then install it:

```bash
sudo dpkg -i stapikcalendar_*.deb
sudo apt install -f   # resolves any missing runtime dependencies
```

### Option 2 — build `.deb` from source

If you'd rather build the package yourself:

```bash
cd cmake-build-release
cpack -G DEB
sudo dpkg -i stapikcalendar_*.deb
sudo apt install -f
```

Either option installs the app to `/usr/lib/stapikcalendar/`, with a launcher at `/usr/bin/stapikcalendar`, and it appears in the desktop environment's application menu.

### Option 3 — per-user install (no sudo required)

```bash
cmake --install cmake-build-release --prefix "$HOME/.local"
```

Installs to `~/.local/lib/stapikcalendar/`, with a launcher at `~/.local/bin/stapikcalendar`. Make sure `~/.local/bin` is in your `PATH`.

## Uninstalling

### If installed via `.deb`

```bash
sudo dpkg -r stapikcalendar
```

### If installed per-user

```bash
rm -rf ~/.local/lib/stapikcalendar
rm ~/.local/bin/stapikcalendar
rm ~/.local/share/applications/stapikcalendar.desktop
rm ~/.local/share/icons/hicolor/256x256/apps/stapikcalendar.png
```

Either way, your calendar data, cloud config, language and theme preferences remain at `~/.local/share/stapikcalendar/` — see [Data Storage](#data-storage) below if you want to remove those too.

## Cloud Sync

The app supports synchronization via a self-hosted [Stapik Cloud](https://github.com/Stapik-Group/stapik-cloud) server (or any other with compatible api [Stapik Cloud API](https://github.com/Stapik-Group/stapik-cloud/blob/master/backend/src/main/resources/openapi/app-api.yaml)). Go to **File → Connect**, enter the server URL and the API key issued for this specific device (generated per-device from the Stapik Cloud admin panel — there is no shared or global key, and each device gets its own, independently revocable).

Once connected, the app compares the local file and the cloud copy using a timestamp and keeps whichever one is newer, overwriting the other **as a whole document**. There is no field-level or entry-level merging — if both copies changed since the last sync, the write that's based on the older cloud state loses.

Unlike the original simple sync protocol, a losing write isn't necessarily gone for good. Each cloud document slot has a conflict-resolution strategy set in the admin panel:
- **Last-write-wins with shadow copy** (the default) — the losing write is kept in the document's version history and can be restored from the admin panel.
- **Last-write-wins** — the losing write is discarded, matching the original behavior.

Data is saved locally after every change, and the app also attempts to push it to the cloud right away. If the cloud is unreachable at that moment, the change stays saved locally and the app quietly retries on the next save — no data is lost, but the cloud copy will lag behind until the next successful write. You can also trigger a sync manually from **File → Sync**.

**Caution for multi-device use:** editing the calendar offline on two different machines before either one reconnects can still cause one set of changes to lose the conflict. With shadow-copy resolution (the default), that copy isn't destroyed — it's recoverable from the admin panel's version history — but it won't reappear in the app on its own; recovering it means restoring it from the admin panel and syncing again. If you use the app on more than one device, sync (or at least go online) after each editing session to avoid needing a manual restore.

### API

The app talks to a Stapik Cloud server (or any server implementing the same contract):

- `GET /api/v1/documents/{slotKey}` — returns `{ "slotKey": "...", "content": "...", "contentHash": "...", "updatedAt": "..." }`
- `PUT /api/v1/documents/{slotKey}` — accepts `{ "content": "...", "clientLastKnownUpdate": "..." }`, returns the same shape as `GET`. Status `200` means the write was accepted; `409` means it lost a conflict, and the response body is the current, winning document rather than an echo of what was sent.

Authentication: `x-api-key` header with a per-device key from the admin panel. The old `/read`/`/write` endpoints and the single shared API key are no longer supported — existing installations need to be reconnected in **File → Connect** with a newly issued key.

## Data Storage

Calendar data is stored locally at `~/.local/share/stapikcalendar/calendar.json`, wrapped with a `lastUpdate` timestamp used for cloud sync. Each entry stores a name, an optional link and a color; files saved before entry colors were introduced are read without a `color` field and default to the standard color automatically.

Cloud config at `~/.local/share/stapikcalendar/config.json`. Language preference at `~/.local/share/stapikcalendar/locale.txt`. Theme preference at `~/.local/share/stapikcalendar/theme.txt`.

## Themes

![Screenshot](screenshots/screenshot_2.png)
![Screenshot](screenshots/screenshot_3.png)

## TODO

- [x] General refactor
- [x] Cloud sync with conflict resolution
- [x] `.deb` package for easier distribution
- [x] Entry colors — assign a color to each entry
- [x] Drag and drop entries between cells (with copy via Ctrl)
- [x] Theme switcher — Classic / Modern / Classic Pink
- [ ] Export to iCal format (.ics)
- [ ] Entry search
- [ ] Flatpak package