# Soundystem Fever

A Flipper Zero app, read through the GPIO header, for checking gain-staging between
line-level audio gear in a cascade — CD/CDJ → mixer → send/return → delay/echo → amplifier
input — by measuring levels in dBu/dBV/dBm-family units.

- The plan and build roadmap: [PLAN.md](PLAN.md)
- The dB/dBu/dBV/dBm/dBFS background this app's math is built on: [docs/audio-levels.md](docs/audio-levels.md)
- The unbalanced RCA input circuit (schematic, BOM, signal budget): [docs/rca-input-frontend.md](docs/rca-input-frontend.md)
- Bench-testing that circuit before connecting it to the Flipper: [docs/breadboard-test-guide.md](docs/breadboard-test-guide.md)
- Repo-wide Flipper Zero background (hardware, firmware architecture, GPIO, limitations):
  [../../AGENTS.md](../../AGENTS.md)

Current status: builds cleanly with `ufbt` against real firmware (target 7, API 88.2) as a
three-page carousel (Measurement, Audio Theory, About); no real ADC sampling yet — see
PLAN.md's milestones for what's next. **Do not wire audio directly into the GPIO header** —
read the "Hard constraint" section in PLAN.md first.

## Code layout

```
soundystem_fever_app.h   - shared app struct (SoundystemApp), passed to every page
soundystem_fever.c       - entry point: the page array, carousel navigation, title bar/dots
pages/
  page.h                 - the page interface (SoundystemPage) every page implements
  page_measurement.c/.h  - "Measurement" page
  page_docs.c/.h         - "Audio Theory" page (condensed from docs/audio-levels.md)
  page_about.c/.h        - "About" page
lib/
  db_convert/
    db_convert.c/.h      - pure dBu/dBV/volts/ADC-code conversions, no Flipper headers;
                           reusable from any page, and unit-testable with a plain C
                           compiler (see the commit history for the standalone test used
                           to verify it against docs/audio-levels.md's reference values)
```

`lib/db_convert` is wired up as a `fap_private_lib` in `application.fam` (`Lib(name="db_convert", ...)`)
rather than just being another source file: the build system reserves the top-level `lib/`
directory for exactly this - a private-library source tree named `lib/<name>/`, built as its
own static library and linked in. Don't drop loose `.c` files directly under `lib/`; they
won't be picked up by the default app sources and won't be compiled.

**A real, non-obvious constraint found by actually building this**: this firmware's exported
libm is almost entirely disabled - `log10f`, `fabsf`, `sqrtf`, every `log`/`exp` variant, etc.
are listed in the SDK's API table but not linked into the firmware, so calling them fails
`ufbt`'s symbol-resolution check. Only `powf` and `roundf` are enabled. `db_convert.c` doesn't
use `log10f`/`fabsf` for exactly this reason - its `db_log10f` computes log10 by numerically
inverting `powf(10, y)` via bisection (cheap enough for a UI refresh, not meant for a hot
sampling loop). If M3's RMS/peak math ever needs a square root, reach for `powf(x, 0.5f)`
rather than `sqrtf` for the same reason.

The app is a single `ViewPort` whose draw callback renders a shared title bar + page-dot
indicator, then delegates to the current page's own `draw`. Left/Right cycle pages (with
wraparound — it's a carousel); Back exits; every other input goes to the current page's
`input` callback, if it has one.

**To add a new page**: write `pages/page_<name>.c/.h` implementing the `SoundystemPage`
interface in `pages/page.h` (a `draw`, and optionally `input`/`on_enter`), then add one line
to the `pages` array in `soundystem_fever.c`. Nothing else changes.

Built and passing `ufbt`'s API-symbol check (`soundystem_fever.fap`, target 7 / API 88.2).
`lib/db_convert` is additionally compiled and tested standalone (plain `gcc`) against
`docs/audio-levels.md`'s reference values. Not yet run on real hardware - no physical Flipper
is attached in this environment, so flashing and on-device testing are still outstanding.

## Building

```
pip install ufbt            # once
cd apps/soundystem_fever
ufbt build                  # or: ufbt launch   (build + upload + run, with a Flipper over USB)
```

The built `.fap` is copied to this folder's own `dist/` (gitignored - a build artifact, not
source) for easy access, e.g. from Windows via a WSL path. See "Loading the app on the
device" below for installing `dist/soundystem_fever.fap`.

## Loading the app on the device

The build runs in WSL (Linux), the device gets flashed from Windows. First get the `.fap`
from WSL into somewhere Windows can see it - from a WSL shell, either:

```
explorer.exe .                     # opens this folder in Windows Explorer
```

or browse to `\\wsl$\<your-distro-name>\home\node\projects\flipper_dev\apps\soundystem_fever\dist\`
(or `\\wsl.localhost\...` on newer Windows) directly in Explorer.

**qFlipper (recommended - this part is well-established):**

1. Install qFlipper from flipperzero.one if you don't have it, and connect the Flipper over
   USB-C. qFlipper should show it as connected.
2. Open qFlipper's **File Manager** (left sidebar).
3. Browse to `/ext/apps/GPIO/` (this app's `fap_category` in `application.fam` is `GPIO`,
   which is the folder the firmware's Apps menu groups it under; create the folder if it
   doesn't exist yet).
4. Drag `soundystem_fever.fap` from the Windows Explorer window into that File Manager pane.
5. On the Flipper itself: **Apps → GPIO → Soundystem Fever** to launch it.

**Flipper Lab (lab.flipper.net), as an alternative:** the site can install apps from the
public catalog over WebUSB/WebSerial in a Chromium-based browser, and has historically also
supported loading an arbitrary local, unlisted `.fap`. I'm not fully certain of the current
exact click-path there (that UI changes over time, and I can't verify it from here), so
treat qFlipper above as the reliable path and check lab.flipper.net's own instructions if you
want to try the browser route instead.
