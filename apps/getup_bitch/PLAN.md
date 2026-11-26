# Plan / status

## Done

- App scaffolded and building cleanly against real firmware via `ufbt` (target 7, API 88.2,
  zero compiler warnings).
- Data model: 8 fixed alarm slots (hour, minute, enabled, weekday repeat mask, repetitions
  3-10), persisted to `APP_DATA_PATH("alarms.bin")` with a magic/version header; falls back
  to defaults (and flags `storage_ok = false` on the clock screen) if there's no SD card or
  no file yet.
- Weekday handling doesn't assume the firmware's `DateTime.weekday` numbering - it derives
  the mapping at startup from a known reference timestamp (see `alarm_init_weekday_table()`),
  so "weekdays only" can't silently fire on the wrong days if that numbering differs from
  what was assumed.
- Four-screen UI via `ViewDispatcher`: clock face → alarm list (`Submenu`) → alarm editor
  (`VariableItemList`) → ringing/math-gate screen (custom `View`), with Back centrally
  handled in `getup_bitch.c`'s navigation callback (list/edit pop back; clock exits).
- The alarm engine: a 1 Hz tick regenerates each alarm's today-schedule once per calendar day
  (randomized pre-buzz ladder + exact final time), fires pre-buzzes (vibration + short tone,
  escalating duration, non-dismissible) and the final siren (continuous, screen forced on via
  `NotificationApp`'s backlight-enforce sequence), gated behind a "last digit of a×b" math
  problem to dismiss, with a 10-minute auto-stop safety net.

## Not done / open questions

- **Never run on physical hardware.** Everything above is verified by reading the Flipper
  SDK headers and a clean `ufbt build`, not by `ufbt launch` on a real device. Before relying
  on this to actually wake you up, at minimum: set an alarm a couple minutes out and confirm
  the pre-buzz ladder, the final siren, backlight-forced-on, and math dismissal all behave as
  expected; check behavior with the SD card removed; check two alarms close together.
- No icon is set in `application.fam` (defaults to the firmware's generic FAP icon) - cosmetic,
  not functional.
- The siren is a simple two-tone alternation at the 1 Hz tick rate, not a richer waveform -
  effective but coarse; a faster tick (e.g. 250 ms) while ringing would make it punchier, at
  the cost of a bit more engine complexity (two tick rates instead of one).
- No per-alarm label/name field - editing only exposes time, repeat days, repetitions,
  enabled. Easy to add later via the firmware's `TextInput` keyboard module if wanted.
- No "test ring" action in the editor to preview the siren/math screen without waiting for a
  real alarm - would be a convenient addition.
