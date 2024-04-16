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
// static float a0_user = 1.0037;
// static float a1_user = -2;
// static float a2_user = 0.9963;
// static float b0_user = 0.0037;
// static float b2_user = -0.0037;
static float a0_user = 0;
static float a1_user = 0;
static float a2_user = 0;
static float b0_user = 0;
static float b2_user = 0;



//Normal set of bands
 static float a0[BAND_MAX] = {1.0008, 1.0015, 1.0031, 1.0062, 1.0124, 1.0247, 1.0371, 1.0494, 1.0618, 1.099, 1.1997};
 static float a1[BAND_MAX] = {-1.9999, -1.9997, -1.9987, -1.9949, -1.9797, -1.9194, -1.8201, -1.6839, -1.5136, -0.8355, 1.3019};
 static float a2[BAND_MAX] = {0.9992, 0.9985, 0.9969, 0.9938, 0.9876, 0.9753, 0.9629, 0.9506, 0.9382, 0.9010, 0.8003};
 static float b0[BAND_MAX] = {0.007787, 0.0015, 0.0031, 0.0062, 0.0124, 0.0247, 0.0371, 0.0494, 0.0618, 0.099, 0.1997};
 static float b2[BAND_MAX] = {-0.007787, -0.0015, -0.0031, -0.0062, -0.0124, -0.0247, -0.0371, -0.0494, -0.0618, -0.099, -0.1997};




