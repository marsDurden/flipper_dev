#pragma once

#include <furi.h>
#include <furi_hal_rtc.h>
#include <gui/gui.h>
#include <gui/view.h>
#include <gui/view_dispatcher.h>
#include <gui/modules/submenu.h>
#include <gui/modules/variable_item_list.h>
#include <notification/notification.h>
#include <storage/storage.h>

#define GETUP_MAX_ALARMS         8
#define GETUP_MIN_REPETITIONS    3
#define GETUP_MAX_REPETITIONS    10
#define GETUP_RING_TIMEOUT_TICKS 600 /* 10 min of unattended ringing at the 1 Hz engine tick, then it gives up */

typedef enum {
    GetupViewClock,
    GetupViewList,
    GetupViewEdit,
    GetupViewRinging,
} GetupViewId;

/* One configured alarm slot - persisted as-is, see alarm.c. */
typedef struct {
    uint8_t hour; /* 0-23 */
    uint8_t minute; /* 0-59 */
    bool enabled;
    uint8_t weekday_mask; /* bit 0 = Monday .. bit 6 = Sunday - see alarm_init_weekday_table() */
    uint8_t repetitions; /* total buzzes incl. the final one, GETUP_MIN_REPETITIONS..GETUP_MAX_REPETITIONS */
} Alarm;

/* Today's firing schedule for one alarm - runtime only, regenerated (with
   fresh randomization) once per calendar day, never persisted. */
typedef struct {
    uint8_t day; /* day-of-month this was last evaluated for, 0 = never */
    bool armed; /* true if today's occurrence is still pending */
    uint32_t buzz_ts[GETUP_MAX_REPETITIONS]; /* ascending unix timestamps, last = the exact alarm time */
    uint8_t count; /* valid entries in buzz_ts, == alarm.repetitions */
    uint8_t next_idx; /* index of the next buzz not yet fired */
} AlarmSchedule;

typedef struct GetupApp GetupApp;

struct GetupApp {
    Gui* gui;
    NotificationApp* notifications;
    Storage* storage;
    ViewDispatcher* view_dispatcher;

    View* clock_view;
    Submenu* list_view;
    VariableItemList* edit_view;
    View* ringing_view;

    GetupViewId current_view;
    bool storage_ok;

    Alarm alarms[GETUP_MAX_ALARMS];
    AlarmSchedule schedule[GETUP_MAX_ALARMS];
    uint8_t weekday_raw_to_index[8]; /* indexed by DateTime.weekday (1-7) */

    int8_t editing_index; /* alarm slot bound to edit_view, -1 if none */

    bool ringing;
    uint8_t fired_alarm_index;
    uint16_t ring_ticks;
    uint8_t prebuzz_ticks_remaining;
    bool speaker_owned;

    int math_answer_last_digit;
    uint8_t wrong_flash_ticks;
};

/* Cross-view navigation helpers, implemented in getup_bitch.c. Views call
   these instead of touching the ViewDispatcher directly, so each view stays
   self-contained. */
void getup_goto_clock(GetupApp* app);
void getup_goto_list(GetupApp* app);
void getup_goto_edit(GetupApp* app, uint8_t alarm_index);
void getup_math_submit(GetupApp* app);
