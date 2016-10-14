#include <pebble.h>

// Deactivate APP_LOG in this file.
//#undef APP_LOG
//#define APP_LOG(...)

#include "../misc.h"
#include "../data/KivaModel.h"
#include "../libs/WndDataMenu.h"

#include "wndLenderBasics.h"
#include "wndMainMenu.h"

#define NUM_MENU_SECTIONS 1

enum MenuItems {
  MNU_ITEM_LENDER_INFO = 0,
  MNU_ITEM_LOANS_FOR_YOU,
#if 0
  MNU_ITEM_ACHIEVEMENTS,
  MNU_ITEM_TEAMS,
  MNU_ITEM_COUNTRIES,
  MNU_ITEM_SECTORS,
  MNU_ITEM_ACTIVITIES,
  MNU_ITEM_FIELD_PARTNERS,
  MNU_ITEM_ACKNOWLEDGEMENTS,
#endif

  NUM_MENU_ITEMS
};


#define LOAN_MSG_SZ 32

static Window* wndMainMenu;
static MenuLayer* lyrMainMenu;
static GBitmap* bmpLogo;
static const KivaModel* kivaModel;
static wndMainMenuHandlers myHandlers;
static char loanMsg[LOAN_MSG_SZ];

static WndDataMenu* wndCountries;
static WndDataMenu* wndPrefLoans;


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static uint16_t wndMainMenu_get_num_sections_callback(MenuLayer* menu_layer, void* data) {
  return NUM_MENU_SECTIONS;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static uint16_t wndMainMenu_get_num_rows_callback(MenuLayer* menu_layer, uint16_t section_index, void* data) {
  switch (section_index) {
    case 0:
      return NUM_MENU_ITEMS;
    default:
      return 0;
  }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static void wndMainMenu_draw_row_callback(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* data) {
  GSize size = layer_get_frame(cell_layer).size;
  GRect drawRect = GRect(0, 0, size.w, size.h);

  switch (cell_index->section) {
    case 0: {
      switch (cell_index->row) {

        case MNU_ITEM_LENDER_INFO:
#if defined(PBL_ROUND)
          if (menu_cell_layer_is_highlighted(cell_layer)) {
            graphics_draw_bitmap_in_rect(ctx, bmpLogo, drawRect);
          } else {
            menu_cell_basic_draw(ctx, cell_layer, "Global Lender", NULL, NULL);
          }
#else
          graphics_draw_bitmap_in_rect(ctx, bmpLogo, drawRect);
#endif

          if (menu_cell_layer_is_highlighted(cell_layer)) {
            graphics_context_set_stroke_color(ctx, COLOR_FALLBACK(GColorPictonBlue, GColorWhite));
            graphics_context_set_stroke_width(ctx, 4);
            graphics_context_set_antialiased(ctx, true);
            graphics_draw_round_rect(ctx, drawRect, 3);
          }

          break;

        case MNU_ITEM_LOANS_FOR_YOU:
          menu_cell_basic_draw(ctx, cell_layer, loanMsg, NULL, NULL);
          break;
#if 0
        case MNU_ITEM_ACHIEVEMENTS:
          menu_cell_basic_draw(ctx, cell_layer, "Achievements", NULL, NULL);
          break;
        case MNU_ITEM_TEAMS:
          menu_cell_basic_draw(ctx, cell_layer, "Teams", NULL, NULL);
          break;
        case MNU_ITEM_COUNTRIES:
          menu_cell_basic_draw(ctx, cell_layer, "Countries", NULL, NULL);
          break;
        case MNU_ITEM_SECTORS:
          menu_cell_basic_draw(ctx, cell_layer, "Sectors", NULL, NULL);
          break;
        case MNU_ITEM_ACTIVITIES:
          menu_cell_basic_draw(ctx, cell_layer, "Activities", NULL, NULL);
          break;
        case MNU_ITEM_FIELD_PARTNERS:
          menu_cell_basic_draw(ctx, cell_layer, "Field Partners", NULL, NULL);
          break;
        case MNU_ITEM_ACKNOWLEDGEMENTS:
          menu_cell_basic_draw(ctx, cell_layer, "Acknowledgements", NULL, NULL);
          break;
#endif
      }
      break;
    }
  }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static void wndMainMenu_select_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data) {
  MagPebApp_ErrCode mpaRet = MPA_SUCCESS;
  uint16_t prefLoanQty = 0;

  switch (cell_index->row) {
    case MNU_ITEM_LENDER_INFO: {
      if (!myHandlers.getLenderInfo) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");
      } else {
        (*myHandlers.getLenderInfo)();
      }

      wndLenderBasics_push();
      wndLenderBasics_updateView(kivaModel);
      break;
    }
    case MNU_ITEM_LOANS_FOR_YOU: {
      if (!myHandlers.getPrefLoans) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");
      } else {
        (*myHandlers.getPrefLoans)();
      }

      if ( (mpaRet = KivaModel_getPreferredLoanQty(kivaModel, &prefLoanQty) ) != MPA_SUCCESS) {
          APP_LOG(APP_LOG_LEVEL_ERROR, "Error getting number of preferred loans: %s", MagPebApp_getErrMsg(mpaRet));
      }
      if (prefLoanQty > 0) {
        WndDataMenu_push(wndPrefLoans);
      }

      break;
    }
#if 0
    case MNU_ITEM_COUNTRIES: {
      WndDataMenu_push(wndCountries);
      break;
    }
    case MNU_ITEM_ACKNOWLEDGEMENTS: {
      // Matthew Tole - Data-Processor (MIT License)
      // Troy D. Hanson - uthash (BSD License)
      // Blake Swopes - Kiva Country Collector
      // Helpful Pebble Forums posts by:
      //   @LeFauve, @allan, @Christian Reinbacher

      break;
    }
#endif
  }
}


