#pragma once

#include <gui/modules/variable_item_list.h>

typedef struct GetupApp GetupApp;

VariableItemList* view_edit_alloc(GetupApp* app);
void view_edit_free(VariableItemList* list);

/* (Re)builds every row from app->alarms[alarm_index] - call whenever
   entering this view for a given alarm slot. */
void view_edit_populate(VariableItemList* list, GetupApp* app, uint8_t alarm_index);
