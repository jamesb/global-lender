#include <pebble.h>
#include "data/KivaModel_Internal.h"
#include "misc.h"

#define KIVA_MODEL_RETURN_IF_NULL(var)                                        \
    if (var == NULL) {                                                        \
      APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");   \
      return KIVA_MODEL_NULL_POINTER_ERR;                                     \
    }


/**************************************************************************
 * Returns error messages for data model error codes.
 **************************************************************************/
const char* KivaModel_getErrMsg(const KivaModel_ErrCode errCode) {
  switch (errCode) {
    case KIVA_MODEL_SUCCESS:           return "success";
    case KIVA_MODEL_UNKNOWN_ERR:       return "unknown error";
    case KIVA_MODEL_NULL_POINTER_ERR:  return "operation on null pointer";
    case KIVA_MODEL_STRING_ERR:        return "string operation error";
    case KIVA_MODEL_INVALID_INPUT_ERR: return "invalid input error";
    case KIVA_MODEL_OUT_OF_MEMORY_ERR: return "out of memory error";
    default:                           return "unlisted error";
  } // end switch
}




/////////////////////////////////////////////////////////////////////////////
/// Allocates a CountryRec pointer. 
/// @param[out]     cntry   double-pointer to CountryRec; must be NULL on entry
/////////////////////////////////////////////////////////////////////////////
static KivaModel_ErrCode KivaModel_CountryRec_create(CountryRec** cntry) {
  if (*cntry != NULL) { return KIVA_MODEL_INVALID_INPUT_ERR; }
  
  *cntry = (CountryRec*) malloc(sizeof(CountryRec));
  if (*cntry == NULL) { goto lowmem; }
  (*cntry)->id = NULL;
  (*cntry)->name = NULL;
    
  return KIVA_MODEL_SUCCESS;   
  
lowmem:
  if (*cntry != NULL) { free(*cntry); }
  return KIVA_MODEL_OUT_OF_MEMORY_ERR;
}

  
/////////////////////////////////////////////////////////////////////////////
/// Initializes a CountryRec pointer and its members. 
/// @param[in,out]  this  Pointer to CountryRec; must be already
///       allocated upon entry, but heap-allocated members must be NULL
/// @param[in]      countryId  ID of country (ISO-3316); must be non-null;
/// @param[in]      countryName  Name of country; must be non-null;
/////////////////////////////////////////////////////////////////////////////
static KivaModel_ErrCode KivaModel_CountryRec_init(CountryRec* this, const char* countryId, const char* countryName) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  if (this->id != NULL)   { 
    APP_LOG(APP_LOG_LEVEL_ERROR, "CountryRec id must be NULL.");
    return KIVA_MODEL_INVALID_INPUT_ERR; 
  }
  if (this->name != NULL) { 
    APP_LOG(APP_LOG_LEVEL_ERROR, "CountryRec name must be NULL.");
    return KIVA_MODEL_INVALID_INPUT_ERR; 
  }
  if (countryId == NULL)   { 
    APP_LOG(APP_LOG_LEVEL_ERROR, "Init parameter countryId must not be NULL.");
    return KIVA_MODEL_INVALID_INPUT_ERR; 
  }
  if (countryName == NULL) { 
    APP_LOG(APP_LOG_LEVEL_ERROR, "Init parameter countryName must not be NULL.");
    return KIVA_MODEL_INVALID_INPUT_ERR; 
  }
    
  if ( (this->id = malloc(strlen(countryId) + 1)) == NULL) { goto lowmem; }
  strcpy(this->id, countryId);
  
  if ( (this->name = malloc(strlen(countryName) + 1)) == NULL) { goto lowmem; }
  strcpy(this->name, countryName);
  
  return KIVA_MODEL_SUCCESS;   
  
lowmem:
  if (this->id != NULL)   { free(this->id);   this->id = NULL; }
  if (this->name != NULL) { free(this->name); this->name = NULL; }
  return KIVA_MODEL_OUT_OF_MEMORY_ERR;
}

  
/////////////////////////////////////////////////////////////////////////////
/// Frees all memory associated with a CountryRec pointer. 
/// @param[in,out]  this  Pointer to CountryRec; must be already allocated
/////////////////////////////////////////////////////////////////////////////
static KivaModel_ErrCode KivaModel_CountryRec_destroy(CountryRec* this) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  HEAP_LOG("Freeing CountryRec");
  if (this->id != NULL)   { free(this->id);   this->id = NULL; }
  if (this->name != NULL) { free(this->name); this->name = NULL; }
  free(this); this = NULL;
  return KIVA_MODEL_SUCCESS;   
}




/**************************************************************************
 * Constructor
 **************************************************************************/
