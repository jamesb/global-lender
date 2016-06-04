#pragma once
#include <pebble.h>

#include "data/KivaModel.h"

// GetPrefLoansHandler is a pointer to a function that takes no
// parameter and returns nothing.
typedef void (*GetPrefLoansHandler)(void);

// wndMainMenuHandlers is a struct that contains the values of the handlers.
typedef struct wndMainMenuHandlers {
  GetPrefLoansHandler getPrefLoans;     ///< Function that this View calls to request data from its Controller.
} wndMainMenuHandlers;


void wndMainMenu_setHandlers(const wndMainMenuHandlers);

void wndMainMenu_updateClock(struct tm*);
void wndMainMenu_updateData(const KivaModel*);
void wndMainMenu_createPush();
void wndMainMenu_destroy();
