/*****************************************
 * ECE 477 Team 8 "Artisyn" Credits:
 * 
 * Bailey Mosher:
 *    * Coefficient Magnitude -> Bar Height Mapping
 *    * Frame Display Logic
 *    * Modifiable Band Number Display Logic
 * 
 * Kaden Merrill:
 *    * Testing and Integration
 * 
*****************************************/
#ifndef Equalizer_H
#define Equalizer_H

#define COEFF_MIN 0
#define COEFF_MAX 300

#define ATTEN 2

extern int coeff_1;
extern int coeff_2;
extern int coeff_3;
extern int coeff_4;
extern int coeff_5;
extern int coeff_6;
extern int coeff_7;
extern int coeff_8;
extern int band_num;

static int GAP_WIDTH = 2;
static int BAND_START = 5;
static int BAND_WIDTH = 9;

class Equalizer : public Drawable {
private:
    int time = 0;
    int cycles = 0;

public:
    Equalizer() {
        name = (char *)"Equalizer";
    }

    byte hue = 0;

    void updateWidths() {
        switch(band_num) {
            case 2 :    GAP_WIDTH = 3; BAND_START = 10; BAND_WIDTH = 40;
                        break;
            case 3 :    GAP_WIDTH = 2; BAND_START = 7; BAND_WIDTH = 26;
                        break;
            case 4 :    GAP_WIDTH = 2; BAND_START = 5; BAND_WIDTH = 20;
                        break;
            case 5 :    GAP_WIDTH = 2; BAND_START = 4; BAND_WIDTH = 16;
                        break;
            case 6 :    GAP_WIDTH = 2; BAND_START = 4; BAND_WIDTH = 13;
                        break;
            case 7 :    GAP_WIDTH = 3; BAND_START = 4; BAND_WIDTH = 10;
                        break;
            default :   GAP_WIDTH = 2; BAND_START = 5; BAND_WIDTH = 9;
        }
    }

    unsigned int drawFrame() {
        effects.ClearFrame();
        updateWidths();
        int curr_band_num = 1;
        int curr_y_max = 0;
        bool new_band_flag = true;
        for (int x = VPANEL_W - 1; x >= 0; x--) {
            // Check if this is a new band
            if(x % (BAND_WIDTH + GAP_WIDTH) == BAND_START) {
                new_band_flag = true;
            }
            // Check if x is in a gap
            else if(x < BAND_START || x > VPANEL_W - BAND_START - 1 || (x - BAND_START) % (BAND_WIDTH + GAP_WIDTH) >= BAND_WIDTH) {
                continue;
            }
            else {
                // Only update height when moving to new band
                if(new_band_flag) {
                    new_band_flag = false;
                    curr_y_max = curr_band_num == 8 ? coeff_8 : curr_band_num == 7 ? coeff_7 :
                        curr_band_num == 6 ? coeff_6 : curr_band_num == 5 ? coeff_5 :
                        curr_band_num == 4 ? coeff_4 : curr_band_num == 3 ? coeff_3 :
                        curr_band_num == 2 ? coeff_2 : coeff_1 / 10;

                    // Standardize to value in range [0, 31]
                    curr_y_max = ((curr_y_max - COEFF_MIN) * VPANEL_H * ATTEN) / (COEFF_MAX - COEFF_MIN);
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

        // return 20;
        return 5;  // delay 5 ms before drawing next frame
    }
};

#endif