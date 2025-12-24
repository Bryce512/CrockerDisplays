#include "timer_functions.h"
#include "squarelineUI/ui.h"
#include "alarm.h"

// ============ TIMER STATE ============
TimerState timer = {0, 0, false};

// ============ TIMER IMPLEMENTATION ============

void start_timer(uint32_t duration) {
    timer.start_time = millis();
    timer.duration = duration * 1000;
    timer.active = true;
    
    Serial.print("Timer started: ");
    Serial.print(duration);
    Serial.println(" seconds");
}

void update_timer_display() {
    if (!timer.active) return;
    
    uint32_t elapsed = millis() - timer.start_time;
    
    if (elapsed >= timer.duration) {
        // Timer finished
        timer.active = false;
        update_timer(timer.duration, timer.duration);  // Arc at 100%
        trigger_alarm();
        Serial.println("Timer finished!");
        return;
    }
    
    // Update the arc with elapsed time
    update_timer(elapsed, timer.duration);
    
    // Update time label with countdown
    uint32_t remaining = timer.duration - elapsed;
    uint16_t minutes = remaining / 60000;
    uint16_t seconds = (remaining % 60000) / 1000;
    
    char time_str[16];
    sprintf(time_str, "%d:%02d", minutes, seconds);
    update_time_text(time_str);
}

void update_current_time() {
    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    
    char time_str[16];
    sprintf(time_str, "%02d:%02d", timeinfo->tm_hour, timeinfo->tm_min);
    
    update_time_text(time_str);
}

// Gettter function to determine if the timer is active
bool _is_timer_active() {
    return timer.active;
}