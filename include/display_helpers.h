#ifndef DISPLAY_STATE_H
#define DISPLAY_STATE_H

#include <Arduino.h>
#include <lvgl.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// ============ STATE STRUCTURE ============
struct DisplayState {
    // Timer
    uint32_t timer_ms;
    uint32_t timer_max_ms;  // For arc calculation - needs uint32_t for large durations
    
    // Brightness
    uint8_t brightness;
    
    // Text fields
    char event_text[256];
    char time_text[64];
    
    // Background image path (for SD card images)
    char bg_image_path[256];
    bool bg_image_changed;
};

// ============ GLOBAL STATE ============
extern struct DisplayState display_state;

// ============ STATE MANAGEMENT FUNCTIONS ============

/**
 * Initialize display state with defaults
 */
void display_state_init();

/**
 * Update timer value and automatically update UI
 * @param milliseconds Current timer value in milliseconds
 * @param max_milliseconds Maximum timer value for arc calculation
 */
void update_timer(uint32_t milliseconds, uint32_t max_milliseconds);

/**
 * Update brightness value and apply to display
 * @param value Brightness value (0-255)
 */
void update_brightness(uint8_t value);

/**
 * Update event text label
 * @param text New text for event label
 */
void update_event_text(const char* text);

/**
 * Update time label
 * @param text New text for time label (e.g., "14:32")
 */
void update_time_text(const char* text);

/**
 * Update background image from SD card
 * @param image_path Path to image on SD card (e.g., "/sd/display/bg.png")
 */
void update_background_image(const char* image_path);

/**
 * Render all pending state changes to the UI
 * Call this in your main loop or when UI needs to refresh
 */
void render_display_state();

/**
 * Manual update for a specific UI element
 * Useful if you need to update without waiting for next render_display_state() call
 */
void force_update_ui();

#ifdef __cplusplus
}
#endif

#endif // DISPLAY_STATE_H
