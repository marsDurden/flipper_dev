#pragma once

#include <gui/canvas.h>
#include <input/input.h>

typedef struct SoundystemApp SoundystemApp;

/*
 * One carousel page. To add a new page: write a page_<name>.c/.h implementing
 * this interface, add one line to the `pages` array in soundystem_fever.c -
 * nothing else needs to change. The title bar, Left/Right carousel
 * navigation, and the page-dot indicator are all handled generically by
 * soundystem_fever.c; a page only draws its own body content and (optionally)
 * reacts to other input.
 */
typedef struct {
    const char* name;

    /* Draw this page's content into the body area (canvas y >= SOUNDYSTEM_BODY_TOP).
       The title bar above it is drawn by the caller - don't redraw it. */
    void (*draw)(Canvas* canvas, SoundystemApp* app);

    /* Handle an input event that isn't carousel navigation - Left, Right and
       Back are consumed by soundystem_fever.c before a page ever sees them.
       May be NULL if the page has nothing to react to. */
    void (*input)(InputEvent* event, SoundystemApp* app);

    /* Called once whenever the carousel lands on this page (including at
       startup, for the initial page). May be NULL. */
    void (*on_enter)(SoundystemApp* app);
} SoundystemPage;

/* Canvas y where a page's own content should start drawing, leaving room for
   the shared title bar above it. */
#define SOUNDYSTEM_BODY_TOP 13
