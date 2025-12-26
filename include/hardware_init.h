#ifndef HARDWARE_INIT_H
#define HARDWARE_INIT_H

#include <Arduino_GFX_Library.h>
#include <lvgl.h>

extern Arduino_ESP32SPI* bus;
extern Arduino_RGB_Display* gfx;
extern lv_display_t *display;

void hardware_init();
void initBLEService();

#endif
