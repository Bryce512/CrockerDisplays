#include "ui_callbacks.h"
#include "display_state.h"
#include "squarelineUI/ui.h"
#include "alarm.h"
#include "persistent_storage.h"
#include <Arduino.h>

/**
 * BRIGHTNESS SLIDER CALLBACK (Screen 3)
 * Called whenever ui_Slider1 value changes
 */
void brightness_slider_event_cb(lv_event_t * e) {
    // IMPORTANT: Cast void* to lv_obj_t* to fix the compiler error
    lv_obj_t * slider = (lv_obj_t *)lv_event_get_target(e);
    
    // Get slider value (typically 0-100)
    int32_t slider_value = lv_slider_get_value(slider);
    
    // Convert to brightness range (0-255)
    uint8_t brightness = (slider_value * 255) / 100;
    
    // Update display brightness
    update_brightness(brightness);
    
    // Save to persistent storage
    save_brightness(brightness);
    
    Serial.print("Brightness slider moved to: ");
    Serial.println(slider_value);
    Serial.print("Brightness value (0-255): ");
    Serial.println(brightness);
}

/**
 * ALARM TOGGLE CALLBACK (Screen 3)
 * Called whenever ui_Switch1 (alarm toggle) changes
 */
void alarm_toggle_event_cb(lv_event_t * e) {
    lv_obj_t * switch_obj = (lv_obj_t *)lv_event_get_target(e);
    
    // Get switch state (checked/unchecked)
    bool is_checked = lv_obj_has_state(switch_obj, LV_STATE_CHECKED);
    
    // Update alarm enabled state
    set_alarm_enabled(is_checked);
    
    // Save to persistent storage
    save_alarm_enabled(is_checked);
}

/**
 * Register all UI event callbacks
 * Call this in setup() after ui_init()
 */
void setup_ui_callbacks(void) {
    // Register brightness slider callback (Screen 3)
    if (ui_Slider1 != NULL) {
        lv_obj_add_event_cb(ui_Slider1, brightness_slider_event_cb, 
                           LV_EVENT_VALUE_CHANGED, NULL);
        Serial.println("Brightness slider callback registered");
    } else {
        Serial.println("ERROR: ui_Slider1 is NULL!");
    }
    
    // Register alarm toggle callback (Screen 3)
    if (ui_Switch1 != NULL) {
        lv_obj_add_event_cb(ui_Switch1, alarm_toggle_event_cb, 
                           LV_EVENT_VALUE_CHANGED, NULL);
        Serial.println("Alarm toggle callback registered");
    } else {
        Serial.println("ERROR: ui_Switch1 is NULL!");
    }
}
