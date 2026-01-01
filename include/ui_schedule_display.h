#ifndef UI_SCHEDULE_DISPLAY_H
#define UI_SCHEDULE_DISPLAY_H

#include <lvgl.h>

/**
 * Populate the schedule container with all events from the schedule
 * Each line shows: "HH:MM - Event Label"
 * Container should be a scrollable flex container (column layout)
 */
void ui_populate_schedule_list(lv_obj_t* container);

/**
 * Clear all items from the schedule container
 */
void ui_clear_schedule_list(lv_obj_t* container);

#endif
