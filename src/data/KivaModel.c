#include <pebble.h>

// Deactivate APP_LOG in this file.
//#undef APP_LOG
//#define APP_LOG(...)

#include "data/KivaModel_Internal.h"
#include "misc.h"




/////////////////////////////////////////////////////////////////////////////
/// Allocates a CountryRec pointer.
/// @param[out]     cntry   double-pointer to CountryRec; must be NULL on entry
/////////////////////////////////////////////////////////////////////////////
static MagPebApp_ErrCode KivaModel_CountryRec_create(CountryRec** cntry) {
  if (*cntry != NULL) { return MPA_INVALID_INPUT_ERR; }

  *cntry = (CountryRec*) malloc(sizeof(CountryRec));
  if (*cntry == NULL) { goto freemem; }
  (*cntry)->id = NULL;
  (*cntry)->name = NULL;

  return MPA_SUCCESS;

freemem:
  if (*cntry != NULL) { free(*cntry); }
  return MPA_OUT_OF_MEMORY_ERR;
}


/////////////////////////////////////////////////////////////////////////////
/// Initializes a CountryRec pointer and its members.
/// @param[in,out]  this  Pointer to CountryRec; must be already
///       allocated upon entry, but heap-allocated members must be NULL
/// @param[in]      countryId  ID of country (ISO-3316); must be non-null;
/// @param[in]      countryName  Name of country; must be non-null;
/////////////////////////////////////////////////////////////////////////////
static MagPebApp_ErrCode KivaModel_CountryRec_init(CountryRec* this, const char* countryId, const char* countryName) {
  MPA_RETURN_IF_NULL(this);
  if (this->id != NULL)   {
    APP_LOG(APP_LOG_LEVEL_ERROR, "CountryRec id must be NULL.");
    return MPA_INVALID_INPUT_ERR;
  }
  if (this->name != NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "CountryRec name must be NULL.");
    return MPA_INVALID_INPUT_ERR;
  }
  if (countryId == NULL)   {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Init parameter countryId must not be NULL.");
    return MPA_INVALID_INPUT_ERR;
  }
  if (countryName == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Init parameter countryName must not be NULL.");
    return MPA_INVALID_INPUT_ERR;
  }

  if ( (this->id = malloc(strlen(countryId) + 1)) == NULL) { goto freemem; }
  strcpy(this->id, countryId);

  if ( (this->name = malloc(strlen(countryName) + 1)) == NULL) { goto freemem; }
  strcpy(this->name, countryName);

  return MPA_SUCCESS;

freemem:
  if (this->id != NULL)   { free(this->id);   this->id = NULL; }
  if (this->name != NULL) { free(this->name); this->name = NULL; }
  return MPA_OUT_OF_MEMORY_ERR;
}


/////////////////////////////////////////////////////////////////////////////
/// Frees all memory associated with a CountryRec pointer.
/// @param[in,out]  this  Pointer to CountryRec; must be already allocated
/////////////////////////////////////////////////////////////////////////////
static MagPebApp_ErrCode KivaModel_CountryRec_destroy(CountryRec* this) {
  MPA_RETURN_IF_NULL(this);
  //HEAP_LOG("Freeing CountryRec");
  if (this->id != NULL)   { free(this->id);   this->id = NULL; }
  if (this->name != NULL) { free(this->name); this->name = NULL; }
  free(this); this = NULL;
  return MPA_SUCCESS;
}




/////////////////////////////////////////////////////////////////////////////
/// Allocates a LoanRec pointer.
/// @param[out]     loan  double-pointer to LoanRec; must be NULL on entry
/////////////////////////////////////////////////////////////////////////////
static MagPebApp_ErrCode KivaModel_LoanRec_create(LoanRec** loan) {
  if (*loan != NULL) { return MPA_INVALID_INPUT_ERR; }

  *loan = (LoanRec*) malloc(sizeof(LoanRec));
  if (*loan == NULL) { goto freemem; }
  (*loan)->data.name = NULL;
  (*loan)->data.use = NULL;
  (*loan)->data.countryCode = NULL;

  return MPA_SUCCESS;

freemem:
  if (*loan != NULL) { free(*loan); }
  return MPA_OUT_OF_MEMORY_ERR;
}


