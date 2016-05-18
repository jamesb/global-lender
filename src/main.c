#include <pebble.h>

// Deactivate APP_LOG in this file.
#undef APP_LOG
#define APP_LOG(...)

#include "comm.h"
#include "misc.h"
#include "ui/wndMainMenu.h"


/**************************************************************************
 * Callback for TickTimerService
 **************************************************************************/
static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  { // limiting timebuf in a local scope
    char timebuf[40];
    if (strftime(timebuf, sizeof(timebuf), "%a, %d %b %Y %T %z", tick_time) == 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "strftime() returned 0");
    } else {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "tick_handler (%s) -- units_changed = %d", timebuf, units_changed);
    }
  }

//  wndLenderBasics_updateTime();

  // Get update every 30 minutes
  if(tick_time->tm_min % 30 == 0) {
    comm_sendMsgCstr(KEY_GET_LENDER_INFO, NULL);
  }
}


/**************************************************************************
 * Used for the creation of all Pebble SDK elements.
 **************************************************************************/
static void init() {
  wndMainMenu_createPush();
  wndMainMenu_setHandlers( (wndMainMenuHandlers) {
    .getPrefLoans = comm_getPreferredLoans
  });

  comm_setHandlers( (CommHandlers) {
    .notifyView = wndMainMenu_updateView
  });
  comm_open();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}


/**************************************************************************
 * Used for the destruction of all Pebble SDK elements.
 **************************************************************************/
static void deinit() {
  wndMainMenu_destroy();

  comm_close();
}


/**************************************************************************
 * Standard Pebble main function.
 **************************************************************************/
int main(void) {
  HEAP_LOG("at start");
  init();
  HEAP_LOG("after init");

  /* wait for system events until exit */
  app_event_loop();

  deinit();
}
