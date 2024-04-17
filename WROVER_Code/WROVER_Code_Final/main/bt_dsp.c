/*****************************************
 * ECE 477 Team 8 "Artisyn" Credits:
 * 
 * Kaden Merrill:
 *    * FIR Filter Coefficients and Logic
 *    * FIR-based Spectrum Analyzer Logic and Optimization
 *    * DSP Integration and Testing
 * 
 * Bailey Mosher:
 *    * Machine Code -> Embedded Code Translation
 *    * DSP Integration and Testing
 * 
*****************************************/
#include "bt_dsp.h"

/*******************************
 * STATIC VARIABLE DEFINITIONS
 ******************************/
static int16_t sample_l = 0;
static int16_t sample_r = 0;
static float sample_l_f = 0.0f;
static float sample_r_f = 0.0f;
static float a0_user = 0;
static float a1_user = 0;
static float a2_user = 0;
static float b0_user = 0;
static float b2_user = 0;
//                                      FREQUENCY BANDS (NOTE: 16 kHz currently not displayed on design - 8 bands only)
//                             63           125          250        500        1000      2000       4000       8000      16000 
static float a0[BAND_MAX] = {1.0008,       1.0015,     1.0031,    1.0062,    1.0124,    1.0247,    1.0494,    1.099,    1.1997};
static float a1[BAND_MAX] = {-1.9999,     -1.9997,    -1.9987,   -1.9949,   -1.9797,   -1.9194,   -1.6839,   -0.8355,   1.3019};
static float a2[BAND_MAX] = {0.9992,       0.9985,     0.9969,    0.9938,    0.9876,    0.9753,    0.9506,    0.9010,   0.8003};
static float b0[BAND_MAX] = {0.007787,     0.0015,     0.0031,    0.0062,    0.0124,    0.0247,    0.0494,    0.099,    0.1997};
static float b2[BAND_MAX] = {-0.007787,   -0.0015,    -0.0031,   -0.0062,   -0.0124,   -0.0247,   -0.0494,   -0.099,   -0.1997};

static float y_l[BAND_MAX] = {0};
static float y1_l[BAND_MAX] = {0};
static float y2_l[BAND_MAX] = {0};
static int16_t x0_l[BAND_MAX] = {0};
static int16_t x1_l[BAND_MAX] = {0};
static int16_t x2_l[BAND_MAX] = {0};
static float y_r[BAND_MAX] = {0};
static float y1_r[BAND_MAX] = {0};
static float y2_r[BAND_MAX] = {0};
static int16_t x0_r[BAND_MAX] = {0};
static int16_t x1_r[BAND_MAX] = {0};
static int16_t x2_r[BAND_MAX] = {0};

static float y_l_audio = 0;
static float y1_l_audio = 0;
static float y2_l_audio = 0;
static int16_t x0_l_audio = 0;
static int16_t x1_l_audio = 0;
static int16_t x2_l_audio = 0;
static float y_r_audio = 0;
static float y1_r_audio = 0;
static float y2_r_audio = 0;
static int16_t x0_r_audio = 0;
static int16_t x1_r_audio = 0;
static int16_t x2_r_audio = 0;
static float outValue = 0;

static int coeffs_to_send[BAND_MAX];

