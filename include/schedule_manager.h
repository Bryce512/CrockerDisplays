#ifndef SCHEDULE_MANAGER_H
#define SCHEDULE_MANAGER_H

#include <Arduino.h>
#include <time.h>
#include "JSON_reader.h"

typedef struct readConfig ScheduleEvent;

#ifdef __cplusplus
extern "C" {
#endif

/**
 * IMPORTANT: Event start times must be stored in JSON as MINUTES SINCE MIDNIGHT (0-1439)
 * Examples:
 *   7:00 AM = 420 minutes
 *   2:30 PM = 870 minutes
 *   11:59 PM = 1439 minutes
 * Duration must be in SECONDS
 */

ScheduleEvent* getCurrentScheduleEvent(void);
ScheduleEvent* getNextScheduleEvent(void);
size_t getAllScheduleEvents(ScheduleEvent** out_events, size_t max_events);
void invalidateScheduleCache(void);
uint16_t getMinutesUntilNextEvent(void);
uint16_t getMinutesRemainingInCurrentEvent(void);
bool isEventActive(void);
uint16_t getCurrentMinutesSinceMidnight(void);
const char* getTimeDisplayFormat(uint16_t minutes, char* out_buffer, size_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDULE_MANAGER_H */
