#ifndef OMSINT_H_INCLUDED
#define OMSINT_H_INCLUDED

#include <sys/types.h>
#include "collect.h"

#define PC68_BASE 0x320
#define PC68_DATA PC68_BASE
#define PC68_DONE (PC68_BASE+1)
#define PC68_CONTROL (PC68_BASE+2)
#define PC68_STATUS (PC68_BASE+3)

#define PC68_INIT_S (1<<1)
#define PC68_TBE_S (1<<6)
#define PC68_IBF_S (1<<5)
#define PC68_IRQ_S (1<<7)

#define IBUF_SIZE 80
typedef struct readreq_s {
  int req_type;
  union {
    struct {
      send_id tmid;
      void (* handler)(struct readreq_s *req);
    } tm;
    char hdr[IBUF_SIZE];
  } u;
  char cmd[IBUF_SIZE];
  char ibuf[IBUF_SIZE];
} readreq;
#define OMSREQ_IGNORE 0
#define OMSREQ_PROCESSTM 1
#define OMSREQ_LOG    2
#define OMSREQ_NO_RESPONSE 3
/* cmd can hold
   - the text of the command which solicited the
   request a replacement header for log requests
*/

typedef struct {
  readreq *req;
  int pending;
} tm_data_req;

typedef struct {
  int head, tail, size;
  readreq **buf;
} reqqueue;

typedef struct {
  int head, tail, size;
  char *buf;
} charqueue;

extern reqqueue *pending_queue;
extern reqqueue *free_queue;
extern charqueue *output_queue;
int enqueue_req( reqqueue *queue, readreq *req );
readreq *dequeue_req( reqqueue *queue );
int enqueue_char( charqueue *queue, char qchar );
char dequeue_char( charqueue *queue );
reqqueue *new_reqqueue( int size );
charqueue *new_charqueue( int size );
void handle_recv_data(void);

/* omsdrv.c */
void oms_init_options( int argc, char **argv );
char *quote_np(const char *s);
void set_oms_timeout( int ms );

/* omsirq.c */
void service_int( void );
void pq_check(void);
void pq_recycle(void);
void pq_timeout(void);
extern readreq *current_req;

#define PQ_MODE_IDLE 0
#define PQ_MODE_SENDING 1
#define PQ_MODE_RECEIVING 2
#define OMS_INITIAL_TIMEOUT 500
#define OMS_PAUSE_TIMEOUT 20

#endif
