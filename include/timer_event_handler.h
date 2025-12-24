#pragma once
#ifndef TIMER_EVENT_HANDLER_H
#define TIMER_EVENT_HANDLER_H

#include "JSON_reader.h"
#include <cstddef>

bool timer_load_queue();
bool timer_has_next();
bool timer_pop_next(readConfig& out_event);
size_t _queue_count();
size_t _queue_index();

#endif
