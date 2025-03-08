#ifndef BLUETOOTH_TRANSFER_H
#define BLUETOOTH_TRANSFER_H

#include <Arduino.h>

extern bool btTransferActive;

void bt_init();
void bt_send_file(const char* filename);
void bt_receive_file();
void bt_diagnostic();

#endif