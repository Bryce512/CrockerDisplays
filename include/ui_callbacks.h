#ifndef UI_CALLBACKS_H
#define UI_CALLBACKS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Brightness slider callback for Screen3
 * Register with: lv_obj_add_event_cb(ui_Slider1, brightness_slider_event_cb, 
 *                                    LV_EVENT_VALUE_CHANGED, NULL);
 */
void brightness_slider_event_cb(lv_event_t * e);

/**
 * Initialize all UI callbacks
 * Call this in setup() after ui_init()
 */
void setup_ui_callbacks(void);

#ifdef __cplusplus
}
#endif

#endif // UI_CALLBACKS_H
