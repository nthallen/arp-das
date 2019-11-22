#include <stdlib.h>
#include <string.h>
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

board_id_t board_id[MKS_MAX_DRIVES];

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
     (mksflow.drive[i].ACK & 0x1) ?
       ((mksflow.drive[i].Stale < 255) ?
        (mksflow.drive[i].Stale+1) : 255)
       : 0;
  }
  Col_send(TMid);
  for (i = 0; i < n_drives; ++i) {
    mksflow.drive[i].ACK = 0x1;
  }
  Stor->set_gflag(0);
  return 0;
}

void report_board_id(MKS_Query *Q) {
  uint8_t address = Q->get_address();
  uint16_t index;
  
  index = Q->get_index();
  if (index < 0) {
    msg(MSG_ERROR, "report_board_id(): Invalid index from query");
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
  if (bdp->is_mfc) {
    msg(0, "Valve type: %s", bdp->valve_type);
    msg(0, "Valve power off state: %s", bdp->valve_power_off_state);
  }
}

void identify_board(MKS_Ser *ser, int index, uint8_t address) {
  nl_assert(index >= 0 && index < n_drives);
  board_id_t *bdp = &board_id[index];
  uint8_t *ACK2 = &mksflow.drive[index].ACK2;
  MKS_Query *Q = ser->new_query();
  Q->setup_query(address, "MF?", bdp->manufacturer, 4, ACK2, 0x01);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_query(address, "MD?", bdp->model, 40, ACK2, 0x02);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_query(address, "SN?", bdp->serial_number, 20, ACK2, 0x04);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_query(address, "DT?", 0, 4, ACK2, 0x08);
  Q->set_callback(cb_device_type);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_query(address, "SGN?", 0, 4, ACK2, 0x10);
  Q->set_callback(cb_gas_number);
  ser->enqueue_request(Q);
}

void poll_board(MKS_Ser *ser, int index, uint8_t address) {
  nl_assert(index >= 0 && index < n_drives);
  mks_drive_t *mksdp = &mksflow.drive[index];
  board_id_t *bdp = &board_id[index];
  if (!bdp->is_polling) {
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

    Q = ser->new_query();
    Q->setup_query(address, "T?", &mksdp->DeviceStatus, 0, &mksdp->ACK, 0x04);
    Q->set_callback(cb_status);
    Q->set_persistent(true);
    ser->enqueue_request(Q);

    if (bdp->is_mfc) {
      Q = ser->new_query();
      Q->setup_query(address, "SX?", &mksdp->FlowSetPoint, 0, &mksdp->ACK, 0x08);
      Q->set_callback(cb_float);
      Q->set_persistent(true);
      ser->enqueue_request(Q);
    }
    bdp->is_polling = true;
  }
}

void cb_valve_type(MKS_Query *Q, const char *rep) {
  Q->store_string(0, rep);
  board_id_t *bdp = &board_id[Q->get_index()];
  msg(0, "%s: Valve Type:%s Power Off State:%s",
    bdp->valve_type, bdp->valve_power_off_state);
}

/**
 * Callback for the DT query. The ret_ptr must point to the board_id_t
 * for the specific device, since we need to access other fields in that
 * structure.
 */
void cb_device_type(MKS_Query *Q, const char *rep) {
  board_id_t *bdp = &board_id[Q->get_index()];
  uint8_t *ACK2 = &mksflow.drive[index].ACK2;
  Q->store_string(bdp->device_type, rep);
  msg(0,"%s: Addr:%d Mfg:%s Mdl:%s SN:%s Type:%s", bdp->mnemonic, bdp->device_address,
    bdp->manufacturer, bdp->model,
    bdp->serial_number, bdp->device_type);
  if (strcmp(bdp->device_type,"MFC")) {
    bdp->is_mfc = true;
    MKS_Ser *ser = Q->get_ser();
    MKS_Query *Q1 = ser->new_query();
    Q1->setup_query(Q->get_address(), "VT?", bdp->valve_type, 20,
      ACK2, 0x40);
    ser->enqueue_request(Q1);
    Q1 = ser->new_query();
    Q1->setup_query(Q->get_address(), "VPO?",
      bdp->valve_power_off_state, 20, ACK2, 0x80);
    Q1->set_callback(cb_valve_type);
    ser->enqueue_request(Q1);
  }
}

void cb_report(MKS_Query *Q, const char *rep) {
  int index = Q->get_index();
  nl_assert(index >= 0);
  msg(0, "%s: %s %s", board_id[index].mnemonic, rep, Q->get_caption());
}

void extract_csv(const char *src, int &si, int src_sz,
                 char *dest, int dest_sz) {
  int j;
  if (src[si] == '\0') {
    msg(MSG_ERROR,
      "Unexpected end of string in extract_csv: '%s'",
      ascii_escape(src));
    dest[0] = '\0';
    return;
  }
  for (j = 0; j < dest_sz && si < src_sz; ++j, ++si) {
    if (src[si] == ',' || src[si] == '\0') {
      dest[j] = '\0';
      if (src[si] != '\0')
        ++j;
      return;
    }
  }
  if (j >= dest_sz) {
    msg(MSG_ERROR, "Parsed string too long: '%s'",
      ascii_escape(src));
    dest[dest_sz-1] = '\0';
  } else {
    dest[j] = '\0';
  }
  if (si >= src_sz) {
    msg(MSG_ERROR,
      "Unterminated source string in extract_csv");
  }
}

void cb_gas_search(MKS_Query *Q, const char *rep) {
  board_id_t *bdp = &board_id[Q->get_index()];
  Q->store_string(bdp->gas_search, rep);
  char *gs = bdp->gas_search;
  int si = 0;
  extract_csv(gs, si, 80, bdp->gas_name, 20);
  char gas_num[4];
  extract_csv(gs, si, 80, gas_num, 4);
  extract_csv(gs, si, 80, bdp->full_scale, 10);
  extract_csv(gs, si, 80, bdp->gas_units, 8);
  if (strcmp(gas_num, bdp->gas_number) != 0) {
    msg(MSG_ERROR,
      "Search returned wrong gas number: "
      "expected '%s' recd '%s'",
      bdp->gas_number, gas_num);
  }
  msg(0, "%s: %s %s %s", bdp->mnemonic, bdp->gas_name,
    bdp->full_scale, bdp->gas_units);
  poll_board(Q->get_ser(), bdp->device_index,
    bdp->device_address);
}

void cb_gas_number(MKS_Query *Q, const char *rep) {
  board_id_t *bdp = &board_id[Q->get_index()];
  uint8_t *ACK2 = &mksflow.drive[index].ACK2;
  Q->store_string(bdp->gas_number, rep);
  MKS_Ser *ser = Q->get_ser();
  MKS_Query *Q1 = ser->new_query();
  char req[10];
  snprintf(req, 10, "GN?%s", bdp->gas_number);
  Q1->setup_query(Q->get_address(), req, bdp, 80, ACK2, 0x20);
  Q->set_callback(cb_gas_search);
  ser->enqueue_request(Q1);
}

void cb_float(MKS_Query *Q, const char *rep) {
  float *fltval = (float*)Q->get_ret_ptr();
  char *endptr = 0;
  *fltval = strtof(rep, &endptr);
  if (endptr && *endptr != '\0') {
    board_id_t *bdp = &board_id[Q->get_index()];
    msg(MSG_ERROR, "%s: cb_float() input not float: '%s'",
      bdp->mnemonic, ascii_escape(rep));
  }
}

#define CB_STATUS_MAX_ERRORS 5

void cb_status_error(MKS_Query *Q, const char *rep, const char *p) {
  static int cb_status_n_errors = 0;
  if (cb_status_n_errors < CB_STATUS_MAX_ERRORS) {
    if (++cb_status_n_errors == CB_STATUS_MAX_ERRORS) {
      msg(MSG_ERROR, "%s: status syntax error messages suppressed",
        board_id[Q->get_index()].mnemonic);
    } else {
      msg(MSG_ERROR, "%s: status syntax in '%s' at '%s'",
        board_id[Q->get_index()].mnemonic, rep, p);
    }
  }
}

void cb_status(MKS_Query *Q, const char *rep) {
  uint16_t *status = (uint16_t*)Q->get_ret_ptr();
  const char *p = rep;
  *status = 0;
  while (*p != '\0') {
    switch (*p) {
      case 'C':
        if (p[1] == 'R') {
          ++p; // advance for two-letter code
          *status |= MKS_STAT_CR; // CR = Calibration recommended
        } else {
          *status |= MKS_STAT_C; // C = Valve closed
        }
        break;
      case 'E':
        *status |= MKS_STAT_E; break; // E = System error
      case 'H':
        if (p[1] == 'H') {
          ++p;
          *status |= MKS_STAT_HH; // HH = High-high alarm condition
        } else {
          *status |= MKS_STAT_H; // H = High alarm condition
        }
        break;
      case 'L':
        if (p[1] == 'L') {
          ++p;
          *status |= MKS_STAT_LL; // LL = Low-low alarm condition
        } else {
          *status |= MKS_STAT_L; // L = Low alarm condition
        }
        break;
      case 'M':
        *status |= MKS_STAT_M; break; // M = Memory (EEPROM) failure
      case 'O':
        if (p[1] == 'C') {
          ++p;
          *status |= MKS_STAT_OC; // OC = Unexpected change in operating
        }
        break; // O = OK, no errors to report
      case 'P':
        *status |= MKS_STAT_P; break; // P = Purge
      case 'T':
        *status |= MKS_STAT_T; break; // T = Over temperature
      case 'U':
        *status |= MKS_STAT_U; break; // U = Uncalibrated
      case 'V':
        *status |= MKS_STAT_V; break; // V = Valve drive level alarm condition 
      case 'I':
        if (p[1] == 'P') {
          ++p;
          *status |= MKS_STAT_IP; // IP = Insufficient gas inlet pressure
          break;
        } // else fall through for syntax complaint
      default:
        cb_status_error(Q, rep, p);
        return;
    }
    ++p;
    if (*p == ',') {
      ++p;
    } else if (*p != '\0') {
      cb_status_error(Q, rep, p);
      return;
    }
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
    if (address < MKS_MIN_ADDRESS || address > MKS_MAX_ADDRESS) {
      msg(MSG_FATAL,"Invalid device address %d in option string '%s'",
        address, address_opts);
    }
    if (++n_drives > MKS_MAX_DRIVES) {
      msg(MSG_FATAL, "Address string exceeds MKS_MAX_DRIVES value of %d",
        MKS_MAX_DRIVES);
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
    else if (*s == '\0') break;
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
  Ser.setup(9600, 8, 'n', 1, 1, 1);
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

