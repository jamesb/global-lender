#pragma once

#include <pebble.h>
#include "../libs/magpebapp.h"


// Model struct typedef
typedef struct KivaModel KivaModel;


typedef struct LoanInfo {
  uint32_t id;              ///< numeric loan ID
  char*    name;            ///< the name of the loan (generally the individual or group receiving the loan)
  char*    use;             ///< description of how the loan will be used
  char*    countryCode;     ///< the two-character ISO-3361 code
  uint16_t fundedAmt;       ///< amount (USD) of funding received by the loan
  uint16_t loanAmt;         ///< amount (USD) requested for the loan
} LoanInfo;


typedef struct KivaModel_PrefLoan_CIter {
  const LoanInfo* data;     ///< pointer to the LoanInfo struct this iterator points to
  uint16_t        idx;      ///< enumerates the iteration count, starting with zero
  const void*     internal; ///< internal state used by KivaModel -- clients: do not touch <!-- LoanRec* -->
} KivaModel_PrefLoan_CIter;


typedef struct KivaModel_Modified {
  unsigned int kivaCountryQty     : 1;
  unsigned int lenderCountryQty   : 1;
  unsigned int lenderLoanQty      : 1;
  unsigned int preferredLoanQty   : 1;
} KivaModel_Modified;


KivaModel* KivaModel_create(const char*);
MagPebApp_ErrCode KivaModel_destroy(KivaModel* this);

// Setters
MagPebApp_ErrCode KivaModel_setLenderId(KivaModel* this, const char*);
MagPebApp_ErrCode KivaModel_setLenderName(KivaModel* const this, const char*);
MagPebApp_ErrCode KivaModel_setLenderLoc(KivaModel* this, const char*);
MagPebApp_ErrCode KivaModel_setLenderLoanQty(KivaModel* this, const int);

MagPebApp_ErrCode KivaModel_addLenderCountry(KivaModel* this, const char*, const char*);
MagPebApp_ErrCode KivaModel_addKivaCountry(KivaModel* this, const char*, const char*);

MagPebApp_ErrCode KivaModel_addPreferredLoan(KivaModel* this, const LoanInfo);
MagPebApp_ErrCode KivaModel_clearPreferredLoans(KivaModel* this);

// Getters
MagPebApp_ErrCode KivaModel_getMods(const KivaModel* this, KivaModel_Modified*);

MagPebApp_ErrCode KivaModel_getLenderId(const KivaModel* this, char**);
MagPebApp_ErrCode KivaModel_getLenderName(const KivaModel* this, char**);
MagPebApp_ErrCode KivaModel_getLenderLoc(const KivaModel* this, char**);
MagPebApp_ErrCode KivaModel_getLenderLoanQty(const KivaModel* this, int*);

MagPebApp_ErrCode KivaModel_getLenderCountryQty(const KivaModel* this, int*);
MagPebApp_ErrCode KivaModel_getLenderCountryCodes(const KivaModel* this, const bool, char**);
MagPebApp_ErrCode KivaModel_getKivaCountryQty(const KivaModel* this, int*);
MagPebApp_ErrCode KivaModel_getKivaCountryName(const KivaModel* this, const char*, const char**);

MagPebApp_ErrCode KivaModel_getPreferredLoanQty(const KivaModel* this, uint16_t*);
KivaModel_PrefLoan_CIter* KivaModel_firstPrefLoan(const KivaModel* this);
KivaModel_PrefLoan_CIter* KivaModel_nextPrefLoan(KivaModel_PrefLoan_CIter*);
void KivaModel_donePrefLoan(KivaModel_PrefLoan_CIter*);


