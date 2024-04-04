/*
  BLE Code based on Neil Kolban example for IDF: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/tests/BLE%20Tests/SampleNotify.cpp
  Ported to Arduino ESP32 by Evandro Copercini
  Ported to be compatible with VSCode ESP-IDF Extension (C++) by Bailey Mosher

  LED Matrix Code based on ESP32-HUB75-MatrixPanel-DMA example: https://github.com/mrfaptastic/ESP32-HUB75-MatrixPanel-DMA/tree/master/examples/ChainedPanelsAuroraDemo
  Ported to be compatible with VSCode ESP-IDF Extension (C++) by Bailey Mosher
*/
// Standard includes
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

// BLE Includes
#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLE2902.h"

// ADC Includes
#include <driver/adc.h>

// UART Includes
#include "driver/uart.h"
#include "driver/gpio.h"

// NOTE: includes for LED Matrix are under the comment header: LED MATRIX VARIABLE DECLARATIONS
// This is done so that the Matrix object can be easily handed off to dependent files within main/ directory

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

/********************************
 * ADC DEFINES
 *******************************/
#define LIGHT_SENSOR_MIN 1100
#define LIGHT_SENSOR_MAX 2000 
#define BRIGHT_LVL_0 40
#define BRIGHT_LVL_1 95
#define BRIGHT_LVL_2 135
#define BRIGHT_LVL_3 255

/********************************
 * UART DEFINES
 *******************************/
#define WROOM_UART_BUF_SIZE 1024
#define WROOM_UART_BAUD 115200
#define WROOM_UART_PORT_NUM 2
#define WROOM_UART_RX 16
#define WROOM_UART_TX 17
#define WROOM_UART_STACK_SIZE 2048
#define WROOM_EXPECTED_RX_VALS 13

// WROVER-Controlled Variables
static int coeff_1;
static int coeff_2;
static int coeff_3;
static int coeff_4;
static int coeff_5;
static int coeff_6;
static int coeff_7;
static int coeff_8;
static int coeff_9;
static int coeff_10;
static int coeff_11;
static int coeff_12;

static int disp_idle_mode;

// TODO delete these once we have good coeffs
// WROOM-Controlled Variables
static int fir_1 = 11;
static int fir_2 = 12;
static int fir_3 = 13;
static int fir_4 = 14;
static int fir_5 = 15;

/********************************
 * LED MATRIX VARIABLE DECLARATIONS
 *******************************/
// LED Matrix Includes
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
static int adc_avg_val;

/********************************
 * BLE CLASS DEFINITIONS
 *******************************/
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      // Start advertising
      pServer->getAdvertising()->start();
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
 * LED MATRIX STATIC FUNCTION DEFINITIONS
 *******************************/
// Advances pattern to next in rotation
static void patternAdvance(){
    // Go to next pattern in the list (see Patterns.h)
    patterns.stop();
    patterns.move(1);
    patterns.start();
}

/********************************
 * ADC STATIC FUNCTION DEFINITIONS
 *******************************/
// Update the current brightness in accordance with brightness thresholds
static void updateCurrBright(int new_bright_val){
  // Quickly determine where new value sits within thresholds and update curr_bright
  curr_bright = new_bright_val < BRIGHT_LVL_1 ? BRIGHT_LVL_0 :  // Too small of threshold - has to be REALLY dark to fully shut off
                new_bright_val < BRIGHT_LVL_2 ? BRIGHT_LVL_1 :
                new_bright_val < BRIGHT_LVL_3 ? BRIGHT_LVL_2 : BRIGHT_LVL_3;
}

/********************************
 * UART STATIC FUNCTION DECLARATIONS
 *******************************/
/* UART configuration function - intended to be called from app_main */
static void wroom_uart_init();
/* Processing function for RX data */
static void update_coeff_vals(uint8_t* data, int len);
/* Populating function for TX data */
static bool populate_tx_buf(uint8_t* data, int len);
/* RX Task - Data received from WROVER */
static void wroom_rx_task(void *arg);
/* TX Task - Data sent to WROVER */
static void wroom_tx_task(void *arg);

