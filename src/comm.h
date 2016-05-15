#pragma once

#include "data/KivaModel.h"

typedef enum MsgKey {
  KEY_PEBKIT_READY = 0,
  KEY_PEBBLE_READY,
  KEY_GET_LENDER_INFO,
  KEY_GET_PREFERRED_LOANS,

  KEY_KIVA_COUNTRY_SET = 10,
  KEY_KIVA_SECTOR_SET,
  KEY_KIVA_ACTIVITY_SET,
  KEY_KIVA_FIELD_PARTNER_SET,

  APP_ACHIEVEMENT_SET = 20,

  KEY_LENDER_ID = 30,
  KEY_LENDER_NAME,
  KEY_LENDER_LOC,
  KEY_LENDER_LOAN_QTY,
  KEY_LENDER_TEAM_SET,
  KEY_LENDER_COUNTRY_SET,
  KEY_LENDER_SECTOR_SET,
  KEY_LENDER_ACTIVITY_SET,
  KEY_LENDER_FIELD_PARTNER_SET,
  KEY_LENDER_TARGET_LOAN_SET,
  KEY_LENDER_ACHIEVEMENT_SET,
  KEY_LOAN_SET,

  KEY_PUT_LOANS_IN_BASKET = 100,
} MsgKey;



// NotifyViewHandler is a pointer to a function that takes a single
// parameter (const KivaModel pointer) and returns nothing.
typedef void (*NotifyViewHandler)(const KivaModel*);

// CommHandlers is a struct that contains the values of the handlers.
typedef struct CommHandlers {
  NotifyViewHandler notifyView;     ///< Function that Comm calls to notify the View of a data model update.
} CommHandlers;


bool comm_pebkitReady();
void comm_open();
void comm_close();

void comm_sendMessage(const MsgKey);
void comm_setHandlers(const CommHandlers);

void comm_getPreferredLoans();


