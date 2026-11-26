#include "page_docs.h"
#include "../soundystem_fever_app.h"

#include <gui/canvas.h>

/*
 * Condensed from ../docs/audio-levels.md. Up/Down pages through these
 * screens (clamped, not wrapping - Left/Right is reserved for the top-level
 * page carousel, which does wrap). Keep each line short: at FontSecondary,
 * roughly 22 characters is the practical limit for the 128px-wide display.
 */
static const char* const docs_screens[][4] = {
    {"dB is a ratio, not a", "value: dB=20*log10", "(V1/V2). Needs a", "reference: dBu, dBV..."},
    {"0dBu = 0.775V RMS", "(pro, bridging ref).", "0dBV = 1V RMS", "(consumer ref)."},
    {"-10dBV (consumer) vs", "+4dBu (pro): about an", "11.8dB nominal gap -", "a common mismatch."},
    {"Modern gear bridges:", "low output Z drives", "high input Z. Old", "600ohm match: history."},
};
#define DOCS_SCREEN_COUNT (sizeof(docs_screens) / sizeof(docs_screens[0]))

static void draw(Canvas* canvas, SoundystemApp* app) {
    canvas_set_font(canvas, FontSecondary);
    uint8_t screen = app->docs_subpage;
    if(screen >= DOCS_SCREEN_COUNT) screen = 0;

    for(size_t i = 0; i < 4; i++) {
        canvas_draw_str(canvas, 2, SOUNDYSTEM_BODY_TOP + 9 + (uint8_t)(i * 10), docs_screens[screen][i]);
    }
}

static void input(InputEvent* event, SoundystemApp* app) {
    if(event->key == InputKeyUp) {
        if(app->docs_subpage > 0) app->docs_subpage--;
    } else if(event->key == InputKeyDown) {
        if((size_t)app->docs_subpage + 1 < DOCS_SCREEN_COUNT) app->docs_subpage++;
    }
}

static void on_enter(SoundystemApp* app) {
    app->docs_subpage = 0;
}

const SoundystemPage page_docs = {
    .name = "Audio Theory",
    .draw = draw,
    .input = input,
    .on_enter = on_enter,
};
