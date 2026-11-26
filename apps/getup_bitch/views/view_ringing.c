#include "view_ringing.h"
#include "../getup_bitch_app.h"

#include <input/input.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    uint8_t alarm_hour;
    uint8_t alarm_minute;
    int math_a;
    int math_b;
    char answer_prefix[8];
    int guess_digit;
    bool wrong_flash;
} RingingModel;

static void ringing_draw(Canvas* canvas, void* model_ptr) {
    const RingingModel* model = model_ptr;
    char buf[32];

    canvas_clear(canvas);
    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str_aligned(canvas, 64, 10, AlignCenter, AlignBottom, "GETUP BITCH");

    canvas_set_font(canvas, FontSecondary);
    snprintf(buf, sizeof(buf), "alarm was %02u:%02u", model->alarm_hour, model->alarm_minute);
    canvas_draw_str_aligned(canvas, 64, 20, AlignCenter, AlignBottom, buf);

    canvas_set_font(canvas, FontPrimary);
    snprintf(
        buf,
        sizeof(buf),
        "%d x %d = %s%d",
        model->math_a,
        model->math_b,
        model->answer_prefix,
        model->guess_digit);
    canvas_draw_str_aligned(canvas, 64, 38, AlignCenter, AlignBottom, buf);

    canvas_set_font(canvas, FontSecondary);
    canvas_draw_str_aligned(canvas, 64, 50, AlignCenter, AlignBottom, "Up/Down: last digit");
    canvas_draw_str_aligned(canvas, 64, 60, AlignCenter, AlignBottom, "OK: submit");

    if(model->wrong_flash) {
        canvas_draw_box(canvas, 0, 0, 128, 64);
        canvas_invert_color(canvas);
        canvas_draw_str_aligned(canvas, 64, 32, AlignCenter, AlignBottom, "WRONG");
        canvas_invert_color(canvas);
    }
}

static bool ringing_input(InputEvent* event, void* context) {
    GetupApp* app = context;
    View* view = app->ringing_view;

    if(event->type == InputTypeShort) {
        if(event->key == InputKeyUp) {
            with_view_model(
                view, RingingModel * model, { model->guess_digit = (model->guess_digit + 1) % 10; }, true);
        } else if(event->key == InputKeyDown) {
            with_view_model(
                view,
                RingingModel * model,
                { model->guess_digit = (model->guess_digit + 9) % 10; },
                true);
        } else if(event->key == InputKeyOk) {
            getup_math_submit(app);
        }
    }
    /* Every input is consumed, including Back - there is no way out of this
       view but solving the problem. */
    return true;
}

View* view_ringing_alloc(void) {
    View* view = view_alloc();
    view_allocate_model(view, ViewModelTypeLockFree, sizeof(RingingModel));
    view_set_draw_callback(view, ringing_draw);
    view_set_input_callback(view, ringing_input);
    return view;
}

void view_ringing_free(View* view) {
    view_free(view);
}

void view_ringing_set_problem(
    View* view,
    uint8_t alarm_hour,
    uint8_t alarm_minute,
    int a,
    int b,
    const char* answer_prefix,
    int initial_guess_digit) {
    with_view_model(
        view,
        RingingModel * model,
        {
            model->alarm_hour = alarm_hour;
            model->alarm_minute = alarm_minute;
            model->math_a = a;
            model->math_b = b;
            strncpy(model->answer_prefix, answer_prefix, sizeof(model->answer_prefix) - 1);
            model->answer_prefix[sizeof(model->answer_prefix) - 1] = '\0';
            model->guess_digit = initial_guess_digit;
            model->wrong_flash = false;
        },
        true);
}

int view_ringing_get_guess(View* view) {
    int guess;
    with_view_model(view, RingingModel * model, { guess = model->guess_digit; }, false);
    return guess;
}

void view_ringing_set_wrong_flash(View* view, bool on) {
    with_view_model(view, RingingModel * model, { model->wrong_flash = on; }, true);
}
