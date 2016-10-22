#include <pebble.h>

// Deactivate APP_LOG in this file.
//#undef APP_LOG
//#define APP_LOG(...)

#include "misc.h"

#include "comm.h"
#include "data/KivaModel.h"
#include "libs/data-processor.h"
#include "libs/RingBuffer.h"


static KivaModel* dataModel;
static CommHandlers commHandlers;
static RingBuffer* sendBuffer;
static ClaySettings settings;
static char** strSettings;
static AppTimer* sendRetryTimer;
static uint8_t sendRetryCount;
static bool pebkitReady;

const uint8_t MAX_SEND_RETRIES = 5;
const uint8_t SEND_BUF_SIZE = 10;
const uint32_t SETTINGS_STRUCT_KEY = 0x1000;


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static Message* comm_msg_create(uint32_t msgKey, void* payload) {
  Message* newMsg = malloc(sizeof(*newMsg));
  if (newMsg == NULL) return NULL;
  
  newMsg->key = msgKey;
  newMsg->payload = payload;
  return newMsg;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static void comm_msg_destroy(Message* msg) {
  if (msg != NULL) { free(msg); msg = NULL; }
}


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
/// Deserializes a MESSAGE_KEY_KIVA_COUNTRY_SET tuple.
///
/// @param[in]      tuple  This tuple must be non-null and its key must
///       match MESSAGE_KEY_KIVA_COUNTRY_SET.
///
/// @return  MPA_SUCCESS on success
///          MPA_NULL_POINTER_ERR if parameters tuple or ... is
///            NULL upon entry.
///          MPA_OUT_OF_MEMORY_ERR if a memory allocation fails
///
/////////////////////////////////////////////////////////////////////////////
static MagPebApp_ErrCode unloadKivaCountrySet(Tuple* tuple) {
  MPA_RETURN_IF_NULL(tuple);

  const char* readable = "Kiva-Served Countries";
  char* countrySetBuf = NULL;
  ProcessingState* state = NULL;
  uint16_t num_strings = 0;
  uint16_t idx = 0;
  char** strings = NULL;
  MagPebApp_ErrCode mpaRet = MPA_SUCCESS, myret = MPA_OUT_OF_MEMORY_ERR;

  size_t bufsize = strlen(tuple->value->cstring)+1;
  if (bufsize == 1) { myret = MPA_INVALID_INPUT_ERR; goto freemem; }
  if ( (countrySetBuf = malloc(bufsize)) == NULL) { goto freemem; }

  if (!unloadTupleStr(&countrySetBuf, bufsize, tuple, readable)) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error in unloadTupleStr.");
    goto freemem;
  }

  if ( (state = data_processor_create(countrySetBuf, '|')) == NULL) { goto freemem; }
  num_strings = data_processor_count(state);
  if (num_strings == 0) { myret = MPA_INVALID_INPUT_ERR; goto freemem; }
  if ( (strings = calloc(num_strings, sizeof(*strings))) == NULL) { goto freemem; }

  for (idx = 0; idx < num_strings; idx += 2) {
    strings[idx] = data_processor_get_string(state);
    if (strings[idx] == NULL) { goto freemem; }

    strings[idx+1] = data_processor_get_string(state);
    if (strings[idx+1] == NULL) {goto freemem; }

    if ( (mpaRet = KivaModel_addKivaCountry(dataModel, strings[idx], strings[idx+1])) != MPA_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error adding Kiva country to data model: %s", MagPebApp_getErrMsg(mpaRet));
    }

    if (strings[idx+1] != NULL) { free(strings[idx+1]);  strings[idx+1] = NULL; }
    if (strings[idx] != NULL) { free(strings[idx]);  strings[idx] = NULL; }
  }

  if (strings != NULL) { free(strings);  strings = NULL; }
  if (state != NULL) { free(state);  state = NULL; }
  if (countrySetBuf != NULL) { free(countrySetBuf); countrySetBuf = NULL; }

  int kivaCountryQty = 0;
  if ( (mpaRet = KivaModel_getKivaCountryQty(dataModel, &kivaCountryQty)) != MPA_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error getting Kiva country quantity from data model: %s", MagPebApp_getErrMsg(mpaRet));
  }
  APP_LOG(APP_LOG_LEVEL_INFO, "Kiva active country total: %d", kivaCountryQty);

  return MPA_SUCCESS;

freemem:
  for (idx = 0; idx < num_strings; idx ++) {
    if (strings[idx] != NULL) { free(strings[idx]);  strings[idx] = NULL; }
  }
  if (strings != NULL) { free(strings);  strings = NULL; }
  if (state != NULL) { free(state);  state = NULL; }
  if (countrySetBuf != NULL) { free(countrySetBuf);  countrySetBuf = NULL; }

  return myret;
}


