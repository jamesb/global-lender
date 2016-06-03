#include <pebble.h>

// Deactivate APP_LOG in this file.
//#undef APP_LOG
//#define APP_LOG(...)

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
  if (*buffer == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "unloadTupleStr: Cannot write to null buffer!");
    return false;
  }
  if (tuple != NULL) {
    long ret = 0;
    if ((ret = snprintf(*buffer, bufsize, "%s", tuple->value->cstring)) < 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "%s string was not written correctly. Ret=%ld", readable, ret);
      return false;
    } else if ((size_t)ret >= bufsize) {
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
/// Deserializes a KEY_KIVA_COUNTRY_SET tuple.
///
/// @param[in]      tuple  This tuple must be non-null and its key must
///       match KEY_KIVA_COUNTRY_SET.
///
/// @return  MPA_SUCCESS on success
///          MPA_NULL_POINTER_ERR if parameters tuple or ... is
///            NULL upon entry.
///          MPA_OUT_OF_MEMORY_ERR if a memory allocation fails
///
/////////////////////////////////////////////////////////////////////////////
static MagPebApp_ErrCode unloadKivaCountrySet(Tuple* tuple) {
  MPA_RETURN_IF_NULL(tuple);

  size_t bufsize = strlen(tuple->value->cstring)+1;
  const char* readable = "Kiva-Served Countries";
  char* countrySetBuf = NULL;
  ProcessingState* state = NULL;
  uint16_t num_strings = 0;
  uint16_t idx = 0;
  char** strings = NULL;
  MagPebApp_ErrCode mpaRet = MPA_SUCCESS;

  if ( (countrySetBuf = malloc(bufsize)) == NULL) { goto freemem; }

  APP_LOG(APP_LOG_LEVEL_DEBUG, "About to deserialize Kiva countries!");

  if (!unloadTupleStr(&countrySetBuf, bufsize, tuple, readable)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error in unloadTupleStr.");
    goto freemem;
  }

  if ( (state = data_processor_create(countrySetBuf, '|')) == NULL) { goto freemem; }
  num_strings = data_processor_count(state);
  if ( (strings = calloc(num_strings, sizeof(*strings))) == NULL) { goto freemem; }

  for (idx = 0; idx < num_strings; idx += 2) {
    strings[idx] = data_processor_get_string(state);
    if (strings[idx] == NULL) { goto freemem; }

    strings[idx+1] = data_processor_get_string(state);
    if (strings[idx+1] == NULL) {goto freemem; }

    if ( (mpaRet = KivaModel_addKivaCountry(dataModel, strings[idx], strings[idx+1])) != MPA_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error adding Kiva country to data model: %s", MagPebApp_getErrMsg(mpaRet));
    }

    if (strings[idx+1] != NULL) {
      free(strings[idx+1]);  strings[idx+1] = NULL;
    }
    if (strings[idx] != NULL) {
      free(strings[idx]);  strings[idx] = NULL;
    }
  }

  if (strings != NULL) {
    free(strings);  strings = NULL;
  }
  if (state != NULL) {
    free(state);  state = NULL;
  }
  if (countrySetBuf != NULL) {
    free(countrySetBuf); countrySetBuf = NULL;
  }

  int kivaCountryQty = 0;
  if ( (mpaRet = KivaModel_getKivaCountryQty(dataModel, &kivaCountryQty)) != MPA_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error getting Kiva country quantity from data model: %s", MagPebApp_getErrMsg(mpaRet));
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "Kiva active country total: %d", kivaCountryQty);

  return MPA_SUCCESS;

freemem:
  for (idx = 0; idx < num_strings; idx ++) {
    if (strings[idx] != NULL) {
      free(strings[idx]);  strings[idx] = NULL;
    }
  }
  if (strings != NULL) {
    free(strings);  strings = NULL;
  }
  if (state != NULL) {
    free(state);  state = NULL;
  }
  if (countrySetBuf != NULL) {
    free(countrySetBuf);  countrySetBuf = NULL;
  }

  return MPA_OUT_OF_MEMORY_ERR;
}


/////////////////////////////////////////////////////////////////////////////
/// Deserializes a KEY_KIVA_COUNTRY_SET tuple.
///
/// @param[in]      tuple  This tuple must be non-null and its key must
///       match KEY_KIVA_COUNTRY_SET.
///
/// @return  MPA_SUCCESS on success
///          MPA_NULL_POINTER_ERR if parameters tuple or ... is
///            NULL upon entry.
///          MPA_OUT_OF_MEMORY_ERR if a memory allocation fails
///
/////////////////////////////////////////////////////////////////////////////
static MagPebApp_ErrCode unloadPreferredLoanSet(Tuple* tuple) {
  MPA_RETURN_IF_NULL(tuple);

  size_t bufsize = strlen(tuple->value->cstring)+1;
  const char* readable = "Preferred Loans";
  char* loanSetBuf = NULL;
  ProcessingState* state = NULL;
  uint16_t num_fields = 0;
  uint16_t idx = 0;
  char* name = NULL;
  char* use = NULL;
  char* countryCode = NULL;
  MagPebApp_ErrCode mpaRet = MPA_SUCCESS;


  if ( (loanSetBuf = malloc(bufsize)) == NULL) { goto freemem; }

  APP_LOG(APP_LOG_LEVEL_DEBUG, "About to deserialize loans!");

  if (!unloadTupleStr(&loanSetBuf, bufsize, tuple, readable)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error in unloadTupleStr.");
    goto freemem;
  }

  if ( (mpaRet = KivaModel_clearPreferredLoans(dataModel)) != MPA_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error clearing preferred loan list: %s", MagPebApp_getErrMsg(mpaRet));
    return mpaRet;
  }

  if ( (state = data_processor_create(loanSetBuf, '|')) == NULL) { goto freemem; }
  num_fields = data_processor_count(state);
  APP_LOG(APP_LOG_LEVEL_INFO, "Found %d loans of interest. (%d fields)", num_fields/6, num_fields);
  for (idx = 0; idx < num_fields; idx += 6) {
    uint32_t id =        data_processor_get_int(state);
    name =               data_processor_get_string(state);
    use =                data_processor_get_string(state);
    countryCode =        data_processor_get_string(state);
    uint16_t fundedAmt = data_processor_get_int(state);
    uint16_t loanAmt =   data_processor_get_int(state);
    APP_LOG(APP_LOG_LEVEL_INFO, "[%ld] [%s] [%s] [%d] [%d] [%s]", id, name, countryCode, fundedAmt, loanAmt, use);
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
    if (name != NULL) {
      free(name);  name = NULL;
    }
    if (use != NULL) {
      free(use);  use = NULL;
    }
    if (countryCode != NULL) {
      free(countryCode);  countryCode = NULL;
    }
  }
  if (state != NULL) {
    free(state);  state = NULL;
  }
  if (loanSetBuf != NULL) {
    free(loanSetBuf);  loanSetBuf = NULL;
  }

  return MPA_SUCCESS;

freemem:
  if (name != NULL) {
    free(name);  name = NULL;
  }
  if (use != NULL) {
    free(use);  use = NULL;
  }
  if (countryCode != NULL) {
    free(countryCode);  countryCode = NULL;
  }
  if (state != NULL) {
    free(state);  state = NULL;
  }
  if (loanSetBuf != NULL) {
    free(loanSetBuf);  loanSetBuf = NULL;
  }

  return MPA_OUT_OF_MEMORY_ERR;
}


/////////////////////////////////////////////////////////////////////////////
/// Handles callbacks from the JS component
/////////////////////////////////////////////////////////////////////////////
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Inbox receive successful.");

  MagPebApp_ErrCode mpaRet = MPA_SUCCESS;
  Tuple *tuple = NULL;
  if ( (tuple = dict_find(iterator, KEY_PEBKIT_READY)) != NULL ) {
    // PebbleKit JS is ready! Safe to send messages
    pebkitReady = true;
  }

  if ( (tuple = dict_find(iterator, KEY_KIVA_COUNTRY_SET)) != NULL ) {
    if ( (mpaRet = unloadKivaCountrySet(tuple)) != MPA_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error retrieving Kiva-served countries: %s", MagPebApp_getErrMsg(mpaRet));
      return;
    }
    // Get Lender info
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
      uint16_t num_strings = data_processor_count(state);
      char** strings = malloc(sizeof(char*) * num_strings);
      for (uint16_t n = 0; n < num_strings; n += 2) {
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
    if ( (mpaRet = unloadPreferredLoanSet(tuple)) != MPA_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error retrieving preferred loans: %s", MagPebApp_getErrMsg(mpaRet));
      return;
    }
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


