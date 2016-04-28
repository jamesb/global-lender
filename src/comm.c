#include <pebble.h>

#include "misc.h"

#include "comm.h"
#include "data/KivaModel.h"


enum {
  KEY_PEBKIT_READY = 0,
  KEY_PEBBLE_READY,
  
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
  
  KEY_PUT_LOANS_IN_BASKET = 100,
};


static KivaModel* dataModel;
static CommHandlers commHandlers;
static bool pebkitReady;


/**************************************************************************
 * Converts a tuple to a simple data type.
 **************************************************************************/
static bool unloadTupleStr(char buffer[], size_t bufsize, Tuple* tuple, char readable[]) {
  if (tuple) {
    long ret = 0;
    if ((ret = snprintf(buffer, bufsize, "%s", tuple->value->cstring)) < 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "%s string was not written correctly. Ret=%ld", readable, ret);
      return false;
    } else if ((size_t)ret > bufsize) {
      APP_LOG(APP_LOG_LEVEL_WARNING, "%s string was truncated. %ld characters required.", readable, ret);
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "unloadTupleStr %s: %s", readable, buffer);
    return true;
  }
  return false;
}


/**************************************************************************
 * Converts a tuple to a simple data type.
 **************************************************************************/
static bool unloadTupleLong(long int* buffer, Tuple* tuple, char readable[]) {
  if (tuple) {
    *buffer = (long int)tuple->value->int32;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "unloadTupleLong %s: %ld", readable, *buffer);
    return true;
  }
  return false;
}


/**************************************************************************
 * Handles callbacks from the JS component
 **************************************************************************/
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Inbox receive successful.");
  
  Tuple *ready_tuple = dict_find(iterator, KEY_PEBKIT_READY);
  if (ready_tuple) {
    // PebbleKit JS is ready! Safe to send messages
    pebkitReady = true;
  }

  Tuple *tuple = NULL;
  if ( (tuple = dict_find(iterator, KEY_LENDER_ID)) != NULL) {
    KivaModel_LenderId lenderIdBuf; KivaModel_LenderId compareBuf;
    if (unloadTupleStr(lenderIdBuf, sizeof(lenderIdBuf), tuple, "Lender ID")) {
      if (KivaModel_getLenderId(dataModel, compareBuf) != KIVA_MODEL_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "String copy error into compareBuf.");
      } else {
        // If we have a new Lender ID, then we need to reload everything.
        if (strcmp(compareBuf, lenderIdBuf) != 0) {
          KivaModel_destroy(dataModel); 
          dataModel = NULL;
          dataModel = KivaModel_create(lenderIdBuf);
        }
      }
    }
  }
  
  tuple = dict_find(iterator, KEY_LENDER_NAME);
  KivaModel_LenderName lenderNameBuf;
  if (unloadTupleStr(lenderNameBuf, sizeof(lenderNameBuf), tuple, "Lender Name"))
    KivaModel_setLenderName(dataModel, lenderNameBuf);
    
  tuple = dict_find(iterator, KEY_LENDER_LOC);
  KivaModel_LenderLoc lenderLocBuf;
  if (unloadTupleStr(lenderLocBuf, sizeof(lenderLocBuf), tuple, "Lender Location")) 
    KivaModel_setLenderLoc(dataModel, lenderLocBuf);
    
  tuple = dict_find(iterator, KEY_LENDER_LOAN_QTY);
  long int lenderLoanQty = 0;
  if (unloadTupleLong(&lenderLoanQty, tuple, "Lender Loan Quantity"))
    KivaModel_setLenderLoanQty(dataModel, (int)lenderLoanQty);
    
  if (!commHandlers.notifyApp) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");   
  } else {
    (*commHandlers.notifyApp)(dataModel);
  }
  
}


static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox receive failed! Reason: %d", (int)reason);
}


static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed! Reason: %d", (int)reason);
}


static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send successful.");
}


/**************************************************************************
 * Returns whether the communication channel is ready for sending.
 **************************************************************************/
bool comm_pebkitReady() {
  return pebkitReady;
}


/**************************************************************************
 * Request an update of data from PebbleKit.
 **************************************************************************/
void comm_sendUpdateRequest() {
  if (!pebkitReady) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Tried to send a message from the watch before PebbleKit JS is ready.");
    return;
  }
  
  DictionaryIterator *iter;
  AppMessageResult result = app_message_outbox_begin(&iter);
  if(result != APP_MSG_OK) {
    // The outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
  } else {
    dict_write_uint8(iter, KEY_PEBBLE_READY, 0);
    app_message_outbox_send();
  }
}


/**************************************************************************
 * Set our callback handlers.
 **************************************************************************/
void comm_setHandlers(const CommHandlers cmh) {
  commHandlers = cmh;
}


/**************************************************************************
 * Opens communication to PebbleKit and allocates memory.
 **************************************************************************/
void comm_open() {
  dataModel = KivaModel_create(""); 
  
  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  // JRB TODO: Consider optimizing buffer sizes in the future if memory is constrained.
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());
}
  
  
/**************************************************************************
 * Closes communication and frees memory.
 **************************************************************************/
void comm_close() {
  KivaModel_destroy(dataModel);
}
  