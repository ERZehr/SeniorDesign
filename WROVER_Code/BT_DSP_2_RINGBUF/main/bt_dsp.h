#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "esp_log.h"

#define DSP_TAG "DSP"
#define BAND_MAX 12
#define L_R_BYTE_NUM 4
#define DSP_OK true

/* 
    DSP Capabilities inspired by these past projects:

        https://hackaday.io/project/166122-esp32-as-bt-receiver-with-dsp-capabilities
        https://github.com/YetAnotherElectronicsChannel/ESP32_Bluetooth_DSP_Speaker/tree/master
*/

/**
 * @brief Time-domain representation of an IIR filter with coefficients for each of 12 bands
 *
 * @param [in] media       byte array of media data
 * @param [in] len         length of media array (typ. 4096)
 * 
 * @return  true if malloc'd and filtered correctly - false otherwise
 */
bool bt_media_biquad_bilinear_filter(uint8_t *media, uint32_t len);

/**
 * @brief Updates multiplier coefficients for each frequency band
 *
 * @param [in] mults       byte array of media data
 * 
 * @return  true if malloc'd and filtered correctly - false otherwise
 * 
 * @NOTE: Length not needed - assumed to be equal to BAND_MAX
 */
void update_multipliers(uint8_t *mults);