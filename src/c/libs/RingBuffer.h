#pragma once

#include <pebble.h>
#include "magpebapp.h"


typedef struct RingBuffer RingBuffer;
  

RingBuffer* RingBuffer_create(size_t capacity);
MagPebApp_ErrCode RingBuffer_destroy(RingBuffer* this);

MagPebApp_ErrCode RingBuffer_empty(RingBuffer* this, bool*);
MagPebApp_ErrCode RingBuffer_full(RingBuffer* this, bool*);

MagPebApp_ErrCode RingBuffer_peek(RingBuffer* this, void**);
MagPebApp_ErrCode RingBuffer_drop(RingBuffer* this);
MagPebApp_ErrCode RingBuffer_read(RingBuffer* this, void**);

MagPebApp_ErrCode RingBuffer_write(RingBuffer* this, void*);