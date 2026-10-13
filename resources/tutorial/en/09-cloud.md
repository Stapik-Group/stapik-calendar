# Stapik Cloud synchronization

Stapik Cloud is a server that you run yourself. It keeps one copy of your calendar so that all your computers show the same entries. The cloud is optional — without it the calendar simply works on one computer.

## What you need

- A running Stapik Cloud server that your computer can reach, and its **address** (the API URL), for example `https://cloud.example.com`.
- An **API key** issued for this device. In the Stapik Cloud admin panel every device gets its own key; there is no shared key.

The calendar is stored in the document named `calendar.json` on the server.

## Connecting

1. Choose **File → Connect to cloud**.
2. Enter the **API URL**. It must start with `http://` or `https://`.
3. Enter the **API key**. Tick **Show key** if you want to check what you typed.
4. Click **Connect**.
5. After a moment a message tells you whether the connection worked (*Connected*) or not (*Could not connect*, with the reason).

> If the address starts with `http://`, the window warns you that the connection is not secure: the key and your calendar are sent unencrypted. Use `https://` whenever the server is reachable over the internet.

![The Connect to cloud window with the API URL and API key fields](images/connect-dialog.png)

![The message confirming a successful connection](images/connect-success.png)

The settings are remembered. Next time the application connects by itself, and the window you open from *Connect to cloud* is already filled in, so you can use it to change the server or the key.

**What happens at the first synchronization:** if the cloud is still empty, your calendar is uploaded. If the copy in the cloud is newer than yours, it replaces your local calendar. Otherwise your local calendar is uploaded.

## The status bar

When a cloud is configured, the bottom of the window shows the current state:

- Up to date — Your calendar and the cloud are the same.
- Syncing… — A synchronization is in progress.
- Offline — The server cannot be reached. Your changes are safe on this computer, and the application retries by itself (first after 2 seconds, then less and less often, at most every 5 minutes).
- Conflict: server version kept — The cloud held a newer version, and it replaced your local one (see *Conflicts*).
- Sync error — The synchronization failed for another reason, for example the server rejected the request. Local data is not affected.

![The status bar at the bottom of the window showing the synchronization state](images/sync-status.png)

## How synchronization works

- Every change is saved on your computer immediately.
- Sending to the cloud happens in the background, about 1.5 seconds after your last change, so the window never freezes. Several quick changes are sent together.
- When you close the application, any change that has not been sent yet is sent first.
- **File → Sync** synchronizes immediately.

## Conflicts and several devices

The whole calendar is compared as one document, and the newer version wins. Entries are never merged one by one. If you change the calendar on two computers while both are offline, the version that reaches the server later replaces the other.

When the server version wins, it replaces the calendar on your computer and the undo history is cleared. By default Stapik Cloud keeps the replaced version in the document history, so it can be restored from the admin panel; restore it there and synchronize again.

> Tip for several devices: after you finish editing, wait for *Up to date* (or use *Sync*) before you switch to another computer.

## Changing the server or stopping synchronization

To use another server or key, open **File → Connect to cloud** again and enter the new values. There is no disconnect button: to stop using the cloud, close the application and delete the file `~/.config/stapikcalendar/config.json`. Your local calendar is not affected.

The API key is stored in that file in plain text, readable only by you, so keep your home directory protected.
