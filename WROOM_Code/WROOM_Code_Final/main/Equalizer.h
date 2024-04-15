#ifndef Equalizer_H
#define Equalizer_H

#define GAP_WIDTH 2
#define BAND_WIDTH 9
#define BAND_START 5
#define GROUP_WIDTH 12
#define COEFF_MIN 0
#define COEFF_MAX 10000000  // experimental max (remember this is 100*actual val)


#define COEFF_MIN_1 255000
#define COEFF_MAX_1 400000
#define COEFF_MIN_2 1500
#define COEFF_MAX_2 25000
#define COEFF_MIN_3 10000
#define COEFF_MAX_3 20000
#define COEFF_MIN_4 30000
#define COEFF_MAX_4 40000
#define COEFF_MIN_5 40000
#define COEFF_MAX_5 58000
#define COEFF_MIN_6 60000
#define COEFF_MAX_6 70000
#define COEFF_MIN_7 100000
#define COEFF_MAX_7 125000
#define COEFF_MIN_8 90000
#define COEFF_MAX_8 108000
// #define COEFF_MIN_9 80000
// #define COEFF_MAX_9 90000
// #define COEFF_MIN_10 70000
// #define COEFF_MAX_10 78000
// #define COEFF_MIN_11 110000
// #define COEFF_MAX_11 125000
// #define COEFF_MIN_12 600000
// #define COEFF_MAX_12 630000

#define ATTEN_1 4
#define ATTEN_2 40 / 3
#define ATTEN_3 20 / 6
#define ATTEN_4 6
#define ATTEN_5 4
#define ATTEN_6 5 / 3
#define ATTEN_7 4 / 3
#define ATTEN_8 4 / 3
// #define ATTEN_9 1
// #define ATTEN_10 1
// #define ATTEN_11 1
// #define ATTEN_12 1

extern int coeff_1;
extern int coeff_2;
extern int coeff_3;
extern int coeff_4;
extern int coeff_5;
extern int coeff_6;
extern int coeff_7;
extern int coeff_8;
// extern int coeff_9;
// extern int coeff_10;
// extern int coeff_11;
// extern int coeff_12;

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
            // Check if this is a new band
            if(x % (BAND_WIDTH + GAP_WIDTH) == BAND_START) {
                new_band_flag = true;
            }
            // Check if x is in a gap
            else if(x < BAND_START || x > VPANEL_W - BAND_START - 1 || (x - BAND_START - 1) % (BAND_WIDTH + GAP_WIDTH) >= BAND_WIDTH) {
                continue;
            }
            else {
                // Only update height when moving to new band
                if(new_band_flag) {
                    new_band_flag = false;
                    // curr_y_max = curr_band_num == 12 ? coeff_12 * ATTEN_12 : curr_band_num == 11 ? coeff_11 * ATTEN_11 :
                    //             curr_band_num == 10 ? coeff_10 * ATTEN_10 : curr_band_num == 9 ? coeff_9 * ATTEN_9 :
                    //             curr_band_num == 8 ? coeff_8 * ATTEN_8 : curr_band_num == 7 ? coeff_7 * ATTEN_7 :
                    //             curr_band_num == 6 ? coeff_6 * ATTEN_6 : curr_band_num == 5 ? coeff_5 * ATTEN_5 :
                    //             curr_band_num == 4 ? coeff_4 * ATTEN_4 : curr_band_num == 3 ? coeff_3 * ATTEN_3 :
                    //             curr_band_num == 2 ? coeff_2 * ATTEN_2 : coeff_1 * ATTEN_1;
                    // curr_y_max = curr_band_num == 12 ? coeff_12 : curr_band_num == 11 ? coeff_11 :
                    //             curr_band_num == 10 ? coeff_10 : curr_band_num == 9 ? coeff_9 :
                    //             curr_band_num == 8 ? coeff_8 : curr_band_num == 7 ? coeff_7 :
                    //             curr_band_num == 6 ? coeff_6 : curr_band_num == 5 ? coeff_5 :
                    //             curr_band_num == 4 ? coeff_4 : curr_band_num == 3 ? coeff_3 :
                    //             curr_band_num == 2 ? coeff_2 : coeff_1;
                    curr_y_max = curr_band_num == 8 ? coeff_8 : curr_band_num == 7 ? coeff_7 :
                        curr_band_num == 6 ? coeff_6 : curr_band_num == 5 ? coeff_5 :
                        curr_band_num == 4 ? coeff_4 : curr_band_num == 3 ? coeff_3 :
                        curr_band_num == 2 ? coeff_2 : coeff_1;
                                
                    // Recall that coefficient was multiplied by 100, and is really a float.
                    // But the ratio of given coefficient to the max value takes care of this - it's just a fraction
                    // Also standardize to value in range [0, 31]
                    // Place within each coefficient's min and max values
                    curr_y_max = curr_band_num == 8 ? ((curr_y_max - COEFF_MIN_8) * VPANEL_H) / (COEFF_MAX_8 - COEFF_MIN_8) :
                                 curr_band_num == 7 ? ((curr_y_max - COEFF_MIN_7) * VPANEL_H) / (COEFF_MAX_7 - COEFF_MIN_7) :
                                 curr_band_num == 6 ? ((curr_y_max - COEFF_MIN_6) * VPANEL_H) / (COEFF_MAX_6 - COEFF_MIN_6) :
                                 curr_band_num == 5 ? ((curr_y_max - COEFF_MIN_5) * VPANEL_H) / (COEFF_MAX_5 - COEFF_MIN_5) :
                                 curr_band_num == 4 ? ((curr_y_max - COEFF_MIN_4) * VPANEL_H) / (COEFF_MAX_4 - COEFF_MIN_4) :
                                 curr_band_num == 3 ? ((curr_y_max - COEFF_MIN_3) * VPANEL_H) / (COEFF_MAX_3 - COEFF_MIN_3) :
                                 curr_band_num == 2 ? ((curr_y_max - COEFF_MIN_2) * VPANEL_H) / (COEFF_MAX_2 - COEFF_MIN_2) :
                                 ((curr_y_max - COEFF_MIN_1) * VPANEL_H) / (COEFF_MAX_1 - COEFF_MIN_1);
                    // Increment next band number
                    curr_band_num++;

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