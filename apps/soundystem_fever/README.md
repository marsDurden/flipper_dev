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

Current status: a UI skeleton with three carousel pages (Measurement, Audio Theory, About) and
no real ADC sampling yet — see PLAN.md's milestones for what's next. **Do not wire audio
directly into the GPIO header** — read the "Hard constraint" section in PLAN.md first.

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
  db_convert.c/.h        - pure dBu/dBV/volts/ADC-code conversions, no Flipper headers;
                           reusable from any page, and unit-testable with a plain C
                           compiler (see the commit history for the standalone test used
                           to verify it against docs/audio-levels.md's reference values)
```

The app is a single `ViewPort` whose draw callback renders a shared title bar + page-dot
indicator, then delegates to the current page's own `draw`. Left/Right cycle pages (with
wraparound — it's a carousel); Back exits; every other input goes to the current page's
`input` callback, if it has one.

**To add a new page**: write `pages/page_<name>.c/.h` implementing the `SoundystemPage`
interface in `pages/page.h` (a `draw`, and optionally `input`/`on_enter`), then add one line
to the `pages` array in `soundystem_fever.c`. Nothing else changes.

Not yet built with `ufbt`/`fbt` against a real SDK in this environment — the pure-C
`lib/db_convert` module has been compiled and tested standalone (plain `gcc`), and the
Flipper-specific files have been syntax-checked against hand-written stub headers matching
the API calls they make, but neither is a substitute for a real firmware build.
