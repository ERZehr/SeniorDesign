/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include "freertos/xtensa_api.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "bt_app_all.h"
#include "driver/i2s_std.h"
#include "freertos/ringbuf.h"
#include "sys/lock.h"

#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"

#define RINGBUF_HIGHEST_WATER_LEVEL    (32 * 1024)
#define RINGBUF_PREFETCH_WATER_LEVEL   (20 * 1024)

enum {
    RINGBUFFER_MODE_PROCESSING,    /* ringbuffer is buffering incoming audio data, I2S is working */
    RINGBUFFER_MODE_PREFETCHING,   /* ringbuffer is buffering incoming audio data, I2S is waiting */
    RINGBUFFER_MODE_DROPPING       /* ringbuffer is not buffering (dropping) incoming audio data, I2S is working */
};

/*******************************
 * STATIC FUNCTION DECLARATIONS
 ******************************/

/* handler for application task */
static void bt_app_task_handler(void *arg);
/* handler for I2S task */
static void bt_i2s_task_handler(void *arg);
/* handler for DSP task */
// static void bt_dsp_task_handler(void *arg);
/* message sender */
static bool bt_app_send_msg(bt_app_msg_t *msg);
/* handle dispatched messages */
static void bt_app_work_dispatched(bt_app_msg_t *msg);

/*******************************
 * STATIC VARIABLE DEFINITIONS
 ******************************/

static QueueHandle_t s_bt_app_task_queue = NULL;  /* handle of work queue */
static TaskHandle_t s_bt_app_task_handle = NULL;  /* handle of application task  */
static TaskHandle_t s_bt_i2s_task_handle = NULL;  /* handle of I2S task */
// static TaskHandle_t s_bt_dsp_task_handle = NULL;  /* handle of I2S task */
static RingbufHandle_t s_ringbuf_i2s = NULL;     /* handle of ringbuffer for I2S */
// static RingbufHandle_t s_ringbuf_dsp = NULL;     /* handle of ringbuffer for DSP */
static SemaphoreHandle_t s_i2s_write_semaphore = NULL;
// static SemaphoreHandle_t s_dsp_semaphore = NULL;
static uint16_t i2s_ringbuffer_mode = RINGBUFFER_MODE_PROCESSING;
// static uint16_t dsp_ringbuffer_mode = RINGBUFFER_MODE_PROCESSING;

/*********************************
 * EXTERNAL FUNCTION DECLARATIONS
 ********************************/
extern i2s_chan_handle_t tx_chan;

/*******************************
 * STATIC FUNCTION DEFINITIONS
 ******************************/

static bool bt_app_send_msg(bt_app_msg_t *msg)
{
    if (msg == NULL) {
        return false;
    }

    /* send the message to work queue */
    if (xQueueSend(s_bt_app_task_queue, msg, 10 / portTICK_PERIOD_MS) != pdTRUE) {
        ESP_LOGE(BT_APP_CORE_TAG, "%s xQueue send failed", __func__);
        return false;
    }
    return true;
}

static void bt_app_work_dispatched(bt_app_msg_t *msg)
{
    if (msg->cb) {
        msg->cb(msg->event, msg->param);
    }
}

static void bt_app_task_handler(void *arg)
{
    bt_app_msg_t msg;

    for (;;) {
        /* receive message from work queue and handle it */
        if (pdTRUE == xQueueReceive(s_bt_app_task_queue, &msg, (TickType_t)portMAX_DELAY)) {
            ESP_LOGD(BT_APP_CORE_TAG, "%s, signal: 0x%x, event: 0x%x", __func__, msg.sig, msg.event);

            switch (msg.sig) {
            case BT_APP_SIG_WORK_DISPATCH:
                bt_app_work_dispatched(&msg);
                break;
            default:
                ESP_LOGW(BT_APP_CORE_TAG, "%s, unhandled signal: %d", __func__, msg.sig);
                break;
            } /* switch (msg.sig) */

            if (msg.param) {
                free(msg.param);
            }
        }
    }
}

