/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 * 
 * UART Inspired by: https://github.com/espressif/esp-idf/blob/master/examples/peripherals/uart/uart_echo/main/uart_echo_example_main.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_log.h"

#include "esp_bt.h"
#include "bt_app_all.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"

#include "driver/uart.h"
#include "driver/gpio.h"

/********************************
 * BT DEFINES
 *******************************/
#define LOCAL_DEVICE_NAME    "ARTISYN"

/********************************
 * UART DEFINES
 *******************************/
#define WROVER_UART_BUF_SIZE 1024
#define WROVER_UART_BAUD 115200
#define WROVER_UART_PORT_NUM 2
#define WROVER_UART_RX 14
#define WROVER_UART_TX 27
#define WROVER_UART_STACK_SIZE 2048

// TODO delete these once we have good coeffs
static int coeff_1 = 101;
static int coeff_2 = 102;
static int coeff_3 = 103;
static int coeff_4 = 104;
static int coeff_5 = 105;
static int coeff_6 = 106;
static int coeff_7 = 107;
static int coeff_8 = 108;
static int coeff_9 = 109;
static int coeff_10 = 110;
static int coeff_11 = 111;
static int coeff_12 = 112;

/* event for stack up */
enum {
    BT_APP_EVT_STACK_UP = 0,
};

/********************************
 * BT STATIC FUNCTION DECLARATIONS
 *******************************/
/* GAP callback function */
static void bt_app_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
/* handler for bluetooth stack enabled events */
static void bt_av_hdl_stack_evt(uint16_t event, void *p_param);

/*******************************
 * BT STATIC FUNCTION DEFINITIONS
 ******************************/
static void bt_app_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    uint8_t *bda = NULL;

    switch (event) {
    /* when authentication completed, this event comes */
    case ESP_BT_GAP_AUTH_CMPL_EVT: {
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(BT_AV_TAG, "authentication success: %s", param->auth_cmpl.device_name);
            esp_log_buffer_hex(BT_AV_TAG, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
        } else {
            ESP_LOGE(BT_AV_TAG, "authentication failed, status: %d", param->auth_cmpl.stat);
        }
        break;
    }

#if (CONFIG_BT_SSP_ENABLED == true)
    /* when Security Simple Pairing user confirmation requested, this event comes */
    case ESP_BT_GAP_CFM_REQ_EVT:
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %"PRIu32, param->cfm_req.num_val);
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;
    /* when Security Simple Pairing passkey notified, this event comes */
    case ESP_BT_GAP_KEY_NOTIF_EVT:
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_KEY_NOTIF_EVT passkey: %"PRIu32, param->key_notif.passkey);
        break;
    /* when Security Simple Pairing passkey requested, this event comes */
    case ESP_BT_GAP_KEY_REQ_EVT:
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
        break;
#endif

    /* when GAP mode changed, this event comes */
    case ESP_BT_GAP_MODE_CHG_EVT:
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_MODE_CHG_EVT mode: %d", param->mode_chg.mode);
        break;
    /* when ACL connection completed, this event comes */
    case ESP_BT_GAP_ACL_CONN_CMPL_STAT_EVT:
        bda = (uint8_t *)param->acl_conn_cmpl_stat.bda;
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_ACL_CONN_CMPL_STAT_EVT Connected to [%02x:%02x:%02x:%02x:%02x:%02x], status: 0x%x",
                 bda[0], bda[1], bda[2], bda[3], bda[4], bda[5], param->acl_conn_cmpl_stat.stat);
        break;
    /* when ACL disconnection completed, this event comes */
    case ESP_BT_GAP_ACL_DISCONN_CMPL_STAT_EVT:
        bda = (uint8_t *)param->acl_disconn_cmpl_stat.bda;
        ESP_LOGI(BT_AV_TAG, "ESP_BT_GAP_ACL_DISC_CMPL_STAT_EVT Disconnected from [%02x:%02x:%02x:%02x:%02x:%02x], reason: 0x%x",
                 bda[0], bda[1], bda[2], bda[3], bda[4], bda[5], param->acl_disconn_cmpl_stat.reason);
        break;
    /* others */
    default: {
        ESP_LOGI(BT_AV_TAG, "event: %d", event);
        break;
    }
    }
}

static void bt_av_hdl_stack_evt(uint16_t event, void *p_param)
{
    ESP_LOGD(BT_AV_TAG, "%s event: %d", __func__, event);

    switch (event) {
    /* when do the stack up, this event comes */
    case BT_APP_EVT_STACK_UP: {
        esp_bt_dev_set_device_name(LOCAL_DEVICE_NAME);
        esp_bt_gap_register_callback(bt_app_gap_cb);

        assert(esp_a2d_sink_init() == ESP_OK);
        esp_a2d_register_callback(&bt_app_a2d_cb);
        esp_a2d_sink_register_data_callback(bt_app_a2d_data_cb);

        /* Get the default value of the delay value */
        esp_a2d_sink_get_delay_value();

        /* set discoverable and connectable mode, wait to be connected */
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        break;
    }
    /* others */
    default:
        ESP_LOGE(BT_AV_TAG, "%s unhandled event: %d", __func__, event);
        break;
    }
}


