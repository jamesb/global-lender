#include <pebble.h>

// Deactivate APP_LOG in this file.
//#undef APP_LOG
//#define APP_LOG(...)

#include "misc.h"
#include "libs/WndDataMenu_Internal.h"


const uint16_t WndDataMenu_defaultNumSections = 1;
const uint16_t WndDataMenu_defaultNumRows =     1;
const char* WndDataMenu_defaultRowTitle =       "Please wait...";


/////////////////////////////////////////////////////////////////////////////
/// Allocates a SectionInfo pointer.
/// @param[out]     section  double-pointer to SectionInfo; must be NULL on
///       entry
/////////////////////////////////////////////////////////////////////////////
static MagPebApp_ErrCode WndDataMenu_SectionInfo_create(SectionInfo** section) {
  if (*section != NULL) { return MPA_INVALID_INPUT_ERR; }

  *section = (SectionInfo*) malloc(1 * sizeof(**section));
  if (*section == NULL) { goto freemem; }
  (*section)->headerTitle = NULL;
  (*section)->numRows = WndDataMenu_defaultNumRows;
  (*section)->rowTitles = NULL;
  (*section)->rowSubtitles = NULL;

  return MPA_SUCCESS;

freemem:
  APP_LOG(APP_LOG_LEVEL_ERROR, "Error... freeing memory");
  if (*section != NULL) { free(*section);  *section = NULL; }
  return MPA_OUT_OF_MEMORY_ERR;
}


