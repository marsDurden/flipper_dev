# dB, dBu, dBV, dBm, dBFS — a working glossary

This is the domain background the app's math and its "cascade matching" feature are built
on. It's written to be read standalone, and is also the intended source text for the app's
own in-device "Theory" screen later.

## dB is always a ratio

A decibel is a dimensionless, logarithmic ratio between two quantities of the same kind —
never a quantity on its own. "80 dB" means nothing without knowing the reference; what comes
after "dB" names it (dBm, dBu, dBV, dBFS, dB SPL, …).

- For power ratios: `dB = 10 · log10(P1 / P2)`
- For voltage (or other amplitude) ratios: `dB = 20 · log10(V1 / V2)`

The factor is 20 instead of 10 for voltage because power is proportional to voltage squared
— squaring inside a log is the same as doubling the multiplier outside it.

## The references that matter for line-level audio

- **dBm** — power referenced to 1 milliwatt, defined for a specific impedance (classically
  600 Ω, from old telephone and broadcast practice): `dBm = 10 · log10(P / 1 mW)`. Because
  it bakes in an impedance assumption, dBm is only a true power measurement when that
  impedance is actually present. Modern line-level gear doesn't work that way (see
  "bridging" below), so you'll hear "dBm" used loosely to mean "dBu" in casual conversation
  — worth being precise about, since they're only numerically equal at 600 Ω.
- **dBu** — voltage referenced to 0.775 V RMS (the voltage that develops 1 mW across 600 Ω,
  kept as a reference value without requiring 600 Ω to be present). `0 dBu = 0.775 V RMS`.
  This decoupling from impedance is exactly why dBu displaced dBm as the standard
  professional line-level reference once bridging interconnects took over.
- **dBV** — voltage referenced to 1 V RMS. `0 dBV = 1 V RMS`. Common on consumer-grade gear
  spec sheets.
  - Conversion: `dBu ≈ dBV + 2.2 dB` (equivalently `0 dBV = +2.2 dBu`), since
    `20·log10(1 V / 0.775 V) ≈ 2.2 dB`.
- **dBFS** — referenced to a digital system's maximum representable amplitude ("full
  scale"). `0 dBFS` is the hard ceiling (clipping); every real signal sits below it.
  Professional digital gear typically targets an average operating level well under full
  scale to leave peak headroom — common conventions are around **-18 dBFS** (EBU R68) or
  **-20 dBFS** (SMPTE RP155) for "nominal/0 VU", not 0 dBFS.
- **dB SPL** — sound *pressure* level in air, referenced to 20 µPa. Not part of this app's
  electrical signal chain, but worth naming so it isn't confused with the voltage-referenced
  dB values above when the word "decibel" comes up in conversation.

## Nominal operating levels, and why gear disagrees

- **Consumer line level**: nominal **-10 dBV** (≈ 0.316 V RMS). Typical of consumer CD
  players, tape decks, and home-mixer RCA outputs.
- **Professional line level**: nominal **+4 dBu** (≈ 1.228 V RMS). Typical of pro mixers,
  CDJs' balanced/pro outputs, rack effects units, and balanced TRS/XLR interconnects.
- The gap between -10 dBV and +4 dBu nominal is **≈ 11.8 dB** — not a rounding error, a
  real and commonly cited figure. Plug consumer gear into a pro input (or the reverse)
  without compensating for that gap, and you get either a weak, noisy signal or a signal
  that's driving the next stage into clipping. This is the core mismatch this app exists to
  surface.
- **Headroom**: the margin between nominal ("0 VU") and the hard clip point. Pro gear
  typically has on the order of 18–24 dB of headroom above +4 dBu; consumer gear often has
  noticeably less. A level that looks "fine" on average can still clip on transient peaks if
  headroom runs out.
- **Sends/returns (effects loops)**: expect to match the mixer's own internal operating
  level — better mixers make this switchable between -10 dBV and +4 dBu per send. A level
  mismatch here shows up as a noisy, buried effect return, or a wet signal that distorts
  before the dry signal would.
- **Delays/echo units**: rack line-level units expect line level as above; many guitar-pedal
  style delay/echo boxes are designed around instrument level instead (roughly -20 dBu
  territory), which is a different nominal level again — a very common real-world mismatch
  when pedal-format effects get patched into a line-level rack chain.
- **Amplifier inputs**: sized around line level (roughly 0.775–1.5 V RMS) to reach the amp's
  rated output at its specified gain/sensitivity setting. Too low an input level means never
  reaching full power (worse signal-to-noise at the top), too high risks clipping the amp's
  own input stage before the power section even gets involved.

## Impedance: bridging, not matching

- The old 600 Ω "matched impedance" convention (output impedance = input impedance = 600 Ω)
  comes from telephone and broadcast practice, and is the reason dBm assumes 600 Ω.
- Modern line-level gear instead uses **bridging**: a low output impedance (tens to a few
  hundred ohms) driving a much higher input impedance (commonly 10 kΩ–47 kΩ or more). This
  maximizes voltage transfer and lets one output feed several inputs in parallel without
  significant loading — and it's why a power measurement referenced to a fixed impedance
  (dBm) stopped being the practical standard for gain-staging line-level gear, in favor of a
  voltage-only reference (dBu).
- For this app specifically, impedance mostly matters as a constraint on the measurement
  front end: it needs a high input impedance relative to whatever it's clipped onto, so
  probing a signal doesn't itself load down and change the very level being measured.
