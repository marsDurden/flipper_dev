#pragma once

#include "getup_bitch_app.h"

#define GETUP_ALARM_FILE_PATH APP_DATA_PATH("alarms.bin")

/* "Mo".."Su", indexed by the canonical 0=Monday..6=Sunday bit used in
   Alarm.weekday_mask (not by DateTime.weekday's own raw numbering). */
extern const char* const alarm_weekday_abbrev[7];

/* Builds app->weekday_raw_to_index[] once, at startup. Maps the firmware's
   own DateTime.weekday numbering (1-7, undocumented order) to our canonical
   0=Monday..6=Sunday by asking datetime_timestamp_to_datetime() what it
   calls a known Monday, rather than assuming an order and risking "weekdays
   only" alarms firing on the wrong days. */
void alarm_init_weekday_table(GetupApp* app);

void alarm_set_defaults(Alarm* alarm);

/* Loads alarms from storage into app->alarms, falling back to defaults for
   every slot if there's nothing to load or storage isn't available (no SD
   card). Returns false in that fallback case - not fatal, just unsaved. */
bool alarm_storage_load(GetupApp* app);

/* Saves app->alarms to storage. Returns false if it couldn't be written
   (e.g. no SD card); callers keep running on the in-RAM state either way. */
bool alarm_storage_save(GetupApp* app);

/* Re-evaluates and (re)generates today's firing schedule for every alarm
   whose schedule hasn't been evaluated yet today. Cheap - safe to call every
   engine tick. */
void alarm_engine_refresh_schedules(GetupApp* app, const DateTime* now, uint32_t now_ts);
