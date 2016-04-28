#pragma once
#include <pebble.h>

#include "data/KivaModel.h"

void wndMainMenu_updateView(const KivaModel*);
void wndMainMenu_createPush(); 
void wndMainMenu_destroy();
