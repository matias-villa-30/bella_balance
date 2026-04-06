#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/* NimBLE Stack Headers */
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "host/util/util.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"

static const char *TAG = "BLE_APP";
static const char *DEVICE_NAME = "ESP32-SERVER"; // Slightly shorter for safety

//---------------------------------------------------------
// 1. GLOBAL UUID DEFINITIONS
//---------------------------------------------------------
static const ble_uuid128_t g_svc_uuid =
    BLE_UUID128_INIT(0x59, 0x46, 0x2f, 0x12, 0x95, 0x43, 0x99, 0x99,
                     0x12, 0xc8, 0x58, 0xb4, 0x59, 0xa2, 0x71, 0x2d);

static const ble_uuid128_t g_chr_uuid =
    BLE_UUID128_INIT(0x33, 0x33, 0x33, 0x33, 0x22, 0x22, 0x22, 0x22,
                     0x11, 0x11, 0x11, 0x11, 0x00, 0x00, 0x00, 0x00);

//---------------------------------------------------------
// 2. GATT CALLBACKS
//---------------------------------------------------------
static int gatt_svr_access_cb(uint16_t conn_handle, uint16_t attr_handle,
                             struct ble_gatt_access_ctxt *ctxt, void *arg)
{
    if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
        // Use ESP_LOG_BUFFER_CHAR to see raw bytes clearly
        ESP_LOGI(TAG, "Write received! Data:");
        ESP_LOG_BUFFER_CHAR(TAG, ctxt->om->om_data, ctxt->om->om_len);
        return 0;
    }
    return BLE_ATT_ERR_UNLIKELY;
}

//---------------------------------------------------------
// 3. GLOBAL GATT TABLE
//---------------------------------------------------------
static const struct ble_gatt_svc_def g_gatt_svcs[] = {
    {
        .type = BLE_GATT_SVC_TYPE_PRIMARY,
        .uuid = &g_svc_uuid.u, // Use the .u member
        .characteristics = (struct ble_gatt_chr_def[]) {
            {
                .uuid = &g_chr_uuid.u,
                .access_cb = gatt_svr_access_cb,
                .flags = BLE_GATT_CHR_F_WRITE,
            },
            {0}
        },
    },
    {0}
};

//---------------------------------------------------------
// 4. BLE ADVERTISING
//---------------------------------------------------------
static void ble_app_advertise(void)
{
    struct ble_gap_adv_params adv_params;
    struct ble_hs_adv_fields fields;
    int rc;

    /**
     * PACKET 1: Main Advertising Data
     * We put the UUID and Flags here.
     */
    memset(&fields, 0, sizeof(fields));
    fields.flags = BLE_HS_ADV_F_DISC_GEN | BLE_HS_ADV_F_BREDR_UNSUP;
    
    // Put UUID in the primary packet
    fields.uuids128 = &g_svc_uuid;
    fields.num_uuids128 = 1;
    fields.uuids128_is_complete = 1;

    rc = ble_gap_adv_set_fields(&fields);
    if (rc != 0) { ESP_LOGE(TAG, "Error setting adv fields; rc=%d", rc); return; }

    /**
     * PACKET 2: Scan Response Data
     * We put the long Device Name here so it fits!
     */
    memset(&fields, 0, sizeof(fields));
    fields.name = (uint8_t *)DEVICE_NAME;
    fields.name_len = strlen(DEVICE_NAME);
    fields.name_is_complete = 1;

    rc = ble_gap_adv_rsp_set_fields(&fields);
    if (rc != 0) { ESP_LOGE(TAG, "Error setting scan rsp fields; rc=%d", rc); return; }

    // Start Advertising
    memset(&adv_params, 0, sizeof(adv_params));
    adv_params.conn_mode = BLE_GAP_CONN_MODE_UND;
    adv_params.disc_mode = BLE_GAP_DISC_MODE_GEN;

    rc = ble_gap_adv_start(BLE_OWN_ADDR_PUBLIC, NULL, BLE_HS_FOREVER, &adv_params, NULL, NULL);
    if (rc != 0) { ESP_LOGE(TAG, "Error starting adv; rc=%d", rc); return; }
    
    ESP_LOGI(TAG, "Advertising started with Scan Response (Name Split)!");
}

static void ble_app_on_sync(void)
{
    // Use the NimBLE helper to determine the address type automatically
    uint8_t addr_type;
    ble_hs_id_infer_auto(0, &addr_type);
    ble_app_advertise();
}

void ble_host_task(void *param)
{
    ESP_LOGI(TAG, "BLE Host Task Started");
    nimble_port_run();
}

void app_main(void)
{
    // 1. NVS Init
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 2. NimBLE Init
    nimble_port_init();

    // 3. Configure Host
    ble_hs_cfg.sync_cb = ble_app_on_sync;

    // 4. Init Services
    ble_svc_gap_init();
    ble_svc_gatt_init();
    ble_svc_gap_device_name_set(DEVICE_NAME);

    // 5. Build GATT Tree
    ble_gatts_count_cfg(g_gatt_svcs);
    ble_gatts_add_svcs(g_gatt_svcs);

    // 6. Launch Host Task
    xTaskCreate(ble_host_task, "ble_host", 4096, NULL, 5, NULL);
}