/********************************
 * UART STATIC FUNCTION DEFINITIONS
 *******************************/
static TaskHandle_t s_wroom_rx_handle = NULL;  /* handle of wroom rx task  */
static TaskHandle_t s_wroom_tx_handle = NULL;  /* handle of wroom tx task */

static void wroom_uart_init() {
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t wroom_uart_config = {
        .baud_rate = WROOM_UART_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_driver_install(WROOM_UART_PORT_NUM, WROOM_UART_BUF_SIZE * 2, 0, 0, NULL, 0));
    ESP_ERROR_CHECK(uart_param_config(WROOM_UART_PORT_NUM, &wroom_uart_config));
    // Ensure for WROOM: TX : GPIO17, RX : GPIO16 - don't use RTS/CTS
    ESP_ERROR_CHECK((uart_set_pin(WROOM_UART_PORT_NUM, WROOM_UART_TX, WROOM_UART_RX, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE)));

    // Start a task for both the RX and TX
    xTaskCreate(wroom_rx_task, "wroom_rx_task", WROOM_UART_STACK_SIZE, NULL, 20, &s_wroom_rx_handle);
    xTaskCreate(wroom_tx_task, "wroom_tx_task", WROOM_UART_STACK_SIZE, NULL, 21, &s_wroom_tx_handle);
}

static void update_coeff_vals(uint8_t* data, int len) {
    // TODO ensure accurate number of sent values
    // Begin parsing received JSON-like string
    // EXPECTED FORMAT (JSON-like):
    /*
        {
            "COEFFS" : {
                            "C1" : 1,
                            "C2" : 2,
                            .....
                            "C12" : 12
            },
            "IDLE MODE" : 0
        }
    */
  //  // DEBUGGING ONLY - comment out otherwise, it's unnecessary
  //  coeff_1 = coeff_2 = coeff_3 = coeff_4 = coeff_5 = coeff_6 = 0;
  //  coeff_7 = coeff_8 = coeff_9 = coeff_10 = coeff_11 = coeff_12 = 0;
  //  // DEBUGGING ONLY - comment out otherwise, it's unnecessary

    sscanf((const char*)data, "{\"COEFFS\" : {\"C1\" : %d, \"C2\" : %d, \"C3\" : %d, \"C4\" : %d, \"C5\" : %d, \"C6\" : %d, \"C7\" : %d, \"C8\" : %d, \"C9\" : %d, \"C10\" : %d, \"C11\" : %d, \"C12\" : %d}, \"IDLE MODE\" : %d}",
           &coeff_1, &coeff_2, &coeff_3, &coeff_4, &coeff_5, &coeff_6, &coeff_7, &coeff_8, &coeff_9, &coeff_10, &coeff_11, &coeff_12, &disp_idle_mode);                      

    // // DEBUGGING - values should NOT be 0
    // ESP_LOGI("UART", "COEFFS:\nC1:%d,\tC2:%d,\tC3:%d\nC4:%d,\tC5:%d,\tC6:%d\nC7:%d,\tC8:%d,\tC9:%d\nC10:%d,\tC11:%d,\tC12:%d\n\n",
    //          coeff_1, coeff_2, coeff_3, coeff_4, coeff_5, coeff_6, coeff_7, coeff_8, coeff_9, coeff_10, coeff_11, coeff_12);
}

static bool populate_tx_buf(uint8_t* data, int len) {
    // TODO - actually grab coefficients (delete predefined ones at the top of the file)
    // Use snprintf() to quickly and safely place coefficients in data buffer
    // EXPECTED FORMAT (JSON-like):
    /*
        {
            "FIRS" : {
                            "F1" : 1,
                            "F2" : 2,
                            .....
                            "F5" : 5
            }
        }
    */
    int written = snprintf((char*)data, len, "{\"FIRS\" : {\"F1\" : %d, \"F2\" : %d, \"F3\" : %d, \"F4\" : %d, \"F5\" : %d}}",
                           fir_1, fir_2, fir_3, fir_4, fir_5);

    // Error if not enough space for bytes, or if 0 or ERROR bytes are written
    if(written > len || written < 1) {
        return false;
    }
    return true;
}