bool bt_media_biquad_bilinear_filter(uint8_t *media, uint32_t len, uint8_t *outBuf, uint8_t band_num) {
    /* Inspiration for processing hierarchy: https://hackaday.io/project/166122-esp32-as-bt-receiver-with-dsp-capabilities
                                             https://github.com/YetAnotherElectronicsChannel/ESP32_Bluetooth_DSP_Speaker/tree/master
    */                                   
    /*
        Outer loop processes each packet in the format:
          LSB                                                                                  MSB
         _________________________________________________________________________________________
        |     |     |     |     |     |     |     |     |                |     |     |     |     |
        |  L  |  L  |  R  |  R  |  L  |  L  |  R  |  R  | .............  |  L  |  L  |  R  |  R  |
        |_____|_____|_____|_____|_____|_____|_____|_____|________________|_____|_____|_____|_____|
        
           0     1     2     3     4     5     6     7    .............  len-4 len-3 len-2 len-1
    */ 
    /* -------------------------------------------------------------------------
        DSP FOR AUDIO
       -------------------------------------------------------------------------
    */
    // First grab coefficients
    a0_user = get_dsp_coeff(1);
    a1_user = get_dsp_coeff(2);
    a2_user = get_dsp_coeff(3);
    b0_user = get_dsp_coeff(4);
    b2_user = get_dsp_coeff(5);
    for(uint32_t i = 0; i < len; i += L_R_BYTE_NUM) {
        // Grab left and right samples from within media data packet
        sample_l = (int16_t)((media[i + 1] << 8) | media[i]);
        sample_r = (int16_t)((media[i + 3] << 8) | media[i + 2]);

        sample_l_f = (float)sample_l;
        sample_r_f = (float)sample_r;

        // Coefficient generation for each iteration - left channel
        y2_l_audio = y1_l_audio;
        y1_l_audio = y_l_audio;
        x2_l_audio = x1_l_audio;
        x1_l_audio = x0_l_audio;
        x0_l_audio = sample_l_f;
        y_l_audio = (b0_user*x0_l_audio + b2_user*x2_l_audio - (a1_user*y1_l_audio + a2_user*y2_l_audio)) / a0_user;
        
        // Coefficient generation for each iteration - right channel
        y2_r_audio = y1_r_audio;
        y1_r_audio = y_r_audio;
        x2_r_audio = x1_r_audio;
        x1_r_audio = x0_r_audio;
        x0_r_audio = sample_r_f;
        y_r_audio = (b0_user*x0_r_audio + b2_user*x2_r_audio - (a1_user*y1_r_audio + a2_user*y2_r_audio)) / a0_user;

        // Audio processing - left channel
        sample_l_f = y_l_audio;
        // Clamp audio between uint16_t max limits - strictly in range [0, 65534]
        if(sample_l_f > 32767) {sample_l_f = 32767;}
        if(sample_l_f < -32768) {sample_l_f = -32768;}
        // Convert back to int and place in output buffer
        sample_l = (int16_t) sample_l_f;
        outBuf[i + 1] = (uint8_t) ((sample_l >> 8) & 0xff);
        outBuf[i] = (uint8_t) (0xff & sample_l);

        // Audio processing - right channel
        sample_r_f = y_r_audio;
        // Clamp audio between uint16_t max limits - strictly in range [0, 65534]
        if(sample_r_f > 32767) {sample_r_f = 32767;}
        if(sample_r_f < -32768) {sample_r_f = -32768;}
        // Convert back to int and place in output buffer
        sample_r = (int16_t) sample_r_f;
        outBuf[i + 3] = (uint8_t) ((sample_r >> 8) & 0xff);
        outBuf[i + 2] = (uint8_t) (0xff & sample_r);
    }

    /* -------------------------------------------------------------------------
        DSP FOR DISPLAY
       -------------------------------------------------------------------------
    */
    for(uint32_t i = 0; i < len; i += L_R_BYTE_NUM) {
        // Grab left and right samples from within media data packet
        sample_l = (int16_t)((media[i + 1] << 8) | media[i]);
        sample_r = (int16_t)((media[i + 3] << 8) | media[i + 2]);
        
        sample_l_f = (float)sample_l;
        sample_r_f = (float)sample_r;
        x2_r[1] = x1_r[1];
        x1_r[1] = x0_r[1];
        x0_r[1] = sample_r_f;
        for(uint32_t j = 0; j < band_num; j++) {
            // Coefficient generation for each iteration - right channel
            y2_r[j] = y1_r[j];
            y1_r[j] = y_r[j];
            y_r[j] = (b0[j]*x0_r[1] + b2[j]*x2_r[1] - (a1[j]*y1_r[j] + a2[j]*y2_r[j])) / a0[j];
            // Clamp audio between uint16_t max limits - strictly in range [0, 65534]
            if(sample_r_f > 32767) {sample_r_f = 32767;}
            if(sample_r_f < -32768) {sample_r_f = -32768;}
        }
    }
    
    // Note: the coefficients are converted to ints (losing all decimals)
    for(int i = 0; i < BAND_MAX; i++) {
        // Magnitude of the coefficients only for visual readout
        outValue = y_r[i] < 0 ? y_r[i] * -1.0f : y_r[i];
        // // DEBUGGING
        // if(i == 0) {
        //    ESP_LOGI(DSP_TAG, "Value of band %d: %.9f", i, (y_r[i]*100000.0f));
        // }
        // Zero-out any coefficients that aren't requested to be viewed by the user
        // (Not really necessary, but just a redundancy to ensure extraneous bars aren't displayed)
        if(i >= band_num) {
            coeffs_to_send[i] = 0;
        }
        else {
            coeffs_to_send[i] = (int)((outValue));
        }
    }
    // Check if some weird error occurred - very unlikely
    if(media == NULL || outBuf == NULL) {
        ESP_LOGI(DSP_TAG, "Media Buffer Error");
        return false;
    }
    return true;
}

int get_coeff(int idx) {
    return coeffs_to_send[idx-1];
}