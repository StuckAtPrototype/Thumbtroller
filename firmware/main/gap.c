#include "gap.h"
#include "led.h"

uint8_t addr_type;

#define ROVER_NAME "Racer3"

static uint16_t global_conn_handle = BLE_HS_CONN_HANDLE_NONE;

static const ble_uuid128_t target_service_uuid =
        BLE_UUID128_INIT(0xd8, 0xe6, 0xfd, 0x1d, 0x4a, 0x14, 0xc6, 0xb1, 0x53, 0x4c, 0x4c, 0x59, 0x6d, 0xd9, 0xf1, 0xd6);

static const ble_uuid128_t target_char_uuid =
        BLE_UUID128_INIT(0xb0, 0xa5, 0xf8, 0x45, 0x8d, 0xca, 0x89, 0x9b, 0xd8, 0x4c, 0x40, 0x1f, 0x88, 0x88, 0x40, 0x23);

static uint16_t target_char_handle = 0;

int gap_event_handler(struct ble_gap_event *event, void *arg);


uint8_t gap_connected(void){

    if (global_conn_handle == BLE_HS_CONN_HANDLE_NONE) {
        ESP_LOGE(LOG_TAG_GAP, "Not connected");
        return 0;
    } else if (target_char_handle == 0) {
        ESP_LOGE(LOG_TAG_GAP, "Characteristic handle not found");
        return 0;
    } else{
        return 1;
    }


}

static int gatt_svr_chr_write(uint16_t conn_handle, const struct ble_gatt_error *error,
                              struct ble_gatt_attr *attr, void *arg)
{
    if (error->status == 0) {
        ESP_LOGI(LOG_TAG_GAP, "Write to characteristic successful");
    } else {
        ESP_LOGE(LOG_TAG_GAP, "Write to characteristic failed: status=%d", error->status);
    }
    return 0;
}

void write_to_target_characteristic(uint8_t * data, uint32_t data_size)
{
    ESP_LOGI(LOG_TAG_GAP, "write_to_target_characteristic");

    if (global_conn_handle == BLE_HS_CONN_HANDLE_NONE) {
        ESP_LOGE(LOG_TAG_GAP, "Not connected");
        return;
    }

    if (data == NULL || data_size == 0) {
        ESP_LOGE(LOG_TAG_GAP, "Invalid data or data size");
        return;
    }

    if (target_char_handle == 0) {
        ESP_LOGE(LOG_TAG_GAP, "Characteristic handle not found");
        return;
    }

    int rc = ble_gattc_write_flat(global_conn_handle, target_char_handle, data, data_size, gatt_svr_chr_write, NULL);
    if (rc != 0) {
        ESP_LOGE(LOG_TAG_GAP, "Error initiating write to characteristic: rc=%d", rc);
    } else {
        ESP_LOGI(LOG_TAG_GAP, "Write to characteristic initiated");
    }
}

static void print_addr(const void *addr)
{
    const uint8_t *u8p;
    u8p = addr;
    ESP_LOGI(LOG_TAG_GAP, "%02x:%02x:%02x:%02x:%02x:%02x",
             u8p[5], u8p[4], u8p[3], u8p[2], u8p[1], u8p[0]);
}

void advertise() {
  struct ble_gap_adv_params adv_params;
  struct ble_hs_adv_fields fields;
  int rc;

  memset(&fields, 0, sizeof(fields));

  // flags: discoverability + BLE only
  fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;

  // include power levels
  fields.tx_pwr_lvl_is_present = 1;
  fields.tx_pwr_lvl = BLE_HS_ADV_TX_PWR_LVL_AUTO;

  // include device name
  fields.name = (uint8_t *)device_name;
  fields.name_len = strlen(device_name);
  fields.name_is_complete = 1;

  rc = ble_gap_adv_set_fields(&fields);
  if (rc != 0) {
    ESP_LOGE(LOG_TAG_GAP, "Error setting advertisement data: rc=%d", rc);
    return;
  }

  // start advertising
  memset(&adv_params, 0, sizeof(adv_params));
  adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
  adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;
  rc = ble_gap_adv_start(addr_type, NULL, BLE_HS_FOREVER, &adv_params,
                         gap_event_handler, NULL);
  if (rc != 0) {
    ESP_LOGE(LOG_TAG_GAP, "Error enabling advertisement data: rc=%d", rc);
    return;
  }
}


