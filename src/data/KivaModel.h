#pragma once

// Model struct typedef
typedef struct KivaModel KivaModel;
  
  
// Error codes
typedef enum KivaModel_ErrCode {
  KIVA_MODEL_SUCCESS             = 0,
  KIVA_MODEL_UNKNOWN_ERR,
  KIVA_MODEL_NULL_POINTER_ERR,
  KIVA_MODEL_STRING_ERR,
  KIVA_MODEL_INVALID_INPUT_ERR,
  KIVA_MODEL_OUT_OF_MEMORY_ERR,
  
  KIVA_MODEL_LAST_ERRCODE
} KivaModel_ErrCode;

const char* KivaModel_getErrMsg(const KivaModel_ErrCode);

KivaModel* KivaModel_create(const char*);
KivaModel_ErrCode KivaModel_destroy(KivaModel* this);

// Setters
KivaModel_ErrCode KivaModel_setLenderId(KivaModel* this, const char*);
KivaModel_ErrCode KivaModel_setLenderName(KivaModel* const this, const char*);  
KivaModel_ErrCode KivaModel_setLenderLoc(KivaModel* this, const char*);
KivaModel_ErrCode KivaModel_setLenderLoanQty(KivaModel* this, const int);

KivaModel_ErrCode KivaModel_addKivaCountry(KivaModel* this, const char*, const char*);
KivaModel_ErrCode KivaModel_addLenderCountryId(KivaModel* this, const char*);

// Getters
KivaModel_ErrCode KivaModel_getLenderId(const KivaModel* this, char**);
KivaModel_ErrCode KivaModel_getLenderName(const KivaModel* this, char**);  
KivaModel_ErrCode KivaModel_getLenderLoc(const KivaModel* this, char**);
KivaModel_ErrCode KivaModel_getLenderLoanQty(const KivaModel* this, int*);

KivaModel_ErrCode KivaModel_getLenderCountryQty(const KivaModel* this, int*);
KivaModel_ErrCode KivaModel_getKivaCountryName(const KivaModel* this, const char*, const char**);

