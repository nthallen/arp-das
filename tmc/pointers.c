/* pointers.c handles pointer sharing and proxy sharing.
 * $Log$
 * Revision 1.3  2008/07/29 20:11:22  ntallen
 * Changes for Col_send
 *
 * Revision 1.2  2008/07/03 18:18:48  ntallen
 * To compile under QNX6 with minor blind adaptations to changes between
 * dbr.h and tm.h
 *
 * Revision 1.1  2008/07/03 15:11:07  ntallen
 * Copied from QNX4 version V1R9
 *
 * Revision 1.6  1998/02/17 17:16:31  nort
 * Support for col_send() messages longer than 95 bytes.
 *
 * Revision 1.5  1995/10/18  02:04:36  nort
 * Support for "Receive" data
 *
 * Revision 1.4  1993/09/27  19:40:23  nort
 * Cleanup, common compiler functions.
 *
 * Revision 1.3  1993/01/09  02:28:10  nort
 * Update before minor upgrade
 */
#include <stdio.h>
#include <string.h>
#include "tmc.h"
#include "nortlib.h"

static char rcsid[] =
      "$Id$";

static struct ppp {
  struct ppp *next;
  char type;
  unsigned char id;
  char *name;
} *pps = NULL;
#define PPP_PTR 1
#define PPP_PROXY 2
#define PPP_RECV 4
static char ppp_has = 0;

void add_ptr_proxy(char *type, char *name, int id) {
  struct ppp *npp;
  char type_code;
  
  if (stricmp(type, "\"pointer\"") == 0) type_code = PPP_PTR;
  else if (stricmp(type, "\"proxy\"") == 0) type_code = PPP_PROXY;
  else if (stricmp(type, "\"receive\"") == 0) type_code = PPP_RECV;
  else
    compile_error(2, "Undefined type %s in pointer or proxy definition", type);

  if ( type_code == PPP_RECV ) {
    if ( id < 0 || id > 1 )
      compile_error(2, "TM 'Receive' ID (synch code) must be 0 or 1" );
  } else {
    for (npp = pps; npp != NULL; npp = npp->next)
    	if (id == npp->id && type_code == npp->type)
    	  compile_error(2, "Duplicate ID %d in %s definition", id, type);
    if (id < 0 || id > 255)
      compile_error(2, "Illegal pointer or proxy ID %d", id);
  }
  npp = new_memory(sizeof(struct ppp));
  npp->next = pps;
  npp->id = id;
  npp->name = name;
  npp->type = type_code;
  ppp_has |= npp->type;
  pps = npp;
}

