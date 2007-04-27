#ifndef NL_ASSERT_H_INCLUDED
#define NL_ASSERT_H_INCLUDED

#ifdef NDEBUG
#define assert(p) ((void)0)
#else
#include "nortlib.h"

#define assert(e) \
  ((e) ? (void)0 : \
  nl_error( 4, "%s:%d - Assert Failed: '%s'", __FILE__, __LINE__, #e ))
#endif

#endif
