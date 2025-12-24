#include "timer_event_handler.h"
#include "JSON_reader.h"
#include <Arduino.h>

#define MAX_EVENTS 18

static readConfig queue[MAX_EVENTS];
static size_t queue_count = 0;
static size_t queue_index = 0;

bool timer_load_queue()
{
    queue_count = 0;
    queue_index = 0;
    if (!readJSONQueue(queue, MAX_EVENTS, queue_count)) {
        Serial.println("Failed to load timer queue");
        return false;
    }
    return true;
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