static void print_pp_cases(void) {
  struct ppp *npp;

  if (ppp_has & PPP_PTR) {
    #ifdef PPP_PTR_IMPLEMENTED
  	fprintf(ofile,
  	  "\tcase COL_SET_POINTER:\n"
  	  "\t  cmsg = (struct colmsg *)msg_ptr;\n"
  	  "\t  switch (cmsg->id) {\n");
  	for (npp = pps; npp != NULL; npp = npp->next)
  	  if (npp->type == PPP_PTR) {
  		fprintf(ofile,
  		  "\t\tcase %d: "
  		  "COL_get_pointer(sent_tid, &%s, cmsg->u.pointer); "
  		  "break;\n", npp->id, npp->name);
  	  }
  	fprintf(ofile,
  	  "\t\tdefault: return(reply_byte(sent_tid, DAS_UNKN));\n"
  	  "\t  }\n"
  	  "\t  return(0);\n"
  	  "\tcase COL_RESET_POINTER:\n"
  	  "\t  cmsg = (struct colmsg *)msg_ptr;\n"
  	  "\t  switch (cmsg->id) {\n");
  	for (npp = pps; npp != NULL; npp = npp->next)
  	  if (npp->type == PPP_PTR) {
  		fprintf(ofile,
  		  "\t\tcase %d: "
  		  "COL_free_pointer(sent_tid, &%s); "
  		  "break;\n", npp->id, npp->name);
  	  }
  	fprintf(ofile,
  	  "\t\tdefault: return(reply_byte(sent_tid, DAS_UNKN));\n"
  	  "\t  }\n"
  	  "\t  return(0);\n");
    #endif
  }
  if (ppp_has & PPP_PROXY) {
    #if PPP_PROXY_IMPLEMENTED
    fprintf(ofile,
      "\tcase COL_SET_PROXY:\n"
      "\t  cmsg = (struct colmsg *)msg_ptr;\n"
      "\t  switch (cmsg->id) {\n");
    for (npp = pps; npp != NULL; npp = npp->next)
      if (npp->type == PPP_PROXY) {
        fprintf(ofile,
          "\t\tcase %d: "
          "COL_recv_proxy(sent_tid, &%s, cmsg->u.proxy); "
          "break;\n", npp->id, npp->name);
      }
    fprintf(ofile,
      "\t\tdefault: return(reply_byte(sent_tid,DAS_UNKN));\n"
      "\t  }\n"
      "\t  return(0);\n"
      "\tcase COL_RESET_PROXY:\n"
      "\t  cmsg = (struct colmsg *)msg_ptr;\n"
      "\t  switch (cmsg->id) {\n");
  	for (npp = pps; npp != NULL; npp = npp->next)
  	  if (npp->type == PPP_PROXY) {
    		fprintf(ofile,
    		  "\t\tcase %d: "
    		  "COL_end_proxy(sent_tid, &%s, cmsg); "
    		  "break;\n", npp->id, npp->name);
    	  }
  	fprintf(ofile,
  	  "\t\tdefault: return(reply_byte(sent_tid,DAS_UNKN));\n"
  	  "\t  }\n"
  	  "\t  return(0);\n");
    #endif
  }
  if (ppp_has & PPP_RECV) {
  	for (npp = pps; npp != NULL; npp = npp->next)
  	  if (npp->type == PPP_RECV) {
    		fprintf(ofile,
    		  "  receive(\"%s\", &%s, sizeof(%s), %d);\n",
          npp->name, npp->name, npp->name, npp->id );
      }
  }
}

void print_ptr_proxy(void) {
  Skel_copy(ofile, "init_senders_head", 1);
  print_pp_cases();
  Skel_copy(ofile, "init_senders_tail", 1);
}

#ifdef __IMPLEMENTATION
Before DG_other
  #include "ofname"

In DG_other
  case COL_SET_POINTER:
	switch (cmsg->id) {
	  case n: DG_get_pointer(sent_tid, &name, cmsg->u.ptr); break;
	  default: return(reply_byte(DAS_UNKN));
	}
	return(0);
  case COL_RESET_POINTER:
	switch (cmsg->id) {
	  case n: DG_free_pointer(send_tid, &name); break;
	  default: return(reply_byte(DAS_UNKN));
	}
	return(0);
  case COL_SET_PROXY:
	switch (id) {
	  case n: if (name != 0) return(DAS_UNKN); name = proxy; break;
	  default: return DAS_UNKN;
	}
	break;
  case COL_RESET_PROXY:
	{
	  pid_t proxy;
	  
	  switch (id) {
		case n: proxy = name; name = 0; break;
		default: return DAS_UNKN
	  }
	  return proxy;
	}
	break;
  case COL_SEND:
	cmsg = (struct colmsg *)msg_ptr;
	switch (cmsg->id) {
	  case COL_SEND_INIT:
		if (stricmp(cmsg->u.name, "first") == 0) {
		  cmsg->u.data.id = first.id;
		  cmsg->u.data.size = first.size;
		  cmsg->type = DAS_OK;
		} else if (stricmp...) {
		} else return reply_byte(sent_tid,DAS_UNKN);
		Reply(sent_tid, cmsg, offsetof(struct colmsg, u.data.data));
		return 0;
	  case COL_SEND:
		switch (cmsg->u.data.id) {
		  case n:
			memcpy(cmsg->u.data.data, structure, 
					min(sizeof(structure), cmsg->u.data.size));
			break;
		  default: return reply_byte(sent_tid, DAS_UNKN);
		}
		break;
	  case COL_SEND_RESET: break;
	  default: return reply_byte(sent_tid,DAS_UNKN);
	}
	return reply_byte(sent_tid, DAS_OK);
#endif
