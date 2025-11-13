#ifndef GLOBALS_H
#define GLOBALS_H

// System libraries
#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include <ESP32_4848S040.h>
#include <lvgl.h>

// Local libraries
#include "touch.h"

// Screen declarations
#include "Screens/screens.h"


// Global screen pointer to track previous screen for cleanup
extern lv_obj_t * prev_screen;
extern lv_obj_t * current_screen;
extern lv_obj_t * home;
extern lv_obj_t * details;

extern uint16_t inner_circle_size;
extern uint16_t arc_outer_size;
extern uint16_t arc_thickness;

#endif // GLOBALS_H