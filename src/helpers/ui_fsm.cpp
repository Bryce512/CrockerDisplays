#include "ui_fsm.h"
#include <Arduino.h>


enum class Screen { 
    home, 
    schedule, 
    settings,
};

static Screen currentScreen;

void ui_fsm_init()
{
    currentScreen = Screen::home;
}

void ui_fsm_tick()
{
    // transitions
    switch (currentScreen)
    {
        case Screen::home:
            if() currentScreen = Screen::schedule;
            break;

        case Screen::schedule:
            if() currentScreen = Screen::home;
            if() currentScreen = Screen::settings;
            break;

        case Screen::settings:
            if() currentScreen = Screen::schedule;
            break;

        default:
            Serial.println("error in the transitions ui_fsm");
            break;
    }
    
    // actions
    switch (currentScreen)
    {
        case Screen::home: 
            
            break;
        
        case Screen::schedule: 
              
            break;
        
        case Screen::settings: 

            break;

        default:
            Serial.println("error in the action ui_fsm");
            break;
    }

    
}