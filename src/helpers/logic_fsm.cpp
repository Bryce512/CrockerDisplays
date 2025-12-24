#include "logic_fsm.h"
#include "alarm.h"
#include "timer_functions.h"
#include "timer_event_handler.h"
#include "JSON_reader.h"



enum class State { 
    init,
    start, 
    run,
    end, 
    error };

static State currentState;
static readConfig current_evt;

void logic_fsm_init()
{
    currentState = State::init;
}

void logic_fsm_tick()
{
    // transitions
    switch (currentState)
    {
        case State::init: // If it is time for a timer to start, move to run state
            if(_queue_count() != 0) {
                currentState = State::start;
                Serial.println("Idle -> Start");
            }
            break;

        case State::start:
            if(_is_timer_active()) {
                currentState = State::run;
                Serial.println("Start -> Run");
            }
            break;

        case State::run: // runs for the duration of the timer, when the timer is finished move to end
            if (!_is_timer_active()) {
                currentState = State::end;
                Serial.println("Run -> End");
            }
            break;
        
        case State::end: // Plays the alarm sound until the end, checks 
            if (_queue_index() == _queue_count()) {
                currentState = State::init;
                Serial.println("End -> Init");
            } else if (!_is_alarm_active()) {
                currentState = State::start;
                Serial.println("End -> Start");
            }
            break;

        case State::error:
            break;

        default:
            break;
    }
    
    // actions
    switch (currentState)
    {
        case State::init: 
            timer_load_queue();
            break;
        
        case State::start:
            timer_pop_next(current_evt);
            start_timer(current_evt.duration);
            update_event_text(current_evt.label);
            break;

        case State::run: {
            // Update timer display
            update_timer_display();
            break;
        }

        case State::end:
            //update_alarm();
            break;

        case State::error: 
            break;

        default:
            break;
    }

    
}