#ifndef JSON_READER_H
#define JSON_READER_H

#include <stdint.h>
#include <stddef.h>

/* Event configuration structure */
struct readConfig {
    uint16_t start;
    uint16_t duration;
    char label[32];
    char path[32];
};

#ifdef __cplusplus
extern "C" {
#endif

bool readJSONQueue(struct readConfig* out_events, size_t max_events, size_t* out_count);

#ifdef __cplusplus
}
#endif

#endif /* JSON_READER_H */