KivaModel* KivaModel_create(const char* lenderId) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating KivaModel [%s]", lenderId);
  int kmret;
  
  KivaModel* newKivaModel = malloc(sizeof(KivaModel));
  if ( (kmret = KivaModel_init(newKivaModel, lenderId)) != KIVA_MODEL_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not initialize Kiva Model: %s", KivaModel_getErrMsg(kmret));
    KivaModel_destroy(newKivaModel);  newKivaModel = NULL;
  }
  
  return newKivaModel;
}


/**************************************************************************
 * Destroys KivaModel and frees allocated memory.
 **************************************************************************/
KivaModel_ErrCode KivaModel_destroy(KivaModel* this) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Destroying KivaModel [%s]", this->lenderInfo.id);
  KivaModel_ErrCode kmret;
  
  // Free memory for this->kivaCountries and its data
  CountryRec* cntry = NULL; CountryRec* tmp = NULL;
  HASH_ITER(hh, this->kivaCountries, cntry, tmp) {
    if ( (kmret = KivaModel_CountryRec_destroy(cntry)) != KIVA_MODEL_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Error trying to destroy a country record: %s", KivaModel_getErrMsg(kmret));
    }
    HASH_DEL(this->kivaCountries, cntry);
    cntry = NULL;
  }
  HEAP_LOG("Freed kivaCountries and its data.");
  
  if (this->lenderInfo.loc != NULL)  { free(this->lenderInfo.loc);  this->lenderInfo.loc = NULL; }
  if (this->lenderInfo.name != NULL) { free(this->lenderInfo.name); this->lenderInfo.name = NULL; }
  if (this->lenderInfo.id != NULL)   { free(this->lenderInfo.id);   this->lenderInfo.id = NULL; }
  
  HEAP_LOG("Freed lenderInfo and its data.");
  
  free(this); this = NULL;
  return KIVA_MODEL_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Internal initialization
/////////////////////////////////////////////////////////////////////////////
KivaModel_ErrCode KivaModel_init(KivaModel* this, const char* lenderId) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Initializing KivaModel [%s]", lenderId);
  KivaModel_ErrCode kmret;
  
  this->lenderInfo.id = NULL;
  this->lenderInfo.name = NULL;
  this->lenderInfo.loc = NULL;
  this->lenderInfo.loanQty = 0;

  this->kivaCountries = NULL;
  
  if ( (kmret = KivaModel_initKivaCountries(this)) != KIVA_MODEL_SUCCESS) { 
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not initialize Kiva countries: %s", KivaModel_getErrMsg(kmret));
    return kmret; 
  }
  
  if ( (kmret = KivaModel_setLenderId(this, lenderId)) != KIVA_MODEL_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not set Lender ID. Destroying Kiva model.");
    KivaModel_destroy(this);  this = NULL;
  }
  
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Kiva Model [%s] initialized with %d countries.", this->lenderInfo.id, HASH_COUNT(this->kivaCountries));
  return KIVA_MODEL_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Country hash table initialization
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated;
///       this->kivaCountries must be NULL
/////////////////////////////////////////////////////////////////////////////
KivaModel_ErrCode KivaModel_initKivaCountries(KivaModel* this) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  if (this->kivaCountries != NULL) { return KIVA_MODEL_INVALID_INPUT_ERR; }
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Initializing Kiva countries...");
  KivaModel_ErrCode kmret; 
  if ( (kmret = KivaModel_addKivaCountry(this, "AF", "Afghanistan")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "AL", "Albania")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "AM", "Armenia")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "AZ", "Azerbaijan")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "BA", "Bosnia and Herzegovina")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "BF", "Burkina Faso")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "BG", "Bulgaria")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "BI", "Burundi")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "BJ", "Benin")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "BO", "Bolivia")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "BR", "Brazil")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "BW", "Botswana")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "BZ", "Belize")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "CD", "Congo (Dem. Rep.)")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "CG", "Congo (Rep.)")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "CI", "Cote D'Ivoire")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "CL", "Chile")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "CM", "Cameroon")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "CN", "China")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "CO", "Colombia")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "CR", "Costa Rica")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "DO", "Dominican Republic")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "EC", "Ecuador")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "EG", "Egypt")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "GE", "Georgia")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "GH", "Ghana")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "GT", "Guatemala")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "GZ", "Gaza")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "HN", "Honduras")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "HT", "Haiti")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "ID", "Indonesia")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "IL", "Israel")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "IN", "India")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "IQ", "Iraq")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "JO", "Jordan")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "KE", "Kenya")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "KG", "Kyrgyzstan")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "KH", "Cambodia")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "LA", "Lao PDR")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "LB", "Lebanon")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "LK", "Sri Lanka")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "LR", "Liberia")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "MD", "Moldova")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "MG", "Madagascar")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "ML", "Mali")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "MM", "Myanmar (Burma)")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "MN", "Mongolia")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "MR", "Mauritania")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "MW", "Malawi")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "MX", "Mexico")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "MZ", "Mozambique")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "NA", "Namibia")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "NG", "Nigeria")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "NI", "Nicaragua")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "NP", "Nepal")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "PA", "Panama")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "PE", "Peru")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "PG", "Papua New Guinea")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "PH", "Philippines")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "PK", "Pakistan")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "PS", "Palestine")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "PY", "Paraguay")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "QS", "South Sudan")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "RW", "Rwanda")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "SB", "Solomon Islands")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "SG", "Singapore")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "SL", "Sierra Leone")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "SN", "Senegal")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "SO", "Somalia")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "SR", "Suriname")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "SV", "El Salvador")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "TD", "Chad")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "TG", "Togo")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "TH", "Thailand")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "TJ", "Tajikistan")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "TL", "Timor-Leste")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "TN", "Tunisia")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "TR", "Turkey")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "TZ", "Tanzania")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "UA", "Ukraine")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "UG", "Uganda")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "US", "United States")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "VC", "St Vincent")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "VN", "Vietnam")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "VU", "Vanuatu")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "WS", "Samoa")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "XK", "Kosovo")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "YE", "Yemen")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "ZA", "South Africa")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "ZM", "Zambia")) != KIVA_MODEL_SUCCESS) { return kmret; }
  if ( (kmret = KivaModel_addKivaCountry(this, "ZW", "Zimbabwe")) != KIVA_MODEL_SUCCESS) { return kmret; }
  return KIVA_MODEL_SUCCESS;
}


