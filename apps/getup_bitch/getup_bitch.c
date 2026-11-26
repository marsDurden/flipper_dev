#include "getup_bitch_app.h"
#include "alarm.h"
#include "views/view_clock.h"
#include "views/view_list.h"
#include "views/view_edit.h"
#include "views/view_ringing.h"

#include <furi_hal_speaker.h>
#include <furi_hal_vibro.h>
#include <furi_hal_random.h>
#include <notification/notification_messages.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void getup_goto_clock(GetupApp* app) {
    app->current_view = GetupViewClock;
    view_dispatcher_switch_to_view(app->view_dispatcher, GetupViewClock);
}

void getup_goto_list(GetupApp* app) {
    view_list_rebuild(app->list_view, app);
    app->current_view = GetupViewList;
    view_dispatcher_switch_to_view(app->view_dispatcher, GetupViewList);
}

void getup_goto_edit(GetupApp* app, uint8_t alarm_index) {
    view_edit_populate(app->edit_view, app, alarm_index);
    app->current_view = GetupViewEdit;
    view_dispatcher_switch_to_view(app->view_dispatcher, GetupViewEdit);
}

static void stop_speaker_if_mine(GetupApp* app) {
    if(app->speaker_owned) {
        furi_hal_speaker_stop();
        furi_hal_speaker_release();
        app->speaker_owned = false;
    }
}

static void fire_pre_buzz(GetupApp* app, uint8_t occurrence_number) {
    uint8_t duration_ticks = occurrence_number;
    if(duration_ticks > 4) duration_ticks = 4;
    app->prebuzz_ticks_remaining = duration_ticks;

    furi_hal_vibro_on(true);
    if(furi_hal_speaker_acquire(1000)) {
        app->speaker_owned = true;
        furi_hal_speaker_start(880.0f, 0.6f);
    }
}

static void start_ringing(GetupApp* app, uint8_t alarm_index) {
    app->ringing = true;
    app->fired_alarm_index = alarm_index;
    app->ring_ticks = 0;
    app->prebuzz_ticks_remaining = 0;
    furi_hal_vibro_on(false);
    stop_speaker_if_mine(app); /* in case a pre-buzz was still winding down */
    notification_message(app->notifications, &sequence_display_backlight_enforce_on);

    /* "34 x 2 = ?" style problem: a 2-digit number times a small multiplier,
       so the product always has a real prefix plus one adjustable digit. */
    int a = 11 + (int)(furi_hal_random_get() % 85); /* 11..95 */
    int b = 2 + (int)(furi_hal_random_get() % 7); /* 2..8 */
    int answer = a * b;
    app->math_answer_last_digit = answer % 10;

    int wrong_start;
    do {
        wrong_start = (int)(furi_hal_random_get() % 10);
    } while(wrong_start == app->math_answer_last_digit);

    char prefix[8];
    snprintf(prefix, sizeof(prefix), "%d", answer / 10);

    view_ringing_set_problem(
        app->ringing_view,
        app->alarms[alarm_index].hour,
        app->alarms[alarm_index].minute,
        a,
        b,
        prefix,
        wrong_start);

    app->current_view = GetupViewRinging;
    view_dispatcher_switch_to_view(app->view_dispatcher, GetupViewRinging);
}

static void stop_ringing(GetupApp* app) {
    app->ringing = false;
    furi_hal_vibro_on(false);
    stop_speaker_if_mine(app);
    notification_message(app->notifications, &sequence_display_backlight_enforce_auto);
    getup_goto_clock(app);
}

void getup_math_submit(GetupApp* app) {
    int guess = view_ringing_get_guess(app->ringing_view);
    if(guess == app->math_answer_last_digit) {
        stop_ringing(app);
    } else {
        app->wrong_flash_ticks = 2;
        view_ringing_set_wrong_flash(app->ringing_view, true);
    }
}

static void update_ringing(GetupApp* app) {
    app->ring_ticks++;
    bool high_tone = (app->ring_ticks % 2) == 0;
    furi_hal_vibro_on(high_tone);
    if(app->speaker_owned || furi_hal_speaker_acquire(1000)) {
        app->speaker_owned = true;
        furi_hal_speaker_start(high_tone ? 1200.0f : 700.0f, 0.8f);
    }

    if(app->wrong_flash_ticks > 0) {
        app->wrong_flash_ticks--;
        if(app->wrong_flash_ticks == 0) {
            view_ringing_set_wrong_flash(app->ringing_view, false);
        }
    }

    if(app->ring_ticks >= GETUP_RING_TIMEOUT_TICKS) {
        /* Safety net: don't let an unattended alarm siren all night. */
        stop_ringing(app);
    }
}