/////////////////////////////////////////////////////////////////////////////
/// Initializes a LoanRec pointer and its members.
/// @param[in,out]  this  Pointer to LoanRec; must be already
///       allocated upon entry, but heap-allocated members must be NULL
/// @param[in]      loanInfo  A fully initialized LoanInfo variable. All heap-
///       allocated members are expected to be non-NULL, allocated and
///       populated with data.
/////////////////////////////////////////////////////////////////////////////
static MagPebApp_ErrCode KivaModel_LoanRec_init(LoanRec* this, const LoanInfo loanInfo) {
  MPA_RETURN_IF_NULL(this);
  if (this->data.name != NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "LoanRec name must be NULL.");
    return MPA_INVALID_INPUT_ERR;
  }
  if (this->data.use != NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "LoanRec use must be NULL.");
    return MPA_INVALID_INPUT_ERR;
  }
  if (this->data.countryCode != NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "LoanRec country code must be NULL.");
    return MPA_INVALID_INPUT_ERR;
  }

  if ( (this->data.name = malloc(strlen(loanInfo.name) + 1)) == NULL) { goto freemem; }
  strcpy(this->data.name, loanInfo.name);

  if ( (this->data.use = malloc(strlen(loanInfo.use) + 1)) == NULL) { goto freemem; }
  strcpy(this->data.use, loanInfo.use);

  if ( (this->data.countryCode = malloc(strlen(loanInfo.countryCode) + 1)) == NULL) { goto freemem; }
  strcpy(this->data.countryCode, loanInfo.countryCode);

  return MPA_SUCCESS;

freemem:
  if (this->data.name != NULL) { free(this->data.name); this->data.name = NULL; }
  if (this->data.use != NULL) { free(this->data.use);   this->data.use = NULL; }
  if (this->data.countryCode != NULL) { free(this->data.countryCode);   this->data.countryCode = NULL; }
  return MPA_OUT_OF_MEMORY_ERR;
}


/////////////////////////////////////////////////////////////////////////////
/// Frees all memory associated with a LoanRec pointer.
/// @param[in,out]  this  Pointer to LoanRec; must be already allocated
/////////////////////////////////////////////////////////////////////////////
static MagPebApp_ErrCode KivaModel_LoanRec_destroy(LoanRec* this) {
  MPA_RETURN_IF_NULL(this);
  //HEAP_LOG("Freeing LoanRec");
  if (this->data.name != NULL) { free(this->data.name); this->data.name = NULL; }
  if (this->data.use != NULL) { free(this->data.use); this->data.use = NULL; }
  if (this->data.countryCode != NULL) { free(this->data.countryCode); this->data.countryCode = NULL; }
  free(this); this = NULL;
  return MPA_SUCCESS;
}




/////////////////////////////////////////////////////////////////////////////
/// Constructor
/// @param[in]      lenderId   Pointer to the lender ID C-string. Identifies
///       the lender uniquely on kiva.org. <em>Ownership is not transferred
///       to this function, so the caller is still responsible for freeing
///       this variable.</em>
/////////////////////////////////////////////////////////////////////////////
KivaModel* KivaModel_create(const char* lenderId) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating KivaModel [%s]", lenderId);
  int mpaRet;

  KivaModel* newKivaModel = malloc(sizeof(KivaModel));
  if ( (mpaRet = KivaModel_init(newKivaModel, lenderId)) != MPA_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not initialize Kiva Model: %s", MagPebApp_getErrMsg(mpaRet));
    KivaModel_destroy(newKivaModel);  newKivaModel = NULL;
  }

  return newKivaModel;
}


