#include <sys/types.h>
#include "collect.h";

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
  int n_req;
  int req_type;
  int n_req_togo;
  union {
    struct {
      send_id tmid;
      void (* handler)(struct readreq_s *req);
    } tm;
    char hdr[IBUF_SIZE];
  } u;
  char ibuf[IBUF_SIZE];
} readreq;
#define OMSREQ_IGNORE 0
#define OMSREQ_PROCESSTM 1
#define OMSREQ_LOG    2
/* cmd can hold
   - the text of the command which solicited the
   request a replacement header for log requests
*/

typedef struct {
  readreq *req;
  pid_t tm_proxy;
  char *cmd;
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
extern reqqueue *satisfied_queue;
extern charqueue *output_queue;
extern int enqueue_req( reqqueue *queue, readreq *req );
extern readreq *dequeue_req( reqqueue *queue );
extern int enqueue_char( charqueue *queue, char qchar );
extern char dequeue_char( charqueue *queue );
extern reqqueue *new_reqqueue( int size );
extern charqueue *new_charqueue( int size );
extern void handle_recv_data(void);

/* omsdrv.c */
void oms_init_options( int argc, char **argv );

/* omsirq.c */
void service_int( void );

#define OMS_PROXY 2
#define OMS_ST_PROXY 4
