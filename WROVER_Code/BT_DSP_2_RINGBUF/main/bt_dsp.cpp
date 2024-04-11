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
static float y_l_audio = 0;
static float y1_l_audio = 0;
static float y2_l_audio = 0;
static int16_t x0_l = 0;
static int16_t x1_l = 0;
static int16_t x2_l = 0;
static float y_r_audio = 0;
static float y1_r_audio = 0;
static float y2_r_audio = 0;
static int16_t x0_r = 0;
static int16_t x1_r = 0;
static int16_t x2_r = 0;

// Visual
int bandValues[BAND_MAX] = {0};
double vReal[COEFF_SAMPLE_MAX];
double vImag[COEFF_SAMPLE_MAX];
ArduinoFFT FFT = ArduinoFFT<double>(vReal, vImag, COEFF_SAMPLE_MAX, 44100);

static int coeffs_to_send[BAND_MAX];

bool bt_media_biquad_bilinear_filter(const uint8_t *media, uint32_t len, uint8_t *outBuf) {
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
    // Reset the band values
    memset(bandValues, 0, sizeof(bandValues));
    // Fill the Real Vector
    for(int i = 0; i < COEFF_SAMPLE_MAX; i++) {
        // Rely on average between channels
        vReal[i] = (double)((((media[i + 1] << 8) | media[i]) + ((media[i + 3] << 8) | media[i + 2]) ) / 2);
        vImag[i] = 0;
    }
    // Compute FFT
    FFT.dcRemoval();
    FFT.windowing(FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.compute(FFT_FORWARD);
    FFT.complexToMagnitude();

    // Analyze FFT results
    for(int i = 2; i < (COEFF_SAMPLE_MAX / 2); i++) {
        if(vReal[i] > 500) {  // Crude noise filter
            if(i <= 2) {
                bandValues[0] += (int)vReal[i];
            }
            else if(i == 3) {
                bandValues[1] += (int)vReal[i];
            }
            else if(i <= 5) {
                bandValues[2] += (int)vReal[i];
            }
            else if(i <= 7) {
                bandValues[3] += (int)vReal[i];
            }
            else if(i <= 3) {
                bandValues[1] += (int)vReal[i];
            }
            else if(i == 3) {
                bandValues[1] += (int)vReal[i];
            }
            else if(i == 3) {
                bandValues[1] += (int)vReal[i];
            }
            else if(i == 3) {
                bandValues[1] += (int)vReal[i];
            }
            else if(i == 3) {
                bandValues[1] += (int)vReal[i];
            }
            else if(i == 3) {
                bandValues[1] += (int)vReal[i];
            }
            else if(i == 3) {
                bandValues[1] += (int)vReal[i];
            }

        }
    }

    // DSP FOR DISPLAY
    for(uint32_t i = 0; i < COEFF_SAMPLE_MAX; i++) {
        // Grab left and right samples from within media data packet
        sample_l = (int16_t)((media[i + 1] << 8) | media[i]);
        sample_r = (int16_t)((media[i + 3] << 8) | media[i + 2]);
        // // Absolute value only
        // sample_l = sample_l < 0 ? sample_l * -1 : sample_l;
        // sample_r = sample_r < 0 ? sample_r * -1 : sample_r;
        
        sample_l_f = (float)sample_l;
        sample_r_f = (float)sample_r;
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
    for(int i = 0; i < BAND_MAX; i++) {
        // Magnitude of the coefficients only for visual readout
        y_l[i] = y_l[i] < 0 ? y_l[i] * -1.0f : y_l[i];
        y_r[i] = y_r[i] < 0 ? y_r[i] * -1.0f : y_r[i];
        // Equivalent to ((coeff_l + coeff_r) / 2) * 100
        coeffs_to_send[i] = (int)((y_l[i] + y_r[i]) * 50.0f);
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
        x2_l = x1_l;
        x1_l = x0_l;
        x0_l = sample_l_f;
        y_l_audio = (b0_user*x0_l + b2_user*x2_l - (a1_user*y1_l_audio + a2_user*y2_l_audio)) / a0_user;
        
        // Coefficient generation for each iteration - right channel
        y2_r_audio = y1_r_audio;
        y1_r_audio = y_r_audio;
        x2_r = x1_r;
        x1_r = x0_r;
        x0_r = sample_r_f;
        y_r_audio = (b0_user*x0_r + b2_user*x2_r - (a1_user*y1_r_audio + a2_user*y2_r_audio)) / a0_user;

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