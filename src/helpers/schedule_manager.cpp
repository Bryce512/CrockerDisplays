#include "schedule_manager.h"
#include "JSON_reader.h"
#include <time.h>
#include <algorithm>

// Local event cache (small buffer, refetch as needed)
static readConfig eventBuffer[16];  // Support up to 16 events per day (~1088 bytes)
static size_t eventBufferCount = 0;
static unsigned long lastFetchTime = 0;
static const unsigned long CACHE_DURATION = 60000;  // 60 seconds

/**
 * Invalidate the event cache (called after new JSON is saved via BLE)
 */
void invalidateScheduleCache() {
    eventBufferCount = 0;
    lastFetchTime = 0;
    Serial.println("[SCHEDULE] Cache invalidated, will refetch from SD card");
}

/**
 * Get current time as minutes since midnight
 */
uint16_t getCurrentMinutesSinceMidnight() {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    return (timeinfo->tm_hour * 60) + timeinfo->tm_min;
}

/**
 * Fetch and cache events from JSON (with caching to avoid repeated SD reads)
 * Events are expected to have start times in MINUTES SINCE MIDNIGHT (0-1439)
 */
static void fetchEventsIfNeeded() {
    unsigned long now = millis();
    
    // Only refetch if cache is stale or empty
    if (eventBufferCount == 0 || (now - lastFetchTime > CACHE_DURATION)) {
        Serial.printf("[SCHEDULE] Fetching events from SD card (count=%u, age=%lu ms)\n", 
            eventBufferCount, now - lastFetchTime);
        
        eventBufferCount = 0;
        if (readJSONQueue(eventBuffer, 16, &eventBufferCount)) {
            // Sort by start time (ascending)
            std::sort(eventBuffer, eventBuffer + eventBufferCount,
                [](const readConfig& a, const readConfig& b) {
                    return a.start < b.start;
                });
            
            // Debug: Log loaded events
            Serial.printf("[SCHEDULE] Successfully loaded %u events:\n", eventBufferCount);
            for (size_t i = 0; i < eventBufferCount; i++) {
                uint16_t hours = eventBuffer[i].start / 60;
                uint16_t mins = eventBuffer[i].start % 60;
                uint16_t durationMins = eventBuffer[i].duration / 60;
                Serial.printf("  [%u] %02u:%02u - %s (duration: %u min)\n", 
                    i, hours, mins, eventBuffer[i].label, durationMins);
            }
        } else {
            Serial.println("[SCHEDULE] Failed to load events from JSON");
        }
        lastFetchTime = now;
    }
}

/**
 * Get the current event happening right now (if any)
 * Compares current time (in minutes since midnight) against event start and end times
 */
ScheduleEvent* getCurrentScheduleEvent() {
    fetchEventsIfNeeded();
    
    uint16_t currentMinutes = getCurrentMinutesSinceMidnight();
    char timeStr[8];
    getTimeDisplayFormat(currentMinutes, timeStr, sizeof(timeStr));
    Serial.printf("[SCHEDULE] getCurrentScheduleEvent() - current time: %s, checking %u events\n", 
        timeStr, (unsigned int)eventBufferCount);
    
    for (size_t i = 0; i < eventBufferCount; i++) {
        uint16_t eventEnd = eventBuffer[i].start + (eventBuffer[i].duration / 60);
        
        // Check if current time falls within this event's window
        if (currentMinutes >= eventBuffer[i].start && currentMinutes < eventEnd) {
            Serial.printf("[SCHEDULE]   -> Found current event: %s\n", eventBuffer[i].label);
            return &eventBuffer[i];
        }
    }
    
    Serial.println("[SCHEDULE]   -> No current event found");
    return nullptr;
}

/**
 * Get the next event coming up
 */
ScheduleEvent* getNextScheduleEvent() {
    fetchEventsIfNeeded();
    
    uint16_t currentMinutes = getCurrentMinutesSinceMidnight();
    
    for (size_t i = 0; i < eventBufferCount; i++) {
        if (eventBuffer[i].start > currentMinutes) {
            Serial.printf("[SCHEDULE]   -> Found next event: %s\n", eventBuffer[i].label);
            return &eventBuffer[i];
        }
    }
    
    Serial.println("[SCHEDULE]   -> No next event found");
    return nullptr;
}

/**
 * Get minutes until the next event starts
 */
uint16_t getMinutesUntilNextEvent() {
    ScheduleEvent* nextEvent = getNextScheduleEvent();
    
    if (!nextEvent) {
        return 0;
    }
    
    uint16_t currentMinutes = getCurrentMinutesSinceMidnight();
    return nextEvent->start - currentMinutes;
}

/**
 * Get how many minutes remaining in current event
 */
uint16_t getMinutesRemainingInCurrentEvent() {
    ScheduleEvent* currentEvent = getCurrentScheduleEvent();
    
    if (!currentEvent) {
        return 0;
    }
    
    uint16_t currentMinutes = getCurrentMinutesSinceMidnight();
    uint16_t eventEnd = currentEvent->start + (currentEvent->duration / 60);
    
    if (currentMinutes >= eventEnd) {
        return 0;
    }
    
    return eventEnd - currentMinutes;
}

/**
 * Check if an event is currently active
 */
bool isEventActive() {
    return getCurrentScheduleEvent() != nullptr;
}
/**
 * Get all events for today (used by UI to display the full schedule)
 */
size_t getAllScheduleEvents(ScheduleEvent** out_events, size_t max_events) {
    fetchEventsIfNeeded();
    
    size_t count = eventBufferCount < max_events ? eventBufferCount : max_events;
    for (size_t i = 0; i < count; i++) {
        out_events[i] = &eventBuffer[i];
    }
    
    Serial.printf("[SCHEDULE] getAllScheduleEvents() returning %u events\n", (unsigned int)count);
    return count;
}

/**
 * Convert minutes since midnight to HH:MM format for display
 * Input: minutes (0-1439)
 * Output: buffer filled with "HH:MM" format (e.g., "07:00", "14:30")
 * Returns: pointer to the output buffer
 */
const char* getTimeDisplayFormat(uint16_t minutes, char* out_buffer, size_t buffer_size) {
    if (!out_buffer || buffer_size < 6) {
        return "";  // Buffer too small
    }
    
    uint16_t hours = minutes / 60;
    uint16_t mins = minutes % 60;
    
    snprintf(out_buffer, buffer_size, "%02u:%02u", hours, mins);
    return out_buffer;
}