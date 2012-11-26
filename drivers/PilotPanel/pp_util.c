#include <ctype.h>
#include <unistd.h>
#include "subbus.h"
#include "nortlib.h"

void lk204_set_led(int led, int color) {
  sbwr(0x1102, 0x1FE);
  sbwr(0x1102, (color&1) ? 0x157 : 0x156);
  sbwr(0x1102, 0x100 + 2*led+1);
  sbwr(0x1102, 0x1FE);
  sbwr(0x1102, (color&2) ? 0x157 : 0x156);
  sbwr(0x1102, 2*led + 2);
}

void lk204_set_gpo(int gpo, int on) {
  if (gpo >= 0 && gpo < 8) {
    sbwr(0x1102, 0x1FE);
    sbwr(0x1102, on ? 0x157 : 0x156);
    sbwr(0x1102, gpo);
  }
}

unsigned xdig2num(unsigned char dig) {
  if (isdigit(dig)) return dig - '0';
  if (isxdigit(dig)) return tolower(dig) - 'a' + 10;
  return 0;
}

void lk204_seq(const char *str_in) {
  const unsigned char *str = (const unsigned char *)str_in;
  while (*str != '\0') {
    unsigned short val = *str++;
    if (val == '\n' || val == '\r') break;
    if (val == '\\') {
      switch (*str) {
        case '\n':
        case '\r':
        case 0:
          return;
        case 'r': val = '\r'; ++str; break;
        case 'n': val = '\n'; ++str; break;
        case 'x':
          val = 0;
          str++;
          if (isxdigit(*str)) {
            val = xdig2num(*str++);
            if (isxdigit(*str)) {
              val = val*16 + xdig2num(*str++);
            }
          }
          break;
        default: val = *str++; break;
      }
    }
    if (*str != '\0') val |= 0x100;
    sbwr(0x1102, val);
  }
}

void lk204_keypad_brightness(int brightness) {
  if (brightness < 0) lk204_seq("\xfe\x9b");
  else {
    if (brightness > 255) brightness = 255;
    sbwr(0x1102, 0x1FE);
    sbwr(0x1102, 0x19C);
    sbwr(0x1102, brightness);
  }
}

int main(int argc, char **argv) {
  if (load_subbus() == 0)
    nl_error(3, "Unable to load subbus library");
  lk204_seq("\\xfe\\x58Stand By...\\n");
  sleep(1);
  lk204_seq(
    "\\xfe\\x40                    "
    " \\x00\\x01\\x02 Anderson Group "
    " \\x03\\x04\\x05 Harvard SEAS   "
    "  \\x06                 " );
  sleep(5);
  lk204_seq("\\xfe\\x58\n"
    " \\x00\\x01\\x02 Done\n"
    " \\x03\\x04\\x05\n"
    "  \\x06");
  return 0;
}