/////////////////////////////////////////////////////////////////////////////
/// Frees all memory associated with a SectionInfo pointer.
/// @param[in,out]  this  Pointer to SectionInfo; must be already allocated
/////////////////////////////////////////////////////////////////////////////
static MagPebApp_ErrCode WndDataMenu_SectionInfo_destroy(SectionInfo* this) {
  MPA_RETURN_IF_NULL(this);

  for (int rowIdx=0; rowIdx<this->numRows; rowIdx++) {
    if (this->rowTitles != NULL && this->rowTitles[rowIdx] != NULL) {
      free(this->rowTitles[rowIdx]);  this->rowTitles[rowIdx] = NULL;
    }
    if (this->rowSubtitles != NULL && this->rowSubtitles[rowIdx] != NULL) {
      free(this->rowSubtitles[rowIdx]);  this->rowSubtitles[rowIdx] = NULL;
    }
  }

  if (this->rowSubtitles != NULL) { free(this->rowSubtitles); this->rowSubtitles = NULL; }
  if (this->rowTitles != NULL)    { free(this->rowTitles);    this->rowTitles = NULL; }
  if (this->headerTitle != NULL)  { free(this->headerTitle);  this->headerTitle = NULL; }
  free(this); this = NULL;
  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Initializes a SectionInfo pointer and its members.
/// @param[in,out]  this  Pointer to SectionInfo; must be already
///       allocated upon entry, but heap-allocated members must be NULL
/////////////////////////////////////////////////////////////////////////////
static MagPebApp_ErrCode WndDataMenu_SectionInfo_init(SectionInfo* this) {
  MPA_RETURN_IF_NULL(this);
  MagPebApp_ErrCode mpaRet = MPA_SUCCESS;

  if (this->headerTitle != NULL)   {
    return MPA_INVALID_INPUT_ERR;
  }
  if (this->rowTitles != NULL) {
    return MPA_INVALID_INPUT_ERR;
  }
  if (this->rowSubtitles != NULL) {
    return MPA_INVALID_INPUT_ERR;
  }

  if ( (this->rowTitles = calloc(this->numRows, sizeof(*this->rowTitles)) ) == NULL) { goto freemem; }
  if ( (this->rowSubtitles = calloc(this->numRows, sizeof(*this->rowSubtitles)) ) == NULL) { goto freemem; }

  // Initialize row title to default; subtitle to NULL
  for (int rowIdx=0; rowIdx<this->numRows; rowIdx++) {
    if ( (this->rowTitles[rowIdx] = malloc(strlen(WndDataMenu_defaultRowTitle) + 1)) == NULL) { goto freemem; }
    strcpy(this->rowTitles[rowIdx], WndDataMenu_defaultRowTitle);
    this->rowSubtitles[rowIdx] = NULL;
  }

  return MPA_SUCCESS;

freemem:
  APP_LOG(APP_LOG_LEVEL_ERROR, "Error... freeing memory");
  if ( (mpaRet = WndDataMenu_SectionInfo_destroy(this)) != MPA_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Error destroying SectionInfo: %s", MagPebApp_getErrMsg(mpaRet));
  }
  return MPA_OUT_OF_MEMORY_ERR;
}




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static uint16_t WndDataMenu_getNumSectionsCallback(MenuLayer* menu_layer, void* callback_context) {
  WndDataMenu* this = (WndDataMenu*) callback_context;
  if (this == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");   \
    return 0;
  }

  return this->numSections;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static uint16_t WndDataMenu_getNumRowsCallback(MenuLayer* menu_layer, uint16_t section_index, void* callback_context) {
  WndDataMenu* this = (WndDataMenu*) callback_context;
  if (this == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");   \
    return 0;
  }

  if (section_index >= this->numSections) {
    return 0;
  }

  return this->sections[section_index]->numRows;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static void WndDataMenu_drawRowCallback(GContext* ctx, const Layer* cell_layer, MenuIndex* cell_index, void* callback_context) {
  WndDataMenu* this = (WndDataMenu*) callback_context;
  if (this == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");   \
    return;
  }

  if (cell_index->section >= this->numSections) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Section index %d out of bounds (max=%d)", cell_index->section, this->numSections - 1);
    return;
  }
  uint16_t sectIdx = cell_index->section;

  if (cell_index->row >= this->sections[sectIdx]->numRows) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Row index %d out of bounds (max=%d)", cell_index->row, this->sections[sectIdx]->numRows - 1);
    return;
  }
  uint16_t rowIdx = cell_index->row;

  if (this->sections[sectIdx] == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");   \
    return;
  }

  // title and subtitle may be NULL; menu_cell_basic_draw can handle that
  const char* title = this->sections[sectIdx]->rowTitles[rowIdx];
  const char* subtitle = this->sections[sectIdx]->rowSubtitles[rowIdx];

  menu_cell_basic_draw(ctx, cell_layer, title, subtitle, NULL);
}


#if defined(PBL_RECT)
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static void WndDataMenu_drawHeaderCallback(GContext* ctx, const Layer* cell_layer, uint16_t sectIdx, void* callback_context) {
  WndDataMenu* this = (WndDataMenu*) callback_context;
  if (this == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");   \
    return;
  }

  if (sectIdx >= this->numSections) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Section index %d out of bounds (max=%d)", sectIdx, this->numSections - 1);
    return;
  }

  if (this->sections[sectIdx] == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");   \
    return;
  }

  // headerTitle may be NULL; menu_cell_basic_header_draw can handle that
  const char* title = this->sections[sectIdx]->headerTitle;
  menu_cell_basic_header_draw(ctx, cell_layer, title);
}
#endif


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static void WndDataMenu_selectCallback(MenuLayer* menu_layer, MenuIndex* cell_index, void* callback_context) {
  WndDataMenu* this = (WndDataMenu*) callback_context;
  if (this == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");
    return;
  }

}


#if defined(PBL_ROUND)
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int16_t WndDataMenu_getCellHeightCallback(MenuLayer* menu_layer, MenuIndex* cell_index, void* callback_context) {
  WndDataMenu* this = (WndDataMenu*) callback_context;
  if (this == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");
    return 0;
  }

  Layer* lyrRoot = window_get_root_layer(this->window);
  GRect bounds = layer_get_bounds(lyrRoot);
  return (int)(bounds.size.h / 3);
}
#endif


