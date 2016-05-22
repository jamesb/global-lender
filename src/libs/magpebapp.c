#include "magpebapp.h"

/////////////////////////////////////////////////////////////////////////////
/// Returns error messages for MagApp error codes.
/////////////////////////////////////////////////////////////////////////////
const char* MagPebApp_getErrMsg(const MagPebApp_ErrCode errCode) {
  switch (errCode) {
    case MPA_SUCCESS:           return "success";
    case MPA_UNKNOWN_ERR:       return "unknown error";
    case MPA_NULL_POINTER_ERR:  return "operation on null pointer";
    case MPA_STRING_ERR:        return "string operation error";
    case MPA_INVALID_INPUT_ERR: return "invalid input error";
    case MPA_OUT_OF_MEMORY_ERR: return "out of memory error";
    default:                    return "unlisted error";
  } // end switch
}


