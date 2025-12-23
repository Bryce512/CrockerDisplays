#include "hardware_init.h"
#include "board_pins.h"
#include "board_configs.h"
#include <ESP32_4848S040.h>
#include "touch.h"
#include "TCA9554PWR.h"
#include <Wire.h>
#include <Arduino.h>
#include "squarelineUI/ui.h"
#include "SD_MMC.h"

Arduino_ESP32SPI* bus = NULL;
Arduino_RGB_Display* gfx = NULL;
lv_display_t *display = NULL;

#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565))
static uint8_t buf1[480 * 480 / 10 * BYTE_PER_PIXEL];

#define TFT_HOR_RES   TFT_WIDTH
#define TFT_VER_RES   TFT_HEIGHT
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];

// ============ LVGL CALLBACKS ============

void my_disp_flush(lv_display_t *disp, const lv_area_t *area, uint8_t * px_map) {
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
#endif

  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_t * indev, lv_indev_data_t * data) {
  if (touch_has_signal()) {
    if (touch_touched()) {
      data->state = LV_INDEV_STATE_PRESSED;
      data->point.x = touch_last_x;
      data->point.y = touch_last_y;
    } else if (touch_released()) {
      data->state = LV_INDEV_STATE_RELEASED;
    }
  } else {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

static uint32_t my_tick(void) {
    return millis();
}

// ============ HARDWARE INITIALIZATION ============

void hardware_init() {
  Serial.println("Setup...");

  #if BOARD_TYPE == 1 
    bus = new Arduino_ESP32SPI(
      GFX_NOT_DEFINED /* DC */, 39 /* CS */, 48 /* SCK */, 47 /* MOSI */, GFX_NOT_DEFINED /* MISO */);

  #elif BOARD_TYPE == 2
    // ===== STEP 1: Initialize I2C for PCA9554 =====
    Serial.println("Initializing I2C...");
    
    // First, try basic recovery
    for (int attempt = 0; attempt < 3; attempt++) {
      Wire.end();
      delay(100);
      
      // Manual GPIO recovery with CORRECT pins for ESP32-S3
      pinMode(TOUCH_GT911_SDA, OUTPUT);  // SDA
      pinMode(TOUCH_GT911_SCL, OUTPUT);  // SCL
      digitalWrite(TOUCH_GT911_SDA, HIGH);
      digitalWrite(TOUCH_GT911_SCL, HIGH);
      delay(50);
      
      Wire.begin(TOUCH_GT911_SDA, TOUCH_GT911_SCL);
      Wire.setClock(100000);
      delay(100);
      
      // Test communication with PCA9554
      Wire.beginTransmission(0x20);
      if (Wire.endTransmission() == 0) {
        Serial.println("I2C bus recovered successfully");
        break;
      }
      Serial.println("I2C recovery attempt failed, retrying...");
    }
    delay(50);
    
    // ===== STEP 2: Initialize PCA9554 (all pins as outputs) =====
    Serial.println("Initializing PCA9554...");
    TCA9554PWR_Init();
    delay(10);
    Set_EXIO(EXIO_PIN8, Low);  // Buzzer OFF
    delay(10);
    Serial.println("PCA9554 I2C status: OK");
    
    // ===== STEP 3: Reset GT911 AFTER I2C is confirmed stable =====
    Serial.println("Resetting touch...");
    digitalWrite(EXIO_PIN2, LOW);
    delay(20);
    digitalWrite(EXIO_PIN2, HIGH);
    delay(500);  // Give GT911 plenty of time to boot
    
    // ===== STEP 4: Reset Display =====
    Serial.println("Resetting display...");
    Set_EXIO(EXIO_PIN1, Low);   // Display reset LOW
    delay(100);
    Set_EXIO(EXIO_PIN1, High);  // Display reset HIGH
    delay(100);
    
    // ===== STEP 5: Initialize SPI bus =====
    Serial.println("Initializing SPI bus...");
    bus = new Arduino_ESP32SPI(
      GFX_NOT_DEFINED /* DC */, 
      GFX_NOT_DEFINED /* CS */, 
      2 /* SCK */, 
      1 /* MOSI */, 
      GFX_NOT_DEFINED /* MISO */
    );
    
    // ===== STEP 6: Set CS active (LOW) for display initialization =====
    Set_EXIO(EXIO_PIN3, Low);  // CS LOW (active)
  #endif
  
  // ===== STEP 7: Initialize RGB Panel =====
  Serial.println("Initializing RGB panel...");
  Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    DE_PIN, VSYNC_PIN, HSYNC_PIN, PCLK_PIN,
    R0_PIN, R1_PIN, R2_PIN, R3_PIN, R4_PIN,
    G0_PIN, G1_PIN, G2_PIN, G3_PIN, G4_PIN, G5_PIN,
    B0_PIN, B1_PIN, B2_PIN, B3_PIN, B4_PIN,
    HSYNC_POLARITY, HSYNC_FRONT_PORCH, HSYNC_PULSE_WIDTH, HSYNC_BACK_PORCH,
    VSYNC_POLARITY, VSYNC_FRONT_PORCH, VSYNC_PULSE_WIDTH, VSYNC_BACK_PORCH);

  // ===== STEP 8: Create GFX Display Object =====
  #if BOARD_TYPE == 1
    gfx = new Arduino_RGB_Display(
      TFT_WIDTH, TFT_HEIGHT, rgbpanel, 0, true, bus, 
      GFX_NOT_DEFINED, st7701_square_init_operations, sizeof(st7701_square_init_operations));
  #elif BOARD_TYPE == 2
    gfx = new Arduino_RGB_Display(
      TFT_WIDTH, TFT_HEIGHT, rgbpanel, 0, true, bus, 
      GFX_NOT_DEFINED, st7701_round_init_operations, sizeof(st7701_round_init_operations));
  #endif

  // ===== STEP 9: Initialize Display =====
  Serial.println("Calling gfx->begin()...");
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }

  // ===== STEP 10: Set CS inactive (HIGH) after initialization =====
  #if BOARD_TYPE == 2
    Set_EXIO(EXIO_PIN3, High);  // CS HIGH (inactive)
  #endif

  // ===== STEP 11: Enable Backlight =====
  #define GFX_BL 6
  #ifdef GFX_BL
    pinMode(GFX_BL, OUTPUT);
    digitalWrite(GFX_BL, HIGH);
  #endif

  gfx->fillScreen(BLACK);
  gfx->setTextColor(BLUE);
  gfx->setTextSize(6, 6, 2);
  gfx->println("Initializing...");

  // ===== STEP 12: Initialize Touch (AFTER PCA9554) =====
  Serial.println("Initializing touch...");
  touch_init();

  // ===== STEP 13: Initialize LVGL =====
  Serial.println("Initializing LVGL...");
  lv_init();
  lv_tick_set_cb(my_tick);
  
  lv_display_t * disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
  lv_indev_set_read_cb(indev, my_touchpad_read);
  
  ui_init();
  lv_scr_load(ui_Screen1);

  // ===== STEP 14: Initialize SD Card =====
  Serial.println("Initializing SD Card...");
  SD_MMC.setPins(/*clk=*/2, /*cmd=*/1, /*d0=*/42, /*d1=*/-1, /*d2=*/-1, /*d3=*/-1);
  if (!SD_MMC.begin("/sdcard", /*mode1bit=*/true, /*format_if_mount_failed=*/false)) {
    Serial.println("SD_MMC mount failed");
  } else {
    Serial.println("SD_MMC mounted");
  }
}

