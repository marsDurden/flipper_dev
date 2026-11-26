# How a Flipper app should feel

The hardware shapes the UX more than any style guide does: a 128×64 1-bit screen, a 5-way
D-pad, no pointer, and an audience of hobbyists who expect the device to behave like a
purpose-built tool, not a smartphone.

## Visual language

- Design for 1 bit per pixel — no anti-aliasing, no gradients, no subtle grays. Favor dense
  pixel fonts, bold icons, and high-contrast layout over fine detail.
- Screen space is scarce. Default to the standard GUI modules (submenu, list, dialog,
  widget) rather than custom-drawn screens; they already handle scrolling, selection, and
  layout within the 128×64 budget.
- Motion is minimal. Apps favor instant transitions and short animations over flourish —
  consistent with a device that's meant to boot and respond immediately.

## Interaction model

- Navigation is exclusively the D-pad: up/down/left/right to move or adjust, OK to
  select/confirm, Back to cancel or go up one level. There is no touch and no pointer.
- The typical app is a scrollable top-level menu (a `Submenu`) that drills into scenes —
  closer to an old feature-phone menu or a stack of wizard screens than to a modern app's
  navigation.
- A long press on Back commonly means "exit to the apps menu" — treat it as an escape
  hatch, not just "one level up."
- Confirmation is multi-modal by convention: a short vibration plus a single tone usually
  accompanies a meaningful event (e.g. a successful NFC read), because a user glancing at
  the device may not be looking at the screen at that exact moment.

## Lifecycle expectations

- Apps are launched from the Apps menu, run in the foreground, and exit — they are not
  long-running background daemons. The official firmware runs one user app in the
  foreground at a time, alongside system services.
- There is no OS-level suspend/resume between app switches. A well-behaved app persists
  anything the user would expect to survive (a capture, a saved signal, a setting) to the
  SD card explicitly before it exits, rather than assuming it can pick up where it left off.
- Single-purpose over suite: the ecosystem favors small, focused tools (one app per
  protocol or task) over multi-function apps, which also keeps each app's menu tree
  shallow enough to navigate with a D-pad.
