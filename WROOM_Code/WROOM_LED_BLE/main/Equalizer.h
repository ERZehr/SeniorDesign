#ifndef Equalizer_H
#define Equalizer_H

#define GAP_WIDTH 1
#define BAND_WIDTH 6
#define GROUP_WIDTH 8
#define COEFF_MIN 0
#define COEFF_MAX 400000 * 13  // TODO: probably not correct (remember this is 100*actual val)
#define COEFF_1 2
#define COEFF_2 4
#define COEFF_3 6
#define COEFF_4 8
#define COEFF_5 10
#define COEFF_6 12
#define COEFF_7 14
#define COEFF_8 16
#define COEFF_9 18
#define COEFF_10 20
#define COEFF_11 22
#define COEFF_12 24

#define ATTEN_1 4
#define ATTEN_2 40 / 3
#define ATTEN_3 20 / 3
#define ATTEN_4 6
#define ATTEN_5 4
#define ATTEN_6 5 / 3
#define ATTEN_7 4 / 3
#define ATTEN_8 4 / 3
#define ATTEN_9 1
#define ATTEN_10 1
#define ATTEN_11 1
#define ATTEN_12 1

// TODO: connect extern coeffs
extern int coeff_1;
extern int coeff_2;
extern int coeff_3;
extern int coeff_4;
extern int coeff_5;
extern int coeff_6;
extern int coeff_7;
extern int coeff_8;
extern int coeff_9;
extern int coeff_10;
extern int coeff_11;
extern int coeff_12;

class Equalizer : public Drawable {
private:
    int time = 0;
    int cycles = 0;

public:
    Equalizer() {
        name = (char *)"Equalizer";
    }

    byte hue = 0;

    unsigned int drawFrame() {
        effects.ClearFrame();
        int curr_band_num = 1;
        int curr_y_max = 0;
        bool new_band_flag = true;
        for (int x = VPANEL_W - 1; x >= 0; x--) {
            // Check if this is a gap - if not, continue with next pixel column
            if(!(x != 0 && x % GROUP_WIDTH < BAND_WIDTH + GAP_WIDTH)) {
                new_band_flag = true;
                continue;
            }
            else {
                // TODO: determine y_max experimentally by multiplying coeff by certain constant
                // Only update height when moving to new band
                if(new_band_flag) {
                    new_band_flag = false;
                    // DEBUGGING
                    // curr_y_max = curr_band_num == 1 ? COEFF_1 : curr_band_num == 2 ? COEFF_2 :
                    //             curr_band_num == 3 ? COEFF_3 : curr_band_num == 4 ? COEFF_4 :
                    //             curr_band_num == 5 ? COEFF_5 : curr_band_num == 6 ? COEFF_6 :
                    //             curr_band_num == 7 ? COEFF_7 : curr_band_num == 8 ? COEFF_8 :
                    //             curr_band_num == 9 ? COEFF_9 : curr_band_num == 10 ? COEFF_10 :
                    //             curr_band_num == 11 ? COEFF_11 : COEFF_12;
                                
                    curr_y_max = curr_band_num == 1 ? coeff_1 * ATTEN_1 : curr_band_num == 2 ? coeff_2 * ATTEN_2 :
                                curr_band_num == 3 ? coeff_3 * ATTEN_3 : curr_band_num == 4 ? coeff_4 * ATTEN_4 :
                                curr_band_num == 5 ? coeff_5 * ATTEN_5 : curr_band_num == 6 ? coeff_6 * ATTEN_6 :
                                curr_band_num == 7 ? coeff_7 * ATTEN_7 : curr_band_num == 8 ? coeff_8 * ATTEN_8 :
                                curr_band_num == 9 ? coeff_9 * ATTEN_9 : curr_band_num == 10 ? coeff_10 * ATTEN_10 :
                                curr_band_num == 11 ? coeff_11 * ATTEN_11 : coeff_12 * ATTEN_12;
                    // Increment next band number
                    curr_band_num++;

                    // Recall that coefficient was multiplied by 100, and is really a float.
                    // But the ratio of given coefficient to the max value takes care of this - it's just a fraction
                    // Also standardize to value in range [0, 31]
                    // TODO: uncomment
                    curr_y_max = ((curr_y_max - COEFF_MIN) * VPANEL_H) / (COEFF_MAX - COEFF_MIN);
                }
                
            }
            for (int y = 0; y < VPANEL_H; y++) {
                // Cut off drawing at the coefficient-dictated level
                if(y >= curr_y_max) {
                    break;
                }
                // Draw height of each band
                effects.drawBackgroundFastLEDPixelCRGB(x, y, effects.ColorFromCurrentPalette(hue));
            }
        }

        EVERY_N_MILLIS(200) {
            hue++;
        }

        effects.ShowFrame();

        // TODO: find good return value - adds to delay ms in main logic
        return 20;
    }
};

#endif