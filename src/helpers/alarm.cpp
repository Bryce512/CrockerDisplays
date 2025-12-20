#include "alarm.h"
#include "TCA9554PWR.h"

// ============ ALARM STATE ============
AlarmState alarm_state = {
    .enabled = true,
    .active = false,
    .beep_start_time = 0,
    .last_beep_time = 0,
    .beep_count = 0,
    .pattern_cycle = 0,
    .currently_beeping = false
};

// ============ TIMING CONSTANTS ============
#define BEEP_DURATION_MS 100       // How long each beep lasts (75ms faster beeps)
#define PAUSE_BETWEEN_BEEPS_MS 50 // Pause between individual beeps within a set
#define PAUSE_BETWEEN_PATTERNS_MS 500 // Pause between 4-beep patterns (500ms between sets)
#define TOTAL_BEEPS 16            // Total number of beeps (4 beeps, pause, 4 beeps, pause = 1 cycle)

// ============ ALARM IMPLEMENTATION ============

void alarm_init() {
    alarm_state.enabled = true;
    alarm_state.active = false;
    alarm_state.beep_count = 0;
    alarm_state.pattern_cycle = 0;
    alarm_state.currently_beeping = false;
}

void set_alarm_enabled(bool enabled) {
    alarm_state.enabled = enabled;
    if (!enabled) {
        stop_alarm();
    }
    Serial.print("Alarm ");
    Serial.println(enabled ? "enabled" : "disabled");
}

void trigger_alarm() {
    if (!alarm_state.enabled) return;
    
    alarm_state.active = true;
    alarm_state.beep_count = 0;
    alarm_state.pattern_cycle = 0;
    alarm_state.beep_start_time = millis();
    alarm_state.last_beep_time = millis();
    alarm_state.currently_beeping = true;
    
    // Start first beep
    Set_EXIO(EXIO_PIN8, High);  // Buzzer ON
    Serial.println("Alarm triggered!");
}

void update_alarm() {
    if (!alarm_state.active) return;
    
    uint32_t now = millis();
    uint32_t elapsed_since_last_action = now - alarm_state.last_beep_time;
    
    if (alarm_state.currently_beeping) {
        // Currently beeping - check if we should stop the beep
        if (elapsed_since_last_action >= BEEP_DURATION_MS) {
            Set_EXIO(EXIO_PIN8, Low);  // Buzzer OFF
            alarm_state.currently_beeping = false;
            alarm_state.last_beep_time = now;
            alarm_state.beep_count++;
            
            // Check if we've completed 16 beeps
            if (alarm_state.beep_count >= TOTAL_BEEPS) {
                stop_alarm();
                return;
            }
        }
    } else {
        // Currently in pause - check if we should start next beep
        uint32_t pause_duration;
        
        // After every 4 beeps, use longer pause
        if ((alarm_state.beep_count % 4) == 0 && alarm_state.beep_count > 0) {
            pause_duration = PAUSE_BETWEEN_PATTERNS_MS;
        } else {
            pause_duration = PAUSE_BETWEEN_BEEPS_MS;
        }
        
        if (elapsed_since_last_action >= pause_duration) {
            // Start next beep
            Set_EXIO(EXIO_PIN8, High);  // Buzzer ON
            alarm_state.currently_beeping = true;
            alarm_state.last_beep_time = now;
        }
    }
}

void stop_alarm() {
    Set_EXIO(EXIO_PIN8, Low);  // Buzzer OFF
    alarm_state.active = false;
    alarm_state.currently_beeping = false;
    alarm_state.beep_count = 0;
    alarm_state.pattern_cycle = 0;
    Serial.println("Alarm stopped!");
}
