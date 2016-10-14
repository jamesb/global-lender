#pragma once

#include "data/KivaModel.h"

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

void comm_enqMsg(const Message*);
void comm_sendMsg(const Message*);
void comm_sendBufMsg();
void comm_tickHandler(struct tm *tick_time, TimeUnits units_changed);
void comm_setHandlers(const CommHandlers);

void comm_getLenderInfo();
void comm_getPreferredLoans();


