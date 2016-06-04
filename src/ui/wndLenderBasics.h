#pragma once

#include "data/KivaModel.h"

void wndLenderBasics_updateTime(struct tm*);
void wndLenderBasics_updateView(const KivaModel*);
void wndLenderBasics_create();
void wndLenderBasics_push();
void wndLenderBasics_createPush();
void wndLenderBasics_destroy();