/////////////////////////////////////////////////////////////////////////////
/// Deserializes a MESSAGE_KEY_KIVA_COUNTRY_SET tuple.
///
/// @param[in]      tuple  This tuple must be non-null and its key must
///       match MESSAGE_KEY_KIVA_COUNTRY_SET.
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
    if (name != NULL) { free(name);  name = NULL; }
    if (use != NULL) { free(use);  use = NULL; }
    if (countryCode != NULL) { free(countryCode);  countryCode = NULL; }
  }
  if (state != NULL) { free(state);  state = NULL; }
  if (loanSetBuf != NULL) { free(loanSetBuf);  loanSetBuf = NULL; }

  return MPA_SUCCESS;

freemem:
  if (name != NULL) { free(name);  name = NULL; }
  if (use != NULL) { free(use);  use = NULL; }
  if (countryCode != NULL) { free(countryCode);  countryCode = NULL; }
  if (state != NULL) { free(state);  state = NULL; }
  if (loanSetBuf != NULL) { free(loanSetBuf);  loanSetBuf = NULL; }

  return MPA_OUT_OF_MEMORY_ERR;
}


/////////////////////////////////////////////////////////////////////////////
/// Handles callbacks from the JS component
/////////////////////////////////////////////////////////////////////////////
static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Inbox receive successful.");

  MagPebApp_ErrCode mpaRet = MPA_SUCCESS;
  Tuple *tuple = NULL;
  
  if ( (tuple = dict_find(iterator, MESSAGE_KEY_PEBKIT_READY)) != NULL ) {
    // PebbleKit JS is ready! Safe to send messages
    pebkitReady = true;
    APP_LOG(APP_LOG_LEVEL_INFO, "PebbleKit JS sent ready message!");
    
    comm_enqMsg(comm_msg_create(MESSAGE_KEY_GET_KIVA_INFO, ""));
    
    bool empty = true;
    if ( (sendBuffer != NULL) && ( (mpaRet = RingBuffer_empty(sendBuffer, &empty)) != MPA_SUCCESS) ) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error checking sendBuffer: %s", MagPebApp_getErrMsg(mpaRet));
        return;
    }
    if (!empty) { comm_sendBufMsg(); }
  }

  if ( (tuple = dict_find(iterator, MESSAGE_KEY_KIVA_COUNTRY_SET)) != NULL ) {
    if ( (mpaRet = unloadKivaCountrySet(tuple)) != MPA_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error retrieving Kiva-served countries: %s", MagPebApp_getErrMsg(mpaRet));
      return;
    }
    // Ready to load saved data (like Lender ID) from persistent memory now.
    comm_loadPersistent();
  }

  if ( (tuple = dict_find(iterator, MESSAGE_KEY_LENDER_ID)) != NULL ) {
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
    if (lenderIdBuf != NULL) { free(lenderIdBuf); lenderIdBuf = NULL; }
    comm_savePersistent();
    comm_getLenderInfo();
  }

  if ( (tuple = dict_find(iterator, MESSAGE_KEY_LENDER_NAME)) != NULL ) {
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
    if (lenderNameBuf != NULL) { free(lenderNameBuf); lenderNameBuf = NULL; }
  }

  if ( (tuple = dict_find(iterator, MESSAGE_KEY_LENDER_LOC)) != NULL ) {
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
    if (lenderLocBuf != NULL) { free(lenderLocBuf); lenderLocBuf = NULL; }
  }

  if ( (tuple = dict_find(iterator, MESSAGE_KEY_LENDER_LOAN_QTY)) != NULL ) {
    const char* readable = "Lender Loan Quantity";
    long int lenderLoanQty = 0;
    if (unloadTupleLong(&lenderLoanQty, tuple, readable)) {
      if ( (mpaRet = KivaModel_setLenderLoanQty(dataModel, (int)lenderLoanQty)) != MPA_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error setting %s in data model: %s", readable, MagPebApp_getErrMsg(mpaRet));
      }
    }
  }

  if ( (tuple = dict_find(iterator, MESSAGE_KEY_LENDER_COUNTRY_SET)) != NULL ) {
    const char* readable = "Lender-Supported Countries";
    size_t bufsize = strlen(tuple->value->cstring)+1;
    char* countrySetBuf = NULL;
    countrySetBuf = malloc(bufsize);
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
        if (strings[n] != NULL) { free(strings[n]); strings[n] = NULL; }
        if (strings[n+1] != NULL) { free(strings[n+1]); strings[n+1] = NULL; }
      }
      if (strings != NULL) { free(strings); strings = NULL; }
      if (state != NULL) { free(state); state = NULL; }
    }
    if (countrySetBuf != NULL) { free(countrySetBuf); countrySetBuf = NULL; }
    comm_getPreferredLoans();
  }

  if ( (tuple = dict_find(iterator, MESSAGE_KEY_LOAN_SET)) != NULL ) {
    if ( (mpaRet = unloadPreferredLoanSet(tuple)) != MPA_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error retrieving preferred loans: %s", MagPebApp_getErrMsg(mpaRet));
      return;
    }
  }

  if (!commHandlers.updateViewData) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");
  } else {
    (*commHandlers.updateViewData)(dataModel);
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
  comm_startResendTimer();
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
/// Queue data and send to PebbleKit.
/////////////////////////////////////////////////////////////////////////////
void comm_enqMsg(Message* msg) {
  MagPebApp_ErrCode mpaRet = MPA_SUCCESS;
  if (msg == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Tried to buffer a null message.");
    return;
  }
  
  if (sendBuffer == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Send buffer is null.");
    return;
  }
  
  if ( (mpaRet = RingBuffer_write(sendBuffer, (void*)msg)) != MPA_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error buffering message: %s", MagPebApp_getErrMsg(mpaRet));
    comm_msg_destroy(msg);
    return;
  }
  
  comm_sendBufMsg();
}


/////////////////////////////////////////////////////////////////////////////
/// Send buffered data to PebbleKit.
/////////////////////////////////////////////////////////////////////////////
void comm_sendBufMsg() {
  MagPebApp_ErrCode mpaRet = MPA_SUCCESS;
  
  void* data = NULL;
  if ( (mpaRet = RingBuffer_peek(sendBuffer, &data)) != MPA_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error reading buffered message: %s", MagPebApp_getErrMsg(mpaRet));
    return;
  }
  
  Message* msg = (Message*) data;
  if (msg == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Send buffer contained a null message.");
    return;
  }
  
  // At this point, we know there is a non-null message in the send buffer.
  
  // Check if PebbleKit JS is ready to receive...
  if (!comm_pebkitReady()) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Buffering message to phone until PebbleKit JS is ready...");
    comm_startResendTimer();
    return;
  }

  // Prepare the outbox buffer for this message
  DictionaryIterator *outIter;
  AppMessageResult result = app_message_outbox_begin(&outIter);
  if (result != APP_MSG_OK) {
    // The outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_WARNING, "Error preparing the outbox for message %d.  Result: %d", (int)msg->key, (int)result);
    comm_startResendTimer();
    return;
  }
  
  // Ready to write to app message outbox...
  dict_write_cstring(outIter, msg->key, msg->payload);

  // Send this message
  result = app_message_outbox_send();
    
  if(result != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Error sending the outbox for message %d.  Result: %d", (int)msg->key, (int)result);
    comm_startResendTimer();
    return;
  }
  
  // Successful send attempt!
  APP_LOG(APP_LOG_LEVEL_INFO, "Sent outbox message %d!", (int)msg->key);
  sendRetryCount = 0;
  RingBuffer_drop(sendBuffer);
  comm_msg_destroy(msg);
  if (sendRetryTimer != NULL) { app_timer_cancel(sendRetryTimer);  sendRetryTimer = NULL; }
}


