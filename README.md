# Stapik Calendar

A desktop calendar application for Linux, written in C++20 using GTK4/gtkmm. Styled after the retro old-school aesthetic.

![Screenshot](screenshots/screenshot_1.png)

## Features

- **Monthly view** - calendar grid with month and year navigation
- **Calendar entries** - add, edit and delete entries with a name and optional link
- **Quick add from clipboard** - right-clicking a cell automatically fetches the page title from a URL copied to the clipboard and creates an entry
- **Undo/Redo** - full operation history for adding, editing and deleting entries
- **Cloud sync** - save and load data via an external API (compatible with a self-hosted server), with automatic conflict resolution based on timestamps
- **Multilingual UI** - Polish, English and German interface with instant switching
- **Auto-save** - calendar data saved locally after every change
- **Retro aesthetic** — classic look with raised buttons, blue navigation bar and grey cells

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

Either way, your calendar data, cloud config and language preference remain at `~/.local/share/stapikcalendar/` — see [Data Storage](#data-storage) below if you want to remove those too.

## Cloud Sync

The app supports synchronization via a self-hosted API server. Go to **File → Connect**, enter the server URL and API key.

Once connected, the app compares the local file and the cloud copy using a `lastUpdate` timestamp and keeps whichever one is newer, overwriting the other **as a whole document**. There is no field-level or entry-level merging — if both copies changed since the last sync, the older one is fully replaced.

Data is saved locally after every change, and the app also attempts to push it to the cloud right away. If the cloud is unreachable at that moment, the change stays saved locally and the app quietly retries on the next save — no data is lost, but the cloud copy will lag behind until the next successful write. You can also trigger a sync manually from **File → Sync**.

**Caution for multi-device use:** since conflict resolution is whole-document and last-write-wins, editing the calendar offline on two different machines before either one reconnects can cause one set of changes to be silently discarded. If you use the app on more than one device, make sure to sync (or at least go online) after each editing session to avoid overwriting your own changes.

The API must expose two endpoints:
- `GET /read?filename=calendar.json` — returns `{ "content": "..." }`
- `POST /write` — accepts `{ "filename": "calendar.json", "content": "..." }`

## Data Storage

Calendar data is stored locally at `~/.local/share/stapikcalendar/calendar.json`, wrapped with a `lastUpdate` timestamp used for cloud sync. Cloud config at `~/.local/share/stapikcalendar/config.json`. Language preference at `~/.local/share/stapikcalendar/locale.txt`.

## TODO

- [x] General refactor
- [x] Cloud sync with conflict resolution
- [x] `.deb` package for easier distribution
- [ ] Export to iCal format (.ics)
- [ ] Entry colors — assign a color to each entry
- [ ] Drag and drop entries between cells
- [ ] Entry search
- [ ] Flatpak package