#ifndef MEERSTETTER_INT_H_INCLUDED
#define MEERSTETTER_INT_H_INCLUDED
#include <list>
#include <stdint.h>
#include "SerSelector.h"
/*
 * Cmd client
 * TM client
 * Serial client
 */
class Me_Ser;

class Me_Query {
  friend class Me_Ser;
  public:
    Me_Query();
    ~Me_Query();
    void init();
    /**
     * @param cmdlen If not NULL, length of the command string is stored
     * Increments the global sequence number, inserts it into the previously
     * formatted command string, then recalculates the CRC.
     * @return The command string to be written to the device
     */
    const char *get_cmd(int *cmdlen);
    const char *get_raw_cmd();
    typedef enum { Me_ACK, Me_INT32, Me_FLOAT32 } MeParType;
    void setup_int32_query(uint8_t address, uint16_t MeParID, int32_t *ret_ptr);
    void setup_float32_query(uint8_t address, uint16_t MeParID, float *ret_ptr);
    void setup_uint32_cmd(uint8_t address, uint16_t MeParID, uint32_t value);
    void set_persistent(bool persistent);
    // inline bool get_persistent() { return persistent; }
    // inline uint8_t get_address() { return address; }
    // inline uint16_t get_MeParID() { return MeParID; }
    // inline MeParType get_MeParType() { return MeParType; }
  protected:
    /** true if query lives on the TM_queue, false if it is from
     * the Cmd_queue and should be removed and recycled onto the
     * Free_queue.
     */
    bool persistent;
    MeParType ret_type;
    void *ret_ptr;
    /* Store these in case it's useful for debugging messages */
    uint8_t address;
    uint16_t req_crc;
    uint16_t MeParID;
    uint16_t SeqNr;
  private:
    void setup_address(uint8_t address, uint16_t MeParID);
    /**
     * Fill in a command field with a hex-encoded value.
     * @param val the value to be encoded
     * @param width the number of characters that should suffice
     * @param cp the starting position in the cmd[] array
     */
    void to_hex(uint32_t val, int width, int cp);
    static const int max_command_length = 40;
    char cmd[max_command_length];
    int cmdlen;
    static uint16_t Sequence_Number;
};

class Me_Ser : public Ser_Selectee {
	public:
		Me_Ser();
    void enqueue_request(Me_Query *req, bool persistent);
    Me_Query *new_query();
	protected:
		bool ProcessData(int flags);
		bool protocol_input();
		bool protocol_timeout();
    bool tm_sync();
    void process_requests();
    Timeout TO;
    Me_Query *pending;
    std::list<Me_Query*> Transient_queue;
    std::list<Me_Query*> TM_queue;
    std::list<Me_Query*> Free_queue;
    std::list<Me_Query*>::const_iterator cur_poll;
};

class Me_Cmd : public Cmd_Selectee {
	public:
		Me_Cmd(Me_Ser *ser);
	protected:
		bool ProcessData(int flags);
		bool app_input();
    int not_hex(uint32_t &hex32);
    bool not_any(const char *alternatives);
    Me_Ser *ser;
};

#endif
