#include <stdio.h>
#include <stdbool.h>
#include <strings.h>
#include <stdlib.h>

#define DIGIT_OFFSET 15
static const char lc_digit_array[] = "fedcba9876543210123456789abcdef";
const char *UL_6_4_f_c( unsigned long int x) {
  static char obuf[7];
  int iov;

  if (x > 99999) return("******");
  obuf[6] = '\0';
  obuf[5] = lc_digit_array[(x % 10)+DIGIT_OFFSET];
  iov = x/10;
  obuf[4] = lc_digit_array[(iov % 10)+DIGIT_OFFSET];
  iov /= 10;
  obuf[3] = lc_digit_array[(iov % 10)+DIGIT_OFFSET];
  iov /= 10;
  obuf[2] = lc_digit_array[(iov % 10)+DIGIT_OFFSET];
  iov /= 10;
  obuf[1] = '.';
  obuf[0] = lc_digit_array[(iov % 10)+DIGIT_OFFSET];
  return(obuf);
}

static const char uc_digit_array[] = "FEDCBA9876543210123456789ABCDEF";
const char *US_4_4_X_0( unsigned short int x) {
  static char obuf[5];
  int iov;

  obuf[4] = '\0';
  obuf[3] = uc_digit_array[(x % 16)+DIGIT_OFFSET];
  iov = x/16;
  if (iov == 0) goto space2;
  obuf[2] = uc_digit_array[(iov % 16)+DIGIT_OFFSET];
  iov /= 16;
  if (iov == 0) goto space1;
  obuf[1] = uc_digit_array[(iov % 16)+DIGIT_OFFSET];
  iov /= 16;
  if (iov == 0) goto space0;
  obuf[0] = uc_digit_array[(iov % 16)+DIGIT_OFFSET];
  goto nospace;
  space2: obuf[2] = ' ';
  space1: obuf[1] = ' ';
  space0: obuf[0] = ' ';
  nospace:
  return(obuf);
}
const char *US_4_4_x_0( unsigned short int x) {
  static char obuf[5];
  int iov;

  obuf[4] = '\0';
  obuf[3] = lc_digit_array[(x % 16)+DIGIT_OFFSET];
  iov = x/16;
  if (iov == 0) goto space2;
  obuf[2] = lc_digit_array[(iov % 16)+DIGIT_OFFSET];
  iov /= 16;
  if (iov == 0) goto space1;
  obuf[1] = lc_digit_array[(iov % 16)+DIGIT_OFFSET];
  iov /= 16;
  if (iov == 0) goto space0;
  obuf[0] = lc_digit_array[(iov % 16)+DIGIT_OFFSET];
  goto nospace;
  space2: obuf[2] = ' ';
  space1: obuf[1] = ' ';
  space0: obuf[0] = ' ';
  nospace:
  return(obuf);
}
const char *SS_6_4_d_0( short int x) {
  static char obuf[7];
  int neg;

  neg = (x < 0) ? 1 : 0;
  obuf[6] = '\0';
  obuf[5] = lc_digit_array[(x % 10)+DIGIT_OFFSET];
  x /= 10;
  if (x == 0) {
    if (neg) { obuf[4] = '-'; goto space3; }
    else goto space4;
  }
  obuf[4] = lc_digit_array[(x % 10)+DIGIT_OFFSET];
  x /= 10;
  if (x == 0) {
    if (neg) { obuf[3] = '-'; goto space2; }
    else goto space3;
  }
  obuf[3] = lc_digit_array[(x % 10)+DIGIT_OFFSET];
  x /= 10;
  if (x == 0) {
    if (neg) { obuf[2] = '-'; goto space1; }
    else goto space2;
  }
  obuf[2] = lc_digit_array[(x % 10)+DIGIT_OFFSET];
  x /= 10;
  if (x == 0) {
    if (neg) { obuf[1] = '-'; goto space0; }
    else goto space1;
  }
  obuf[1] = lc_digit_array[(x % 10)+DIGIT_OFFSET];
  x /= 10;
  if (x == 0) {
    if (neg) { obuf[0] = '-'; goto nospace; }
    else goto space0;
  }
  obuf[0] = lc_digit_array[(x % 10)+DIGIT_OFFSET];
  goto nospace;
  space4: obuf[4] = ' ';
  space3: obuf[3] = ' ';
  space2: obuf[2] = ' ';
  space1: obuf[1] = ' ';
  space0: obuf[0] = ' ';
  nospace:
  return(obuf);
}
const char *SS_4_2_f_c( short int x) {
  static char obuf[5];
  int neg;

  if (x < -99 || x > 999) return("****");
  neg = (x < 0) ? 1 : 0;
  obuf[4] = '\0';
  obuf[3] = lc_digit_array[(x % 10)+DIGIT_OFFSET];
  x /= 10;
  obuf[2] = lc_digit_array[(x % 10)+DIGIT_OFFSET];
  x /= 10;
  obuf[1] = '.';
  if (neg) obuf[0] = '-'; else
  obuf[0] = lc_digit_array[(x % 10)+DIGIT_OFFSET];
  return(obuf);
}
/* _CVT_8() int tcvt AD8_F -> VOLTS */
typedef double AD8_F;

int main(int argc, char **argv) {
  unsigned short us;
  signed short ss;
  unsigned long ul;
  char buf[10];
  int fail_count;

  us = 0;
  fail_count = 0;
  do {
    snprintf(buf,10,"%4X",us);
    if (strcmp(buf, US_4_4_X_0(us))) {
      fprintf(stderr, "Error: US_4_4_X_0(%u) retd '%s', not '%s'\n",
        us, US_4_4_X_0(us), buf);
      if (++fail_count > 20) exit(1);
    }
  } while (++us != 0);
  if (fail_count == 0)
    printf("US_4_4_X_0() passed\n");

  ss = 0;
  fail_count = 0;
  do {
    snprintf(buf,10,"%6d",ss);
    if (strcmp(buf, SS_6_4_d_0(ss))) {
      fprintf(stderr, "Error: SS_6_4_d_0(%u) retd '%s', not '%s'\n",
        ss, SS_6_4_d_0(ss), buf);
      if (++fail_count > 20) exit(1);
    }
  } while (++ss != 0);
  if (fail_count == 0)
    printf("SS_6_4_d_0() passed\n");

  ul = 0;
  fail_count = 0;
  do {
    if (ul <= 99999)
      snprintf(buf,10,"%6.4lf",ul/10000.);
    else strcpy(buf, "******");
    if (strcmp(buf, UL_6_4_f_c(ul))) {
      fprintf(stderr, "Error: UL_6_4_f_c(%lu) retd '%s', not '%s'\n",
        ul, UL_6_4_f_c(ul), buf);
      if (++fail_count > 20) exit(1);
    }
  } while (++ul != 100001L);
  if (fail_count == 0)
    printf("UL_6_4_f_c() passed\n");

  return 0;
}