static void bt_i2s_task_handler(void *arg)
{
    uint8_t *data = NULL;
    size_t item_size = 0;
    /**
     * The total length of DMA buffer of I2S is:
     * `dma_frame_num * dma_desc_num * i2s_channel_num * i2s_data_bit_width / 8`.
     * Transmit `dma_frame_num * dma_desc_num` bytes to DMA is trade-off.
     */
    const size_t item_size_upto = 240 * 6;
    size_t bytes_written = 0;

    for (;;) {
        if (pdTRUE == xSemaphoreTake(s_i2s_write_semaphore, portMAX_DELAY)) {
            for (;;) {
                item_size = 0;
                /* receive data from ringbuffer and write it to I2S DMA transmit buffer */
                data = (uint8_t *)xRingbufferReceiveUpTo(s_ringbuf_i2s, &item_size, (TickType_t)pdMS_TO_TICKS(20), item_size_upto);
                if (item_size == 0) {
                    ESP_LOGI(BT_APP_CORE_TAG, "i2s ringbuffer underflowed! mode changed: RINGBUFFER_MODE_PREFETCHING");
                    i2s_ringbuffer_mode = RINGBUFFER_MODE_PREFETCHING;
                    break;
                }

                i2s_channel_write(tx_chan, data, item_size, &bytes_written, portMAX_DELAY);
                vRingbufferReturnItem(s_ringbuf_i2s, (void *)data);
            }
        }
    }
}

// static void bt_dsp_task_handler(void *arg)
// {
//     uint8_t *data = NULL;
//     size_t item_size = 0;
//     const size_t item_size_upto = 240 * 6;
//     for (;;) {
//         if (pdTRUE == xSemaphoreTake(s_dsp_semaphore, portMAX_DELAY)) {
//             for (;;) {
//                 item_size = 0;
//                 /* receive data from ringbuffer and process it through DSP algorithm */
//                 data = (uint8_t *)xRingbufferReceiveUpTo(s_ringbuf_dsp, &item_size, (TickType_t)pdMS_TO_TICKS(20), item_size_upto);
//                 if (item_size == 0) {
//                     ESP_LOGI(BT_APP_CORE_TAG, "dsp ringbuffer underflowed! mode changed: RINGBUFFER_MODE_PREFETCHING");
//                     dsp_ringbuffer_mode = RINGBUFFER_MODE_PREFETCHING;
//                     break;
//                 }

//                 // if(!bt_media_biquad_bilinear_filter(data, item_size)) {
//                 //     ESP_LOGI(BT_APP_CORE_TAG, "Failed DSP!");
//                 //     break;
//                 // }
//                 // Successful DSP processing, send to next ringbuffer
//                 else {
//                     write_ringbuf(data, item_size, "I2S");
//                 }
//                 // TODO: possibly corrupted data if already altered by dsp()?
//                 vRingbufferReturnItem(s_ringbuf_dsp, (void *)data);
//             }
//         }
//     }
// }

/********************************
 * EXTERNAL FUNCTION DEFINITIONS
 *******************************/

bool bt_app_work_dispatch(bt_app_cb_t p_cback, uint16_t event, void *p_params, int param_len, bt_app_copy_cb_t p_copy_cback)
{
    ESP_LOGD(BT_APP_CORE_TAG, "%s event: 0x%x, param len: %d", __func__, event, param_len);

    bt_app_msg_t msg;
    memset(&msg, 0, sizeof(bt_app_msg_t));

    msg.sig = BT_APP_SIG_WORK_DISPATCH;
    msg.event = event;
    msg.cb = p_cback;

    if (param_len == 0) {
        return bt_app_send_msg(&msg);
    } else if (p_params && param_len > 0) {
        if ((msg.param = malloc(param_len)) != NULL) {
            memcpy(msg.param, p_params, param_len);
            /* check if caller has provided a copy callback to do the deep copy */
            if (p_copy_cback) {
                p_copy_cback(msg.param, p_params, param_len);
            }
            return bt_app_send_msg(&msg);
        }
    }

    return false;
}

void bt_app_task_start_up(void)
{
    s_bt_app_task_queue = xQueueCreate(10, sizeof(bt_app_msg_t));
    xTaskCreate(bt_app_task_handler, "BtAppTask", 3072, NULL, 10, &s_bt_app_task_handle);
}

