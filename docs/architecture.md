# Firmware and app architecture

## Stack

FreeRTOS sits at the bottom. On top of it, the firmware has a thin internal layer ("furi")
providing threads, a named-record registry, pub/sub, message queues, and an event loop —
the primitives apps actually use instead of touching FreeRTOS directly. System services
(GUI, storage, notifications, Sub-GHz/NFC/LF-RFID/infrared subsystems, dialogs, …) run as
furi threads and publish themselves as named records that apps look up at runtime.

## Apps: manifest, build, loading

- Every app is a directory containing an `application.fam` manifest (Python-like syntax)
  declaring its `appid`, display name, `entry_point`, `category` (Main, Tools, GPIO,
  Infrared, NFC, RFID, Sub-GHz, Bluetooth, USB, Games, …), stack size, icon, required
  resources, and the firmware API version it targets.
- Apps are either built into a firmware image (in-tree) or compiled as standalone **FAP**
  (Flipper Application Package) files: position-independent binaries stored in the SD
  card's `apps/` folders and dynamically loaded and linked against the firmware's exported
  API table by the on-device loader when launched.
- Build tooling: the firmware repository's own `fbt` (SCons-based) builds in-tree apps;
  **uFBT** is the standalone, pip-installable equivalent for building a FAP out-of-tree
  without cloning the whole firmware. Both ultimately produce the same kind of artifact.
- Because FAPs link against a versioned API surface, a given build is tied to the firmware
  API version it was compiled against — see [limitations.md](limitations.md).

## GUI pattern

- A `Gui` service owns the framebuffer. An app registers a `ViewPort` directly for raw
  drawing, or — far more commonly — uses `View` + `ViewDispatcher` to switch between
  higher-level screens.
- Most non-trivial apps are organized with the **scene manager** pattern: a stack of named
  scenes, each implementing `on_enter` / `on_event` / `on_exit`. This plays the same role a
  router or a stack of activities/fragments plays elsewhere — Back pops the stack, OK/events
  drive transitions forward.
- Standard reusable GUI modules cover the common screens so apps rarely draw raw pixels for
  UI chrome: `Submenu` (scrollable list menu), `VariableItemList` (settings-style list),
  `Dialog`/`DialogEx`, `TextInput`/`ByteInput`, `Widget` (formatted/rich text), `Popup`, and
  `Loading`.

## Inter-component communication

- **FuriRecord**: named singleton services (`"gui"`, `"storage"`, `"notification"`,
  `"dialogs"`, …) are opened/closed by name via `furi_record_open` / `furi_record_close`
  rather than passed around as globals.
- **FuriPubSub**: broadcast/subscribe for events multiple components care about.
- **FuriMessageQueue**: thread-safe queues for passing data between an app's own threads.
- **FuriEventLoop**: an epoll-style event loop used by newer code for async I/O instead of
  polling.

## Storage and subsystem libraries

- The storage API abstracts the SD card and the small internal flash filesystem behind a
  POSIX-like file interface, plus structured "key file" helpers used for the saved-signal
  formats (Sub-GHz captures, NFC/RFID dumps, IR remotes) so apps share one on-disk format.
- Peripheral access for the built-in radios goes through subsystem libraries (`subghz`,
  `nfc`, `lfrfid`, `infrared`) that already implement the common protocol parsing/encoding.
  Apps normally call into these rather than driving the CC1101 or ST25R3916 registers
  directly.

## Firmware forks

Official firmware is conservative by design — it enforces regulatory limits (e.g. blocking
transmission of certain rolling-code protocols) and ships a narrower default feature set.
Community forks (Unleashed, RogueMaster, Momentum, and others) add features, relax region
locks, and extend the GUI/SDK, and can drift from the official API. Know which target an
app is being built for, since behavior and available APIs can differ.
