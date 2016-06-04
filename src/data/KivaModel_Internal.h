#include <pebble.h>
#include "data/KivaModel.h"
#include "libs/uthash.h"


struct LenderInfo {
  char*    id;              ///< "3 to 24 characters long and consist of only letters and numbers."  https://groups.google.com/d/msg/build-kiva/9BTQKT40Wq0/S2GJrDVwDZsJ
  char*    name;            ///< The full name of the lender (user-entered and may not strictly be Firstname Lastname format)
  char*    loc;             ///< This is the general location string, like: "Lexington, KY"
  uint16_t loanQty;
};


typedef struct CountryRec {
  char* id;                 ///< the two-character ISO-3361 code
  char* name;               ///< the name of the country
  bool lenderSupport : 1;   ///< indicates if the lender has supported any loans in this country
  bool kivaActive : 1;      ///< indicates if Kiva actively serves loans in this country;
  UT_hash_handle hh;
} CountryRec;


typedef struct LoanRec {
  LoanInfo data;
  UT_hash_handle hh;
} LoanRec;




struct KivaModel {
  struct LenderInfo lenderInfo;
  CountryRec* kivaCountries;
  LoanRec* prefLoans;

  KivaModel_Modified* mods;
};


MagPebApp_ErrCode KivaModel_init(KivaModel* this, const char*);