void bt_app_task_shut_down(void)
{
    if (s_bt_app_task_handle) {
        vTaskDelete(s_bt_app_task_handle);
        s_bt_app_task_handle = NULL;
    }
    if (s_bt_app_task_queue) {
        vQueueDelete(s_bt_app_task_queue);
        s_bt_app_task_queue = NULL;
    }
}

void bt_i2s_task_start_up(void)
{
    ESP_LOGI(BT_APP_CORE_TAG, "ringbuffer data empty! mode changed: RINGBUFFER_MODE_PREFETCHING");
    i2s_ringbuffer_mode = RINGBUFFER_MODE_PREFETCHING;
    // dsp_ringbuffer_mode = RINGBUFFER_MODE_PREFETCHING;
    if ((s_i2s_write_semaphore = xSemaphoreCreateBinary()) == NULL) {
        ESP_LOGE(BT_APP_CORE_TAG, "%s, i2s semaphore create failed", __func__);
        return;
    }
    if ((s_ringbuf_i2s = xRingbufferCreate(RINGBUF_HIGHEST_WATER_LEVEL, RINGBUF_TYPE_BYTEBUF)) == NULL) {
        ESP_LOGE(BT_APP_CORE_TAG, "%s, i2s ringbuffer create failed", __func__);
        return;
    }
    // if ((s_dsp_semaphore = xSemaphoreCreateBinary()) == NULL) {
    //     ESP_LOGE(BT_APP_CORE_TAG, "%s, dsp semaphore create failed", __func__);
    //     return;
    // }
    // if ((s_ringbuf_dsp = xRingbufferCreate(RINGBUF_HIGHEST_WATER_LEVEL, RINGBUF_TYPE_BYTEBUF)) == NULL) {
    //     ESP_LOGE(BT_APP_CORE_TAG, "%s, dsp ringbuffer create failed", __func__);
    //     return;
    // }
    xTaskCreate(bt_i2s_task_handler, "BTI2STask", 2048, NULL, configMAX_PRIORITIES - 3, &s_bt_i2s_task_handle);
    // xTaskCreate(bt_dsp_task_handler, "BTDSPTask", 2048, NULL, configMAX_PRIORITIES - 2, &s_bt_dsp_task_handle);
}

void bt_i2s_task_shut_down(void)
{
    if (s_bt_i2s_task_handle) {
        vTaskDelete(s_bt_i2s_task_handle);
        s_bt_i2s_task_handle = NULL;
    }
    if (s_ringbuf_i2s) {
        vRingbufferDelete(s_ringbuf_i2s);
        s_ringbuf_i2s = NULL;
    }
    if (s_i2s_write_semaphore) {
        vSemaphoreDelete(s_i2s_write_semaphore);
        s_i2s_write_semaphore = NULL;
    }
    // if (s_ringbuf_dsp) {
    //     vRingbufferDelete(s_ringbuf_dsp);
    //     s_ringbuf_dsp = NULL;
    // }
    // if (s_dsp_semaphore) {
    //     vSemaphoreDelete(s_dsp_semaphore);
    //     s_dsp_semaphore = NULL;
    // }
}