#if defined(PBL_RECT)
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
static int16_t WndDataMenu_getHeaderHeightCallback(MenuLayer* menu_layer, uint16_t sectIdx, void* callback_context) {
  WndDataMenu* this = (WndDataMenu*) callback_context;
  if (this == NULL) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Attempted operation on NULL pointer.");
    return 0;
  }

  if (sectIdx >= this->numSections) {
    return 0;
  }

  if (this->sections[sectIdx] == NULL || this->sections[sectIdx]->headerTitle == NULL) {
    return 0;
  }

  return MENU_CELL_BASIC_HEADER_HEIGHT;
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
    if ( (this->lyrDataMenu = menu_layer_create(bounds)) == NULL) { goto freemem; }
    menu_layer_set_callbacks(this->lyrDataMenu, this, (MenuLayerCallbacks) {
      .get_num_sections =      WndDataMenu_getNumSectionsCallback,
      .get_num_rows =          WndDataMenu_getNumRowsCallback,
      .get_cell_height =       PBL_IF_ROUND_ELSE(WndDataMenu_getCellHeightCallback, NULL),
      .draw_background =       NULL,
      .draw_row =              WndDataMenu_drawRowCallback,
      .get_header_height =     PBL_IF_RECT_ELSE(WndDataMenu_getHeaderHeightCallback, NULL),
      .draw_header =           PBL_IF_RECT_ELSE(WndDataMenu_drawHeaderCallback, NULL),
      .get_separator_height =  NULL,
      .draw_separator =        NULL,
      .select_click =          WndDataMenu_selectCallback,
      .select_long_click =     NULL,
      .selection_changed =     NULL,
      .selection_will_change = NULL
    });

    menu_layer_set_normal_colors(this->lyrDataMenu, this->palette.normalBack, this->palette.normalFore);
    menu_layer_set_highlight_colors(this->lyrDataMenu, this->palette.highltBack, this->palette.highltFore);
    menu_layer_set_click_config_onto_window(this->lyrDataMenu, window);
    layer_add_child(lyrRoot, menu_layer_get_layer(this->lyrDataMenu));

  }

  return;  // success

freemem:
  APP_LOG(APP_LOG_LEVEL_ERROR, "Error... freeing memory");
  if (this->lyrDataMenu != NULL) {
    menu_layer_destroy(this->lyrDataMenu);
    this->lyrDataMenu = NULL;
  }
  return;
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
  MPA_RETURN_IF_NULL(this);

  if (this->lyrDataMenu != NULL) {
    menu_layer_reload_data(this->lyrDataMenu);
  }

  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Sets the number of sections for this data menu.
/// @param[in,out]  this  Pointer to the WndDataMenu "object"; must be
///       already allocated.
/// @param[in]      num   Number of sections
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode WndDataMenu_setNumSections(WndDataMenu* this, const uint16_t numSections) {
  MPA_RETURN_IF_NULL(this);
  MagPebApp_ErrCode mpaRet = MPA_SUCCESS;

  // Resize section array
  if (this->numSections != numSections) {
    MPA_RETURN_IF_NULL(this->sections);

    // Free sections, if requested
    if (numSections < this->numSections) {
      for (uint16_t sectIdx=this->numSections - 1; sectIdx >= numSections; --sectIdx) {
        if (this->sections[sectIdx] != NULL) {
          if ( (mpaRet = WndDataMenu_SectionInfo_destroy(this->sections[sectIdx])) != MPA_SUCCESS) {
            APP_LOG(APP_LOG_LEVEL_WARNING, "Error destroying SectionInfo: %s", MagPebApp_getErrMsg(mpaRet));
          } else {
            this->sections[sectIdx] = NULL;
          }
        }
      }
    }

    // Reallocate section array to requested size
    SectionInfo** tmpSects = NULL;
    tmpSects = realloc(this->sections, numSections * sizeof(*tmpSects));
    if (tmpSects == NULL) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Insufficient memory");
      return MPA_OUT_OF_MEMORY_ERR;
    }

    // Allocate sections, if requested
    if (numSections > this->numSections) {
      for (uint16_t sectIdx=this->numSections; sectIdx < numSections; sectIdx++) {
        this->sections[sectIdx] = NULL;
      }

      for (uint16_t sectIdx=this->numSections; sectIdx < numSections; sectIdx++) {
        if ( (mpaRet = WndDataMenu_SectionInfo_create(&this->sections[sectIdx])) != MPA_SUCCESS) {
          APP_LOG(APP_LOG_LEVEL_ERROR, "Error creating new SectionInfo: %s", MagPebApp_getErrMsg(mpaRet));
          goto freemem;
        }
        if ( (mpaRet = WndDataMenu_SectionInfo_init(this->sections[sectIdx])) != MPA_SUCCESS) {
          APP_LOG(APP_LOG_LEVEL_ERROR, "Error initializing new SectionInfo: %s", MagPebApp_getErrMsg(mpaRet));
          goto freemem;
        }
      }
    }

    // Assign to new allocations
    this->sections = tmpSects;
    tmpSects = NULL;

    this->numSections = numSections;
  }

  return MPA_SUCCESS;

