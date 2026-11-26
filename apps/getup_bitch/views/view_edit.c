#include "view_edit.h"
#include "../getup_bitch_app.h"
#include "../alarm.h"

#include <stdio.h>

/* Row order, fixed - used both when populating and when the change
   callback figures out which field was touched. */
enum {
    FieldEnabled = 0,
    FieldHour,
    FieldMinute,
    FieldRepeats,
    FieldDayMon,
    FieldDaySun = FieldDayMon + 6,
    FieldCount,
};

static Alarm* current_alarm(GetupApp* app) {
    if(app->editing_index < 0) return NULL;
    return &app->alarms[app->editing_index];
}

static void change_callback(VariableItem* item) {
    GetupApp* app = variable_item_get_context(item);
    Alarm* alarm = current_alarm(app);
    if(!alarm) return;

    uint8_t field = variable_item_list_get_selected_item_index(app->edit_view);
    uint8_t value = variable_item_get_current_value_index(item);
    char text[16];

    if(field == FieldEnabled) {
        alarm->enabled = value != 0;
        variable_item_set_current_value_text(item, alarm->enabled ? "On" : "Off");
    } else if(field == FieldHour) {
        alarm->hour = value;
        snprintf(text, sizeof(text), "%02u", alarm->hour);
        variable_item_set_current_value_text(item, text);
    } else if(field == FieldMinute) {
        alarm->minute = value;
        snprintf(text, sizeof(text), "%02u", alarm->minute);
        variable_item_set_current_value_text(item, text);
    } else if(field == FieldRepeats) {
        alarm->repetitions = (uint8_t)(GETUP_MIN_REPETITIONS + value);
        snprintf(text, sizeof(text), "%u", alarm->repetitions);
        variable_item_set_current_value_text(item, text);
    } else if(field >= FieldDayMon && field <= FieldDaySun) {
        uint8_t day = (uint8_t)(field - FieldDayMon);
        uint8_t bit = (uint8_t)(1u << day);
        if(value) {
            alarm->weekday_mask |= bit;
        } else {
            alarm->weekday_mask &= (uint8_t)~bit;
        }
        variable_item_set_current_value_text(item, value ? "On" : "Off");
    }
}

VariableItemList* view_edit_alloc(GetupApp* app) {
    VariableItemList* list = variable_item_list_alloc();
    UNUSED(app);
    return list;
}

void view_edit_free(VariableItemList* list) {
    variable_item_list_free(list);
}

void view_edit_populate(VariableItemList* list, GetupApp* app, uint8_t alarm_index) {
    app->editing_index = (int8_t)alarm_index;
    const Alarm* alarm = &app->alarms[alarm_index];
    char text[16];
    VariableItem* item;

    variable_item_list_reset(list);

    item = variable_item_list_add(list, "Enabled", 2, change_callback, app);
    variable_item_set_current_value_index(item, alarm->enabled ? 1 : 0);
    variable_item_set_current_value_text(item, alarm->enabled ? "On" : "Off");

    item = variable_item_list_add(list, "Hour", 24, change_callback, app);
    variable_item_set_current_value_index(item, alarm->hour);
    snprintf(text, sizeof(text), "%02u", alarm->hour);
    variable_item_set_current_value_text(item, text);

    item = variable_item_list_add(list, "Minute", 60, change_callback, app);
    variable_item_set_current_value_index(item, alarm->minute);
    snprintf(text, sizeof(text), "%02u", alarm->minute);
    variable_item_set_current_value_text(item, text);

    item = variable_item_list_add(
        list, "Repeats", (uint8_t)(GETUP_MAX_REPETITIONS - GETUP_MIN_REPETITIONS + 1), change_callback, app);
    variable_item_set_current_value_index(item, (uint8_t)(alarm->repetitions - GETUP_MIN_REPETITIONS));
    snprintf(text, sizeof(text), "%u", alarm->repetitions);
    variable_item_set_current_value_text(item, text);

    for(uint8_t d = 0; d < 7; d++) {
        bool on = alarm->weekday_mask & (1u << d);
        item = variable_item_list_add(list, alarm_weekday_abbrev[d], 2, change_callback, app);
        variable_item_set_current_value_index(item, on ? 1 : 0);
        variable_item_set_current_value_text(item, on ? "On" : "Off");
    }

    variable_item_list_set_selected_item(list, 0);
}
