#include <pebble.h>

// Deactivate APP_LOG in this file.
#undef APP_LOG
#define APP_LOG(...)

#include "misc.h"
#include "ui/lyrDigitime.h"

static TextLayer *lyrDigitime;


/**************************************************************************
 * Creates the text layer and adds it to the parent layer
 **************************************************************************/
void lyrDigitime_create(const GRect position, Layer* lyrParent) {
  if (lyrDigitime) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to re-create layer before destroying.");
  } else {
    lyrDigitime = text_layer_create(position);
    text_layer_set_text(lyrDigitime, "00:00");
    layer_add_child(lyrParent, text_layer_get_layer(lyrDigitime));
  }
}


/**************************************************************************
 * Destroys the text layer
 **************************************************************************/
void lyrDigitime_destroy() {
  if (!lyrDigitime) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to destroy a null pointer!");
  } else {
    text_layer_destroy(lyrDigitime);
    lyrDigitime = NULL;
  }
}


/**************************************************************************
 * Stylizes the text layer to the spec 
 **************************************************************************/
void lyrDigitime_stylize(const GColor bgcolor, const GColor txtColor,
                                const GTextAlignment txtAlign, const GFont txtFont) {
  if (!lyrDigitime) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to stylize before layer is created!");
  } else {
    textLayer_stylize(lyrDigitime, bgcolor, txtColor, txtAlign, txtFont);
  }
}


/**************************************************************************
 * Updates the displayed time
 **************************************************************************/
void lyrDigitime_updateTime() {
  if (!lyrDigitime) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to update the time before layer is created!");
  } else {
    // Get a tm structure
    time_t temp = time(NULL); 
    struct tm *curr_time = localtime(&temp);

    // Write the current hours and minutes into a buffer
    static char buffer[10];
    strftime(buffer, sizeof(buffer), clock_is_24h_style() ?
        "%H:%M" : "%l:%M %p", curr_time);

    // Display this time on the TextLayer
    text_layer_set_text(lyrDigitime, buffer);
  }
}

