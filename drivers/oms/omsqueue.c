#include <stdlib.h>
#include "omsint.h"

reqqueue *new_reqqueue( int size ) {
  reqqueue *rqueue;
  rqueue = malloc( sizeof(reqqueue) );
  rqueue->head = rqueue->tail = 0;
  rqueue->size = size;
  rqueue->buf = malloc( size * sizeof(readreq *) );
  return rqueue;
}

charqueue *new_charqueue( int size ) {
  charqueue *cqueue;
  cqueue = malloc( sizeof(charqueue) );
  cqueue->head = cqueue->tail = 0;
  cqueue->size = size;
  cqueue->buf = malloc( size * sizeof(char) );
  return cqueue;
}
