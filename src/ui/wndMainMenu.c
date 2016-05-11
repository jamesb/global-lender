#include <pebble.h>

// Deactivate APP_LOG in this file.
#undef APP_LOG
#define APP_LOG(...)

#include "misc.h"
#include "data/KivaModel.h"
#include "ui/wndLenderBasics.h"
#include "ui/wndCountries.h"
#include "ui/wndMainMenu.h"

#define NUM_MENU_SECTIONS 1

enum MenuItems {
  MNU_ITEM_LENDER_INFO = 0,
  MNU_ITEM_LOANS_FOR_YOU,
  MNU_ITEM_ACHIEVEMENTS,
  MNU_ITEM_TEAMS,
  MNU_ITEM_COUNTRIES,
  MNU_ITEM_SECTORS,
  MNU_ITEM_ACTIVITIES,
  MNU_ITEM_FIELD_PARTNERS,
  MNU_ITEM_ACKNOWLEDGEMENTS,

  NUM_MENU_ITEMS
};


static Window* wndMainMenu;
static MenuLayer* lyrMainMenu;
static GBitmap* bmpLogo;
static const KivaModel* kivaModel;


/**************************************************************************
 *
 **************************************************************************/
static uint16_t wndMainMenu_get_num_sections_callback(MenuLayer* menu_layer, void* data) {
  return NUM_MENU_SECTIONS;
}


/**************************************************************************
 *
 **************************************************************************/
static uint16_t wndMainMenu_get_num_rows_callback(MenuLayer* menu_layer, uint16_t section_index, void* data) {
  switch (section_index) {
    case 0:
      return NUM_MENU_ITEMS;
    default:
      return 0;
  }
}


/**************************************************************************
 *
 **************************************************************************/
static void wndMainMenu_draw_row_callback(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* data) {
  GSize size = layer_get_frame(cell_layer).size;
  GRect drawRect = GRect(0, 0, size.w, size.h);

  switch (cell_index->section) {
    case 0: {
      switch (cell_index->row) {
        case MNU_ITEM_LENDER_INFO:
          if (menu_cell_layer_is_highlighted(cell_layer)) {
            graphics_draw_bitmap_in_rect(ctx, bmpLogo, drawRect);
            graphics_context_set_stroke_color(ctx, COLOR_FALLBACK(GColorPictonBlue, GColorWhite));
            graphics_context_set_stroke_width(ctx, 6);
            graphics_context_set_antialiased(ctx, true);
            graphics_draw_round_rect(ctx, drawRect, 3);
          } else {
            menu_cell_basic_draw(ctx, cell_layer, "Global Lender", NULL, NULL);
          }
          break;
        case MNU_ITEM_LOANS_FOR_YOU:
          menu_cell_basic_draw(ctx, cell_layer, "Loans for You", NULL, NULL);
          break;
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
      }
      break;
    }
  }
}


/**************************************************************************
 *
 **************************************************************************/
static void wndMainMenu_select_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* data) {
  // Use the row to specify which item will receive the select action
  switch (cell_index->row) {
    case MNU_ITEM_LENDER_INFO: {
      wndLenderBasics_push();
      wndLenderBasics_updateView(kivaModel);
      break;
    }
    case MNU_ITEM_COUNTRIES: {
      wndCountries_push();
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
  }
}


#if defined(PBL_ROUND)
/**************************************************************************
 *
 **************************************************************************/
static int16_t wndMainMenu_get_cell_height_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* callback_context) {
  Layer* lyrRoot = window_get_root_layer(wndMainMenu);
  GRect bounds = layer_get_bounds(lyrRoot);
  return (int)(bounds.size.h / 3);
}
#endif


/**************************************************************************
 * Updates the displayed Kiva information.
 **************************************************************************/
void wndMainMenu_updateView(const KivaModel* km) {
  kivaModel = km;
  wndLenderBasics_updateView(kivaModel);
  wndCountries_updateView(kivaModel);
}


/**************************************************************************
 *
 **************************************************************************/
static void wndMainMenu_load(Window* window) {
  window_set_background_color(window, COLOR_FALLBACK(GColorDarkGreen, GColorBlack));
  Layer* lyrRoot = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(lyrRoot);

  if (lyrMainMenu) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to re-create layer before destroying.");
  } else {
    lyrMainMenu = menu_layer_create(bounds);
    menu_layer_set_callbacks(lyrMainMenu, NULL, (MenuLayerCallbacks) {
      .get_num_sections = wndMainMenu_get_num_sections_callback,
      .get_num_rows = wndMainMenu_get_num_rows_callback,
      .draw_row = wndMainMenu_draw_row_callback,
      .select_click = wndMainMenu_select_callback,
      .get_cell_height = PBL_IF_ROUND_ELSE(wndMainMenu_get_cell_height_callback, NULL)
    });

    menu_layer_set_normal_colors(lyrMainMenu, GColorBlack, GColorWhite);
    menu_layer_set_highlight_colors(lyrMainMenu, COLOR_FALLBACK(GColorPictonBlue, GColorWhite), GColorBlack);
    menu_layer_set_click_config_onto_window(lyrMainMenu, window);
    layer_add_child(lyrRoot, menu_layer_get_layer(lyrMainMenu));

    bmpLogo = gbitmap_create_with_resource(RESOURCE_ID_IMG_LOGO_WIDE);
    HEAP_LOG("after bmpLogo created");

    wndLenderBasics_create();
    wndCountries_create();
  }
}


/**************************************************************************
 *
 **************************************************************************/
static void wndMainMenu_unload(Window* window) {
  wndCountries_destroy();
  wndLenderBasics_destroy();
  gbitmap_destroy(bmpLogo);   bmpLogo = NULL;

  if (!lyrMainMenu) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to destroy a null pointer!");
  } else {
    menu_layer_destroy(lyrMainMenu);   lyrMainMenu = NULL;
  }
}


/**************************************************************************
 *
 **************************************************************************/
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


/**************************************************************************
 *
 **************************************************************************/
void wndMainMenu_destroy() {
  if (!wndMainMenu) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to destroy a null pointer!");
  } else {
    window_destroy(wndMainMenu);   wndMainMenu = NULL;
  }
}

