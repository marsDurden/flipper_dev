#include "soundystem_fever_app.h"
#include "pages/page.h"
#include "pages/page_measurement.h"
#include "pages/page_docs.h"
#include "pages/page_about.h"

#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>
#include <stdlib.h>

/*
 * The carousel: to add a page, write pages/page_<name>.c/.h implementing
 * SoundystemPage (see pages/page.h) and add one line here. Everything else -
 * title bar, Left/Right navigation with wraparound, the page-dot indicator -
 * is generic and doesn't change.
 */
static const SoundystemPage* const pages[] = {
    &page_measurement,
    &page_docs,
    &page_about,
};
#define PAGE_COUNT (sizeof(pages) / sizeof(pages[0]))

static void draw_page_chrome(Canvas* canvas, SoundystemApp* app) {
    const SoundystemPage* page = pages[app->page_index];

    canvas_set_font(canvas, FontPrimary);
    canvas_draw_str(canvas, 2, 9, "<");
    canvas_draw_str(canvas, 122, 9, ">");
    canvas_draw_str_aligned(canvas, 64, 9, AlignCenter, AlignBottom, page->name);
    canvas_draw_line(canvas, 0, 11, 127, 11);

    if(page->draw) {
        page->draw(canvas, app);
    }

    uint8_t spacing = 7;
    uint8_t total_width = (uint8_t)((PAGE_COUNT - 1) * spacing);
    uint8_t start_x = (uint8_t)(64 - total_width / 2);
    for(size_t i = 0; i < PAGE_COUNT; i++) {
        uint8_t x = (uint8_t)(start_x + i * spacing);
        if(i == app->page_index) {
            canvas_draw_disc(canvas, x, 61, 2);
        } else {
            canvas_draw_circle(canvas, x, 61, 2);
        }
    }
}

static void draw_callback(Canvas* canvas, void* ctx) {
    SoundystemApp* app = ctx;
    canvas_clear(canvas);
    draw_page_chrome(canvas, app);
}

static void input_callback(InputEvent* input_event, void* ctx) {
    FuriMessageQueue* event_queue = ctx;
    furi_message_queue_put(event_queue, input_event, FuriWaitForever);
}

static void goto_page(SoundystemApp* app, size_t new_index) {
    app->page_index = new_index;
    const SoundystemPage* page = pages[app->page_index];
    if(page->on_enter) {
        page->on_enter(app);
    }
}

int32_t soundystem_fever_app(void* p) {
    UNUSED(p);
    SoundystemApp* app = malloc(sizeof(SoundystemApp));
    app->event_queue = furi_message_queue_alloc(8, sizeof(InputEvent));
    app->view_port = view_port_alloc();
    app->running = true;
    app->page_index = 0;
    app->docs_subpage = 0;

    view_port_draw_callback_set(app->view_port, draw_callback, app);
    view_port_input_callback_set(app->view_port, input_callback, app->event_queue);

    app->gui = furi_record_open(RECORD_GUI);
    gui_add_view_port(app->gui, app->view_port, GuiLayerFullscreen);

    goto_page(app, app->page_index);

    InputEvent event;
    while(app->running) {
        if(furi_message_queue_get(app->event_queue, &event, FuriWaitForever) != FuriStatusOk) {
            continue;
        }
        if(event.type != InputTypeShort) {
            continue;
        }

        if(event.key == InputKeyBack) {
            app->running = false;
        } else if(event.key == InputKeyLeft) {
            goto_page(app, (app->page_index + PAGE_COUNT - 1) % PAGE_COUNT);
        } else if(event.key == InputKeyRight) {
            goto_page(app, (app->page_index + 1) % PAGE_COUNT);
        } else {
            const SoundystemPage* page = pages[app->page_index];
            if(page->input) {
                page->input(&event, app);
            }
        }

        view_port_update(app->view_port);
    }

    gui_remove_view_port(app->gui, app->view_port);
    furi_record_close(RECORD_GUI);
    view_port_free(app->view_port);
    furi_message_queue_free(app->event_queue);
    free(app);
    return 0;
}
