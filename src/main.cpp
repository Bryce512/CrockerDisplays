#include <Arduino.h>
#include "board_pins.h"
#include "board_configs.h"
#include "display_state.h"
#include "ui_callbacks.h"
#include "timer_functions.h"
#include "alarm.h"
#include "battery_management.h"
#include "persistent_storage.h"
#include "hardware_init.h"
#include "system_init.h"

#define GFX_BL BL_PIN


void setup()
{
  Serial.begin(115200);
  
  // Initialize all hardware (I2C, display, LVGL, touch, etc.)
  hardware_init();
  
  // Initialize system state (storage, brightness, alarm, battery, timer)
  system_state_init();
  
  Serial.println("Setup complete!");
}

void loop()
{  
    // Update alarm (beeping pattern)
    update_alarm();
    
    // Update timer display
    update_timer_display();
    
    // Update battery display
    update_battery_display();
    
    // Render all display state changes
    render_display_state();
    
    // LVGL GUI handler
    lv_timer_handler();
    
    delay(5);
}
