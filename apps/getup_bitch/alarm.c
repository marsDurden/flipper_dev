#include "alarm.h"

#include <datetime/datetime.h>
#include <furi_hal_random.h>
#include <string.h>

const char* const alarm_weekday_abbrev[7] = {"Mo", "Tu", "We", "Th", "Fr", "Sa", "Su"};

void alarm_init_weekday_table(GetupApp* app) {
    /* 2024-01-01 00:00:00 UTC was a Monday. Ask the firmware's own timestamp
       conversion what weekday value it assigns to that moment (and the six
       days after it), instead of guessing DateTime.weekday's numbering. */
    const uint32_t reference_monday_ts = 1704067200UL;
    for(uint8_t i = 0; i < 7; i++) {
        DateTime dt;
        datetime_timestamp_to_datetime(reference_monday_ts + (uint32_t)i * 86400UL, &dt);
        app->weekday_raw_to_index[dt.weekday] = i;
    }
}

void alarm_set_defaults(Alarm* alarm) {
    alarm->hour = 7;
    alarm->minute = 0;
    alarm->enabled = false;
    alarm->weekday_mask = 0x7F; /* every day */
    alarm->repetitions = GETUP_MIN_REPETITIONS;
}

typedef struct {
    uint16_t magic;
    uint8_t version;
    Alarm alarms[GETUP_MAX_ALARMS];
} AlarmFile;

#define ALARM_FILE_MAGIC   0x4742 /* "GB" */
#define ALARM_FILE_VERSION 1

bool alarm_storage_load(GetupApp* app) {
    for(uint8_t i = 0; i < GETUP_MAX_ALARMS; i++) {
        alarm_set_defaults(&app->alarms[i]);
    }

    File* file = storage_file_alloc(app->storage);
    bool ok = false;
    if(storage_file_open(file, GETUP_ALARM_FILE_PATH, FSAM_READ, FSOM_OPEN_EXISTING)) {
        AlarmFile data;
        size_t read = storage_file_read(file, &data, sizeof(data));
        if(read == sizeof(data) && data.magic == ALARM_FILE_MAGIC &&
           data.version == ALARM_FILE_VERSION) {
            memcpy(app->alarms, data.alarms, sizeof(app->alarms));
            ok = true;
        }
    }
    storage_file_close(file);
    storage_file_free(file);
    return ok;
}

bool alarm_storage_save(GetupApp* app) {
    AlarmFile data;
    data.magic = ALARM_FILE_MAGIC;
    data.version = ALARM_FILE_VERSION;
    memcpy(data.alarms, app->alarms, sizeof(app->alarms));

    File* file = storage_file_alloc(app->storage);
    bool ok = false;
    if(storage_file_open(file, GETUP_ALARM_FILE_PATH, FSAM_WRITE, FSOM_CREATE_ALWAYS)) {
        ok = storage_file_write(file, &data, sizeof(data)) == sizeof(data);
    }
    storage_file_close(file);
    storage_file_free(file);
    return ok;
}

/* Builds today's buzz schedule for one alarm: `repetitions - 1` pre-buzzes
   geometrically spaced before `target_ts` (doubling going backward), each
   jittered +/-30%, plus the exact target as the final entry.

   Consecutive nominal offsets differ by a factor of 2, and +/-30% jitter
   can't flip two values that far apart: the smaller nominal's jittered max
   is nominal*1.3, the larger nominal's (= 2x) jittered min is
   nominal*2*0.7 = nominal*1.4 > nominal*1.3. So the result comes out already
   ascending in time - no sort needed. */
static void generate_schedule(AlarmSchedule* sched, const Alarm* alarm, uint32_t target_ts) {
    uint8_t reps = alarm->repetitions;
    uint8_t pre_count = (uint8_t)(reps - 1);
    const uint32_t base_seconds = 120; /* nearest pre-buzz: ~2 minutes before the target */

    for(uint8_t k = 0; k < pre_count; k++) {
        uint8_t exponent = (uint8_t)(pre_count - 1 - k);
        uint32_t nominal = base_seconds << exponent;
        int32_t jitter_range = (int32_t)(nominal * 30 / 100);
        int32_t jitter = 0;
        if(jitter_range > 0) {
            jitter = (int32_t)(furi_hal_random_get() % (uint32_t)(2 * jitter_range + 1)) -
                     jitter_range;
        }
        int32_t offset = (int32_t)nominal + jitter;
        if(offset < 20) offset = 20;
        sched->buzz_ts[k] = ((uint32_t)offset < target_ts) ? target_ts - (uint32_t)offset : 0;
    }
    sched->buzz_ts[pre_count] = target_ts;
    sched->count = reps;
    sched->next_idx = 0;
    sched->armed = true;
}

void alarm_engine_refresh_schedules(GetupApp* app, const DateTime* now, uint32_t now_ts) {
    for(uint8_t i = 0; i < GETUP_MAX_ALARMS; i++) {
        AlarmSchedule* sched = &app->schedule[i];
        if(sched->day == now->day) continue; /* already evaluated today */
        sched->day = now->day;

        const Alarm* alarm = &app->alarms[i];
        if(!alarm->enabled) {
            sched->armed = false;
            continue;
        }
        uint8_t wd = app->weekday_raw_to_index[now->weekday];
        if(!(alarm->weekday_mask & (1u << wd))) {
            sched->armed = false;
            continue;
        }

        DateTime target = *now;
        target.hour = alarm->hour;
        target.minute = alarm->minute;
        target.second = 0;
        uint32_t target_ts = datetime_datetime_to_timestamp(&target);
        if(target_ts <= now_ts) {
            sched->armed = false; /* already passed today */
            continue;
        }
        generate_schedule(sched, alarm, target_ts);
    }
}