freemem:
  APP_LOG(APP_LOG_LEVEL_ERROR, "Error... freeing memory");
  if (numSections > this->numSections) {
    for (uint16_t sectIdx=this->numSections; sectIdx < numSections; sectIdx++) {
      if (this->sections[sectIdx] != NULL) {
        if ( (mpaRet = WndDataMenu_SectionInfo_destroy(this->sections[sectIdx])) != MPA_SUCCESS) {
          APP_LOG(APP_LOG_LEVEL_WARNING, "Error destroying SectionInfo: %s", MagPebApp_getErrMsg(mpaRet));
        } else {
          this->sections[sectIdx] = NULL;
        }
      }
    }
  }

  return MPA_OUT_OF_MEMORY_ERR;
}


/////////////////////////////////////////////////////////////////////////////
/// Sets the title for a section of this data menu.
///
/// @param[in,out]  this  Pointer to the WndDataMenu "object"; must be
///       already allocated.
/// @param[in]      sectIdx   The index of the section that this function
///       will build.
/// @param[in]      headerTitle  The title for this section. May be NULL.
///
/// @return  MPA_SUCCESS on success
///          MPA_INVALID_INPUT_ERR if sectIdx is not a valid section index
///          MPA_NULL_POINTER_ERR if parameters this or this->sections are
///            NULL upon entry.
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode WndDataMenu_setSectionTitle(WndDataMenu* this, const uint16_t sectIdx, const char* headerTitle) {
  MPA_RETURN_IF_NULL(this);
  if (sectIdx >= this->numSections) {
    return MPA_INVALID_INPUT_ERR;
  }

  MPA_RETURN_IF_NULL(this->sections);
  MPA_RETURN_IF_NULL(this->sections[sectIdx]);
  SectionInfo* theSect = this->sections[sectIdx];

  MagPebApp_ErrCode mpaRet = MPA_SUCCESS;

  if ( (mpaRet = strxcpyalloc(&(theSect->headerTitle), headerTitle) != MPA_SUCCESS) ) {
    goto freemem;
  }

  return MPA_SUCCESS;


freemem:
  APP_LOG(APP_LOG_LEVEL_ERROR, "Error... freeing memory");
  if (theSect->headerTitle != NULL) {
    free(theSect->headerTitle);
    theSect->headerTitle = NULL;
  }
  return MPA_OUT_OF_MEMORY_ERR;
}


/////////////////////////////////////////////////////////////////////////////
/// Sets up a section of this data menu.
///
/// A call to this function may be preceded by a call to setNumSections to
/// indicate the number of sections in this data menu (default is 1).
/// A call to this function should be followed by a call buildRow to setup
/// the rows within this section.
///
/// @param[in,out]  this  Pointer to the WndDataMenu "object"; must be
///       already allocated.
/// @param[in]      sectIdx   The index of the section that this function
///       will build.
/// @param[in]      numRows  Indicates the number of rows this section will
///       contain.
/// @param[in]      headerTitle  The title for this section. May be NULL.
///
/// @return  MPA_SUCCESS on success
///          MPA_INVALID_INPUT_ERR if sectIdx is not a valid section index
///          MPA_NULL_POINTER_ERR if parameters this or this->sections are
///            NULL upon entry.
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode WndDataMenu_buildSection(WndDataMenu* this, const uint16_t sectIdx, const uint16_t numRows, const char* headerTitle) {
  MPA_RETURN_IF_NULL(this);
  if (sectIdx >= this->numSections) {
    return MPA_INVALID_INPUT_ERR;
  }

  MPA_RETURN_IF_NULL(this->sections);
  MPA_RETURN_IF_NULL(this->sections[sectIdx]);
  SectionInfo* theSect = this->sections[sectIdx];

  // Resize row arrays
  char** tmpRT = NULL;
  char** tmpRS = NULL;

  if (theSect->numRows != numRows) {
    MPA_RETURN_IF_NULL(theSect->rowTitles);
    MPA_RETURN_IF_NULL(theSect->rowSubtitles);

    // Deallocate rows, if requested
    if (numRows < theSect->numRows) {
      for (uint16_t rowIdx=theSect->numRows - 1; rowIdx >= numRows; --rowIdx) {
        if (theSect->rowTitles[rowIdx] != NULL) {
          free(theSect->rowTitles[rowIdx]);  theSect->rowTitles[rowIdx] = NULL;
        }
        if (theSect->rowSubtitles[rowIdx] != NULL) {
          free(theSect->rowSubtitles[rowIdx]);  theSect->rowSubtitles[rowIdx] = NULL;
        }
      }
    }

    tmpRT = realloc(theSect->rowTitles, numRows * sizeof(*tmpRT));
    if (tmpRT == NULL) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Insufficient memory");
      goto freemem;
    }

    tmpRS = realloc(theSect->rowSubtitles, numRows * sizeof(*tmpRS));
    if (tmpRS == NULL) {
      APP_LOG(APP_LOG_LEVEL_ERROR, "Insufficient memory");
      goto freemem;
    }

    theSect->rowTitles = tmpRT;
    theSect->rowSubtitles = tmpRS;
    tmpRT = NULL;
    tmpRS = NULL;

    // Initialize rows, if more rows requested
    if (numRows > theSect->numRows) {
      for (uint16_t rowIdx=theSect->numRows; rowIdx < numRows; rowIdx++) {
        theSect->rowTitles[rowIdx] = NULL;
        theSect->rowSubtitles[rowIdx] = NULL;
      }
    }

    theSect->numRows = numRows;
  }

  MagPebApp_ErrCode mpaRet = MPA_SUCCESS;
  if ( (mpaRet = WndDataMenu_setSectionTitle(this, sectIdx, headerTitle) != MPA_SUCCESS) ) {
    goto freemem;
  }
  return MPA_SUCCESS;