/********************************
 * UART STATIC FUNCTION DECLARATIONS
 *******************************/
/* UART configuration function - intended to be called from app_main */
static void wrover_uart_init();
/* Processing function for RX data */
static bool update_fir_vals(uint8_t* data, int len);
/* Populating function for TX data */
static bool populate_tx_buf(uint8_t* data, int len);
/* RX Task - Data received from WROOM */
static void wrover_rx_task(void *arg);
/* TX Task - Data sent to WROOM */
static void wrover_tx_task(void *arg);

/********************************
 * UART STATIC FUNCTION DEFINITIONS
 *******************************/
static void wrover_uart_init() {
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t wrover_uart_config = {
        .baud_rate = WROVER_UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(WROVER_UART_PORT_NUM, WROVER_UART_BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(WROVER_UART_PORT_NUM, &wrover_uart_config));
    // Ensure for WROVER: TX : GPIO27, RX : GPIO14 - don't use RTS/CTS
    ESP_ERROR_CHECK((uart_set_pin(WROVER_UART_PORT_NUM, WROVER_UART_TX, WROVER_UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE)));

    // Start a task for both the RX and TX
    xTaskCreate(wrover_rx_task, "wrover_rx_task", WROVER_UART_STACK_SIZE, NULL, 20, NULL);
    xTaskCreate(wrover_tx_task, "wrover_tx_task", WROVER_UART_STACK_SIZE, NULL, 21, NULL);
}

static void wrover_rx_task(void *arg) {
    // Allocate a buffer for incoming data
    uint8_t* rx_data = (uint8_t*) malloc(WROVER_UART_BUF_SIZE);

    for(;;) {
        // Give enough of a delay before reading
        // Read in a maximum of (BUF_SIZE-1) bytes - leave space for a terminating '\0'
        int byte_len = uart_read_bytes(WROVER_UART_PORT_NUM, rx_data, (WROVER_UART_BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);

        if(byte_len) {
            // Terminate the byte array, and send for processing
            rx_data[byte_len] = '\0';
            if(!update_fir_vals(rx_data, byte_len)) {
                ESP_LOGE("UART", "RX FIR Updating Failed");
            }
        }
    }
    free(rx_data);
}

static void wrover_tx_task(void *arg) {
    // Allocate a buffer for outgoing data
    uint8_t* tx_data = (uint8_t*) malloc(WROVER_UART_BUF_SIZE);

    for(;;) {
        if(!populate_tx_buf(tx_data, WROVER_UART_BUF_SIZE)) {
            ESP_LOGE("UART", "TX Coefficient Populating Failed");
            continue;
        }
        // Give enough of a delay before sending - about 60 times per second
        vTaskDelay(pdMS_TO_TICKS(1000 / 60));
        int err = uart_write_bytes(WROVER_UART_PORT_NUM, (const char*)tx_data, WROVER_UART_BUF_SIZE);

        if(err == -1) {
            ESP_LOGE("UART", "TX Send Failed");
        }
    }
    free(tx_data);
}

static bool update_fir_vals(uint8_t* data, int len) {
    // TODO
    return true;
}

static bool populate_tx_buf(uint8_t* data, int len) {
    // TODO - actually grab coefficients (delete predefined ones at the top of the file)
    // Use snprintf() to quickly and safely place coefficients in data buffer
    int written = snprintf((char*)data, len, "{\"COEFFS\" : {\"C1\" : %d, \"C2\" : %d, \"C3\" : %d, \"C4\" : %d, \"C5\" : %d, \"C6\" : %d, \"C7\" : %d, \"C8\" : %d, \"C9\" : %d, \"C10\" : %d, \"C11\" : %d, \"C12\" : %d}}",
    coeff_1, coeff_2, coeff_3, coeff_4, coeff_5, coeff_6, coeff_7, coeff_8, coeff_9, coeff_10, coeff_11, coeff_12);

    // Error if not enough space for bytes, or if 0 or ERROR bytes are written
    if(written > len || written < 1) {
        return false;
    }
    return true;
}

/*******************************
 * MAIN ENTRY POINT
 ******************************/

void app_main(void)
{
    /* initialize NVS — it is used to store PHY calibration data */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    /*
     * This example only uses the functions of Classical Bluetooth.
     * So release the controller memory for Bluetooth Low Energy.
     */
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    if ((err = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }
    if ((err = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }
    if ((err = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }
    if ((err = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(BT_AV_TAG, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(err));
        return;
    }

#if (CONFIG_BT_SSP_ENABLED == true)
    /* set default parameters for Secure Simple Pairing */
    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
    esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
#endif

    /* set default parameters for Legacy Pairing (use fixed pin code 1234) */
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_FIXED;
    esp_bt_pin_code_t pin_code;
    pin_code[0] = '1';
    pin_code[1] = '2';
    pin_code[2] = '3';
    pin_code[3] = '4';
    esp_bt_gap_set_pin(pin_type, 4, pin_code);

    bt_app_task_start_up();
    /* bluetooth device name, connection mode and profile set up */
    bt_app_work_dispatch(bt_av_hdl_stack_evt, BT_APP_EVT_STACK_UP, NULL, 0, NULL);

    // Start the UART channel
    wrover_uart_init();
}
