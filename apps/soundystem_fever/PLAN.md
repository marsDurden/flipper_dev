# Soundystem Fever — plan

## Goal

A Flipper Zero app that reads a conditioned line-level audio signal through the GPIO header
and reports its level in dB-family units (dBu, dBV, dBm-as-commonly-used, dBFS-equivalent),
so a user can check whether consecutive devices in a signal chain — CD player/CDJ → mixer
channel → send/return effects loop → delay/echo unit → amplifier input — are gain-staged
correctly before something clips or gets buried in noise. See
[docs/audio-levels.md](docs/audio-levels.md) for the theory this is built on.

This is **not** a spectrum analyzer or oscilloscope project (yet) — the core deliverable is
level metering plus a "does this match the next device's expected input?" judgment, built on
top of what's in docs/audio-levels.md.

## Hard constraint: never wire audio directly into a GPIO pin

Flipper's GPIO pins are 3.3 V logic and not 5 V tolerant (see
[../../docs/gpio.md](../../docs/gpio.md)). Line-level audio swings around 0 V (AC-coupled)
and at professional nominal level alone already sits at ≈1.2 V RMS before accounting for
headroom — a hot signal can swing several volts peak-to-peak in both directions. Connecting
a line output straight to a header pin can exceed the pin's voltage range in either
direction and damage the MCU, and it also drags the source down because the pin's native
input impedance isn't designed to be a proper bridging load.

Every signal this app reads must first pass through an external conditioning front end:

1. **Input protection / impedance**: a buffer or at minimum a high-impedance divider so the
   front end doesn't load the source being measured (see "bridging" in
   [docs/audio-levels.md](docs/audio-levels.md)).
2. **Attenuation**: a resistor divider sized so the loudest signal this app is meant to
   measure (pro line level plus headroom, not just nominal) lands safely inside 0–3.3 V.
3. **AC coupling + bias**: a coupling capacitor plus a bias network centering the waveform
   at mid-rail (~1.65 V) so an AC audio signal fits an ADC pin that can only read 0–3.3 V,
   not negative voltages.
4. **Overvoltage protection**: clamping (e.g. a small series resistor plus low-capacitance
   clamping diodes or a TVS) in case of an unexpectedly hot source or a wiring mistake.

Whether metering is done by sampling the conditioned waveform directly (true peak/RMS, needs
a usable ADC sample rate) or by adding an analog rectifier/envelope-follower stage ahead of
the ADC (slower-changing DC level, much easier to sample, at the cost of extra analog parts)
is an open decision — see "Open questions" below.

## Milestones

- **M0 — scaffold (this commit)**: app folder, manifest, a minimal buildable skeleton
  (`soundystem_fever.c`) that just boots, draws a placeholder screen, and exits on Back, plus
  this plan and the theory reference.
- **M1 — reference data finalized**: nominal/headroom numbers per device class (CD/CDJ,
  mixer channel, send, return, delay/echo, amp input) turned into the lookup table the
  "cascade check" feature will use. Builds on [docs/audio-levels.md](docs/audio-levels.md).
- **M2 — front-end hardware**: breadboard the conditioning circuit and bench-verify it
  against a known reference signal (signal generator or a calibrated source) before trusting
  any reading from it. Unbalanced RCA v1 is designed on paper — see
  [docs/rca-input-frontend.md](docs/rca-input-frontend.md) — and still needs to be built and
  measured; [docs/breadboard-test-guide.md](docs/breadboard-test-guide.md) is the bench-test
  procedure to run before any wire from it touches the Flipper's GPIO header.
- **M3 — ADC sampling + level math**: firmware reads the conditioned signal, computes
  RMS/peak over a sampling window, and converts the raw reading to dBu/dBV using the
  calibration from M2. Live meter screen (bar graph + numeric readout + peak hold + clip
  indicator).
- **M4 — in-app calibration**: a calibration flow so the conversion from raw ADC counts to
  dBu doesn't rely on hand-tuned constants baked into firmware — feed a known level, tell the
  app what it is, store the resulting calibration.
- **M5 — cascade check**: pick a source device type and a destination device type from the
  M1 table; compare the live measured level against the destination's expected input range
  and report "too hot / clipping risk," "too low / noise floor risk," or "matched."
- **M6 — persistence**: save calibration and settings (unit preference: dBu/dBV/volts) across
  runs; optionally log sessions to the SD card as CSV.
- **M7 — stretch**: basic frequency-domain checks (e.g. mains-hum detection at 50/60 Hz,
  simple tone presence via a Goertzel filter) — only once level metering is solid.

## Decisions made

- **Connectors**: unbalanced RCA first (v1) — see
  [docs/rca-input-frontend.md](docs/rca-input-frontend.md). Balanced TRS/XLR handling
  (send/return loops, pro gear) is deferred to a later front end rather than designed in now;
  it needs either a differential input stage or a small transformer to stay ground-loop-safe,
  which unbalanced RCA doesn't need.
- **Detection method**: sample the conditioned waveform directly on the ADC and compute
  RMS/peak in firmware, rather than an analog rectifier/envelope-follower stage. Keeps the
  front end passive (no op-amp, no extra supply concerns) for v1.

## Open questions (need a decision before M2 is built)

- **Which header pin(s)**: the ADC-capable header pins need confirming against the official
  pinout and whichever ADC access the firmware SDK actually exposes to external apps before
  locking the front-end design to a specific pin (see the caveat in
  [../../docs/gpio.md](../../docs/gpio.md)). `docs/rca-input-frontend.md` uses `PC0` as a
  working label pending that confirmation.

## Status

M0 only. Nothing beyond the skeleton has been built or bench-tested; `soundystem_fever.c`
has not been compiled against a real SDK in this environment, so treat it as "written to the
known-good minimal-app pattern" rather than "verified" until it's built with `ufbt` or `fbt`.
