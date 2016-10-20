#pragma once

#include "data/KivaModel.h"

typedef struct ClaySettings {
  uint8_t settingsVer;
} ClaySettings;


// Setting string definitions
typedef enum StrSettings {
  LENDER_ID_STR_SETTING  = 0,

  LAST_STR_SETTING
} StrSettings;


typedef struct Message {
  uint32_t  key;
  char*     payload;
} Message;


// UpdateViewClockHandler is a pointer to a function that takes a single
// parameter (struct tm pointer) and returns nothing.
typedef void (*UpdateViewClockHandler)(struct tm*);

// UpdateViewDataHandler is a pointer to a function that takes a single
// parameter (const KivaModel pointer) and returns nothing.
typedef void (*UpdateViewDataHandler)(const KivaModel*);

// CommHandlers is a struct that contains the values of the handlers.
typedef struct CommHandlers {
  UpdateViewClockHandler updateViewClock;     ///< Function that Comm calls to notify the View of a clock update.
  UpdateViewDataHandler  updateViewData;      ///< Function that Comm calls to notify the View of a data model update.
} CommHandlers;


bool comm_pebkitReady();
void comm_open();
void comm_close();

// For buffered sending
void comm_enqMsg(Message*);
void comm_sendBufMsg();
void comm_startResendTimer();

// For unbuffered sending
void comm_sendMsg(const Message*);

void comm_tickHandler(struct tm *tick_time, TimeUnits units_changed);
void comm_setHandlers(const CommHandlers);

void comm_getLenderInfo();
void comm_getPreferredLoans();

void comm_savePersistent();
void comm_loadPersistent();
