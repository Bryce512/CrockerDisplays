#include <Arduino.h>
#include "board_pins.h"
#include "board_configs.h"
#include "display_helpers.h"
#include "ui_callbacks.h"
#include "battery_management.h"
#include "persistent_storage.h"
#include "hardware_init.h"
#include "system_init.h"
#include "alarm.h"
#include "logic_fsm.h"
//#include "ui_fsm.h"

#define GFX_BL BL_PIN


void setup()
{
  Serial.begin(115200);
  
  // Initialize all hardware (I2C, display, LVGL, touch, etc.)
  hardware_init();
  
  // Initialize system state (storage, brightness, alarm, battery, timer)
  system_state_init();

  // Initialize the device state machine
  logic_fsm_init();
  
  Serial.println("Setup complete!");
}

void loop()
{  
    // Update battery display. Screen 3 is dynamic, screen 1 is not
    update_battery_display();

    // Logic tick function, controls the timers and the alarms
    logic_fsm_tick();
    
    // Updates the alarm
    update_alarm();

    // Render all display state changes
    render_display_state();
    
    // LVGL GUI handler
    lv_timer_handler();
    
    delay(5);
}