/**************************************************************************
 * Sets the Lender ID and resets any lender-specific data so that it
 * will be NULL or empty until it is reloaded.
 **************************************************************************/
KivaModel_ErrCode KivaModel_setLenderId(KivaModel* this, const char* lenderId) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  
  if (lenderId == NULL) { return KIVA_MODEL_INVALID_INPUT_ERR; }
  if (this->lenderInfo.id != NULL) { free(this->lenderInfo.id); this->lenderInfo.id = NULL; }
  size_t bufsize = strlen(lenderId)+1;
  char* tmp = realloc(this->lenderInfo.id, bufsize);
  if (tmp == NULL) { return KIVA_MODEL_OUT_OF_MEMORY_ERR; }
  this->lenderInfo.id = tmp;
  
  if (!strxcpy(this->lenderInfo.id, bufsize, lenderId, "Lender ID")) { return KIVA_MODEL_STRING_ERR; }
  // Clear out any lender-dependent fields since the lender ID has changed.
  if (this->lenderInfo.name != NULL) { free(this->lenderInfo.name); this->lenderInfo.name = NULL; }
  if (this->lenderInfo.loc != NULL)  { free(this->lenderInfo.loc);  this->lenderInfo.loc = NULL; }
  this->lenderInfo.loanQty = 0;
  // JRB TODO: reset the CountryRec lender-supported fields to NULL
  
  return KIVA_MODEL_SUCCESS;
}


/**************************************************************************
 * 
 **************************************************************************/
KivaModel_ErrCode KivaModel_setLenderName(KivaModel* this, const char* lenderName) {  
  KIVA_MODEL_RETURN_IF_NULL(this);
  
  if (lenderName == NULL) { return KIVA_MODEL_INVALID_INPUT_ERR; }
  if (this->lenderInfo.name != NULL) { free(this->lenderInfo.name); this->lenderInfo.name = NULL; }
  size_t bufsize = strlen(lenderName) + 1;
  char* tmp = realloc(this->lenderInfo.name, bufsize);
  if (tmp == NULL) { return KIVA_MODEL_OUT_OF_MEMORY_ERR; }
  this->lenderInfo.name = tmp;
  
  if (!strxcpy(this->lenderInfo.name, bufsize, lenderName, "Lender Name")) {
    return KIVA_MODEL_STRING_ERR;
  }
  return KIVA_MODEL_SUCCESS;
}


/**************************************************************************
 * 
 **************************************************************************/
