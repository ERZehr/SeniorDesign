#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

/********************************
 * HUB75 -> GPIO Mappings
 *******************************/
#define R_1 25
#define G_1 26
#define B_1 27
#define R_2 14
#define G_2 12
#define B_2 13
#define CH_A 23
#define CH_B 19
#define CH_C 5
#define CH_D 33
#define CH_E -1 // UNUSED - always -1 for 32x32 matrix
#define CLK 18
#define STB 4
#define OE 15

/* -------------------------- Display Config Initialisation -------------------- */
#define PANEL_RES_X 32 // Number of pixels wide of each INDIVIDUAL panel module. 
#define PANEL_RES_Y 32 // Number of pixels tall of each INDIVIDUAL panel module.

#define NUM_ROWS 1 // Number of rows of chained INDIVIDUAL PANELS
#define NUM_COLS 3 // Number of INDIVIDUAL PANELS per ROW
#define PANEL_CHAIN NUM_ROWS*NUM_COLS    // total number of panels chained one to another

// Change this to your needs, for details on VirtualPanel pls see ChainedPanels example
#define SERPENT false
#define TOPDOWN false

// Virtual Panel dimensions
#define VPANEL_W PANEL_RES_X*NUM_COLS // Kosso: All Pattern files have had the MATRIX_WIDTH and MATRIX_HEIGHT replaced by these.
#define VPANEL_H PANEL_RES_Y*NUM_ROWS //

/* -------------------------- Class Initialisation -------------------------- */
#include "ESP32-VirtualMatrixPanel-I2S-DMA.h"
#include "FastLED.h"  // Used for some mathematics calculations and effects.

// placeholder for the matrix object
MatrixPanel_I2S_DMA *matrix = nullptr;

// placeholder for the virtual display object
VirtualMatrixPanel  *virtualDisp = nullptr;

// Aurora related
#include "Effects.h"
Effects effects;

#include "Drawable.h"
// #include "Playlist.h"
//#include "Geometry.h"
#include "Patterns.h"
Patterns patterns;

/* -------------------------- Some timing variables -------------------------- */
unsigned long ms_current  = 0;
unsigned long ms_previous = 0;
unsigned long ms_animation_max_duration = 20000; // 10 seconds
unsigned long next_frame = 0;

static void patternAdvance(){
    // Go to next pattern in the list (se Patterns.h)
    patterns.stop();
    patterns.move(1);
    patterns.start();
}

/********************************
 * Task Handling
 *******************************/
static TaskHandle_t s_matrix_driving_task_handle = NULL;  /* handle of driving task  */

static void matrix_driving_handler(void *arg) {
  for(;;) {
    // Delay until next cycle
    vTaskDelay(1 / portTICK_PERIOD_MS);

    ms_current = esp_timer_get_time() / 1000;
    if ((ms_current - ms_previous) > ms_animation_max_duration) {
       patternAdvance();
       ms_previous = ms_current;
    }
 
    if ( next_frame < ms_current) {
      next_frame = patterns.drawFrame() + ms_current;
    }
  }
}

/********************************
 * Main Entry Point
 *******************************/
extern "C" void app_main(void)
{
  initArduino();
  // Configure matrix setup
  HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN);

  // Custom pin mapping
  HUB75_I2S_CFG::i2s_pins _pins={R_1, G_1, B_1, R_2, G_2, B_2, CH_A, CH_B, CH_C, CH_D, CH_E, STB, OE, CLK};
  mxconfig.gpio = _pins;

  // Create the matrix object
  matrix = new MatrixPanel_I2S_DMA(mxconfig);

  // Allocate memory and start DMA display
  matrix -> begin();
  // if(!matrix->begin())
    // TODO - error message (ESP_LOGE)
      // Serial.println("****** !KABOOM! I2S memory allocation failed ***********");

  // Adjust default brightness to about 75% - plenty bright for normal conditions
  // (to be adjusted further by light sensor)
  matrix -> setBrightness8(192);    // range is 0-255, 0 - 0%, 255 - 100%

  // Create VirtualDisplay object based on our newly created dma_display object
  virtualDisp = new VirtualMatrixPanel((*matrix), NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, CHAIN_TOP_LEFT_DOWN);

   // Setup the effects generator
  effects.Setup();
  
  patterns.setPattern("Life"); // start with PatternLife
  patterns.start();
    
  // Create the task that handles LED matrix driving
  xTaskCreate(matrix_driving_handler, "LEDMatrixTask", 3072, NULL, 2, &s_matrix_driving_task_handle);

}