#if defined(PBL_ROUND)
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int16_t wndMainMenu_get_cell_height_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* callback_context) {
  Layer* lyrRoot = window_get_root_layer(wndMainMenu);
  GRect bounds = layer_get_bounds(lyrRoot);
  return (int)(bounds.size.h / 3);
}
#endif


/////////////////////////////////////////////////////////////////////////////
/// Set our callback handlers.
/////////////////////////////////////////////////////////////////////////////
void wndMainMenu_setHandlers(const wndMainMenuHandlers wmmh) {
  myHandlers = wmmh;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void wndMainMenu_updateClock(struct tm* in_time) {
  struct tm* curr_time = NULL;

  if (in_time != NULL) {
    curr_time = in_time;
  } else {
    // Get a tm structure
    time_t temp = time(NULL);
    curr_time = localtime(&temp);
  }

  wndLenderBasics_updateTime(curr_time);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void wndMainMenu_updateData(const KivaModel* km) {
  kivaModel = km;
  MagPebApp_ErrCode mpaRet = MPA_SUCCESS;
  KivaModel_Modified mods;
  uint16_t prefLoanQty = 0;

  if ( (mpaRet = KivaModel_getMods(kivaModel, &mods) ) != MPA_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "MPA error: %s", MagPebApp_getErrMsg(mpaRet));
  }

  if (mods.preferredLoanQty) {
    if ( (mpaRet = KivaModel_getPreferredLoanQty(kivaModel, &prefLoanQty) ) != MPA_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_ERROR, "Error getting number of preferred loans: %s", MagPebApp_getErrMsg(mpaRet));
    }
    if (prefLoanQty > 0) {
      if (!strxcpy(loanMsg, LOAN_MSG_SZ, "Loans for You!", NULL)) { return; }
    } else {
      if (!strxcpy(loanMsg, LOAN_MSG_SZ, "No Loans Found", NULL)) { return; }
    }
  }

  menu_layer_reload_data(lyrMainMenu);
  MenuIndex sel = menu_layer_get_selected_index(lyrMainMenu);

  switch (sel.row) {
    case MNU_ITEM_LOANS_FOR_YOU: {
      uint16_t prefLoanQty = 0;
      if ( (mpaRet = KivaModel_getPreferredLoanQty(kivaModel, &prefLoanQty) ) != MPA_SUCCESS) {
          APP_LOG(APP_LOG_LEVEL_ERROR, "Error getting number of preferred loans: %s", MagPebApp_getErrMsg(mpaRet));
      }
      if ( (mpaRet = WndDataMenu_buildSection(wndPrefLoans, 0, prefLoanQty, "Loans for You") ) != MPA_SUCCESS) {
          APP_LOG(APP_LOG_LEVEL_ERROR, "Could not build section: %s", MagPebApp_getErrMsg(mpaRet));
      }

      for (KivaModel_PrefLoan_CIter* plIter = KivaModel_firstPrefLoan(kivaModel);
           plIter != NULL; plIter = KivaModel_nextPrefLoan(kivaModel, plIter)) {
        long lret = 0;
        const uint8_t bufsize = 30;
        char buffer[bufsize];
        if ( (lret = snprintf(buffer, bufsize, "$%d of $%d funded", plIter->data->fundedAmt, plIter->data->loanAmt)) < 0) {
          APP_LOG(APP_LOG_LEVEL_ERROR, "Error in sprintf. Ret=%ld", lret);
        } else if ((size_t)lret >= bufsize) {
          APP_LOG(APP_LOG_LEVEL_WARNING, "String truncated by sprintf. %ld characters required.", lret);
        }
        if ( (mpaRet = WndDataMenu_buildRow(wndPrefLoans, 0, plIter->idx, plIter->data->name, buffer) ) != MPA_SUCCESS) {
            APP_LOG(APP_LOG_LEVEL_ERROR, "Could not build row: %s", MagPebApp_getErrMsg(mpaRet));
        }
      }
      WndDataMenu_updateView(wndPrefLoans);
      break;
    }
    case MNU_ITEM_LENDER_INFO: {
      wndLenderBasics_updateView(kivaModel);
      break;
    }
#if 0
    case MNU_ITEM_COUNTRIES: {
      WndDataMenu_updateView(wndCountries);
    }
#endif
  }

}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static void wndMainMenu_load(Window* window) {
  window_set_background_color(window, COLOR_FALLBACK(GColorDarkGreen, GColorBlack));
  Layer* lyrRoot = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(lyrRoot);
  MagPebApp_ErrCode mpaRet = MPA_SUCCESS;

  if (lyrMainMenu) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to re-create layer before destroying.");
    return;
  }

  if (!strxcpy(loanMsg, LOAN_MSG_SZ, "Please wait...", NULL)) { return; }

  lyrMainMenu = menu_layer_create(bounds);
  menu_layer_set_callbacks(lyrMainMenu, NULL, (MenuLayerCallbacks) {
    .get_num_sections = wndMainMenu_get_num_sections_callback,
    .get_num_rows = wndMainMenu_get_num_rows_callback,
    .draw_row = wndMainMenu_draw_row_callback,
    .select_click = wndMainMenu_select_callback,
    .get_cell_height = PBL_IF_ROUND_ELSE(wndMainMenu_get_cell_height_callback, NULL)
  });

  WndDataMenu_Palette colors = (WndDataMenu_Palette) {
    .normalBack = GColorBlack,
    .normalFore = GColorWhite,
    .highltBack = COLOR_FALLBACK(GColorPictonBlue, GColorWhite),
    .highltFore = GColorBlack
  };

  menu_layer_set_normal_colors(lyrMainMenu, colors.normalBack, colors.normalFore);
  menu_layer_set_highlight_colors(lyrMainMenu, colors.highltBack, colors.highltFore);
  menu_layer_set_click_config_onto_window(lyrMainMenu, window);
  layer_add_child(lyrRoot, menu_layer_get_layer(lyrMainMenu));

  bmpLogo = gbitmap_create_with_resource(RESOURCE_ID_IMG_LOGO_WIDE);
  HEAP_LOG("after bmpLogo created");

  wndLenderBasics_create();

  wndCountries = WndDataMenu_create();
  if ( (mpaRet = WndDataMenu_setNumSections(wndCountries, 1) ) != MPA_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Could not set colors: %s", MagPebApp_getErrMsg(mpaRet));
  }
  if ( (mpaRet = WndDataMenu_setPalette(wndCountries, colors) ) != MPA_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_WARNING, "Could not set colors: %s", MagPebApp_getErrMsg(mpaRet));
  }

  wndPrefLoans = WndDataMenu_create();
  if ( (mpaRet = WndDataMenu_setNumSections(wndPrefLoans, 1) ) != MPA_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Could not initialize sections: %s", MagPebApp_getErrMsg(mpaRet));
  }
  if ( (mpaRet = WndDataMenu_setPalette(wndPrefLoans, colors) ) != MPA_SUCCESS) {
      APP_LOG(APP_LOG_LEVEL_WARNING, "Could not set colors: %s", MagPebApp_getErrMsg(mpaRet));
  }

  wndMainMenu_updateClock(NULL);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static void wndMainMenu_unload(Window* window) {
  WndDataMenu_destroy(wndPrefLoans);
  WndDataMenu_destroy(wndCountries);
  wndLenderBasics_destroy();
  gbitmap_destroy(bmpLogo);   bmpLogo = NULL;

  if (!lyrMainMenu) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to destroy a null pointer!");
  } else {
    menu_layer_destroy(lyrMainMenu);   lyrMainMenu = NULL;
  }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void wndMainMenu_createPush() {
  if (wndMainMenu) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to re-create window before destroying.");
  } else {
    wndMainMenu = window_create();

    // Set handlers to manage the elements inside the Window
    window_set_window_handlers(wndMainMenu, (WindowHandlers) {
      .load =   wndMainMenu_load,
      .unload = wndMainMenu_unload
    });
  }

  // Show the Window on the watch, with animated=true
  window_stack_push(wndMainMenu, true);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
void wndMainMenu_destroy() {
  if (!wndMainMenu) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to destroy a null pointer!");
  } else {
    window_destroy(wndMainMenu);   wndMainMenu = NULL;
  }
}

