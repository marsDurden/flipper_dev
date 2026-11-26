# Unbalanced RCA input front end — schematic (v1)

Scope: a single unbalanced RCA input, conditioned for a Flipper GPIO ADC pin. No balanced
(TRS/XLR) handling here — that's a separate front end for later, per `PLAN.md`'s open
questions. This is the first design pass; treat all values as "computed, not yet bench
verified" until M2 is actually breadboarded and measured.

Read [audio-levels.md](audio-levels.md) for the dB/dBu/dBV background and
[../../../docs/gpio.md](../../../docs/gpio.md) for why this circuit exists at all — Flipper
GPIO pins are 3.3 V, not 5 V tolerant, and line-level audio does not fit that range directly.

## Breadboard layout

![Suggested breadboard layout for the RCA input front end: an RCA jack feeds C1, R1 into Node A, then C2 into Node B, where R3/R4 set the 1.65 V bias, C3 filters it, a jumper bridges to a second column for R5 and the clamp diode, and the result heads to the Flipper's GPIO header.](images/rca-breadboard.svg)

This is a suggested physical arrangement, generated from the same component values and
connections as the list below — it's a layout reference, not a substitute for it. If the
image and the connection list ever disagree, the connection list wins. Column numbers are
arbitrary (any node could sit in a different column); what matters is which column each
component's leads share within a column. Note also the GND-rail trace running near R3
(3V3→Node B): it passes close to the GND rail without connecting to it — two separate rails,
not a short, same as on a real breadboard.

## Topology

```
                     Node A                              Node B
RCA TIP o---C1---+-----+                        +----+----+----+----+---R5---o  ADC pin
  (signal)      1uF    |                        |    |    |    |    |  1k    (PC0 — confirm
                       R1                       C2    R3   R4   C3   |        against the
                      27k                       1uF  220k 220k  1nF  |        physical pinout
                (series)|                     (couple) |    |        |        before wiring)
                        R2                             |    |        |
                       18k                            3V3  GND      GND
                (shunt) |                         (bias   (bias
                        |                          supply) supply)
                       GND -------- Node A --------+----+----+----+--- Node B
                                                    |
                                      D_clamp: three-terminal series
                                      dual-Schottky (e.g. BAT54S/BAV99):
                                      outer pin -> GND, outer pin -> 3V3,
                                      CENTER pin -> Node B

RCA SLEEVE o-------------------------------------------------------------- GND
                                                              (shared with header GND pin)
```

### Connection list (authoritative — if the ASCII art and this list ever disagree, trust this)

- RCA tip → **C1** → **R1** → **Node A**
- **Node A** → **R2** → GND
- **Node A** → **C2** → **Node B**
- 3V3 (header) → **R3** → **Node B**
- GND (header) → **R4** → **Node B**
- **Node B** → **C3** → GND
- **Node B** → **R5** → GPIO ADC pin
- Overvoltage clamp: a three-terminal series dual-Schottky (BAT54S or BAV99) with its
  **center pin on Node B**, one outer pin to GND, the other outer pin to 3V3 — this gives one
  diode GND→Node B (clamps low) and one diode Node B→3V3 (clamps high) from a single part.
- RCA sleeve → GND, same GND net as the header's GND pin and as R2/R4/the clamp.

## Bill of materials

| Ref | Value | Role |
|---|---|---|
| J1 | RCA female jack | Unbalanced input connector |
| C1 | 1 µF film or ceramic | DC-blocks the source before the attenuator |
| R1 | 27 kΩ | Attenuator series leg |
| R2 | 18 kΩ | Attenuator shunt leg (to GND) |
| C2 | 1 µF film or ceramic | AC-couples the attenuated, ground-referenced signal onto the biased node |
| R3 | 220 kΩ | Bias leg, to 3V3 |
| R4 | 220 kΩ | Bias leg, to GND |
| C3 | 1 nF ceramic | High-frequency/EMI filtering at the ADC node |
| R5 | 1 kΩ | Series fault-current limiting + filter pole, right before the MCU pin |
| D_clamp | BAT54S or BAV99 (series dual-Schottky, SOT-23) | Overvoltage/undervoltage protection for the ADC pin |

All resistors standard ±5% (or better) tolerance is fine — exact attenuation is read back and
compensated by the in-app calibration step (`PLAN.md` M4), not relied on as an exact constant.

## Why these values

- **Input impedance ≈ R1 + R2 = 45 kΩ.** High enough to bridge (not load) a typical line
  output, in the same range consumer line inputs themselves commonly spec. See the
  "Impedance: bridging, not matching" section of `audio-levels.md`.
- **Attenuation ≈ R2 / (R1+R2) = 18k/45k = 0.4 (−8 dB).** A clean, simple ratio chosen so the
  linear (non-clipped) measurement range comfortably spans both consumer (−10 dBV) and
  professional (+4 dBu) nominal levels with margin — see "Signal budget" below.
- **C1, C2 = 1 µF** each put the resulting high-pass corner well under 10 Hz against the
  resistances they see, so bass content isn't attenuated.
- **R3 = R4 = 220 kΩ** set Node B's quiescent bias at 3.3V/2 ≈ 1.65 V (the standard
  "single-supply AC-coupled ADC input" trick — it lets a signal that swings positive and
  negative around 0 V sit inside the ADC's 0–3.3 V-only range) while being high enough in
  value that they don't meaningfully load the AC signal passing through C2.
- **R5 + C3** form a simple, mild low-pass (~160 kHz corner) purely for EMI/noise rejection
  right at the MCU pin — far above the audio band, so it doesn't touch the measurement.
- **D_clamp** is a hard backstop, not the normal operating mechanism: normal signals should
  stay inside the linear range below. It exists for the fault case — an unexpectedly hot
  source, or a wiring mistake.

## Signal budget (approximate — confirm on the bench)

- ADC node (Node B) can swing roughly 0.3 V–3.3 V before the clamp starts conducting
  (Schottky forward drop ≈0.3 V on each side of the 0–3.3 V rails). Around a 1.65 V bias,
  that's about **±1.35 V** of linear swing at Node B.
- Working back through the −8 dB (0.4×) attenuator, that corresponds to an input signal of
  roughly **±3.4 V peak**, i.e. about **+10 dBu RMS** for a sine wave — comfortably above
  both consumer and pro nominal levels, which is the point: normal signals from any of the
  gear types in `PLAN.md` (CD/CDJ, mixer, send/return, delay/echo, amp input) should read
  without clipping the front end.
- The clamp's hard limit sits higher still, around **±4.9 V peak at the RCA jack**
  (~+15 dBu-equivalent peak) — anything hotter than that gets clipped by the diodes rather
  than passed through, protecting the ADC pin instead of measuring accurately. That's
  expected and fine for a gain-staging tool; it is not a precision peak meter.
- Quiet end: a −10 dBV consumer signal (≈0.316 V RMS) attenuated by 0.4 lands around
  0.126 V RMS at the ADC — with a 12-bit ADC (3.3 V / 4096 ≈ 0.8 mV per count) that's roughly
  ±220 counts of peak swing, which is enough resolution for level metering, though very quiet
  signals (deep into the noise floor) will be limited by ADC/front-end noise rather than by
  this design on paper. Expect to characterize the real noise floor at M2.

## Known caveats

- **Ground loop / hum risk**: the RCA sleeve, R2/R4, and the clamp all return to the same GND
  net as the Flipper's own GND pin — and that GND is also whatever the Flipper itself is
  referenced to (e.g. a laptop's USB ground, if charging while measuring). Tying the audio
  source's ground to a different-potential ground is exactly how unbalanced gear picks up
  hum; it's a property of unbalanced interconnects in general, not a bug in this circuit.
  Balanced input handling (a separate front end) is the usual fix, and is explicitly out of
  scope for this v1.
- **This is a measurement range, not a universal one.** Anything louder than the clamp's hard
  limit is clipped, not measured. If a source needs to be checked above that range later
  (e.g. a very hot pro output), that's a reason to revisit the attenuation ratio for a
  "hot" variant, not to raise the clamp thresholds.
- **Pin assignment is tentative.** `PC0` is used as the working label here because it's one
  of the ADC-capable pins named in `../../../docs/gpio.md` — confirm the exact physical pin
  against the official pinout before wiring, per that doc's own caveat.

## Status

Designed on paper only. Not yet breadboarded, not yet bench-verified against a known
reference signal. This is M2 in `PLAN.md`; building and measuring this circuit is the next
step, after which the "Signal budget" numbers above should be corrected to match reality
before M3 firmware trusts them.
