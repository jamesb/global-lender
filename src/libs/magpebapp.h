#pragma once


#define MPA_RETURN_IF_NULL(var)                                               \
    if (var == NULL) {                                                        \
      APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");   \
      return MPA_NULL_POINTER_ERR;                                            \
    }


// Error codes
typedef enum MagPebApp_ErrCode {
  MPA_SUCCESS             = 0,
  MPA_UNKNOWN_ERR,
  MPA_NULL_POINTER_ERR,
  MPA_STRING_ERR,
  MPA_INVALID_INPUT_ERR,
  MPA_OUT_OF_MEMORY_ERR,

  MPA_LAST_ERRCODE
} MagPebApp_ErrCode;


const char* MagPebApp_getErrMsg(const MagPebApp_ErrCode errCode);
