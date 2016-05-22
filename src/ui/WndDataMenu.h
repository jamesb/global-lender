#pragma once
#include <pebble.h>
#include "libs/magpebapp.h"


typedef struct WndDataMenu WndDataMenu;

MagPebApp_ErrCode WndDataMenu_updateView(WndDataMenu*);

MagPebApp_ErrCode WndDataMenu_push(WndDataMenu*);
WndDataMenu* WndDataMenu_create();
MagPebApp_ErrCode WndDataMenu_destroy(WndDataMenu*);
