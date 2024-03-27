/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#ifndef __BT_APP_ALL_H__
#define __BT_APP_ALL_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "bt_dsp.h"
#include "esp_a2dp_api.h"

/* log tags */
#define BT_AV_TAG       "BT_AV"
#define BT_RC_TG_TAG    "RC_TG"
#define BT_RC_CT_TAG    "RC_CT"
#define BT_APP_CORE_TAG    "BT_APP_CORE"

/* I2S Pin Definitions */
#define I2S_BCK_PIN (gpio_num_t)26
#define I2S_DATA_PIN (gpio_num_t)25
#define I2S_LRCK_PIN (gpio_num_t)22

/* signal for `bt_app_work_dispatch` */
#define BT_APP_SIG_WORK_DISPATCH    (0x01)

struct thread_args {
    uint8_t* in_media;
    uint32_t len;
    uint8_t* out_media;
    int thread_id;
};

/**
 * @brief  callback function for A2DP sink
 *
 * @param [in] event  event id
 * @param [in] param  callback parameter
 */
void bt_app_a2d_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param);

/**
 * @brief  callback function for A2DP sink audio data stream
 *
 * @param [out] data  data stream writteen by application task
 * @param [in]  len   length of data stream in byte
 */
void bt_app_a2d_data_cb(const uint8_t *data, uint32_t len);

/**
 * @brief  handler for the dispatched work
 *
 * @param [in] event  event id
 * @param [in] param  handler parameter
 */
typedef void (* bt_app_cb_t) (uint16_t event, void *param);

/* message to be sent */
typedef struct {
    uint16_t       sig;      /*!< signal to bt_app_task */
    uint16_t       event;    /*!< message event id */
    bt_app_cb_t    cb;       /*!< context switch callback */
    void           *param;   /*!< parameter area needs to be last */
} bt_app_msg_t;

/**
 * @brief  parameter deep-copy function to be customized
 *
 * @param [out] p_dest  pointer to destination data
 * @param [in]  p_src   pointer to source data
 * @param [in]  len     data length in byte
 */
typedef void (* bt_app_copy_cb_t) (void *p_dest, void *p_src, int len);

/**
 * @brief  work dispatcher for the application task
 *
 * @param [in] p_cback       callback function
 * @param [in] event         event id
 * @param [in] p_params      callback paramters
 * @param [in] param_len     parameter length in byte
 * @param [in] p_copy_cback  parameter deep-copy function
 *
 * @return  true if work dispatch successfully, false otherwise
 */
bool bt_app_work_dispatch(bt_app_cb_t p_cback, uint16_t event, void *p_params, int param_len, bt_app_copy_cb_t p_copy_cback);

/**
 * @brief  start up the application task
 */
void bt_app_task_start_up(void);

/**
 * @brief  shut down the application task
 */
void bt_app_task_shut_down(void);

/**
 * @brief  start up the is task
 */
void bt_i2s_task_start_up(void);

/**
 * @brief  shut down the I2S task
 */
void bt_i2s_task_shut_down(void);

/**
 * @brief  write data to ringbuffer
 *
 * @param [in] data  pointer to data stream
 * @param [in] size  data length in byte
 * @param [in] ringbuf_name name of ringbuffer
 *
 * @return size if writteen ringbuffer successfully, 0 others
 */
size_t write_ringbuf(const uint8_t *data, size_t size, const char* ringbuf_name);

#endif /* __BT_APP_ALL_H__*/