/********************************
 * TASK HANDLING - ALL FUNCTIONS
 *******************************/
static TaskHandle_t s_matrix_driving_task_handle = NULL;  /* handle of driving task  */
static TaskHandle_t s_matrix_bright_handle = NULL;  /* handle of brightness task  */

static void matrix_driving_handler(void *arg) {
  for(;;) {
    // Delay until next cycle
    vTaskDelay(1 / portTICK_PERIOD_MS);

    // TODO: if(disp_idle_mode)
    if(true) {
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

static void matrix_bright_handler(void *arg) {
  for(;;) {
    // Delay for about 5 seconds - too frequent reading yields poor output stability
    vTaskDelay(pdMS_TO_TICKS(2000));

    // Read raw ADC value, average over 10 samples
    uint16_t adc_raw_val = 0;
    for(int i = 0; i < 10; i++) {
      adc_raw_val += analogRead(36);
      vTaskDelay(pdMS_TO_TICKS(5));
    }
    adc_avg_val = adc_raw_val / 10;

    // (NEW - LOW) * MAX_BRIGHT / (HIGH - LOW)
    updateCurrBright(((adc_avg_val - LIGHT_SENSOR_MIN) * MAX_BRIGHT) / (LIGHT_SENSOR_MAX - LIGHT_SENSOR_MIN));
    // ESP_LOGI("ADC", "RAW: %d", adc_avg_val);
    // Only attempt to update brightness if there is a change - no need otherwise
    if(curr_bright != prev_bright) {
      matrix -> setBrightness8(curr_bright);
      prev_bright = curr_bright;
    }
  }
}

static void wroom_rx_task(void *arg) {
    // Allocate a buffer for incoming data
    uint8_t* rx_data = (uint8_t*) malloc(WROOM_UART_BUF_SIZE);

    for(;;) {
        // Give enough of a delay before reading
        // Read in a maximum of (BUF_SIZE-1) bytes - leave space for a terminating '\0'
        int byte_len = uart_read_bytes(WROOM_UART_PORT_NUM, rx_data, (WROOM_UART_BUF_SIZE - 1), 20 / portTICK_PERIOD_MS);

        // Coeff values are at LEAST 160 characters long
        if(byte_len > 160) {
            // Terminate the byte array, and send for processing
            rx_data[byte_len] = '\0';
            update_coeff_vals(rx_data, byte_len);
        }
    }
    free(rx_data);
}

static void wroom_tx_task(void *arg) {
    // Allocate a buffer for outgoing data
    uint8_t* tx_data = (uint8_t*) malloc(WROOM_UART_BUF_SIZE);

    for(;;) {
        if(!populate_tx_buf(tx_data, WROOM_UART_BUF_SIZE)) {
            ESP_LOGE("UART", "TX FIR Populating Failed");
            continue;
        }
        // Give enough of a delay before sending - about 1 time per second
        vTaskDelay(pdMS_TO_TICKS(1000));
        int err = uart_write_bytes(WROOM_UART_PORT_NUM, (const char*)tx_data, WROOM_UART_BUF_SIZE);

        if(err == -1) {
            ESP_LOGE("UART", "TX Send Failed");
        }
    }
    free(tx_data);
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

  // Initialize brightness - adjusted further in runtime by light sensor
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
  
  // Create the task that handles LED matrix driving
  xTaskCreate(matrix_driving_handler, "LEDMatrixTask", 3072, NULL, 2, &s_matrix_driving_task_handle);
  // Create the task that handles matrix brightness updating
  xTaskCreate(matrix_bright_handler, "BrightTask", 3072, NULL, 11, &s_matrix_bright_handle);
  // Start the UART channel
  wroom_uart_init();
}