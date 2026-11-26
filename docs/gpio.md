# GPIO

## Physical header

- An 18-pin, 2.54 mm-pitch header on the top edge, keyed so it can only be seated one way.
- It carries power pins (5 V and 3.3 V supply, several GND), a 1-Wire pin (the same bus the
  iButton reader uses), a UART TX/RX pair, two pins reserved for SWD (hardware debug/flash
  access), and a set of general-purpose pins named after their underlying STM32 port and pin
  (e.g. `PA7`, `PA6`, `PA4`, `PB2`, `PB3`, `PC0`, `PC1`, `PC3`).
- Treat the exact silkscreen-number-to-signal mapping as something to confirm against the
  official pinout reference (docs.flipper.net's hardware wiring page) before wiring anything
  up, rather than relying on a copied table from memory — swapping a power pin and a signal
  pin is how boards and modules get damaged.

## Electrical characteristics

- Logic level is 3.3 V. GPIO pins are **not** 5 V tolerant as digital inputs; the 5 V pin on
  the header is a power-output pin, not a tolerant I/O pin.
- Per-pin drive current and the total current budget across the header are limited — the
  header draws from the same battery/USB supply as the rest of the device, it is not a bench
  power supply.
- There is no galvanic isolation on the header. Miswiring or back-EMF from an external load
  (a relay, a motor, an inductive coil) can feed back into the MCU.

## Pin roles and multiplexing

- Most general-purpose pins are multiplexed at the silicon level: usable as plain digital
  input/output, but also mappable to on-chip peripherals — UART, SPI, I²C, PWM, ADC —
  depending on the specific pin and how firmware configures it at runtime.
- A few pins have a fixed, dedicated role on the header rather than being general-purpose:
  the 1-Wire pin, the UART TX/RX pair (USART1), and the SWC/SWD pair, which hardware
  debug/flash tooling (an ST-Link-style probe) takes over and which firmware does not expose
  as ordinary GPIO while a probe is attached.

## Software access (`furi_hal_gpio`)

- The low-level HAL exposes init/configure/write/read calls per pin: digital input or
  output, push-pull or open-drain, pull-up/pull-down, and interrupt callback registration for
  edge-triggered input.
- App code references header pins through `GpioPin` structs the HAL already defines (one per
  header position), rather than raw STM32 port/pin numbers — the firmware does the mapping
  from "header pin" to "MCU pin" once, centrally.
- Pin ownership is exclusive: only one app or service can hold a given pin's configuration at
  a time, consistent with the single-foreground-app model (see
  [limitations.md](limitations.md)). A well-behaved app deinitializes any pin it configured
  before it exits, so it doesn't leave the next app or service with a misconfigured pin.
- An app that touches GPIO declares that hardware dependency in its `application.fam`
  manifest (see [architecture.md](architecture.md)) rather than assuming access is implicit.

## Expansion modules vs. raw wiring

- Official add-on boards (such as the WiFi Dev Board) attach over the UART TX/RX pins using
  a defined "expansion protocol": the module and the Flipper negotiate control (e.g. the
  module driving the display/buttons, or exchanging structured data) instead of every
  accessory needing bespoke per-pin wiring and a dedicated app.
- Many community accessories (relay boards, sensor breakouts, extra RF/NFC front ends)
  instead just use the header as generic digital/analog/bus I/O, driven by a custom FAP or
  the stock GPIO app — no expansion-protocol negotiation involved.

## Built-in GPIO app

- Stock firmware ships a "GPIO" category app for manual pin exercise from the device itself —
  toggling pins and a basic USB-UART bridge — useful for bring-up and continuity checks
  before writing a custom app against the same pins.
