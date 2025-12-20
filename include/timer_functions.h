#ifndef TIMER_FUNCTIONS_H
#define TIMER_FUNCTIONS_H

#include "display_state.h"
#include <Arduino.h>
#include <time.h>

// ============ TIMER STATE ============
struct TimerState {
    uint32_t start_time;
    uint32_t duration;  // total milliseconds
    bool active;
};

extern TimerState timer;

// ============ TIMER FUNCTIONS ============

/**
 * Start a countdown timer
 * @param duration_ms Timer duration in milliseconds
 */
void start_timer(uint32_t duration_ms);

/**
 * Update timer display (call in main loop)
 */
void update_timer_display();

/**
 * Display current time on time label
 */
void update_current_time();

#endif // TIMER_FUNCTIONS_H
