#include "touch.h"
#include "board_pins.h"

#define GT911_I2C_ADDRESS 0x5D  

int touch_last_x = 0, touch_last_y = 0;
Touch_GT911 ts = Touch_GT911(TOUCH_GT911_SDA, TOUCH_GT911_SCL, TOUCH_GT911_INT, TOUCH_GT911_RST, max(TOUCH_MAP_X1, TOUCH_MAP_X2), max(TOUCH_MAP_Y1, TOUCH_MAP_Y2));


void touch_init()
{
    if (BOARD_TYPE == 1) {
      ts.begin(GT911_I2C_ADDRESS);
      ts.setRotation(TOUCH_GT911_ROTATION);
  } else {
    // Don't call touch_init() - it will reinitialize Wire!
    // Instead, just initialize the touch sensor directly
      ts.begin(GT911_I2C_ADDRESS);  // Initialize the GT911 touch sensor
      ts.setRotation(TOUCH_GT911_ROTATION);
  }
}

bool touch_has_signal()
{
  return true;
}

bool touch_touched()
{

  ts.read();
  if (ts.isTouched)
  {
#if defined(TOUCH_SWAP_XY)
    touch_last_x = map(ts.points[0].y, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, 480 - 1);
    touch_last_y = map(ts.points[0].x, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, 480 - 1);
#else
    touch_last_x = map(ts.points[0].x, TOUCH_MAP_X1, TOUCH_MAP_X2, 0, 480 - 1);
    touch_last_y = map(ts.points[0].y, TOUCH_MAP_Y1, TOUCH_MAP_Y2, 0, 480 - 1);
#endif
    return true;
  }
  else
  {
    return false;
  }
}

bool touch_released()
{
    return true;

}

