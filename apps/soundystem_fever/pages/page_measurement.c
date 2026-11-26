#include "page_measurement.h"
#include "../soundystem_fever_app.h"
#include "db_convert.h"

#include <gui/canvas.h>
#include <stdbool.h>
#include <stdio.h>

/*
 * No front end is built yet (see ../PLAN.md M2/M3) - this page intentionally
 * does not pretend to show a live reading. It shows what it will show once
 * ADC sampling exists, computed from the real db_convert module so the
 * conversion path is exercised end to end, clearly labeled as a fixed demo
 * value rather than a live measurement.
 */

/* Formats to one decimal place using only %d/%s - deliberately avoids %f:
   the firmware's printf may not support floating-point format specifiers. */
static void format_one_decimal(float value, char* out, size_t out_size) {
    bool negative = value < 0.0f;
    float av = negative ? -value : value;
    int whole = (int)av;
    int frac = (int)((av - (float)whole) * 10.0f + 0.5f);
    if(frac >= 10) {
        frac -= 10;
        whole += 1;
    }
    snprintf(out, out_size, "%s%d.%d", negative ? "-" : "", whole, frac);
}

static void draw(Canvas* canvas, SoundystemApp* app) {
    UNUSED(app);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, SOUNDYSTEM_BODY_TOP + 9, "No front end wired up yet.");
    canvas_draw_str(canvas, 2, SOUNDYSTEM_BODY_TOP + 19, "ADC sampling: see PLAN.md M3.");
    canvas_draw_str(canvas, 2, SOUNDYSTEM_BODY_TOP + 29, "Demo (fixed, not live):");

    /* Sized for the compiler's worst-case int width, not the realistic one -
       format_one_decimal's whole/frac are small in practice, but GCC's
       -Wformat-truncation reasons about the full int range. */
    char dbu_str[32];
    format_one_decimal(db_convert_volts_to_dbu(0.775f), dbu_str, sizeof(dbu_str));
    char line[64];
    snprintf(line, sizeof(line), "0.775V -> %s dBu", dbu_str);
    canvas_draw_str(canvas, 2, SOUNDYSTEM_BODY_TOP + 39, line);
}

const SoundystemPage page_measurement = {
    .name = "Measurement",
    .draw = draw,
    .input = NULL,
    .on_enter = NULL,
};
