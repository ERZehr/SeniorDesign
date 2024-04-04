#include "bt_dsp.h"
/*******************************
 * STATIC VARIABLE DEFINITIONS
 ******************************/
// // USER MODIFIED - see update_multipliers function for details
// static float multipliers[BAND_MAX] = {1,1,0,0,0,0,0,0,0,0,0,0};

static int16_t sample_l = 0;
static int16_t sample_r = 0;
static float sample_l_f = 0.0f;
static float sample_r_f = 0.0f;
static float a0_user = 1.0037;
static float a1_user = -2;
static float a2_user = 0.9963;
static float b0_user = 0.0037;
static float b2_user = -0.0037;
static float a0[BAND_MAX] = {1.0037, 1.0036, 1.0071};
static float a1[BAND_MAX] = {-2,-1.9999, -1.9996};
static float a2[BAND_MAX] = {0.9963, 0.9964, 0.9929};
static float b0[BAND_MAX] = {0.0037, 0.0036, 0.0071};
static float b2[BAND_MAX] = {-0.0037, -0.0036, -0.0071};
static float y_l[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static float y1_l[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static float y2_l[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static float y_l = 0;
static float y1_l = 0;
static float y2_l = 0;
static int16_t x0_l = 0;
static int16_t x1_l = 0;
static int16_t x2_l = 0;
static float y_r[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static float y1_r[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static float y2_r[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static float y_r = 0;
static float y1_r = 0;
static float y2_r = 0;
static int16_t x0_r = 0;
static int16_t x1_r = 0;
static int16_t x2_r = 0;


// // Int only
// static int sample_l = 0;
// static int sample_r = 0;
// // EVERYTHING THAT IS 10101 IS PLACEHOLDER
// static int a0[BAND_MAX] = {10037, 10036, 10071, 10101, 10101, 10101, 10101, 10101, 10101, 10101, 10101, 10101};
// static int a1[BAND_MAX] = {-20000, -19999, -19996, -10101, -10101, -10101, -10101, -10101, -10101, -10101, -10101, -10101};
// static int a2[BAND_MAX] = {9963, 9964, 9929, 10101, 10101, 10101, 10101, 10101, 10101, 10101, 10101, 10101};
// static int b0[BAND_MAX] = {37, 36, 71, 10101, 10101, 10101, 10101, 10101, 10101, 10101, 10101, 10101};
// static int b2[BAND_MAX] = {-37, -36, -71, -10101, -10101, -10101, -10101, -10101, -10101, -10101, -10101, -10101};
// static int y_l[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
// static int y1_l[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
// static int y2_l[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
// static int x0_l = 0;
// static int x1_l = 0;
// static int x2_l = 0;
// static int y_r[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
// static int y1_r[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
// static int y2_r[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
// static int x0_r = 0;
// static int x1_r = 0;
// static int x2_r = 0;

// // USER MODIFIED - see update_multipliers function for details
// static int multipliers[BAND_MAX] = {1,1,0,0,0,0,0,0,0,0,0,0};

bool bt_media_biquad_bilinear_filter(uint8_t *media, uint32_t len) {
    /* Inspiration for processing hierarchy: https://hackaday.io/project/166122-esp32-as-bt-receiver-with-dsp-capabilities
                                             https://github.com/YetAnotherElectronicsChannel/ESP32_Bluetooth_DSP_Speaker/tree/master
    */                                    
    // Allocate memory to output buffer - more interrupt-safe than modifying media in-place   
    uint8_t *outBuf = malloc(len);
    if(outBuf == NULL) {
        ESP_LOGI(DSP_TAG, "Malloc fail for output buffer - skipping algorithm");
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
    // Reset the accumulators
    y_l = y1_l = y2_l = y_r = y1_r = y1_l = {0};
    // DSP FOR DISPLAY
    for(uint32_t i = 0; i < COEFF_SAMPLE_MAX; i++) {
        // Grab left and right samples from within media data packet
        sample_l = (int16_t)((media[i + 1] << 8) | media[i]);
        sample_r = (int16_t)((media[i + 3] << 8) | media[i + 2]);
        for(uint32_t j = 0; j < BAND_MAX; j++) {
            // Coefficient generation for each iteration - left channel
            y2_l[j] = y1_l[j];
            y1_l[j] = y_l[j];
            x2_l = x1_l;
            x1_l = x0_l;
            x0_l = sample_l_f;
            y_l[j] = (b0[j]*x0_l + b2[j]*x2_l - (a1[j]*y1_l[j] + a2[j]*y2_l[j])) / a0[j];
            
            // Coefficient generation for each iteration - right channel
            y2_r[j] = y1_r[j];
            y1_r[j] = y_r[j];
            x2_r = x1_r;
            x1_r = x0_r;
            x0_r = sample_r_f;
            y_r[j] = (b0[j]*x0_r + b2[j]*x2_r - (a1[j]*y1_r[j] + a2[j]*y2_r[j])) / a0[j];

            // Audio processing - left channel
            sample_l_f = y_l[j];
            // Clamp audio between uint16_t max limits - strictly in range [0, 65534]
            if(sample_l_f > 32767) {sample_l_f = 32767;}
            if(sample_l_f < -32768) {sample_l_f = -32768;}

            // Audio processing - right channel
            sample_r_f = y_r[j];
            // Clamp audio between uint16_t max limits - strictly in range [0, 65534]
            if(sample_r_f > 32767) {sample_r_f = 32767;}
            if(sample_r_f < -32768) {sample_r_f = -32768;}
        }
    }
    // Average the left/right coefficients to yield 12 total band coefficients
    // Note: the coefficients are converted to ints (losing all but two decimals) such that: (int)After = 100 * (float)Before
    // WROOM will need to be able to handle this conversion
    int coeffs_to_send[BAND_MAX];
    for(int i = 0; i < BAND_MAX; i++) {
        // Equivalent to ((coeff_l + coeff_r) / 2) * 100
        coeffs_to_send[i] = (int)((y_l[i] + y_r[i]) * 50.0f);
    }
    // TODO send coeffs to main for UART


    // DSP FOR AUDIO
    for(uint32_t i = 0; i < len; i += L_R_BYTE_NUM) {
        // Grab left and right samples from within media data packet
        sample_l = (int16_t)((media[i + 1] << 8) | media[i]);
        sample_r = (int16_t)((media[i + 3] << 8) | media[i + 2]);

        
        sample_l_f = (float)sample_l;
        sample_r_f = (float)sample_r;

        // Coefficient generation for each iteration - left channel
        y2_l = y1_l;
        y1_l = y_l;
        x2_l = x1_l;
        x1_l = x0_l;
        x0_l = sample_l_f;
        y_l = (b0*x0_l + b2*x2_l - (a1*y1_l + a2*y2_l)) / a0;
        
        // Coefficient generation for each iteration - right channel
        y2_r = y1_r;
        y1_r = y_r;
        x2_r = x1_r;
        x1_r = x0_r;
        x0_r = sample_r_f;
        y_r = (b0*x0_r + b2*x2_r - (a1*y1_r + a2*y2_r)) / a0;

        // Audio processing - left channel
        sample_l_f = y_l;
        // Clamp audio between uint16_t max limits - strictly in range [0, 65534]
        if(sample_l_f > 32767) {sample_l_f = 32767;}
        if(sample_l_f < -32768) {sample_l_f = -32768;}
        // Convert back to int and place in output buffer
        sample_l = (int16_t) sample_l_f;
        outBuf[i + 1] = (uint8_t) ((sample_l >> 8) & 0xff);
        outBuf[i] = (uint8_t) (0xff & sample_l);

        // Audio processing - right channel
        sample_r_f = y_r;
        // Clamp audio between uint16_t max limits - strictly in range [0, 65534]
        if(sample_r_f > 32767) {sample_r_f = 32767;}
        if(sample_r_f < -32768) {sample_r_f = -32768;}
        // Convert back to int and place in output buffer
        sample_r = (int16_t) sample_r_f;
        outBuf[i + 3] = (uint8_t) ((sample_r >> 8) & 0xff);
        outBuf[i + 2] = (uint8_t) (0xff & sample_r);
    }

    // Copy over elements and free malloc'd buffer
    memcpy(media, outBuf, len);
    // Check if some weird error occurred - very unlikely
    if(media == NULL) {
        ESP_LOGI(DSP_TAG, "Memcpy error");
        return false;
    }
    free(outBuf);
    return true;
}

void update_multipliers(uint8_t *mults) {
    // for(uint8_t i = 0; i < BAND_MAX; i++) {
    //     multipliers[i] = mults[i];
    // }
}