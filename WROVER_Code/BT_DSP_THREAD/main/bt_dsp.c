#include "bt_dsp.h"
/*******************************
 * STATIC VARIABLE DEFINITIONS
 ******************************/
static int16_t sample_l = 0;
static int16_t sample_r = 0;
static float sample_l_f = 0.0f;
static float sample_r_f = 0.0f;
static float a0[BAND_MAX] = {1.0037, 1.0036, 1.0071};
static float a1[BAND_MAX] = {-2,-1.9999, -1.9996};
static float a2[BAND_MAX] = {0.9963, 0.9964, 0.9929};
static float b0[BAND_MAX] = {0.0037, 0.0036, 0.0071};
static float b2[BAND_MAX] = {-0.0037, -0.0036, -0.0071};
static float y_l[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static float y1_l[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static float y2_l[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static int16_t x0_l = 0;
static int16_t x1_l = 0;
static int16_t x2_l = 0;
static float y_r[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static float y1_r[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static float y2_r[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static int16_t x0_r = 0;
static int16_t x1_r = 0;
static int16_t x2_r = 0;

// USER MODIFIED - see update_multipliers function for details
static float multipliers[BAND_MAX] = {1,1,0,0,0,0,0,0,0,0,0,0};

bool bt_media_biquad_bilinear_filter(const uint8_t *media, uint32_t len, uint8_t* out_media, int thread_id) {
    /* Inspiration for processing hierarchy: https://hackaday.io/project/166122-esp32-as-bt-receiver-with-dsp-capabilities
                                             https://github.com/YetAnotherElectronicsChannel/ESP32_Bluetooth_DSP_Speaker/tree/master
    */                                    
    // Allocate memory to output buffer - more interrupt-safe than modifying media in-place   
    if(out_media == NULL) {
        ESP_LOGI(DSP_TAG, "Nonexistent output buffer - skipping algorithm");
        return false;
    }

    /*
        Outer loop processes each packet in the format:
          LSB                                                                                  MSB
         _________________________________________________________________________________________
        |     |     |     |     |     |     |     |     |                |     |     |     |     |
        |  L  |  L  |  R  |  R  |  L  |  L  |  R  |  R  | .............  |  L  |  L  |  R  |  R  |
        |_____|_____|_____|_____|_____|_____|_____|_____|________________|_____|_____|_____|_____|
        
           0     1     2     3     4     5     6     7    .............  len-4 len-3 len-2 len-1
    */ 
    for(uint32_t i = 0; i < len; i += L_R_BYTE_NUM) {
        // Grab left and right samples from within media data packet
        sample_l = (int16_t)((media[i + 1] << 8) | media[i]);
        sample_r = (int16_t)((media[i + 3] << 8) | media[i + 2]);

        // Replace inner loop with thread id-controlled access
        // TODO: inter-band dependency of sample_l/sample_r: removable?
        // ^ otherwise impossible to thread - everything is sequential
        for(uint8_t z = 0; z < BAND_MAX; z++) {
            // Re-initialize floats with any changes to samples
            sample_l_f = (float)sample_l / 0x8000;
            sample_r_f = (float)sample_r / 0x8000;

            // Coefficient generation for each iteration - left channel
            y2_l[z] = y1_l[z];
            y1_l[z] = y_l[z];
            x2_l = x1_l;
            x1_l = x0_l;
            x0_l = sample_l_f;
            y_l[z] = (b0[z]*x0_l + b2[z]*x2_l - (a1[z]*y1_l[z] + a2[z]*y2_l[z])) / a0[z];
            
            // Coefficient generation for each iteration - right channel
            y2_r[z] = y1_r[z];
            y1_r[z] = y_r[z];
            x2_r = x1_r;
            x1_r = x0_r;
            x0_r = sample_r_f;
            y_r[z] = (b0[z]*x0_r + b2[z]*x2_r - (a1[z]*y1_r[z] + a2[z]*y2_r[z])) / a0[z];

            // Audio processing - left channel
            sample_l_f -= multipliers[z] * y_l[z];
            // Clamp audio between uint16_t max limits - strictly in range [0, 65534]
            if(sample_l_f > 32767) {sample_l_f = 32767;}
            if(sample_l_f < -32768) {sample_l_f = -32768;}
            // Convert back to int and place in output buffer
            sample_l = (int16_t) sample_l_f;
            out_media[i + 1] = (uint8_t) ((sample_l >> 8) & 0xff);
            out_media[i] = (uint8_t) (0xff & sample_l);

            // Audio processing - right channel
            sample_r_f -= multipliers[z] * y_r[z];
            // Clamp audio between uint16_t max limits - strictly in range [0, 65534]
            if(sample_r_f > 32767) {sample_r_f = 32767;}
            if(sample_r_f < -32768) {sample_r_f = -32768;}
            // Convert back to int and place in output buffer
            sample_r = (int16_t) sample_r_f;
            out_media[i + 3] = (uint8_t) ((sample_r >> 8) & 0xff);
            out_media[i + 2] = (uint8_t) (0xff & sample_r);
        }

        // // Inner loop generates coefficients for each band
        // for(uint8_t z = 0; z < BAND_MAX; z++) {
        //     // Re-initialize floats with any changes to samples
        //     sample_l_f = (float)sample_l / 0x8000;
        //     sample_r_f = (float)sample_r / 0x8000;

        //     // Coefficient generation for each iteration - left channel
        //     y2_l[z] = y1_l[z];
        //     y1_l[z] = y_l[z];
        //     x2_l = x1_l;
        //     x1_l = x0_l;
        //     x0_l = sample_l_f;
        //     y_l[z] = (b0[z]*x0_l + b2[z]*x2_l - (a1[z]*y1_l[z] + a2[z]*y2_l[z])) / a0[z];
            
        //     // Coefficient generation for each iteration - right channel
        //     y2_r[z] = y1_r[z];
        //     y1_r[z] = y_r[z];
        //     x2_r = x1_r;
        //     x1_r = x0_r;
        //     x0_r = sample_r_f;
        //     y_r[z] = (b0[z]*x0_r + b2[z]*x2_r - (a1[z]*y1_r[z] + a2[z]*y2_r[z])) / a0[z];

        //     // Audio processing - left channel
        //     sample_l_f -= multipliers[z] * y_l[z];
        //     // Clamp audio between uint16_t max limits - strictly in range [0, 65534]
        //     if(sample_l_f > 32767) {sample_l_f = 32767;}
        //     if(sample_l_f < -32768) {sample_l_f = -32768;}
        //     // Convert back to int and place in output buffer
        //     sample_l = (int16_t) sample_l_f;
        //     out_media[i + 1] = (uint8_t) ((sample_l >> 8) & 0xff);
        //     out_media[i] = (uint8_t) (0xff & sample_l);

        //     // Audio processing - right channel
        //     sample_r_f -= multipliers[z] * y_r[z];
        //     // Clamp audio between uint16_t max limits - strictly in range [0, 65534]
        //     if(sample_r_f > 32767) {sample_r_f = 32767;}
        //     if(sample_r_f < -32768) {sample_r_f = -32768;}
        //     // Convert back to int and place in output buffer
        //     sample_r = (int16_t) sample_r_f;
        //     out_media[i + 3] = (uint8_t) ((sample_r >> 8) & 0xff);
        //     out_media[i + 2] = (uint8_t) (0xff & sample_r);
        // }
    }

    return true;
}

void update_multipliers(uint8_t *mults) {
    for(uint8_t i = 0; i < BAND_MAX; i++) {
        multipliers[i] = mults[i];
    }
}