size_t write_ringbuf(const uint8_t *data, size_t size, const char* ringbuf_name)
{
    size_t item_size = 0;
    BaseType_t done = pdFALSE;

    // Write to the DSP ringbuffer
    // if(strcmp(ringbuf_name, "DSP") == 0) {
    //     if (dsp_ringbuffer_mode == RINGBUFFER_MODE_DROPPING) {
    //         ESP_LOGW(BT_APP_CORE_TAG, "dsp ringbuffer is full, drop this packet!");
    //         vRingbufferGetInfo(s_ringbuf_dsp, NULL, NULL, NULL, NULL, &item_size);
    //         if (item_size <= RINGBUF_PREFETCH_WATER_LEVEL) {
    //             ESP_LOGI(BT_APP_CORE_TAG, "dsp ringbuffer data decreased! mode changed: RINGBUFFER_MODE_PROCESSING");
    //             dsp_ringbuffer_mode = RINGBUFFER_MODE_PROCESSING;
    //         }
    //         return 0;
    //     }
    //     done = xRingbufferSend(s_ringbuf_dsp, (void *)data, size, (TickType_t)0);

    //     if (!done) {
    //         ESP_LOGW(BT_APP_CORE_TAG, "dsp ringbuffer overflowed, ready to decrease data! mode changed: RINGBUFFER_MODE_DROPPING");
    //         dsp_ringbuffer_mode = RINGBUFFER_MODE_DROPPING;
    //     }

    //     if (dsp_ringbuffer_mode == RINGBUFFER_MODE_PREFETCHING) {
    //         vRingbufferGetInfo(s_ringbuf_dsp, NULL, NULL, NULL, NULL, &item_size);
    //         if (item_size >= RINGBUF_PREFETCH_WATER_LEVEL) {
    //             ESP_LOGI(BT_APP_CORE_TAG, "dsp ringbuffer data increased! mode changed: RINGBUFFER_MODE_PROCESSING");
    //             dsp_ringbuffer_mode = RINGBUFFER_MODE_PROCESSING;
    //             if (pdFALSE == xSemaphoreGive(s_dsp_semaphore)) {
    //                 ESP_LOGE(BT_APP_CORE_TAG, "dsp semaphore give failed");
    //             }
    //         }
    //     }
    // }
    // Write to the I2S ringbuffer
    // else {
        if (i2s_ringbuffer_mode == RINGBUFFER_MODE_DROPPING) {
            ESP_LOGW(BT_APP_CORE_TAG, "i2s ringbuffer is full, drop this packet!");
            vRingbufferGetInfo(s_ringbuf_i2s, NULL, NULL, NULL, NULL, &item_size);
            if (item_size <= RINGBUF_PREFETCH_WATER_LEVEL) {
                ESP_LOGI(BT_APP_CORE_TAG, "i2s ringbuffer data decreased! mode changed: RINGBUFFER_MODE_PROCESSING");
                i2s_ringbuffer_mode = RINGBUFFER_MODE_PROCESSING;
            }
            return 0;
        }

        done = xRingbufferSend(s_ringbuf_i2s, (void *)data, size, (TickType_t)0);

        if (!done) {
            ESP_LOGW(BT_APP_CORE_TAG, "i2s ringbuffer overflowed, ready to decrease data! mode changed: RINGBUFFER_MODE_DROPPING");
            i2s_ringbuffer_mode = RINGBUFFER_MODE_DROPPING;
        }

        if (i2s_ringbuffer_mode == RINGBUFFER_MODE_PREFETCHING) {
            vRingbufferGetInfo(s_ringbuf_i2s, NULL, NULL, NULL, NULL, &item_size);
            if (item_size >= RINGBUF_PREFETCH_WATER_LEVEL) {
                ESP_LOGI(BT_APP_CORE_TAG, "i2s ringbuffer data increased! mode changed: RINGBUFFER_MODE_PROCESSING");
                i2s_ringbuffer_mode = RINGBUFFER_MODE_PROCESSING;
                if (pdFALSE == xSemaphoreGive(s_i2s_write_semaphore)) {
                    ESP_LOGE(BT_APP_CORE_TAG, "i2s semaphore give failed");
                }
            }
        }

    // }

    // Regardless of target ringbuffer, return written information
    return done ? size : 0;
}

/* Application layer causes delay value */
#define APP_DELAY_VALUE                  50  // 5ms

/*******************************
 * STATIC FUNCTION DECLARATIONS
 ******************************/
/* installation for i2s */
static void bt_i2s_driver_install(void);
/* uninstallation for i2s */
static void bt_i2s_driver_uninstall(void);
/* a2dp event handler */
static void bt_av_hdl_a2d_evt(uint16_t event, void *p_param);

/*******************************
 * STATIC VARIABLE DEFINITIONS
 ******************************/

static uint32_t s_pkt_cnt = 0;               /* count for audio packet */
static esp_a2d_audio_state_t s_audio_state = ESP_A2D_AUDIO_STATE_STOPPED;
                                             /* audio stream datapath state */
static const char *s_a2d_conn_state_str[] = {"Disconnected", "Connecting", "Connected", "Disconnecting"};
                                             /* connection state in string */
static const char *s_a2d_audio_state_str[] = {"Suspended", "Stopped", "Started"};
                                             /* audio stream datapath state in string */
i2s_chan_handle_t tx_chan = NULL;

/********************************
 * STATIC FUNCTION DEFINITIONS
 *******************************/

