#include <pebble.h>
#include "WndDataMenu.h"


typedef struct SectionInfo {
  char*     headerTitle;
  uint16_t  numRows;
  char**    rowTitles;
  char**    rowSubtitles;
  // JRB TODO: select-clicked handler; per row? per section? one for all?
} SectionInfo;


struct WndDataMenu {
  Window* window;
  MenuLayer*           lyrDataMenu;
  WndDataMenu_Palette  palette;
  uint16_t             numSections;
  SectionInfo**        sections;
};


MagPebApp_ErrCode WndDataMenu_init(WndDataMenu*);