/////////////////////////////////////////////////////////////////////////////
/// Starts a backoff timer to retry the first message in the send buffer.
/////////////////////////////////////////////////////////////////////////////
void comm_startResendTimer() {
  MagPebApp_ErrCode mpaRet = MPA_SUCCESS;
  
  void* data = NULL;
  if ( (mpaRet = RingBuffer_peek(sendBuffer, &data)) != MPA_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error reading buffered message: %s", MagPebApp_getErrMsg(mpaRet));
    return;
  }
  
  Message* msg = (Message*) data;
  if (msg == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Send buffer contained a null message.");
    return;
  }
  
  if (sendRetryCount < MAX_SEND_RETRIES) {
    sendRetryCount++;
    uint16_t retryIntervalMs = (sendRetryCount * 1000);
    APP_LOG(APP_LOG_LEVEL_INFO, "Retrying message (%d) send in %d ms...", (int)msg->key, retryIntervalMs);
    sendRetryTimer = app_timer_register(retryIntervalMs, comm_sendBufMsg, NULL);
  } else {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Max retries failed. Abandoning message (%d).", (int)msg->key);
    sendRetryCount = 0;
    RingBuffer_drop(sendBuffer);
    if (sendRetryTimer != NULL) { app_timer_cancel(sendRetryTimer);  sendRetryTimer = NULL; }
  }
}