KivaModel_ErrCode KivaModel_setLenderLoc(KivaModel* this, const char* lenderLoc) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  
  if (lenderLoc == NULL) { return KIVA_MODEL_INVALID_INPUT_ERR; }
  if (this->lenderInfo.loc != NULL) { free(this->lenderInfo.loc); this->lenderInfo.loc = NULL; }
  size_t bufsize = strlen(lenderLoc) + 1;
  char* tmp = realloc(this->lenderInfo.loc, bufsize);
  if (tmp == NULL) { return KIVA_MODEL_OUT_OF_MEMORY_ERR; }
  this->lenderInfo.loc = tmp;
  
  if (!strxcpy(this->lenderInfo.loc, bufsize, lenderLoc, "Lender Loc")) {
    return KIVA_MODEL_STRING_ERR;
  }
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
KivaModel_ErrCode KivaModel_addLenderCountryId(KivaModel* this, const char* countryId) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  // JRB TODO: do stuff 
  return KIVA_MODEL_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Gets the currently-stored lender ID. 
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[out]     lenderId   Pointer to the lender ID C-string; must be 
///       NULL on entry. Ownership is not transferred to the caller, so
///       the caller should not free this variable.
/////////////////////////////////////////////////////////////////////////////
KivaModel_ErrCode KivaModel_getLenderId(const KivaModel* this, char** lenderId) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  if (*lenderId != NULL) { return KIVA_MODEL_INVALID_INPUT_ERR; }
  
  *lenderId = this->lenderInfo.id;
  return KIVA_MODEL_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Gets the currently-stored lender name. 
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[out]     lenderName   Pointer to the lender name C-string; must 
///       be NULL on entry. Ownership is not transferred to the caller, so
///       the caller should not free this variable.
/////////////////////////////////////////////////////////////////////////////
KivaModel_ErrCode KivaModel_getLenderName(const KivaModel* this, char** lenderName) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  if (*lenderName != NULL) { return KIVA_MODEL_INVALID_INPUT_ERR; }
  
  *lenderName = this->lenderInfo.name;
  return KIVA_MODEL_SUCCESS;
} 


/////////////////////////////////////////////////////////////////////////////
/// Gets the currently-stored lender location. 
/// @param[in,out]  this  Pointer to KivaModel; must be already allocated
/// @param[out]     lenderLoc   Pointer to the lender location C-string; 
///       must be NULL on entry. Ownership is not transferred to the 
///       caller, so the caller should not free this variable.
/////////////////////////////////////////////////////////////////////////////
KivaModel_ErrCode KivaModel_getLenderLoc(const KivaModel* this, char** lenderLoc) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  if (*lenderLoc != NULL) { return KIVA_MODEL_INVALID_INPUT_ERR; }
  
  *lenderLoc = this->lenderInfo.loc;
  return KIVA_MODEL_SUCCESS;
}


/**************************************************************************
 * 
 **************************************************************************/
KivaModel_ErrCode KivaModel_getLenderLoanQty(const KivaModel* this, int* lenderLoanQty) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  *lenderLoanQty = this->lenderInfo.loanQty;
  return KIVA_MODEL_SUCCESS;
}


/**************************************************************************
 * 
 **************************************************************************/
KivaModel_ErrCode KivaModel_getLenderCountryQty(const KivaModel* this, int* lenderCountryQty) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  // JRB TODO: do stuff
  return KIVA_MODEL_SUCCESS;
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
KivaModel_ErrCode KivaModel_addKivaCountry(KivaModel* this, const char* countryId, const char* countryName) {
  KIVA_MODEL_RETURN_IF_NULL(this);
  KivaModel_ErrCode kmret;
  
  CountryRec *newCntry = NULL;
  if ( (kmret = KivaModel_CountryRec_create(&newCntry)) != KIVA_MODEL_SUCCESS) { 
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not create new country record (%s): %s", countryId, KivaModel_getErrMsg(kmret));
    return kmret; 
  }
  if ( (kmret = KivaModel_CountryRec_init(newCntry, countryId, countryName)) != KIVA_MODEL_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not initialize new country record (%s): %s", countryId, KivaModel_getErrMsg(kmret));
    KivaModel_CountryRec_destroy(newCntry);  newCntry = NULL;
    return kmret;
  }
  
  CountryRec *cntry = NULL;
  HASH_FIND_STR(this->kivaCountries, countryId, cntry);
  
  if (cntry == NULL) {
    // Value of countryId is not already a key in the hash table; add new record.
    HEAP_LOG("adding new record...");
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
  return KIVA_MODEL_SUCCESS;
}


/**************************************************************************
 * Returns a pointer to the country name identified by the specified ID.
 * Caller does not own (should not free) countryName.
 **************************************************************************/
KivaModel_ErrCode KivaModel_getKivaCountryName(const KivaModel* this, const char* countryId, const char** countryName) {
  CountryRec* cntry = NULL;
  
  HASH_FIND_STR(this->kivaCountries, countryId, cntry);
  if (cntry == NULL) {
    *countryName = NULL;
    return KIVA_MODEL_INVALID_INPUT_ERR;
  }
  *countryName = cntry->name;
  return KIVA_MODEL_SUCCESS;
}

