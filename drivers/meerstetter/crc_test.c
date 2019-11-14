#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "crc_test.h"

// The used standard is: CRC-CCITT (CRC-16)
void CRC16Algorithm(unsigned short *CRC, unsigned char Ch) {
  unsigned int genPoly = 0x1021; //CCITT CRC-16 Polynominal
  unsigned int uiCharShifted = ((unsigned int)Ch & 0x00FF) << 8;
  *CRC = *CRC ^ uiCharShifted;
  for (int i = 0; i < 8; i++)
    if ( *CRC & 0x8000 ) *CRC = (*CRC << 1) ^ genPoly;
    else *CRC = *CRC << 1;
  *CRC &= 0xFFFF;
}

unsigned short CRC16wrapper(unsigned short init, const uint8_t *mem, size_t len) {
  unsigned short CRC = init;
  if (mem == 0) len =0;
  
  for (int i = 0; i < len; ++i) {
    CRC16Algorithm(&CRC, mem[i]);
  }
  return CRC;
}

int main(void) {
    unsigned char data[31];
    srandom(time(NULL));
    {
        uint64_t ran = 1;
        size_t n = sizeof(data);
        do {
            if (ran < 0x100)
                ran = (ran << 31) + random();
            data[--n] = ran;
            ran >>= 8;
        } while (n);
    }
    uintmax_t init, crc;

    // crc16xmodem
    init = crc16xmodem_bit(0, NULL, 0);
    if (crc16xmodem_bit(init, "123456789", 9) != 0x31c3)
        fputs("bit-wise mismatch for crc16xmodem\n", stderr);
    crc = crc16xmodem_bit(init, data + 1, sizeof(data) - 1);
    if (crc16xmodem_bit(init, "\xda", 1) !=
        crc16xmodem_rem(crc16xmodem_rem(init, 0xda, 3), 0xd0, 5))
        fputs("small bits mismatch for crc16xmodem\n", stderr);
    if (crc16xmodem_byte(0, NULL, 0) != init ||
        crc16xmodem_byte(init, "123456789", 9) != 0x31c3 ||
        crc16xmodem_byte(init, data + 1, sizeof(data) - 1) != crc)
        fputs("byte-wise mismatch for crc16xmodem\n", stderr);
    if (crc16xmodem_word(0, NULL, 0) != init ||
        crc16xmodem_word(init, "123456789", 9) != 0x31c3 ||
        crc16xmodem_word(init, data + 1, sizeof(data) - 1) != crc)
        fputs("word-wise mismatch for crc16xmodem\n", stderr);

    uintmax_t MEinit, MEcrc1, MEcrc2;
    MEinit = CRC16wrapper(0, NULL, 0);
    MEcrc1 = CRC16wrapper(MEinit, (const uint8_t *)"123456789", 9);
    MEcrc2 = CRC16wrapper(MEinit, (const uint8_t *)(data + 1), sizeof(data) - 1);
    const char *algo = "XMODEM";
    uint16_t short_crc = 0x31c3;
    
    if (MEinit != init)
        printf("init from CRC16wrapper is 0x%04lX, from %s it is %04lX\n", MEinit, algo, init);
    if (MEcrc1 != short_crc)
        printf("1: CRC16wrapper returned 0x%04lX, %s returned 0x%04X\n",
          MEcrc1, algo, short_crc);
    if (MEcrc2 != crc)
        printf("2: CRC16wrapper returned 0x%04lX, %s returned 0x%04lX\n",
          MEcrc2, algo, crc);

    return 0;
}
