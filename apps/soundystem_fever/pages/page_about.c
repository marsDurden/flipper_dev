#include "page_about.h"
#include "../soundystem_fever_app.h"

#include <gui/canvas.h>

/* Keep this in sync with application.fam's fap_version by hand - there's no
   build-time substitution set up for this yet. */
#define SOUNDYSTEM_VERSION "v0.1"

static void draw(Canvas* canvas, SoundystemApp* app) {
    UNUSED(app);
    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str(canvas, 2, SOUNDYSTEM_BODY_TOP + 9, "Soundystem Fever " SOUNDYSTEM_VERSION);
    canvas_draw_str(canvas, 2, SOUNDYSTEM_BODY_TOP + 19, "Audio level analyzer for");
    canvas_draw_str(canvas, 2, SOUNDYSTEM_BODY_TOP + 29, "gain-staging a cascade of");
    canvas_draw_str(canvas, 2, SOUNDYSTEM_BODY_TOP + 39, "line-level gear via GPIO.");
    canvas_draw_str(canvas, 2, SOUNDYSTEM_BODY_TOP + 49, "See README.md / PLAN.md.");
}

const SoundystemPage page_about = {
    .name = "About",
    .draw = draw,
    .input = NULL,
    .on_enter = NULL,
};