/////////////////////////////////////////////////////////////////////////////
/// Send data to PebbleKit.
/////////////////////////////////////////////////////////////////////////////
void comm_sendMsg(const Message* msg) {
  if (msg == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Tried to send a null message.");
    return;
  }
  
  if (!comm_pebkitReady()) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Tried to send a message from the watch before PebbleKit JS is ready.");
    return;
  }

  // Declare the dictionary's iterator
  DictionaryIterator *outIter;

  // Prepare the outbox buffer for this message
  AppMessageResult result = app_message_outbox_begin(&outIter);
  if (result == APP_MSG_OK) {
    dict_write_cstring(outIter, msg->key, msg->payload);

    // Send this message
    result = app_message_outbox_send();
  
    if(result == APP_MSG_OK) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Sent outbox message %d!", (int)msg->key);
    } else {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error sending the outbox for message %d.  Result: %d", (int)msg->key, (int)result);
    }
    
  } else {
    // The outbox cannot be used right now
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error preparing the outbox for message %d.  Result: %d", (int)msg->key, (int)result);
  }
}


/////////////////////////////////////////////////////////////////////////////
/// Requests PebbleKit to send lender information (name, location, etc).
/////////////////////////////////////////////////////////////////////////////
void comm_getLenderInfo() {
  if (dataModel == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Kiva Model is not yet initialized.");
    return;
  }

  MagPebApp_ErrCode mpaRet;
  char* lenderId = NULL;
  if ( (mpaRet = KivaModel_getLenderId(dataModel, &lenderId)) != MPA_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not retrieve lender ID: %s", MagPebApp_getErrMsg(mpaRet));
    return;
  }

  // JRB TODO: Validate that lender ID is not blank.
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Get lender info for ID: %s", lenderId);
  comm_enqMsg(comm_msg_create(MESSAGE_KEY_GET_LENDER_INFO, lenderId));
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
    if ( (mpaRet = KivaModel_getLenderCountryCodes(dataModel, false, &countryCodes)) != MPA_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Could not retrieve lender country codes: %s", MagPebApp_getErrMsg(mpaRet));
      return;
    }

    APP_LOG(APP_LOG_LEVEL_DEBUG, "Get loans for country codes: %s", countryCodes);
    comm_enqMsg(comm_msg_create(MESSAGE_KEY_GET_PREFERRED_LOANS, countryCodes));
    if (countryCodes != NULL) { free(countryCodes); countryCodes = NULL; }
}


/////////////////////////////////////////////////////////////////////////////
/// Saves app settings to persistent storage.
/////////////////////////////////////////////////////////////////////////////
void comm_savePersistent() {
  if (dataModel == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Kiva Model is not yet initialized.");
    return;
  }

  persist_write_data(SETTINGS_STRUCT_KEY, &settings, sizeof(settings));
  
  // Write all string settings
  MagPebApp_ErrCode mpaRet;
  for (int keyIdx=0; keyIdx<LAST_STR_SETTING; keyIdx++) {
    // Fetch data stored elsewhere
    switch(keyIdx) {
      case LENDER_ID_STR_SETTING: {
        if (strSettings[keyIdx] != NULL) { free(strSettings[keyIdx]); strSettings[keyIdx] = NULL; }
        if ( (mpaRet = KivaModel_getLenderId(dataModel, &strSettings[keyIdx])) != MPA_SUCCESS) {
          APP_LOG(APP_LOG_LEVEL_ERROR, "Could not retrieve lender ID: %s", MagPebApp_getErrMsg(mpaRet));
          return;
        }
        break;
      }
    } // end switch(keyIdx)
    
    // Write data to persistent memory on watch
    status_t result = 0;
    if ( (result = persist_write_string(keyIdx, strSettings[keyIdx])) < 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Could not write string #%d to persistent storage. Error: %ld", keyIdx, result);
    } else {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "Wrote string #%d (%s) to persistent storage.", keyIdx, strSettings[keyIdx]);
    }
        
  } // end for
  return;

}


