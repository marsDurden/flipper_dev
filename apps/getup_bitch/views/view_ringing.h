#pragma once

#include <gui/view.h>

View* view_ringing_alloc(void);
void view_ringing_free(View* view);

/* Arms the math-dismissal screen for one firing. `answer_prefix` is every
   digit of a*b except the last, as text; `initial_guess_digit` (0-9) is
   where the adjustable last digit starts (deliberately wrong). */
void view_ringing_set_problem(
    View* view,
    uint8_t alarm_hour,
    uint8_t alarm_minute,
    int a,
    int b,
    const char* answer_prefix,
    int initial_guess_digit);

int view_ringing_get_guess(View* view);
void view_ringing_set_wrong_flash(View* view, bool on);
