/*
  BLE Code based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
  Ported to Arduino ESP32 by Evandro Copercini
  Ported to be compatible with VSCode ESP-IDF Extension (C++) by Bailey Mosher

  LED Matrix Code based on ESP32-HUB75-MatrixPanel-DMA example: https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-DMA/tree/master/examples/ChainedPanelsAuroraDemo
  Ported to be compatible with VSCode ESP-IDF Extension (C++) by Bailey Mosher
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"

// #include "esp_adc/adc_oneshot.h"
#include <driver/adc.h>

// NOTE: includes for LED Matrix are under the comment header: LED MATRIX VARIABLE DECLARATIONS
// This is done so that the Matrix object can be easily handed off to dependent files within main/

/********************************
 * DISPLAY CONFIGURATION DEFINES
 *******************************/
// NOTE: These defines are used in Effects, Drawable, and Patterns. Thus, they must be declared before those includes
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
#define VPANEL_H PANEL_RES_Y*NUM_ROWS
// Display is either in Idle Mode, or in Playback Mode
#define DISPLAY_IDLE_MODE true  // TODO: parse a field in WROVER-sent UART command to determine this var's state
// Set initial brightness - 192 ~ 75% brightness which is plenty for most conditions
#define INIT_BRIGHT 192
#define MAX_BRIGHT 255
#define INIT_PATTERN "Life"

/********************************
 * HUB75 -> GPIO DEFINES
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
#define CH_E -1 // UNUSED - always -1 for our 32x32 matrices
#define CLK 18
#define STB 4
#define OE 15

/********************************
 * BLE UUID DEFINES
 *******************************/
  // See the following for generating UUIDs:
  // https://www.uuidgenerator.net/
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

/********************************
 * ADC DEFINES
 *******************************/
#define LIGHT_SENSOR_MIN 0
#define LIGHT_SENSOR_MAX 205  // TODO: experimentally determine this

/********************************
 * LED MATRIX VARIABLE DECLARATIONS
 *******************************/
#include "ESP32-VirtualMatrixPanel-I2S-DMA.h"
#include "FastLED.h"  // Used for some mathematics calculations and effects.

// Custom pin mapping
HUB75_I2S_CFG::i2s_pins _pins={R_1, G_1, B_1, R_2, G_2, B_2, CH_A, CH_B, CH_C, CH_D, CH_E, STB, OE, CLK};
// Configure matrix setup
static HUB75_I2S_CFG mxconfig(PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN, _pins);
// Create the matrix object
MatrixPanel_I2S_DMA *matrix = new MatrixPanel_I2S_DMA(mxconfig);
// placeholder for the virtual display object
VirtualMatrixPanel *virtualDisp = nullptr;

// Include the Effects class and declare it (for use in Patterns)
#include "Effects.h"
Effects effects;

#include "Drawable.h"
#include "Patterns.h"
Patterns patterns;

// State machine-esque control vars over LED brightness
static uint8_t curr_bright = INIT_BRIGHT;
static uint8_t prev_bright = INIT_BRIGHT;

// Vars needed for timing of frame-drawing
unsigned long ms_current  = 0;
unsigned long ms_previous = 0;
unsigned long ms_animation_max_duration = 20000; // 10 seconds
unsigned long next_frame = 0;

/********************************
 * BLE VARIABLE DECLARATIONS
 *******************************/
BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

/********************************
 * ADC VARIABLE DECLARATIONS
 *******************************/
static int adc_raw_val;

/********************************
 * BLE CLASS DEFINITIONS
 *******************************/
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      String rxValue = pCharacteristic->getValue();
      int rxLen = rxValue.length();
      // Use ESP_LOGE for now...
      if (rxLen > 0) {
        const char* rxValueCharArr = rxValue.c_str();
        ESP_LOGI("RXValue", "*********");
        ESP_LOGI("RXValue", "Received Value: %s", rxValueCharArr);
        ESP_LOGI("RXValue", "*********");
      }
    }
};

/********************************
 * LED MATRIX STATIC FUNCTIONS
 *******************************/
// Advances pattern to next in rotation
static void patternAdvance(){
    // Go to next pattern in the list (see Patterns.h)
    patterns.stop();
    patterns.move(1);
    patterns.start();
}

/********************************
 * ADC STATIC FUNCTIONS
 *******************************/
