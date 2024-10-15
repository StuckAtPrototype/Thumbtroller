#pragma once

#include "esp_log.h"
//#include "esp_nimble_hci.h"
#include "host/ble_hs.h"
#include "nimble/ble.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"

#define LOG_TAG_GAP "gap"

static const char device_name[] = "Rover-v1";

void advertise();
void reset_cb(int reason);
void sync_cb(void);
void host_task(void *param);
void write_to_target_characteristic(uint8_t * data, uint32_t data_size);


uint8_t gap_connected(void);