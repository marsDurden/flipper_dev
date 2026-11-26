# Limitations to design around

These are constraints of the device and the official firmware, not gaps to work around with
clever tricks — they should shape what an app attempts in the first place.

## Compute and memory

- 256 KB of RAM and a single 64 MHz Cortex-M4 core are shared across firmware, system
  services, and the running app. This is a microcontroller budget, not a smartphone one —
  heavy parsing, cryptography, or anything ML-like belongs off-device, not in a Flipper app.
- There is no virtual memory and no swap; an app that runs out of RAM crashes rather than
  degrading gracefully. Size data structures and buffers deliberately.

## Storage

- Internal flash is small and mostly reserved for firmware and settings. Almost all
  app-generated data (captures, dumps, saved remotes, logs, larger assets) lives on the
  user-supplied microSD card — which may be missing, slow, full, or a different filesystem
  state than expected. Handle its absence or errors explicitly rather than assuming it's
  always there.

## Execution model

- One foreground app at a time: the official firmware does not support running two
  third-party apps simultaneously, and FAPs do not get true background execution — only
  built-in system services run continuously. Don't design a feature that assumes the app
  keeps running once the user navigates away.

## API and build stability

- The external app (FAP) API is versioned and does change between firmware releases. A FAP
  built against one firmware version can fail to load, or misbehave, on another. Community
  firmwares can diverge from the official API further still — pin and state which firmware
  version/branch an app targets.

## Radio and regulatory limits

- RF behavior is deliberately constrained in official firmware for legal reasons, not just
  hardware ones: transmission of certain rolling-code protocols and some regional
  frequency ranges is blocked or limited by firmware policy. "The hardware can" does not
  mean "the firmware will," and what's legal to transmit depends on the user's region.
- NFC/Mifare Classic "cracking" support relies on known-weak-key dictionaries and the
  cipher's documented weaknesses — it is not a universal break of all cards, and plenty of
  tags (most newer Mifare DESFire/Plus configurations, for instance) are out of reach.

## No native networking

- The base device has no WiFi or cellular radio. Anything that needs IP networking requires
  an external GPIO add-on (e.g. a WiFi dev board) with its own firmware and toolchain —
  don't assume network access is available to an app by default.

## Physical I/O

- GPIO pins are 3.3 V logic with limited drive current and no galvanic isolation — careless
  wiring can damage the device or whatever it's connected to.
- Battery capacity is modest, and RF transmission plus the display backlight are the
  heaviest power draws; there is no low-power "always listening" mode for most of the
  radios, so long scanning sessions cost real battery.

## Tooling

- The SDK and uFBT are usable but still rough in places: documentation can lag the actual
  API, breaking changes land between firmware versions, and on-device debugging is mostly
  printf-style logging over USB-serial rather than full source-level debugging, unless a
  hardware debug probe is attached.
