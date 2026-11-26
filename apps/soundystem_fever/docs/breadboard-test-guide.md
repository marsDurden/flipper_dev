# Breadboard bring-up and test guide

Purpose: verify the circuit in [rca-input-frontend.md](rca-input-frontend.md) behaves as
designed **using bench equipment only**, before any wire from it ever touches the Flipper's
GPIO header. A wiring mistake is cheap to find with a multimeter and a bench supply; it is
not cheap to find by frying a GPIO pin.

**Do not connect this circuit's output to the Flipper until it has passed every step below.**

## Tools needed

- A digital multimeter (DMM) with AC and DC voltage ranges — required.
- An oscilloscope — strongly recommended for Step 4 (clamp behavior), optional elsewhere. If
  none is available, the DMM-only fallback notes in each step work, just less precisely.
- A bench power supply that can output 3.3 V with an adjustable current limit, **or** any
  other 3.3 V source that is *not* the Flipper itself (a USB power bank through a 3.3 V LDO
  breakout, a couple of AA cells through a small regulator, a second dev board's 3.3 V pin).
  The point is to power the bias network from something other than the device you're trying
  to protect while you're still debugging.
- A test signal source: a function generator is ideal. Failing that, a phone or laptop
  playing a 1 kHz test tone at a known, moderate volume works fine for these checks — exact
  calibrated accuracy isn't the goal yet (that's `PLAN.md` M4); confirming the circuit
  behaves correctly is.
- Breadboard, jumper wires, the parts from the BOM in `rca-input-frontend.md`.

## Step 0 — build and inspect before powering anything

1. Build the circuit from `rca-input-frontend.md`'s connection list.
2. Before applying any power, visually check:
   - No direct short between the 3V3 and GND rails on the breadboard.
   - The dual-diode clamp's orientation — its datasheet pinout determines which outer leg is
     which polarity; get this backwards and it won't clamp correctly (or will just sit there
     conducting). If in doubt, confirm with the DMM's diode-test mode before power-up:
     each diode direction should show a forward drop, the reverse direction should show
     open/OL.
   - If you substituted polarized electrolytic capacitors for C1/C2 instead of film/ceramic,
     double-check their polarity — reversed electrolytics on an AC-coupling node is a classic
     way to get a dead or drifting circuit. Film/ceramic (as specified) avoids this entirely.
3. With everything unpowered, use the DMM in continuity mode to confirm Node A and Node B are
   only connected to what the connection list says they're connected to — no accidental
   bridges from a stray breadboard row.

## Step 1 — power up from the bench supply, no signal yet

1. Set the bench supply to 3.3 V with a conservative current limit (tens of mA is plenty —
   this circuit draws microamps to low milliamps at most; a tripped current limit here is a
   signal something's wired wrong, not a problem to raise the limit past).
2. Connect bench 3.3 V → the breadboard's 3V3 net, bench GND → the breadboard's GND net. Leave
   the RCA input unconnected (no signal yet) and leave the "to ADC pin" wire unconnected to
   anything (just probe it directly with the DMM).
3. **Pass criterion**: the supply's current draw is low (consistent with a resistive bias
   network pulling microamps to low milliamps) and stable — not climbing, not at the current
   limit. If the current limit trips, power off and recheck Step 0 before trying again.

## Step 2 — quiescent bias check

1. With the supply still on and no signal applied, measure Node B (the "to ADC pin" point) to
   GND with the DMM in **DC** voltage mode.
2. **Pass criterion**: roughly 1.65 V (half of 3.3 V), within normal resistor-tolerance range
   — a few percent off is expected and fine; this gets corrected in firmware calibration
   (`PLAN.md` M4), not here.
3. If it's badly off (pinned near 0 V, near 3.3 V, or reads strangely):
   - Near 0 V or near 3.3 V usually means one of R3/R4 is missing, swapped, or the clamp diode
     is backwards and conducting continuously.
   - Recheck Step 0's continuity and diode-orientation checks before moving on.

## Step 3 — attenuation ratio check with a known test signal

1. Apply a 1 kHz test tone to the RCA input at a moderate level — comfortably inside the
   linear range from `rca-input-frontend.md` (well under the ~+10 dBu ceiling; a typical
   phone/laptop line output at a moderate volume setting is fine to start).
2. Measure the input signal itself, RMS, with the DMM in **AC** voltage mode, tip to sleeve,
   right at the RCA jack. Record this as `V_in`.
3. Measure Node A to GND, DMM **AC** mode. Record as `V_A`. Compare `V_A / V_in` against the
   attenuator's ~0.4 (−8 dB) design ratio — expect it to be in the right ballpark, not exact.
