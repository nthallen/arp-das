#ifndef QCLI_H_INCLUDED
#define QCLI_H_INCLUDED 1

#define ANAIO_BASE 0xC00
#define ANAIO_INC 0x80
#define ANAIO_STATUS_OFFSET 6
#define ANAIO_DAC_OFFSET 0x60
#define ANAIO_HRDAC_OFFSET 0x70
#define ANAIO_HRADC_OFFSET 0x78
#define DAC_MODE_BITS 0x0C
#define QCLI_BASE 0x1000
#define QCLI_INC  0x0010

#define QCLI_S_BUSY     0x8000
#define QCLI_S_CHKSUM   0x4000
#define QCLI_S_CMDERR   0x2000
#define QCLI_S_LASEROFF 0x1000
#define QCLI_S_CORDTE   0x0800
#define QCLI_S_READY    0x0200
#define QCLI_S_WAVEERR  0x0100
#define QCLI_S_FLSHDATA 0x0080
#define QCLI_S_FLSHTGL  0x0040
#define QCLI_S_DOT      0x0020
#define QCLI_S_LOT      0x0010
#define QCLI_S_LOC      0x0008
#define QCLI_S_MODE     0x0007
#define QCLI_S_HWERR (QCLI_S_DOT|QCLI_S_LOT|QCLI_S_LOC)
#define QCLI_S_FWERR (QCLI_S_CMDERR|QCLI_S_CORDTE)
#define QCLI_S_ERR (QCLI_S_HWERR|QCLI_S_FWERR)
#define QCLI_IDLE_MODE 0
#define QCLI_PROGRAM_MODE 1
#define QCLI_PSECTOR_MODE 2
#define QCLI_RUN_MODE 3
#define QCLI_SELECT_MODE 4

#define QCLI_NOOP 0x0000
#define QCLI_STOP 0x0100
#define QCLI_LOAD_MSB 0x0200
#define QCLI_WRITE_ADDRESS 0x0300
#define QCLI_WRITE_DATA 0x0400
#define QCLI_READ_DATA  0x0500
#define QCLI_WRITE_CHKSUM  0x0600
#define QCLI_PROGRAM_SECTOR  0x0700
#define QCLI_CLEAR_ERROR 0x0800
#define QCLI_SELECT_WAVEFORM 0x0900
#define QCLI_RUN_WAVEFORM 0x0A00
#define QCLI_SELECT_DAC 0x0B00
#define QCLI_WRITE_DAC 0x0C00
#define QCLI_WRITE_TON 0x0D00
#define QCLI_WRITE_TOFF 0x0E00
#define QCLI_WRITE_TPRE 0x0F00
#define QCLI_BAD_CMD 0x1000

void report_status( unsigned short status );
int check_status( unsigned short status, unsigned short mask,
		unsigned short value, char *text, int dump );
unsigned short read_qcli( int fresh );
void write_qcli( unsigned short value );
unsigned short wr_rd_qcli( unsigned short value );
void wr_stop_qcli( unsigned short value );
int qcli_diags( int verbose );
void qclisrvr_init( int argc, char **argv );
void delay3msec(void);

#endif
