#include <pebble.h>

#include "misc.h"

#include "comm.h"
#include "data/KivaModel.h"
#include "libs/data-processor.h"


static KivaModel* dataModel;
static CommHandlers commHandlers;
static bool pebkitReady;


/**************************************************************************
 * Converts a tuple to a simple data type.
 **************************************************************************/
static bool unloadTupleStr(char** buffer, size_t bufsize, Tuple* tuple, const char* readable) {
  if (tuple != NULL) {
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
static bool unloadTupleLong(long int* buffer, Tuple* tuple, const char* readable) {
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

  KivaModel_ErrCode kmret;
  Tuple *tuple = NULL;
  if ( (tuple = dict_find(iterator, KEY_PEBKIT_READY)) != NULL ) {
    // PebbleKit JS is ready! Safe to send messages
    pebkitReady = true;
  }

  if ( (tuple = dict_find(iterator, KEY_KIVA_COUNTRY_SET)) != NULL ) {
    const char* readable = "Kiva-Served Countries";
    size_t bufsize = strlen(tuple->value->cstring)+1;
    char* countrySetBuf = NULL;
    countrySetBuf = malloc(bufsize);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "About to deserialize countries!");
    if (!unloadTupleStr(&countrySetBuf, bufsize, tuple, readable)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error in unloadTupleStr.");
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "%s are %s", readable, countrySetBuf);
      ProcessingState* state = data_processor_create(countrySetBuf, '|');
      uint8_t num_strings = data_processor_count(state);
      char** strings = malloc(sizeof(char*) * num_strings);
      for (uint8_t n = 0; n < num_strings; n += 2) {
        strings[n] = data_processor_get_string(state);
        strings[n+1] = data_processor_get_string(state);
        if ( (kmret = KivaModel_addKivaCountry(dataModel, strings[n], strings[n+1])) != KIVA_MODEL_SUCCESS) {
            APP_LOG(APP_LOG_LEVEL_ERROR, "Error adding Kiva country to data model: %s", KivaModel_getErrMsg(kmret));
        }
        free(strings[n]);
        free(strings[n+1]);
      }
      free(strings);
      free(state);
    }
    free(countrySetBuf); countrySetBuf = NULL;
    int kivaCountryQty = 0;
    if ( (kmret = KivaModel_getKivaCountryQty(dataModel, &kivaCountryQty)) != KIVA_MODEL_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error getting Kiva country quantity from data model: %s", KivaModel_getErrMsg(kmret));
    }
    APP_LOG(APP_LOG_LEVEL_INFO, "Kiva active country total: %d", kivaCountryQty);
    comm_sendMsgCstr(KEY_GET_LENDER_INFO, NULL);
  }

  if ( (tuple = dict_find(iterator, KEY_LENDER_ID)) != NULL ) {
    const char* readable = "Lender Id";
    size_t bufsize = strlen(tuple->value->cstring)+1;
    char* lenderIdBuf = NULL;
    lenderIdBuf = malloc(bufsize);
    if (!unloadTupleStr(&lenderIdBuf, bufsize, tuple, readable)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error in unloadTupleStr.");
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "lenderIdBuf = %s", lenderIdBuf);
      if ( (kmret = KivaModel_setLenderId(dataModel, lenderIdBuf)) != KIVA_MODEL_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error setting %s in data model: %s", readable, KivaModel_getErrMsg(kmret));
      }
    }
    free(lenderIdBuf); lenderIdBuf = NULL;
  }

  if ( (tuple = dict_find(iterator, KEY_LENDER_NAME)) != NULL ) {
    const char* readable = "Lender Name";
    size_t bufsize = strlen(tuple->value->cstring)+1;
    char* lenderNameBuf = NULL;
    lenderNameBuf = malloc(bufsize);
    if (!unloadTupleStr(&lenderNameBuf, bufsize, tuple, readable)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error in unloadTupleStr.");
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "lenderNameBuf = %s", lenderNameBuf);
      if ( (kmret = KivaModel_setLenderName(dataModel, lenderNameBuf)) != KIVA_MODEL_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error setting %s in data model: %s", readable, KivaModel_getErrMsg(kmret));
      }
    }
    free(lenderNameBuf); lenderNameBuf = NULL;
  }

  if ( (tuple = dict_find(iterator, KEY_LENDER_LOC)) != NULL ) {
    const char* readable = "Lender Location";
    size_t bufsize = strlen(tuple->value->cstring)+1;
    char* lenderLocBuf = NULL;
    lenderLocBuf = malloc(bufsize);
    if (!unloadTupleStr(&lenderLocBuf, bufsize, tuple, readable)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error in unloadTupleStr.");
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "lenderLocBuf = %s", lenderLocBuf);
      if ( (kmret = KivaModel_setLenderLoc(dataModel, lenderLocBuf)) != KIVA_MODEL_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error setting %s in data model: %s", readable, KivaModel_getErrMsg(kmret));
      }
    }
    free(lenderLocBuf); lenderLocBuf = NULL;
  }

  if ( (tuple = dict_find(iterator, KEY_LENDER_LOAN_QTY)) != NULL ) {
    const char* readable = "Lender Loan Quantity";
    long int lenderLoanQty = 0;
    if (unloadTupleLong(&lenderLoanQty, tuple, readable)) {
      if ( (kmret = KivaModel_setLenderLoanQty(dataModel, (int)lenderLoanQty)) != KIVA_MODEL_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error setting %s in data model: %s", readable, KivaModel_getErrMsg(kmret));
      }
    }
  }

  if ( (tuple = dict_find(iterator, KEY_LENDER_COUNTRY_SET)) != NULL ) {
    const char* readable = "Lender-Supported Countries";
    size_t bufsize = strlen(tuple->value->cstring)+1;
    char* countrySetBuf = NULL;
    countrySetBuf = malloc(bufsize);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "About to deserialize countries!");
    if (!unloadTupleStr(&countrySetBuf, bufsize, tuple, readable)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error in unloadTupleStr.");
    } else {
      APP_LOG(APP_LOG_LEVEL_INFO, "%s are %s", readable, countrySetBuf);
      ProcessingState* state = data_processor_create(countrySetBuf, '|');
      uint8_t num_strings = data_processor_count(state);
      char** strings = malloc(sizeof(char*) * num_strings);
      for (uint8_t n = 0; n < num_strings; n += 2) {
        strings[n] = data_processor_get_string(state);
        strings[n+1] = data_processor_get_string(state);
        if ( (kmret = KivaModel_addLenderCountry(dataModel, strings[n], strings[n+1])) != KIVA_MODEL_SUCCESS) {
            APP_LOG(APP_LOG_LEVEL_ERROR, "Error adding Lender country to data model: %s", KivaModel_getErrMsg(kmret));
        }
        free(strings[n]);
        free(strings[n+1]);
      }
      free(strings);
      free(state);
    }
    free(countrySetBuf); countrySetBuf = NULL;
  }

  if (!commHandlers.notifyView) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");
  } else {
    (*commHandlers.notifyView)(dataModel);
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
 * Request data from PebbleKit.
 **************************************************************************/
void comm_sendMsgCstr(const MsgKey msgKey, const char* payload) {
  if (!pebkitReady) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Tried to send a message from the watch before PebbleKit JS is ready.");
    return;
  }

  // Declare the dictionary's iterator
  DictionaryIterator *outIter;

  // Prepare the outbox buffer for this message
  AppMessageResult result = app_message_outbox_begin(&outIter);
  if (result == APP_MSG_OK) {
    dict_write_cstring(outIter, msgKey, payload);

    // Send this message
    result = app_message_outbox_send();
    if(result != APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox: %d", (int)result);
    }
  } else {
    // The outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox: %d", (int)result);
  }
}


/////////////////////////////////////////////////////////////////////////////
/// Requests PebbleKit to send a list of preferred loans for the lender.
/////////////////////////////////////////////////////////////////////////////
void comm_getPreferredLoans() {
    if (dataModel == NULL) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Kiva Model is not yet initialized.");
      return;
    }

    KivaModel_ErrCode kmret;
    char* countryCodes = NULL;
    if ( (kmret = KivaModel_getLenderCountryCodes(dataModel, false, &countryCodes)) != KIVA_MODEL_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Could not retrieve lender country codes: %s", KivaModel_getErrMsg(kmret));
      return;
    }

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Get loans for country codes: %s", countryCodes);
    comm_sendMsgCstr(KEY_GET_PREFERRED_LOANS, countryCodes);
    free(countryCodes);
}


/////////////////////////////////////////////////////////////////////////////
/// Set our callback handlers.
/////////////////////////////////////////////////////////////////////////////
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