static bool navigation_callback(void* context) {
    GetupApp* app = context;
    switch(app->current_view) {
    case GetupViewList:
        getup_goto_clock(app);
        return true;
    case GetupViewEdit:
        if(app->editing_index >= 0) {
            app->storage_ok = alarm_storage_save(app);
            /* Force a re-evaluation of this one alarm on the next tick, in
               case it was just enabled/retimed for later today. */
            app->schedule[app->editing_index].day = 0;
        }
        getup_goto_list(app);
        return true;
    case GetupViewClock:
    default:
        view_dispatcher_stop(app->view_dispatcher);
        return true;
    }
}

static void engine_tick(void* context) {
    GetupApp* app = context;
    DateTime now;
    furi_hal_rtc_get_datetime(&now);
    uint32_t now_ts = furi_hal_rtc_get_timestamp();

    alarm_engine_refresh_schedules(app, &now, now_ts);

    if(app->ringing) {
        update_ringing(app);
        return;
    }

    uint8_t armed_count = 0;
    for(uint8_t i = 0; i < GETUP_MAX_ALARMS; i++) {
        if(app->schedule[i].armed) armed_count++;
    }

    for(uint8_t i = 0; i < GETUP_MAX_ALARMS; i++) {
        AlarmSchedule* sched = &app->schedule[i];
        if(!sched->armed) continue;
        while(sched->next_idx < sched->count && sched->buzz_ts[sched->next_idx] <= now_ts) {
            uint8_t fired_idx = sched->next_idx;
            sched->next_idx++;
            bool final = (fired_idx == sched->count - 1);
            if(final) {
                sched->armed = false;
                start_ringing(app, i);
                /* Entering the ringing view mid-loop is fine - the loop just
                   won't find any more due buzzes this tick since we return
                   via the ringing branch above on the next call. */
                return;
            }
            fire_pre_buzz(app, (uint8_t)(fired_idx + 1));
        }
    }

    if(app->prebuzz_ticks_remaining > 0) {
        app->prebuzz_ticks_remaining--;
        if(app->prebuzz_ticks_remaining == 0) {
            furi_hal_vibro_on(false);
            stop_speaker_if_mine(app);
        }
    }

    if(app->current_view == GetupViewClock) {
        view_clock_update(app->clock_view, now.hour, now.minute, armed_count, app->storage_ok);
    }
}

int32_t getup_bitch_app(void* p) {
    UNUSED(p);
    GetupApp* app = malloc(sizeof(GetupApp));
    memset(app, 0, sizeof(GetupApp));
    app->editing_index = -1;
    app->current_view = GetupViewClock;

    app->gui = furi_record_open(RECORD_GUI);
    app->notifications = furi_record_open(RECORD_NOTIFICATION);
    app->storage = furi_record_open(RECORD_STORAGE);

    alarm_init_weekday_table(app);
    app->storage_ok = alarm_storage_load(app);

    app->clock_view = view_clock_alloc();
    view_set_context(app->clock_view, app);
    app->list_view = view_list_alloc(app);
    app->edit_view = view_edit_alloc(app);
    app->ringing_view = view_ringing_alloc();
    view_set_context(app->ringing_view, app);

    app->view_dispatcher = view_dispatcher_alloc();
    view_dispatcher_set_event_callback_context(app->view_dispatcher, app);
    view_dispatcher_set_navigation_event_callback(app->view_dispatcher, navigation_callback);
    view_dispatcher_set_tick_event_callback(app->view_dispatcher, engine_tick, 1000);

    view_dispatcher_add_view(app->view_dispatcher, GetupViewClock, app->clock_view);
    view_dispatcher_add_view(app->view_dispatcher, GetupViewList, submenu_get_view(app->list_view));
    view_dispatcher_add_view(
        app->view_dispatcher, GetupViewEdit, variable_item_list_get_view(app->edit_view));
    view_dispatcher_add_view(app->view_dispatcher, GetupViewRinging, app->ringing_view);

    view_clock_update(app->clock_view, 0, 0, 0, app->storage_ok);
    view_dispatcher_switch_to_view(app->view_dispatcher, GetupViewClock);

    view_dispatcher_attach_to_gui(app->view_dispatcher, app->gui, ViewDispatcherTypeFullscreen);
    view_dispatcher_run(app->view_dispatcher);

    alarm_storage_save(app);

    view_dispatcher_remove_view(app->view_dispatcher, GetupViewClock);
    view_dispatcher_remove_view(app->view_dispatcher, GetupViewList);
    view_dispatcher_remove_view(app->view_dispatcher, GetupViewEdit);
    view_dispatcher_remove_view(app->view_dispatcher, GetupViewRinging);
    view_dispatcher_free(app->view_dispatcher);

    view_clock_free(app->clock_view);
    view_list_free(app->list_view);
    view_edit_free(app->edit_view);
    view_ringing_free(app->ringing_view);

    furi_record_close(RECORD_STORAGE);
    furi_record_close(RECORD_NOTIFICATION);
    furi_record_close(RECORD_GUI);
    free(app);
    return 0;
}
