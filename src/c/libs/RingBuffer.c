#include <pebble.h>
#include "RingBuffer_Internal.h"


#define RING_BUFFER_EMPTY(RB) ((RB)->read == (RB)->write)

#define RING_BUFFER_FULL(RB) ((RB)->read == ( (RB)->write + 1 ) % ((RB)->length))

#define RING_BUFFER_ADVANCE_READ(RB, N) ((RB)->read = ((RB)->read + N) % ((RB)->length) )

#define RING_BUFFER_ADVANCE_WRITE(RB, N) ((RB)->write = ((RB)->write + N) % ((RB)->length) )

/////////////////////////////////////////////////////////////////////////////
/// Constructor
/// @param[in]      capacity   Defines the number of slots available in this
///       RingBuffer data structure.
/////////////////////////////////////////////////////////////////////////////
RingBuffer* RingBuffer_create(size_t capacity) {
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Creating RingBuffer [%zd]", capacity);
  int mpaRet;

  RingBuffer* newRingBuffer = malloc(sizeof(*newRingBuffer));
  if ( (mpaRet = RingBuffer_init(newRingBuffer, capacity)) != MPA_SUCCESS) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "Could not initialize: %s", MagPebApp_getErrMsg(mpaRet));
    RingBuffer_destroy(newRingBuffer);  newRingBuffer = NULL;
  }

  return newRingBuffer;
}


/////////////////////////////////////////////////////////////////////////////
/// Internal initialization
/// @param[in,out]  this  Pointer to RingBuffer; must be already allocated
/// @param[in]      capacity   Defines the number of slots in the RingBuffer
///       data structure.
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode RingBuffer_init(RingBuffer* this, size_t capacity) {
  MPA_RETURN_IF_NULL(this);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Initializing RingBuffer [%zd]", capacity);
  MagPebApp_ErrCode mpaRet = MPA_SUCCESS, myRet = MPA_OUT_OF_MEMORY_ERR;

  this->buf = NULL;
  this->read = 0;
  this->write = 0;

  if ( (capacity == 0) || ((capacity + 1) == 0) ) { myRet = MPA_INVALID_INPUT_ERR; goto freemem; }
  this->length = capacity + 1;
  
  // Allocate slots
  this->buf = calloc(this->length, sizeof(*this->buf));
  if (this->buf == NULL) { goto freemem; }

  return MPA_SUCCESS;

freemem:
  APP_LOG(APP_LOG_LEVEL_ERROR, "Error... freeing memory");
  if (this != NULL) {
    RingBuffer_destroy(this);  this = NULL;
  }
  return myRet;
}


/////////////////////////////////////////////////////////////////////////////
/// Destroys RingBuffer and frees allocated memory for slots. If the slots
/// are pointers to heap-allocated memory, that heap-allocated memory is
/// not freed (by default).
/// @param[in,out]  this  Pointer to RingBuffer; must be already allocated
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode RingBuffer_destroy(RingBuffer* this) {
  MPA_RETURN_IF_NULL(this);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "Destroying RingBuffer");
  MagPebApp_ErrCode mpaRet;

  // Free memory for slots
  if (this->buf != NULL) {
    free(this->buf);
    this->buf = NULL;
  }

  free(this); this = NULL;
  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Returns whether the RingBuffer is empty (has no data stored in slots).
/// @param[in,out]  this  Pointer to RingBuffer; must be already allocated
/// @param[out]     out  Pointer to boolean (true if empty)
///
/// @return  MPA_SUCCESS on success
///          MPA_NULL_POINTER_ERR if the RingBuffer pointer is null
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode RingBuffer_empty(RingBuffer* this, bool* out) {
  MPA_RETURN_IF_NULL(this);
  *out = RING_BUFFER_EMPTY(this);
  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Returns whether the RingBuffer is full (has no data slot capacity).
/// @param[in,out]  this  Pointer to RingBuffer; must be already allocated
/// @param[out]     out  Pointer to boolean (true if full)
///
/// @return  MPA_SUCCESS on success
///          MPA_NULL_POINTER_ERR if the RingBuffer pointer is null
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode RingBuffer_full(RingBuffer* this, bool* out) {
  MPA_RETURN_IF_NULL(this);
  *out = RING_BUFFER_FULL(this);
  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Provides the first data element without removing any buffer slots.
/// @param[in,out]  this  Pointer to RingBuffer; must be already allocated
/// @param[out]     data  Void double-pointer to data at the first slot.
///
/// @return  MPA_SUCCESS on success
///          MPA_NULL_POINTER_ERR if the RingBuffer pointer is null
///          MPA_EMPTY_ERR if the RingBuffer is empty (data will be NULL)
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode RingBuffer_peek(RingBuffer* this, void** data) {
  MPA_RETURN_IF_NULL(this);
  
  if (RING_BUFFER_EMPTY(this)) {
    *data = NULL;
    return MPA_EMPTY_ERR;
  }
  
  *data = this->buf[this->read];
  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Removes the first data element from the buffer slot. (Advances the read 
/// pointer without retrieving the data.)
/// @param[in,out]  this  Pointer to RingBuffer; must be already allocated
///
/// @return  MPA_SUCCESS on success
///          MPA_NULL_POINTER_ERR if the RingBuffer pointer is null
///          MPA_EMPTY_ERR if the RingBuffer is empty (nothing happens)
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode RingBuffer_drop(RingBuffer* this) {
  MPA_RETURN_IF_NULL(this);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "RingBuffer DROP");
  
  if (RING_BUFFER_EMPTY(this)) {
    return MPA_EMPTY_ERR;
  }
  
  RING_BUFFER_ADVANCE_READ(this, 1);
  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Provides the first data element and removes it from the buffer slot.
/// @param[in,out]  this  Pointer to RingBuffer; must be already allocated
/// @param[out]     data  Void double-pointer to data at the first slot.
///
/// @return  MPA_SUCCESS on success
///          MPA_NULL_POINTER_ERR if the RingBuffer pointer is null
///          MPA_EMPTY_ERR if the RingBuffer is empty (data will be NULL)
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode RingBuffer_read(RingBuffer* this, void** data) {
  MPA_RETURN_IF_NULL(this);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "RingBuffer READ");
  
  if (RING_BUFFER_EMPTY(this)) {
    *data = NULL;
    return MPA_EMPTY_ERR;
  }
  
  *data = this->buf[this->read];
  RING_BUFFER_ADVANCE_READ(this, 1);
  
  return MPA_SUCCESS;
}


/////////////////////////////////////////////////////////////////////////////
/// Provides the first data element and removes it from the buffer slot.
/// @param[in,out]  this  Pointer to RingBuffer; must be already allocated
/// @param[out]     data  Void pointer to data to be written.
///
/// @return  MPA_SUCCESS on success
///          MPA_NULL_POINTER_ERR if the RingBuffer pointer is null
///          MPA_FULL_ERR if the RingBuffer capacity is already used; no
///            data will be written in this case.
/////////////////////////////////////////////////////////////////////////////
MagPebApp_ErrCode RingBuffer_write(RingBuffer* this, void* data) {
  MPA_RETURN_IF_NULL(this);
  APP_LOG(APP_LOG_LEVEL_DEBUG, "RingBuffer WRITE");
  
  if (RING_BUFFER_FULL(this)) {
    return MPA_FULL_ERR;
  }
  
  this->buf[this->write] = data;
  RING_BUFFER_ADVANCE_WRITE(this, 1);
  return MPA_SUCCESS;
}

