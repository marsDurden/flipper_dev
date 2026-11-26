# AGENTS.md — Flipper Zero app development

This repository is for building apps for the [Flipper Zero](https://flipperzero.one/), a
battery-powered, pocket-sized multi-tool for hardware hacking and radio protocol research.
Flipper apps are native C programs compiled against the device's firmware SDK, either baked
into a firmware build or packaged as standalone `.fap` files loaded from the SD card.

Read this file first. It links out to the background an agent needs before writing or
reviewing code for this device — the hardware it runs on, how its firmware is put together,
how its apps are expected to feel, and where the device pushes back.

## Map of this repo's docs

- [docs/hardware.md](docs/hardware.md) — the physical device: MCU, radios, display, storage,
  I/O, battery, expansion.
- [docs/gpio.md](docs/gpio.md) — the GPIO header in detail: pinout caveats, electrical
  limits, pin multiplexing, the `furi_hal_gpio` API, and expansion modules vs. raw wiring.
- [docs/architecture.md](docs/architecture.md) — firmware stack, the app manifest and build
  tooling, the GUI/scene-manager pattern, and the services apps talk to.
- [docs/app-ux.md](docs/app-ux.md) — the look and feel users expect from a Flipper app, and
  the interaction conventions that come from a 1-bit screen and a D-pad.
- [docs/limitations.md](docs/limitations.md) — hard constraints (RAM, flash, single foreground
  app, RF/legal limits, no native networking) to design around rather than discover late.

## Apps in this repo

- [apps/soundystem_fever](apps/soundystem_fever/) — audio level analyzer (dBu/dBV/dBm) for
  gain-staging a cascade of line-level gear (CD/CDJ, mixer, send/return, delay/echo,
  amplifier input) via the GPIO header. See its own `README.md`/`PLAN.md` for status.

## Working on an app here

- Target the official firmware's app SDK unless a task says otherwise. Community forks
  (Unleashed, RogueMaster, Momentum, …) extend or relax the official API in places — note
  explicitly in the app's manifest/README if a given app depends on one of them.
- An app is a directory with an `application.fam` manifest plus its C sources; see
  [docs/architecture.md](docs/architecture.md) for what the manifest declares and how the
  loader resolves the app against the firmware's exported API.
- Build and run against real firmware with [uFBT](https://github.com/flipperdevices/flipperzero-ufbt)
  (`ufbt`, `ufbt launch`, `ufbt flash`) for out-of-tree app development, or the firmware
  repo's own `fbt` if the app lives in-tree. Neither is vendored in this repo; install
  whichever matches how the app is meant to be distributed.
- Prefer the existing subsystem libraries (`gui`, `storage`, `notification`, `subghz`, `nfc`,
  `lfrfid`, `infrared`, `dialogs`, …) over talking to peripherals or the filesystem directly —
  see [docs/architecture.md](docs/architecture.md) for how apps reach them.
- Keep memory and storage use deliberate: there is no virtual memory and no swap. See
  [docs/limitations.md](docs/limitations.md) before assuming a design that would be
  unremarkable on a phone or laptop.
