#pragma once

void lyrDigitime_create(const GRect position, Layer* lyrParent); 
  
void lyrDigitime_destroy();

void lyrDigitime_stylize(const GColor, const GColor, const GTextAlignment, const GFont); 

void lyrDigitime_updateTime();
