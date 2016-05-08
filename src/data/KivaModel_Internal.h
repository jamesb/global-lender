#include <pebble.h>
#include "data/KivaModel.h"
#include "libs/uthash.h"


struct LenderInfo {
  char* id;              ///< "3 to 24 characters long and consist of only letters and numbers."  https://groups.google.com/d/msg/build-kiva/9BTQKT40Wq0/S2GJrDVwDZsJ
  char* name;            ///< The full name of the lender (user-entered and may not strictly be Firstname Lastname format)
  char* loc;             ///< This is the general location string, like: "Lexington, KY"
  int loanQty;
};


typedef struct CountryRec { 
  char* id;              ///< the two-character ISO-3361 code
  char* name; 
  bool lenderSupports;
  UT_hash_handle hh;
} CountryRec;


struct KivaModel {
  struct LenderInfo lenderInfo;
  CountryRec* kivaCountries;
};


KivaModel_ErrCode KivaModel_init(KivaModel* this, const char*);

KivaModel_ErrCode KivaModel_initKivaCountries(KivaModel* this);

