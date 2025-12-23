#pragma once
#include <cstdint>

#ifndef JSON_READER_H
#define JSON_READER_H

struct readConfig {
    uint16_t start = 0;    // start time in 24hr timer
    uint16_t duration = 0; // duration in seconds
    char label[32] = "";   // activity label
    char path[32] = "";    // image path
};

bool readJSON(readConfig& cfg);

#endif