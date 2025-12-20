#include "battery_management.h"
#include "squarelineUI/ui.h"
#include <Arduino.h>

void battery_init() {
    analogSetPinAttenuation(BATTERY_ADC_PIN, ADC_11db);  // For 0-3.3V range
    pinMode(BATTERY_ADC_PIN, INPUT);
}

float get_battery_voltage() {
    uint32_t raw = analogRead(BATTERY_ADC_PIN);
    // Convert to voltage (0-3.3V range from ADC, doubled by voltage divider)
    float voltage = (raw / (float)ADC_MAX_VALUE) * 3.3 * 2;
    return voltage;
}

uint8_t get_battery_percentage() {
    float voltage = get_battery_voltage();
    
    // Clamp to battery range
    if (voltage >= MAX_BATTERY_VOLTAGE) return 100;
    if (voltage <= MIN_BATTERY_VOLTAGE) return 0;
    
    // Linear interpolation between min/max
    uint8_t percentage = (uint8_t)(((voltage - MIN_BATTERY_VOLTAGE) / 
                                    (MAX_BATTERY_VOLTAGE - MIN_BATTERY_VOLTAGE)) * 100);
    return percentage;
}

void update_battery_display() {
    uint8_t percent = get_battery_percentage();
    float voltage = get_battery_voltage();
    
    if (ui_batteryBar2) {
        lv_bar_set_value(ui_batteryBar2, percent, LV_ANIM_OFF);
    }
    
    if (ui_batteryPercent2) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "%.2fV\n%d%%", voltage, percent);
        lv_label_set_text(ui_batteryPercent2, buffer);
    }
}