//Normal set of bands
// static float a0[BAND_MAX] = {1.0004, 1.0008, 1.0015, 1.0031, 1.0062, 1.0124, 1.0247, 1.0371, 1.0494, 1.0618, 1.099, 1.1997};
// static float a1[BAND_MAX] = {-2,-1.9999, -1.9997, -1.9987, -1.9949, -1.9797, -1.9194, -1.8201, -1.6839, -1.5136, -0.8355, 1.3019};
// static float a2[BAND_MAX] = {0.9996, 0.9992, 0.9985, 0.9969, 0.9938, 0.9876, 0.9753, 0.9629, 0.9506, 0.9382, 0.9010, 0.8003};
// static float b0[BAND_MAX] = {0.003832, 0.007787, 0.0015, 0.0031, 0.0062, 0.0124, 0.0247, 0.0371, 0.0494, 0.0618, 0.099, 0.1997};
// static float b2[BAND_MAX] = {-0.003832, -0.007787, -0.0015, -0.0031, -0.0062, -0.0124, -0.0247, -0.0371, -0.0494, -0.0618, -0.099, -0.1997};
// Narrower bandwidth
//                                                                                  FREQUENCY BANDS
//                              31            63          125          250         500        1000        2000       3000       4000         5000        8000       16000 
//static float a0[BAND_MAX] = { 1.00,        1.0002,      1.0003,      1.0006,     1.0031,     1.0010,      1.002,     1.003,     1.004,      1.0049,     1.4076,     1.9406};
//static float a1[BAND_MAX] = {-2.00,       -1.9999,     -1.9997,     -1.9987,    -1.9987,    -1.9797,     -1.9194,   -1.8201,   -1.6839,    -1.5136,    -0.8355,     1.3019};
//static float a2[BAND_MAX] = { 1.00,        0.9998,      0.9997,      0.9994,     0.9969,     0.999,       0.998,     0.997,     0.996,      0.9951,     0.5924,     0.0594};
//static float b0[BAND_MAX] = { 0.00003061,  0.0001555,   0.0003086,   0.0006173,  0.0012,     0.0009876,   0.002,     0.003,     0.004,      0.0049,     0.4076,     0.9406};
//static float b2[BAND_MAX] = {-0.00003061, -0.0001555,  -0.0003086,  -0.0006173, -0.0012,    -0.0009876,  -0.002,    -0.003,    -0.004,     -0.0049,    -0.4076,    -0.9406};
static float y_l[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static float y1_l[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static float y2_l[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static float y_l_audio = 0;
static float y1_l_audio = 0;
static float y2_l_audio = 0;
static int16_t x0_l[BAND_MAX + 1] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
static int16_t x1_l[BAND_MAX + 1] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
static int16_t x2_l[BAND_MAX + 1] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
static float y_r[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static float y1_r[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static float y2_r[BAND_MAX] = {0,0,0,0,0,0,0,0,0,0,0,0};
static float scalarFix[BAND_MAX] = {1,1,1,1,1,1,1,1,1,1,1,1};
static float y_r_audio = 0;
static float y1_r_audio = 0;
static float y2_r_audio = 0;
static int16_t x0_r[BAND_MAX + 1] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
static int16_t x1_r[BAND_MAX + 1] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
static int16_t x2_r[BAND_MAX+ 1] = {0,0,0,0,0,0,0,0,0,0,0,0,0};
static float outValue = 0;

static int coeffs_to_send[BAND_MAX];

bool bt_media_biquad_bilinear_filter(uint8_t *media, uint32_t len, uint8_t *outBuf) {
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
    // // Reset the accumulators
    // y_l = y1_l = y2_l = y_r = y1_r = y1_l = {0};

   

    // DSP FOR DISPLAY
    for(uint32_t i = 0; i < len; i+= L_R_BYTE_NUM) {
        // Grab left and right samples from within media data packet
        sample_l = (int16_t)((media[i + 1] << 8) | media[i]);
        sample_r = (int16_t)((media[i + 3] << 8) | media[i + 2]);
        // // Absolute value only
        // sample_l = sample_l < 0 ? sample_l * -1 : sample_l;
        // sample_r = sample_r < 0 ? sample_r * -1 : sample_r;
        
        sample_l_f = (float)sample_l;
        sample_r_f = (float)sample_r;
        x2_r[1] = x1_r[1];
        x1_r[1] = x0_r[1];
        x0_r[1] = sample_r_f;
        for(uint32_t j = 0; j < BAND_MAX; j++) {
            // Coefficient generation for each iteration - right channel
            y2_r[j] = y1_r[j];
            y1_r[j] = y_r[j];
            y_r[j] = (b0[j]*x0_r[1] + b2[j]*x2_r[1] - (a1[j]*y1_r[j] + a2[j]*y2_r[j])) / a0[j];
            // Audio processing - right channel
            //sample_r_f = y_r[j];
            // Clamp audio between uint16_t max limits - strictly in range [0, 65534]
           if(sample_r_f > 32767) {sample_r_f = 32767;}
           if(sample_r_f < -32768) {sample_r_f = -32768;}
        }
    }
    
    // Average the left/right coefficients to yield 12 total band coefficients
    // Note: the coefficients are converted to ints (losing all but two decimals) such that: (int)After = 10000 * (float)Before
    // WROOM will need to be able to handle this conversion
    for(int i = 0; i < BAND_MAX; i++) {
        // Magnitude of the coefficients only for visual readout
        outValue = y_r[i] < 0 ? y_r[i] * -1.0f : y_r[i];
        // Equivalent to ((coeff_l + coeff_r) / 2) * 10000
            if(i == 0)
            {
               ESP_LOGI(DSP_TAG, "Value of band %d: %.9f", i, (y_r[i]*100000.0f));
            }
        coeffs_to_send[i] = (int)((outValue));
        
        // Equivalent to ((coeff_l + coeff_r) / 2) * 1000000
    }

    // DSP FOR AUDIO
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
        x2_l[12] = x1_l[12];
        x1_l[12] = x0_l[12];
        x0_l[12] = sample_l_f;
        y_l_audio = (b0_user*x0_l[12] + b2_user*x2_l[12] - (a1_user*y1_l_audio + a2_user*y2_l_audio)) / a0_user;
        
        // Coefficient generation for each iteration - right channel
        y2_r_audio = y1_r_audio;
        y1_r_audio = y_r_audio;
        x2_r[12] = x1_r[12];
        x1_r[12] = x0_r[12];
        x0_r[12] = sample_r_f;
        y_r_audio = (b0_user*x0_r[12] + b2_user*x2_r[12] - (a1_user*y1_r_audio + a2_user*y2_r_audio)) / a0_user;

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

    // Copy over elements and free malloc'd buffer - TODO comment out when DSP works
    // memcpy(outBuf, media, len);
    // Check if some weird error occurred - very unlikely
    if(media == NULL || outBuf == NULL) {
        ESP_LOGI(DSP_TAG, "Memcpy error");
        return false;
    }
    return true;
}

void update_multipliers(uint8_t *mults) {
    // for(uint8_t i = 0; i < BAND_MAX; i++) {
    //     multipliers[i] = mults[i];
    // }
}

int get_coeff(int idx) {
    return coeffs_to_send[idx-1];
}
