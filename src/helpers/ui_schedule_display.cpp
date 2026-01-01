#include "ui_schedule_display.h"
#include "schedule_manager.h"
#include <stdio.h>

/**
 * Populate the schedule container with all events from the schedule
 * Each line shows: "HH:MM - Event Label"
 */
void ui_populate_schedule_list(lv_obj_t* container) {
    if (!container) return;
    
    // Clear existing children
    ui_clear_schedule_list(container);
    
    // Load events from schedule manager
    ScheduleEvent* currentEvent = getCurrentScheduleEvent();
    ScheduleEvent* nextEvent = getNextScheduleEvent();
    
    Serial.println("[UI Schedule] Populating schedule list...");
    
    // We need to get all events - for now, we'll call the internal fetch
    // In a production system, you'd want a proper "getAllEvents()" function in schedule_manager
    
    // Create labels for all events by iterating through upcoming ones
    uint16_t currentTime = getCurrentMinutesSinceMidnight();
    
    // Display current event (if any)
    if (currentEvent) {
        lv_obj_t* label = lv_label_create(container);
        char timeStr[8];
        getTimeDisplayFormat(currentEvent->start, timeStr, sizeof(timeStr));
        
        char text[64];
        snprintf(text, sizeof(text), "%s - %s (NOW)", timeStr, currentEvent->label);
        lv_label_set_text(label, text);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFD700), LV_PART_MAIN);  // Gold for current
        Serial.printf("[UI Schedule] Added current: %s\n", text);
    }
    
    // Display next event (if any)
    if (nextEvent) {
        lv_obj_t* label = lv_label_create(container);
        char timeStr[8];
        getTimeDisplayFormat(nextEvent->start, timeStr, sizeof(timeStr));
        
        uint16_t minutesUntil = getMinutesUntilNextEvent();
        char text[64];
        snprintf(text, sizeof(text), "%s - %s (in %u min)", timeStr, nextEvent->label, minutesUntil);
        lv_label_set_text(label, text);
        Serial.printf("[UI Schedule] Added next: %s\n", text);
    }
}

/**
 * Clear all items from the schedule container
 */
void ui_clear_schedule_list(lv_obj_t* container) {
    if (!container) return;
    
    lv_obj_clean(container);
    Serial.println("[UI Schedule] Cleared schedule list");
}
