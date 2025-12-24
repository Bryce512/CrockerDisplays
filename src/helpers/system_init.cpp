#include "system_init.h"
#include "persistent_storage.h"
#include "display_helpers.h"
#include "battery_management.h"
#include "alarm.h"
#include "timer_functions.h"
#include "ui_callbacks.h"
#include "squarelineUI/ui.h"
#include "JSON_writer.h"
#include <Arduino.h>

void system_state_init() {
    // ===== PERSISTENT STORAGE =====
    storage_init();
    
    // ===== DISPLAY STATE =====
    display_state_init();
    
    // ===== BRIGHTNESS (load and restore UI) =====
    uint8_t saved_brightness = load_brightness();
    update_brightness(saved_brightness);
    
    if (ui_Slider1) {
        int32_t slider_value = (saved_brightness * 100) / 255;
        lv_slider_set_value(ui_Slider1, slider_value, LV_ANIM_OFF);
        Serial.printf("Restored brightness slider to: %d\n", slider_value);
    }
    
    // ===== ALARM (initialize, load, and restore UI) =====
    alarm_init();
    
    bool saved_alarm_enabled = load_alarm_enabled();
    set_alarm_enabled(saved_alarm_enabled);
    
    if (ui_Switch1) {
        if (saved_alarm_enabled) {
            lv_obj_set_state(ui_Switch1, LV_STATE_CHECKED, 0);
        } else {
            lv_obj_clear_state(ui_Switch1, LV_STATE_CHECKED);
        }
        Serial.printf("Restored alarm state to: %s\n", saved_alarm_enabled ? "enabled" : "disabled");
    }
    
    // ===== BATTERY =====
    battery_init();
    
    // ===== UI CALLBACKS =====
    setup_ui_callbacks();

    // ===== TEST =====

    // 24hr time 1030 = 10:30am 2115 = 9:15pm, time in seconds, event name, image path on the sd card
    
//     writeConfig wcfg{1030, 30, "Breakfast", "/doc/file/img"};

//     writeJSON(wcfg); 
}
