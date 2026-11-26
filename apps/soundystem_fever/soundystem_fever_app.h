#pragma once

#include <furi.h>
#include <gui/gui.h>
#include <input/input.h>

/*
 * Shared app context, passed to every page. Keep this to state that's cheap
 * and small: with only a handful of pages, a page's own bit of state (like
 * docs_subpage below) lives here rather than behind a generic per-page
 * context pointer - see pages/page.h for the extension point this enables.
 */
typedef struct SoundystemApp {
    FuriMessageQueue* event_queue;
    ViewPort* view_port;
    Gui* gui;
    bool running;

    size_t page_index;

    /* Owned by pages/page_docs.c - which sub-screen of the Audio Theory page
       is showing. */
    uint8_t docs_subpage;
} SoundystemApp;
