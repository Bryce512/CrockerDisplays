#ifndef BOARD_PINS_H
#define BOARD_PINS_H

// Select your board: 1 for 4" square, 2 for 2.8" round
#define BOARD_TYPE 2 // Change to 2 for 2.8" round display

// #if BOARD_TYPE == 1
//   // 4.0" Square Display (ESP32-4848S040)
//   #define BOARD_NAME "4.0 Square"
//   #define TFT_WIDTH 480
//   #define TFT_HEIGHT 480
  
//   // RGB Panel Pins for 4" Square
//   #define DE_PIN 18
//   #define VSYNC_PIN 17
//   #define HSYNC_PIN 16
//   #define PCLK_PIN 21
  
//   // Red pins
//   #define R0_PIN 11
//   #define R1_PIN 12
//   #define R2_PIN 13
//   #define R3_PIN 14
//   #define R4_PIN 0
  
//   // Green pins
//   #define G0_PIN 8
//   #define G1_PIN 20
//   #define G2_PIN 3
//   #define G3_PIN 46
//   #define G4_PIN 9
//   #define G5_PIN 10
  
//   // Blue pins
//   #define B0_PIN 4
//   #define B1_PIN 5
//   #define B2_PIN 6
//   #define B3_PIN 7
//   #define B4_PIN 15
  
//   // Touch pins (GT911 i2c)
//   #define TOUCH_SDA 19
//   #define TOUCH_SCL 45
//   #define TOUCH_INT -1
//   #define TOUCH_RST -1  
  
//   // Backlight
//   #define BL_PIN 38
  
//   // RGB Panel timing parameters (4" square)
//   #define HSYNC_POLARITY 1
//   #define HSYNC_FRONT_PORCH 10
//   #define HSYNC_PULSE_WIDTH 8
//   #define HSYNC_BACK_PORCH 50
//   #define VSYNC_POLARITY 1
//   #define VSYNC_FRONT_PORCH 10
//   #define VSYNC_PULSE_WIDTH 8
//   #define VSYNC_BACK_PORCH 20
  
//   // PCLK frequency for 4" square
//   // #define TFT_PCLK_FREQ_MHZ 16

// #elif BOARD_TYPE == 2
  // 2.8" Round Display (ESP32-S3-8048S070)
  #define BOARD_NAME "2.8 Round"
  #define TFT_WIDTH 480
  #define TFT_HEIGHT 480
  
  // RGB Panel Pins for 2.8" Round
  #define DE_PIN 40
  #define VSYNC_PIN 39
  #define HSYNC_PIN 38
  #define PCLK_PIN 41
  
  // Red pins
  #define R0_PIN 46
  #define R1_PIN 3
  #define R2_PIN 8
  #define R3_PIN 18
  #define R4_PIN 17
  
  // Green pins
  #define G0_PIN 14
  #define G1_PIN 13
  #define G2_PIN 12
  #define G3_PIN 11
  #define G4_PIN 10
  #define G5_PIN 9
  
  // Blue pins
  #define B0_PIN 5
  #define B1_PIN 45
  #define B2_PIN 48
  #define B3_PIN 47
  #define B4_PIN 21
  
  // Touch pins (GT911 i2c via PCA9554)
  #define TOUCH_SDA 15
  #define TOUCH_SCL 7
  #define TOUCH_INT 16
  #define TOUCH_RST 1   // Via PCA9554 GPIO1
  
  // NOTE: PCA9554 GPIO Expander (I2C address 0x20) needed for:
  // - CS pin (PCA9554:2)
  // - Display reset (PCA9554:0)  
  // - Touch reset (PCA9554:1)
  
  // SPI pins for round display (using SPI3)
  #define SPI_CLK 2
  #define SPI_MOSI 1
  #define SPI_CS GFX_NOT_DEFINED  // Controlled via PCA9554:2
  #define SPI_RST GFX_NOT_DEFINED // Controlled via PCA9554:0
  
  // Backlight
  #define BL_PIN 6
  
  // RGB Panel timing parameters (2.8" round - tuned for round display)
  #define HSYNC_POLARITY 1
  #define HSYNC_FRONT_PORCH 10
  #define HSYNC_PULSE_WIDTH 8
  #define HSYNC_BACK_PORCH 50
  #define VSYNC_POLARITY 1
  #define VSYNC_FRONT_PORCH 10
  #define VSYNC_PULSE_WIDTH 8
  #define VSYNC_BACK_PORCH 20
  
  // PCLK frequency for 2.8" round
  #define TFT_PCLK_FREQ_MHZ 16

#else
  #error "Invalid BOARD_TYPE. Use 1 for 4\" square or 2 for 2.8\" round"
#endif

// #endif // BOARD_PINS_H