freemem:
  APP_LOG(APP_LOG_LEVEL_ERROR, "Error... freeing memory");
  if (tmpRT != NULL) {
    free(tmpRT);  tmpRT = NULL;
  }
  if (tmpRS != NULL) {
    free(tmpRS);  tmpRS = NULL;
  }

  return MPA_OUT_OF_MEMORY_ERR;
}


/////////////////////////////////////////////////////////////////////////////
/// Sets up a row in a section of this data menu.
///
/// A call to this function must be preceded by a call to buildSection to
/// allocate the number of rows in this data menu section (default is 1).
///
/// @param[in,out]  this  Pointer to the WndDataMenu "object"; must be
///       already allocated.
/// @param[in]      sectIdx   The index of the section that this function
///       will build.
/// @param[in]      rowIdx   The index of the row that this function will
///       build.
/// @param[in]      rowTitle  The title for this row. May be NULL.
/// @param[in]      rowSubtitle  The subtitle for this section. May be NULL.
///
/// @return  MPA_SUCCESS on success
///          MPA_INVALID_INPUT_ERR if sectIdx is not a valid section index
///            or rowIdx is not a valid row index in that section.
///          MPA_NULL_POINTER_ERR if parameters this or this->sections are
///            NULL upon entry.
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode WndDataMenu_buildRow(WndDataMenu* this, const uint16_t sectIdx, const uint16_t rowIdx, const char* rowTitle, const char* rowSubtitle) {
  MPA_RETURN_IF_NULL(this);
  if (sectIdx >= this->numSections) {
    return MPA_INVALID_INPUT_ERR;
  }

  MPA_RETURN_IF_NULL(this->sections);
  MPA_RETURN_IF_NULL(this->sections[sectIdx]);
  SectionInfo* theSect = this->sections[sectIdx];

  MagPebApp_ErrCode mpaRet = MPA_SUCCESS;

  if (rowIdx >= theSect->numRows) {
    return MPA_INVALID_INPUT_ERR;
  }

  if ( (mpaRet = strxcpyalloc(&(theSect->rowTitles[rowIdx]), rowTitle) != MPA_SUCCESS) ) {
    goto freemem;
  }

  if ( (mpaRet = strxcpyalloc(&(theSect->rowSubtitles[rowIdx]), rowSubtitle) != MPA_SUCCESS) ) {
    goto freemem;
  }

  return MPA_SUCCESS;


freemem:
  APP_LOG(APP_LOG_LEVEL_ERROR, "Error... freeing memory");
  if (theSect->rowTitles[rowIdx] != NULL) {
    free(theSect->rowTitles[rowIdx]);
    theSect->rowTitles[rowIdx] = NULL;
  }
  if (theSect->rowSubtitles[rowIdx] != NULL) {
    free(theSect->rowSubtitles[rowIdx]);
    theSect->rowSubtitles[rowIdx] = NULL;
  }
  return MPA_OUT_OF_MEMORY_ERR;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode WndDataMenu_setPalette(WndDataMenu* this, const WndDataMenu_Palette pal) {
  MPA_RETURN_IF_NULL(this);
  this->palette = pal;
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
/////////////////////////////////////////////////////////////////////////////
WndDataMenu* WndDataMenu_create() {
  WndDataMenu* this = NULL;
  MagPebApp_ErrCode mpaRet;

  if ( (this = malloc(1 * sizeof(*this)) ) == NULL) { goto freemem; }

  if ( (mpaRet = WndDataMenu_init(this)) != MPA_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not initialize data menu: %s", MagPebApp_getErrMsg(mpaRet));
    WndDataMenu_destroy(this);  this = NULL;
    return this;
  }

  return this;  // success

freemem:
  APP_LOG(APP_LOG_LEVEL_ERROR, "Error... freeing memory");
  if (this != NULL) { free(this);  this=NULL; }
  return this;
}


/////////////////////////////////////////////////////////////////////////////
/// Internal initialization
/// @param[in,out]  this  Pointer to WndDataMenu; must be already allocated
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode WndDataMenu_init(WndDataMenu* this) {
  MPA_RETURN_IF_NULL(this);
  MagPebApp_ErrCode mpaRet;

  this->window = NULL;
  if ( (this->window = window_create()) == NULL) { goto freemem; }

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(this->window, (WindowHandlers) {
    .load =   WndDataMenu_load,
    .unload = WndDataMenu_unload
  });

  window_set_user_data(this->window, this);

  this->lyrDataMenu = NULL;
  // lyrDataMenu will be allocated / initialized in the window loader

  this->palette = (WndDataMenu_Palette) {
    .normalBack = GColorBlack,
    .normalFore = GColorWhite,
    .highltBack = GColorWhite,
    .highltFore = GColorBlack
  };

  this->numSections = WndDataMenu_defaultNumSections;
  if ( (this->sections = calloc(this->numSections, sizeof(*this->sections)) ) == NULL) { goto freemem; }

  for (int sectIdx=0; sectIdx<this->numSections; sectIdx++) {
    if ( (mpaRet = WndDataMenu_SectionInfo_create(&this->sections[sectIdx])) != MPA_SUCCESS) { goto freemem; }
    if ( (mpaRet = WndDataMenu_SectionInfo_init(this->sections[sectIdx])) != MPA_SUCCESS) { goto freemem; }
  }

  return MPA_SUCCESS;

freemem:
  APP_LOG(APP_LOG_LEVEL_ERROR, "Error... freeing memory");
  if (this->window != NULL) {
    window_destroy(this->window);
    this->window = NULL;
  }

  for (int sectIdx=0; sectIdx<this->numSections; sectIdx++) {
    if (this->sections != NULL && this->sections[sectIdx] != NULL) {
      WndDataMenu_SectionInfo_destroy(this->sections[sectIdx]);
      this->sections[sectIdx] = NULL;
    }
  }

  if (this->sections != NULL) {
    free(this->sections);  this->sections = NULL;
  }

  return MPA_OUT_OF_MEMORY_ERR;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode WndDataMenu_destroy(WndDataMenu* this) {
  MPA_RETURN_IF_NULL(this);
  MagPebApp_ErrCode mpaRet;

  if (this->window != NULL) {
    window_destroy(this->window);
    this->window = NULL;
  }

  if (this->lyrDataMenu != NULL) {
    menu_layer_destroy(this->lyrDataMenu);
    this->lyrDataMenu = NULL;
  }

  for (int sectIdx=0; sectIdx<this->numSections; sectIdx++) {
    if (this->sections != NULL && this->sections[sectIdx] != NULL) {
      if ( (mpaRet = WndDataMenu_SectionInfo_destroy(this->sections[sectIdx])) != MPA_SUCCESS) {
        APP_LOG(APP_LOG_LEVEL_WARNING, "Error encountered while destroying section info: %s", MagPebApp_getErrMsg(mpaRet));
      }
      this->sections[sectIdx] = NULL;
    }
  }

  if (this->sections != NULL) {
    free(this->sections);  this->sections = NULL;
  }

  if (this != NULL) {
    free(this);  this = NULL;
  }
  return MPA_SUCCESS;
}


