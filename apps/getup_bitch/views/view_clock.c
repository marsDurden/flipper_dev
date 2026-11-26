#include "view_clock.h"
#include "../getup_bitch_app.h"

#include <input/input.h>
#include <stdio.h>

typedef struct {
    uint8_t hour;
    uint8_t minute;
    uint8_t armed_count;
    bool storage_ok;
} ClockModel;

static void clock_draw(Canvas* canvas, void* model_ptr) {
    const ClockModel* model = model_ptr;
    char buf[32];

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 9, AlignCenter, AlignBottom, "Getup Bitch");
    canvas_draw_line(canvas, 0, 11, 127, 11);

    canvas_set_font(canvas, FontBigNumbers);
    snprintf(buf, sizeof(buf), "%02u:%02u", model->hour, model->minute);
    canvas_draw_str_aligned(canvas, 64, 40, AlignCenter, AlignBottom, buf);

    canvas_set_font(canvas, FontSecondary);
    snprintf(buf, sizeof(buf), "%u armed today - OK: alarms", model->armed_count);
    canvas_draw_str_aligned(canvas, 64, 52, AlignCenter, AlignBottom, buf);

    canvas_draw_str_aligned(
        canvas,
        64,
        63,
        AlignCenter,
        AlignBottom,
        model->storage_ok ? "keep this app open to ring!" : "no SD card - not saved!");
}

static bool clock_input(InputEvent* event, void* context) {
    GetupApp* app = context;
    if(event->type == InputTypeShort && event->key == InputKeyOk) {
        getup_goto_list(app);
        return true;
    }
    return false; /* Back falls through to the ViewDispatcher's navigation callback */
}

View* view_clock_alloc(void) {
    View* view = view_alloc();
    view_allocate_model(view, ViewModelTypeLockFree, sizeof(ClockModel));
    view_set_draw_callback(view, clock_draw);
    view_set_input_callback(view, clock_input);
    return view;
}

void view_clock_free(View* view) {
    view_free(view);
}

void view_clock_update(View* view, uint8_t hour, uint8_t minute, uint8_t armed_count, bool storage_ok) {
    with_view_model(
        view,
        ClockModel * model,
        {
            model->hour = hour;
            model->minute = minute;
            model->armed_count = armed_count;
            model->storage_ok = storage_ok;
        },
        true);
}
