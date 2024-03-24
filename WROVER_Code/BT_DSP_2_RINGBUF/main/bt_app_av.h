/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#ifndef __BT_APP_AV_H__
#define __BT_APP_AV_H__

#include <stdint.h>
#include "esp_a2dp_api.h"

/* log tags */
#define BT_AV_TAG       "BT_AV"
#define BT_RC_TG_TAG    "RC_TG"
#define BT_RC_CT_TAG    "RC_CT"

/* I2S Pin Definitions */
#define I2S_BCK_PIN 26
#define I2S_DATA_PIN 25
#define I2S_LRCK_PIN 22

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

#endif /* __BT_APP_AV_H__*/
