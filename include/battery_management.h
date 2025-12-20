#ifndef BATTERY_MANAGEMENT_H
#define BATTERY_MANAGEMENT_H

#include <stdint.h>

#define BATTERY_ADC_PIN 4
#define BATTERY_ADC_CHANNEL ADC1_CH3
#define MAX_BATTERY_VOLTAGE 4.2
#define MIN_BATTERY_VOLTAGE 2.5
#define ADC_MAX_VALUE 4095

void battery_init();
uint8_t get_battery_percentage();
float get_battery_voltage();
void update_battery_display();

#endif
