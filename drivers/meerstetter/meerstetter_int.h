#ifndef MEERSTETTER_INT_H_INCLUDED
#define MEERSTETTER_INT_H_INCLUDED
#include <list>
#include <stdint.h>
#include "SerSelector.h"
#include "meerstetter.h"

extern meerstetter_t meerstetter;
int get_addr_index(uint8_t address);

extern const char *Me_Ser_path;
extern bool rs485_echos;
extern const char *address_opts;
extern const char *Me_Name;
#define ME_MIN_ADDRESS 1
#define ME_MAX_ADDRESS 254

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
    enum MeParType { Me_ACK, Me_INT32, Me_FLOAT32 };
    void setup_int32_query(uint8_t address, uint16_t MeParID, int32_t *ret_ptr,
        uint16_t *mask_ptr = 0, uint16_t mask_bit = 0);
    void setup_float32_query(uint8_t address, uint16_t MeParID, float *ret_ptr,
        uint16_t *mask_ptr = 0, uint16_t mask_bit = 0);
    void setup_uint32_cmd(uint8_t address, uint16_t MeParID, uint32_t value,
        uint16_t *mask_ptr = 0, uint16_t mask_bit = 0);
    void set_persistent(bool persistent);
    void set_callback(void (*callback)(Me_Query *));
    inline uint8_t get_address() { return address; }
    void set_bit();
    void clear_bit();
  protected:
    /** true if query lives on the TM_queue, false if it is from
     * the Cmd_queue and should be removed and recycled onto the
     * Free_queue. Defaults to false.
     */
    bool persistent;
    MeParType ret_type;
    void *ret_ptr;
    /**
     * mask_ptr points to a bit-mapped register for tracking queries and commands.
     * The 0 bit will be set after each time data is sent to telemetry. Each query or command
     * will be mapped to an individual bit in the register. That bit is set when the
     * query is issue and clear when the query is acknowledged. After acknowledgement,
     * if all bits but the 0 bit are zero, the 0 bit will be cleared.
     * Before data is sent to telemetry, if the 0 bit is set, the corresponding Stale
     * value will be incremented.
     */
    uint16_t *mask_ptr;
    uint16_t mask_bit;
    void (*callback)(Me_Query*);
    /* Store these in case it's useful for debugging messages */
    /** The RS-485 device ID */
    uint8_t address;
    uint16_t req_crc;
    /** The Meerstetter parameter ID (i.e. register address) */
    uint16_t MeParID;
    uint16_t SeqNr;
    int replen;
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
    bool crc_applied;
};

class Me_Ser : public Ser_Sel {
  public:
    Me_Ser(const char *path);
    void enqueue_request(Me_Query *req);
    Me_Query *new_query();
    inline Timeout *GetTimeout() { return &TO; }
  protected:
    int ProcessData(int flags);
    bool protocol_input();
    bool protocol_timeout();
    bool tm_sync();
    void free_pending();
    void process_requests();
    int not_hex(uint32_t &hex32, int width);
    void set_RTS(bool RTS);
    Timeout TO;
    Me_Query *pending;
    const char *pending_cmd;
    int pending_cmdlen;
    int pending_replen;
    std::list<Me_Query*> Transient_queue;
    std::list<Me_Query*> TM_queue;
    std::list<Me_Query*> Free_queue;
    std::list<Me_Query*>::const_iterator cur_poll;
};

class Me_Cmd : public Cmd_Selectee {
  public:
    Me_Cmd(Me_Ser *ser);
  protected:
    int ProcessData(int flag);
    bool app_input();
    int not_hex(uint32_t &hex32);
    bool not_uint16(uint16_t &output_val);
    bool not_uint8(uint8_t &val);
    bool not_any(const char *alternatives);
    Me_Ser *ser;
};

class Me_TM_Selectee : public TM_Selectee {
  public:
    inline Me_TM_Selectee(const char *name) :
        TM_Selectee(name, &meerstetter, sizeof(meerstetter)) {}
    int ProcessData(int flag);
};

#endif