// Update the current brightness in accordance with brightness thresholds
static void updateCurrBright(int new_bright_val){
  // Quickly determine where new value sits within thresholds and update curr_bright
  curr_bright = new_bright_val < BRIGHT_LVL_1 ? BRIGHT_LVL_0 :
                new_bright_val < BRIGHT_LVL_2 ? BRIGHT_LVL_1 :
                new_bright_val < BRIGHT_LVL_3 ? BRIGHT_LVL_2 :
                new_bright_val < BRIGHT_LVL_4 ? BRIGHT_LVL_3 :
                new_bright_val < BRIGHT_LVL_5 ? BRIGHT_LVL_4 :
                new_bright_val < BRIGHT_LVL_6 ? BRIGHT_LVL_5 :
                new_bright_val < BRIGHT_LVL_7 ? BRIGHT_LVL_6 : BRIGHT_LVL_7;
}

/********************************
 * TASK HANDLING - ALL FUNCTIONS
 *******************************/
static TaskHandle_t s_matrix_driving_task_handle = NULL;  /* handle of driving task  */
static TaskHandle_t s_rx_to_config_handle = NULL;  /* handle of config task  */
static TaskHandle_t s_matrix_bright_handle = NULL;  /* handle of brightness task  */

static void matrix_driving_handler(void *arg) {
  for(;;) {
    // Delay until next cycle
    vTaskDelay(1 / portTICK_PERIOD_MS);

    if(DISPLAY_IDLE_MODE) {
      ms_current = esp_timer_get_time() / 1000;
      if ((ms_current - ms_previous) > ms_animation_max_duration) {
        patternAdvance();
        ms_previous = ms_current;
      }
  
      if ( next_frame < ms_current) {
        next_frame = patterns.drawFrame() + ms_current;
      }
    }
    else {
      // TODO: real-time graphic eq display logic
    }

  }
}

static void rx_to_config_handler(void *arg) {
  for(;;) {
    // Delay until next cycle
    vTaskDelay(1 / portTICK_PERIOD_MS);

    // Somewhere in here need to have logic for palette changing
    // setPalette("Ocean");
  }
}

static void matrix_bright_handler(void *arg) {
  for(;;) {
    // Delay for about 5 seconds - too frequent reading yields poor output stability
    vTaskDelay(pdMS_TO_TICKS(5000));

    // Read raw ADC value
    adc_raw_val = adc1_get_raw(ADC1_CHANNEL_0);

    // TESTING: ESP_LOG the percentage the light sensor is experiencing
    updateCurrBright(adc_raw_val * MAX_BRIGHT) / LIGHT_SENSOR_MAX;

    // TODO: link ADC-read value to discrete brightness settings
    // curr_bright = TODO: read in light sensor value
    // Only attempt to update brightness if there is a change - no need otherwise
    // if(curr_bright != prev_bright) {
    //   matrix -> setBrightness8(curr_bright);
    // }
  }
}

/********************************
 * MAIN ENTRY POINT
 *******************************/
extern "C" void app_main(void)
{
  initArduino();
  // Start the DMA display
  if(!matrix->begin()) {
    ESP_LOGE("DMA_I2S", "I2S Memory Allocation Failed!");
  }

  // Initialize brightness
  // TODO: (to be adjusted further by light sensor)
  matrix -> setBrightness8(INIT_BRIGHT);
  // Create VirtualDisplay object based on our newly created dma_display object
  virtualDisp = new VirtualMatrixPanel((*matrix), NUM_ROWS, NUM_COLS, PANEL_RES_X, PANEL_RES_Y, CHAIN_TOP_LEFT_DOWN);

   // Setup the effects generator
  effects.Setup();
  // Start the patterns
  patterns.setPattern(INIT_PATTERN);
  patterns.start();

  // Create the BLE Device
  BLEDevice::init("ESP32_BLE"); // Give it a name
  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID_TX,
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  pCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                        CHARACTERISTIC_UUID_RX,
                                        BLECharacteristic::PROPERTY_WRITE
                                      );
  pCharacteristic->setCallbacks(new MyCallbacks());
  // Start the service
  pService->start();
  // Start advertising
  pServer->getAdvertising()->start();
  ESP_LOGI("BLEClient", "Waiting on a client connection to notify...");

  // Initialize ADC1 (values from 0-4095)
  adc1_config_width(ADC_WIDTH_BIT_12);
  // Configure ADC1 to use Channel 0 (full-scale voltage to 1.5V)
  adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_2_5);
  
  // Create the task that handles LED matrix driving
  xTaskCreate(matrix_driving_handler, "LEDMatrixTask", 3072, NULL, 2, &s_matrix_driving_task_handle);
  // Create the task that handles config settings updating
  // xTaskCreate(rx_to_config_handler, "ConfigValueTask", 2048, NULL, 10, &s_rx_to_config_handle);
  // Create the task that handles matrix brightness updating
  // xTaskCreate(matrix_bright_handler, "BrightTask", 2048, NULL, 11, &s_matrix_bright_handle);
}