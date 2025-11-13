/*******************************************************************************
 * Touch libraries:
 * GT911: https://github.com/TAMCTec/gt911-arduino.git
 ******************************************************************************/
#ifndef TOUCH_H
#define TOUCH_H

#define TOUCH_GT911
#define TOUCH_GT911_SCL 45
#define TOUCH_GT911_SDA 19
#define TOUCH_GT911_INT -1
#define TOUCH_GT911_RST -1
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