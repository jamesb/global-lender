#include <pebble.h>

#include "misc.h"

#include "comm.h"
#include "data/KivaModel.h"
#include "data-processor.h"


// Presently, Kiva recognizes 91 countries. Country codes have a length
// of 2. Country codes are separated by a pipe character (length = 1).
// So, if n = 91:
// (n * 2) + (n-1) + 1 = 273
// Giving ourselves a little room to grow...
#define COMM_COUNTRY_SET_SERIALIZED_LEN 300


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
// JRB TODO: This is crap.
static bool unloadTupleStr(char** buffer, size_t bufsize, Tuple* tuple, char* readable) {
  if (tuple) {
    long ret = 0;
    if ((ret = snprintf(*buffer, bufsize, "%s", tuple->value->cstring)) < 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "%s string was not written correctly. Ret=%ld", readable, ret);
      return false;
    } else if ((size_t)ret > bufsize) {
      APP_LOG(APP_LOG_LEVEL_WARNING, "%s string was truncated. %ld characters required.", readable, ret);
    }
    APP_LOG(APP_LOG_LEVEL_DEBUG, "unloadTupleStr %s: %s", readable, *buffer);
    return true;
  }
  return false;
}


/**************************************************************************
 * Converts a tuple to a simple data type.
 **************************************************************************/
static bool unloadTupleLong(long int* buffer, Tuple* tuple, char* readable) {
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
  
  Tuple *tuple = NULL;
  if ( (tuple = dict_find(iterator, KEY_PEBKIT_READY)) != NULL ) {
    // PebbleKit JS is ready! Safe to send messages
    pebkitReady = true;
  }
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Made it here.");
  
  if ( (tuple = dict_find(iterator, KEY_LENDER_ID)) != NULL ) {
    char* lenderIdBuf = NULL;
    size_t bufsize = strlen(tuple->value->cstring)+1;
    lenderIdBuf = malloc(bufsize);
    if (unloadTupleStr(&lenderIdBuf, bufsize, tuple, "Lender ID")) {
      // lenderIdBuf contains the tuple string
      APP_LOG(APP_LOG_LEVEL_INFO, "lenderIdBuf = %s", lenderIdBuf);
      char* compareBuf = NULL;
      if (KivaModel_getLenderId(dataModel, &compareBuf) != KIVA_MODEL_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "String copy error into compareBuf.");
      } else {
        // compareBuf contains our currently-known lender ID
        APP_LOG(APP_LOG_LEVEL_INFO, "compareBuf = %s", compareBuf);
        if (strcmp(compareBuf, lenderIdBuf) != 0) {
          APP_LOG(APP_LOG_LEVEL_INFO, "Different Lender IDs: %s != %s", lenderIdBuf, compareBuf);
          // If we have a new Lender ID, then we need to reload everything.
          if (dataModel != NULL) {
            APP_LOG(APP_LOG_LEVEL_INFO, "Destroying data model...");
            KivaModel_destroy(dataModel);  dataModel = NULL;
          }
          APP_LOG(APP_LOG_LEVEL_INFO, "Creating new data model with lenderIdBuf = %s.", lenderIdBuf);
          dataModel = KivaModel_create(lenderIdBuf);
          if (dataModel == NULL) {
            APP_LOG(APP_LOG_LEVEL_ERROR, "Could not create data model.");
            window_stack_pop_all(false);
          }
          APP_LOG(APP_LOG_LEVEL_INFO, "now lenderIdBuf = %s", lenderIdBuf);
        }
      }
    }
    free(lenderIdBuf); lenderIdBuf = NULL;
  }
  
  char* lenderNameBuf = NULL;
  if ( (tuple = dict_find(iterator, KEY_LENDER_NAME)) != NULL ) {
    size_t bufsize = strlen(tuple->value->cstring)+1;
    lenderNameBuf = malloc(bufsize);
    if (unloadTupleStr(&lenderNameBuf, bufsize, tuple, "Lender Name")) {
      APP_LOG(APP_LOG_LEVEL_INFO, "lenderNameBuf = %s", lenderNameBuf);
      KivaModel_setLenderName(dataModel, lenderNameBuf);
    }
    free(lenderNameBuf); lenderNameBuf = NULL;
  }
    
  char* lenderLocBuf = NULL;
  if ( (tuple = dict_find(iterator, KEY_LENDER_LOC)) != NULL ) {
    size_t bufsize = strlen(tuple->value->cstring)+1;
    lenderLocBuf = malloc(bufsize);
    if (unloadTupleStr(&lenderLocBuf, bufsize, tuple, "Lender Location")) {
      APP_LOG(APP_LOG_LEVEL_INFO, "lenderLocBuf = %s", lenderLocBuf);
      KivaModel_setLenderLoc(dataModel, lenderLocBuf);
    }
    free(lenderLocBuf); lenderLocBuf = NULL;
  }
    
  long int lenderLoanQty = 0;
  if ( (tuple = dict_find(iterator, KEY_LENDER_LOAN_QTY)) != NULL ) {
    if (unloadTupleLong(&lenderLoanQty, tuple, "Lender Loan Quantity")) {
      KivaModel_setLenderLoanQty(dataModel, (int)lenderLoanQty);
    }
  }
    
  char countrySetBuf[COMM_COUNTRY_SET_SERIALIZED_LEN];
  if ( (tuple = dict_find(iterator, KEY_LENDER_COUNTRY_SET)) != NULL ) {
    if (unloadTupleStr((char**)&countrySetBuf, COMM_COUNTRY_SET_SERIALIZED_LEN, tuple, "Lender Supported Countries")) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Lender Supported Countries are %s", countrySetBuf);
      ProcessingState* state = data_processor_create(countrySetBuf, '|');
      uint8_t num_strings = data_processor_count(state);
      char** strings = malloc(sizeof(char*) * num_strings);
      for (uint8_t n = 0; n < num_strings; n += 1) {
        strings[n] = data_processor_get_string(state);
        const char* countryName = NULL;
        if (KivaModel_getKivaCountryName(dataModel, strings[n], &countryName) == KIVA_MODEL_SUCCESS) {
          APP_LOG(APP_LOG_LEVEL_DEBUG, "COMM %s = %s", strings[n], countryName);
          KivaModel_addLenderCountryId(dataModel, strings[n]);
        }
        free(strings[n]);
      }
      free(strings);
      free(state);
    }
  }
    
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
  dataModel = NULL;
  if ( (dataModel = KivaModel_create("")) == NULL) { APP_LOG(APP_LOG_LEVEL_ERROR, "Could not initialize data model."); }
  
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
  if (dataModel != NULL) {
    KivaModel_destroy(dataModel);  dataModel = NULL;
  }
}
  