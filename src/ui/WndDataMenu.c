#include <pebble.h>

// Deactivate APP_LOG in this file.

#include "misc.h"
#include "ui/WndDataMenu_Internal.h"


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




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static uint16_t WndDataMenu_get_num_sections_callback(MenuLayer* menu_layer, void* callback_context) {
  WndDataMenu* this = (WndDataMenu*) callback_context;
  MPA_RETURN_IF_NULL(this);

  return NUM_MENU_SECTIONS;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static uint16_t WndDataMenu_get_num_rows_callback(MenuLayer* menu_layer, uint16_t section_index, void* callback_context) {
  WndDataMenu* this = (WndDataMenu*) callback_context;
  MPA_RETURN_IF_NULL(this);

  switch (section_index) {
    case 0:
      return NUM_MENU_ITEMS;
    default:
      return 0;
  }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static void WndDataMenu_draw_row_callback(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* callback_context) {
  WndDataMenu* this = (WndDataMenu*) callback_context;
  if (this == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");   \
    return;
  }

  switch (cell_index->section) {
    case 0: {
      switch (cell_index->row) {
        case MNU_ITEM_LENDER_INFO:
          menu_cell_basic_draw(ctx, cell_layer, "Global Lender", NULL, NULL);
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


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static void WndDataMenu_select_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* callback_context) {
  WndDataMenu* this = (WndDataMenu*) callback_context;
  if (this == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");   \
    return;
  }

  // Use the row to specify which item will receive the select action
  switch (cell_index->row) {
    case MNU_ITEM_LENDER_INFO: {
      break;
    }
    case MNU_ITEM_LOANS_FOR_YOU: {
      break;
    }
    case MNU_ITEM_COUNTRIES: {
      break;
    }
    case MNU_ITEM_ACKNOWLEDGEMENTS: {
      break;
    }
  }
}


#if defined(PBL_ROUND)
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int16_t WndDataMenu_get_cell_height_callback(MenuLayer* menu_layer, MenuIndex* cell_index, void* callback_context) {
  WndDataMenu* this = (WndDataMenu*) callback_context;
  MPA_RETURN_IF_NULL(this);

  Layer* lyrRoot = window_get_root_layer(this->window);
  GRect bounds = layer_get_bounds(lyrRoot);
  return (int)(bounds.size.h / 3);
}
#endif


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static void WndDataMenu_load(Window* window) {
  WndDataMenu* this = (WndDataMenu*) window_get_user_data(window);
  if (this == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");   \
    return;
  }

  window_set_background_color(window, COLOR_FALLBACK(GColorDarkGreen, GColorBlack));
  Layer* lyrRoot = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(lyrRoot);

  if (this->lyrDataMenu) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to re-create layer before destroying.");
  } else {
    this->lyrDataMenu = menu_layer_create(bounds);
    menu_layer_set_callbacks(this->lyrDataMenu, this, (MenuLayerCallbacks) {
      .get_num_sections = WndDataMenu_get_num_sections_callback,
      .get_num_rows = WndDataMenu_get_num_rows_callback,
      .draw_row = WndDataMenu_draw_row_callback,
      .select_click = WndDataMenu_select_callback,
      .get_cell_height = PBL_IF_ROUND_ELSE(WndDataMenu_get_cell_height_callback, NULL)
    });

    menu_layer_set_normal_colors(this->lyrDataMenu, GColorBlack, GColorWhite);
    menu_layer_set_highlight_colors(this->lyrDataMenu, COLOR_FALLBACK(GColorPictonBlue, GColorWhite), GColorBlack);
    menu_layer_set_click_config_onto_window(this->lyrDataMenu, window);
    layer_add_child(lyrRoot, menu_layer_get_layer(this->lyrDataMenu));

  }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static void WndDataMenu_unload(Window* window) {
  WndDataMenu* this = (WndDataMenu*) window_get_user_data(window);
  if (this == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");   \
    return;
  }

  if (this->lyrDataMenu == NULL) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to destroy a null pointer!");
  } else {
    menu_layer_destroy(this->lyrDataMenu);   this->lyrDataMenu = NULL;
  }
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode WndDataMenu_updateView(WndDataMenu* this) {

  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode WndDataMenu_push(WndDataMenu* this) {
  // Show the Window on the watch, with animated=true
  window_stack_push(this->window, true);
  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Creates a data menu window and pushes it on the window stack.
/// @param[in,out]  this  Pointer to the WndDataMenu "object"; must be NULL
/////////////////////////////////////////////////////////////////////////////
WndDataMenu* WndDataMenu_create() {
  WndDataMenu* this = (WndDataMenu*) calloc(1, sizeof(WndDataMenu));
  this->window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(this->window, (WindowHandlers) {
    .load =   WndDataMenu_load,
    .unload = WndDataMenu_unload
  });

  window_set_user_data(this->window, this);

  return this;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode WndDataMenu_destroy(WndDataMenu* this) {
  MPA_RETURN_IF_NULL(this);
  if (this == NULL) {
    APP_LOG(APP_LOG_LEVEL_WARNING, "Attempting to destroy a null pointer!");
  } else {
    window_destroy(this->window);   this->window = NULL;
  }

  if (this != NULL) {
    free(this);  this = NULL;
  }
  return MPA_SUCCESS;
}