/////////////////////////////////////////////////////////////////////////////
/// Loads app settings from persistent storage.
/////////////////////////////////////////////////////////////////////////////
void comm_loadPersistent() {
  if (dataModel == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Kiva Model is not yet initialized.");
    return;
  }

  if (persist_exists(SETTINGS_STRUCT_KEY)) {
    persist_read_data(SETTINGS_STRUCT_KEY, &settings, sizeof(settings));
  }
  
  // Read all string settings
  MagPebApp_ErrCode mpaRet;
  int keyIdx;
  for (keyIdx=0; keyIdx<LAST_STR_SETTING; keyIdx++) {
    if (persist_exists(keyIdx)) {
      if (strSettings[keyIdx] != NULL) { free(strSettings[keyIdx]); strSettings[keyIdx] = NULL; }
      int size = persist_get_size(keyIdx);
      if ( (strSettings[keyIdx] = calloc(size, sizeof(*strSettings[keyIdx])) ) == NULL) { goto freemem; }
      
      // Read the string from persistent watch storage.
      status_t result;
      if ( (result = persist_read_string(keyIdx, strSettings[keyIdx], size)) < 0) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Could not read string #%d from persistent storage. Error: %ld", keyIdx, result);
      } else {
        APP_LOG(APP_LOG_LEVEL_DEBUG, "Read string #%d (%s) from persistent storage.", keyIdx, strSettings[keyIdx]);
      }
        
      switch(keyIdx) {
        case LENDER_ID_STR_SETTING: {
          if ( (mpaRet = KivaModel_setLenderId(dataModel, strSettings[keyIdx])) != MPA_SUCCESS) {
            APP_LOG(APP_LOG_LEVEL_ERROR, "Error setting %s in data model: %s", "Lender ID", MagPebApp_getErrMsg(mpaRet));
          } else {
            comm_getLenderInfo();
          }
          break;
        }
      } // end switch(keyIdx)
    } // end if persist_exists(keyIdx)
  } // end for
  
  return;
  
freemem:
  APP_LOG(APP_LOG_LEVEL_ERROR, "Error... freeing memory");
  if (strSettings[keyIdx] != NULL) { free(strSettings[keyIdx]); strSettings[keyIdx] = NULL; }
}


/////////////////////////////////////////////////////////////////////////////
/// Callback for TickTimerService
/////////////////////////////////////////////////////////////////////////////
void comm_tickHandler(struct tm *tick_time, TimeUnits units_changed) {
  { // limiting timebuf in a local scope
    const size_t bufsize = 40;
    char timebuf[bufsize];
    size_t ret = 0;

    if ( (ret = strftime(timebuf, (int)bufsize, "%a, %d %b %Y %T %z", tick_time)) == 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error returned: %d", ret);
      return;
    }
    HEAP_LOG("tick_handler");
  }

  if (!commHandlers.updateViewClock) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");
  }
  (*commHandlers.updateViewClock)(tick_time);

  // Get update every 10 minutes
  if(tick_time->tm_min % 10 == 0) {
    comm_getLenderInfo();
  }
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
  
  strSettings = NULL;
  if ( (strSettings = calloc(LAST_STR_SETTING, sizeof(*strSettings)) ) == NULL) { goto freemem; }
  // Initialize each string setting to NULL.
  for (int idx=0; idx<LAST_STR_SETTING; idx++) {
    strSettings[idx] = NULL;
  }
  
  sendBuffer = NULL;
  if ( (sendBuffer = RingBuffer_create(SEND_BUF_SIZE)) == NULL) { APP_LOG(APP_LOG_LEVEL_ERROR, "Could not initialize send buffer."); }

  // Register callbacks
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);

  // Open AppMessage
  // JRB TODO: Consider optimizing buffer sizes in the future if memory is constrained.
  app_message_open(app_message_inbox_size_maximum(), 300);
  
  return;
  
freemem:
  APP_LOG(APP_LOG_LEVEL_ERROR, "Error... freeing memory");
  if (strSettings != NULL) { free(strSettings);  strSettings = NULL; }
}


/////////////////////////////////////////////////////////////////////////////
/// Closes communication and frees memory.
/////////////////////////////////////////////////////////////////////////////
void comm_close() {
  if (dataModel != NULL) {
    KivaModel_destroy(dataModel);  dataModel = NULL;
  }
  
  if (sendBuffer != NULL) {
    RingBuffer_destroy(sendBuffer);  sendBuffer = NULL;
  }
  
  if (strSettings != NULL) {
    for (int idx=0; idx<LAST_STR_SETTING; idx++) {
      if (strSettings[idx] != NULL) { free(strSettings[idx]); strSettings[idx] = NULL; }
    }
    if (strSettings != NULL) { free(strSettings); strSettings = NULL; }
  }
  
  app_message_deregister_callbacks();
}


