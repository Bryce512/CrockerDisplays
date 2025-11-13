/*******************************************************************************
 * Touch libraries:
 * GT911: https://github.com/TAMCTec/gt911-arduino.git
 ******************************************************************************/
#ifndef TOUCH_H
#define TOUCH_H

#include "board_pins.h"

#define TOUCH_GT911
#define TOUCH_GT911_SCL TOUCH_SCL
#define TOUCH_GT911_SDA TOUCH_SDA
#define TOUCH_GT911_INT TOUCH_INT
#define TOUCH_GT911_RST TOUCH_RST
#define TOUCH_GT911_ROTATION ROTATION_NORMAL
#define TOUCH_MAP_X1 480
#define TOUCH_MAP_X2 0
#define TOUCH_MAP_Y1 480
#define TOUCH_MAP_Y2 0

#include <Wire.h>
#include <Touch_GT911.h>

// Global variables (declare, don't define)
extern int touch_last_x;
extern int touch_last_y;

// Function declarations only
void touch_init();
bool touch_has_signal();
bool touch_touched();
bool touch_released();

#endif