#include "timer_event_handler.h"
#include "schedule_manager.h"
#include <Arduino.h>

#define MAX_EVENTS 1  // Only queue the current event

static readConfig queue[MAX_EVENTS];
static size_t queue_count = 0;
static size_t queue_index = 0;

/**
 * Load the current active event into the queue
 * If no event is active, queue is empty
 * This makes the timer FSM time-aware instead of sequentially popping events
 */
bool timer_load_queue()
{
    queue_count = 0;
    queue_index = 0;
    
    // Get the current active event from schedule manager
    ScheduleEvent* current_event = getCurrentScheduleEvent();
    
    if (current_event) {
        queue[0] = *current_event;
        queue_count = 1;
        Serial.printf("[TIMER] Queue loaded with current event: %s\n", current_event->label);
        return true;
    } else {
        Serial.println("[TIMER] No active event - queue empty");
        return false;  // Queue is empty (no active event)
    }
}

bool timer_has_next()
{
    return queue_index < queue_count;
}

bool timer_pop_next(readConfig& out_event)
{
    if (!timer_has_next()) return false;
    out_event = queue[queue_index++];
    return true;
}

size_t _queue_count() {
    return queue_count;
}

size_t _queue_index() {
    return queue_index;
}