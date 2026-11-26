#pragma once

#include <gui/modules/submenu.h>

typedef struct GetupApp GetupApp;

Submenu* view_list_alloc(GetupApp* app);
void view_list_free(Submenu* submenu);

/* Rebuilds the row labels from app->alarms - call before switching to this
   view so it reflects whatever was just edited. */
void view_list_rebuild(Submenu* submenu, GetupApp* app);
