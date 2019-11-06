#include "mksflow_int.h"
#include "mksflow.h"
#include "msg.h"
#include "oui.h"
#include "nl_assert.h"

const char *MKS_Ser_path = "/dev/ser1";
const char *address_opts = "1";
const char *MKS_Name = "MKS";
static int n_drives;
mksflow_t mksflow;

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
  char     mnemonic[MNEM_LEN];
  uint8_t  ACK;
  bool     is_mfc;
  bool     is_polling;
} board_id_t;
board_id_t board_id[ME_MAX_DRIVES];

int get_addr_index(uint8_t address) {
  for (int i = 0; i < n_drives; ++i) {
    if (board_id[i].device_address == address)
      return(i);
  }
  return -1;
}

int MKS_TM_Selectee::ProcessData(int flag) {
  int i;
  for (i = 0; i < n_drives; ++i) {
    mksflow.drive[i].Stale =
     (mksflow.drive[i].Mask & 0x1) ?
       ((mksflow.drive[i].Stale < 255) ?
        (mksflow.drive[i].Stale+1) : 255)
       : 0;
  }
  Col_send(TMid);
  for (i = 0; i < n_drives; ++i) {
    mksflow.drive[i].Mask = 0x1;
  }
  Stor->set_gflag(0);
  return 0;
}

void report_board_id(MKS_Query *Q) {
  uint8_t address = Q->get_address();
  uint16_t index;
  
  index = get_addr_index(address);
  if (index < 0) {
    msg(MSG_ERROR, "Invalid address %d in report_board_id", address);
    return;
  }
  board_id_t *bdp = &board_id[index];
  msg(0,
    "Addr:%d:%s %s %s Model:%s SN:%s",
    address, bdp->mnemonic,
    bdp->manufacturer, bdp->device_type, bdp->model, bdp->serial_number);
  msg(0,
    "Gas:%d:%s Full Scale:%.1f %s",
    bdp->gas_number, bdp->gas_name, bdp->full_scale, bdp->gas_units);
  if (is_mfc) {
    msg(0, "Valve type: %s", bdp->valve_type);
    msg(0, "Valve power off state: %s", bdp->valve_power_off_state);
  }
}

void cb_valve_type(MKS_Query *Q, const char *rep) {
  board_id_t *bdp = (board_id_t*)Q->ret_ptr;
  Q->store_string(bdp->valve_type, rep);
}

/**
 * Callback for the DT query. The ret_ptr must point to the board_id_t
 * for the specific device, since we need to access other fields in that
 * structure.
 */
void cb_device_type(MKS_Query *Q, const char *rep) {
  board_id_t *bdp = (board_id_t*)Q->ret_ptr;
  Q->store_string(bdp->device_type, rep);
  // strncpy(bdp->device_type, rep, 4);
  // if (bdp->device_type[3]) {
    // msg(1, "DT response was longer than 3 chars: '%s'", rep);
    // bdp->device_type[3] = '\0';
  // }
  msg(0,"%s: Addr:%d Mfg:%s Mdl:%s SN:%s Type:%s", bdp->mnemonic, bdp->device_address,
    bdp->manufacturer, bdp->model,
    bdp->serial_number, bdp->device_type);
  if (strcmp(bdp->device_type,"MFC")) {
    bdp->is_mfc = true;
    MKS_Ser *ser = Q->get_ser();
    MKS_Query *Q1 = ser->new_query();
    Q1->setup_query(Q->address, "VT", bdp->valve_type, 20, &bdp->ACK, 0x40);
    ser->enqueue_request(Q1);
    Q1 = ser->new_query();
    Q1->setup_query(Q->address, "VT", bdp, 20, &bdp->ACK, 0x80);
    Q1->set_callback(cb_valve_type);
    ser->enqueue_request(Q1);
  }
}

void cb_report(MKS_Query *Q, const char *rep) {
  int index = get_addr_index(address);
  nl_assert(index >= 0);
  msg(0, "%s: %s %s", board_id[index].mnemonic, rep, Q->get_caption());
}

void extract_csv(const char *src, int &si, int src_sz, char *dest, int dest_sz) {
  if (src[si] == '\0') {
    msg(MSG_ERROR, "Unexpected end of string in extract_csv: '%s'", ascii_escape(src));
    dest[0] = '\0';
    return;
  }
  for (int j = 0; j < dest_sz && si < src_sz; ++j, ++si) {
    if (src[si] == ',' || src[si] == '\0') {
      dest[j] = '\0';
      if (src[si] != '\0')
        ++j;
      return;
    }
  }
  if (j >= dest_sz) {
    msg(MSG_ERROR, "Parsed string too long: '%s'", ascii_escape(src));
    dest[dest_sz-1] = '\0';
  } else {
    dest[j] = '\0';
  }
  if (si >= src_sz) {
    msg(MSG_ERROR), "Unterminated source string in extract_csv");
  }
}

void cb_gas_search(MKS_Query *Q, const char *rep) {
  board_id_t *bdp = (board_id_t*)Q->ret_ptr;
  Q->store_string(bdp->gas_search, rep);
  char *gs = bdp->gas_search;
  char *dest = bdp->gas_name;
  int si = 0;
  extract_csv(gs, si, 80, bdp->gas_name, 20);
  char gas_num[4];
  extract_csv(gs, si, 80, gas_num, 4);
  extract_csv(gs, si, 80, bdp->full_scale, 10);
  extract_csv(gs, si, 80, bdp->gas_units, 8);
  if (strcmp(gas_num, bdp->gas_number) != 0) {
    msg(MSG_ERROR,"Search returned wrong gas number: expected '%s' recd '%s'",
      bdp->gas_number, gas_num);
  }
  msg(0, "%s: %s %s %s", bdp->mnemonic, bdp->gas_name,
    bdp->full_scale, bdp->gas_units);
  poll_board(ser, bdp->device_index, bdp->device_address);
}