4. Measure Node B to GND, DMM **AC** mode (a DMM in AC mode reads just the AC component and
   ignores the 1.65 V DC bias, so this is safe and meaningful directly on the biased node).
   Record as `V_B`. Expect `V_B` to be close to `V_A`, possibly a little lower — the bias
   network's resistors do load the AC signal somewhat, which is accounted for by calibrating
   in firmware rather than trusting a hand-calculated constant.
5. **Pass criterion**: `V_A` and `V_B` both move up and down proportionally as you change the
   test tone's volume, staying in a consistent ratio to `V_in`. That's the signature of a
   working, linear (non-clipping) attenuator and coupling path. If `V_A` is near zero
   regardless of input, suspect C1 (open, or wired to the wrong node) or R1/R2. If `V_A` looks
   right but `V_B` is near zero, suspect C2.

## Step 4 — clamp behavior check (do this incrementally, not in one jump)

1. If you have a scope: probe Node B directly, with the test tone source volume starting low
   and the time base set to see a few cycles of the waveform.
2. Slowly raise the test signal's level while watching Node B.
   - **Expected**: the waveform's amplitude grows proportionally, then — once the design's
     linear ceiling is approached — the top and bottom of the waveform start to visibly flatten
     ("clip") rather than continuing to grow, settling at roughly the 0.3 V–3.3 V-ish rails
     described in `rca-input-frontend.md`'s signal budget. That flattening, not a sudden jump
     or a dead signal, is what correct clamp behavior looks like.
3. **DMM-only fallback** (no scope): watch the Node B AC RMS reading as you raise the input
   level in small steps. Initially it should climb roughly proportionally with the input.
   Once clamping engages, the RMS reading's growth will clearly slow down and level off even
   as you keep raising the input — that flattening-off in the numbers is the same thing the
   scope would show as a visibly clipped waveform. Stop increasing the test level once you see
   that — you've found the practical ceiling, and there's no need to push it further to prove
   the point.
4. **Pass criterion**: the signal clamps (flattens / stops growing) rather than Node B's DC
   bias collapsing, drifting, or the supply current spiking. If raising the input instead
   causes the bias point itself to shift far from 1.65 V, or current draw to climb
   noticeably, stop and recheck the clamp diode's orientation and R5 before going further.

## Step 5 — quiet-signal sanity check

1. Lower the test tone to a quiet level — something in the neighborhood of consumer nominal
   (−10 dBV, ≈0.3 V RMS at the jack) or below.
2. Confirm Node B's AC reading is still clearly present and proportional (not lost in noise,
   not reading zero). This is a sanity check, not a calibration — exact noise-floor
   characterization is part of M2's bench work once this guide's checks all pass, and doesn't
   need to block it.

## Step 6 — only now, connect to the real Flipper header

Do this only after Steps 1–5 all passed on the bench supply.

1. With the breadboard circuit's RCA input and bench supply **disconnected**, first check the
   Flipper's own 3V3 and GND header pins with the DMM (device powered on) to confirm they
   read what's expected, before wiring anything to them.
2. Connect the breadboard's 3V3 and GND nets to the Flipper's header 3V3/GND pins — signal
   input and the "to ADC pin" wire still not connected to anything yet.
3. Measure Node B again, DMM DC mode. **Pass criterion**: same ~1.65 V bias as in Step 2,
   now powered from the Flipper's own rail instead of the bench supply. If it's noticeably
   different, stop and figure out why before going further (don't assume the bench-supply
   result still holds).
4. Reconnect the RCA test signal and repeat a quick version of Step 3 (check `V_B` moves
   proportionally with the input) powered from the Flipper's rail.
5. Only after that checks out, connect the "to ADC pin" wire to the chosen GPIO pin (see the
   pin-confirmation caveat in `rca-input-frontend.md` and `../../../docs/gpio.md`) — no
   firmware running yet is required for this physical connection step itself, since the ADC
   pin's high input impedance means simply wiring it up should draw negligible current if
   everything upstream has already checked out.

## Checklist summary

- [ ] Step 0: visual/continuity/diode-orientation check passed, unpowered.
- [ ] Step 1: bench-supply current draw low and stable with no signal.
- [ ] Step 2: Node B quiescent bias ≈1.65 V DC.
- [ ] Step 3: Node A and Node B AC readings track the input proportionally.
- [ ] Step 4: Node B visibly/measurably clamps (flattens) at high input rather than failing
      some other way.
- [ ] Step 5: a quiet, near-nominal-consumer-level signal still reads clearly.
- [ ] Step 6: bias and attenuation re-confirmed powered from the Flipper's own 3V3/GND before
      the ADC pin wire goes anywhere near the header.

Only once every box above is checked should this circuit be considered ready for the M3
firmware work (ADC sampling) in `PLAN.md`.
