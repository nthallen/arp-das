#include "meerstetter_int.h"
#include "meerstetter.h"
#include "msg.h"
#include "oui.h"
#include "nl_assert.h"

const char *Me_Ser_path = "/dev/ser1";
const char *address_opts = "1";
const char *Me_Name = "ME";
static int n_drives;
meerstetter_t meerstetter;

#define MNEM_LEN 10

typedef struct {
  uint16_t device_index;
  uint16_t device_address;
  int32_t device_type; // 100
  int32_t hw_version; // 101, fixed point %.2lf
  int32_t serial_num; // 102
  int32_t fw_version; // 103
  int32_t fw_build;   // 1051
  char mnemonic[MNEM_LEN];
} board_id_t;
board_id_t board_id[ME_MAX_DRIVES];

int get_addr_index(uint8_t address) {
  for (int i = 0; i < n_drives; ++i) {
    if (board_id[i].device_address == address)
      return(i);
  }
  return -1;
}

int Me_TM_Selectee::ProcessData(int flag) {
  int i;
  for (i = 0; i < n_drives; ++i) {
    meerstetter.drive[i].Stale =
     (meerstetter.drive[i].Mask & 0x1) ?
       ((meerstetter.drive[i].Stale < 255) ?
        (meerstetter.drive[i].Stale+1) : 255)
       : 0;
  }
  Col_send(TMid);
  for (i = 0; i < n_drives; ++i) {
    meerstetter.drive[i].Mask = 0x1;
  }
  Stor->set_gflag(0);
  return 0;
}

void report_board_id(Me_Query *Q) {
  uint8_t address = Q->get_address();
  uint16_t index;
  for (index = 0; index < n_drives; ++index) {
    if (board_id[index].device_address == address)
      break;
  }
  if (index >= n_drives) {
    msg(MSG_ERROR, "Invalid address %d in report_board_id", address);
    return;
  }
  board_id_t *bdp = &board_id[index];
  msg(0,
    "Addr:%d:%s TEC-%ld HW V%.2lf S/N %ld FW V%.2lf Build %ld",
    address, bdp->mnemonic, bdp->device_type,
    bdp->hw_version * 0.01,
    bdp->serial_num,
    bdp->fw_version * 0.01,
    bdp->fw_build);
}

void identify_board(Me_Ser *ser, int index, uint8_t address) {
  nl_assert(index < n_drives);
  board_id_t *bdp = &board_id[index];
  Me_Query *Q = ser->new_query();
  Q->setup_int32_query(address, 100, &bdp->device_type);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_int32_query(address, 101, &bdp->hw_version);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_int32_query(address, 102, &bdp->serial_num);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_int32_query(address, 103, &bdp->fw_version);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_int32_query(address, 1051, &bdp->fw_build);
  Q->set_callback(report_board_id);
  ser->enqueue_request(Q);
}

void poll_board(Me_Ser *ser, int index, uint8_t address) {
  nl_assert(index < n_drives);
  me_drive_t *medp = &meerstetter.drive[index];
  // board_id_t *bdp = &board_id[index];
  Me_Query *Q = ser->new_query();
  Q->setup_int32_query(address, 104, &medp->DeviceStatus, &medp->Mask, 1<<1);
  Q->set_persistent(true);
  ser->enqueue_request(Q);

  Q = ser->new_query();
  Q->setup_int32_query(address, 105, &medp->ErrorNumber, &medp->Mask, 1<<2);
  Q->set_persistent(true);
  ser->enqueue_request(Q);

  Q = ser->new_query();
  Q->setup_int32_query(address, 106, &medp->ErrorInstance, &medp->Mask, 1<<3);
  Q->set_persistent(true);
  ser->enqueue_request(Q);

  Q = ser->new_query();
  Q->setup_int32_query(address, 107, &medp->ErrorParameter, &medp->Mask, 1<<4);
  Q->set_persistent(true);
  ser->enqueue_request(Q);

  Q = ser->new_query();
  Q->setup_float32_query(address, 1000, &medp->ObjectTemp, &medp->Mask, 1<<5);
  Q->set_persistent(true);
  ser->enqueue_request(Q);

  Q = ser->new_query();
  Q->setup_float32_query(address, 1001, &medp->SinkTemp, &medp->Mask, 1<<6);
  Q->set_persistent(true);
  ser->enqueue_request(Q);

  Q = ser->new_query();
  Q->setup_float32_query(address, 1010, &medp->TargetObjectTemp, &medp->Mask, 1<<7);
  Q->set_persistent(true);
  ser->enqueue_request(Q);

  Q = ser->new_query();
  Q->setup_float32_query(address, 1020, &medp->ActualOutputCurrent, &medp->Mask, 1<<8);
  Q->set_persistent(true);
  ser->enqueue_request(Q);

  Q = ser->new_query();
  Q->setup_float32_query(address, 1021, &medp->ActualOutputVoltage, &medp->Mask, 1<<9);
  Q->set_persistent(true);
  ser->enqueue_request(Q);
}

void enqueue_requests(Me_Ser *ser) {
  const char *s = address_opts;
  int index = 0;
  while (*s) {
    int address = 0;
    if (!isdigit(*s)) {
      msg(MSG_FATAL,
        "Invalid character in address option string after %d address(es): '%s'",
        index, address_opts);
    }
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
  }
  msg(0, "Addressing %d drives", n_drives);
  for (index = 0; index < n_drives; ++index)
    identify_board(ser, index, board_id[index].device_address);
  for (index = 0; index < n_drives; ++index)
    poll_board(ser, index, board_id[index].device_address);
}

int main(int argc, char **argv) {
  oui_init_options(argc, argv);
  Selector S;
  Me_Ser Ser(Me_Ser_path);
  Ser.setup(57600, 8, 'n', 1, 1, 1);
  Ser.set_ohflow(false);
  Me_Cmd Cmd(&Ser);
  Me_TM_Selectee TM(Me_Name);
  S.add_child(&Ser);
  S.add_child(&Cmd);
  S.add_child(&TM);
  msg(0, "Starting: V1.1 standalone");
  enqueue_requests(&Ser);
  S.event_loop();
  msg(0, "Terminating");
  return 0;
}