void bt_i2s_driver_install(void)
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0, I2S_ROLE_MASTER);
    chan_cfg.auto_clear = true;
    i2s_std_config_t std_cfg = {
        .clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(44100),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO),
        .gpio_cfg = {
            .mclk = I2S_GPIO_UNUSED,
            .bclk = I2S_BCK_PIN,
            .ws = I2S_LRCK_PIN,
            .dout = I2S_DATA_PIN,
            .din = I2S_GPIO_UNUSED,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv = false,
            },
        },
    };
    /* enable I2S */
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_chan, NULL));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));
}

void bt_i2s_driver_uninstall(void)
{
    ESP_ERROR_CHECK(i2s_channel_disable(tx_chan));
    ESP_ERROR_CHECK(i2s_del_channel(tx_chan));
}

static void bt_av_hdl_a2d_evt(uint16_t event, void *p_param)
{
    ESP_LOGD(BT_AV_TAG, "%s event: %d", __func__, event);

    esp_a2d_cb_param_t *a2d = NULL;

    switch (event) {
    /* when connection state changed, this event comes */
    case ESP_A2D_CONNECTION_STATE_EVT: {
        a2d = (esp_a2d_cb_param_t *)(p_param);
        uint8_t *bda = a2d->conn_stat.remote_bda;
        ESP_LOGI(BT_AV_TAG, "A2DP connection state: %s, [%02x:%02x:%02x:%02x:%02x:%02x]",
            s_a2d_conn_state_str[a2d->conn_stat.state], bda[0], bda[1], bda[2], bda[3], bda[4], bda[5]);
        if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED) {
            esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
            bt_i2s_driver_uninstall();
            bt_i2s_task_shut_down();
        } else if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED){
            esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
            bt_i2s_task_start_up();
        } else if (a2d->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTING) {
            bt_i2s_driver_install();
        }
        break;
    }
    /* when audio stream transmission state changed, this event comes */
    case ESP_A2D_AUDIO_STATE_EVT: {
        a2d = (esp_a2d_cb_param_t *)(p_param);
        ESP_LOGI(BT_AV_TAG, "A2DP audio state: %s", s_a2d_audio_state_str[a2d->audio_stat.state]);
        s_audio_state = a2d->audio_stat.state;
        if (ESP_A2D_AUDIO_STATE_STARTED == a2d->audio_stat.state) {
            s_pkt_cnt = 0;
        }
        break;
    }
    /* when audio codec is configured, this event comes */
    case ESP_A2D_AUDIO_CFG_EVT: {
        a2d = (esp_a2d_cb_param_t *)(p_param);
        ESP_LOGI(BT_AV_TAG, "A2DP audio stream configuration, codec type: %d", a2d->audio_cfg.mcc.type);
        /* for now only SBC stream is supported */
        if (a2d->audio_cfg.mcc.type == ESP_A2D_MCT_SBC) {
            int sample_rate = 16000;
            int ch_count = 2;
            char oct0 = a2d->audio_cfg.mcc.cie.sbc[0];
            if (oct0 & (0x01 << 6)) {
                sample_rate = 32000;
            } else if (oct0 & (0x01 << 5)) {
                sample_rate = 44100;
            } else if (oct0 & (0x01 << 4)) {
                sample_rate = 48000;
            }

            if (oct0 & (0x01 << 3)) {
                ch_count = 1;
            }
            i2s_channel_disable(tx_chan);
            i2s_std_clk_config_t clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(sample_rate);
            i2s_std_slot_config_t slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, ch_count);
            i2s_channel_reconfig_std_clock(tx_chan, &clk_cfg);
            i2s_channel_reconfig_std_slot(tx_chan, &slot_cfg);
            i2s_channel_enable(tx_chan);
            ESP_LOGI(BT_AV_TAG, "Configure audio player: %x-%x-%x-%x",
                     a2d->audio_cfg.mcc.cie.sbc[0],
                     a2d->audio_cfg.mcc.cie.sbc[1],
                     a2d->audio_cfg.mcc.cie.sbc[2],
                     a2d->audio_cfg.mcc.cie.sbc[3]);
            ESP_LOGI(BT_AV_TAG, "Audio player configured, sample rate: %d", sample_rate);
        }
        break;
    }
    /* when a2dp init or deinit completed, this event comes */
    case ESP_A2D_PROF_STATE_EVT: {
        a2d = (esp_a2d_cb_param_t *)(p_param);
        if (ESP_A2D_INIT_SUCCESS == a2d->a2d_prof_stat.init_state) {
            ESP_LOGI(BT_AV_TAG, "A2DP PROF STATE: Init Complete");
        } else {
            ESP_LOGI(BT_AV_TAG, "A2DP PROF STATE: Deinit Complete");
        }
        break;
    }
    /* When protocol service capabilities configured, this event comes */
    case ESP_A2D_SNK_PSC_CFG_EVT: {
        a2d = (esp_a2d_cb_param_t *)(p_param);
        ESP_LOGI(BT_AV_TAG, "protocol service capabilities configured: 0x%x ", a2d->a2d_psc_cfg_stat.psc_mask);
        if (a2d->a2d_psc_cfg_stat.psc_mask & ESP_A2D_PSC_DELAY_RPT) {
            ESP_LOGI(BT_AV_TAG, "Peer device support delay reporting");
        } else {
            ESP_LOGI(BT_AV_TAG, "Peer device unsupport delay reporting");
        }
        break;
    }
    /* when set delay value completed, this event comes */
    case ESP_A2D_SNK_SET_DELAY_VALUE_EVT: {
        a2d = (esp_a2d_cb_param_t *)(p_param);
        if (ESP_A2D_SET_INVALID_PARAMS == a2d->a2d_set_delay_value_stat.set_state) {
            ESP_LOGI(BT_AV_TAG, "Set delay report value: fail");
        } else {
            ESP_LOGI(BT_AV_TAG, "Set delay report value: success, delay_value: %u * 1/10 ms", a2d->a2d_set_delay_value_stat.delay_value);
        }
        break;
    }
    /* when get delay value completed, this event comes */
    case ESP_A2D_SNK_GET_DELAY_VALUE_EVT: {
        a2d = (esp_a2d_cb_param_t *)(p_param);
        ESP_LOGI(BT_AV_TAG, "Get delay report value: delay_value: %u * 1/10 ms", a2d->a2d_get_delay_value_stat.delay_value);
        /* Default delay value plus delay caused by application layer */
        esp_a2d_sink_set_delay_value(a2d->a2d_get_delay_value_stat.delay_value + APP_DELAY_VALUE);
        break;
    }
    /* others */
    default:
        ESP_LOGE(BT_AV_TAG, "%s unhandled event: %d", __func__, event);
        break;
    }
}

