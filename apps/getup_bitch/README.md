# Getup Bitch

A multi-alarm clock for Flipper Zero, for people who need more than one nudge to actually get
out of bed. Each of up to 8 alarms has its own time, weekly repeat days, and a repetition
count (3+): instead of one beep at the set time, it builds an escalating, randomized ladder
of buzzes in the run-up to it, and the final one only stops when you solve a math problem.

- The plan and build roadmap: [PLAN.md](PLAN.md)
- Repo-wide Flipper Zero background (hardware, firmware architecture, GPIO, limitations):
  [../../AGENTS.md](../../AGENTS.md)

Current status: builds cleanly with `ufbt` against real firmware (target 7, API 88.2). Not
yet tested on physical hardware — see PLAN.md for what that would involve.

## The one thing you must know before using this

**Flipper apps don't run in the background.** The official firmware runs exactly one
foreground app at a time, and a `.fap` gets no execution once you navigate away from it (see
[../../docs/limitations.md](../../docs/limitations.md)). That means this app can only ring an
alarm while it is itself the open, running app on screen.

In practice: launch Getup Bitch, leave it open (ideally with the Flipper plugged in to
charge), and don't press Back out to the main menu or launch anything else overnight. The
screen can dim/go dark on its own - the alarm engine keeps ticking underneath - but leaving
the app itself is the one thing that silently breaks it.

## How it works

- **Alarm list**: up to 8 slots, each shown as time + repeat days + repetition count, or
  `off`. Select one to edit it.
- **Edit screen**: enabled on/off, hour, minute, repetitions (3-10), and a toggle per weekday
  for weekly repeats.
- **The ladder**: when an alarm is due today, the app computes `repetitions - 1` pre-buzzes
  before the set time, spaced backward from it in roughly doubling gaps (e.g. ~2, ~4, ~8
  minutes before), each randomized by about ±30% so the pattern can't be learned and slept
  through. These are short, escalating vibration+tone bursts that fire on their own and
  aren't dismissible by any button - mashing OK doesn't make them stop early, that's the
  point. This schedule is regenerated fresh every day, never saved, so tomorrow's ladder is
  different again.
- **The final alarm**: at the exact time you set, a continuous siren (speaker + vibration,
  screen forced on) takes over the display. The only way to stop it is to solve a small
  multiplication problem shown with every digit of the answer filled in except the last one -
  Up/Down cycles that one digit, OK submits. Wrong answers just buzz and let you try again. If
  left unattended for 10 minutes it gives up on its own as a battery-safety net, not as a way
  out.
- Alarms persist to the app's own data file on the SD card; with no SD card they still work
  for the current session, they just won't survive relaunching the app (the clock screen
  flags this).

## Code layout

```
getup_bitch_app.h    - shared app struct (GetupApp), alarm/schedule types, cross-view
                        navigation helpers
getup_bitch.c        - entry point: ViewDispatcher wiring, the 1 Hz engine tick (schedule
                        checking, firing pre-buzzes/the final alarm, the siren), navigation
alarm.h/.c           - Alarm persistence (storage load/save) and the pure schedule-generation
                        logic (weekday-table bootstrap, today's buzz ladder)
views/
  view_clock.c/.h    - the idle clock face (default screen)
  view_list.c/.h      - the alarm list (a Submenu)
  view_edit.c/.h      - the per-alarm editor (a VariableItemList)
  view_ringing.c/.h   - the final-alarm/math-gate screen
```

## Loading the app on the device

The build runs in WSL (Linux), the device gets flashed from Windows. First get the `.fap`
from WSL into somewhere Windows can see it - from a WSL shell, either:

```
explorer.exe .                     # opens this folder in Windows Explorer
```

or browse to `\\wsl$\<your-distro-name>\home\node\projects\flipper_dev\apps\getup_bitch\dist\`
(or `\\wsl.localhost\...` on newer Windows) directly in Explorer.

**qFlipper:**

1. Install qFlipper from flipperzero.one if you don't have it, and connect the Flipper over
   USB-C. qFlipper should show it as connected.
2. Open qFlipper's **File Manager** (left sidebar).
3. Browse to `/ext/apps/Tools/` (this app's `fap_category` in `application.fam` is `Tools`;
   create the folder if it doesn't exist yet).
4. Drag `getup_bitch.fap` from the Windows Explorer window into that File Manager pane.
5. On the Flipper itself: **Apps → Tools → Getup Bitch** to launch it.
