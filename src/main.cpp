#include <Arduino.h>
#include <time.h>
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
#include "ble_service.h"
#include "schedule_manager.h"
#include "squarelineUI/ui.h"
//#include "ui_fsm.h"

#define GFX_BL BL_PIN

void setup()
{
  Serial.begin(115200);
  
  // Initialize all hardware (I2C, display, LVGL, touch, etc.)
  hardware_init();
  
  // Initialize system state (storage, brightness, alarm, battery, timer)
  system_state_init();
  
  // Restore time from NVS (persisted from last phone sync)
  initTimeFromNVS();

  // Initialize the device state machine
  logic_fsm_init();
  
  // Load schedule from SD card and display on Screen 2
  ui_Screen2_updateScheduleDisplay();
  
  // Update Screen 1 with countdown to next event
  ui_Screen1_updateCountdown();
  
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
    
    // Periodically update NVS with current time (ensures it's never stale after reboot)
    updateNVSTimeIfNeeded();
    
    // Check for 2 AM schedule sync (if connected to phone)
    checkAndSyncScheduleIfNeeded();
    
    // Process any pending BLE JSON config (must be in main loop to avoid stack overflow)
    processBLEConfig();
    
    // Process any pending BLE file transfers (must be in main loop to avoid stack overflow)
    processBLEFileData();
    
    // Check if Screen 2 needs update after time sync (thread-safe flag from BLE callback)
    if (shouldUpdateScreen2AfterTimeSync()) {
        Serial.println("[MAIN] Updating Screen 2 after time sync");
        ui_Screen2_updateScheduleDisplay();
    }
    
    // Check schedule every minute and log state
    static unsigned long lastScheduleLog = 0;
    if (millis() - lastScheduleLog > 60000) {
        lastScheduleLog = millis();
        
        // Get current time for logging
        uint16_t currentMin = getCurrentMinutesSinceMidnight();
        char currentTimeStr[8];
        getTimeDisplayFormat(currentMin, currentTimeStr, sizeof(currentTimeStr));
        Serial.printf("[SCHEDULE] Current time: %s\n", currentTimeStr);
        
        if (isEventActive()) {
            ScheduleEvent* current = getCurrentScheduleEvent();
            uint16_t remaining = getMinutesRemainingInCurrentEvent();
            Serial.printf("[SCHEDULE] Active: %s (%u min remaining)\n", current->label, remaining);
        } else {
            ScheduleEvent* next = getNextScheduleEvent();
            if (next) {
                uint16_t until = getMinutesUntilNextEvent();
                char nextTimeStr[8];
                getTimeDisplayFormat(next->start, nextTimeStr, sizeof(nextTimeStr));
                Serial.printf("[SCHEDULE] Next: %s at %s (in %u min)\n", next->label, nextTimeStr, until);
            } else {
                Serial.println("[SCHEDULE] No more events today");
            }
        }
        
        // Update Screen 2 schedule display with current data from duration.json
        ui_Screen2_updateScheduleDisplay();
        
        // Update Screen 1 countdown display
        ui_Screen1_updateCountdown();
    }
    
    // LVGL GUI handler
    lv_timer_handler();
    
    // Update time and countdown displays (every second)
    static unsigned long lastDisplayUpdate = 0;
    if (millis() - lastDisplayUpdate > 1000) {
        lastDisplayUpdate = millis();
        
        // Get current time for display
        time_t now = time(nullptr);
        struct tm* timeinfo = localtime(&now);
        char currentTimeStr[32];
        strftime(currentTimeStr, sizeof(currentTimeStr), "%H:%M:%S", timeinfo);
        
        // Update countdown in timeLabel and event label on Screen 1
        ui_Screen1_updateCountdown();
        
        // Update current time at bottom
        if (ui_currentTimeLabel) {
            lv_label_set_text(ui_currentTimeLabel, currentTimeStr);
        }
    }
    
    delay(5);
}