void start_ble_scan(void)
{
    struct ble_gap_disc_params scan_params;
    int rc;

    // Set up scan parameters
    memset(&scan_params, 0, sizeof(scan_params));
    scan_params.filter_duplicates = 1;
    scan_params.passive = 0;
    scan_params.itvl = BLE_GAP_SCAN_FAST_INTERVAL_MIN;
    scan_params.window = BLE_GAP_SCAN_FAST_WINDOW;
    scan_params.filter_policy = BLE_HCI_SCAN_FILT_NO_WL;
    scan_params.limited = 0;

    // Start scanning
    rc = ble_gap_disc(BLE_OWN_ADDR_PUBLIC, BLE_HS_FOREVER, &scan_params, gap_event_handler, NULL);
    if (rc != 0) {
        ESP_LOGE(LOG_TAG_GAP, "Error starting scan: rc=%d", rc);
        return;
    }

    ESP_LOGI(LOG_TAG_GAP, "Scanning started");
}


void reset_cb(int reason) {
  ESP_LOGE(LOG_TAG_GAP, "BLE reset: reason = %d", reason);
}


void sync_cb(void) {
  // determine best adress type
  ble_hs_id_infer_auto(0, &addr_type);

  // start avertising
//  advertise();

    // start scanning
    start_ble_scan();
}




static int gatt_chr_dis_cb(uint16_t conn_handle, const struct ble_gatt_error *error,
                           const struct ble_gatt_chr *chr, void *arg)
{
    if (error->status == 0) {
        char uuid_str[BLE_UUID_STR_LEN];
        ble_uuid_to_str(&chr->uuid.u, uuid_str);
        ESP_LOGI(LOG_TAG_GAP, "Characteristic discovered: UUID=%s, handle=%d", uuid_str, chr->val_handle);

        if (ble_uuid_cmp(&chr->uuid.u, &target_char_uuid.u) == 0) {
            ESP_LOGI(LOG_TAG_GAP, "Target characteristic found: handle=%d", chr->val_handle);
            target_char_handle = chr->val_handle;

        }
    } else if (error->status == BLE_HS_EDONE) {
        ESP_LOGI(LOG_TAG_GAP, "Characteristic discovery complete");
    } else {
        ESP_LOGE(LOG_TAG_GAP, "Error discovering characteristics: status=%d", error->status);
    }
    return 0;
}

static int gatt_svc_dis_cb(uint16_t conn_handle, const struct ble_gatt_error *error,
                           const struct ble_gatt_svc *service, void *arg)
{
    if (error->status == 0) {
        char uuid_str[BLE_UUID_STR_LEN];
        ble_uuid_to_str(&service->uuid.u, uuid_str);
        ESP_LOGI(LOG_TAG_GAP, "Service discovered: UUID=%s, start_handle=%d, end_handle=%d",
                 uuid_str, service->start_handle, service->end_handle);

        if (ble_uuid_cmp(&service->uuid.u, &target_service_uuid.u) == 0) {
            ESP_LOGI(LOG_TAG_GAP, "Target service found: start_handle=%d end_handle=%d",
                     service->start_handle, service->end_handle);

            // Initiate characteristic discovery within this service
            int rc = ble_gattc_disc_all_chrs(conn_handle, service->start_handle, service->end_handle,
                                             gatt_chr_dis_cb, NULL);
            if (rc != 0) {
                ESP_LOGE(LOG_TAG_GAP, "Error initiating characteristic discovery: rc=%d", rc);
            }
        }
    } else if (error->status == BLE_HS_EDONE) {
        ESP_LOGI(LOG_TAG_GAP, "Service discovery complete");
    } else {
        ESP_LOGE(LOG_TAG_GAP, "Error discovering services: status=%d", error->status);
    }
    return 0;
}


