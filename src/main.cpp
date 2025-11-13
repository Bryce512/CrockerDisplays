#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <ESP32_4848S040.h>
#include <lvgl.h>
#include "touch.h"
#include "squarelineUI/ui.h"

// #define board = "4Square"
#define board = "2.8"

#define GFX_BL 38
Arduino_ESP32SPI* bus;
Arduino_RGB_Display* gfx;

int16_t w, h, text_size, banner_height, graph_baseline, graph_height, channel_width, signal_width;
#define BYTE_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB565)) /*will be 2 for RGB565 */
static uint8_t buf1[480 * 480 / 10 * BYTE_PER_PIXEL];
lv_display_t *display;


/* lvgl */
#define TFT_HOR_RES   480
#define TFT_VER_RES   480
#define TFT_ROTATION  LV_DISPLAY_ROTATION_0

/*LVGL draw into this buffer, 1/10 screen size usually works well. The size is in bytes*/
#define DRAW_BUF_SIZE (TFT_HOR_RES * TFT_VER_RES / 10 * (LV_COLOR_DEPTH / 8))
uint32_t draw_buf[DRAW_BUF_SIZE / 4];


// Relay config (gpio)
#define GPIO_RELAY1  40
#define GPIO_RELAY2  2
#define GPIO_RELAY3  1


int i = 0;

/* Display flushing */
void my_disp_flush( lv_display_t *disp, const lv_area_t *area, uint8_t * px_map)
{
  uint32_t w = (area->x2 - area->x1 + 1);
  uint32_t h = (area->y2 - area->y1 + 1);

#if (LV_COLOR_16_SWAP != 0)
  gfx->draw16bitBeRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
#else
  gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)px_map, w, h);
#endif

  lv_disp_flush_ready(disp);
}

void my_touchpad_read(lv_indev_t * indev, lv_indev_data_t * data )
{
  if (touch_has_signal())
  {
    if (touch_touched())
    {
      data->state = LV_INDEV_STATE_PRESSED;

      /*Set the coordinates*/
      data->point.x = touch_last_x;
      data->point.y = touch_last_y;
      //Serial.print("Touched : "); Serial.print(touch_last_x); Serial.print(" x ");Serial.println(touch_last_y);
    }
    else if (touch_released())
    {
      data->state = LV_INDEV_STATE_RELEASED;
    }
  }
  else
  {
    data->state = LV_INDEV_STATE_RELEASED;
  }
}

static uint32_t my_tick(void)
{
    return millis();
}


void setup()
{
  Serial.begin(115200);
  Serial.println("Setup...");
  touch_init();

  // 9-bit mode SPI
  bus = new Arduino_ESP32SPI(
  GFX_NOT_DEFINED /* DC */, 39 /* CS */, 48 /* SCK */, 47 /* MOSI */, GFX_NOT_DEFINED /* MISO */);

  // panel (Hardware) specific
  // Swap R and B GPIO pins
  Arduino_ESP32RGBPanel *rgbpanel = new Arduino_ESP32RGBPanel(
    18 /* DE */, 17 /* VSYNC */, 16 /* HSYNC */, 21 /* PCLK */,
    11 /* R0 */, 12 /* R1 */, 13 /* R2 */, 14 /* R3 */, 0 /* R4 */,
    8 /* G0 */, 20 /* G1 */, 3 /* G2 */, 46 /* G3 */, 9 /* G4 */, 10 /* G5 */,
    4 /* B0 */, 5 /* B1 */, 6 /* B2 */, 7 /* B3 */, 15 /* B4 */,
    1 /* hsync_polarity */, 10 /* hsync_front_porch */, 8 /* hsync_pulse_width */, 50 /* hsync_back_porch */,
    1 /* vsync_polarity */, 10 /* vsync_front_porch */, 8 /* vsync_pulse_width */, 20 /* vsync_back_porch */);

  // panel parameters & setup
  gfx = new Arduino_RGB_Display(
    480 /* width */, 480 /* height */, rgbpanel, 0 /* rotation */, true /* auto_flush */,
    bus, GFX_NOT_DEFINED /* RST */, st7701_4848s040_init_operations, sizeof(st7701_4848s040_init_operations));

  if (!gfx->begin())
  {
    Serial.println("gfx->begin() failed!");
  }
#ifdef GFX_BL
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, HIGH);
#endif

  gfx->setCursor(100, 200);
  gfx->displayOn();
  gfx->fillScreen(BLACK);
  gfx->setTextColor(BLUE);
  gfx->setTextSize(6 /* x scale */, 6 /* y scale */, 2 /* pixel_margin */);
  gfx->println("Prout !");

  /* Configuration LVGL */
  lv_init();

  /*Set a tick source so that LVGL will know how much time elapsed. */
  lv_tick_set_cb(my_tick);
  lv_display_t * disp;
  disp = lv_display_create(TFT_HOR_RES, TFT_VER_RES);
  lv_display_set_flush_cb(disp, my_disp_flush);
  lv_display_set_buffers(disp, draw_buf, NULL, sizeof(draw_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

  // Touch Driver
  lv_indev_t * indev = lv_indev_create();
  lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);   /*See below.*/
  lv_indev_set_read_cb(indev, my_touchpad_read);

  // build GUI
  ui_init();
  lv_scr_load(ui_Screen1);
}

void loop()
{  
 #if 0   
    Serial.println("");
    Serial.println("In the loop");
    Serial.print("Total heap: "); Serial.println(ESP.getHeapSize());
    Serial.print("Free heap: "); Serial.println(ESP.getFreeHeap());
    Serial.print("Total PSRAM: "); Serial.println(ESP.getPsramSize());
    Serial.print("Free PSRAM: "); Serial.println(ESP.getFreePsram());    
 #endif   
    lv_timer_handler(); /* let the GUI do its work */
    //lv_tick_inc(10);
    delay(5);
}