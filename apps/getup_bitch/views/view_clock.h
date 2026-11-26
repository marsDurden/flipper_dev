#pragma once

#include <gui/view.h>

View* view_clock_alloc(void);
void view_clock_free(View* view);

/* Refreshes the live clock face. Called from the engine tick whenever this
   view is the one currently on screen. */
void view_clock_update(View* view, uint8_t hour, uint8_t minute, uint8_t armed_count, bool storage_ok);
