 #include "logic_fsm.h"

enum class State { 
    idle, 
    run,
    end, 
    error };

static State currentState;

void logic_fsm_init()
{
    currentState = State::idle;
}

void logic_fsm_tick()
{
    // transitions
    switch (currentState)
    {
        case State::idle:
            if() currentState = State::run;
            break;

        case State::run:
            if () currentState = State::error;
            if () currentState = State::end;
            break;
        
        case State::end:
            if () currentState = State::idle;

        case State::error:
            if () currentState = State::idle;
            break;

        default:
            break;
    }
    
    // actions
    switch (currentState)
    {
        case State::idle: 
             
            break;
        
        case State::run: 
              
            break;
        
        case State::error: 
            break;

        default:
            break;
    }

    
}