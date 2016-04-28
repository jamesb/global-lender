#pragma once

#include "data/KivaModel.h"

// NotifyAppHandler is a pointer to a function that takes a single
// parameter (const KivaModel pointer) and returns nothing.
typedef void (*NotifyAppHandler)(const KivaModel*);

// CommHandlers is a struct that contains the values of the handlers.
typedef struct CommHandlers {
  NotifyAppHandler notifyApp;
} CommHandlers;


void comm_open();
void comm_close();

void comm_requestUpdate();
void comm_setHandlers(const CommHandlers);