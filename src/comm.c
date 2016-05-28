#include <pebble.h>

// Deactivate APP_LOG in this file.
#undef APP_LOG
#define APP_LOG(...)

#include "misc.h"

#include "comm.h"
#include "data/KivaModel.h"
#include "libs/data-processor.h"


static KivaModel* dataModel;
static CommHandlers commHandlers;
static bool pebkitReady;


/////////////////////////////////////////////////////////////////////////////
/// Converts a tuple to a simple data type.
/////////////////////////////////////////////////////////////////////////////
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


/////////////////////////////////////////////////////////////////////////////
/// Converts a tuple to a simple data type.
/////////////////////////////////////////////////////////////////////////////
static bool unloadTupleLong(long int* buffer, Tuple* tuple, const char* readable) {
  if (tuple) {
    *buffer = (long int)tuple->value->int32;
    APP_LOG(APP_LOG_LEVEL_DEBUG, "unloadTupleLong %s: %ld", readable, *buffer);
    return true;
  }
  return false;
}


/////////////////////////////////////////////////////////////////////////////
/// Handles callbacks from the JS component
/////////////////////////////////////////////////////////////////////////////
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Inbox receive successful.");

  MagPebApp_ErrCode mpaRet;
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
    APP_LOG(APP_LOG_LEVEL_DEBUG, "About to deserialize Kiva countries!");
    if (!unloadTupleStr(&countrySetBuf, bufsize, tuple, readable)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error in unloadTupleStr.");
    } else {
      ProcessingState* state = data_processor_create(countrySetBuf, '|');
      uint8_t num_strings = data_processor_count(state);
      char** strings = malloc(sizeof(char*) * num_strings);
      for (uint8_t n = 0; n < num_strings; n += 2) {
        strings[n] = data_processor_get_string(state);
        strings[n+1] = data_processor_get_string(state);
        if ( (mpaRet = KivaModel_addKivaCountry(dataModel, strings[n], strings[n+1])) != MPA_SUCCESS) {
            APP_LOG(APP_LOG_LEVEL_ERROR, "Error adding Kiva country to data model: %s", MagPebApp_getErrMsg(mpaRet));
        }
        free(strings[n]);
        free(strings[n+1]);
      }
      free(strings);
      free(state);
    }
    free(countrySetBuf); countrySetBuf = NULL;
    int kivaCountryQty = 0;
    if ( (mpaRet = KivaModel_getKivaCountryQty(dataModel, &kivaCountryQty)) != MPA_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error getting Kiva country quantity from data model: %s", MagPebApp_getErrMsg(mpaRet));
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
      if ( (mpaRet = KivaModel_setLenderId(dataModel, lenderIdBuf)) != MPA_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error setting %s in data model: %s", readable, MagPebApp_getErrMsg(mpaRet));
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
      if ( (mpaRet = KivaModel_setLenderName(dataModel, lenderNameBuf)) != MPA_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error setting %s in data model: %s", readable, MagPebApp_getErrMsg(mpaRet));
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
      if ( (mpaRet = KivaModel_setLenderLoc(dataModel, lenderLocBuf)) != MPA_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error setting %s in data model: %s", readable, MagPebApp_getErrMsg(mpaRet));
      }
    }
    free(lenderLocBuf); lenderLocBuf = NULL;
  }

  if ( (tuple = dict_find(iterator, KEY_LENDER_LOAN_QTY)) != NULL ) {
    const char* readable = "Lender Loan Quantity";
    long int lenderLoanQty = 0;
    if (unloadTupleLong(&lenderLoanQty, tuple, readable)) {
      if ( (mpaRet = KivaModel_setLenderLoanQty(dataModel, (int)lenderLoanQty)) != MPA_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error setting %s in data model: %s", readable, MagPebApp_getErrMsg(mpaRet));
      }
    }
  }

  if ( (tuple = dict_find(iterator, KEY_LENDER_COUNTRY_SET)) != NULL ) {
    const char* readable = "Lender-Supported Countries";
    size_t bufsize = strlen(tuple->value->cstring)+1;
    char* countrySetBuf = NULL;
    countrySetBuf = malloc(bufsize);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "About to deserialize lender countries!");
    if (!unloadTupleStr(&countrySetBuf, bufsize, tuple, readable)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error in unloadTupleStr.");
    } else {
      ProcessingState* state = data_processor_create(countrySetBuf, '|');
      uint8_t num_strings = data_processor_count(state);
      char** strings = malloc(sizeof(char*) * num_strings);
      for (uint8_t n = 0; n < num_strings; n += 2) {
        strings[n] = data_processor_get_string(state);
        strings[n+1] = data_processor_get_string(state);
        if ( (mpaRet = KivaModel_addLenderCountry(dataModel, strings[n], strings[n+1])) != MPA_SUCCESS) {
            APP_LOG(APP_LOG_LEVEL_ERROR, "Error adding Lender country to data model: %s", MagPebApp_getErrMsg(mpaRet));
        }
        free(strings[n]);
        free(strings[n+1]);
      }
      free(strings);
      free(state);
    }
    free(countrySetBuf); countrySetBuf = NULL;
  }

  if ( (tuple = dict_find(iterator, KEY_LOAN_SET)) != NULL ) {
    const char* readable = "Preferred Loans";
    size_t bufsize = strlen(tuple->value->cstring)+1;
    char* loanSetBuf = NULL;
    loanSetBuf = malloc(bufsize);
    APP_LOG(APP_LOG_LEVEL_DEBUG, "About to deserialize loans!");
    if (!unloadTupleStr(&loanSetBuf, bufsize, tuple, readable)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error in unloadTupleStr.");
    } else {
      if ( (mpaRet = KivaModel_clearPreferredLoans(dataModel)) != MPA_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error clearing preferred loan list: %s", MagPebApp_getErrMsg(mpaRet));
      } else {
        ProcessingState* state = data_processor_create(loanSetBuf, '|');
        uint8_t num_fields = data_processor_count(state);
        APP_LOG(APP_LOG_LEVEL_INFO, "Found %d loans of interest.", num_fields/6);
        for (uint8_t n = 0; n < num_fields; n += 6) {
          uint32_t id =        data_processor_get_int(state);
          char* name =         data_processor_get_string(state);
          char* use =          data_processor_get_string(state);
          char* countryCode =  data_processor_get_string(state);
          uint16_t fundedAmt = data_processor_get_int(state);
          uint16_t loanAmt =   data_processor_get_int(state);
          APP_LOG(APP_LOG_LEVEL_INFO, "0:[%ld] 1:[%s] 2:[%s] 3:[%s] 4:[%d] 5:[%d]", id, name, use, countryCode, fundedAmt, loanAmt);
          if ( (mpaRet = KivaModel_addPreferredLoan(dataModel, (LoanInfo) {
                  .id =          id,
                  .name =        name,
                  .use =         use,
                  .countryCode = countryCode,
                  .fundedAmt =   fundedAmt,
                  .loanAmt =     loanAmt
                })) != MPA_SUCCESS) {
              APP_LOG(APP_LOG_LEVEL_ERROR, "Error adding preferred loan to data model: %s", MagPebApp_getErrMsg(mpaRet));
          }
          free(name);
          free(use);
          free(countryCode);
        }
        free(state);
      }
    }
    free(loanSetBuf); loanSetBuf = NULL;
  }

  if (!commHandlers.notifyView) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");
  } else {
    (*commHandlers.notifyView)(dataModel);
  }

}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Inbox receive failed! Reason: %d", (int)reason);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed! Reason: %d", (int)reason);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send successful.");
}


/////////////////////////////////////////////////////////////////////////////
/// Returns whether the communication channel is ready for sending.
/////////////////////////////////////////////////////////////////////////////
bool comm_pebkitReady() {
  return pebkitReady;
}


/////////////////////////////////////////////////////////////////////////////
/// Send data to PebbleKit.
/////////////////////////////////////////////////////////////////////////////
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

    MagPebApp_ErrCode mpaRet;
    char* countryCodes = NULL;
    if ( (mpaRet = KivaModel_getLenderCountryCodes(dataModel, true, &countryCodes)) != MPA_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Could not retrieve lender country codes: %s", MagPebApp_getErrMsg(mpaRet));
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


/////////////////////////////////////////////////////////////////////////////
/// Opens communication to PebbleKit and allocates memory.
/////////////////////////////////////////////////////////////////////////////
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


/////////////////////////////////////////////////////////////////////////////
/// Closes communication and frees memory.
/////////////////////////////////////////////////////////////////////////////
void comm_close() {
  if (dataModel != NULL) {
    KivaModel_destroy(dataModel);  dataModel = NULL;
  }
}


