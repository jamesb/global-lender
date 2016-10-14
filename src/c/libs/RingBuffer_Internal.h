#include <pebble.h>
#include "magpebapp.h"
#include "RingBuffer.h"


struct RingBuffer {
  size_t    length;
  size_t    read;
  size_t    write;
  void**    buf;
};


MagPebApp_ErrCode RingBuffer_init(RingBuffer* this, size_t);
