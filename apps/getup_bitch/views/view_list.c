#include "view_list.h"
#include "../getup_bitch_app.h"
#include "../alarm.h"

#include <stdio.h>

static void list_item_selected(void* context, uint32_t index) {
    GetupApp* app = context;
    getup_goto_edit(app, (uint8_t)index);
}

Submenu* view_list_alloc(GetupApp* app) {
    Submenu* submenu = submenu_alloc();
    submenu_set_header(submenu, "Getup Bitch - Alarms");
    for(uint8_t i = 0; i < GETUP_MAX_ALARMS; i++) {
        submenu_add_item(submenu, "...", i, list_item_selected, app);
    }
    return submenu;
}

void view_list_free(Submenu* submenu) {
    submenu_free(submenu);
}

void view_list_rebuild(Submenu* submenu, GetupApp* app) {
    char label[48];
    char days[8];

    for(uint8_t i = 0; i < GETUP_MAX_ALARMS; i++) {
        const Alarm* alarm = &app->alarms[i];
        if(!alarm->enabled) {
            snprintf(label, sizeof(label), "%u. -- off", i + 1);
        } else {
            size_t pos = 0;
            for(uint8_t d = 0; d < 7 && pos + 2 < sizeof(days); d++) {
                bool on = alarm->weekday_mask & (1u << d);
                days[pos++] = on ? alarm_weekday_abbrev[d][0] : '-';
            }
            days[pos] = '\0';
            snprintf(
                label,
                sizeof(label),
                "%u. %02u:%02u x%u %s",
                i + 1,
                alarm->hour,
                alarm->minute,
                alarm->repetitions,
                days);
        }
        submenu_change_item_label(submenu, i, label);
    }
}
