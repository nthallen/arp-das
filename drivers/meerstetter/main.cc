#include "meerstetter_int.h"
#include "meerstetter.h"
#include "msg.h"
#include "oui.h"

const char *Me_Ser_path = "/dev/ser1";
meerstetter_t meerstetter;

struct board_id_t {
  int32_t device_type; // 100
  int32_t hw_version; // 101, fixed point %.2lf
  int32_t serial_num; // 102
  int32_t fw_version; // 103
  int32_t fw_build;   // 1051
} board_id;

void report_board_id(Me_Query *Q) {
  msg(0,
    "Addr:%d TEC%ld HW V%.2lf S/N %ld FW V%.2lf Build %ld",
    Q->get_address(), board_id.device_type,
    board_id.hw_version * 0.01,
    board_id.serial_num,
    board_id.fw_version * 0.01,
    board_id.fw_build);
}

void identify_board(Me_Ser *ser, uint8_t address) {
  Me_Query *Q;
  Q = ser->new_query();
  Q->setup_int32_query(address, 100, &board_id.device_type);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_int32_query(address, 101, &board_id.hw_version);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_int32_query(address, 102, &board_id.serial_num);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_int32_query(address, 103, &board_id.fw_version);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_int32_query(address, 1051, &board_id.fw_build);
  Q->set_callback(report_board_id);
  ser->enqueue_request(Q);
}

void poll_board(Me_Ser *ser, uint8_t address) {
  Me_Query *Q;
  Q = ser->new_query();
  Q->setup_int32_query(address, 104, &meerstetter.DeviceStatus);
  Q->set_persistent(true);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_float32_query(address, 1000, &meerstetter.ObjectTemp);
  Q->set_persistent(true);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_float32_query(address, 1001, &meerstetter.SinkTemp);
  Q->set_persistent(true);
  ser->enqueue_request(Q);
  Q = ser->new_query();
  Q->setup_float32_query(address, 1010, &meerstetter.TargetObjectTemp);
  Q->set_persistent(true);
  ser->enqueue_request(Q);
}

void enqueue_requests(Me_Ser *ser) {
  identify_board(ser, 1);
  poll_board(ser, 1);
}

int main(int argc, char **argv) {
  oui_init_options(argc, argv);
  Selector S;
  Me_Ser Ser(Me_Ser_path);
  Ser.setup(57600, 8, 'n', 1, 1, 1);
  Me_Cmd Cmd(&Ser);
  TM_Selectee TM("meerstetter", &meerstetter, sizeof(meerstetter));
  S.add_child(&Ser);
  S.add_child(&Cmd);
  S.add_child(&TM);
  enqueue_requests(&Ser);
  msg(0, "Starting: V1.0");
  S.event_loop();
  msg(0, "Terminating");
  return 0;
}

