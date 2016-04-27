#pragma once

#define LEN_LENDER_ID     25
  // "3 to 24 characters long and consist of only letters and numbers."
  // https://groups.google.com/d/msg/build-kiva/9BTQKT40Wq0/S2GJrDVwDZsJ
typedef char KivaModel_LenderId[LEN_LENDER_ID];

#define LEN_LENDER_NAME   50
typedef char KivaModel_LenderName[LEN_LENDER_NAME];

#define LEN_LENDER_LOC    50
  // This is the general location string: "Lexington, KY"
typedef char KivaModel_LenderLoc[LEN_LENDER_LOC];


// Model struct typedef
typedef struct KivaModel KivaModel;
  
  
// Error codes
typedef enum KivaModel_ErrCode {
  KIVA_MODEL_SUCCESS             = 0,
  KIVA_MODEL_UNKNOWN_ERR,
  KIVA_MODEL_NULL_POINTER_ERR,
  KIVA_MODEL_STRING_ERR,
  
  KIVA_MODEL_LAST_ERRCODE
} KivaModel_ErrCode;

  
KivaModel* KivaModel_create(const KivaModel_LenderId);
KivaModel_ErrCode KivaModel_destroy(KivaModel* this);

// Setters
KivaModel_ErrCode KivaModel_setLenderName(KivaModel* this, const KivaModel_LenderName);  
KivaModel_ErrCode KivaModel_setLenderLoc(KivaModel* this, const KivaModel_LenderLoc);
KivaModel_ErrCode KivaModel_setLenderLoanQty(KivaModel* this, const int);
KivaModel_ErrCode KivaModel_setLenderCountryQty(KivaModel* this, const int);

KivaModel_ErrCode KivaModel_setKivaCountryQty(KivaModel* this, const int);

// Getters
KivaModel_ErrCode KivaModel_getLenderId(const KivaModel* this, KivaModel_LenderId);
KivaModel_ErrCode KivaModel_getLenderName(const KivaModel* this, KivaModel_LenderName);  
KivaModel_ErrCode KivaModel_getLenderLoc(const KivaModel* this, KivaModel_LenderLoc);
KivaModel_ErrCode KivaModel_getLenderLoanQty(const KivaModel* this, int*);
KivaModel_ErrCode KivaModel_getLenderCountryQty(const KivaModel* this, int*);

KivaModel_ErrCode KivaModel_getKivaCountryQty(const KivaModel* this, int*);