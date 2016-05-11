#include <pebble.h>

// Deactivate APP_LOG in this file.

#include "data/KivaModel.h"
#include "misc.h"
#include "ui/wndLenderBasics.h"
#include "ui/lyrDigitime.h"

static Window* wndLenderBasics;

static TextLayer* lyrLenderName;
static TextLayer* lyrLenderLoc;
static TextLayer* lyrLenderLoanSummary;


/**************************************************************************
 * Updates the displayed digital time
 **************************************************************************/
void wndLenderBasics_updateTime() {
  lyrDigitime_updateTime();
}


/**************************************************************************
 * Updates the displayed Kiva information.
 **************************************************************************/
void wndLenderBasics_updateView(const KivaModel* km) {
  if (!wndLenderBasics) return;
  if (!window_is_loaded(wndLenderBasics)) return;

  KivaModel_ErrCode kmret;
  static char* lenName = NULL;
  lenName = NULL;
  if ( (kmret = KivaModel_getLenderName(km, &lenName)) != KIVA_MODEL_SUCCESS ) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "KivaModel error: %s", KivaModel_getErrMsg(kmret));
  } else {
    text_layer_set_text(lyrLenderName, lenName);
  }

  static char* lenLoc = NULL;
  lenLoc = NULL;
  if ( (kmret = KivaModel_getLenderLoc(km, &lenLoc)) != KIVA_MODEL_SUCCESS ) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Kiva Model error: %s", KivaModel_getErrMsg(kmret));
  } else {
    text_layer_set_text(lyrLenderLoc, lenLoc);
  }

  int loanQty = 0;
  if ( (kmret = KivaModel_getLenderLoanQty(km, &loanQty)) != KIVA_MODEL_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Kiva Model error: %s", KivaModel_getErrMsg(kmret));
    return;
  }

  long lret = 0;  static char buffer[100];
  if ( (lret = snprintf(buffer, sizeof(buffer), "%d Loans", loanQty)) < 0) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Lender Summary string was not written correctly. Ret=%ld", lret);
  } else if ((size_t)lret > sizeof(buffer)) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Lender Summary string was truncated. %ld characters required.", lret);
  } else {
    text_layer_set_text(lyrLenderLoanSummary, buffer);
  }
}


/**************************************************************************
 *
 **************************************************************************/
Window* wndLenderBasics_getWindow() {
  return wndLenderBasics;
}


/**************************************************************************
 *
 **************************************************************************/
void wndLenderBasics_push() {
  if (!wndLenderBasics) APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");
  else window_stack_push(wndLenderBasics, true);
}


/**************************************************************************
 *
 **************************************************************************/
void wndLenderBasics_createPush() {
  wndLenderBasics_create();
  wndLenderBasics_push();
}


/**************************************************************************
 *
 **************************************************************************/
static void wndLenderBasics_load(Window* window) {
  window_set_background_color(window, COLOR_FALLBACK(GColorPictonBlue, GColorBlack));
  Layer* lyrRoot = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(lyrRoot);
  GColor fontColor = GColorBlack;

  // Digital Time
  lyrDigitime_create(GRect(0, (int)(bounds.size.h * 0.05), bounds.size.w, 20), lyrRoot);
  lyrDigitime_stylize(GColorClear, fontColor, GTextAlignmentCenter, fonts_get_system_font(FONT_KEY_GOTHIC_14));

  // Lender Name
  lyrLenderName = text_layer_create( GRect(0, (int)(bounds.size.h * 0.3), bounds.size.w, 30) );
  textLayer_stylize(lyrLenderName, GColorClear, fontColor, GTextAlignmentCenter, fonts_get_system_font(FONT_KEY_ROBOTO_CONDENSED_21));
  text_layer_set_text(lyrLenderName, "Loading...");
  layer_add_child(lyrRoot, text_layer_get_layer(lyrLenderName));

  // Lender Location
  lyrLenderLoc = text_layer_create( GRect(0, (int)(bounds.size.h * 0.42), bounds.size.w, 30) );
  textLayer_stylize(lyrLenderLoc, GColorClear, fontColor, GTextAlignmentCenter, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text(lyrLenderLoc, "");
  layer_add_child(lyrRoot, text_layer_get_layer(lyrLenderLoc));

  // Lender Loan Summary
  lyrLenderLoanSummary = text_layer_create( GRect(0, (int)(bounds.size.h * 0.55), bounds.size.w, 40) );
  textLayer_stylize(lyrLenderLoanSummary, GColorClear, fontColor, GTextAlignmentCenter, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text(lyrLenderLoanSummary, "");
  layer_add_child(lyrRoot, text_layer_get_layer(lyrLenderLoanSummary));
}


/**************************************************************************
 *
 **************************************************************************/
static void wndLenderBasics_unload(Window* window) {
  // Destroy text layers
  text_layer_destroy(lyrLenderLoanSummary);  lyrLenderLoanSummary = NULL;
  text_layer_destroy(lyrLenderLoc);          lyrLenderLoc = NULL;
  text_layer_destroy(lyrLenderName);         lyrLenderName = NULL;

  lyrDigitime_destroy();
}


/**************************************************************************
 *
 **************************************************************************/
void wndLenderBasics_create() {
  if (wndLenderBasics) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to re-create window before destroying.");
  } else {
    wndLenderBasics = window_create();

    // Set handlers to manage the elements inside the Window
    window_set_window_handlers(wndLenderBasics, (WindowHandlers) {
      .load =   wndLenderBasics_load,
      .unload = wndLenderBasics_unload
    });
  }
}


/**************************************************************************
 *
 **************************************************************************/
void wndLenderBasics_destroy() {
  if (!wndLenderBasics) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to destroy a null pointer!");
  } else {
    window_destroy(wndLenderBasics);
    wndLenderBasics = NULL;
  }
}

