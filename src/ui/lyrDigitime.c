#include <pebble.h>

// Deactivate APP_LOG in this file.
#undef APP_LOG
#define APP_LOG(...)

#include "misc.h"
#include "ui/lyrDigitime.h"

static TextLayer *lyrDigitime;


/////////////////////////////////////////////////////////////////////////////
/// Creates the text layer and adds it to the parent layer
/////////////////////////////////////////////////////////////////////////////
void lyrDigitime_create(const GRect position, Layer* lyrParent) {
  if (lyrDigitime) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to re-create layer before destroying.");
    return;
  }
  lyrDigitime = text_layer_create(position);
  text_layer_set_text(lyrDigitime, "00:00");
  lyrDigitime_updateTime(NULL);
  layer_add_child(lyrParent, text_layer_get_layer(lyrDigitime));
}


/////////////////////////////////////////////////////////////////////////////
/// Destroys the text layer
/////////////////////////////////////////////////////////////////////////////
void lyrDigitime_destroy() {
  if (!lyrDigitime) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to destroy a null pointer!");
  } else {
    text_layer_destroy(lyrDigitime);
    lyrDigitime = NULL;
  }
}


/////////////////////////////////////////////////////////////////////////////
/// Returns the digital time layer
/////////////////////////////////////////////////////////////////////////////
TextLayer* lyrDigitime_getLayer() {
  return lyrDigitime;
}


/////////////////////////////////////////////////////////////////////////////
/// Stylizes the text layer to the spec
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode lyrDigitime_stylize(const GColor bgcolor, const GColor txtColor,
                                const GTextAlignment txtAlign, const GFont txtFont) {
  MPA_RETURN_IF_NULL(lyrDigitime);
  textLayer_stylize(lyrDigitime, bgcolor, txtColor, txtAlign, txtFont);

  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Updates the displayed time
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode lyrDigitime_updateTime(struct tm* in_time) {
  MPA_RETURN_IF_NULL(lyrDigitime);
  struct tm* curr_time = NULL;

  if (in_time != NULL) {
    curr_time = in_time;
  } else {
    // Get a tm structure
    time_t temp = time(NULL);
    curr_time = localtime(&temp);
  }

  // Write the current hours and minutes into a buffer
  static char buffer[10];
  strftime(buffer, sizeof(buffer), clock_is_24h_style() ?
      "%H:%M" : "%l:%M %p", curr_time);

  // Display this time on the TextLayer
  text_layer_set_text(lyrDigitime, buffer);

  return MPA_SUCCESS;
}

