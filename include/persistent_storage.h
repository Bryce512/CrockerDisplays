#ifndef PERSISTENT_STORAGE_H
#define PERSISTENT_STORAGE_H

#include <stdint.h>

void storage_init();
void save_brightness(uint8_t brightness);
uint8_t load_brightness();
void save_alarm_enabled(bool enabled);
bool load_alarm_enabled();

#endif
