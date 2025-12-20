#ifndef ALARM_H
#define ALARM_H

#include <Arduino.h>

// ============ ALARM STATE ============
struct AlarmState {
    bool enabled;
    bool active;
    uint32_t beep_start_time;
    uint32_t last_beep_time;
    int beep_count;
    int pattern_cycle;  // Track which cycle of beeps (4 beeps, pause, 4 beeps, pause)
    bool currently_beeping;
};

extern AlarmState alarm_state;

// ============ ALARM FUNCTIONS ============

/**
 * Initialize alarm system
 */
void alarm_init();

/**
 * Set alarm enabled/disabled
 */
void set_alarm_enabled(bool enabled);

/**
 * Trigger the alarm (call when timer finishes)
 */
void trigger_alarm();

/**
 * Update alarm state (call in main loop for beeping pattern)
 */
void update_alarm();

/**
 * Stop the alarm
 */
void stop_alarm();

#endif // ALARM_H