/////////////////////////////////////////////////////////////////////////////
/// Destroys KivaModel and frees allocated memory.
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_destroy(KivaModel* this) {
  MPA_RETURN_IF_NULL(this);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Destroying KivaModel [%s]", this->lenderInfo.id);
  MagPebApp_ErrCode mpaRet;

  // Free memory for this->kivaCountries and its data
  CountryRec* cntry = NULL; CountryRec* tmpCntry = NULL;
  HASH_ITER(hh, this->kivaCountries, cntry, tmpCntry) {
    if ( (mpaRet = KivaModel_CountryRec_destroy(cntry)) != MPA_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error trying to destroy a country record: %s", MagPebApp_getErrMsg(mpaRet));
    }
    HASH_DEL(this->kivaCountries, cntry);
    cntry = NULL;
  }
  HEAP_LOG("Freed kivaCountries and its data.");

  // Free memory for this->prefLoans and its data
  if ( (mpaRet = KivaModel_clearPreferredLoans(this)) != MPA_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error freeing preferred loan list: %s", MagPebApp_getErrMsg(mpaRet));
  }
  HEAP_LOG("Freed prefLoans and its data.");

  if (this->lenderInfo.loc != NULL)  { free(this->lenderInfo.loc);  this->lenderInfo.loc = NULL; }
  if (this->lenderInfo.name != NULL) { free(this->lenderInfo.name); this->lenderInfo.name = NULL; }
  if (this->lenderInfo.id != NULL)   { free(this->lenderInfo.id);   this->lenderInfo.id = NULL; }

  HEAP_LOG("Freed lenderInfo and its data.");

  free(this); this = NULL;
  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Internal initialization
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[in]      lenderId   Pointer to the lender ID C-string. Identifies
///       the lender uniquely on kiva.org. <em>Ownership is not transferred
///       to this function, so the caller is still responsible for freeing
///       this variable.</em>
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_init(KivaModel* this, const char* lenderId) {
  MPA_RETURN_IF_NULL(this);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Initializing KivaModel [%s]", lenderId);
  MagPebApp_ErrCode mpaRet;

  this->lenderInfo.id = NULL;
  this->lenderInfo.name = NULL;
  this->lenderInfo.loc = NULL;
  this->lenderInfo.loanQty = 0;

  this->kivaCountries = NULL;

  if ( (mpaRet = KivaModel_setLenderId(this, lenderId)) != MPA_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not set Lender ID. Destroying Kiva model.");
    KivaModel_destroy(this);  this = NULL;
  }

  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Sets the Lender ID and resets any lender-specific data so that it
/// will be NULL or empty until it is reloaded.
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[in]      lenderId   Pointer to the lender ID C-string. Identifies
///       the lender uniquely on kiva.org. <em>Ownership is not transferred
///       to this function, so the caller is still responsible for freeing
///       this variable.</em>
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_setLenderId(KivaModel* this, const char* lenderId) {
  MPA_RETURN_IF_NULL(this);

  if (lenderId == NULL) { return MPA_INVALID_INPUT_ERR; }
  if ( (this->lenderInfo.id != NULL) && (strcmp(lenderId, this->lenderInfo.id) == 0) ) {
    return MPA_SUCCESS;
  }

  size_t bufsize = strlen(lenderId)+1;
  char* tmp = realloc(this->lenderInfo.id, bufsize);
  if (tmp == NULL) { return MPA_OUT_OF_MEMORY_ERR; }
  this->lenderInfo.id = tmp;

  if (!strxcpy(this->lenderInfo.id, bufsize, lenderId, "Lender ID")) { return MPA_STRING_ERR; }
  // Clear out any lender-dependent fields since the lender ID has changed.
  if (this->lenderInfo.name != NULL) { free(this->lenderInfo.name); this->lenderInfo.name = NULL; }
  if (this->lenderInfo.loc != NULL)  { free(this->lenderInfo.loc);  this->lenderInfo.loc = NULL; }
  this->lenderInfo.loanQty = 0;

  // Reset the CountryRec lender-dependent fields to NULL
  CountryRec* cntry = NULL;
  for (cntry=this->kivaCountries; cntry!=NULL; cntry=cntry->hh.next) {
    cntry->lenderSupport = false;
  }

  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Sets the lender name for the registered lender.
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[in]      lenderName   Pointer to the lender name C-string, as it
///       is written on kiva.org. This may be a first name and last name,
///       but is not limited to that format. <em>Ownership is not transferred
///       to this function, so the caller is still responsible for freeing
///       this variable.</em>
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_setLenderName(KivaModel* this, const char* lenderName) {
  MPA_RETURN_IF_NULL(this);

  if (lenderName == NULL) { return MPA_INVALID_INPUT_ERR; }
  if (this->lenderInfo.name != NULL) { free(this->lenderInfo.name); this->lenderInfo.name = NULL; }
  size_t bufsize = strlen(lenderName) + 1;
  char* tmp = realloc(this->lenderInfo.name, bufsize);
  if (tmp == NULL) { return MPA_OUT_OF_MEMORY_ERR; }
  this->lenderInfo.name = tmp;

  if (!strxcpy(this->lenderInfo.name, bufsize, lenderName, "Lender Name")) {
    return MPA_STRING_ERR;
  }
  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Sets the lender location for the registered lender.
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[in]      lenderName   Pointer to the lender location C-string, as
///       it appears on kiva.org. This is a free format text field.
///       <em>Ownership is not transferred to this function, so the caller
///       is still responsible for freeing this variable.</em>
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_setLenderLoc(KivaModel* this, const char* lenderLoc) {
  MPA_RETURN_IF_NULL(this);

  if (lenderLoc == NULL) { return MPA_INVALID_INPUT_ERR; }
  if (this->lenderInfo.loc != NULL) { free(this->lenderInfo.loc); this->lenderInfo.loc = NULL; }
  size_t bufsize = strlen(lenderLoc) + 1;
  char* tmp = realloc(this->lenderInfo.loc, bufsize);
  if (tmp == NULL) { return MPA_OUT_OF_MEMORY_ERR; }
  this->lenderInfo.loc = tmp;

  if (!strxcpy(this->lenderInfo.loc, bufsize, lenderLoc, "Lender Loc")) {
    return MPA_STRING_ERR;
  }
  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_setLenderLoanQty(KivaModel* this, const int lenderLoanQty) {
  MPA_RETURN_IF_NULL(this);
  this->lenderInfo.loanQty = lenderLoanQty;
  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Adds a country ID to the list of countries supported by the lender.
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[in]      countryId  ID of the country to add; a two-character
///       ISO-3361 country code. <em>Ownership is not transferred from the
///       caller, so the caller should free this variable.</em>
///       If the countryId is not yet in the list of recognized Kiva
///       countries, then it will be added to the hash.
/// @param[in]      countryName  Name of the country to add. If countryId
///       is already a recognized Kiva country, then the countryName will be
///       ignored. If the countryId is not yet in the list of countries that
///       this KivaModel recognizes, then this will be the new country name
///       for this ID. If NULL, then the countryId will be used as the name.
///       <em>Ownership is not transferred from the caller, so
///       the caller should free this variable.</em>
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_addLenderCountry(KivaModel* this, const char* countryId, const char* countryName) {
  MPA_RETURN_IF_NULL(this);
  MagPebApp_ErrCode mpaRet;

  CountryRec *cntry = NULL;
  HASH_FIND_STR(this->kivaCountries, countryId, cntry);

  if (cntry == NULL) {
    // Value of countryId is not already a key in the hash table; add new record.
    APP_LOG(APP_LOG_LEVEL_INFO, "Lender supports a country (%s -> %s) that isn't in our official Kiva list.", countryId, countryName);

    const char* tmpName = NULL;
    tmpName = (countryName == NULL ? countryId : countryName);

    if ( (mpaRet = KivaModel_addKivaCountry(this, countryId, tmpName)) != MPA_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error adding new country (%s): %s", countryId, MagPebApp_getErrMsg(mpaRet));
      return mpaRet;
    }

    HASH_FIND_STR(this->kivaCountries, countryId, cntry);
    if (cntry == NULL) {
      return MPA_NULL_POINTER_ERR;
    }

    // This country wasn't in the Kiva list. Mark it as inactive.
    cntry->kivaActive = false;
  }

  // We have the correct country from the hash table; mark it lender-supported.
  cntry->lenderSupport = true;

  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Gets the currently-stored lender ID.
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[out]     lenderId   Pointer to the lender ID C-string; must be
///       NULL on entry. <em>Ownership is not transferred to the caller, so
///       the caller should not free this variable.</em>
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_getLenderId(const KivaModel* this, char** lenderId) {
  MPA_RETURN_IF_NULL(this);
  if (*lenderId != NULL) { return MPA_INVALID_INPUT_ERR; }

  *lenderId = this->lenderInfo.id;
  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Gets the currently-stored lender name.
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[out]     lenderName   Pointer to the lender name C-string; must
///       be NULL on entry. <em>Ownership is not transferred to the caller, so
///       the caller should not free this variable.</em>
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_getLenderName(const KivaModel* this, char** lenderName) {
  MPA_RETURN_IF_NULL(this);
  if (*lenderName != NULL) { return MPA_INVALID_INPUT_ERR; }

  *lenderName = this->lenderInfo.name;
  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Gets the currently-stored lender location.
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[out]     lenderLoc   Pointer to the lender location C-string;
///       must be NULL on entry. <em>Ownership is not transferred to the
///       caller, so the caller should not free this variable.</em>
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_getLenderLoc(const KivaModel* this, char** lenderLoc) {
  MPA_RETURN_IF_NULL(this);
  if (*lenderLoc != NULL) { return MPA_INVALID_INPUT_ERR; }

  *lenderLoc = this->lenderInfo.loc;
  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Gets the quantity of loans by the registered lender.
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[out]     lenderQty   Pointer to the lender loan quantity int
///       variable
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_getLenderLoanQty(const KivaModel* this, int* lenderLoanQty) {
  MPA_RETURN_IF_NULL(this);
  *lenderLoanQty = this->lenderInfo.loanQty;
  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Gets the quantity of countries actively served by Kiva.
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[out]     kivaCountryQty   Pointer to the Kiva country quantity
///       int variable
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_getKivaCountryQty(const KivaModel* this, int* kivaCountryQty) {
  MPA_RETURN_IF_NULL(this);

  int kivaCntryCount = 0;
  CountryRec* cntry = NULL;
  for (cntry=this->kivaCountries; cntry!=NULL; cntry=cntry->hh.next) {
    if (cntry->kivaActive) {
      APP_LOG(APP_LOG_LEVEL_INFO, "Active Kiva Country: %s", cntry->name);
      kivaCntryCount++;
    }
  }
  *kivaCountryQty = kivaCntryCount;

  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Gets the quantity of countries supported by the registered lender.
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[out]     lenderCountryQty   Pointer to the lender country quantity
///       int variable
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_getLenderCountryQty(const KivaModel* this, int* lenderCountryQty) {
  MPA_RETURN_IF_NULL(this);

  int lenderCntryCount = 0;
  CountryRec* cntry = NULL;
  for (cntry=this->kivaCountries; cntry!=NULL; cntry=cntry->hh.next) {
    if (cntry->lenderSupport) {
      //APP_LOG(APP_LOG_LEVEL_INFO, "Lender supports %s", cntry->name);
      lenderCntryCount++;
    }
  }
  *lenderCountryQty = lenderCntryCount;

  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Gets a list of country codes relative to the registered lender.
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[in]      support  If true, returns a list of country codes that
///       registered lender has supported with loans. If false, returns a
///       list of country codes that the registered lender has NOT yet
///       supported with loans.
/// @param[out]     countryCodes  If countries exist that meet the support
///       criteria relative to the lender in this KivaModel, then this
///       parameter will be replaced with a pointer to a string of comma-
///       separated country codes. Must be NULL upon entry to this function.
///       If there are no countries that meet the criteria, then this
///       parameter will remain NULL. <em>Ownership of this string is
///       transferred to the caller after this function call. Caller owns
///       (and must free) the memory associated with countryCodes
///       (if the string is not NULL).</em>
/// @return  MPA_SUCCESS on success
///          MPA_INVALID_INPUT_ERR if countryCodes is not NULL on entry
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_getLenderCountryCodes(const KivaModel* this, const bool support, char** countryCodes) {
  MPA_RETURN_IF_NULL(this);
  if (*countryCodes != NULL) { return MPA_INVALID_INPUT_ERR; }

  const char* sep = ",";

  char* lenderCntryCodes = NULL;
  size_t bufsize = 16;    ///< Arbitrary start size
  size_t buflen = 0;

  if ( (lenderCntryCodes = (char*) calloc(bufsize, sizeof(char))) == NULL) { goto freemem; }

  CountryRec* cntry = NULL;
  for (cntry=this->kivaCountries; cntry!=NULL; cntry=cntry->hh.next) {
    if (cntry->lenderSupport == support) {
      buflen = strxlen(lenderCntryCodes);
      size_t reqdSize = (buflen + strxlen(cntry->id) + strxlen(sep) + 1);

      if (bufsize < reqdSize) {
        while (bufsize < reqdSize) bufsize = bufsize * 2;
        char* tmp = NULL;
        if ( (tmp = realloc(lenderCntryCodes, bufsize)) == NULL) { goto freemem; }
        lenderCntryCodes = tmp;
      }
      lenderCntryCodes = strncat(lenderCntryCodes, cntry->id, bufsize - buflen - 1);
      buflen = strxlen(lenderCntryCodes);
      lenderCntryCodes = strncat(lenderCntryCodes, sep, bufsize - buflen - 1);
      buflen = strxlen(lenderCntryCodes);
    }
  }

  // Remove the trailing separator
  buflen = strxlen(lenderCntryCodes);
  if (lenderCntryCodes != NULL && (buflen > strxlen(sep))) {
    lenderCntryCodes[buflen - strxlen(sep)] = 0;
  }

  *countryCodes = lenderCntryCodes;
  return MPA_SUCCESS;

freemem:
  if (lenderCntryCodes != NULL) { free(lenderCntryCodes); }
  return MPA_OUT_OF_MEMORY_ERR;
}




/////////////////////////////////////////////////////////////////////////////
/// Clears the list of preferred loans, freeing all heap-allocated members
/// of LoanInfo data.
/// Preferred loans are a list of fundraising loans in which the lender has
/// indicated an interest.
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_clearPreferredLoans(KivaModel* this) {
  MPA_RETURN_IF_NULL(this);
  MagPebApp_ErrCode mpaRet;

  LoanRec* loanRec = NULL; LoanRec* tmpLoan = NULL;
  HASH_ITER(hh, this->prefLoans, loanRec, tmpLoan) {
    if ( (mpaRet = KivaModel_LoanRec_destroy(loanRec)) != MPA_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error trying to destroy a loan record: %s", MagPebApp_getErrMsg(mpaRet));
    }
    HASH_DEL(this->prefLoans, loanRec);
    loanRec = NULL;
  }

  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Adds a new loan to the list of preferred loans.
/// Preferred loans are a list of fundraising loans in which the lender has
/// indicated an interest.
/// The heap-allocated members of loanInfo are copied; <em>ownership of those
/// parameters is not transferred by this function. The caller is still
/// responsible for freeing that data.</em>
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[in]      loanInfo  information about the preferred loan
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_addPreferredLoan(KivaModel* this, const LoanInfo loanInfo) {
  MPA_RETURN_IF_NULL(this);
  MagPebApp_ErrCode mpaRet;

  LoanRec *newLoanRec = NULL;
  if ( (mpaRet = KivaModel_LoanRec_create(&newLoanRec)) != MPA_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not create new loan record (%ld): %s", loanInfo.id, MagPebApp_getErrMsg(mpaRet));
    return mpaRet;
  }
  if ( (mpaRet = KivaModel_LoanRec_init(newLoanRec, loanInfo)) != MPA_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not initialize new loan record (%ld): %s", loanInfo.id, MagPebApp_getErrMsg(mpaRet));
    KivaModel_LoanRec_destroy(newLoanRec);  newLoanRec = NULL;
    return mpaRet;
  }

  LoanRec *loanRec = NULL;
  HASH_FIND_INT(this->prefLoans, &loanInfo.id, loanRec);

  if (loanRec == NULL) {
    // Value of loanInfo.id is not already a key in the hash table; add new record.
    //HEAP_LOG("adding new record...");
    HASH_ADD_INT(this->prefLoans, data.id, newLoanRec);
  } else {
    // Value of loanInfo.id was already a key in the hash table; replace it.
    HASH_ADD_INT(this->prefLoans, data.id, newLoanRec);
    HASH_DEL(this->prefLoans, loanRec);
    if (loanRec != NULL) { KivaModel_LoanRec_destroy(loanRec);  loanRec = NULL; }
    else {
      APP_LOG(APP_LOG_LEVEL_WARNING, "This shouldn't happen. The LoanRec we just found (%ld) is no longer in the hash.", loanInfo.id);
    }
  }
  return MPA_SUCCESS;
}




/////////////////////////////////////////////////////////////////////////////
/// Adds a new country to the list of Kiva-recognized countries.
/// The countryId and countryName parameters are copied; <em>ownership of those
/// parameters is not transferred by this function. The caller is still
/// responsible for freeing that data.</em>
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[in]      countryId  ID of the country to add; a two-character
///       ISO-3361 country code
/// @param[in]      countryName  Name of the country to add
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_addKivaCountry(KivaModel* this, const char* countryId, const char* countryName) {
  MPA_RETURN_IF_NULL(this);
  MagPebApp_ErrCode mpaRet;

  CountryRec *newCntry = NULL;
  if ( (mpaRet = KivaModel_CountryRec_create(&newCntry)) != MPA_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not create new country record (%s): %s", countryId, MagPebApp_getErrMsg(mpaRet));
    return mpaRet;
  }
  if ( (mpaRet = KivaModel_CountryRec_init(newCntry, countryId, countryName)) != MPA_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not initialize new country record (%s): %s", countryId, MagPebApp_getErrMsg(mpaRet));
    KivaModel_CountryRec_destroy(newCntry);  newCntry = NULL;
    return mpaRet;
  }

  CountryRec *cntry = NULL;
  HASH_FIND_STR(this->kivaCountries, countryId, cntry);

  if (cntry == NULL) {
    // Value of countryId is not already a key in the hash table; add new record.
    //HEAP_LOG("adding new record...");
    HASH_ADD_KEYPTR(hh, this->kivaCountries, newCntry->id, strlen(newCntry->id), newCntry);
  } else {
    // Value of countryId was already a key in the hash table; replace it.
    HASH_ADD_KEYPTR(hh, this->kivaCountries, newCntry->id, strlen(newCntry->id), newCntry);
    HASH_DEL(this->kivaCountries, cntry);
    if (cntry != NULL) { KivaModel_CountryRec_destroy(cntry);  cntry = NULL; }
    else {
      APP_LOG(APP_LOG_LEVEL_WARNING, "This shouldn't happen. The CountryRec we just found (%s) is no longer in the hash.", countryId);
    }
  }
  // Mark as active in Kiva
  newCntry->kivaActive = true;
  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Provides a pointer to the country name identified by the specified ID.
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[in]      countryId  ID of the country to add; a two-character
///       ISO-3361 country code. <em>Ownership of this parameter is not
///       transferred, so the caller is still responsible for freeing this
///       variable.</em>
/// @param[out]     countryName  If the country ID is recognized by this
///       KivaModel, this parameter will be replaced with a pointer to the
///       recognized country name. Must be NULL upon entry to this function.
///       If the country ID is not recognized, then this parameter will
///       remain NULL.
///       <em>Caller does not own (should not free) the memory associated
///       with countryName.</em>
/// @return  MPA_SUCCESS on success
///          MPA_INVALID_INPUT_ERR if countryName is not NULL on entry
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode KivaModel_getKivaCountryName(const KivaModel* this, const char* countryId, const char** countryName) {
  MPA_RETURN_IF_NULL(this);
  if (*countryName != NULL) { return MPA_INVALID_INPUT_ERR; }

  CountryRec* cntry = NULL;

  HASH_FIND_STR(this->kivaCountries, countryId, cntry);
  if (cntry == NULL) {
    *countryName = NULL;
    return MPA_SUCCESS;
  }
  *countryName = cntry->name;
  return MPA_SUCCESS;
}