/********************************
 * EXTERNAL FUNCTION DEFINITIONS
 *******************************/

void bt_app_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param)
{
    switch (event) {
    case ESP_A2D_CONNECTION_STATE_EVT:
    case ESP_A2D_AUDIO_STATE_EVT:
    case ESP_A2D_AUDIO_CFG_EVT:
    case ESP_A2D_PROF_STATE_EVT:
    case ESP_A2D_SNK_PSC_CFG_EVT:
    case ESP_A2D_SNK_SET_DELAY_VALUE_EVT:
    case ESP_A2D_SNK_GET_DELAY_VALUE_EVT: {
        bt_app_work_dispatch(bt_av_hdl_a2d_evt, event, param, sizeof(esp_a2d_cb_param_t), NULL);
        break;
    }
    default:
        ESP_LOGE(BT_AV_TAG, "Invalid A2DP event: %d", event);
        break;
    }
}

void bt_app_a2d_data_cb(const uint8_t *data, uint32_t len)
{
    // Attempt to conduct DSP before right to the ringbuffer
    // if(!bt_media_biquad_bilinear_filter(data, len)) {
    //     ESP_LOGW(BT_AV_TAG, "DSP Failed!");
    // }
    // Perform I2S volume control - speakers are too loud

    write_ringbuf(data, len, "I2S");  // Testing purposes
    // write_ringbuf(data, len, "DSP");  // Should be this

    /* log the number every 100 packets */
    if (++s_pkt_cnt % 100 == 0) {
        ESP_LOGI(BT_AV_TAG, "Audio packet count: %"PRIu32, s_pkt_cnt);
    }
}