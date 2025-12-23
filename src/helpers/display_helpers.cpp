#include "display_helpers.h"
#include "squarelineUI/ui.h"
#include "board_pins.h"
#include <Arduino.h>

// ============ GLOBAL STATE INSTANCE ============
DisplayState display_state = {
    .timer_ms = 0,
    .timer_max_ms = 10000,
    .brightness = 128,
    .event_text = {0},
    .time_text = {0},
    .bg_image_path = {0},
    .bg_image_changed = false
};

// ============ FORWARD DECLARATIONS ============
static void update_timer_ui();
static void update_brightness_ui();
static void update_text_ui();
static void update_image_ui();

// ============ INITIALIZATION ============
void display_state_init() {
    strcpy(display_state.event_text, "Ready");
    strcpy(display_state.time_text, "--:--");
    strcpy(display_state.bg_image_path, "");
    
    render_display_state();
}

// ============ STATE UPDATE FUNCTIONS ============

void update_timer(uint32_t milliseconds, uint32_t max_milliseconds) {
    display_state.timer_ms = milliseconds;
    display_state.timer_max_ms = max_milliseconds;
    update_timer_ui();
}

void update_brightness(uint8_t value) {
    // Enforce minimum brightness of 10 to prevent completely dark display
    if (value < 10) {
        value = 10;
    }
    display_state.brightness = value;
    update_brightness_ui();
}

void update_event_text(const char* text) {
    if (text) {
        strncpy(display_state.event_text, text, sizeof(display_state.event_text) - 1);
        display_state.event_text[sizeof(display_state.event_text) - 1] = '\0';
    }
    update_text_ui();
}

void update_time_text(const char* text) {
    if (text) {
        strncpy(display_state.time_text, text, sizeof(display_state.time_text) - 1);
        display_state.time_text[sizeof(display_state.time_text) - 1] = '\0';
    }
    update_text_ui();
}

void update_background_image(const char* image_path) {
    if (image_path) {
        strncpy(display_state.bg_image_path, image_path, sizeof(display_state.bg_image_path) - 1);
        display_state.bg_image_path[sizeof(display_state.bg_image_path) - 1] = '\0';
        display_state.bg_image_changed = true;
    }
    update_image_ui();
}

// ============ UI UPDATE FUNCTIONS ============

static void update_timer_ui() {
    if (!ui_timer_arc) return;
    
    // Calculate arc value as percentage of time REMAINING (countdown)
    // Start at 100% and count down to 0%
    uint16_t arc_value = ((display_state.timer_max_ms - display_state.timer_ms) * 100) / display_state.timer_max_ms;
    arc_value = (arc_value > 100) ? 100 : arc_value;
    
    lv_arc_set_value(ui_timer_arc, arc_value);
}

static void update_brightness_ui() {
    // Apply brightness to display backlight using PWM on BL_PIN
    // BL_PIN is defined in board_pins.h
    
    // Use analogWrite for PWM brightness control (0-255)
    analogWrite(BL_PIN, display_state.brightness);
}

static void update_text_ui() {
    if (ui_timeLabel) {
        lv_label_set_text(ui_timeLabel, display_state.time_text);
    }
    
    if (ui_eventLabel) {
        lv_label_set_text(ui_eventLabel, display_state.event_text);
    }
}

static void update_image_ui() {
    if (!ui_Image1 || !display_state.bg_image_changed) return;
    
    // If path is empty, use default
    if (strlen(display_state.bg_image_path) == 0) {
        lv_img_set_src(ui_Image1, &ui_img_backpacks_png);
    } else {
        // Load from SD card path
        lv_img_set_src(ui_Image1, display_state.bg_image_path);
    }
    
    display_state.bg_image_changed = false;
}

// ============ RENDER FUNCTION ============

void render_display_state() {
    update_timer_ui();
    update_brightness_ui();
    update_text_ui();
    update_image_ui();
    
}

void force_update_ui() {
    render_display_state();
    lv_refr_now(NULL);  // Force immediate refresh
}
