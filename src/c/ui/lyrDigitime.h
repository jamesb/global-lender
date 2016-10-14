#pragma once

#include "../libs/magpebapp.h"

void lyrDigitime_create(const GRect position, Layer* lyrParent);
void lyrDigitime_destroy();
TextLayer* lyrDigitime_getLayer();

MagPebApp_ErrCode lyrDigitime_stylize(const GColor, const GColor, const GTextAlignment, const GFont);
MagPebApp_ErrCode lyrDigitime_updateTime(struct tm*);
