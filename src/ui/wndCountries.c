#include <pebble.h>
#include "misc.h"
#include "ui/wndCountries.h"

#include <pebble.h>

static Window* wndCountries;
static TextLayer *lyrCountries;
static TextLayer *lyrCountrySummary;


/**************************************************************************
 * Updates the displayed Kiva information.
 **************************************************************************/
void wndCountries_updateView(const KivaModel* km) {
  if (!wndCountries) return;
  if (!window_is_loaded(wndCountries)) return;
  
  HEAP_LOG("started");
}


/**************************************************************************
 *
 **************************************************************************/
void wndCountries_push() {
  if (!wndCountries) APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");
  else window_stack_push(wndCountries, true);
}


/**************************************************************************
 *
 **************************************************************************/
void wndCountries_createPush() {
  wndCountries_create();
  wndCountries_push();
}


/**************************************************************************
 * 
 **************************************************************************/
static void wndCountries_load(Window* window) {
  window_set_background_color(window, COLOR_FALLBACK(GColorPictonBlue, GColorBlack));
  Layer* lyrRoot = window_get_root_layer(window); 
  GRect bounds = layer_get_bounds(lyrRoot);
  GColor fontColor = GColorBlack;

  // lyrCountries
  lyrCountries = text_layer_create( GRect(0, (int)(bounds.size.h * 0.3), bounds.size.w, 30) );
  textLayer_stylize(lyrCountries, GColorClear, fontColor, GTextAlignmentCenter, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text(lyrCountries, "Loading...");
  layer_add_child(lyrRoot, text_layer_get_layer(lyrCountries));

  // lyrCountrySummary
  lyrCountrySummary = text_layer_create( GRect(0, (int)(bounds.size.h * 0.55), bounds.size.w, 40) );
  textLayer_stylize(lyrCountrySummary, GColorClear, fontColor, GTextAlignmentCenter, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text(lyrCountrySummary, "");
  layer_add_child(lyrRoot, text_layer_get_layer(lyrCountrySummary));
}


/**************************************************************************
 * 
 **************************************************************************/
static void wndCountries_unload(Window* window) {
  // Destroy text layers
  text_layer_destroy(lyrCountries);          lyrCountries = NULL;
  text_layer_destroy(lyrCountrySummary);     lyrCountrySummary = NULL;
}


/**************************************************************************
 * 
 **************************************************************************/
void wndCountries_create() {
  if (wndCountries) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to re-create window before destroying.");
  } else {
    wndCountries = window_create();

    // Set handlers to manage the elements inside the Window
    window_set_window_handlers(wndCountries, (WindowHandlers) {
      .load =   wndCountries_load,
      .unload = wndCountries_unload
    });
  }
}


/**************************************************************************
 * 
 **************************************************************************/
void wndCountries_destroy() {
  if (!wndCountries) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to destroy a null pointer!");
  } else {
    window_destroy(wndCountries);
    wndCountries = NULL;
  }
}

