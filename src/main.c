#include <pebble.h>

// Deactivate APP_LOG in this file.
#undef APP_LOG
#define APP_LOG(...)

#include "comm.h"
#include "misc.h"
#include "ui/wndMainMenu.h"


/////////////////////////////////////////////////////////////////////////////
/// Used for the creation of all Pebble SDK elements.
/////////////////////////////////////////////////////////////////////////////
static void init() {
  wndMainMenu_createPush();
  wndMainMenu_setHandlers( (wndMainMenuHandlers) {
    .getPrefLoans = comm_getPreferredLoans
  });

  comm_setHandlers( (CommHandlers) {
    .updateViewClock = wndMainMenu_updateClock,
    .updateViewData = wndMainMenu_updateData
  });
  comm_open();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, comm_tickHandler);
}


/////////////////////////////////////////////////////////////////////////////
/// Used for the destruction of all Pebble SDK elements.
/////////////////////////////////////////////////////////////////////////////
static void deinit() {
  wndMainMenu_destroy();

  comm_close();
}


/////////////////////////////////////////////////////////////////////////////
/// Standard Pebble main function
/////////////////////////////////////////////////////////////////////////////
int main(void) {
  HEAP_LOG("at start");
  init();
  HEAP_LOG("after init");

  /* wait for system events until exit */
  app_event_loop();

  deinit();
}
