/* sic.h defines the pertinent addresses used by the sic card as well as
   some important constant definitions for data stored in the non-volatile
   RAM and for the hardware switches.
   Written April 27, 1987
*/

/* These are the addresses */
#define SIC_RES_CNT 0x31A
#define SIC_PWR_CNT 0x31B
#define SIC_ADDR_LATCH 0x31C
#define SIC_NOVRAM 0x31D
#define SIC_SWITCHES 0x31E
#define SIC_LAMP 0x31F

/* These are the switch masks - all of the switches are true in low */
#define SW_POWER_GOOD 1
#define SW_GROUND_POWER 2
#define SW_DUMP_DATA 4
#define SW_DIAGNOSTIC 8
#define SW_MODE 0x80

/* These are the status values for NOVRAM address 0 - the main status word
   Additional status values can be added at any time.
	SIC_READY	default start-up code before a flight.
	SIC_INIT	Initialization has begun.
	SIC_RUNNING	code written to the sic when the flight program is
			started.  If read on power-up, this code indicates
			an unanticipated crash occurred - bad news.
	SIC_PFAIL_DET	This code is written out to the sic immediately on
			detecting a low power situation.  This is followed
			by emergency shut down procedures (all the off
			commands at once!).  Reading this on power up
			indicates that we saw it coming, but didn't have time
			to do much about it.
	SIC_PFAIL_OK	If after low power is detected, shut down is
			completed before losing it totally, this code is
			written out to indicate that the instrument has been
			secured for power failure.  Reading this on power up
			indicates we successfully responded to a low power
			signal.
	SIC_FLT_OVER	After the flight sequence is completed, this code
			is written out to the SIC.  If read on power up, no
			action should be taken unless the external switches
			are positioned for a tape dump.
	SIC_FLT_OVER2	This code is written after grndctrl regains control
			after the flight algorithm is terminated.
	SIC_DUMPED	After the data has been written to the tape, we write
			this code.  If read on power up, no action should be
			taken.
	SIC_PWR_CYC_REQ	The software has requested a power cycle by lighting
			the failure lamp.  On power up, the situation should
			be diagnosed.  If the situation has not been
			remedied, a failure should be declared.
	SIC_FAILURE	The software has detected an unrecovereable system
			failure.  On power up, the system failure lamp should
			be lit and no further action should be undertaken. 
			The cause of the failure should be recorded in the
			power log file.
	SIC_EARLY	The flight algorithm was terminated early.  This is
			the same as FLT_OVER except in the case that the
			mode switch reads mode 1.  In that case, the command
			count register should be zeroed and the algorithm
			should be restarted.
*/
#define SIC_READY 0
#define SIC_INIT 1
#define SIC_RUNNING 2
#define SIC_PFAIL_DET 3
#define SIC_PFAIL_OK 4
#define SIC_FLT_OVER 5
#define SIC_DUMPED 6
#define SIC_PWR_CYC_REQ 7
#define SIC_FAILURE 8
#define SIC_EARLY 9
#define SIC_FLT_OVER2 10


