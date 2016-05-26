#pragma once
#include <pebble.h>
#include "libs/magpebapp.h"


typedef struct WndDataMenu_Palette {
  GColor normalBack;
  GColor normalFore;
  GColor highltBack;
  GColor highltFore;
} WndDataMenu_Palette;


typedef struct WndDataMenu WndDataMenu;




MagPebApp_ErrCode WndDataMenu_updateView(WndDataMenu* this);
MagPebApp_ErrCode WndDataMenu_setNumSections(WndDataMenu* this, const uint16_t);
MagPebApp_ErrCode WndDataMenu_setSectionTitle(WndDataMenu* this, const uint16_t, const char*);
MagPebApp_ErrCode WndDataMenu_buildSection(WndDataMenu* this, const uint16_t, const uint16_t, const char*);
MagPebApp_ErrCode WndDataMenu_buildRow(WndDataMenu* this, const uint16_t, const uint16_t, const char*, const char*);
MagPebApp_ErrCode WndDataMenu_setPalette(WndDataMenu* this, const WndDataMenu_Palette);

MagPebApp_ErrCode WndDataMenu_push(WndDataMenu* this);
WndDataMenu* WndDataMenu_create();
MagPebApp_ErrCode WndDataMenu_destroy(WndDataMenu* this);
