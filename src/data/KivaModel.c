#include <pebble.h>
#include "data/KivaModel_Internal.h"
#include "misc.h"

#define KIVA_MODEL_RETURN_IF_NULL(var)                                        \
    if (var == NULL) {                                                        \
      APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer."); \
      return KIVA_MODEL_NULL_POINTER_ERR;                                     \
    }


/**************************************************************************
 * Constructor
 **************************************************************************/
KivaModel* KivaModel_create(const KivaModel_LenderId lenderId) {
  KivaModel* newKivaModel = malloc(sizeof(KivaModel));
  if (KivaModel_init(newKivaModel, lenderId) != KIVA_MODEL_SUCCESS) {
    free(newKivaModel);
    newKivaModel = NULL;
  }
  return newKivaModel;
}


/**************************************************************************
 * Internal initialization
 **************************************************************************/
KivaModel_ErrCode KivaModel_init(KivaModel* this, const KivaModel_LenderId lenderId) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  if (!strxcpy(this->lenderInfo.id, LEN_LENDER_ID, lenderId, "Lender ID")) 
    return KIVA_MODEL_STRING_ERR;
  return KIVA_MODEL_SUCCESS;
}


/**************************************************************************
 * 
 **************************************************************************/
KivaModel_ErrCode KivaModel_destroy(KivaModel* this) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  free(this);
  return KIVA_MODEL_SUCCESS;
}


/**************************************************************************
 * 
 **************************************************************************/
KivaModel_ErrCode KivaModel_setLenderName(KivaModel* this, const KivaModel_LenderName lenderName) {  
  KIVA_MODEL_RETURN_IF_NULL(this);
  if (!strxcpy(this->lenderInfo.name, LEN_LENDER_NAME, lenderName, "Lender Name")) 
    return KIVA_MODEL_STRING_ERR;
  return KIVA_MODEL_SUCCESS;
}


/**************************************************************************
 * 
 **************************************************************************/
KivaModel_ErrCode KivaModel_setLenderLoc(KivaModel* this, const KivaModel_LenderLoc lenderLoc) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  if (!strxcpy(this->lenderInfo.loc, LEN_LENDER_LOC, lenderLoc, "Lender Name")) 
    return KIVA_MODEL_STRING_ERR;
  return KIVA_MODEL_SUCCESS;
}


/**************************************************************************
 * 
 **************************************************************************/
KivaModel_ErrCode KivaModel_setLenderLoanQty(KivaModel* this, const int lenderLoanQty) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  this->lenderInfo.loanQty = lenderLoanQty; 
  return KIVA_MODEL_SUCCESS;
}


/**************************************************************************
 * 
 **************************************************************************/
KivaModel_ErrCode KivaModel_setLenderCountryQty(KivaModel* this, const int lenderCountryQty) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  this->lenderInfo.countryQty = lenderCountryQty; 
  return KIVA_MODEL_SUCCESS;
}


/**************************************************************************
 * 
 **************************************************************************/
KivaModel_ErrCode KivaModel_setKivaCountryQty(KivaModel* this, const int kivaCountryQty) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  this->countryQty = kivaCountryQty; 
  return KIVA_MODEL_SUCCESS;
}


/**************************************************************************
 * 
 **************************************************************************/
KivaModel_ErrCode KivaModel_getLenderId(const KivaModel* this, KivaModel_LenderId buffer) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  if (!strxcpy(buffer, LEN_LENDER_ID, this->lenderInfo.id, "Lender ID")) 
    return KIVA_MODEL_STRING_ERR;
  return KIVA_MODEL_SUCCESS;
}


/**************************************************************************
 * 
 **************************************************************************/
KivaModel_ErrCode KivaModel_getLenderName(const KivaModel* this, KivaModel_LenderName buffer) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  if (!strxcpy(buffer, LEN_LENDER_NAME, this->lenderInfo.name, "Lender Name"))
    return KIVA_MODEL_STRING_ERR;
  return KIVA_MODEL_SUCCESS;
} 


/**************************************************************************
 * 
 **************************************************************************/
KivaModel_ErrCode KivaModel_getLenderLoc(const KivaModel* this, KivaModel_LenderLoc buffer) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  if (!strxcpy(buffer, LEN_LENDER_LOC, this->lenderInfo.loc, "Lender Location")) 
    return KIVA_MODEL_STRING_ERR;
  return KIVA_MODEL_SUCCESS;
}


/**************************************************************************
 * 
 **************************************************************************/
KivaModel_ErrCode KivaModel_getLenderLoanQty(const KivaModel* this, int* outval) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  *outval = this->lenderInfo.loanQty;
  return KIVA_MODEL_SUCCESS;
}


/**************************************************************************
 * 
 **************************************************************************/
KivaModel_ErrCode KivaModel_getLenderCountryQty(const KivaModel* this, int* outval) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  *outval = this->lenderInfo.countryQty;
  return KIVA_MODEL_SUCCESS;
}


/**************************************************************************
 * 
 **************************************************************************/
KivaModel_ErrCode KivaModel_getKivaCountryQty(const KivaModel* this, int* outval) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  *outval = this->countryQty;
  return KIVA_MODEL_SUCCESS;
}