void cb_gas_number(MKS_Query *Q, const char *rep) {
  board_id_t *bdp = (board_id_t*)Q->ret_ptr;
  Q->store_string(bdp->gas_number, rep);
  MKS_Ser *ser = Q->get_ser();
  MKS_Query *Q1 = ser->new_query();
  char req[10];
  snprintf(req, 10, "GN?%s", bdp->gas_number);
  Q1->setup_query(Q->address, req, bdp, 80, &bdp->ACK, 0x20);
  Q->set_callback(cb_gas_search);
  ser->enqueue_request(Q1);
}

void cb_float(MKS_Query *Q, const char *rep) {
  float *fltval = (float*)Q->ret_ptr;
  char *endptr = 0;
  *fltval = strtof(rep, &endptr);
  if (endptr && *endptr != '\0')
    msg(MSG_ERROR, "%s: cb_float() input not float: '%s'", bdp->mnemonic, ascii_escape(rep));
}

void cp_caption(MKS_Query *Q, const char *rep) {
  board_id_t *bdp = (board_id_t*)Q->ret_ptr;
  msg(0, "%s: %s %s", bdp->mnemonic, rep, Q->get_caption());
}

void identify_board(MKS_Ser *ser, int index, uint8_t address) {
  nl_assert(index < n_drives);
  board_id_t *bdp = &board_id[index];
  MKS_Ser *ser = Q->get_ser();
  MKS_Query *Q = ser->new_query();
  Q->setup_query(address, "MF?", bdp->manufacturer, 4, &bdp->ACK, 0x01);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_query(address, "MD?", bdp->model, 40, &bdp->ACK, 0x02);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_query(address, "SN?", bdp->serial_number, 20, &bdp->ACK, 0x04);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_query(address, "DT?", bdp, 4, &bdp->ACK, 0x08);
  Q->set_callback(cb_device_type);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_query(address, "SGN?", bdp, 4, &bdp->ACK, 0x10);
  Q->set_callback(cb_gas_number);
  ser->enqueue_request(Q);
}

void poll_board(MKS_Ser *ser, int index, uint8_t address) {
  nl_assert(index < n_drives);
  mks_drive_t *mksdp = &mksflow.drive[index];
  if (!mksdp->is_polling) {
    board_id_t *bdp = &board_id[index];
    nl_assert(bdp->device_address == address);
    MKS_Query *Q = ser->new_query();
    Q->setup_query(address, "FX?", &mksdp->Flow, 0, &mksdp->ACK, 0x01);
    Q->set_callback(cb_float);
    Q->set_persistent(true);
    ser->enqueue_request(Q);

    Q = ser->new_query();
    Q->setup_query(address, "TA?", &mksdp->DeviceTemp, 0, &mksdp->ACK, 0x02);
    Q->set_callback(cb_float);
    Q->set_persistent(true);
    ser->enqueue_request(Q);

    if (bdp->is_mfc) {
      Q = ser->new_query();
      Q->setup_query(address, "SX?", &mksdp->DeviceTemp, 0, &mksdp->ACK, 0x02);
      Q->set_callback(cb_float);
      Q->set_persistent(true);
      ser->enqueue_request(Q);
    }
    mksdp->is_polling = true;
  }
}

void enqueue_requests(MKS_Ser *ser) {
  const char *s = address_opts;
  int index = 0;
  while (*s) {
    int address = 0;
    while (isdigit(*s)) {
      address = address*10 + (*s++) - '0';
    }
    if (address < ME_MIN_ADDRESS || address > ME_MAX_ADDRESS) {
      msg(MSG_FATAL,"Invalid device address %d in option string '%s'",
        address, address_opts);
    }
    if (++n_drives > ME_MAX_DRIVES) {
      msg(MSG_FATAL, "Address string exceeds ME_MAX_DRIVES value of %d",
        ME_MAX_DRIVES);
    }
    board_id[index].device_index = index;
    board_id[index].device_address = address;
    board_id[index].is_polling = false;
    board_id[index].is_mfc = false;
    if (*s == ':') {
      int i;
      ++s;
      for (i = 0; i < MNEM_LEN-1 && *s && isalnum(*s); ++i) {
        board_id[index].mnemonic[i] = *s++;
      }
      board_id[index].mnemonic[i] = '\0';
    } else board_id[index].mnemonic[0] = '\0';
    ++index;
    if (*s == ',') ++s;
    if (*s == '\0') break;
    else msg(MSG_FATAL,
      "Invalid character in address option string after %d address(es): '%s'",
      index, address_opts);
  }
  msg(0, "Addressing %d drives", n_drives);
  for (index = 0; index < n_drives; ++index)
    identify_board(ser, index, board_id[index].device_address);
  // for (index = 0; index < n_drives; ++index)
    // poll_board(ser, index, board_id[index].device_address);
}

int main(int argc, char **argv) {
  oui_init_options(argc, argv);
  Selector S;
  MKS_Ser Ser(MKS_Ser_path);
  Ser.setup(57600, 8, 'n', 1, 1, 1);
  MKS_Cmd Cmd(&Ser);
  MKS_TM_Selectee TM(MKS_Name);
  S.add_child(&Ser);
  S.add_child(&Cmd);
  S.add_child(&TM);
  msg(0, "Starting: V1.1");
  enqueue_requests(&Ser);
  S.event_loop();
  msg(0, "Terminating");
  return 0;
}

