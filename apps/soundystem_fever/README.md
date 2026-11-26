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

Current status: scaffold only (an app skeleton that boots and exits) — see PLAN.md's
milestones for what's next. **Do not wire audio directly into the GPIO header** — read the
"Hard constraint" section in PLAN.md first.
