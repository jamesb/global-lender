#include <pebble.h>
#include "misc.h"


/**************************************************************************
 * Stylizes the text layer to the spec 
 **************************************************************************/
void textLayer_stylize(TextLayer* textLayer, const GColor bgcolor, const GColor txtColor,
                       const GTextAlignment txtAlign, const GFont txtFont) {
  if (!textLayer) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to stylize before text layer is created!");
  } else {
    text_layer_set_background_color(textLayer, bgcolor);
    text_layer_set_text_color(textLayer, txtColor);
    text_layer_set_text_alignment(textLayer, txtAlign);
    text_layer_set_font(textLayer, txtFont);
  }
}


/**************************************************************************
 * Copies a string with error handling and logging.
 **************************************************************************/
bool strxcpy(char buffer[], size_t bufsize, const char source[], const char readable[]) {
  if (source) {
    long ret = 0;
    if ((ret = snprintf(buffer, bufsize, "%s", source)) < 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "%s string output error. snprintf=%ld", readable, ret);
      return false;
    } else if ((size_t)ret > bufsize) {
      APP_LOG(APP_LOG_LEVEL_WARNING, "%s string was truncated. %ld characters required.", readable, ret);
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "strxcpy %s: %s", readable, buffer);
    return true;
  }
  return false;
}

