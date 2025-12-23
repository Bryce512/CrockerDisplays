#pragma once
#include <cstdint>

#ifndef JSON_WRITER_H
#define JSON_WRITER_H

struct writeConfig {
    uint16_t start;    // start time in 24hr timer
    uint16_t duration; // duration in seconds
    const char* label;   // activity label
    const char* path;    // image path
};

void writeJSON(const writeConfig& cfg);

#endif