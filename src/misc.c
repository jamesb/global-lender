#include <pebble.h>
#include "misc.h"


/////////////////////////////////////////////////////////////////////////////
/// Stylizes the text layer to the spec
/////////////////////////////////////////////////////////////////////////////
void textLayer_stylize(TextLayer* textLayer, const GColor bgcolor, const GColor txtColor,
                       const GTextAlignment txtAlign, const GFont txtFont) {
  if (!textLayer) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to stylize before text layer is created!");
  } else {
    text_layer_set_background_color(textLayer, bgcolor);
    text_layer_set_text_color(textLayer, txtColor);
    text_layer_set_text_alignment(textLayer, txtAlign);
    text_layer_set_font(textLayer, txtFont);
  }
}


/////////////////////////////////////////////////////////////////////////////
/// Copies a string with error handling and logging.
/////////////////////////////////////////////////////////////////////////////
bool strxcpy(char* dest, size_t bufSize, const char* src, const char* readable) {
  if (src) {
    long ret = 0;
    if ((ret = snprintf(dest, bufSize, "%s", src)) < 0) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "%s string output error. snprintf=%ld", readable, ret);
      return false;
    } else if ((size_t)ret >= bufSize) {
      APP_LOG(APP_LOG_LEVEL_WARNING, "%s string was truncated. %ld characters required.", readable, ret);
    }
    return true;
  }
  return false;
}


/////////////////////////////////////////////////////////////////////////////
/// Copies a string from src to dest, where these strings may be NULL or
/// allocated, and (*dest) will result in a NULL or will be allocated to fit
/// the src string.
///
/// - If (*dest) is NULL, it will malloc'd to an appropriate size to contain
///   the string in src.
/// - If (*dest) is non-NULL, it will be realloc'd to an appropriate size to
///   contain the string in src.
/// - If src is NULL, then (*dest) will be freed and made NULL (or dest will
///   remain NULL if it is already NULL.
///
/// @param[in]      src  The source string. May be NULL, or it may be a
///       previously-allocated string, or a string literal. If it is a
///       string, it must be null-terminated.
/// @param[in,out]  dest  Pointer to the destination char*. The char* may be
///       NULL, or it may be a previously-allocated string. If it is a
///       string, it must be null-terminated upon entry. Upon exit, the
///       char* pointed to by dest is guaranteed to be either NULL or
///       null-terminated. <em>If the char* pointed to by dest is not NULL
///       on exit, then the caller is responsible for freeing it.</em>
///
/// @return  MPA_SUCCESS on success
///          MPA_NULL_POINTER_ERR if the dest double-pointer is NULL.
///          MPA_OUT_OF_MEMORY_ERR if memory could not be allocated for dest
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode strxcpyalloc(char** dest, const char* src) {
  MPA_RETURN_IF_NULL(dest);

  size_t srcSize = strxlen(src) + 1;
  char* buffer = NULL;

  if (src == NULL) {
    if ((*dest) != NULL) {
      free((*dest));
      (*dest) = NULL;
    } else {
      // src == (*dest) == NULL
      return MPA_SUCCESS;
    }
  } else {
    // src parameter is not NULL
    if ((*dest) == NULL) {
      if ( (buffer = calloc(srcSize, sizeof(*buffer))) == NULL) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Insufficient memory");
        goto freemem;
      }
    } else {
      if ( (buffer = realloc((*dest), srcSize)) == NULL) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Insufficient memory");
        goto freemem;
      }
    }
    // buffer is allocated to the proper size and (*dest)
    // is either NULL or reallocated
    (*dest) = buffer;
    if (!strxcpy((*dest), srcSize, src, NULL)) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "strxcpy failed");
      goto freemem;
    }
  }

  return MPA_SUCCESS;


freemem:
  APP_LOG(APP_LOG_LEVEL_ERROR, "Error in strxcpyalloc()... freeing memory.");
  if ((*dest) != NULL) {
    free((*dest));
    (*dest) = NULL;
  }
  return MPA_OUT_OF_MEMORY_ERR;
}