static int connect_to_rover(const struct ble_gap_event *event)
{
    struct ble_gap_conn_params conn_params;
    int rc;

    memset(&conn_params, 0, sizeof(conn_params));
    conn_params.scan_itvl = 0x0010;
    conn_params.scan_window = 0x0010;
    conn_params.itvl_min = BLE_GAP_INITIAL_CONN_ITVL_MIN;
    conn_params.itvl_max = BLE_GAP_INITIAL_CONN_ITVL_MAX;
//    conn_params.latency = BLE_GAP_INITIAL_CONN_LATENCY;
    conn_params.supervision_timeout = BLE_GAP_INITIAL_SUPERVISION_TIMEOUT;
    conn_params.min_ce_len = BLE_GAP_INITIAL_CONN_MIN_CE_LEN;
    conn_params.max_ce_len = BLE_GAP_INITIAL_CONN_MAX_CE_LEN;

    rc = ble_gap_connect(BLE_OWN_ADDR_PUBLIC, &event->disc.addr, 30000, &conn_params,
                         gap_event_handler, NULL);
    if (rc != 0) {
        ESP_LOGE(LOG_TAG_GAP, "Error initiating connection: rc=%d", rc);
        return rc;
    }

    ESP_LOGI(LOG_TAG_GAP, "Connection initiated");
    return 0;
}

int gap_event_handler(struct ble_gap_event *event, void *arg) {

    struct ble_hs_adv_fields fields;

  switch (event->type) {
    case BLE_GAP_EVENT_CONNECT:

      // A new connection was established or a connection attempt failed
      ESP_LOGI(LOG_TAG_GAP, "GAP: Connection %s: status=%d",
               event->connect.status == 0 ? "established" : "failed",
               event->connect.status);

      // save the connection handle
      global_conn_handle = event->connect.conn_handle;

      // Start service discovery
      ble_gattc_disc_all_svcs(event->connect.conn_handle, gatt_svc_dis_cb, NULL);

      break;

    case BLE_GAP_EVENT_DISCONNECT:
      ESP_LOGD(LOG_TAG_GAP, "GAP: Disconnect: reason=%d\n",
               event->disconnect.reason);

      // clean up the connection handle
          global_conn_handle = BLE_HS_CONN_HANDLE_NONE;

      // turn off the front LEDS
      set_led(0,false);

      // start trying to reconnect
      start_ble_scan();

      break;

    case BLE_GAP_EVENT_ADV_COMPLETE:
        ESP_LOGI(LOG_TAG_GAP, "GAP: adv complete");
      break;

    case BLE_GAP_EVENT_SUBSCRIBE:
        ESP_LOGI(LOG_TAG_GAP, "GAP: Subscribe: conn_handle=%d",
               event->connect.conn_handle);
        break;

    case BLE_GAP_EVENT_MTU:
        ESP_LOGI(LOG_TAG_GAP, "GAP: MTU update: conn_handle=%d, mtu=%d",
               event->mtu.conn_handle, event->mtu.value);
      break;

    case BLE_GAP_EVENT_DISC:
          ble_hs_adv_parse_fields(&fields, event->disc.data,
                                       event->disc.length_data);

//          ESP_LOGI(LOG_TAG_GAP, "  Address: ");
//          print_addr(event->disc.addr.val);
//
//          if (fields.name_len > 0) {
//              ESP_LOGI(LOG_TAG_GAP, "  Name: %.*s", fields.name_len, fields.name);
//          }
//
//          if (fields.flags != 0) {
//              ESP_LOGI(LOG_TAG_GAP, "  Flags: 0x%02x", fields.flags);
//          }
//          if (fields.uuids16 != NULL) {
//              ESP_LOGI(LOG_TAG_GAP, "  16-bit UUIDs: %d", fields.num_uuids16);
//          }
//
//          if (fields.tx_pwr_lvl_is_present) {
//              ESP_LOGI(LOG_TAG_GAP, "  TX Power Level: %d", fields.tx_pwr_lvl);
//          }
//
//          ESP_LOGI(LOG_TAG_GAP, "  RSSI: %d", event->disc.rssi);
//

          if (fields.name_len == strlen(ROVER_NAME) &&
              memcmp(fields.name, ROVER_NAME, fields.name_len) == 0) {
              ESP_LOGI(LOG_TAG_GAP, "Rover found! Attempting to connect...");
              ble_gap_disc_cancel();
              connect_to_rover(event);

          }

          break;
  }

  return 0;
}

void host_task(void *param) {
  // returns only when nimble_port_stop() is executed
  nimble_port_run();
  nimble_port_freertos_deinit();
}
