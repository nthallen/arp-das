#ifndef MKSFLOW_INT_H_INCLUDED
#define MKSFLOW_INT_H_INCLUDED
#include <list>
#include <stdint.h>
#include "SerSelector.h"
#include "mksflow.h"

extern mksflow_t mksflow;
int get_addr_index(uint8_t address);

extern const char *MKS_Ser_path;
extern bool rs485_echos;
extern const char *address_opts;
extern const char *MKS_Name;
extern int n_drives;
#define MKS_MIN_ADDRESS 1
#define MKS_MAX_ADDRESS 254
#define MNEM_LEN 10

typedef struct {
  uint16_t device_index;
  uint16_t device_address;
  char     manufacturer[4]; // MF (always MKS) bit 0
  char     model[40];      // MD bit 1
  char     serial_number[20]; // SN bit 2
  char     device_type[4]; // DT string query bit 3
  char     gas_number[4];  // SGN integer query. Validated against GN output bit 4
  char     gas_search[80]; // GN string query bit 5
  char     gas_name[20];   // parsed from GN output
  char     full_scale[10]; // parsed from GN output
  char     gas_units[8];   // parsed from GN output
  char     valve_type[20]; // VT (only if device_type is MFC) bit 6
  char     valve_power_off_state[20]; // VPO (only if device_type is MFC) bit 7
  char     mnemonic[10];
  // uint8_t  ACK;
  bool     is_mfc;
  bool     is_polling;
} board_id_t;
extern board_id_t board_id[MKS_MAX_DRIVES];

class MKS_Ser;

class MKS_Query {
  friend class MKS_Ser;
  public:
    MKS_Query();
    ~MKS_Query();
    void init();
    /**
     * @param cmdlen If not NULL, length of the command string is stored
     * Increments the global sequence number, inserts it into the previously
     * formatted command string, then recalculates the CRC.
     * @return The command string to be written to the device
     */
    const char *get_cmd(int *cmdlen);
    // const char *get_raw_cmd();
    // enum MKSParType { MKS_ACK, MKS_INT32, MKS_FLOAT32 };
    void setup_query(uint8_t address, const char *req, void *dest, int dsize, uint8_t *ack, uint8_t bit);
    void set_persistent(bool persistent);
    void set_callback(void (*callback)(MKS_Query *, const char *rep));
    inline void set_caption(const char *cap) { caption = cap; }
    void set_bit();
    void clear_bit();
    void store_string(char *dest, const char *rep);
    inline const char *get_caption() { return caption ? caption : ""; }
    inline int get_index() { return index; }
    inline uint8_t get_address() { return address; }
    inline MKS_Ser * get_ser() { return ser; }
    inline void *get_ret_ptr() { return ret_ptr; }
    inline const char *get_cmd() { return cmd; }
  protected:
    /** true if query lives on the TM_queue, false if it is from
     * the Cmd_queue and should be removed and recycled onto the
     * Free_queue. Defaults to false.
     */
    bool persistent;
    // MKSParType ret_type;
    /** Where the result string should be written */
    void *ret_ptr;
    /** The size of the return string buffer */
    int ret_len;
    /**
     * mask_ptr points to a bit-mapped register for tracking queries and commands.
     * The 0 bit will be set after each time data is sent to telemetry. Each query
     * or command
     * will be mapped to an individual bit in the register. That bit is set when the
     * query is issue and clear when the query is acknowledged. After acknowledgement,
     * if all bits but the 0 bit are zero, the 0 bit will be cleared.
     * Before data is sent to telemetry, if the 0 bit is set, the corresponding Stale
     * value will be incremented.
     */
    uint8_t *mask_ptr;
    uint8_t mask_bit;
    void (*callback)(MKS_Query*, const char *rep);
    /* Store these in case it's useful for debugging messages */
    /** The device index */
    int index;
    /** The RS-485 device ID */
    uint8_t address;
    // uint16_t req_crc;
    // uint16_t MKSParID;
    // uint16_t SeqNr;
    MKS_Ser *ser;
    int replen;
    const char *caption;
  private:
    void setup_address(uint8_t address, uint16_t MKSParID);
    /*
     * Fill in a command field with a hex-encoded value.
     * @param val the value to be encoded
     * @param width the number of characters that should suffice
     * @param cp the starting position in the cmd[] array
     */
    // void to_hex(uint32_t val, int width, int cp);
    static const int max_command_length = 40;
    char cmd[max_command_length];
    int cmdlen;
    // static uint16_t Sequence_Number;
    // bool crc_applied;
};

void cb_valve_type(MKS_Query *, const char *rep);
void cb_device_type(MKS_Query *, const char *rep);
void cb_gas_search(MKS_Query *, const char *rep);
void cb_gas_number(MKS_Query *, const char *rep);
void cb_float(MKS_Query *, const char *rep);
void cb_report(MKS_Query *, const char *rep);
void cb_status(MKS_Query *, const char *rep);
void cb_cmd_v(MKS_Query *, const char *rep);
void cb_float_v(MKS_Query *, const char *rep);

class MKS_Ser : public Ser_Sel {
  friend class MKS_Query;
  public:
    MKS_Ser(const char *path);
    void enqueue_request(MKS_Query *req);
    MKS_Query *new_query();
    inline Timeout *GetTimeout() { return &TO; }
  protected:
    int ProcessData(int flags);
    bool protocol_input();
    bool protocol_timeout();
    bool tm_sync();
    void free_pending();
    void process_requests();
    int not_hex(uint32_t &hex32, int width);
    bool checksum_verify(int from, int to, int checksum);
    /** Clean up after a parsing error.
     * @return false
     */
    bool saw_error();
    
    Timeout TO;
    MKS_Query *pending;
    const char *pending_cmd;
    int pending_cmdlen;
    int pending_replen;
    std::list<MKS_Query*> Transient_queue;
    std::list<MKS_Query*> TM_queue;
    std::list<MKS_Query*> Free_queue;
    std::list<MKS_Query*>::const_iterator cur_poll;
    bool all_polling;
    int retry_delay;
};

void identify_board(MKS_Ser *ser, int index, uint8_t address);

class MKS_Cmd : public Cmd_Selectee {
  public:
    MKS_Cmd(MKS_Ser *ser);
  protected:
    int ProcessData(int flag);
    bool app_input();
    int not_hex(uint32_t &hex32);
    bool not_uint16(uint16_t &output_val);
    bool not_uint8(uint8_t &val);
    bool not_any(const char *alternatives);
    MKS_Ser *ser;
};

class MKS_TM_Selectee : public TM_Selectee {
  public:
    inline MKS_TM_Selectee(const char *name) :
        TM_Selectee(name, &mksflow, sizeof(mksflow)) {}
    int ProcessData(int flag);
};

#endif
