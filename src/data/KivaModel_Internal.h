#include "data/KivaModel.h"

struct LenderInfo {
  KivaModel_LenderId id;
  KivaModel_LenderName name;
  KivaModel_LenderLoc loc;
  int loanQty;
  int countryQty;
};


struct KivaModel {
  int countryQty;
  
  struct LenderInfo lenderInfo;
};

KivaModel_ErrCode KivaModel_init(KivaModel* this, const KivaModel_LenderId);