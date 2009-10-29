/* collect.h defines routines applications might use to
   communicate with collection.
   $Log$
   Revision 1.3  2009/10/20 18:25:17  ntallen
   Add support for non-blocking writes with ionotify

   Revision 1.2  2008/07/29 19:21:06  ntallen
   Support for colsend

   Revision 1.1  2008/07/25 13:38:03  ntallen
   Col_send functionality
   Includes a proposed Col_Send class for C++

 * Revision 1.2  1994/11/22  14:53:44  nort
 * New functionality, C++ escapes.
 *
 * Revision 1.1  1993/01/09  15:50:41  nort
 * Initial revision
 *
 */
#ifndef _COLLECT_H_INCLUDED
#define _COLLECT_H_INCLUDED

#include <sys/siginfo.h>

#ifdef __cplusplus
  // This is a proposed C++ wrapper. To my knowledge it has not been implemented
  class Col_Send {
    public:
      Col_Send(char *name, void *data_in, int size_in, int synch);
      ~Col_Send();
      int send();
      int fd;
      int rv;
    private:
      void *data;
      int data_size;
  };

extern "C" {
#endif

/*   
   COL_SEND is used for data which is sent to collection via
   standard QNX6 IPC, namely a POSIX open/write.
   
   COL_SEND/COL_SEND_INIT:  The initialization uses the
   ASCIIZ name for the particular data as specified in the
   'TM "Receive" name 0;' statement. Collection will locate the
   specified name and return the pre-determined ID number. Possible
   return values in the type field:
     DAS_OK   id field of colmsg is the id for future correspondence.
	 DAS_UNKN the name wasn't found. Don't call back!
	 DAS_BUSY the name is in use by another program.

   COL_SEND/COL_SEND_SEND: Uses the data substructure of colmsg.
   The id established with the _INIT col goes in the id field.
   The size as determined by the sender is included with the
   message. It is compared to the size we like, and the smaller
   value is used for the actual data transfer.
   
   COL_SEND/COL_SEND_RESET: Uses the data substructure, but only
   the id field is used.
*/

/* API */

typedef struct {
  int fd;
  void *data;
  int data_size;
  int err_code;
  int armed;
  struct sigevent event;
} send_id_struct, *send_id;

send_id Col_send_init(const char *name, void *data, unsigned short size, int synch);
int Col_send_arm( send_id sender, int coid, short code, int value );
int Col_send(send_id sender);
int Col_send_reset(send_id sender);

#ifdef __cplusplus
};
#endif

#endif
